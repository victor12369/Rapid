#include "ast.h"

#include "allocation.h"
#include "global.h"
namespace rapid {
namespace internal {
struct ZonePage {
  ZonePage* prev;
  size_t size;
  size_t used;
  uint8_t data[];
};
class CMZImpl {
  static constexpr size_t ZonePageSize = 1024 * 4 - sizeof(ZonePage);

 private:
  ZonePage* m_top;
  size_t m_usage;

  CMZImpl() = default;

 public:
  static CMZImpl* Create() {
    CMZImpl* p = Allocate<CMZImpl>();
    p->m_top = nullptr;
    p->m_usage = 0;
    return p;
  }
  void PrepareAlloc() {
    ASSERT(m_top == nullptr);
    m_usage = 0;
    m_top = Allocate<ZonePage>();
    m_top->prev = nullptr;
    m_top->size = m_top->used = 0;
  }
  void FreeAll() {
    while (m_top != nullptr) {
      ZonePage* pdel = m_top;
      m_top = m_top->prev;
      free(pdel);
    }
  }

  void* Alloc(size_t size) {
    ASSERT(size != 0);
    // return malloc(size);  // for debug
    ASSERT(m_top);
    if (m_top->used + size <= m_top->size) {
      void* p = m_top->data + m_top->used;
      m_top->used += size;
      return p;
    }
    size_t alloc_size = size > ZonePageSize / 2  //如果对象较大
                            ? size  //则分配空间相等的ZonePage以避免内存浪费
                            : ZonePageSize;  //否则按ZonePageSize进行分配
    m_usage += alloc_size;
    ZonePage* zp = AllocateSize<ZonePage>(sizeof(ZonePage) + alloc_size);
    zp->prev = m_top;
    zp->size = alloc_size;
    zp->used = size;
    m_top = zp;
    return zp->data;
  }
  size_t GetUsage() { return m_usage; }
};
CompilingMemoryZone* CompilingMemoryZone::Create() {
  return reinterpret_cast<CompilingMemoryZone*>(CMZImpl::Create());
}
void CompilingMemoryZone::PrepareAlloc() {
  return reinterpret_cast<CMZImpl*>(Global::GetCMZ())->PrepareAlloc();
}
void CompilingMemoryZone::FreeAll() {
  return reinterpret_cast<CMZImpl*>(Global::GetCMZ())->FreeAll();
}
size_t CompilingMemoryZone::GetUsage() {
  return reinterpret_cast<CMZImpl*>(Global::GetCMZ())->GetUsage();
}
void* CompilingMemoryZone::Alloc(size_t size) {
  return reinterpret_cast<CMZImpl*>(Global::GetCMZ())->Alloc(size);
}
}  // namespace internal
}  // namespace rapid
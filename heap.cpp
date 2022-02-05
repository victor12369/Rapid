#include "heap.h"

#include <malloc.h>

#include <cstring>
#include <type_traits>

#include "debug.h"
#include "executer.h"
#include "global.h"
#include "handle.h"
#include "object.h"
#include "util.h"
namespace rapid {
namespace internal {

class HeapImpl /*public: Heap --
                  不应该继承，否则Heap中调用函数时可能调用未被重写的函数*/
{
  size_t m_usage;
  size_t m_last_gc_usage;  //上次gc后
  uint8_t m_color;
  Object *m_true, *m_false;
  bool m_enable_gc;
  // Object  *m_null;
  struct {
    HeapObject *first, *last;
  } m_objs, m_roots;
  uint64_t m_object_count;

 public:
  //用于为VS的堆分析提供类型参数
  template <class T>
  __declspec(allocator) T *AllocObject(size_t size) {
    static_assert(std::is_base_of_v<HeapObject, T>, "");
    T *p = (T *)malloc(size);
    m_usage += size;
    p->m_alloc_size = size;
    return p;
  }
  void Register(HeapObject *obj) {
    ++this->m_object_count;
    obj->m_gctag = this->m_color;
    obj->m_nextobj = nullptr;
    this->m_objs.last->m_nextobj = obj;
    this->m_objs.last = obj;
  }

  void RegisterRoot(HeapObject *obj) {
    obj->m_nextobj = nullptr;
    this->m_roots.last->m_nextobj = obj;
    this->m_roots.last = obj;
  }

  void FreeObject(HeapObject *obj) {
    /* switch (obj->m_heapobj_type) {
       case HeapObjectType::Array: break;
       case HeapObjectType::String: break;
       case HeapObjectType::Table: break;
       case HeapObjectType::FuncData: break;
       default: ASSERT(0);
     }*/
    m_usage -= obj->m_alloc_size;
    --this->m_object_count;
    free(obj);
  }

  static HeapImpl *Create() {
    HeapImpl *h = Allocate<HeapImpl>();
    VERIFY(h != nullptr);
    h->m_color = 0;
    h->m_usage = 0;
    h->m_object_count = 0;
    h->m_enable_gc = false;
    h->m_last_gc_usage = 1024;

    h->m_objs.first = h->m_objs.last =
        (HeapObject *)h->AllocObject<HeapObject>(sizeof(HeapObject));
    h->m_objs.last->m_gctag = h->m_color;
    h->m_objs.last->m_nextobj = nullptr;

    h->m_roots.first = h->m_roots.last =
        (HeapObject *)h->AllocObject<HeapObject>(sizeof(HeapObject));
    h->m_roots.last->m_gctag = h->m_color;
    h->m_roots.last->m_nextobj = nullptr;

    // h->m_null =
    //(SpecialValue *)h->AllocObject<SpecialValue>(sizeof(SpecialValue));
    h->m_true =
        (SpecialValue *)h->AllocObject<SpecialValue>(sizeof(SpecialValue));
    h->m_false =
        (SpecialValue *)h->AllocObject<SpecialValue>(sizeof(SpecialValue));

    // reinterpret_cast<SpecialValue *>(h->m_null)->m_heapobj_type =
    // HeapObjectType::SpecialValue;
    reinterpret_cast<SpecialValue *>(h->m_true)->m_heapobj_type =
        HeapObjectType::SpecialValue;
    reinterpret_cast<SpecialValue *>(h->m_false)->m_heapobj_type =
        HeapObjectType::SpecialValue;

    // reinterpret_cast<SpecialValue *>(h->m_null)->m_interface =
    //&SpecialValue::Interface;
    reinterpret_cast<SpecialValue *>(h->m_true)->m_interface =
        &SpecialValue::Interface;
    reinterpret_cast<SpecialValue *>(h->m_false)->m_interface =
        &SpecialValue::Interface;

    // reinterpret_cast<SpecialValue *>(h->m_null)->m_val =
    // SpecialValue::NullVal;
    reinterpret_cast<SpecialValue *>(h->m_true)->m_val = SpecialValue::TrueVal;
    reinterpret_cast<SpecialValue *>(h->m_false)->m_val =
        SpecialValue::FalseVal;

    return h;
  }

  static void Destory(Heap *heap) {
    // TODO:实现HeapImpl::Destory
  }

#define ALLOC_HEAPOBJECT(_p, _t)                    \
  static_assert(std::is_same_v<decltype(_p), _t *>, \
                "ALLOC_HEAPOBJECT type not match"); \
  _p->m_heapobj_type = HeapObjectType::_t;          \
  _p->m_interface = &_t::Interface;                 \
  Register(_p);

  String *AllocString(const char *cstr, size_t length) {
    String *s = (String *)AllocObject<String>(sizeof(String) + length + 1);
    s->m_length = length;
    s->m_hash = hash_string(cstr, length);
    s->m_cached = false;
    memcpy(s->m_data, cstr, length);
    s->m_data[length] = '\0';
    ALLOC_HEAPOBJECT(s, String);
    return s;
  }
  Array *AllocArray(size_t reserved) {
    Array *p = (Array *)AllocObject<Array>(sizeof(Array));
    p->m_length = 0;
    p->m_array = AllocFixedArray(reserved);
    ALLOC_HEAPOBJECT(p, Array);
    return p;
  }
  Table *AllocTable(size_t reserve) {
    Table *p = (Table *)AllocObject<Table>(sizeof(Table));
    p->m_table = AllocFixedTable(reserve * 2);
    ALLOC_HEAPOBJECT(p, Table);
    return p;
  }
  FixedArray *AllocFixedArray(size_t length) {
    FixedArray *p = (FixedArray *)AllocObject<FixedArray>(
        sizeof(FixedArray) + sizeof(Object *) * length);
    p->m_length = length;
    // Object *null_v = NullValue();
    for (size_t i = 0; i < length; i++) p->m_data[i] = nullptr;
    ALLOC_HEAPOBJECT(p, FixedArray);
    return p;
  }
  InstructionArray *AllocInstructionArray(size_t length) {
    InstructionArray *p = (InstructionArray *)AllocObject<InstructionArray>(
        sizeof(InstructionArray) + sizeof(Cmd) * length);
    p->m_length = length;
    memset(p->m_data, 0, sizeof(Cmd) * length);
    ALLOC_HEAPOBJECT(p, InstructionArray);
    return p;
  }
  FixedTable *AllocFixedTable(size_t size) {
    FixedTable *p = (FixedTable *)AllocObject<FixedTable>(
        sizeof(FixedTable) + sizeof(TableNode) * size);
    p->m_size = size;
    p->m_used = 0;
    p->m_pfree = p->m_data;
    memset(p->m_data, 0, sizeof(TableNode) * size);
    ALLOC_HEAPOBJECT(p, FixedTable);
    return p;
  }
  NativeObject *AllocNativeObject(void *data,
                                  const ObjectInterface *interface) {
    NativeObject *p =
        (NativeObject *)AllocObject<NativeObject>(sizeof(NativeObject));
    ALLOC_HEAPOBJECT(p, NativeObject);  //此处也会写入m_interface，放前面
    p->m_interface = interface;
    p->m_data = data;
    return p;
  }
  Exception *AllocExpection(String *type, String *info, Object *data) {
    Exception *p = (Exception *)AllocObject<Exception>(sizeof(Exception));
    p->m_type = type;
    p->m_info = info;
    p->m_data = data;
    p->m_stacktrace = AllocArray(0);
    ALLOC_HEAPOBJECT(p, Exception);
    return p;
  }
  void EnableGC() { m_enable_gc = true; }
  // Object *NullValue() { return this->m_null; }
  Object *TrueValue() { return this->m_true; }
  Object *FalseValue() { return this->m_false; }
#define ALLOC_STRUCT_IMPL(_t)                \
  _t *p = (_t *)AllocObject<_t>(sizeof(_t)); \
  memset(p, 0, sizeof(_t));                  \
  p->m_alloc_size = sizeof(_t);              \
  ALLOC_HEAPOBJECT(p, _t);                   \
  return p;
#define ITERATOR_DEF_ALLOC_STRUCT(T) \
  T *Alloc##T() { ALLOC_STRUCT_IMPL(T); }
  ITER_STRUCT_DERIVED(ITERATOR_DEF_ALLOC_STRUCT)
  //VarInfo *AllocVarData() { ALLOC_STRUCT_IMPL(VarInfo); }
  //ExternVarInfo *AllocExternVarData() { ALLOC_STRUCT_IMPL(ExternVarInfo); }
  //SharedFunctionData *AllocSharedFunctionData() {
  //  ALLOC_STRUCT_IMPL(SharedFunctionData);
  //}
  //ExternVar *AllocExternVar() { ALLOC_STRUCT_IMPL(ExternVar); }
  //FunctionData *AllocFunctionData() { ALLOC_STRUCT_IMPL(FunctionData); }
  uint64_t ObjectCount() { return this->m_object_count; }
  void DoGC() {
    if (!m_enable_gc) return;
    size_t pre_gc_usage = m_usage;
    // return;  // NO_GC
    // DBG_LOG("begin gc\n");
    this->m_color = (this->m_color + 1) & 1;
    GCTracer gct(this->m_color);
    // DBG_LOG("begin trace\n");
    for (HeapObject *p = this->m_roots.first->m_nextobj; p != nullptr;
         p = p->m_nextobj) {
      gct.Trace(p);
      // DBG_LOG("trace object: %p\n", p);
    }
    Executer::TraceStack(&gct);
    // DBG_LOG("trace HandleContainer\n");
    HandleContainer::TraceRef(&gct);
    // DBG_LOG("end trace\n");
    HeapObject *p = this->m_objs.first->m_nextobj, *pre = this->m_objs.first;
    // DBG_LOG("begin sweep\n");
    while (p != nullptr) {
      if (p->m_gctag != this->m_color) {
        HeapObject *pdel = p;
        ASSERT((p == this->m_objs.last) == (p->m_nextobj == nullptr));
        if (p == this->m_objs.last) {
          this->m_objs.last = pre;
        }
        p = p->m_nextobj;
        pre->m_nextobj = p;

        DBG_LOG("sweep object:%p\n", pdel);
        FreeObject(pdel);
      } else {
        pre = p;
        p = p->m_nextobj;
      }
    }
    // DBG_LOG("end sweep\n");
    // DBG_LOG("end gc\n");
    DBG_LOG("gc complete: before:%llu after:%llu reduce:%lld\n", pre_gc_usage,
            m_usage, pre_gc_usage - m_usage);
    m_last_gc_usage = m_usage;
  }
  bool NeedGC() { return m_usage > m_last_gc_usage * 2; }
  void PrintHeap() {
    HeapObject *p = this->m_objs.first->m_nextobj;
    while (p != nullptr) {
      debug_print(stdout, p);
      printf("\n");
      p = p->m_nextobj;
    }
  }
};
#define CALL_HEAP_IMPL(_f, ...) \
  (reinterpret_cast<HeapImpl *>(Global::GetHeap())->_f(__VA_ARGS__))

Heap *Heap::Create() { return reinterpret_cast<Heap *>(HeapImpl::Create()); }

void Heap::Destory(Heap *heap) { return HeapImpl::Destory(heap); }

String *Heap::AllocString(const char *cstr, size_t length) {
  return CALL_HEAP_IMPL(AllocString, cstr, length);
}
Array *Heap::AllocArray(size_t reserved) {
  return CALL_HEAP_IMPL(AllocArray, reserved);
}
Table *Heap::AllocTable(size_t reserved) {
  return CALL_HEAP_IMPL(AllocTable, reserved);
}
FixedArray *Heap::AllocFixedArray(size_t length) {
  return CALL_HEAP_IMPL(AllocFixedArray, length);
}
FixedTable *Heap::AllocFixedTable(size_t size) {
  return CALL_HEAP_IMPL(AllocFixedTable, size);
}
InstructionArray *Heap::AllocInstructionArray(size_t length) {
  return CALL_HEAP_IMPL(AllocInstructionArray, length);
}
// Object *Heap::NullValue() { return CALL_HEAP_IMPL(NullValue); }
Object *Heap::TrueValue() { return CALL_HEAP_IMPL(TrueValue); }
Object *Heap::FalseValue() { return CALL_HEAP_IMPL(FalseValue); }
Exception *Heap::AllocException(String *type, String *info, Object *data) {
  return CALL_HEAP_IMPL(AllocExpection, type, info, data);
}
VarInfo *Heap::AllocVarInfo() { return CALL_HEAP_IMPL(AllocVarInfo); }
TryCatchTable *Heap::AllocTryCatchTable() {
  return CALL_HEAP_IMPL(AllocTryCatchTable);
}
ExternVarInfo *Heap::AllocExternVarInfo() {
  return CALL_HEAP_IMPL(AllocExternVarInfo);
}
SharedFunctionData *Heap::AllocSharedFunctionData() {
  return CALL_HEAP_IMPL(AllocSharedFunctionData);
}
ExternVar *Heap::AllocExternVar() { return CALL_HEAP_IMPL(AllocExternVar); }
FunctionData *Heap::AllocFunctionData() {
  return CALL_HEAP_IMPL(AllocFunctionData);
}
NativeObject *Heap::AllocNativeObject(void *data,
                                      const ObjectInterface *interface) {
  return CALL_HEAP_IMPL(AllocNativeObject, data, interface);
}
uint64_t Heap::ObjectCount() { return CALL_HEAP_IMPL(ObjectCount); }
void Heap::PrintHeap() { return CALL_HEAP_IMPL(PrintHeap); }
void Heap::EnableGC() { return CALL_HEAP_IMPL(EnableGC); }
void Heap::DoGC() { return CALL_HEAP_IMPL(DoGC); }
bool Heap::NeedGC() { return CALL_HEAP_IMPL(NeedGC); }

GCTracer::GCTracer(uint8_t color) : m_color(color) {}

void GCTracer::Trace(HeapObject *p) {
  if (p == nullptr) return;
  if (p->m_gctag == m_color) return;
  p->m_gctag = m_color;
  p->m_interface->trace_ref(p, this);
}
void GCTracer::Trace(Object *p) {
  if (p == nullptr) return;
  if (p->IsHeapObject()) Trace(HeapObject::cast(p));
}
}  // namespace internal
}  // namespace rapid
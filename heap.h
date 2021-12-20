#pragma once
//#include "object.h"
#include "preprocessors.h"
namespace rapid {
namespace internal {
class Object;
class HeapObject;
class String;
class Array;
class Table;
class FixedArray;
class FixedTable;
class VarData;
class ExternVarData;
class ExternVar;
class SharedFunctionData;
class InstructionArray;
class Heap {
 public:
  static void* RawAlloc(size_t size);
  static void RawFree(void* p);

 public:
  static Heap* Create();
  static void Destory(Heap* heap);

 public:  //���º����������GC
  static String* AllocString(const char* cstr, size_t length);
  static Array* AllocArray();
  static Table* AllocTable();
  static FixedArray* AllocFixedArray(size_t length);
  static FixedTable* AllocFixedTable(size_t size);
  static InstructionArray* AllocInstructionArray(size_t length);
  static Object* NullValue();
  static Object* TrueValue();
  static Object* FalseValue();
  static VarData* AllocVarData();
  static ExternVarData* AllocExternVarData();
  static SharedFunctionData* AllocSharedFunctionData();
  static ExternVar* AllocExternVar();
  static uint64_t ObjectCount();

 public:
  static void DoGC();
};

class GCTracer {
  uint8_t m_color;

 public:
  GCTracer(uint8_t color);
  void Trace(HeapObject* p);
  void Trace(Object* p);
  template <class... TArgs>
  inline void TraceAll(TArgs... args) {
    (Trace(args), ...);
  }
};

}  // namespace internal
}  // namespace rapid
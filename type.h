#pragma once
namespace rapid {
namespace internal {

typedef uint8_t byte;
typedef uint8_t Cmd;
class Parameters;
class String;
class Object;
class GCTracer;

typedef Object* (*CFunctionPtr)(Parameters*);
typedef Object* (*GetMember)(Object* obj, String* name);
typedef Object* (*SetMember)(Object* obj, String* name, Object* val);
typedef Object* (*GetIndex)(Object* obj, Object* index);
typedef Object* (*SetIndex)(Object* obj, Object* index, Object* val);
typedef void (*TraceRef)(Object* obj, GCTracer*);

struct ObjectInterface {
  GetMember get_member;
  SetMember set_member;
  GetIndex get_index;
  SetIndex set_index;
  TraceRef trace_ref;
};

}  // namespace internal
}  // namespace rapid
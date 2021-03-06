#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>

#include "handle.h"
#include "preprocessors.h"
namespace rapid {
namespace internal {
/*




*/

enum class Opcode : uint8_t {
  NOP,
  LOADL,
  STOREL,
  STOREE,
  LOADK,
  LOADE,
  // LOAD_THIS,
  LOAD_PARAMS,
  IMPORT,
  COPY,  //复制栈顶并push到栈上
  // PUSH,  //用于定义变量时将初始值push到栈上 ---
  // 不需要此指令，因为变量就保存在计算栈上，无需操作
  PUSH_NULL,  //用于定义变量时将初始值null push到栈上
  POP,
  POPN,  // pop n个，u8[n]

  CLOSE,  // u16 关闭一个externvar

  MAKE_ARRAY,    // u16[元素个数] ; <item_0> <item_1> ... <item_n>
  MAKE_ARRAY_0,  // 0

  MAKE_DICTIONARY,    // u16[键值对的个数] ; <key_0> <val_0> ... <key_n> <val_n>
  MAKE_DICTIONARY_0,  // 0

  MAKE_TABLE,  // u16[tableinfo索引] ;<prop_val_0> <prop_val_1> ... <prop_val_n>

  TABLE_INIT,  // 0 ; <FixedArray:元素名称数组>

  ADD,
  SUB,
  MUL,
  IDIV,
  FDIV,
  MOD,

  BAND,
  BOR,
  BNOT,
  BXOR,
  SHL,
  SHR,

  NOT,
  AND,
  OR,

  LT,   //<
  GT,   //>
  LE,   //<=
  GE,   //>=
  EQ,   //==
  NEQ,  //!=

  GET_P,  // obj name
  GET_I,  // obj index
  SET_P,  // value obj name
  SET_I,  // value obj index

  JMP,
  JMP_T,  //消耗栈顶元素
  JMP_F,  //消耗栈顶元素

  NEG,
  ACT,

  CALL,  // CALL 3 -> func p1 p2 p3

  RET,
  RETNULL,
  CLOSURE,
  // CLOSURE_SELF,

  FOR_RANGE_BEGIN,  // u16[循环变量] s32[跳转位置-FOR_RANGE_END的下一条指令] ---
                    // 循环变量后总是紧跟两个匿名变量end和step
  FOR_RANGE_END,  // s32[跳转位置-FOR_RANGE_BEGIN]

  PLACE_HOLDER,

  __SIZE,

};

enum class TokenType {
  NUL,
  END,
  SYMBOL,
  KVAL,

  FUNC,
  RETURN,
  VAR,
  IF,
  ELSE,
  FOR,
  WHILE,
  CONTINUE,
  BREAK,
  EXPORT,
  IMPORT,
  CONST,
  TRY,
  CATCH,
  FINALLY,
  THROW,
  THIS,
  PARAMS,

  BK_SL,
  BK_SR,
  BK_ML,
  BK_MR,
  BK_LL,
  BK_LR,
  COMMA,
  DOT,
  SEMI,
  COLON,  //:

  ADD,  //既代表二元加法，又代表一元‘正’
  SUB,  //既代表二元减法，又代表一元‘负’
  MUL,
  IDIV,
  FDIV,
  MOD,
  BAND,
  BOR,
  BNOT,
  BXOR,
  SHL,
  SHR,

  NOT,
  AND,
  OR,

  ASSIGN,
  ADD_ASSIGN,
  SUB_ASSIGN,
  MUL_ASSIGN,
  IDIV_ASSIGN,
  FDIV_ASSIGN,
  MOD_ASSIGN,
  BAND_ASSIGN,
  BOR_ASSIGN,
  BNOT_ASSIGN,
  BXOR_ASSIGN,
  SHL_ASSIGN,
  SHR_ASSIGN,

  LT,
  GT,
  LE,
  GE,
  NEQ,
  EQ,

  __SIZE,
};
#define TT_ITER_KEWWORD(V)           \
  V(TokenType::FUNC, "func")         \
  V(TokenType::RETURN, "return")     \
  V(TokenType::VAR, "var")           \
  V(TokenType::IF, "if")             \
  V(TokenType::ELSE, "else")         \
  V(TokenType::FOR, "for")           \
  V(TokenType::WHILE, "while")       \
  V(TokenType::CONTINUE, "continue") \
  V(TokenType::BREAK, "break")       \
  V(TokenType::EXPORT, "export")     \
  V(TokenType::CONST, "const")       \
  V(TokenType::TRY, "try")           \
  V(TokenType::THIS, "this")         \
  V(TokenType::PARAMS, "params")     \
  V(TokenType::CATCH, "catch")

#define TT_ITER_CONTROL(V) \
  V(TokenType::BK_SL, "(") \
  V(TokenType::BK_SR, ")") \
  V(TokenType::BK_ML, "[") \
  V(TokenType::BK_MR, "]") \
  V(TokenType::BK_LL, "{") \
  V(TokenType::BK_LR, "}") \
  V(TokenType::COMMA, ",") \
  V(TokenType::DOT, ".")   \
  V(TokenType::SEMI, ";") V(TokenType::COLON, ":")

#define TT_ITER_OPERATOR(V)        \
  V(TokenType::ADD, "+")           \
  V(TokenType::SUB, "-")           \
  V(TokenType::MUL, "*")           \
  V(TokenType::IDIV, "//")         \
  V(TokenType::FDIV, "/")          \
  V(TokenType::MOD, "%")           \
  V(TokenType::BNOT, "~")          \
  V(TokenType::BAND, "&")          \
  V(TokenType::BOR, "|")           \
  V(TokenType::SHL, "<<")          \
  V(TokenType::SHR, ">>")          \
  V(TokenType::NOT, "!")           \
  V(TokenType::AND, "&&")          \
  V(TokenType::OR, "||")           \
  V(TokenType::ASSIGN, "=")        \
  V(TokenType::ADD_ASSIGN, "+=")   \
  V(TokenType::SUB_ASSIGN, "-=")   \
  V(TokenType::MUL_ASSIGN, "*=")   \
  V(TokenType::IDIV_ASSIGN, "//=") \
  V(TokenType::FDIV_ASSIGN, "/=")  \
  V(TokenType::MOD_ASSIGN, "%=")   \
  V(TokenType::BAND_ASSIGN, "&=")  \
  V(TokenType::BOR_ASSIGN, "|=")   \
  V(TokenType::BNOT_ASSIGN, "~=")  \
  V(TokenType::SHL_ASSIGN, "<<=")  \
  V(TokenType::SHR_ASSIGN, ">>=")  \
  V(TokenType::LT, "<")            \
  V(TokenType::GT, ">")            \
  V(TokenType::LE, "<=")           \
  V(TokenType::GE, ">=")           \
  V(TokenType::NEQ, "!=")          \
  V(TokenType::EQ, "==")

constexpr bool IsOperator(TokenType tt) {
  switch (tt) {
#define IsOperator_ITER(t, s) case t:
    TT_ITER_OPERATOR(IsOperator_ITER)
    return true;
  }
  return false;
}
constexpr bool IsUnaryop(TokenType tt) {
  switch (tt) {
    case TokenType::NOT:
    case TokenType::BNOT:
    case TokenType::ADD:
    case TokenType::SUB:
      return true;
  }
}
constexpr bool IsBinop(TokenType tt) {
  if (tt == TokenType::ADD || tt == TokenType::SUB)
    return true;  // ADD 和 SUB 既是一元又是二元
  return IsOperator(tt) && !IsUnaryop(tt);
}
// constexpr bool IsAssignop(TokenType tt) {}
typedef uint16_t DChar;
constexpr DChar make_dchar(char c1, char c2 = '\0') {
  return (((DChar)c2) << 8) | c1;
}
constexpr DChar make_dchar(const char (&s)[3]) {
  return make_dchar(s[0], s[1]);
}
constexpr DChar make_dchar(const char (&s)[2]) { return make_dchar(s[0]); }
constexpr char dchar_get1(DChar c) { return c & 0xFF; }
constexpr char dchar_get2(DChar c) { return c >> 8; }

struct Token {
  TokenType t;
  int row, col;
  Handle<Object> v;
};
#define CASE_0(_t)                                 \
  case Opcode::_t:                                 \
    if (pbuf != nullptr) sprintf_s(pbuf, 64, #_t); \
    return 1;
#define CASE_1param(_t, _pt1)                                              \
  case Opcode::_t:                                                         \
    if (pbuf != nullptr) sprintf_s(pbuf, 64, #_t " %d", *(_pt1*)(pc + 1)); \
    return 1 + sizeof(_pt1);
#define CASE_2param(_t, _pt1, _pt2)                       \
  case Opcode::_t:                                        \
    if (pbuf != nullptr)                                  \
      sprintf_s(pbuf, 64, #_t " %d %d", *(_pt1*)(pc + 1), \
                *(_pt2*)(pc + 1 + sizeof(_pt1)));         \
    return 1 + sizeof(_pt1) + sizeof(_pt2);
#define CASE_u8(_t) CASE_1param(_t, uint8_t)
#define CASE_u16(_t) CASE_1param(_t, uint16_t)
#define CASE_s16(_t) CASE_1param(_t, int16_t)
#define CASE_s32(_t) CASE_1param(_t, int32_t)
#define CASE_u16_s32(_t) CASE_2param(_t, uint16_t, int32_t)

inline uintptr_t read_bytecode(byte* pc, char* pbuf) {
  switch ((Opcode)*pc) {
    CASE_0(NOP);
    CASE_u16(LOADL);
    CASE_u16(STOREL);
    CASE_u16(LOADK);
    CASE_u16(LOADE);
    CASE_u16(STOREE);
    CASE_u16(CLOSE);
    CASE_u16(MAKE_ARRAY);
    CASE_0(MAKE_ARRAY_0);
    CASE_u16(MAKE_DICTIONARY);
    CASE_0(MAKE_DICTIONARY_0);
    CASE_u16(MAKE_TABLE);
    CASE_0(IMPORT);
    // CASE_0(LOAD_THIS);
    CASE_0(LOAD_PARAMS);
    CASE_0(COPY);
    CASE_0(PUSH_NULL);
    CASE_0(POP);
    CASE_u8(POPN);
    CASE_0(ADD);
    CASE_0(SUB);
    CASE_0(MUL);
    CASE_0(IDIV);
    CASE_0(FDIV);
    CASE_0(MOD);
    CASE_0(BAND);
    CASE_0(BOR);
    CASE_0(BNOT);
    CASE_0(BXOR);
    CASE_0(SHL);
    CASE_0(SHR);
    CASE_0(NOT);
    CASE_0(AND);
    CASE_0(OR);
    CASE_0(LT);
    CASE_0(GT);
    CASE_0(LE);
    CASE_0(GE);
    CASE_0(EQ);
    CASE_0(NEQ);
    CASE_0(GET_P);
    CASE_0(SET_P);
    CASE_0(GET_I);
    CASE_0(SET_I);
    CASE_0(NEG);
    CASE_0(ACT);
    CASE_u16(CALL);
    // CASE_u16(THIS_CALL);
    CASE_0(RET);
    CASE_0(RETNULL);
    CASE_s16(JMP);
    CASE_s16(JMP_T);
    CASE_s16(JMP_F);
    CASE_u16(CLOSURE);
    CASE_u16_s32(FOR_RANGE_BEGIN);
    CASE_s32(FOR_RANGE_END);
      // CASE_0(CLOSURE_SELF);

    default:
      ASSERT(0);
  }
  return -1;
}

}  // namespace internal
}  // namespace rapid
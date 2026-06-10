#pragma once

#include "def.hpp"

#define meta(...)

enum struct [[meta::stringify]]
Operand_Kind
{
  meta("const")    constant,
  meta("register") register_id,
};

enum struct [[meta::stringify]]
Instr_Kind
{
  meta("ret") ret,
  meta("mov") mov,
};

struct Operand
{
  using enum Operand_Kind;
  Operand_Kind op_kind;
  union
  {
    i64 constant;
    u32 reg_id;
  };
};

struct IR_Instruction
{
  using enum Instr_Kind;
  Instr_Kind instr_kind;
  i32 result_id;
  Operand operands[3];
};


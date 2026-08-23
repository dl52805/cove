#pragma once

#include "string.hpp"

#define meta(...)

enum struct [[meta::stringify]]
HIR_Surface_Type : int
{
  unused,
  fn_def,
};

struct HIR_Surface
{
  String8 ident;
  u32 instr_offset;
  u32 instr_count;
};

enum struct [[meta::stringify]]
HIR_Instr_Type : int
{
  unused,
  ret,
  unary,
};

enum struct [[meta::stringify]]
Operand_Kind
{
  meta("unused")   unused,
  meta("const")    constant,
  meta("register") register_id,
};

struct Operand
{
  using enum Operand_Kind;
  Operand_Kind kind;
  union
  {
    i64 const_val;
    u32 reg_id;
  };
};

struct HIR_Instr
{
};

struct HIR_Program
{
};



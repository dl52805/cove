#pragma once

#include "ir.hpp"

enum struct Operand_Kind : int;
static const char *str_from_operand_kind(Operand_Kind e)
{
  switch (e)
  {
    case Operand_Kind::unused:
      return "unused";
    case Operand_Kind::constant:
      return "const";
    case Operand_Kind::register_id:
      return "register";
  }
  return "";
}

static int operand_kind_max_field_len()
{
  return 8;
}

enum struct Instr_Kind : int;
static const char *str_from_instr_kind(Instr_Kind e)
{
  switch (e)
  {
    case Instr_Kind::mov:
      return "mov";
    case Instr_Kind::ret:
      return "return";
  }
  return "";
}

static int instr_kind_max_field_len()
{
  return 6;
}


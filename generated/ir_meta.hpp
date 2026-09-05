#pragma once

#include "ir.hpp"

enum struct Instr_Kind : int;
static const char *str_from_instr_kind(Instr_Kind e)
{
  switch (e)
  {
    case Instr_Kind::unused:
      return "unused";
    case Instr_Kind::mov:
      return "mov";
    case Instr_Kind::ret:
      return "return";
    case Instr_Kind::fn_pre:
      return "fn_preamble";
    case Instr_Kind::label:
      return "label";
    case Instr_Kind::salloc:
      return "salloc";
    case Instr_Kind::neg:
      return "neg";
    case Instr_Kind::b_not:
      return "not";
    case Instr_Kind::add:
      return "add";
    case Instr_Kind::sub:
      return "sub";
    case Instr_Kind::mul:
      return "mul";
    case Instr_Kind::div:
      return "div";
    case Instr_Kind::cdq:
      return "cdq";
  }
  return "";
}

static int instr_kind_max_field_len()
{
  return 11;
}


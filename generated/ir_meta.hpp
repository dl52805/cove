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
    case Instr_Kind::label:
      return "label";
    case Instr_Kind::global:
      return "global";
  }
  return "";
}

static int instr_kind_max_field_len()
{
  return 6;
}


#pragma once

#include "ast.hpp"

enum struct Ast_Kind : int;
static const char *str_from_ast_kind(Ast_Kind e)
{
  switch (e)
  {
    case Ast_Kind::Invalid:
      return "invalid";
    case Ast_Kind::Int_Lit:
      return "int_lit";
    case Ast_Kind::Block:
      return "block";
    case Ast_Kind::Return:
      return "return";
    case Ast_Kind::Fn_Def:
      return "fn_def";
  }
  return "";
}

static int ast_kind_max_field_len()
{
  return 7;
}


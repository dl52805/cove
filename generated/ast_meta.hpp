#pragma once

#include "ast.hpp"

enum struct Unary_Op : int;
static const char *str_from_unary_op(Unary_Op e)
{
  switch (e)
  {
    case Unary_Op::unused:
      return "illegal";
    case Unary_Op::complement:
      return "complement";
    case Unary_Op::negate:
      return "negate";
    case Unary_Op::cond_not:
      return "cond_not";
  }
  return "";
}

static int unary_op_max_field_len()
{
  return 10;
}

enum struct Binary_Op : int;
static const char *str_from_binary_op(Binary_Op e)
{
  switch (e)
  {
    case Binary_Op::add:
      return "add";
    case Binary_Op::subtract:
      return "subtract";
    case Binary_Op::multiply:
      return "multiply";
    case Binary_Op::divide:
      return "divide";
    case Binary_Op::remainder:
      return "remainder";
  }
  return "";
}

static int binary_op_max_field_len()
{
  return 9;
}

enum struct Ast_Kind : int;
static const char *str_from_ast_kind(Ast_Kind e)
{
  switch (e)
  {
    case Ast_Kind::Invalid:
      return "invalid";
    case Ast_Kind::Int_Lit:
      return "int_lit";
    case Ast_Kind::Unary:
      return "unary";
    case Ast_Kind::Binary:
      return "binary";
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


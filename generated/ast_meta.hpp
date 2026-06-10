#pragma once

#include "ast.hpp"

enum struct Ast_Surface_Kind : int;
static const char *str_from_ast_surface_kind(Ast_Surface_Kind e)
{
  switch (e)
  {
    case Ast_Surface_Kind::func_def:
      return "ast_func_def";
  }
  return "";
}

static int ast_surface_kind_max_field_len()
{
  return 12;
}

enum struct Ast_Stmt_Kind : int;
static const char *str_from_ast_stmt_kind(Ast_Stmt_Kind e)
{
  switch (e)
  {
    case Ast_Stmt_Kind::return_stmt:
      return "ast_return";
    case Ast_Stmt_Kind::block_stmt:
      return "ast_block_stmt";
  }
  return "";
}

static int ast_stmt_kind_max_field_len()
{
  return 14;
}

enum struct Ast_Expr_Kind : int;
static const char *str_from_ast_expr_kind(Ast_Expr_Kind e)
{
  switch (e)
  {
    case Ast_Expr_Kind::int_literal:
      return "ast_int_literal";
    case Ast_Expr_Kind::unary_expr:
      return "ast_unary_expr";
  }
  return "";
}

static int ast_expr_kind_max_field_len()
{
  return 15;
}


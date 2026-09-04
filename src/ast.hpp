#pragma once

#include "def.hpp"

#include "lex.hpp"

enum struct [[meta::stringify]]
Unary_Op : int
{
  meta("illegal")    unused,
  meta("complement") complement,
  meta("negate")     negate,
  meta("cond_not")   cond_not,
};

enum struct [[meta::stringify]]
Binary_Op : int
{
  meta("add")        add,
  meta("subtract")   subtract,
  meta("multiply")   multiply,
  meta("divide")     divide,
  meta("remainder")  remainder,
};

enum struct [[meta::stringify]]
Ast_Kind : int
{
  meta("invalid") Invalid,

  // expressions
  meta("int_lit") Int_Lit,
  meta("unary")   Unary,
  meta("binary")  Binary,

  // statements
  meta("block")   Block,
  meta("return")  Return,
  meta("fn_def")  Fn_Def,
};

// distinct integer types for arena indices, created for stronger type
// safety
struct node_idx
{
  u32 idx;
  explicit node_idx(u32 idx) : idx(idx) {}
  explicit node_idx() : idx(0) {}

  bool is_null() { return idx == 0; };
};

struct Ast_Node
{
  Ast_Kind kind;
  union
  {
    struct
    {
      Token tok;
      int val;
    } int_lit;
    struct
    {
      node_idx rhs;
      Unary_Op op;
    } unary;
    struct
    {
      node_idx lhs;
      node_idx rhs;
      Binary_Op op;
    } binary;
    struct
    {
      u32 stmts_start;
      u32 stmts_count;
    } block;
    struct
    {
      node_idx rhs;
    } ret;
    struct
    {
      Token name_ident;
      node_idx body;
    } fn_def;
  };
};

constexpr bool is_expr(Ast_Kind kind)
{
  using enum Ast_Kind;
  return (kind > Invalid) && (kind < Block);
};

// seems redundant, but this is to help with being explicit
constexpr bool is_stmt(Ast_Kind kind)
{
  return !is_expr(kind);
};

constexpr bool is_invalid(Ast_Node *node)
{
  return node->kind == Ast_Kind::Invalid;
};

void init_int_lit(Ast_Node *node, Token tok, String8_View source);
void init_block(Ast_Node *node, u32 stmts_start, u32 stmts_count);
void init_ret(Ast_Node *node, node_idx rhs);
void init_fn_def(Ast_Node *node, Token name_ident, node_idx body);
void init_unary(Ast_Node *node, node_idx rhs, Unary_Op op);
void init_binary(Ast_Node *node, node_idx lhs, node_idx rhs, Binary_Op op);


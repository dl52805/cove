#include "ast.hpp"

void init_int_lit(Ast_Node *node, Token tok, String8_View source)
{
  node->kind = Ast_Kind::Int_Lit;
  char num[21] = {0};
  strncpy(num, (char *) &source.buffer[tok.position], tok.length);
  int value = atoi(num);

  node->int_lit.tok = tok;
  node->int_lit.val = value;
}

void init_block(Ast_Node *node, u32 stmts_start, u32 stmts_count)
{
  node->kind = Ast_Kind::Block;
  node->block.stmts_start = stmts_start;
  node->block.stmts_count = stmts_count;
}

void init_ret(Ast_Node *node, node_idx rhs)
{
  node->kind = Ast_Kind::Return;
  node->ret.rhs = rhs;
}

void init_fn_def(Ast_Node *node, Token name_ident, node_idx body)
{
  node->kind = Ast_Kind::Fn_Def;
  node->fn_def.name_ident = name_ident;
  node->fn_def.body = body;
}

void init_unary(Ast_Node *node, node_idx rhs, Unary_Op op)
{
  node->kind = Ast_Kind::Unary;
  node->unary.rhs = rhs;
  node->unary.op = op;
}

void init_binary(Ast_Node *node, node_idx lhs, node_idx rhs, Binary_Op op)
{
  node->kind = Ast_Kind::Binary;
  node->binary.rhs = rhs;
  node->binary.lhs = lhs;
  node->binary.op = op;
}


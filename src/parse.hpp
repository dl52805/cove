#pragma once

#include "array.hpp"
#include "string.hpp"
#include "typed_arena.hpp"

#include "ast.hpp"
#include "lex.hpp"

struct tok_idx
{
  u32 idx;
  explicit tok_idx(u32 idx) : idx(idx) {}
  explicit tok_idx() : idx(0) {}
};

struct Program
{
  String8_View source;

  Typed_Arena<node_idx> *ast_stream;
  Typed_Arena<Ast_Node> *ast_arena;
  Typed_Arena<node_idx> *child_arena;

  Program() {}

  Program(Typed_Arena<node_idx> *ast_stream, Typed_Arena<Ast_Node> *ast_arena,
          Typed_Arena<node_idx> *child_arena, String8_View source)
  {
    this->source = source;
    this->ast_stream = ast_stream;
    this->ast_arena = ast_arena;
    this->child_arena = child_arena;
  }
};

struct Parser
{
  using enum Token_Type;

  String8_View source;
  Array<Token> tok_stream;

  tok_idx curr_pos;
  tok_idx peek_pos;

  Typed_Arena<node_idx> *ast_stream;
  Typed_Arena<Ast_Node> *ast_arena;
  Typed_Arena<node_idx> *child_arena;

  Parser(String8_View sv, Array<Token> tokens,
         Typed_Arena<node_idx> *ast_stream, Typed_Arena<Ast_Node> *ast_arena,
         Typed_Arena<node_idx> *child_arena)
  {
    source = sv;
    tok_stream = tokens;
    this->ast_stream = ast_stream;
    this->ast_arena = ast_arena;
    this->child_arena = child_arena;
    curr_pos = tok_idx(0);
    peek_pos = tok_idx(1);

    // verify that the arenas have been populated with the error
    // obj at index 0
    assert(this->ast_stream->length() == 1);
    assert(this->ast_arena->length() == 1);
    assert(this->child_arena->length() == 1);
  }

  Program parse_program();

  Token curr_token();
  Token peek_token();
  void next_token();
  bool expect(Token_Type type);
  bool expect_peek(Token_Type type);
  void error(const char *err_msg);
  void match_or_err(Token tok, Token_Type type, const char *err_msg);
  void expect_or_err(Token_Type type, const char *err_msg);
  void expect_peek_or_err(Token_Type type, const char *err_msg);

  node_idx parse_surface_node();
  node_idx parse_stmt();
  node_idx parse_expr();

  node_idx parse_int_lit();
  node_idx parse_block_stmt();
  node_idx parse_ret_stmt();
  node_idx parse_fn_decl();

  void print_indent(int depth);
  void print_node(node_idx node, int depth = 0);
};

enum struct Precedence
{
  lowest      = 1,
  equals      = 2, // ==
  lessgreater = 3, // > or <
  sum         = 4, // +
  product     = 5, // *
  prefix      = 6, // -x or !x
  call        = 7, // func(x)
};


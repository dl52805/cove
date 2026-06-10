#pragma once

#include "allocator.hpp"
#include "array.hpp"
#include "lex.hpp"

#define meta(...)

enum struct [[meta::stringify]]
Ast_Surface_Kind : int
{
  meta("ast_func_def") func_def,
};

enum struct [[meta::stringify]]
Ast_Stmt_Kind : int
{
  meta("ast_return")     return_stmt,
  meta("ast_block_stmt") block_stmt,
};

enum struct [[meta::stringify]]
Ast_Expr_Kind : int
{
  meta("ast_int_literal") int_literal,
  meta("ast_unary_expr")  unary_expr,
};

struct Ast_Node {};

struct Ast_Surface : Ast_Node
{
  Ast_Surface_Kind kind;

  Ast_Surface() {}
  Ast_Surface(Ast_Surface_Kind kind) : kind(kind) {}

  template<typename T>
  bool is_surface()
  {
    if constexpr (__is_base_of(Ast_Surface, T))
    {
      return kind == T::node_kind;
    } else return false;
  }

  template<typename T>
  const T *to_surface()
  {
    if constexpr (__is_base_of(Ast_Surface, T))
    {
      return is_surface<T>() ? static_cast<const T *>(this) : nullptr;
    } else return nullptr;
  }
};

struct Ast_Stmt : Ast_Node
{
  Ast_Stmt_Kind kind;

  Ast_Stmt() {}
  Ast_Stmt(Ast_Stmt_Kind kind) : kind(kind) {}

  template<typename T>
  bool is_stmt()
  {
    if constexpr (__is_base_of(Ast_Stmt, T))
    {
      return kind == T::node_kind;
    } else return false;
  }

  template<typename T>
  const T *to_stmt()
  {
    if constexpr (__is_base_of(Ast_Stmt, T))
    {
      return is_stmt<T>() ? static_cast<const T *>(this) : nullptr;
    } else return nullptr;
  }
};

struct Ast_Expr : Ast_Node
{
  Ast_Expr_Kind kind;

  Ast_Expr() {}
  Ast_Expr(Ast_Expr_Kind kind) : kind(kind) {}

  template<typename T>
  bool is_expr()
  {
    if constexpr (__is_base_of(Ast_Expr, T))
    {
      return kind == T::node_kind;
    } else return false;
  }

  template<typename T>
  const T *to_expr()
  {
    if constexpr (__is_base_of(Ast_Expr, T))
    {
      return is_expr<T>() ? static_cast<const T *>(this) : nullptr;
    } else return nullptr;
  }
};

struct Ast_Return : Ast_Stmt
{
  Ast_Expr *rhs;

  using enum Ast_Stmt_Kind;
  constexpr static Ast_Stmt_Kind node_kind = return_stmt;

  static Ast_Return *init(Ast_Expr *rhs, Allocator *allocator)
  {
    void *ast_obj = allocator->allocate(sizeof(Ast_Return)).unwrap();
    Ast_Return *return_stmt = (Ast_Return *) ast_obj;
    return_stmt->rhs = rhs;
    return_stmt->kind = node_kind;
    return return_stmt;
  }
};

struct Ast_Block_Stmt : Ast_Stmt
{
  Array<Ast_Stmt *> stmts;

  using enum Ast_Stmt_Kind;
  constexpr static Ast_Stmt_Kind node_kind = block_stmt;

  static Ast_Block_Stmt *init(Array<Ast_Stmt *> stmts, Allocator *allocator)
  {
    void *ast_obj = allocator->allocate(sizeof(Ast_Block_Stmt)).unwrap();
    Ast_Block_Stmt *block_stmt = (Ast_Block_Stmt *) ast_obj;
    block_stmt->stmts = stmts;
    block_stmt->kind = node_kind;
    return block_stmt;
  }
};

struct Ast_Int_Literal : Ast_Expr
{
  Token token;
  int value;

  using enum Ast_Expr_Kind;
  constexpr static Ast_Expr_Kind node_kind = int_literal;

  static Ast_Int_Literal *init(Token token, String8_View source,
                               Allocator *allocator)
  {
    void *ast_obj = allocator->allocate(sizeof(Ast_Int_Literal)).unwrap();
    Ast_Int_Literal *int_literal = (Ast_Int_Literal *) ast_obj;

    // any 64-bit number can have at most 20 digits in decimal
    char num[21] = {0};
    strncpy(num, (char *) &source.buffer[token.position], token.length);
    int value = atoi(num);

    int_literal->token = token;
    int_literal->value = value;
    int_literal->kind = node_kind;
    return int_literal;
  }
};

struct Ast_Function_Def : Ast_Surface
{
  Token ident;
  Ast_Block_Stmt *body;

  using enum Ast_Surface_Kind;
  constexpr static Ast_Surface_Kind node_kind = func_def;

  static Ast_Function_Def *init(Token ident, Ast_Block_Stmt *body,
                                Allocator *allocator)
  {
    void *ast_obj = allocator->allocate(sizeof(Ast_Function_Def)).unwrap();
    Ast_Function_Def *func_def = (Ast_Function_Def *) ast_obj;
    func_def->ident = ident;
    func_def->body = body;
    return func_def;
  }
};


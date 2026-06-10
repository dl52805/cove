#pragma once

#include "ast_meta.hpp"

#include "def.hpp"
#include "string.hpp"
#include "ast.hpp"

void print_token(Token token, String8_View source);

struct Program
{
  Array<Ast_Surface *> top_level_decls;

  Program(Array<Ast_Surface *> top_level_decls)
  {
    this->top_level_decls = top_level_decls;
  }
};

struct Parser
{
  String8_View source;
  Array<Token> token_stream;

  u32 curr_position;
  u32 peek_position;

  Allocator *alloc;

  enum struct Precedence
  {
    lowest      = 1,
    equals      = 2, // =
    lessgreater = 3, // <, >
    sum         = 4, // +, -
    product     = 5, // *, /
    prefix      = 6, // -x
    call        = 7, // f(a)
  };

  using enum Token_Type;

  constexpr Precedence get_prec(Token_Type operator_type)
  {
    switch (operator_type)
    {
      case slash:
        return Precedence::product;
      default:
        return Precedence::lowest;
    }
  }

  Program parse_program()
  {
    Array<Ast_Surface *> top_node_arr(alloc);
    while (curr_token().type != eof)
    {
      Ast_Surface *top_level = parse_surface_node();
      if (top_level != nullptr) top_node_arr.append(top_level);
    }

    for (int i = 0; i < top_node_arr.length; i++)
    {
      print_surface(top_node_arr[i], 0);
    }

    Program program(top_node_arr);
    return program;
  }

  Token curr_token()
  {
    if (curr_position >= token_stream.length)
    {
      Token err;
      err.length = -1;
      return err;
    }
    return token_stream[curr_position];
  }

  Token peek_token()
  {
    if (peek_position >= token_stream.length)
    {
      Token err;
      err.length = -1;
      return err;
    }
    return token_stream[peek_position];
  }

  Parser(String8_View sv, Array<Token> tokens,
         Allocator *allocator = new Libc_Alloc())
  {
    source = sv;
    token_stream = tokens;
    alloc = allocator;
    curr_position = 0;
    peek_position = 1;
  }

  void next_token()
  {
    curr_position = peek_position;
    peek_position += 1;
  }

  bool expect(Token_Type type)
  {
    if (curr_token().type == type)
    {
      next_token();
      return true;
    }
    return false;
  }

  bool expect_peek(Token_Type type)
  {
    if (peek_token().type == type)
    {
      next_token();
      return true;
    }
    return false;
  }

  void error(const char *err_msg)
  {
    fprintf(stderr, "%s%s%s\n", red, err_msg, reset);
    exit(0);
  }

  void match_or_err(Token token, Token_Type type, const char *err_msg)
  {
    if (token.type != type)
    {
      fprintf(stderr, "%s%s%s\n", red, err_msg, reset);
      exit(0);
    }
  }

  void expect_or_err(Token_Type type, const char *err_msg)
  {
    if (!expect(type))
    {
      fprintf(stderr, "%s%s%s\n", red, err_msg, reset);
      exit(0);
    }
  }

  void expect_peek_or_err(Token_Type type, const char *err_msg)
  {
    if (!expect_peek(type))
    {
      fprintf(stderr, "%s%s%s\n", red, err_msg, reset);
      exit(0);
    }
  }

  Ast_Surface *parse_surface_node()
  {
    return parse_func_decl();
  }

  Ast_Surface *parse_func_decl()
  {
    expect_or_err(int_keyword, "Expected return type 'int'");

    Token name_ident = curr_token();
    match_or_err(name_ident, ident, "Function missing name identifier");
    next_token();

    expect_or_err(lparen, "Missing opening parentheses");
    expect_or_err(void_keyword, "Invalid function parameters");
    expect_or_err(rparen, "Missing closing parentheses");

    expect_or_err(lbrace, "Missing opening brace");
    Ast_Block_Stmt *body = parse_block_stmt();
    expect_or_err(rbrace, "Missing closing brace");

    return Ast_Function_Def::init(name_ident, body, alloc);
  }

  Ast_Stmt *parse_stmt()
  {
    Token curr = curr_token();
    Token peek = peek_token();

    switch (curr.type)
    {
      case return_keyword:
        return parse_return_stmt();
      default:
        error("Invalid statement, expected 'return'");
        return nullptr;
    }
  }

  Ast_Block_Stmt *parse_block_stmt()
  {
    Array<Ast_Stmt *> statements(32, alloc);

    while (true)
    {
      Token curr = curr_token();
      Token peek = peek_token();

      if ((curr.type == rbrace) || (curr.type == eof)) break;

      Ast_Stmt *stmt = parse_stmt();
      statements.append(stmt);
    }

    return Ast_Block_Stmt::init(statements, alloc);
  }

  Ast_Stmt *parse_return_stmt()
  {
    next_token();
    Ast_Expr *rhs = parse_expr(Precedence::lowest);
    next_token();
    expect_or_err(semicolon, "Expected semicolon");
    return Ast_Return::init(rhs, alloc);
  }

  Ast_Expr *parse_expr(Precedence prec)
  {
    Token curr = curr_token();

    switch (curr.type)
    {
      case int_literal:
        return Ast_Int_Literal::init(curr, source, alloc);
      default:
        error("Unexpected expression");
        return nullptr;
    }
  }

  void print_indentation(int depth)
  {
    for (int i = 0; i < depth; i++)
    {
      if (i == 0) printf("    ");
      else printf(" %s|%s  ", gray, reset);
    }
  }

  void print_expr(Ast_Expr *expr, int depth)
  {
    using enum Ast_Expr_Kind;

    if (depth > 0)
    {
      print_indentation(depth - 1);
      printf(" %s|__%s", gray, reset);
    }

    if (expr == nullptr)
    {
      printf("[expr] %sempty%s\n", red, reset);
      return;
    }

    printf("[expr]: %s%s%s\n", purple,
           str_from_ast_expr_kind(expr->kind), reset);

    switch (expr->kind)
    {
      case int_literal:
        {
          const Ast_Int_Literal *int_lit = expr->to_expr<Ast_Int_Literal>();
          print_indentation(depth);
          int value = int_lit->value;
          printf("[value]: %s%d%s\n", green, value, reset);
          break;
        }
      default:
        return;
    }
  }

  void print_stmt(Ast_Stmt *stmt, int depth)
  {
    using enum Ast_Stmt_Kind;

    if (depth > 0)
    {
      print_indentation(depth - 1);
      printf(" %s|__%s", gray, reset);
    }

    if (stmt == nullptr)
    {
      printf("[stmt] %sempty%s\n", red, reset);
      return;
    }

    printf("[stmt]: %s%s%s\n", purple,
           str_from_ast_stmt_kind(stmt->kind), reset);

    switch (stmt->kind)
    {
      case return_stmt:
        {
          const Ast_Return *return_stmt = stmt->to_stmt<Ast_Return>();
          print_indentation(depth);
          printf("[rhs]: \n");
          print_expr(return_stmt->rhs, depth + 1);
          break;
        }
      case block_stmt:
        {
          const Ast_Block_Stmt *block = stmt->to_stmt<Ast_Block_Stmt>();
          print_indentation(depth);
          printf("[stmts]: \n");
          for (int i = 0; i < block->stmts.length; i++)
          {
            print_stmt(block->stmts[i], depth + 1);
          }
          break;
        }
      default:
        return;
    }
  }

  void print_surface(Ast_Surface *top_node, int depth)
  {
    using enum Ast_Surface_Kind;

    if (depth > 0)
    {
      print_indentation(depth - 1);
      printf(" %s|__%s", gray, reset);
    }

    if (top_node == nullptr)
    {
      printf("[top level] %sempty%s\n", red, reset);
      return;
    }

    printf("[top level]: %s%s%s\n", purple,
           str_from_ast_surface_kind(top_node->kind), reset);

    switch (top_node->kind)
    {
      case func_def:
        {
          const Ast_Function_Def *func =
            top_node->to_surface<Ast_Function_Def>();
          print_indentation(depth);
          printf("[name]: %.*s\n", func->ident.length,
                 &source.buffer[func->ident.position]);
          print_indentation(depth);
          printf("[body]: \n");
          print_stmt(func->body, depth + 1);
          break;
        }
      default:
        return;
    }
  }
};


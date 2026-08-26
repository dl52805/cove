#include "ast.hpp"
#include "parse.hpp"

#include "ast_meta.hpp"

Program Parser::parse_program()
{
  while (curr_token().type != eof)
  {
    node_idx surface = parse_surface_node();
    if (surface.idx != 0)
    {
      u64 idx = ast_stream->push();
      ast_stream->get(idx)->idx = surface.idx;
    }
  }

  // avoid 0 which is reserved as an error index
  for (int i = 1; i < ast_stream->idx; i++)
  {
    print_node(*ast_stream->get(1));
  }

  Program program(ast_stream, ast_arena, child_arena, source);
  return program;
}

Token Parser::curr_token()
{
  if (curr_pos.idx >= tok_stream->length())
  {
    Token err;
    err.length = -1;
    return err;
  }
  return *tok_stream->get(curr_pos.idx);
}

Token Parser::peek_token()
{
  if (peek_pos.idx >= tok_stream->length())
  {
    Token err;
    err.length = -1;
    return err;
  }
  return *tok_stream->get(peek_pos.idx);
}

void Parser::next_token()
{
  curr_pos.idx = peek_pos.idx;
  peek_pos.idx += 1;
}

bool Parser::expect(Token_Type type)
{
  if (curr_token().type == type)
  {
    next_token();
    return true;
  }
  return false;
}

bool Parser::expect_peek(Token_Type type)
{
  if (peek_token().type == type)
  {
    next_token();
    return true;
  }
  return false;
}

void Parser::error(const char *err_msg)
{
  fprintf(stderr, "%s%s%s\n", red, err_msg, red);
  exit(0);
}

void Parser::match_or_err(Token tok, Token_Type type, const char *err_msg)
{
  if (tok.type != type)
  {
    fprintf(stderr, "%s%s%s\n", red, err_msg, red);
    exit(0);
  }
}

void Parser::expect_or_err(Token_Type type, const char *err_msg)
{
  if (!expect(type))
  {
    fprintf(stderr, "%s%s%s\n", red, err_msg, red);
    exit(0);
  }
}

void Parser::expect_peek_or_err(Token_Type type, const char *err_msg)
{
  if (!expect_peek(type))
  {
    fprintf(stderr, "%s%s%s\n", red, err_msg, red);
    exit(0);
  }
}

void Parser::consume_optional(Token_Type type)
{
  Token curr = curr_token();
  if (curr.type == type) next_token();
}

node_idx Parser::parse_surface_node()
{
  return parse_fn_decl();
}

node_idx Parser::parse_stmt()
{
  Token curr = curr_token();
  Token peek = peek_token();

  switch(curr.type)
  {
    case return_keyword:
      return parse_ret_stmt();
    default:
      error("Invalid statement, expected \"return\"");
      return node_idx(0);
  }
}

node_idx Parser::parse_expr()
{
  using enum Token_Type;

  Token curr = curr_token();
  switch (curr.type)
  {
    case int_literal:
      {
        node_idx int_lit = node_idx(ast_arena->push());
        init_int_lit(ast_arena->get(int_lit.idx), curr, source);
        return int_lit;
      }
    case tilde:
    case dash:
    case bang:
      {
        using enum Unary_Op;

        Unary_Op op;
        if (curr.type == tilde) op = complement;
        else if (curr.type == dash) op = negate;
        else if (curr.type == bang) op = cond_not;

        next_token();
        node_idx rhs = parse_expr();
        node_idx unary = node_idx(ast_arena->push());
        Ast_Node *node = ast_arena->get(unary.idx);
        init_unary(node, rhs, op);
        return unary;
      }
    case lparen:
      {
        next_token();
        node_idx expr = parse_expr();
        expect_or_err(rparen, "Expected closing parentheses");
        return expr;
      }
    default:
      error("Unexpected expression");
      return node_idx(0);
  }
}

node_idx Parser::parse_block_stmt()
{
  u32 start_idx = child_arena->idx;
  u32 len = 0;

  while (true)
  {
    Token curr = curr_token();
    Token peek = peek_token();

    if ((curr.type == rbrace) || (curr.type == eof)) break;

    node_idx child = parse_stmt();
    u64 child_idx = child_arena->push();
    child_arena->get(child_idx)->idx = child.idx;
    len += 1;
  }

  node_idx block = node_idx(ast_arena->push());
  init_block(ast_arena->get(block.idx), start_idx, len);
  return block;
}

node_idx Parser::parse_ret_stmt()
{
  next_token();
  node_idx rhs = parse_expr();
  next_token();
  expect_or_err(semicolon, "Expected semicolon");

  node_idx ret_stmt = node_idx(ast_arena->push());
  init_ret(ast_arena->get(ret_stmt.idx), rhs);
  return ret_stmt;
}

node_idx Parser::parse_fn_decl()
{
  expect_or_err(int_keyword, "Expected return type \"int\"");

  Token name_ident = curr_token();
  match_or_err(name_ident, ident, "Function missing name identifier");
  next_token();

  expect_or_err(lparen, "Missing opening parentheses");
  consume_optional(void_keyword);
  expect_or_err(rparen, "Missing closing parentheses");

  expect_or_err(lbrace, "Missing opening brace");
  node_idx body = parse_block_stmt();
  expect_or_err(rbrace, "Missing closing brace");

  node_idx fn_decl = node_idx(ast_arena->push());
  init_fn_def(ast_arena->get(fn_decl.idx), name_ident, body);
  return fn_decl;
}

void Parser::print_indent(int depth)
{
  for (int i = 0; i < depth; i++)
  {
    if (i == 0) printf("    ");
    else printf(" %s|%s  ", gray, reset);
  }
}

void Parser::print_node(node_idx node, int depth)
{
  if (node.idx == 0)
  {
    fprintf(stderr, "%s[ERROR]: printing invalid node index (0)%s\n",
            red, reset);
    exit(0);
  }

  using enum Ast_Kind;

  if (depth > 0)
  {
    print_indent(depth - 1);
    printf(" %s|__%s", gray, reset);
  }

  Ast_Node *node_obj = ast_arena->get(node.idx);

  if (is_invalid(node_obj))
  {
    printf("[node] %sillegal%s\n", red, reset);
    return;
  }
  else if (is_expr(node_obj->kind))
  {
    printf("[expr]: %s%s%s\n", purple,
           str_from_ast_kind(node_obj->kind), reset);

    switch (node_obj->kind)
    {
      case Int_Lit:
        {
          print_indent(depth);
          printf("[value]: %s%d%s\n", green, node_obj->int_lit.val, reset);
          break;
        }
      case Unary:
        {
          print_indent(depth);
          printf("[op]: %s\n", str_from_unary_op(node_obj->unary.op));
          print_indent(depth);
          printf("[expr]: \n");
          print_node(node_obj->unary.rhs, depth + 1);
          break;
        }
      default:
        return;
    }
  }
  else
  {
    printf("[stmt]: %s%s%s\n", purple,
           str_from_ast_kind(node_obj->kind), reset);

    switch (node_obj->kind)
    {
      case Block:
        {
          print_indent(depth);
          printf("[stmts]: \n");
          u32 start_offset = node_obj->block.stmts_start;
          for (int i = 0; i < node_obj->block.stmts_count; i++)
          {
            u32 child_idx = start_offset + i;
            node_idx *ast_idx = child_arena->get(child_idx);
            print_node(*ast_idx, depth + 1);
          }
          break;
        }
      case Return:
        {
          print_indent(depth);
          printf("[rhs]: \n");
          print_node(node_obj->ret.rhs, depth + 1);
          break;
        }
      case Fn_Def:
        {
          print_indent(depth);
          printf("[name]: %.*s\n", node_obj->fn_def.name_ident.length,
             &source.buffer[node_obj->fn_def.name_ident.position]);
          print_indent(depth);
          printf("[body]: \n");
          print_node(node_obj->fn_def.body, depth + 1);
          break;
        }
      default:
        return;
    }
  }
}


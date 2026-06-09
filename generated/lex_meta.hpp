#pragma once

#include "lex.hpp"

enum struct Token_Type : int;
static const char *str_from_token_type(Token_Type e)
{
  switch (e)
  {
    case Token_Type::illegal:
      return "illegal";
    case Token_Type::eof:
      return "eof";
    case Token_Type::int_keyword:
      return "int_keyword";
    case Token_Type::void_keyword:
      return "void_keyword";
    case Token_Type::return_keyword:
      return "return_keyword";
    case Token_Type::int_literal:
      return "int_literal";
    case Token_Type::string_literal:
      return "string_literal";
    case Token_Type::ident:
      return "ident";
    case Token_Type::lparen:
      return "lparen";
    case Token_Type::rparen:
      return "rparen";
    case Token_Type::lbrace:
      return "lbrace";
    case Token_Type::rbrace:
      return "rbrace";
    case Token_Type::semicolon:
      return "semicolon";
    case Token_Type::tilde:
      return "tilde";
    case Token_Type::dash:
      return "dash";
    case Token_Type::dash_dash:
      return "dash_dash";
    case Token_Type::slash:
      return "slash";
  }
  return "";
}

static int token_type_max_field_len()
{
  return 14;
}


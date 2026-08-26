#pragma once

#include "def.hpp"
#include "hash_table.hpp"
#include "string.hpp"

#define meta(...)

enum struct [[meta::stringify]]
Token_Type : int
{
  meta("illegal")        illegal,
  meta("eof")            eof,

  meta("int_keyword")    int_keyword,
  meta("void_keyword")   void_keyword,
  meta("return_keyword") return_keyword,

  meta("int_literal")    int_literal,
  meta("string_literal") string_literal,

  meta("ident")          ident,

  meta("lparen")         lparen,
  meta("rparen")         rparen,
  meta("lbrace")         lbrace,
  meta("rbrace")         rbrace,
  meta("semicolon")      semicolon,
  meta("tilde")          tilde,
  meta("bang")           bang,
  meta("dash_dash")      dash_dash,

  meta("star")           star,
  meta("slash")          slash,
  meta("dash")           dash,
  meta("plus")           plus,
  meta("percent")        percent,
};

struct Token
{
  Token_Type type;
  u32 position;
  u32 length;

  void init_token(Token *tok, Token_Type type, u32 position, u32 length);
};

struct Lexer
{
  Hash_Table<String8_View, Token_Type> ht;
  String8_View source;
  u32 line;
  u32 current;
  u32 start;

  using enum Token_Type;

  Lexer(String8_View source, Allocator *alloc)
  {
    this->source = source;
    this->line = 1;
    this->current = 0;
    this->start = 0;
    this->ht = Hash_Table<String8_View, Token_Type>(
      {
        { String8_View("int"),    int_keyword    },
        { String8_View("void"),   void_keyword   },
        { String8_View("return"), return_keyword },
      },
      alloc
    );
  }

  bool is_at_end();
  char peek();
  char peek_next();
  char advance();
  void skip_whitespace();
  Token scan_integer();
  Token_Type scan_identifier();
  Token scan_string();
  Token scan_token();
};


#pragma once

#define meta(...)

#include "def.hpp"
#include "string.hpp"
#include "hash_table.hpp"

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
  meta("dash")           dash,
  meta("dash_dash")      dash_dash,
  meta("slash")          slash,
};

struct Token
{
  Token_Type type;
  u32 position;
  u32 length;

  Token() {}

  Token(Token_Type type, u32 position, u32 length)
    : type(type), position(position), length(length) {}
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

  bool is_at_end()
  {
    return current >= source.length;
  }

  char peek()
  {
    if (is_at_end()) return '\0';
    return source.buffer[current];
  }

  char peek_next()
  {
    char curr_char = source.buffer[current];
    current += 1;
    return curr_char;
  }

  char advance()
  {
    char curr_char = source.buffer[current];
    current += 1;
    return curr_char;
  }

  void skip_whitespace()
  {
    while (true)
    {
      if (is_at_end()) break;

      char c = peek();
      switch(c)
      {
        case ' ':
        case '\r':
        case '\t':
          advance();
          break;
        case '\n':
          line += 1;
          advance();
          break;
        default:
          return;
      }
    }
  }

  Token scan_integer()
  {
    while (isdigit(peek())) advance();
    return Token(int_literal, start, current - start);
  }

  Token_Type scan_identifier()
  {
    while (isalnum(peek()) || (peek() == '_')) advance();

    u32 ident_length = current - start;
    String8_View ident_name((char *) &source.buffer[start], ident_length);

    Token_Type *keyword_type = ht.find(ident_name);
    if (keyword_type != nullptr) return *keyword_type;
    return ident;
  }

  Token scan_string()
  {
    while (peek() != '"') advance();
    advance();
    return Token(string_literal, start + 1, current - start - 2);
  }

  Token scan_token()
  {
    Token_Type type = illegal;

    skip_whitespace();
    start = current;
    if (is_at_end()) return Token(eof, start, current - start);

    char c = advance();
    switch (c)
    {
      case '/':
        {
          if (peek() == '/')
          {
            while (peek() != '\n') advance();
            skip_whitespace();
            start = current;
            if (is_at_end()) return Token(eof, start, current - start);
          } else type = slash;
          break;
        }
      case '(':
        type = lparen;
        break;
      case ')':
        type = rparen;
        break;
      case '{':
        type = lbrace;
        break;
      case '}':
        type = rbrace;
        break;
      case ';':
        type = semicolon;
        break;
      case '-':
        {
          if (peek() == '-')
          {
            advance();
            type = dash_dash;
          } else type = dash;
          break;
        }
      case '~':
        type = tilde;
        break;
    }

    if (isdigit(c)) return scan_integer();
    if (isalnum(c) || (c == '_')) type = scan_identifier();

    return Token(type, start, current - start);
  }
};


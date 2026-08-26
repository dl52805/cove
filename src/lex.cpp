#include "lex.hpp"

void Token::init_token(Token *tok, Token_Type type, u32 position, u32 length)
{
  tok->type = type;
  tok->position = position;
  tok->length = length;
}

bool Lexer::is_at_end()
{
  return current >= source.length;
}

char Lexer::peek()
{
  if (is_at_end()) return '\0';
  return source.buffer[current];
}

char Lexer::peek_next()
{
  if (current + 1 <= source.length) return '\0';
  return source.buffer[current + 1];
}

char Lexer::advance()
{
  char curr_char = source.buffer[current];
  current += 1;
  return curr_char;
}

void Lexer::skip_whitespace()
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

Token Lexer::scan_integer()
{
  while (isdigit(peek())) advance();
  return Token(int_literal, start, current - start);
}

Token_Type Lexer::scan_identifier()
{
  while (isalnum(peek()) || (peek() == '_')) advance();

  u32 ident_length = current - start;
  String8_View ident_name((char *) &source.buffer[start], ident_length);

  Token_Type *keyword_type = ht.find(ident_name);
  if (keyword_type != nullptr) return *keyword_type;
  return ident;
}

Token Lexer::scan_string()
{
  while (peek() != '"') advance();
  advance();
  return Token(string_literal, start + 1, current - start - 2);
}

Token Lexer::scan_token()
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
    case '!':
      type = bang;
      break;
    case '*':
      type = star;
      break;
    case '+':
      type = plus;
      break;
    case '%':
      type = percent;
      break;
  }

  if (isdigit(c)) return scan_integer();
  if (isalnum(c) || (c == '_')) type = scan_identifier();

  return Token(type, start, current - start);
}



#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "lex_meta.hpp"

#include "lex.hpp"
#include "arena.hpp"
#include "string.hpp"

enum struct Stage
{
  complete,
  lex,
  parse,
  codegen,
};

void print_token(Token token, String8_View source)
{
  switch (token.type)
  {
    default:
      printf("| .%-*s",
             token_type_max_field_len() + 5,
             str_from_token_type(token.type));
      printf("%.*s\n", token.length, &source.buffer[token.position]);
      break;
  }
}

Stage compiler_flag;

void compile(Allocator *alloc, String8_View source, String8_View file_name)
{
  Lexer lex(source, alloc);
  while (true)
  {
    Token token = lex.scan_token();
    print_token(token, source);
    if (token.type == Token_Type::eof) break;
  }

  if (compiler_flag == Stage::lex) exit(0);
}

int main(int argc, char *argv[])
{
  Arena arena(Arena::linked);

  String8 arg;
  if (argc <= 1)
  {
    fprintf(stderr, "%sMissing file name%s\n", red, reset);
    return 1;
  }
  else if (argc == 2)
  {
    arg = String8(argv[1], &arena);
    if (!arg.ends_in(".c"))
    {
      fprintf(stderr, "%sFile must have extension '.c'%s\n", red, reset);
      return 1;
    }
  }
  else
  {
    for (int i = 1; i < argc; i++)
    {
      if (strcmp(argv[i], "--lex") == 0)
      {
        if (compiler_flag != Stage::complete)
        {
          fprintf(stderr, "%sMore than one stage flag%s\n", red, reset);
          return 1;
        }
        compiler_flag = Stage::lex;
      }
      else if (strcmp(argv[i], "--parse") == 0)
      {
        if (compiler_flag != Stage::complete)
        {
          fprintf(stderr, "%sMore than one stage flag%s\n", red, reset);
          return 1;
        }
        compiler_flag = Stage::parse;
      }
      else if (strcmp(argv[i], "--codegen") == 0)
      {
        if (compiler_flag != Stage::complete)
        {
          fprintf(stderr, "%sMore than one stage flag%s\n", red, reset);
          return 1;
        }
        compiler_flag = Stage::codegen;
      }
      else if (strcmp(argv[i], "-S") == 0)
      {
        if (compiler_flag != Stage::complete)
        {
          fprintf(stderr, "%sMore than one stage flag%s\n", red, reset);
          return 1;
        }
        // TODO(dl): flag not yet handled
      }
      else
      {
        arg = String8(argv[i], &arena);
        if (!arg.ends_in(".c"))
        {
          fprintf(stderr, "%sUnknown compiler flag: %s%s\n",
                  red, arg.c_str(), reset);
          return 1;
        }
      }
    }
  }

  String8 file_name(arg.c_str(), &arena);
  file_name.cut(2);

  String8 preprocessed_name(file_name.c_str(), &arena);
  preprocessed_name.cat(".i");

  int pid = fork();
  if (pid == 0)
  {
    char *clang_args[] = {
      (char *) "clang",
      (char *) "-E",
      (char *) "-P",
      arg.c_str(),
      (char *) "-o",
      preprocessed_name.c_str(),
      nullptr,
    };
    execvp("clang", clang_args);

    fprintf(stderr, "(internal) preprocessor: execvp failed\n");
    _exit(1);
  }
  waitpid(pid, nullptr, 0);

  String8 preprocessed;
  String8::read_from_file(&preprocessed, preprocessed_name.c_str(), &arena);
  remove(preprocessed_name.c_str());

  compile(&arena, String8_View(preprocessed), String8_View(file_name));

  String8 assembly_name(file_name.c_str(), &arena);
  assembly_name.cat(".s");

  pid = fork();
  if (pid == 0)
  {
    char *clang_args[] = {
      (char *) "clang",
      assembly_name.c_str(),
      (char *) "-o",
      arg.c_str(),
      nullptr,
    };
    execvp("clang", clang_args);

    fprintf(stderr, "(internal) assembler: execvp failed\n");
    _exit(1);
  }
  waitpid(pid, nullptr, 0);
  remove(assembly_name.c_str());

  arena.deinit();
}


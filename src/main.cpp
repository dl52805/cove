#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "lex_meta.hpp"

#include "lex.hpp"
#include "array.hpp"
#include "arena.hpp"
#include "string.hpp"
#include "parse.hpp"
#include "ir.hpp"
#include "hir.hpp"

enum struct Stage
{
  complete,
  lex,
  parse,
  hir,
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
  Array<Token> tokens(alloc);

  Lexer lex(source, alloc);
  while (true)
  {
    Token token = lex.scan_token();
    tokens.append(token);
    print_token(token, source);
    if (token.type == Token_Type::eof) break;
  }

  if (compiler_flag == Stage::lex) exit(0);

  Typed_Arena<node_idx> *ast_stream = Typed_Arena<node_idx>::create();
  Typed_Arena<Ast_Node> *ast_arena = Typed_Arena<Ast_Node>::create();
  Typed_Arena<node_idx> *child_arena = Typed_Arena<node_idx>::create();

  ast_stream->push();
  ast_arena->push();
  child_arena->push();

  Parser parser(source, tokens, ast_stream, ast_arena, child_arena);
  Program program = parser.parse_program();

  if (compiler_flag == Stage::parse) exit(0);

  String8 assembly_name((char *) file_name.buffer, file_name.length, alloc);
  assembly_name.cat(".s");

  printf("\n=======[hir]=======\n");

  Typed_Arena<HIR_Surface> *surface_stream =
    Typed_Arena<HIR_Surface>::create();
  Typed_Arena<HIR_Instr> *hir_instr_arena = Typed_Arena<HIR_Instr>::create();

  surface_stream->push();
  hir_instr_arena->push();

  HIR_Program hir_program(program, surface_stream, hir_instr_arena, alloc);
  hir_program.translate_ast();

  if (compiler_flag == Stage::hir) exit(0);

  printf("\n=======[assembly]=======\n");

  Typed_Arena<IR_Instr> *ir_stream = Typed_Arena<IR_Instr>::create();
  ir_stream->push();

  IR_Program ir_program(hir_program, ir_stream, alloc);
  ir_program.lower_ir();
  ir_program.fix_instructions();
  ir_program.emit_assembly(assembly_name, true);

  if (compiler_flag == Stage::codegen) exit(0);
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
      else if (strcmp(argv[i], "--hir") == 0)
      {
        if (compiler_flag != Stage::complete)
        {
          fprintf(stderr, "%sMore than one stage flag%s\n", red, reset);
          return 1;
        }
        compiler_flag = Stage::hir;
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

  compile(&arena, String8_View(preprocessed.c_str()),
          String8_View(file_name.c_str()));

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


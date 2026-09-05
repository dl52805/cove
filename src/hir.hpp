#pragma once

#include "ast.hpp"
#include "parse.hpp"
#include "typed_arena.hpp"
#include "string.hpp"

#define meta(...)

enum struct [[meta::stringify]]
HIR_Surface_Kind : int
{
  meta("illegal") unused,
  meta("fn_def")  fn_def,
};

struct HIR_Surface
{
  String8 ident;
  u32 instr_offset;
  u32 instr_count;
};

enum struct [[meta::stringify]]
HIR_Instr_Kind : int
{
  meta("illegal") unused,
  meta("return")  ret,
  meta("unary")   unary,
  meta("binary")  binary,
};

enum struct [[meta::stringify]]
Operand_Kind
{
  meta("unused")   unused,
  meta("const")    constant,
  meta("register") register_id,
};

struct Operand
{
  using enum Operand_Kind;
  Operand_Kind kind;
  union
  {
    i64 const_val;
    u32 reg_id;
  };
};

Operand init_const(i64 const_val);
Operand init_reg(u32 reg_id);

struct HIR_Instr
{
  HIR_Instr_Kind kind;
  union
  {
    struct
    {
      Operand ret_val;
    } ret;
    struct
    {
      Unary_Op unary_type;
      Operand src;
      Operand dest;
    } unary;
    struct
    {
      Binary_Op binary_type;
      Operand src1;
      Operand src2;
      Operand dest;
    } binary;
  };
};

struct HIR_Program
{
  Typed_Arena<HIR_Surface> *surface_stream;
  Typed_Arena<HIR_Instr> *instr_arena;

  Program program;
  Allocator *alloc;

  u64 temporary_var_counter = 0;

  HIR_Program() {}

  HIR_Program(Program program, Typed_Arena<HIR_Surface> *surface_stream,
              Typed_Arena<HIR_Instr> *instr_arena, Allocator *alloc)
  {
    this->surface_stream = surface_stream;
    this->instr_arena = instr_arena;
    this->program = program;
    this->alloc = alloc;
  }

  void translate_ast();
  void translate_surface(Ast_Node *node);
  Operand emit_expr_hir(Ast_Node *expr);
  u64 make_temporary();

  void print_hir(u32 hir_idx);
  void print_op(Operand op);
};

void init_hir_surface(HIR_Surface *surface, String8 ident,
                      u32 instr_offset, u32 instr_count);
void init_hir_unary(HIR_Instr *instr, Unary_Op type,
                    Operand src, Operand dest);
void init_hir_binary(HIR_Instr *instr, Binary_Op type,
                     Operand src1, Operand src2, Operand dest);
void init_hir_ret(HIR_Instr *instr, Operand ret_val);



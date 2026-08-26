#pragma once

#define meta(...)

#include "string.hpp"
#include "typed_arena.hpp"

#include "hir.hpp"

enum struct [[meta::stringify]]
Instr_Kind
{
  meta("unused")      unused,
  meta("mov")         mov,
  meta("return")      ret,
  meta("fn_preamble") fn_pre,
  meta("label")       label,
  meta("salloc")      salloc,
  meta("neg")         neg,
  meta("not")         b_not,
};

enum struct
IR_Op_Kind : int
{
  immediate,
  reg_name,
  pseudo,
  stack_mem,
};

enum struct
Reg : int
{
  eax,
  r10d,
};

struct IR_Op
{
  using enum IR_Op_Kind;
  IR_Op_Kind kind;
  union
  {
    i64 const_val;
    Reg reg;
    u64 pseudo_reg;
    i64 stack;
  };
};

IR_Op init_immediate(i64 const_val);
IR_Op init_reg(Reg reg);
IR_Op init_pseudo(u64 psuedo_reg);
IR_Op init_stack_op(i64 stack_amt);

struct IR_Instr
{
  using enum Instr_Kind;
  Instr_Kind kind;
  union
  {
    struct
    {
      IR_Op result;
      IR_Op operands[3];
    };
    String8 name;
    i64 amt;
  };
};

void init_mov(IR_Instr *instr, IR_Op src, IR_Op dest);
void init_named(IR_Instr *instr, Instr_Kind kind, String8 str);
void init_stack_alloc(IR_Instr *instr, int stack_amt);
void init_unary_neg(IR_Instr *instr, IR_Op op);
void init_unary_not(IR_Instr *instr, IR_Op op);

struct IR_Program
{
  HIR_Program program;
  Typed_Arena<IR_Instr> *ir_stream;
  Allocator *alloc;

  IR_Program(HIR_Program program, Typed_Arena<IR_Instr> *ir_stream,
             Allocator *alloc)
  {
    this->program = program;
    this->ir_stream = ir_stream;
    this->alloc = alloc;
  }

  void lower_ir();
  void translate_hir_surface(HIR_Surface *surface);
  void fix_instructions();
  void emit_assembly(String8 file_name, bool debug_print);
};

void print_op(FILE *fp, IR_Op op);



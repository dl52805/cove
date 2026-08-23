#pragma once

#define meta(...)

#include "def.hpp"
#include "string.hpp"
#include "typed_arena.hpp"

#include "ast.hpp"
#include "parse.hpp"

enum struct [[meta::stringify]]
Operand_Kind
{
  meta("unused")   unused,
  meta("const")    constant,
  meta("register") register_id,
};

enum struct [[meta::stringify]]
Instr_Kind
{
  meta("unused") unused,
  meta("mov")    mov,
  meta("return") ret,
  meta("label")  label,
  meta("global") global,
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

void init_const(Operand *op, i64 const_val);
void init_reg(Operand *op, u32 reg_id);

struct IR_Instr
{
  using enum Instr_Kind;
  Instr_Kind kind;
  union
  {
    struct
    {
      Operand result;
      Operand operands[3];
    };
    String8 name;
  };
};

void init_mov(IR_Instr *instr, Operand src, Operand dest);
void init_ret(IR_Instr *instr, Operand op);
void init_named(IR_Instr *instr, Instr_Kind kind, String8 str);

struct IR_Program
{
  Program program;
  Typed_Arena<IR_Instr> *ir_stream;
  Allocator *alloc;

  IR_Program(Program program, Typed_Arena<IR_Instr> *ir_stream,
             Allocator *alloc)
  {
    this->program = program;
    this->ir_stream = ir_stream;
    this->alloc = alloc;
  }

  void lower_ir();
  void translate_fn_def(Ast_Node *node);
  void emit_assembly(String8 file_name, bool debug_print);
};



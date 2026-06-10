#pragma once

#include "def.hpp"
#include "array.hpp"
#include "string.hpp"

#define meta(...)

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
  meta("mov") mov,
  meta("ret") ret,
};

struct Operand
{
  using enum Operand_Kind;
  Operand_Kind kind;
  union
  {
    i64 constant;
    u32 reg_id;
  };
};

struct IR_Instruction
{
  using enum Instr_Kind;
  Instr_Kind kind;
  Operand result = {};
  Operand operands[3] = {};

  static IR_Instruction *init_mov(Operand src, Operand dest, Allocator *alloc)
  {
    void *obj = alloc->allocate(sizeof(IR_Instruction)).unwrap();
    IR_Instruction *ir = (IR_Instruction *) obj;
    ir->operands[0] = src;
    ir->result = dest;
    ir->kind = mov;
    return ir;
  }

  static IR_Instruction *init_ret(Allocator *alloc)
  {
    void *obj = alloc->allocate(sizeof(IR_Instruction)).unwrap();
    IR_Instruction *ir = (IR_Instruction *) obj;
    ir->kind = ret;
    return ir;
  }
};

struct IR_Func_Def
{
  String8_View name;
  Array<IR_Instruction> instructions;

  static IR_Func_Def *init(Array<IR_Instruction> instructions,
                           String8_View name, Allocator *alloc)
  {
    void *obj = alloc->allocate(sizeof(IR_Func_Def)).unwrap();
    IR_Func_Def *ir = (IR_Func_Def *) obj;
    ir->name = name;
    ir->instructions = instructions;
    return ir;
  }
};


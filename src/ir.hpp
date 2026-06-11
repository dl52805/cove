#pragma once

#include <stdio.h>
#include <string.h>

#include "def.hpp"
#include "arena.hpp"
#include "array.hpp"
#include "ast.hpp"
#include "parse.hpp"
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
  meta("mov")    mov,
  meta("return") ret,
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

  static Operand init_const(i64 const_val)
  {
    Operand op{};
    op.kind = constant;
    op.const_val = const_val;
    return op;
  }

  static Operand init_reg(u32 reg_id)
  {
    Operand op{};
    op.kind = register_id;
    op.reg_id = reg_id;
    return op;
  }
};

struct IR_Instr
{
  using enum Instr_Kind;
  Instr_Kind kind;
  Operand result = {};
  Operand operands[3] = {};

  static IR_Instr init_mov(Operand src, Operand dest)
  {
    IR_Instr ir = {};
    ir.operands[0] = src;
    ir.result = dest;
    ir.kind = mov;
    return ir;
  }

  static IR_Instr init_ret(Operand op)
  {
    IR_Instr ir = {};
    ir.kind = ret;
    ir.operands[0] = op;
    return ir;
  }
};

struct IR_Func_Def
{
  String8 name;
  Array<IR_Instr> instructions;

  static IR_Func_Def init(Array<IR_Instr> instructions, String8 name)
  {
    IR_Func_Def ir = {};
    ir.name = name;
    ir.instructions = instructions;
    return ir;
  }
};

struct IR_Program
{
  IR_Func_Def func_def = {};
  Allocator *alloc;

  IR_Program(Allocator *alloc) : alloc(alloc) {}

  void lower_ir(Program program, String8_View source)
  {
    Array<Ast_Surface *> ast_nodes = program.top_level_decls;
    // currently restrict to only one top level function definition
    assert(ast_nodes.length == 1);
    for (int i = 0; i < ast_nodes.length; i++)
    {
      assert(ast_nodes[i]->is_surface<Ast_Function_Def>());
      const Ast_Function_Def *ast_func_def =
        ast_nodes[i]->to_surface<Ast_Function_Def>();
      func_def = translate_func_def(ast_func_def, source);
    }
  }

  IR_Func_Def translate_func_def(const Ast_Function_Def *ast_func_def,
                                 String8_View source)
  {
    char *ident_ptr = (char *) &source.buffer[ast_func_def->ident.position];
    String8 name(ident_ptr, ast_func_def->ident.length, alloc);

    Array<Ast_Stmt *> stmts = ast_func_def->body->stmts;
    assert(stmts.length == 1);

    assert(stmts[0]->is_stmt<Ast_Return>());
    const Ast_Return *ast_ret = stmts[0]->to_stmt<Ast_Return>();
    const Ast_Int_Literal *ast_const =
      ast_ret->rhs->to_expr<Ast_Int_Literal>();
    i64 constant = ast_const->value;

    Operand op = Operand::init_const(constant);
    IR_Instr ret_instruction = IR_Instr::init_ret(op);

    Array<IR_Instr> instr_arr(32, alloc);
    instr_arr.append(ret_instruction);

    return IR_Func_Def::init(instr_arr, name);
  }

  void emit_assembly(String8 file_name, bool debug_print = false)
  {
    FILE *fp;
    if (debug_print) fp = stdout;
    else fp = fopen(file_name.c_str(), "w");

    fprintf(fp, "    .globl _%s\n", func_def.name.c_str());
    fprintf(fp, "_%s:\n", func_def.name.c_str());
    for (int i = 0; i < func_def.instructions.length; i++)
    {
      IR_Instr curr = func_def.instructions[i];
      assert(curr.kind == Instr_Kind::ret);
      fprintf(fp, "    movl    $%lld, %%eax\n", curr.operands[0].const_val);
      fprintf(fp, "    ret\n");
    }

    fclose(fp);
  }
};


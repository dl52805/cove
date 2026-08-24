#include "ir.hpp"
#include "hir.hpp"

void init_mov(IR_Instr *instr, Operand src, Operand dest)
{
  using enum Instr_Kind;
  instr->kind = mov;
  instr->operands[0] = src;
  instr->result = dest;
}

void init_ret(IR_Instr *instr, Operand op)
{
  using enum Instr_Kind;
  instr->kind = ret;
  instr->operands[0] = op;
}

void init_named(IR_Instr *instr, Instr_Kind kind, String8 str)
{
  using enum Instr_Kind;
  instr->kind = kind;
  instr->name = str;
}

void IR_Program::lower_ir()
{
  for (int i = 1; i < program.ast_stream->idx; i++)
  {
    node_idx ast_idx = *(program.ast_stream->get(i));
    Ast_Node *fn_def = program.ast_arena->get(ast_idx.idx);
    translate_fn_def(fn_def);
  }
}

void IR_Program::translate_fn_def(Ast_Node *node)
{
  char *ident_ptr = (char *)
    &program.source.buffer[node->fn_def.name_ident.position];
  String8 name(ident_ptr, node->fn_def.name_ident.length, alloc);

  Ast_Node *body = program.ast_arena->get(node->fn_def.body.idx);
  // NOTE(dl): @remove, this is temporary
  assert(body->block.stmts_count == 1);

  node_idx child = *(program.child_arena->get(body->block.stmts_start));
  Ast_Node *ret = program.ast_arena->get(child.idx);
  Ast_Node *int_lit = program.ast_arena->get(ret->ret.rhs.idx);

  u64 global = ir_stream->push();
  u64 label = ir_stream->push();
  IR_Instr *global_instr = ir_stream->get(global);
  IR_Instr *label_instr = ir_stream->get(label);
  init_named(global_instr, Instr_Kind::global, name);
  init_named(label_instr, Instr_Kind::label, name);

  Operand ret_int = init_const(int_lit->int_lit.val);

  u64 ret_ir = ir_stream->push();
  IR_Instr *ret_instr = ir_stream->get(ret_ir);
  init_ret(ret_instr, ret_int);
}

void IR_Program::emit_assembly(String8 file_name, bool debug_print = false)
{
  FILE *fp;
  if (debug_print) fp = stdout;
  else fp = fopen(file_name.c_str(), "w");

  for (int i = 1; i < ir_stream->idx; i++)
  {
    using enum Instr_Kind;
    IR_Instr *instr = ir_stream->get(i);
    switch (instr->kind)
    {
      case mov:
        // TODO(dl): not implemented
        return;
      case ret:
        fprintf(fp, "    movl    $%lld, %%eax\n",
          instr->operands[0].const_val);
        fprintf(fp, "    ret\n");
        break;
      case global:
        fprintf(fp, "    .globl _%s\n", instr->name.c_str());
        break;
      case label:
        fprintf(fp, "_%s:\n", instr->name.c_str());
        break;
      default:
        return;
    }
  }

  fclose(fp);
}



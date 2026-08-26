#include "hir.hpp"

#include "ast_meta.hpp"

Operand init_const(i64 const_val)
{
  Operand op;
  using enum Operand_Kind;
  op.kind = constant;
  op.const_val = const_val;
  return op;
}

Operand init_reg(u32 reg_id)
{
  Operand op;
  using enum Operand_Kind;
  op.kind = register_id;
  op.reg_id = reg_id;
  return op;
}

void HIR_Program::translate_ast()
{
  for (int i = 1; i < program.ast_stream->idx; i++)
  {
    node_idx ast_idx = *(program.ast_stream->get(i));
    Ast_Node *node = program.ast_arena->get(ast_idx.idx);
    translate_surface(node);
  }

  for (int i = 1; i < surface_stream->length(); i++)
  {
    print_hir(i);
  }
}

void HIR_Program::translate_surface(Ast_Node *node)
{
  assert(node->kind == Ast_Kind::Fn_Def);

  char *ident_ptr = (char *)
    &program.source.buffer[node->fn_def.name_ident.position];
  String8 name(ident_ptr, node->fn_def.name_ident.length, alloc);

  Ast_Node *body = program.ast_arena->get(node->fn_def.body.idx);

  node_idx child = *(program.child_arena->get(body->block.stmts_start));
  Ast_Node *ret = program.ast_arena->get(child.idx);

  node_idx expr = ret->ret.rhs;

  u64 start_offset = instr_arena->length();
  Ast_Node *expr_node = program.ast_arena->get(expr.idx);
  Operand result = emit_expr_hir(expr_node);

  i64 ret_instr = instr_arena->push();
  HIR_Instr *ret_node = instr_arena->get(ret_instr);
  init_hir_ret(ret_node, result);

  u64 count = instr_arena->length() - start_offset;

  u64 surface_idx = surface_stream->push();
  HIR_Surface *surface_node = surface_stream->get(surface_idx);
  init_hir_surface(surface_node, name, start_offset, count);
}

Operand HIR_Program::emit_expr_hir(Ast_Node *expr)
{
  using enum Ast_Kind;

  switch (expr->kind)
  {
    case Int_Lit:
      {
        return init_const(expr->int_lit.val);
      }
    case Unary:
      {
        node_idx inner_expr = expr->unary.rhs;
        Ast_Node *inner_expr_node = program.ast_arena->get(inner_expr.idx);
        Operand src = emit_expr_hir(inner_expr_node);
        u64 dest_id = make_temporary();
        Operand dest = init_reg(dest_id);

        i64 unary = instr_arena->push();
        HIR_Instr *unary_node = instr_arena->get(unary);
        init_hir_unary(unary_node, expr->unary.op, src, dest);
        return dest;
      }
    default:
      return {};
  }
}

u64 HIR_Program::make_temporary()
{
  return temporary_var_counter++;
}

void HIR_Program::print_hir(u32 hir_idx)
{
  HIR_Surface *surface = surface_stream->get(hir_idx);
  for (int i = 0; i < surface->instr_count; i++)
  {
    int idx = i + surface->instr_offset;
    HIR_Instr *instr = instr_arena->get(idx);
    using enum HIR_Instr_Kind;
    switch (instr->kind)
    {
      case ret:
      {
        fprintf(stdout, "%s|%s <instr>;[ret]: ", gray, reset);
        print_op(instr->ret.ret_val);
        fprintf(stdout, "\n");

        break;
      }
      case unary:
      {
        fprintf(stdout, "%s-%s <instr>;[unary]: \n", gray, reset);
        fprintf(stdout, "  %s|%s %s\n", gray, reset,
          str_from_unary_op(instr->unary.unary_type));

        fprintf(stdout, "  %s|%s %%src: ", gray, reset);
        print_op(instr->unary.src);
        fprintf(stdout, "\n");

        fprintf(stdout, "  %s|%s %%dest: ", gray, reset);
        print_op(instr->unary.dest);
        fprintf(stdout, "\n");

        break;
      }
      default:
        return;
    }
  }
}

void HIR_Program::print_op(Operand op)
{
  using enum Operand_Kind;
  switch (op.kind)
  {
    case constant:
      {
        fprintf(stdout, "const(%lld)", op.const_val);
        return;
      }
    case register_id:
      {
        fprintf(stdout, "temp_var(%u)", op.reg_id);
        return;
      }
    default:
      return;
  }
}

void init_hir_surface(HIR_Surface *surface, String8 ident,
                      u32 instr_offset, u32 instr_count)
{
  surface->ident = ident;
  surface->instr_offset = instr_offset;
  surface->instr_count = instr_count;
}

void init_hir_unary(HIR_Instr *instr, Unary_Op type,
                    Operand src, Operand dest)
{
  instr->kind = HIR_Instr_Kind::unary;
  instr->unary.unary_type = type;
  instr->unary.src = src;
  instr->unary.dest = dest;
}

void init_hir_ret(HIR_Instr *instr, Operand ret_val)
{
  instr->kind = HIR_Instr_Kind::ret;
  instr->ret.ret_val = ret_val;
}



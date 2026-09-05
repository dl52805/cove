#include "ir.hpp"
#include "ir_meta.hpp"
#include "hir_meta.hpp"

IR_Op init_immediate(i64 const_val)
{
  using enum IR_Op_Kind;
  return IR_Op {
    .kind = immediate,
    .const_val = const_val
  };
}

IR_Op init_reg(Reg reg)
{
  return IR_Op {
    .kind = IR_Op_Kind::reg_name,
    .reg = reg,
  };
}

IR_Op init_pseudo(u64 psuedo_reg)
{
  using enum IR_Op_Kind;
  return IR_Op {
    .kind = pseudo,
    .pseudo_reg = psuedo_reg
  };
}

IR_Op init_stack_op(i64 stack_amt)
{
  using enum IR_Op_Kind;
  return IR_Op {
    .kind = stack_mem,
    .stack = stack_amt,
  };
}

IR_Op hir_to_ir_op(Operand op)
{
  IR_Op ir_op;
  switch (op.kind)
  {
    case Operand_Kind::constant:
      {
        ir_op.kind = IR_Op_Kind::immediate;
        ir_op.const_val = op.const_val;
        break;
      }
    case Operand_Kind::register_id:
      {
        ir_op.kind = IR_Op_Kind::pseudo;
        ir_op.pseudo_reg = op.reg_id;
        break;
      }
    default:
      break;
  }
  return ir_op;
}

void init_mov(IR_Instr *instr, IR_Op src, IR_Op dest)
{
  using enum Instr_Kind;
  instr->kind = mov;
  instr->operands[0] = src;
  instr->result = dest;
}

void init_named(IR_Instr *instr, Instr_Kind kind, String8 str)
{
  using enum Instr_Kind;
  instr->kind = kind;
  instr->name = str;
}

void init_stack_alloc(IR_Instr *instr, int stack_amt)
{
  using enum Instr_Kind;
  instr->kind = salloc;
  instr->amt = stack_amt;
}

void init_unary_neg(IR_Instr *instr, IR_Op op)
{
  using enum Instr_Kind;
  instr->kind = neg;
  instr->operands[0] = op;
}

void init_unary_not(IR_Instr *instr, IR_Op op)
{
  using enum Instr_Kind;
  instr->kind = b_not;
  instr->operands[0] = op;
}

void init_binary(IR_Instr *instr, Binary_Op op, IR_Op src, IR_Op dest)
{
  using enum Binary_Op;
  instr->operands[0] = src;
  instr->result = dest;
  switch (op)
  {
    case multiply:
      instr->kind = Instr_Kind::mul;
      break;
    case add:
      instr->kind = Instr_Kind::add;
      break;
    case subtract:
      instr->kind = Instr_Kind::sub;
      break;
    case divide:
    case remainder:
      instr->kind = Instr_Kind::div;
      break;
    default:
      return;
  }
}

void IR_Program::lower_ir()
{
  for (int i = 1; i < program.surface_stream->length(); i++)
  {
    HIR_Surface *surface = program.surface_stream->get(i);
    translate_hir_surface(surface);
  }
}

void IR_Program::translate_hir_surface(HIR_Surface *surface)
{
  u64 fn_preamble_idx = ir_stream->push();
  IR_Instr *fn_preamble = ir_stream->get(fn_preamble_idx);
  using enum Instr_Kind;
  init_named(fn_preamble, fn_pre, surface->ident);

  u64 alloc_stack_idx = ir_stream->push();
  IR_Instr *alloc_stack = ir_stream->get(alloc_stack_idx);
  init_stack_alloc(alloc_stack, 4 * program.temporary_var_counter);

  for (int i = 0; i < surface->instr_count; i++)
  {
    u64 idx = surface->instr_offset + i;
    HIR_Instr *instr = program.instr_arena->get(idx);

    using enum HIR_Instr_Kind;
    switch (instr->kind)
    {
      case ret:
        {
          IR_Op val = hir_to_ir_op(instr->ret.ret_val);

          u64 mov_idx = ir_stream->push();
          IR_Instr *mov_instr = ir_stream->get(mov_idx);
          init_mov(mov_instr, val, init_reg(Reg::eax));

          u64 ret_idx = ir_stream->push();
          IR_Instr *ret_instr = ir_stream->get(ret_idx);
          ret_instr->kind = Instr_Kind::ret;

          break;
        }
      case unary:
        {
          u64 mov_idx = ir_stream->push();
          IR_Instr *mov_instr = ir_stream->get(mov_idx);
          IR_Op src_op = hir_to_ir_op(instr->unary.src);
          IR_Op dest_op = hir_to_ir_op(instr->unary.dest);
          init_mov(mov_instr, src_op, dest_op);

          u64 unary_idx = ir_stream->push();
          IR_Instr *unary_instr = ir_stream->get(unary_idx);
          switch (instr->unary.unary_type)
          {
            case Unary_Op::negate:
              init_unary_neg(unary_instr, dest_op);
              break;
            case Unary_Op::complement:
              init_unary_not(unary_instr, dest_op);
              break;
            default:
              return;
          }

          break;
        }
      case binary:
        {
          u64 mov_idx = ir_stream->push();
          IR_Instr *mov_instr = ir_stream->get(mov_idx);
          IR_Op src1_op = hir_to_ir_op(instr->binary.src1);
          IR_Op src2_op = hir_to_ir_op(instr->binary.src2);
          IR_Op dest_op = hir_to_ir_op(instr->binary.dest);

          using enum Binary_Op;
          if ((instr->binary.binary_type == divide)
              || (instr->binary.binary_type == remainder))
          {
            init_mov(mov_instr, src1_op, init_reg(Reg::eax));
            u64 cdq_idx = ir_stream->push();
            IR_Instr *cdq_instr = ir_stream->get(cdq_idx);
            cdq_instr->kind = Instr_Kind::cdq;
          }
          else
          {
            init_mov(mov_instr, src1_op, dest_op);
          }

          u64 binary_idx = ir_stream->push();
          IR_Instr *binary_instr = ir_stream->get(binary_idx);
          init_binary(binary_instr, instr->binary.binary_type,
                      src2_op, dest_op);

          if (instr->binary.binary_type == divide)
          {
            u64 mov_idx = ir_stream->push();
            IR_Instr *mov_instr = ir_stream->get(mov_idx);
            init_mov(mov_instr, init_reg(Reg::eax), dest_op);
          }
          else if (instr->binary.binary_type == remainder)
          {
            u64 mov_idx = ir_stream->push();
            IR_Instr *mov_instr = ir_stream->get(mov_idx);
            init_mov(mov_instr, init_reg(Reg::edx), dest_op);
          }

          break;
        }
      default:
        return;
    }
  }
}

void IR_Program::fix_instructions()
{
  for (int i = 1; i < ir_stream->length(); i++)
  {
    IR_Instr *instr = ir_stream->get(i);
    using enum Instr_Kind;
    if ((instr->kind == label) || (instr->kind == fn_pre)) continue;
    for (int i = 0; i < 3; i++)
    {
      if (instr->operands[i].kind == IR_Op_Kind::pseudo)
      {
        u64 id = instr->operands[i].pseudo_reg;
        instr->operands[i] = init_stack_op((id + 1) * (-4));
      }
    }
    if (instr->result.kind == IR_Op_Kind::pseudo)
    {
      u64 id = instr->result.pseudo_reg;
      instr->result = init_stack_op((id + 1) * (-4));
    }
  }
}

void IR_Program::emit_assembly(String8 file_name, bool debug_print = false)
{
  FILE *fp;
  if (debug_print) fp = stdout;
  else fp = fopen(file_name.c_str(), "w");

  for (int i = 1; i < ir_stream->length(); i++)
  {
    using enum Instr_Kind;
    IR_Instr *instr = ir_stream->get(i);
    switch (instr->kind)
    {
      case mov:
        {
          if ((instr->operands[0].kind == IR_Op::stack_mem)
              && (instr->result.kind == IR_Op::stack_mem))
          {
            fprintf(fp, "    movl    ");
            print_op(fp, instr->operands[0]);
            fprintf(fp, ", %%r10d\n");
            fprintf(fp, "    movl    %%r10d, ");
            print_op(fp, instr->result);
            fprintf(fp, "\n");
          }
          else
          {
            fprintf(fp, "    movl    ");
            print_op(fp, instr->operands[0]);
            fprintf(fp, ", ");
            print_op(fp, instr->result);
            fprintf(fp, "\n");
          }

          break;
        }
      case ret:
        {
          fprintf(fp, "    movq    %%rbp, %%rsp\n");
          fprintf(fp, "    popq    %%rbp\n");
          fprintf(fp, "    ret\n");

          break;
        }
      case fn_pre:
        {
          fprintf(fp, "    .globl  %s\n", instr->name.c_str());
          fprintf(fp, "%s:\n", instr->name.c_str());
          fprintf(fp, "    pushq   %%rbp\n");
          fprintf(fp, "    movq    %%rsp, %%rbp\n");

          break;
        }
      case salloc:
        {
          fprintf(fp, "    subq    $%lld, %%rsp\n", instr->amt);

          break;
        }
      case neg:
        {
          fprintf(fp, "    negl    ");
          print_op(fp, instr->operands[0]);
          fprintf(fp, "\n");

          break;
        }
      case b_not:
        {
          fprintf(fp, "    notl    ");
          print_op(fp, instr->operands[0]);
          fprintf(fp, "\n");

          break;
        }
      case add:
        {
          if ((instr->operands[0].kind == IR_Op::stack_mem)
              && (instr->result.kind == IR_Op::stack_mem))
          {
            fprintf(fp, "    movl    ");
            print_op(fp, instr->operands[0]);
            fprintf(fp, ", %%r10d\n");

            fprintf(fp, "    addl    %%r10d, ");
            print_op(fp, instr->result);
            fprintf(fp, "\n");
          }
          else
          {
            fprintf(fp, "    addl    ");
            print_op(fp, instr->operands[0]);
            fprintf(fp, ", ");
            print_op(fp, instr->result);
            fprintf(fp, "\n");
          }

          break;
        }
      case sub:
        {
          if ((instr->operands[0].kind == IR_Op::stack_mem)
              && (instr->result.kind == IR_Op::stack_mem))
          {
            fprintf(fp, "    movl    ");
            print_op(fp, instr->operands[0]);
            fprintf(fp, ", %%r10d\n");

            fprintf(fp, "    subl    %%r10d, ");
            print_op(fp, instr->result);
            fprintf(fp, "\n");
          }
          else
          {
            fprintf(fp, "    subl    ");
            print_op(fp, instr->operands[0]);
            fprintf(fp, ", ");
            print_op(fp, instr->result);
            fprintf(fp, "\n");
          }

          break;
        }
      case mul:
        {
          if (instr->result.kind == IR_Op::stack_mem)
          {
            fprintf(fp, "    movl    ");
            print_op(fp, instr->result);
            fprintf(fp, ", %%r11d\n");

            fprintf(fp, "    imull   ");
            print_op(fp, instr->operands[0]);
            fprintf(fp, ", %%r11d\n");

            fprintf(fp, "    movl    %%r11d, ");
            print_op(fp, instr->result);
            fprintf(fp, "\n");
          }
          else
          {
            fprintf(fp, "    imull   ");
            print_op(fp, instr->operands[0]);
            fprintf(fp, ", ");
            print_op(fp, instr->result);
            fprintf(fp, "\n");
          }

          break;
        }
      case div:
        {
          if (instr->operands[0].kind == IR_Op::immediate)
          {
            fprintf(fp, "    movl    ");
            print_op(fp, instr->operands[0]);
            fprintf(fp, ", %%r10d\n");

            fprintf(fp, "    idivl   %%r10d\n");
          }
          else
          {
            fprintf(fp, "    idivl   ");
            print_op(fp, instr->operands[0]);
            fprintf(fp, "\n");
          }

          break;
        }
      case cdq:
        {
          fprintf(fp, "    cdq\n");
          break;
        }
      default:
        return;
    }
  }

  fclose(fp);
}

void print_op(FILE *fp, IR_Op op)
{
  using enum IR_Op_Kind;
  switch (op.kind)
  {
    case reg_name:
      {
        switch (op.reg)
        {
          case Reg::eax:
            fprintf(fp, "%%eax");
            break;
          case Reg::edx:
            fprintf(fp, "%%edx");
            break;
          case Reg::r10d:
            fprintf(fp, "%%r10d");
            break;
          case Reg::r11d:
            fprintf(fp, "%%r11d");
            break;
          default:
            break;
        }
        break;
      }
    case immediate:
      {
        fprintf(fp, "$%lld", op.const_val);
        break;
      }
    case stack_mem:
      {
        fprintf(fp, "%lld(%%rbp)", op.stack);
        break;
      }
    case pseudo:
      return;
    default:
      return;
  }
}



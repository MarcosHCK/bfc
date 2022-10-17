/* Copyright 2021-2025 MarcosHCK
 * This file is part of bfc (BrainFuck Compiler).
 *
 * bfc (BrainFuck Compiler) is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * bfc (BrainFuck Compiler) is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bfc (BrainFuck Compiler).  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <config.h>
#include <codegen.hpp>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/DCE.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Utils.h>
#include <stream.hpp>
using namespace llvm;

G_DEFINE_QUARK (bfc-codegen-error-quark, bfc_codegen_error);
#define BFC_CODEGEN_ERROR (bfc_codegen_error_quark ())
#define BFC_CODEGEN_ERROR_FAILED (0)
static const char* NONAME = "";

struct BfcIterator
{
  BasicBlock* start;
  BasicBlock* end;

  inline static BfcIterator* alloc ()
  {
    return g_slice_new (BfcIterator);
  }

  inline static void free (gpointer pthis)
  {
    g_slice_free (BfcIterator, pthis);
  }
};

class BfcState
{
public:

  BfcState ()
  {
    context = std::unique_ptr<LLVMContext> (new LLVMContext ());
    builder = std::unique_ptr<IRBuilder<>> (new IRBuilder<> (*context));
  }

  inline void prologue (BfcOptions* opt, Module* module, GError** error)
  {
    BasicBlock* block;
    Value* zero = nullptr;
    Type* void_ = nullptr;

    auto beltsz = opt->beltsz;
    auto unitsz = sizeof (char);
    auto link = GlobalValue::ExternalLinkage;
    auto machine = (TargetMachine*) opt->machine;

    unit = Type::getIntNTy (*context, unitsz * 8);

    ioargs [0] = Type::getInt32Ty (*context);
    ioargs [1] = Type::getInt8PtrTy (*context);
    ioargs [2] = Type::getInt32Ty (*context);
    ioerr = BasicBlock::Create (*context);
    ioret = ioargs [0];

    readty = FunctionType::get (ioret, ioargs, false);
    read = Function::Create (readty, link, "read", module);
    writety = FunctionType::get (ioret, ioargs, false);
    write = Function::Create (writety, link, "write", module);

    mainty = FunctionType::get (ioret, false);
    main = Function::Create (mainty, link, "main", module);
    block = BasicBlock::Create (*context, NONAME, main);
    builder->SetInsertPoint (block);

    cursorty = Type::getIntNTy (*context, 32);
    cursor = builder->CreateAlloca (cursorty, nullptr, "cursor");
    {
      auto size = ConstantInt::get (cursorty, unitsz * beltsz, false);
      auto inst = CallInst::CreateMalloc (block, cursorty, unit, size, nullptr, nullptr);
      belt = builder->Insert (inst, "belt");
    }

    builder->CreateStore (ConstantInt::get (cursorty, 0, false), cursor);

    zero = ConstantInt::get (Type::getInt8Ty (*context), 0, false);
    builder->CreateMemSet (this->belt, zero, unitsz * beltsz, MaybeAlign (unitsz));
  }

  inline void epilogue (BfcOptions* opt, Module* module, GError** error)
  {
    auto block = builder->GetInsertBlock ();

    builder->Insert (CallInst::CreateFree (belt, block));
    builder->CreateRet (ConstantInt::get (ioret, 0, false));

    if (opt->checkio)
    {
      auto parent = block->getParent ();
      auto list = &parent->getBasicBlockList ();
        list->push_back (ioerr);

      builder->SetInsertPoint (ioerr);
      builder->Insert (CallInst::CreateFree (belt, ioerr));
      builder->CreateRet (ConstantInt::get (ioret, -1, true));
    }
  }

  inline void generate (BfcOptions* opt, BfcStream* input, GError** error)
  {
    auto stream = input->istream;
    GQueue iterators = G_QUEUE_INIT;
    GError* tmperr = NULL;
    gchar* line = NULL;
    gsize length = 0;
    gsize i;

    guint n_line = 1;
    guint n_column = 1;

    guint in_forwards = 0;
    guint in_backwards = 0;
    guint in_inc = 0;
    guint in_dec = 0;

  #define CLEANUP(...) \
    G_STMT_START { \
      g_queue_clear_full (&iterators, BfcIterator::free); \
      __VA_ARGS__; \
    } G_STMT_END
  #define CURSOR_GET() \
    (G_GNUC_EXTENSION ({ \
      builder->CreateLoad (cursorty, cursor); \
    }))
  #define CURSOR_SET(value) \
    G_STMT_START { \
      auto __aux = ((value)); \
      builder->CreateStore (__aux, cursor); \
    } G_STMT_END
  #define BELT_PTR() \
    (G_GNUC_EXTENSION ({ \
      builder->CreateInBoundsGEP (unit, belt, CURSOR_GET ()); \
    }))
  #define BELT_GET() \
    (G_GNUC_EXTENSION ({ \
      builder->CreateLoad (unit, BELT_PTR ()); \
    }))
  #define BELT_SET(value) \
    G_STMT_START { \
      auto __aux = ((value)); \
      builder->CreateStore (__aux, BELT_PTR ()); \
    } G_STMT_END

    while (1)
    {
      line = g_data_input_stream_read_line (stream, &length, NULL, &tmperr);
      if (G_UNLIKELY (tmperr != NULL))
      {
        g_propagate_error (error, tmperr);
        return;
      }

    #define GUARD(var) \
      G_STMT_START { \
        if ((var) == 0) \
        { \
          if (in_backwards > 0) \
            goto putbackward; \
          if (in_forwards > 0) \
            goto putforward; \
          if (in_dec > 0) \
            goto putdec; \
          if (in_inc > 0) \
            goto putinc; \
        } \
      } G_STMT_END

      if (line == NULL)
        break;
      else
      {
        Value* index = NULL;
        Value* value = NULL;
        Value* aux = NULL;
        gchar* ptr = line;
        gchar* top = ptr + length;
        gunichar c;

        do
        {
          c = g_utf8_get_char (ptr);

          switch (c)
          {
            case (gunichar) '<':
              GUARD (in_backwards);
              ++in_backwards;
              break;
            case (gunichar) '>':
              GUARD (in_forwards);
              ++in_forwards;
              break;
            case (gunichar) '-':
              GUARD (in_dec);
              ++in_dec;
              break;
            case (gunichar) '+':
              GUARD (in_inc);
              ++in_inc;
              break;

            case (gunichar) ',':
              GUARD (0);
              {
                Value* args [] =
                {
                  ConstantInt::get (ioargs [0], 0, false),
                  builder->CreateBitCast (BELT_PTR (), ioargs [1]),
                  ConstantInt::get (ioargs [2], 1, false),
                };

                value = builder->CreateCall (readty, read, args);
                goto checkio;
              }
            case (gunichar) '.':
              GUARD (0);
              {
                value = BELT_GET ();
                Value* args [] =
                {
                  ConstantInt::get (ioargs [0], 1, false),
                  builder->CreateBitCast (BELT_PTR (), ioargs [1]),
                  ConstantInt::get (ioargs [2], 1, false),
                };

                value = builder->CreateCall (writety, write, args);
                goto checkio;
              }

            case (gunichar) '[':
              GUARD (0);
              {
                gchar* n1 = g_utf8_next_char (ptr);
                gchar* n2 = g_utf8_next_char (n1);
                gboolean normal = TRUE;
                if (n2 < ptr)
                {
                  gunichar c1 = g_utf8_get_char (n1);
                  gunichar c2 = g_utf8_get_char (n2);
                  if (c1 == (gunichar) '-' && c2 == (gunichar) ']')
                  {
                    BELT_SET (ConstantInt::get (unit, 0, false));
                    normal = FALSE;
                    ptr = n2;
                  }
                }

                if (normal)
                {
                  BfcIterator* iter;
                  BasicBlock* start;
                  BasicBlock* end;
                  BasicBlock* block;
                  Function* parent;

                  block = builder->GetInsertBlock ();
                  parent = block->getParent ();
                  start = BasicBlock::Create (*context, NONAME, parent);
                  block = BasicBlock::Create (*context, NONAME, parent);
                  end = BasicBlock::Create (*context, NONAME, parent);

                  builder->CreateBr (start);
                  builder->SetInsertPoint (start);

                  aux = ConstantInt::get (unit, 0, 0);
                  value = BELT_GET ();
                  value = builder->CreateICmpEQ (value, aux);

                  builder->CreateCondBr (value, end, block);
                  builder->SetInsertPoint (block);

                  iter = BfcIterator::alloc ();
                  iter->start = start;
                  iter->end = end;
                  g_queue_push_head (&iterators, iter);
                }
              }
              break;
            case (gunichar) ']':
              GUARD (0);
              {
                if (iterators.length == 0)
                {
                  g_set_error
                  (error,
                  BFC_CODEGEN_ERROR,
                  BFC_CODEGEN_ERROR_FAILED,
                  "%s: %i: %i: Unmatched ']' token",
                    input->filename, n_line, n_column);
                  CLEANUP (return);
                }
                else
                {
                  BfcIterator* iter;
                  iter = (BfcIterator*) g_queue_pop_head (&iterators);

                  aux = ConstantInt::get (unit, 0, 0);
                  value = BELT_GET ();
                  value = builder->CreateICmpEQ (value, aux);

                  builder->CreateCondBr (value, iter->end, iter->start);
                  builder->SetInsertPoint (iter->end);
                  BfcIterator::free (iter);
                }
              }
              break;

            default:
              if (opt->strict
                && !(g_unichar_iscntrl (c)
                  || g_unichar_isspace (c)))
              {
                gchar buffer [8] = {0};
                gint wrote = 0;

                wrote = g_unichar_to_utf8 (c, buffer);

                g_set_error
                (error,
                BFC_CODEGEN_ERROR,
                BFC_CODEGEN_ERROR_FAILED,
                "%s: %i: %i: Unknown character '%.*s'",
                  input->filename, n_line, n_column,
                  wrote, buffer);
                CLEANUP (return);
              }
              break;

            putbackward:
              index = CURSOR_GET ();
              aux = ConstantInt::get (cursorty, in_backwards, false);
              CURSOR_SET (builder->CreateSub (index, aux));
              in_backwards = 0;
              continue;
            putforward:
              index = CURSOR_GET ();
              aux = ConstantInt::get (cursorty, in_forwards, false);
              CURSOR_SET (builder->CreateAdd (index, aux));
              in_forwards = 0;
              continue;
            putdec:
              value = BELT_GET ();
              aux = ConstantInt::get (unit, in_dec, false);
              BELT_SET (builder->CreateSub (value, aux));
              in_dec = 0;
              continue;
            putinc:
              value = BELT_GET ();
              aux = ConstantInt::get (unit, in_inc, false);
              BELT_SET (builder->CreateAdd (value, aux));
              in_inc = 0;
              continue;

            checkio:
              if (opt->checkio)
              {
                auto block = builder->GetInsertBlock ();
                auto parent = block->getParent ();
                auto then = BasicBlock::Create (*context, NONAME, parent);

                aux = ConstantInt::get (ioret, 0, false);
                value = builder->CreateICmpSLT (value, aux);

                builder->CreateCondBr (value, ioerr, then);
                builder->SetInsertPoint (then);
              }
              break;
          }

          ptr = g_utf8_next_char (ptr);
          ++n_column;
        }
        while (ptr < top);
      }

      ++n_line;
    #undef GUARD
    }

    if (G_UNLIKELY (iterators.length > 0))
    {
      g_set_error
      (error,
      BFC_CODEGEN_ERROR,
      BFC_CODEGEN_ERROR_FAILED,
      "%s: %i: %i: Unmatched ']' token",
        input->filename, n_line, n_column);
      CLEANUP (return);
    }

  #undef GUARD
  #undef BELT_SET
  #undef BELT_GET
  #undef BELT_PTR
  #undef CURSOR_SET
  #undef CURSOR_GET
    CLEANUP ();
  #undef CLEANUP
  }

  inline void optimize (BfcOptions* opt, Module* module, GError** error)
  {
    auto pass = legacy::FunctionPassManager (module);
    auto level = opt->olevel;

    {
      pass.add (llvm::createVerifierPass ());

      if (level > 0)
      {
        pass.add (llvm::createInstructionCombiningPass ());
        pass.add (llvm::createReassociatePass ());
        pass.add (llvm::createGVNPass ());
        pass.add (llvm::createCFGSimplificationPass ());

        if (level == 1)
        {
          pass.add (llvm::createDeadCodeEliminationPass ());
        }
      }

      if (level > 1)
      {
        pass.add (llvm::createPromoteMemoryToRegisterPass ());
        pass.add (llvm::createAggressiveDCEPass ());
      }

      pass.doInitialization ();
    }

    auto begin = module->begin ();
    auto end = module->end ();

    for (auto iter = begin; iter != end; ++iter)
      pass.run (*iter);
    pass.doFinalization ();
  }

  inline void dump (BfcOptions* opt, Module* module, GError** error)
  {
    auto output = (GOutputStream*) opt->output.stream;
    auto machine = (TargetMachine*) opt->machine;
    auto stream = Bfc::OStream (output);
    auto target = machine->getTargetTriple ();
    auto layout = machine->createDataLayout ();

    module->setDataLayout (layout);
    module->setPICLevel ((PICLevel::Level) opt->pic);
    module->setPIELevel ((PIELevel::Level) opt->pie);
    module->setTargetTriple (target.getTriple ());

    if (opt->assemble && opt->emitll)
    {
      module->print (stream, nullptr, true, false);
      stream.flush ();
    }
    else
    {
      auto type = (opt->assemble) ? CGFT_AssemblyFile : CGFT_ObjectFile;
      auto pass = legacy::PassManager ();

      if (machine->addPassesToEmitFile (pass, stream, nullptr, type))
      {
        g_set_error
        (error,
        BFC_CODEGEN_ERROR,
        BFC_CODEGEN_ERROR_FAILED,
        "Unsupported emit file type");
        return;
      }
      else
      {
        pass.run (*module);
        stream.flush ();
      }
    }
  }

  std::unique_ptr<LLVMContext> context;
  std::unique_ptr<IRBuilder<>> builder;
private:
  FunctionType *mainty, *readty, *writety;
  Function* main, *read, *write;
  Value *belt, *cursor;
  Type *unit, *cursorty, *ioret, *ioargs [3];
  BasicBlock* ioerr;
};

enum Passes
{
  pass_prologue,
  pass_generate,
  pass_epilogue,
  pass_optimize,
  pass_dump,
  pass_max,
};

void
bfc_main (BfcOptions* opt, GError** error)
{
  if (opt->assemble)
    InitializeAllAsmPrinters ();
  else
  {
    InitializeAllTargetInfos ();
    InitializeAllTargets ();
    InitializeAllTargetMCs ();
    InitializeAllAsmParsers ();
    InitializeAllAsmPrinters ();
  }

  if (opt->compile && opt->n_inputs > 1)
  {
    g_set_error
    (error,
     BFC_CODEGEN_ERROR,
     BFC_CODEGEN_ERROR_FAILED,
     "Compilation takes only one file at a time");
    return;
  }

  auto tmperr = (GError*) nullptr;
  auto state = BfcState ();

  for (guint i = 0; i < opt->n_inputs; ++i)
  {
    auto stream = & opt->inputs [i];
    auto module = (Module*) nullptr;
    auto name = (gchar*) stream->filename;

    if (!g_strcmp0 (name, "-"))
      name = g_strdup ("(stdin)");
    else
      name = g_path_get_basename (name);

    module = new Module (name, *(state.context));
      g_free (name);

    for (guint j = 0; j < pass_max; ++j)
    switch (j)
    {
      case pass_prologue:
        state.prologue (opt, module, &tmperr);
        goto check;
      case pass_generate:
        state.generate (opt, stream, &tmperr);
        goto check;
      case pass_epilogue:
        state.epilogue (opt, module, &tmperr);
        goto check;
      case pass_optimize:
        state.optimize (opt, module, &tmperr);
        goto check;
      case pass_dump:
        state.dump (opt, module, &tmperr);
        goto check;

      check:
        if (G_UNLIKELY (tmperr != nullptr))
        {
          g_propagate_error (error, tmperr);
          delete module;
          return;
        }
        break;
    }

    delete module;
  }
}

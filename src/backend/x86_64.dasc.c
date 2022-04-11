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
#include <bfd.h>
#include <codegen.h>
#include <gio/gio.h>

#define throw_action return FALSE;
#include <bfd_error.h>

#include <dynasm.h>
#include <dynasm/dasm_x86.h>
#include <x86_64.h>

#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))

#if __BACKEND__
|.arch x64
|.section code
|.globals globl_
|.actionlist actions
|.globalnames globl_names
|.externnames extern_names

|.define rTape, rbx
|.define rState, r12

|.if G_OS_WINDOWS
  |.define rTapeBegin, rsi
  |.define rTapeEnd, rdi
  |.define rArg1, rcx
  |.define rArg2, rdx
|.else
  |.define rTapeBegin, r13
  |.define rTapeEnd, r14
  |.define rArg1, rdi
  |.define rArg2, rsi
|.endif

|.macro prepcall1, arg1
| mov rArg1, arg1
|.endmacro

|.macro prepcall2, arg1, arg2
| mov rArg1, arg1
| mov rArg2, arg2
|.endmacro

|.define postcall, .nop

|.type state, BfState, rState
#else // !__BACKEND__
# define DASM_MAXSECTION (0)
# define globl__main (0)
# define globl__MAX (0)
# define actions (NULL)
extern const gchar** globl_names;
#endif // __BACKEND__

#define MAX_NESTING 100
#define ALIGNMENT 8

#define BFC_TYPE_CODEGENX8664 (bfc_codegen_get_type ())
#define BFC_CODEGENX8664(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), BFC_TYPE_CODEGENX8664, BfcCodegenX8664))
#define BFC_CODEGENX8664_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), BFC_TYPE_CODEGENX8664, BfcCodegenX8664Class))
#define BFC_IS_CODEGENX8664(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BFC_TYPE_CODEGENX8664))
#define BFC_IS_CODEGENX8664_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BFC_TYPE_CODEGENX8664))
#define BFC_CODEGENX8664_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), BFC_TYPE_CODEGENX8664, BfcCodegenX8664Class))

typedef struct _BfcCodegenX8664 BfcCodegenX8664;
typedef struct _BfcCodegenX8664Class BfcCodegenX8664Class;

struct _BfcCodegenX8664
{
  BfcCodegen parent;

  dasm_State* state;
  gpointer* labels;
  gssize codesz;

  gsize consumed;

  guint loops[MAX_NESTING];
  guint nloops;

  guint nextpc;
  guint npc;
};

struct _BfcCodegenX8664Class
{
  BfcCodegenClass parent;
};

G_DEFINE_FINAL_TYPE
(BfcCodegenX8664,
 bfc_codegen_x86_64,
 BFC_TYPE_CODEGEN);

#define at (self->consumed)
#define loops (self->loops)
#define nloops (self->nloops)
#define nextpc (self->nextpc)
#define npc (self->npc)

static gboolean
bfc_codegen_x86_64_class_dump (BfcCodegen* pself, gpointer abfd, GError** error)
{
  BfcCodegenX8664* self = (BfcCodegenX8664*) pself;
  dasm_State** Dst = &(self->state);
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  guchar* buffer = NULL;
  asection* sec = NULL;
  gsize codesz = self->codesz;
  guint result = 0;

  buffer = g_new (guchar, codesz);
  dasm_encode (Dst, buffer);

  sec =
  bfd_make_section (abfd, ".text");
  if (G_UNLIKELY (sec == NULL))
    {
      _g_free0 (buffer);
      enum bfd_error err = bfd_get_error ();
      const gchar* msg = bfd_strerror (err);
      throw (BFC_CODEGEN_ERROR_FAILED, msg);
    }

  result =
  bfd_set_section_size (sec, self->codesz);
  if (G_UNLIKELY (!result))
    {
      _g_free0 (buffer);
      enum bfd_error err = bfd_get_error ();
      const gchar* msg = bfd_strerror (err);
      throw (BFC_CODEGEN_ERROR_FAILED, msg);
    }

  result =
  bfd_set_section_vma (sec, (bfd_vma) buffer);
  if (G_UNLIKELY (!result))
    {
      _g_free0 (buffer);
      enum bfd_error err = bfd_get_error ();
      const gchar* msg = bfd_strerror (err);
      throw (BFC_CODEGEN_ERROR_FAILED, msg);
    }

  result =
  bfd_set_section_alignment (sec, ALIGNMENT);
  if (G_UNLIKELY (!result))
    {
      _g_free0 (buffer);
      enum bfd_error err = bfd_get_error ();
      const gchar* msg = bfd_strerror (err);
      throw (BFC_CODEGEN_ERROR_FAILED, msg);
    }

  result =
  bfd_set_section_flags (sec, SEC_CODE | SEC_LOAD | SEC_READONLY | SEC_HAS_CONTENTS);
  if (G_UNLIKELY (!result))
    {
      _g_free0 (buffer);
      enum bfd_error err = bfd_get_error ();
      const gchar* msg = bfd_strerror (err);
      throw (BFC_CODEGEN_ERROR_FAILED, msg);
    }

  {
    asymbol* symbol = NULL;
    bfd* abfd_ = abfd;
    guint i;

    asymbol** symtab = g_new (asymbol*, globl__MAX);

    for (i = 0; i < globl__MAX; i++)
    {
      symbol = bfd_make_empty_symbol (abfd_);
      if (G_UNLIKELY (symbol == NULL))
        {
          _g_free0 (buffer);
          enum bfd_error err = bfd_get_error ();
          const gchar* msg = bfd_strerror (err);
          throw (BFC_CODEGEN_ERROR_FAILED, msg);
        }

      symbol->name = globl_names[i];
      symbol->section = sec;
      symbol->flags = BSF_GLOBAL | BSF_EXPORT | BSF_FUNCTION;
      symbol->value = self->labels[i] - (gpointer) buffer;
      symtab[i] = symbol;
    }

    bfd_set_symtab (abfd, symtab, globl__MAX);
  }

  result =
  bfd_set_section_contents (abfd, sec, buffer, 0, codesz);
  _g_free0 (buffer);

  if (G_UNLIKELY (!result))
    {
      enum bfd_error err = bfd_get_error ();
      const gchar* msg = bfd_strerror (err);
      throw (BFC_CODEGEN_ERROR_FAILED, msg);
    }
return success;
}

static guint
bfc_codegen_x86_64_class_consume (BfcCodegen* pself, gconstpointer buffer, gsize size, GError** error)
{
  BfcCodegenX8664* self = (BfcCodegenX8664*) pself;
  dasm_State** Dst = &(self->state);
  guint8* buf = (gpointer) buffer;
  gboolean strict = FALSE;
  gsize i, n;

  strict = bfc_codegen_get_strict (pself);

  for (i = 0; i < size; ++at, ++i)
  switch (*buf++)
  {
  case '<':
    for (n = 1; *buf == '<' && i < size; ++buf, ++i) ++n;
      at += n;
#if __BACKEND__
    | sub rTape, n % TAPE_SIZE
    | cmp rTape, rTapeBegin
    | ja >1
    | add rTape, TAPE_SIZE
    |1:
#endif // __BACKEND__
    break;
  case '>':
    for (n = 1; *buf == '>' && i < size; ++buf, ++i) ++n;
      at += n;
#if __BACKEND__
    | add rTape, n % TAPE_SIZE
    | cmp rTape, rTapeBegin
    | ja >1
    | sub rTape, TAPE_SIZE
    |1:
#endif // __BACKEND__
    break;
  case '+':
    for (n = 1; *buf == '+' && i < size; ++buf, ++i) ++n;
      at += n;
#if __BACKEND__
    | add byte [rTape], n
#endif // __BACKEND__
    break;
  case '-':
    for (n = 1; *buf == '-' && i < size; ++buf, ++i) ++n;
      at += n;
#if __BACKEND__
    | sub byte [rTape], n
#endif // __BACKEND__
    break;
  case ',':
#if __BACKEND__
    | prepcall1 rState
    | call aword state->getc
    | postcall 1
    | mov byte [rTape], al
#endif // __BACKEND__
    break;
  case '.':
#if __BACKEND__
    | movzx rax, byte [rTape]
    | prepcall2 rState, rax
    | call aword state->putc
    | postcall 2
#endif // __BACKEND__
    break;
  case '[':
#if __BACKEND__
    if (nloops == MAX_NESTING)
      throw (BFC_CODEGEN_ERROR_NESTING_TOO_DEEP, "%i: Nesting too deep", at);
    if (buf[0] == '-' && buf[1] == ']')
      {
        buf += 2;
        | xor rax, rax
        | mov byte [rTape], al
      }
    else
      {
        if (nextpc == npc)
          {
            npc *= 2;
            dasm_growpc (Dst, npc);
          }

        | cmp byte [rTape], 0
        | jz =>nextpc+1
        |=>nextpc:
        loops[nloops++] = nextpc;
        nextpc += 2;
      }
#endif // __BACKEND__
    break;
  case ']':
#if __BACKEND__
    if (nloops == 0)
      throw (BFC_CODEGEN_ERROR_UNMATCH_LOOP, "%i: Unmatch loop", at);
    --nloops;
    | cmp byte [rTape], 0
    | jnz =>loops[nloops]
    |=>loops[nloops]+1:
#endif // __BACKEND__
    break;
  default:
    if (strict)
    {
      gchar c = *(buf - 1);
      if (!g_ascii_iscntrl (c)
        && !g_ascii_isspace (c))
        {
          throw (BFC_CODEGEN_ERROR_UNKNOWN_SYMBOL, "%i: Unknown symbol %c", at, *(buf - 1));
        }
    }
    break;
  }
return TRUE;
}
#include <stdio.h>
static void
bfc_codegen_x86_64_class_freeze (BfcCodegen* pself, GError** error)
{
  BfcCodegenX8664* self = (BfcCodegenX8664*) pself;
  dasm_State** Dst = &(self->state);
  size_t sz;

#pragma push(throw_action)
#undef throw_action
#define throw_action return;

  if (nloops > 0)
    throw (BFC_CODEGEN_ERROR_UNMATCH_LOOP, "%i: Unmatch loop", at);

#pragma pop(throw_action)

#if __BACKEND__
  | pop rTapeEnd
  | pop rTapeBegin
  | pop rState
  | pop rTape
  | ret
#endif // __BACKEND__

  dasm_link (Dst, &sz);
  g_assert (sz < G_MAXSSIZE);
  self->codesz = sz;
}

static void
bfc_codegen_x86_64_class_finalize (BfcCodegen* pself)
{
  BfcCodegenX8664* self = (BfcCodegenX8664*) pself;
  dasm_State** Dst = &(self->state);
  g_free (self->labels);
  dasm_free (Dst);
BFC_CODEGEN_CLASS (bfc_codegen_x86_64_parent_class)->finalize (pself);
}

static void
bfc_codegen_x86_64_class_init (BfcCodegenX8664Class* klass)
{
  BfcCodegenClass* oclass = BFC_CODEGEN_CLASS (klass);

  oclass->bfd_arch = "i386:x86-64";
  oclass->dump = bfc_codegen_x86_64_class_dump;
  oclass->consume = bfc_codegen_x86_64_class_consume;
  oclass->freeze = bfc_codegen_x86_64_class_freeze;
  oclass->finalize = bfc_codegen_x86_64_class_finalize;
}

static void
bfc_codegen_x86_64_init (BfcCodegenX8664* self)
{
  self->codesz = -1;
  self->consumed = 0;
  self->labels = g_malloc_n (globl__MAX, sizeof (gpointer));
  dasm_State** Dst = &(self->state);

  nloops = 0;
  nextpc = 0;
  npc = 2;
  at = 0;

  dasm_init (Dst, DASM_MAXSECTION);
  dasm_setupglobal (Dst, self->labels, globl__MAX);
  dasm_setup (Dst, actions);
  dasm_growpc (Dst, npc);

#if __BACKEND__
  |.code
  |->_main:
  | push rTape
  | push rState
  | push rTapeBegin
  | push rTapeEnd
  | mov rState, rArg1
  | mov rTape, state->tape
  | lea rTapeBegin, [rTape-1]
  | lea rTapeEnd, [rTape+TAPE_SIZE-1]
#endif // __BACKEND__
}

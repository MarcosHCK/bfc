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
#include <codegen.h>

#include <dynasm.h>

struct _BfcCodegen
{
  BfcCodegenBackend* backend;
  dasm_State* D;
};

#define BACKEND(name) \
extern const BfcCodegenBackend backend_##name; \
int backend_##name##_init (Dst_DECL); \
int backend_##name##_fini (Dst_DECL); \

BACKEND (x86_64)

BfcCodegen*
bfc_codegen_new (BfcCodegenArch arch)
{
  BfcCodegen* gen = NULL;
  BfcCodegenBackend* backend = NULL;
  dasm_State** Dst = NULL;
  gsize labelsz = 0;

  switch (arch)
  {
  case BFC_CODEGEN_ARCH_X86_64:
    backend = g_memdup2 (&backend_x86_64, sizeof (backend_x86_64));
    break;
  }

  gen = g_slice_new (BfcCodegen);
  gen->backend = backend;
  Dst = &(gen->D);

  labelsz += backend->labelsz;
  labelsz *= backend->n_labels;
  backend->labels = g_malloc (labelsz);

  dasm_init (Dst, backend->n_sections);
  dasm_setupglobal (Dst, backend->labels, backend->n_labels);
  backend_x86_64_init (Dst);
}

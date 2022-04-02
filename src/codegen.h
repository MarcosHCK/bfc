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
#ifndef __BFC_CODEGEN__
#define __BFC_CODEGEN__ 1
#include <glib.h>

typedef enum
{
  BFC_CODEGEN_ARCH_X86_64,
} BfcCodegenArch;

typedef struct _BfcCodegen BfcCodegen;
typedef struct _BfcCodegenBackend BfcCodegenBackend;

#if __cplusplus
extern "C" {
#endif // __cplusplus

struct _BfcCodegenBackend
{
  gpointer* labels;
  guint n_labels;
  guint labelsz;
  guint n_sections;
  guint main;
};

#if __cplusplus
}
#endif // __cplusplus

#endif // __BFC_CODEGEN__
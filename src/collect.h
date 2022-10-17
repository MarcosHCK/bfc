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
#ifndef __BFC_COLLECT__
#define __BFC_COLLECT__ 1
#include <bfc.h>
#include <glib.h>
#include <llvm-c/TargetMachine.h>

G_BEGIN_DECLS

enum
{
  RELOC_DEFAULT = 0,
  RELOC_STATIC,
  RELOC_PIC,
  RELOC_DYNNOPIC,
};

enum
{
  MMODEL_DEFAULT = 0,
  MMODEL_TINY = 2,
  MMODEL_SMALL,
  MMODEL_KERNEL,
  MMODEL_MEDIUM,
  MMODEL_LARGE,
};

enum
{
  PIC_EXE = (1 << 0),
  PIC_SHARED = (1 << 1),
  PIC_ALL = PIC_EXE | PIC_SHARED,
};

#define COLLECT_PIC(pic,PIC) ((pic) | ((PIC) << 1))
#define COLLECT_PIE(pie,PIE) ((pie) | ((PIE) << 1))

G_GNUC_INTERNAL void
collect_codegen (BfcOptions* opt, gboolean static_, guint pic, guint pie, const gchar* mmodel, GError** error);
G_GNUC_INTERNAL void
collect_machine (BfcOptions* opt, const gchar* arch, const gchar* tune, const gchar* features, GError** error);

G_END_DECLS

#endif // __BFC_COLLECT__

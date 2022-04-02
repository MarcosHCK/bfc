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
#ifndef __BFC_OPTIONS__
#define __BFC_OPTIONS__ 1
#include <gio/gio.h>

typedef struct
{
  GOutputStream* output;
  GInputStream** inputs;
  guint n_inputs;
  gboolean dontlink;
} BfcOptions;

typedef enum
{
  BFC_OPEN_READ = 'r',
  BFC_OPEN_WRITE = 'w',
} BfcOpenMode;

gpointer
bfc_options_open (const gchar* filename, BfcOpenMode mode, GError** error);
void
bfc_options_emit (BfcOptions* options, GString* string);
void
bfc_options_clear (BfcOptions* options);

#endif // __BFC_OPTIONS__

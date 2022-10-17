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
#ifndef __BFC_MAIN__
#define __BFC_MAIN__ 1
#include <gio/gio.h>

typedef struct _BfcOptions BfcOptions;
typedef struct _BfcStream BfcStream;

G_BEGIN_DECLS

struct _BfcStream
{
  const gchar* filename;

  union
  {
    gpointer stream;
    GDataInputStream* istream;
    GDataOutputStream* ostream;
  };
};

struct _BfcOptions
{
  gsize beltsz;
  guint assemble : 1;
  guint checkio : 1;
  guint compile : 1;
  guint emitll : 1;
  guint mmodel : 3;
  guint olevel : 6;
  guint pic : 2;
  guint pie : 2;
  guint reloc : 3;
  guint static_ : 1;
  guint strict : 1;

  const gchar* arch;
  const gchar* features;
  const gchar* tune;
  gpointer machine;

  BfcStream output, *inputs;
  guint n_inputs;
};

G_GNUC_INTERNAL void
bfc_main (BfcOptions* opt, GError** error);
G_GNUC_INTERNAL void
bfc_optimize (BfcOptions* opt, gpointer module, GError** error);
G_GNUC_INTERNAL void
bfc_dump (BfcOptions* opt, gpointer module_, GError** error);

G_END_DECLS

#endif // __BFC_MAIN__

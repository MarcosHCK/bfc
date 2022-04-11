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
#include <glib-object.h>

#define BFC_CODEGEN_ERROR (bfc_codegen_error_quark ())

typedef enum
{
  BFC_CODEGEN_ERROR_FAILED,
  BFC_CODEGEN_ERROR_UNKNOWN_SYMBOL,
  BFC_CODEGEN_ERROR_NESTING_TOO_DEEP,
  BFC_CODEGEN_ERROR_UNMATCH_LOOP,
} BfcCodegenError;

typedef enum
{
  BFC_CODEGEN_ARCH_X86_64,
  BFC_CODEGEN_ARCH_I386,
} BfcCodegenArch;

#define BFC_TYPE_CODEGEN (bfc_codegen_get_type ())
#define BFC_CODEGEN(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), BFC_TYPE_CODEGEN, BfcCodegen))
#define BFC_CODEGEN_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), BFC_TYPE_CODEGEN, BfcCodegenClass))
#define BFC_IS_CODEGEN(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BFC_TYPE_CODEGEN))
#define BFC_IS_CODEGEN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BFC_TYPE_CODEGEN))
#define BFC_CODEGEN_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), BFC_TYPE_CODEGEN, BfcCodegenClass))

typedef struct _BfcCodegen BfcCodegen;
typedef struct _BfcCodegenPrivate BfcCodegenPrivate;
typedef struct _BfcCodegenClass BfcCodegenClass;

#if __cplusplus
extern "C" {
#endif // __cplusplus

GQuark
bfc_codegen_error_quark (void) G_GNUC_CONST;
GType
bfc_codegen_arch_get_type (void) G_GNUC_CONST;
GType
bfc_codegen_get_type (void) G_GNUC_CONST;

struct _BfcCodegen
{
  GTypeInstance parent_intance;
  gint ref_count;
};

struct _BfcCodegenClass
{
  GTypeClass parent_class;
  const gchar* bfd_arch;
  gboolean (*dump) (BfcCodegen* codegen, gpointer abfd, GError** error);
  guint (*consume) (BfcCodegen* codegen, gconstpointer buffer, gsize size, GError** error);
  void (*freeze) (BfcCodegen* codegen, GError** error);
  void (*finalize) (BfcCodegen* codegen);
};

gpointer
bfc_codegen_new (BfcCodegenArch arch);
gpointer
bfc_codegen_ref (gpointer codegen);
void
bfc_codegen_unref (gpointer codegen);
void
bfc_codegen_set_strict (gpointer codegen, gboolean strict);
gboolean
bfc_codegen_get_strict (gpointer codegen);
gboolean
bfc_codegen_dump (gpointer codegen, gpointer abfd, GError** error);
guint
bfc_codegen_consume (gpointer codegen, gconstpointer buffer, gsize size, GError** error);
void
bfc_codegen_freeze (gpointer codegen, GError** error);

#if __cplusplus
}
#endif // __cplusplus

#endif // __BFC_CODEGEN__

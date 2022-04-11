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

G_DEFINE_QUARK
(bfc-codegen-error-quark,
 bfc_codegen_error);

static void
bfc_codegen_class_init (BfcCodegenClass* klass);
static void
bfc_codegen_init (BfcCodegen* self);

static gint bfc_codegen_private_offset = 0;
static inline BfcCodegenPrivate*
bfc_codegen_get_instance_private (BfcCodegen* self)
{
	return G_STRUCT_MEMBER_P (self, bfc_codegen_private_offset);
}

struct _BfcCodegenPrivate
{
  guint freeze : 1;
  guint strict : 1;
};

GType
bfc_codegen_arch_get_type (void)
{
  static volatile
  gsize __typeid__ = 0;
  if (g_once_init_enter (&__typeid__))
    {
      GType g_type;

      static const GEnumValue values[] =
      {
        { BFC_CODEGEN_ARCH_X86_64, "BFC_CODEGEN_ARCH_X86_64", "x86_64" },
        { BFC_CODEGEN_ARCH_I386, "BFC_CODEGEN_ARCH_I386", "i386" },
        {0, NULL, NULL},
      };

      g_type =
      g_enum_register_static ("BfcCodegenArch", values);
      g_once_init_leave (&__typeid__, g_type);
    }
return (GType) __typeid__;
}

GType
bfc_codegen_get_type (void)
{
  static volatile
  gsize __typeid__ = 0;
  if (g_once_init_enter (&__typeid__))
    {
      GType g_type;

      static const
      GTypeInfo __info__ =
      {
        sizeof (BfcCodegenClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) bfc_codegen_class_init,
        (GClassFinalizeFunc) NULL,
        NULL,
        sizeof (BfcCodegen),
        0,
        (GInstanceInitFunc) bfc_codegen_init,
      };

      static const
      GTypeFundamentalInfo __fundamental__ =
      {
        (G_TYPE_FLAG_CLASSED
        | G_TYPE_FLAG_INSTANTIATABLE
        | G_TYPE_FLAG_DERIVABLE
        | G_TYPE_FLAG_DEEP_DERIVABLE),
      };

      g_type =
      g_type_register_fundamental
      (g_type_fundamental_next(),
       "BfcCodegen",
       &__info__,
       &__fundamental__,
       G_TYPE_FLAG_ABSTRACT);

      bfc_codegen_private_offset =
      g_type_add_instance_private (g_type, sizeof (BfcCodegenPrivate));
      g_once_init_leave (&__typeid__, g_type);
    }
return (GType) __typeid__;
}

static gboolean
bfc_codegen_class_dump (BfcCodegen* self, gpointer abfd, GError** error)
{
  g_critical ("BfcCodegen::dump not implemented for '%s'", g_type_name_from_instance (&self->parent_intance));
return FALSE;
}

static guint
bfc_codegen_class_consume (BfcCodegen* self, gconstpointer buffer, gsize size, GError** error)
{
  g_critical ("BfcCodegen::consume not implemented for '%s'", g_type_name_from_instance (&self->parent_intance));
return (guint) -1;
}

static void
bfc_codegen_class_freeze (BfcCodegen* self, GError** error)
{
  g_critical ("BfcCodegen::freeze not implemented for '%s'", g_type_name_from_instance (&self->parent_intance));
}

static void
bfc_codegen_class_finalize (BfcCodegen* self)
{
}

static void
bfc_codegen_class_init (BfcCodegenClass* klass)
{
  g_type_class_adjust_private_offset (klass, &bfc_codegen_private_offset);

  klass->dump = bfc_codegen_class_dump;
  klass->consume = bfc_codegen_class_consume;
  klass->freeze = bfc_codegen_class_freeze;
  klass->finalize = bfc_codegen_class_finalize;
}

static void
bfc_codegen_init (BfcCodegen* self)
{
  BfcCodegenPrivate* priv =
  bfc_codegen_get_instance_private (self);
  g_atomic_ref_count_init (&(self->ref_count));
  priv->freeze = FALSE;
}

/*
 * API
 *
 */

#define BACKEND(name) \
GType bfc_codegen_##name##_get_type (void) G_GNUC_CONST;

BACKEND (x86_64)
BACKEND (i386)

gpointer
bfc_codegen_new (BfcCodegenArch arch)
{
  GType g_type = G_TYPE_INVALID;

  switch (arch)
  {
  case BFC_CODEGEN_ARCH_X86_64:
    g_type = bfc_codegen_x86_64_get_type ();
    break;
  case BFC_CODEGEN_ARCH_I386:
    g_type = bfc_codegen_i386_get_type ();
    break;
  }

  g_assert (g_type_is_a (g_type, BFC_TYPE_CODEGEN));
return (gpointer) g_type_create_instance (g_type);
}

gpointer
(bfc_codegen_ref) (gpointer codegen)
{
  g_return_val_if_fail (BFC_IS_CODEGEN (codegen), NULL);
  g_atomic_ref_count_inc (&( ((BfcCodegen*) codegen)->ref_count ));
return codegen;
}

void
(bfc_codegen_unref) (gpointer codegen)
{
  g_return_if_fail (BFC_IS_CODEGEN (codegen));
  BfcCodegen* self = (BfcCodegen*) codegen;
  if (g_atomic_ref_count_dec (&( self->ref_count )))
    {
      if (!bfc_codegen_get_instance_private (self)->freeze)
        {
          GError* tmp_err = NULL;
          bfc_codegen_freeze (self, &tmp_err);
          if (G_UNLIKELY (tmp_err != NULL))
          {
            g_critical
            ("(%s: %i): %s: %i: %s",
             G_STRLOC,
             __LINE__,
             g_quark_to_string
             (tmp_err->domain),
             tmp_err->code,
             tmp_err->message);
            g_error_free (tmp_err);
            g_assert_not_reached ();
          }
        }

      BFC_CODEGEN_GET_CLASS (codegen)->finalize (self);
      g_type_free_instance ((GTypeInstance*) self);
    }
}

void
(bfc_codegen_set_strict) (gpointer codegen, gboolean strict)
{
  g_return_if_fail (BFC_IS_CODEGEN (codegen));
  bfc_codegen_get_instance_private (codegen)->strict = strict;
}

gboolean
(bfc_codegen_get_strict) (gpointer codegen)
{
  g_return_val_if_fail (BFC_IS_CODEGEN (codegen), FALSE);
return bfc_codegen_get_instance_private (codegen)->strict;
}

gboolean
(bfc_codegen_dump) (gpointer codegen, gpointer abfd, GError** error)
{
  g_return_val_if_fail (BFC_IS_CODEGEN (codegen), FALSE);
  g_return_val_if_fail (abfd != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  BfcCodegenClass* klass = BFC_CODEGEN_GET_CLASS (codegen);
return klass->dump (codegen, abfd, error);
}

guint
(bfc_codegen_consume) (gpointer codegen, gconstpointer buffer, gsize size, GError** error)
{
  g_return_val_if_fail (BFC_IS_CODEGEN (codegen), FALSE);
  g_return_val_if_fail (buffer != NULL || size == 0, FALSE);
  g_return_val_if_fail (size < G_MAXSSIZE, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  BfcCodegenClass* klass = BFC_CODEGEN_GET_CLASS (codegen);
return klass->consume (codegen, buffer, size, error);
}


void
(bfc_codegen_freeze) (gpointer codegen, GError** error)
{
  g_return_if_fail (BFC_IS_CODEGEN (codegen));
  g_return_if_fail (error == NULL || *error == NULL);
  BfcCodegenPrivate* priv = bfc_codegen_get_instance_private (codegen);
  if (priv->freeze == FALSE)
    {
      priv->freeze = TRUE;
      BfcCodegenClass* klass = BFC_CODEGEN_GET_CLASS (codegen);
      klass->freeze (codegen, error);
    }
}

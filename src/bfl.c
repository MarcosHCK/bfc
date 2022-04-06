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
#include <bfd_error.h>
#include <bfdlink.h>
#include <codegen.h>
#include <gio/gio.h>
#include <options.h>

G_DEFINE_QUARK
(bfc-linker-error-quark,
 bfc_linker_error);

typedef enum
{
  BFC_LINKER_ERROR_FAILED,
  BFC_LINKER_ERROR_RUNTIME,
} BfcLinkerError;

#define BFC_LINKER_ERROR (bfc_linker_error_quark ())
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
#define _bfd_close0(var) ((var == NULL) ? NULL : (var = (bfd_close (var), NULL)))
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))

int
bfl_main (BfcOptions* options, GError** error)
{
  bfd* lbfd = NULL;
  bfd* abfd = NULL;
  guint i;

  {
    GEnumClass* klass = NULL;
    GEnumValue* value = NULL;
    gchar* libname = NULL;
    guint i;

    klass = g_type_class_ref (bfc_codegen_arch_get_type ());

    for (i = 0; i < klass->n_values; i++)
      {
        value = & klass->values[i];
        if (!g_strcmp0 (value->value_nick, options->arch))
        {
          libname = g_strconcat ("libstd-", value->value_nick, ".a", NULL);
          break;
        }
      }

    g_type_class_unref (klass);

    if (libname == NULL)
      {
        g_warning ("Invalid architecture type '%s'", options->arch);
        return -1;
      }

    {
      gchar* saname = NULL;

      saname = libname;
#if DEVELOPER == 1
      const gchar* ins = "src/backend/.libs/";
      libname = g_build_filename (ABSTOPBUILDDIR, ins, libname, NULL);
#else // !DEVELOPER
      libname = g_build_filename (PKGLIBDIR, libname, NULL);
#endif // DEVELOPER
      g_free (saname);
    }

#undef throw_action
#define throw_action \
  G_STMT_START { \
    _bfd_close0 (lbfd); \
    _bfd_close0 (abfd); \
    return -1; \
  } G_STMT_END

    lbfd = bfd_openr (libname, "default");
    _g_free0 (libname);
    if (G_UNLIKELY (lbfd == NULL))
      {
        enum bfd_error err = bfd_get_error ();
        const gchar* msg = bfd_strerror (err);

        g_set_error
        (error,
         BFC_LINKER_ERROR,
         BFC_LINKER_ERROR_RUNTIME,
         "can't open runtime library bfd object: %s",
         msg);
        throw_action;
      }

    if (!bfd_check_format (lbfd, bfd_archive))
      {
        enum bfd_error err = bfd_get_error ();
        const gchar* msg = bfd_strerror (err);

        g_set_error
        (error,
         BFC_LINKER_ERROR,
         BFC_LINKER_ERROR_RUNTIME,
         "targeted runtime library is not correct: %s",
         msg);
        throw_action;
      }
  }

  _bfd_close0 (lbfd);
  _bfd_close0 (abfd);
return 0;
}

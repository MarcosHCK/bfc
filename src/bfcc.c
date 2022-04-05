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
#include <codegen.h>
#include <gio/gio.h>
#include <options.h>

#ifdef G_OS_WIN32
# include <gio-win32-2.0/gio/gwin32inputstream.h>
# include <gio-win32-2.0/gio/gwin32outputstream.h>
#endif // G_OS_WIN32
#ifdef G_OS_UNIX
# include <gio-unix-2.0/gio/gunixinputstream.h>
# include <gio-unix-2.0/gio/gunixoutputstream.h>
# include <gio-unix-2.0/gio/gfiledescriptorbased.h>
#endif // G_OS_UNIX

#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
#define _bfd_close0(var) ((var == NULL) ? NULL : (var = (bfd_close (var), NULL)))
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))

int
bfcc_main (BfcOptions* options, GError** error)
{
  BfcCodegen* codegen = NULL;
  GInputStream* stream = NULL;
  GError* tmp_err = NULL;
  gchar* buffer = NULL;
  gsize bufsz = 256;
  gssize read = 0;

  if (options->n_inputs > 1)
    {
      g_warning ("Single files only, use bfc for multiple ones");
      return -1;
    }

  {
    GEnumClass* klass = NULL;
    GEnumValue* value = NULL;
    guint i;

    klass = g_type_class_ref (bfc_codegen_arch_get_type ());
    for (i = 0; i < klass->n_values; i++)
      {
        value = & klass->values[i];
        if (!g_strcmp0 (value->value_nick, options->arch))
        {
          codegen = bfc_codegen_new (value->value);
          bfc_codegen_set_strict (codegen, options->strictcode);
          break;
        }
      }
    if (codegen == NULL)
      {
        g_warning ("Invalid architecture type '%s'", options->arch);
        return -1;
      }
  }

  buffer = g_malloc (bufsz);
  stream = options->inputs[0];

  do
  {
    read =
    g_input_stream_read (stream, buffer, bufsz, NULL, &tmp_err);
    if (G_UNLIKELY (tmp_err != NULL))
      {
        g_propagate_error (error, tmp_err);
        bfc_codegen_unref (codegen);
        g_free (buffer);
        return -1;
      }
    
    g_assert (read >= 0);
    bfc_codegen_consume (codegen, buffer, read, &tmp_err);

    if (G_UNLIKELY (tmp_err != NULL))
      {
        g_propagate_error (error, tmp_err);
        bfc_codegen_unref (codegen);
        g_free (buffer);
        return -1;
      }
  } while (read != 0);

  g_free (buffer);

  bfc_codegen_freeze (codegen, &tmp_err);
  if (G_UNLIKELY (tmp_err != NULL))
    {
      g_propagate_error (error, tmp_err);
      bfc_codegen_unref (codegen);
    }

  {
    bfd* abfd = NULL;
    GOutputStream* stream = NULL;
    stream = options->output;

#undef throw_action
#define throw_action \
  G_STMT_START { \
    bfc_codegen_unref (codegen); \
    return -1; \
  } G_STMT_END

#if defined(G_OS_UNIX)
    if (G_IS_UNIX_OUTPUT_STREAM (stream))
      {
        const gchar* name = NULL;
        gint fd = -1;

        name = bfc_options_get_stream_filename (stream);
        fd = g_file_descriptor_based_get_fd ((gpointer) stream);
        abfd = bfd_fdopenw (name, options->target, fd);

        if (G_UNLIKELY (abfd == NULL))
          {
            enum bfd_error err = bfd_get_error ();
            const gchar* msg = bfd_strerror (err);
            throw (BFC_CODEGEN_ERROR_FAILED, msg);
          }

        g_object_set (stream, "close-fd", FALSE, NULL);
        g_set_object (&options->output, g_memory_output_stream_new_resizable ());
      }
    else
    if (G_IS_FILE_OUTPUT_STREAM (stream))
      {
        GFileInfo* info = NULL;
        const gchar* name = NULL;
        gchar* fullname = NULL;

        info =
        g_file_output_stream_query_info
        ((gpointer) stream,
         G_FILE_ATTRIBUTE_STANDARD_NAME,
         NULL,
         &tmp_err);
        if (G_UNLIKELY (tmp_err != NULL))
          {
            g_propagate_error (error, tmp_err);
            bfc_codegen_unref (codegen);
            _g_object_unref0 (info);
            return -1;
          }

        name = g_file_info_get_name (info);
        name = (name) ? name : bfc_options_get_stream_filename (stream);
        fullname = g_strdup (name);
        _g_object_unref0 (info);

        g_set_object (&options->output, g_memory_output_stream_new_resizable ());

        abfd = bfd_openw (fullname, options->target);
        _g_free0 (fullname);
        if (G_UNLIKELY (abfd == NULL))
          {
            enum bfd_error err = bfd_get_error ();
            const gchar* msg = bfd_strerror (err);
            throw (BFC_CODEGEN_ERROR_FAILED, msg);
          }
      }
#endif // G_OS_UNIX
#if defined(G_OS_WINDOWS)
  #error "I had not idea"
#endif // G_OS_WINDOWS

    g_assert (abfd != NULL);
    bfd_set_format (abfd, bfd_object);
    bfd_make_writable (abfd);

    {
      const bfd_arch_info_type* arch = NULL;
      BfcCodegenClass* klass = NULL;

      klass = BFC_CODEGEN_GET_CLASS (codegen);
      arch = bfd_scan_arch (klass->bfd_arch);
      if (G_UNLIKELY (arch == NULL))
        {
          g_set_error_literal
          (error,
           BFC_CODEGEN_ERROR,
           BFC_CODEGEN_ERROR_FAILED,
           "unsupported arch");
          bfc_codegen_unref (codegen);
          _bfd_close0 (abfd);
          return -1;
        }

#undef throw_action
#define throw_action \
  G_STMT_START { \
    bfc_codegen_unref (codegen); \
    _bfd_close0 (abfd); \
    return -1; \
  } G_STMT_END

      if (!bfd_set_format (abfd, bfd_object))
        {
          enum bfd_error err = bfd_get_error ();
          const gchar* msg = bfd_strerror (err);
          throw (BFC_CODEGEN_ERROR_FAILED, msg);
        }

      if (!bfd_set_arch_mach (abfd, arch->arch, arch->mach))
        {
          enum bfd_error err = bfd_get_error ();
          const gchar* msg = bfd_strerror (err);
          throw (BFC_CODEGEN_ERROR_FAILED, msg);
        }

      guint
      flags = HAS_SYMS | WP_TEXT | D_PAGED | BFD_LINKER_CREATED;
      flags &= bfd_applicable_file_flags (abfd);
      if (!bfd_set_file_flags (abfd, flags))
        {
          enum bfd_error err = bfd_get_error ();
          const gchar* msg = bfd_strerror (err);
          throw (BFC_CODEGEN_ERROR_FAILED, msg);
        }

      if (!bfd_set_format (abfd, bfd_object))
        {
          enum bfd_error err = bfd_get_error ();
          const gchar* msg = bfd_strerror (err);
          throw (BFC_CODEGEN_ERROR_FAILED, msg);
        }
    }

    bfc_codegen_dump (codegen, abfd, &tmp_err);
    if (G_UNLIKELY (tmp_err != NULL))
      {
        g_propagate_error (error, tmp_err);
        bfc_codegen_unref (codegen);
        _bfd_close0 (abfd);
        return -1;
      }

    if (bfd_close (abfd) == 0)
      {
        enum bfd_error err = bfd_get_error ();
        const gchar* msg = bfd_strerror (err);
        throw (BFC_CODEGEN_ERROR_FAILED, msg);
      }
  }

  bfc_codegen_unref (codegen);
return 0;
}

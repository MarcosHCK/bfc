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
#include <gio/gio.h>
#include <options.h>

G_DEFINE_QUARK
(bfc-options-filename-quark,
 bfc_options_filename);

#ifdef G_OS_WIN32
# include <gio-win32-2.0/gio/gwin32inputstream.h>
# include <gio-win32-2.0/gio/gwin32outputstream.h>
#endif // G_OS_WIN32
#ifdef G_OS_UNIX
# include <gio-unix-2.0/gio/gunixinputstream.h>
# include <gio-unix-2.0/gio/gunixoutputstream.h>
#endif // G_OS_UNIX

#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))

static gpointer
open_stream (const gchar* filename, BfcOpenMode mode, GError** error)
{
  if (!g_strcmp0 (filename, "-"))
  switch (mode)
  {
  case BFC_OPEN_READ:
#ifdef G_OS_WIN32
    return g_win32_input_stream_new ((void*) STD_INPUT_HANDLE, FALSE);
#endif // G_OS_WIN32
#ifdef G_OS_UNIX
    return g_unix_input_stream_new (STDIN_FILENO, FALSE);
#endif // G_OS_UNIX
  case BFC_OPEN_WRITE:
#ifdef G_OS_WIN32
    return g_win32_output_stream_new ((void*) STD_OUTPUT_HANDLE, FALSE);
#endif // G_OS_WIN32
#ifdef G_OS_UNIX
    return g_unix_output_stream_new (STDOUT_FILENO, FALSE);
#endif // G_OS_UNIX
  default:
#if DEVELOPER == 1
    g_assert_not_reached();
#endif // DEVELOPER
  }

  GFile* file = NULL;
  gpointer stream = NULL;
  GError* tmp_err = NULL;

  file = g_file_new_for_commandline_arg (filename);
  switch (mode)
  {
  case BFC_OPEN_READ:
    stream = g_file_read (file, NULL, &tmp_err);
    break;
  case BFC_OPEN_WRITE:
    stream = g_file_replace (file, NULL, FALSE, 0, NULL, &tmp_err);
    break;
  default:
#if DEVELOPER == 1
    g_assert_not_reached();
#endif // DEVELOPER
  }

  _g_object_unref0 (file);

  if (G_UNLIKELY (tmp_err != NULL))
    {
      g_propagate_error (error, tmp_err);
      _g_object_unref0 (stream);
      return NULL;
    }
return stream;
}

gpointer
bfc_options_open (const gchar* filename, BfcOpenMode mode, GError** error)
{
  gpointer stream = NULL;
  GError* tmp_err = NULL;

  stream = open_stream (filename, mode, &tmp_err);
  if (G_UNLIKELY (tmp_err != NULL))
    {
      g_propagate_error (error, tmp_err);
      return NULL;
    }

  g_object_set_qdata_full
  (G_OBJECT (stream),
   bfc_options_filename_quark (),
   g_strdup (filename),
   g_free);
return stream;
}

const gchar*
bfc_options_get_stream_filename (gpointer stream)
{
  return (const gchar*)
  g_object_get_qdata
  (G_OBJECT (stream),
   bfc_options_filename_quark ());
}

void
bfc_options_emit (BfcOptions* options, GString* string)
{
  const gchar* filename = NULL;
  guint i;

  if (options->dontlink)
    g_string_append (string, "-c ");

  if (options->output)
    {
      filename =
      bfc_options_get_stream_filename (options->output);
#if DEVELOPER == 1
      g_assert (filename != NULL);
 #endif // DEVELOPER
      g_string_append_printf (string, "-o '%s' ", filename);
    }
  else
    {
      g_string_append (string, "-o - ");
    }

  g_string_append (string, "-- ");

  for (i = 0; i < options->n_inputs; i++)
    {
      if (options->inputs[i])
        {
          filename =
          bfc_options_get_stream_filename (options->inputs[i]);
#if DEVELOPER == 1
          g_assert (filename != NULL);
#endif // DEVELOPER
          g_string_append_printf (string, "'%s' ", filename);
        }
      else
        {
          g_string_append (string, "- ");
        }
    }
}

static void
close_stream (gpointer stream)
{
  GError* tmp_err = NULL;
  if (G_IS_INPUT_STREAM (stream))
    g_input_stream_close (stream, NULL, &tmp_err);
  else if (G_IS_OUTPUT_STREAM (stream))
    g_output_stream_close (stream, NULL, &tmp_err);
#if DEVELOPER == 1
  else g_assert_not_reached ();
#endif // DEVELOPER
  if (G_UNLIKELY (tmp_err != NULL))
    {
      g_warning
      ("(%s: %i): %s: %i: %s",
       G_STRFUNC,
       __LINE__,
       g_quark_to_string
       (tmp_err->domain),
       tmp_err->code,
       tmp_err->message);
      g_error_free (tmp_err);
    }
  _g_object_unref0 (stream);
}

void
bfc_options_clear (BfcOptions* options)
{
  guint i;
  if (G_LIKELY (options->output != NULL))
    close_stream (options->output);
  for (i = 0; i < options->n_inputs; i++)
    close_stream (options->inputs[i]);
  _g_free0 (options->inputs);
}

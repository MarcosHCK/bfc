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

static const gchar* description = {"Description"};
static const gchar* summary = {"Summary"};

#ifdef G_OS_WIN32
# include <gio-win32-2.0/gio/gwin32inputstream.h>
#endif // G_OS_WIN32
#ifdef G_OS_UNIX
# include <gio-unix-2.0/gio/gunixinputstream.h>
#endif // G_OS_UNIX

#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))

static gpointer
try_open (const gchar* filename, gchar mode, GError** error)
{
  if (!g_strcmp0 (filename, "-"))
    {
      if (mode == 'r')
#ifdef G_OS_WIN32
  return g_win32_input_stream_new ((void*) STD_INPUT_HANDLE, FALSE);
#endif // G_OS_WIN32
#ifdef G_OS_UNIX
  return g_unix_input_stream_new (STDIN_FILENO, FALSE);
#endif // G_OS_UNIX
      else if (mode == 'w')
#ifdef G_OS_WIN32
  return g_win32_input_stream_new ((void*) STD_OUTPUT_HANDLE, FALSE);
#endif // G_OS_WIN32
#ifdef G_OS_UNIX
  return g_unix_input_stream_new (STDOUT_FILENO, FALSE);
#endif // G_OS_UNIX
#if DEVELOPER == 1
      else g_assert_not_reached ();
#endif // DEVELOPER
    }

  GFile* file = NULL;
  gpointer stream = NULL;
  GError* tmp_err = NULL;

  file = g_file_new_for_commandline_arg (filename);
  if (mode == 'r')
    stream = g_file_read (file, NULL, &tmp_err);
  else if (mode == 'w')
    stream = g_file_replace (file, NULL, FALSE, 0, NULL, &tmp_err);
#if DEVELOPER == 1
  else g_assert_not_reached ();
#endif // DEVELOPER
  _g_object_unref0 (file);

  if (G_UNLIKELY (tmp_err != NULL))
    {
      g_propagate_error (error, tmp_err);
      _g_object_unref0 (stream);
      return NULL;
    }
return stream;
}

static void
try_close (gpointer stream)
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
}

static void
options_free (BfcOptions* options)
{
  guint i;
  if (G_LIKELY (options->output != NULL))
    try_close (options->output);
  for (i = 0; i < options->n_inputs; i++)
    try_close (options->inputs[i]);
  _g_free0 (options->inputs);
}

typedef void (*BfcMain) (BfcOptions* options, GError** error);

void
bfc_main (BfcOptions* options, GError** error);

int
main (int argc, char* argv[])
{
  GOptionContext* context = NULL;
  gboolean success = TRUE;
  GError* tmp_err = NULL;
  BfcOptions options = {0};
  gpointer stream = NULL;
  gchar* argv0 = NULL;
  BfcMain front = NULL;
  guint i;

  const gchar* output = "-";

  GOptionEntry entries[] =
  {
    { "output", 'o', 0, G_OPTION_ARG_FILENAME, &output, "Place the output into <file>", "<file>" },
    { NULL, 0, 0, 0, NULL, NULL, NULL},
  };

  context =
  g_option_context_new (NULL);
  g_option_context_add_main_entries (context, entries, "en_US");
  g_option_context_set_description (context, description);
  g_option_context_set_help_enabled (context, TRUE);
  g_option_context_set_ignore_unknown_options (context, FALSE);
  g_option_context_set_strict_posix (context, FALSE);
  g_option_context_set_summary (context, summary);
  g_option_context_set_translation_domain (context, "en_US");

  success =
  g_option_context_parse (context, &argc, &argv, &tmp_err);
  if (G_UNLIKELY (tmp_err != NULL))
    {
      g_critical
      ("(%s: %i): %s: %i: %s",
       G_STRFUNC,
       __LINE__,
       g_quark_to_string
       (tmp_err->domain),
       tmp_err->code,
       tmp_err->message);
      g_option_context_free (context);
      g_error_free (tmp_err);
      return -1;
    }
  g_option_context_free (context);

  {
    if (1 >= argc)
    {
      g_critical ("(%s: %i): No input files!", G_STRFUNC, __LINE__);
      return 0;
    }

    argv0 = g_path_get_basename (argv[0]);
    if (!g_strcmp0 (argv0, "bfc"))
      front = bfc_main;
    else
      front = bfc_main;

    stream = try_open (output, 'w', &tmp_err);
    if (G_UNLIKELY (tmp_err != NULL))
      {
        g_critical
        ("(%s: %i): %s: %i: %s",
         G_STRFUNC,
         __LINE__,
         g_quark_to_string
         (tmp_err->domain),
         tmp_err->code,
         tmp_err->message);
        g_error_free (tmp_err);
        return -1;
      }
    else
      {
        options.output = stream;
      }

    options.inputs = g_new (GInputStream*, argc - 1);

    for (i = 0; i < argc - 1; i++)
    {
      stream = try_open (argv[i + 1], 'r', &tmp_err);
      if (G_UNLIKELY (tmp_err != NULL))
        {
          g_critical
          ("(%s: %i): %s: %i: %s",
           G_STRFUNC,
           __LINE__,
           g_quark_to_string
           (tmp_err->domain),
           tmp_err->code,
           tmp_err->message);
          g_error_free (tmp_err);
          options_free (&options);
          return -1;
        }
      else
        {
          options.inputs[i] = stream;
          options.n_inputs++;
        }
    }
  }

  g_assert (front != NULL);

  front (&options, &tmp_err);
  if (G_UNLIKELY (tmp_err != NULL))
    {
      g_critical
      ("(%s: %i): %s: %i: %s",
       G_STRFUNC,
       __LINE__,
       g_quark_to_string
       (tmp_err->domain),
       tmp_err->code,
       tmp_err->message);
      g_error_free (tmp_err);
      options_free (&options);
      return -1;
    }

  options_free (&options);
return 0;
}

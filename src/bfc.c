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
#include <bfc.h>
#include <collect.h>
#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>

#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))

#ifdef G_OS_WIN32
# include <gio-win32-2.0/gio/gwin32inputstream.h>
# include <gio-win32-2.0/gio/gwin32outputstream.h>
#endif // G_OS_WIN32
#ifdef G_OS_UNIX
# include <gio-unix-2.0/gio/gunixinputstream.h>
# include <gio-unix-2.0/gio/gunixoutputstream.h>
#endif // G_OS_UNIX

static void
_open_output (BfcStream* s, const gchar* filename, GError** error)
{
  GFile* file = NULL;
  gboolean stdpip = FALSE;
  gpointer stream = NULL;
  GError* tmperr = NULL;

  if (!g_strcmp0 (filename, "-"))
  {
    stdpip = TRUE;
#ifdef G_OS_WIN32
    stream = g_win32_output_stream_new ((void*) STD_OUTPUT_HANDLE, FALSE);
#endif // G_OS_WIN32
#ifdef G_OS_UNIX
    stream = g_unix_output_stream_new (STDOUT_FILENO, FALSE);
#endif // G_OS_UNIX
  }
  else
  {
    file = g_file_new_for_commandline_arg (filename);
    stream = g_file_replace (file, NULL, 0, 0, NULL, &tmperr);
    _g_object_unref0 (file);
  }

  if (G_UNLIKELY (tmperr != NULL))
  {
    g_propagate_error (error, tmperr);
    _g_object_unref0 (stream);
  }
  else
  {
    s->filename = filename;
    s->stream = g_data_output_stream_new (stream);
    g_filter_output_stream_set_close_base_stream (s->stream, !stdpip);
    _g_object_unref0 (stream);
  }
}

static void
_open_input (BfcStream* s, const gchar* filename, GError** error)
{
  GFile* file = NULL;
  gboolean stdpip = FALSE;
  gpointer stream = NULL;
  GError* tmperr = NULL;

  if (!g_strcmp0 (filename, "-"))
  {
    stdpip = TRUE;
#ifdef G_OS_WIN32
    stream = g_win32_input_stream_new ((void*) STD_INPUT_HANDLE, FALSE);
#endif // G_OS_WIN32
#ifdef G_OS_UNIX
    stream = g_unix_input_stream_new (STDIN_FILENO, FALSE);
#endif // G_OS_UNIX
  }
  else
  {
    file = g_file_new_for_commandline_arg (filename);
    stream = g_file_read (file, NULL, &tmperr);
    _g_object_unref0 (file);
  }

  if (G_UNLIKELY (tmperr != NULL))
  {
    g_propagate_error (error, tmperr);
    _g_object_unref0 (stream);
  }
  else
  {
    s->filename = filename;
    s->stream = g_data_input_stream_new (stream);
    g_filter_input_stream_set_close_base_stream (s->stream, !stdpip);
    _g_object_unref0 (stream);
  }
}

static void
_open_inputs (BfcStream** ss, gchar** filenames, guint n_filenames, GError** error)
{
  BfcStream* _s = g_new0 (BfcStream, n_filenames);
  guint i, got = 0, in = 0;
  GError* tmperr = NULL;

  for (i = 0; i < n_filenames; i++)
  {
    if (!g_strcmp0 (filenames [i], "-"))
    {
      if (!in)
        in = TRUE;
      else
      {
        g_set_error
        (error,
         G_IO_ERROR,
         G_IO_ERROR_FAILED,
         "More than one input from standard input");
        for (i = 0; i < got; i++)
          _g_object_unref0 (_s [i].stream);
          _g_free0 (_s);
          return;
      }
    }

    _open_input (& _s [i], filenames [i], &tmperr);
    if (G_UNLIKELY (tmperr != NULL))
    {
      g_propagate_error (error, tmperr);
      for (i = 0; i < got; i++)
        _g_object_unref0 (_s [i].stream);
        _g_free0 (_s);
        return;
    }
  }

  *ss = _s;
}

typedef struct
{
  GOptionContext* context;
  GOptionGroup* helps;
  GOptionGroup* mains;
  GOptionGroup* others;
  guint helped : 1;
} HelpData;

static gboolean
dohelp (const gchar* option_name, const gchar* value, gpointer data_, GError** error)
{
  HelpData* data = data_;
      data->helped = 1;

  if (!g_strcmp0 (option_name, "--help")
    || !g_strcmp0 (option_name, "-h"))
  {
    if (value != NULL)
    {
  #define HELPPRE "--help-"
      gsize length = strlen (value) + sizeof (HELPPRE);
                    g_assert (length < 1024);
      gchar* repl = g_alloca (length + 1);
                    g_snprintf (repl, length + 1, HELPPRE "%s", value);
        option_name = repl;
        value = NULL;
  #undef HELPPRE
    }
    else
    {
      gchar* show =
      g_option_context_get_help (data->context, FALSE, data->mains);
      g_print ("%s", show);
      g_free (show);
      return TRUE;
    }
  }

  if (!g_strcmp0 (option_name, "--help-all"))
  {
    gchar* show = g_option_context_get_help (data->context, FALSE, NULL);
        g_print ("%s", show);
        g_free (show);
  }
  else
  if (!g_strcmp0 (option_name, "--help-help"))
  {
    gchar* show = g_option_context_get_help (data->context, FALSE, data->helps);
        g_print ("%s", show);
        g_free (show);
  }
  else
  if (!g_strcmp0 (option_name, "--help-others"))
  {
    gchar* show = g_option_context_get_help (data->context, FALSE, data->others);
        g_print ("%s", show);
        g_free (show);
  }
  else
  if (!g_strcmp0 (option_name, "--help-target"))
  {
    LLVMInitializeAllTargetInfos ();
    GString* string = g_string_sized_new (256);
    LLVMTargetRef ref = LLVMGetFirstTarget ();

    while (ref != NULL)
    {
      g_string_append_printf (string,
        "%s: %s\r\n",
        LLVMGetTargetName (ref),
        LLVMGetTargetDescription (ref));

      ref = LLVMGetNextTarget (ref);
    }

    g_print ("%s", string->str);
    g_string_free (string, TRUE);
  }
  else
  {
    g_set_error
    (error,
     G_OPTION_ERROR,
     G_OPTION_ERROR_UNKNOWN_OPTION,
     "Unknown option %s", option_name);
    return FALSE;
  }
return TRUE;
}

enum
{
  pass_collect_codegen,
  pass_collect_machine,
  pass_open_inputs,
  pass_open_output,
  pass_codegen,
  pass_close_inputs,
  pass_flush_output,
  pass_close_output,
  pass_max,
};

int
main (int argc, char* argv[])
{
  GOptionContext* context = NULL;
  GOptionGroup* group = NULL;
  GError* tmperr = NULL;

  gchar* basename = argv [0];
  GString* description = g_string_sized_new (64);
  GString* summary = g_string_sized_new (64);
  HelpData helpdata = {0};

  gint olevel = 2;
  gsize beltsz = 1024;
  gboolean assemble = FALSE;
  gboolean compile = FALSE;
  gboolean checkio = TRUE;
  gboolean emitll = FALSE;
  gboolean static_ = FALSE;
  gboolean strict = FALSE;
  gboolean fpic = FALSE;
  gboolean fpie = FALSE;
  gboolean fPIC = FALSE;
  gboolean fPIE = FALSE;
  const gchar* mmodel = "default";
  const gchar* arch = NULL;
  const gchar* features = NULL;
  const gchar* output = "a.out";
  const gchar* tune = NULL;

  const GOptionEntry helps[] =
  {
    { "help", 'h', G_OPTION_FLAG_OPTIONAL_ARG, G_OPTION_ARG_CALLBACK, dohelp, "Show help actions", NULL, },
    { "help-all", 0, G_OPTION_FLAG_OPTIONAL_ARG, G_OPTION_ARG_CALLBACK, dohelp, "Show all help actions", NULL, },
    { "help-target", 0, G_OPTION_FLAG_OPTIONAL_ARG, G_OPTION_ARG_CALLBACK, dohelp, "Show targets descriptions", NULL, },
    { "help-others", 0, G_OPTION_FLAG_OPTIONAL_ARG, G_OPTION_ARG_CALLBACK, dohelp, "Show other options", NULL, },
    { NULL, 0, 0, 0, NULL, NULL, NULL, },
  };

  const GOptionEntry mains[] =
  {
    { "arch", 0, 0, G_OPTION_ARG_STRING, &arch, "Generate code for target <TARGET>", "TARGET", },
    { "assemble", 'S', 0, G_OPTION_ARG_NONE, &assemble, "Assemble only; do not compile or link", NULL, },
    { "compile", 'c', 0, G_OPTION_ARG_NONE, &compile, "Compile only; do not assemble or link", NULL, },
    { "emit-llvm", 0, 0, G_OPTION_ARG_NONE, &emitll, "Emit LLVM IR code (human readable format)", NULL, },
    { "features", 'F', 0, G_OPTION_ARG_STRING, &features, "Specify target-specific features to <FEATURES>", "FEATURES", },
    { "optimize", 'O', 0, G_OPTION_ARG_INT, &olevel, "Optimize code as <LEVEL> strong", "LEVEL", },
    { "output", 'o', 0, G_OPTION_ARG_STRING, &output, "Place the output info <FILE>", "FILE", },
    { "pic", 0, 0, G_OPTION_ARG_NONE, &fpic, "Generate position-independient code if possible (small mode)", NULL, },
    { "pie", 0, 0, G_OPTION_ARG_NONE, &fpie, "Generate position-independient code for executables if possible (small mode)", NULL, },
    { "PIC", 0, 0, G_OPTION_ARG_NONE, &fPIC, "Generate position-independient code if possible (large mode)", NULL, },
    { "PIE", 0, 0, G_OPTION_ARG_NONE, &fPIE, "Generate position-independient code for executables if possible (large mode)", NULL, },
    { "tune", 0, 0, G_OPTION_ARG_STRING, &tune, "Schedule code for cpu <CPU>", "CPU", },
    { "static", 's', 0, G_OPTION_ARG_NONE, &static_, "Do not link against shared libraries", NULL, },
    { "strict", 0, 0, G_OPTION_ARG_NONE, &strict, "Perform strict code parsing", NULL, },
    { NULL, 0, 0, 0, NULL, NULL, NULL, },
  };

  const GOptionEntry others[] =
  {
    { "address-mode", 0, 0, G_OPTION_ARG_STRING, &mmodel, "Use given address mode", NULL, },
    { "belt-size", 0, 0, G_OPTION_ARG_INT, &beltsz, "Override default belt size (in whole units)", NULL, },
    { "check-io", 0, 0, G_OPTION_ARG_NONE, &checkio, "Perform check after every I/O call", NULL, },
    { NULL, 0, 0, 0, NULL, NULL, NULL, },
  };

  helpdata.context = context =
  g_option_context_new ("files ...");
  g_option_context_set_description (context, description->str);
  g_option_context_set_help_enabled (context, FALSE);
  g_option_context_set_ignore_unknown_options (context, FALSE);
  g_option_context_set_strict_posix (context, FALSE);
  g_option_context_set_summary (context, summary->str);
  g_option_context_set_translation_domain (context, "en_US");
  g_string_free (description, TRUE);
  g_string_free (summary, TRUE);

  group =
  g_option_group_new ("help", "Help Options", "Help", &helpdata, NULL);
  g_option_group_add_entries (group, helps);
  g_option_group_set_translation_domain (group, "en_US");
  g_option_context_add_group (context, group);
  helpdata.helps = group;

  group =
  g_option_group_new ("main", "Main Options", "Main", &helpdata, NULL);
  g_option_group_add_entries (group, mains);
  g_option_group_set_translation_domain (group, "en_US");
  g_option_context_add_group (context, group);
  helpdata.mains = group;

  group =
  g_option_group_new ("other", "Other Options", "Other", &helpdata, NULL);
  g_option_group_add_entries (group, others);
  g_option_group_set_translation_domain (group, "en_US");
  g_option_context_add_group (context, group);
  helpdata.others = group;

  g_option_context_parse (context, &argc, &argv, &tmperr);
  g_option_context_free (context);

  if (G_UNLIKELY (tmperr != NULL))
  {
    g_warning
    ("(%s): %s: %i: %s",
     G_STRLOC,
     g_quark_to_string
     (tmperr->domain),
      tmperr->code,
      tmperr->message);
    g_error_free (tmperr);
    return 0;
  }
  else
  {
    if (helpdata.helped)
      return 0;

    BfcOptions opt = {0};
      opt.assemble = assemble;
      opt.checkio = checkio;
      opt.compile = compile;
      opt.emitll = emitll;
      opt.olevel = olevel;
      opt.static_ = static_;
      opt.strict = strict;
      opt.beltsz = beltsz;

    int i, j;
    for (i = 0; i < pass_max; i++)
    {
      switch (i)
      {
        case pass_collect_codegen:
          {
            guint pic = COLLECT_PIC (fpic, fPIC);
            guint pie = COLLECT_PIE (fpie, fPIE);
            collect_codegen (&opt, static_, pic, pie, mmodel, &tmperr);
          }
          goto check;
        case pass_collect_machine:
          collect_machine (&opt, arch, tune, features, &tmperr);
          goto check;
        case pass_open_inputs:
          opt.n_inputs = argc - 1;
          _open_inputs (& opt.inputs, & argv [1], opt.n_inputs, &tmperr);
          goto check;
        case pass_open_output:
          _open_output (& opt.output, output, &tmperr);
          goto check;
        case pass_codegen:
          bfc_main (&opt, &tmperr);
          LLVMDisposeTargetMachine (opt.machine);
          goto check;
        case pass_close_inputs:
          for (j = 0; j < opt.n_inputs; ++j)
          {
            g_input_stream_close (opt.inputs [j].stream, NULL, &tmperr);
            if (G_UNLIKELY (tmperr != NULL))
              break;
          }
            g_free (opt.inputs);
          goto check;
        case pass_flush_output:
          g_output_stream_flush (opt.output.stream, NULL, &tmperr);
          goto check;
        case pass_close_output:
          g_output_stream_close (opt.output.stream, NULL, &tmperr);
          goto check;

        check:
          if (G_UNLIKELY (tmperr != NULL))
          {
            g_warning
            ("(%s): %s: %i: %s",
             G_STRLOC,
             g_quark_to_string
             (tmperr->domain),
              tmperr->code,
              tmperr->message);
            g_error_free (tmperr);
            return -1;
          }
          break;
      }
    }
  }
return 0;
}

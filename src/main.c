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
#include <gio/gio.h>
#include <options.h>
#include <bfd.h>

#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))

typedef int (*BfcMain) (BfcOptions* options, GError** error);

int
bfc_main (BfcOptions* options, GError** error);
int
bfcc_main (BfcOptions* options, GError** error);
int
bfl_main (BfcOptions* options, GError** error);

int
main (int argc, char* argv[])
{
  GOptionContext* context = NULL;
  GError* tmp_err = NULL;
  BfcOptions options = {0};
  gpointer stream = NULL;
  gchar* description = NULL;
  gchar* summary = NULL;
  gchar* argv0 = NULL;
  BfcMain front = NULL;
  int result = 0;
  guint i;

  bfd_init ();

  const gchar* output = "-";
  const gchar* arch = "x86_64";
  const gchar* target = "default";

  /*
   *
   *
   */

  argv0 =
  g_path_get_basename (argv[0]);

  if (!g_strcmp0 (argv0, "bfc"))
    front = bfc_main;
  else
  if (!g_strcmp0 (argv0, "bfcc"))
    front = bfcc_main;
  else
  if (!g_strcmp0 (argv0, "bfl"))
    front = bfl_main;
  else
    front = bfc_main;

  /*
   *
   *
   */

  const GOptionEntry entries[] =
  {
    { "output", 'o', 0, G_OPTION_ARG_FILENAME, &output, "Place the output into <file>", "<file>" },
    { "architecture", 'A', 0, G_OPTION_ARG_STRING, &arch, "Set architecture", "ARCH" },
    { "oformat", '\0', 0, G_OPTION_ARG_STRING, &target, "Specify target of output file", "FORMAT" },
    { "strict", '\0', 0, G_OPTION_ARG_NONE, &(options.strictcode), "Enforce code strictness", NULL },
    { NULL, 0, 0, 0, NULL, NULL, NULL},
  };

  {
    GString* str = g_string_sized_new (64);

    {
      GEnumClass* klass = NULL;
      GEnumValue* value = NULL;
      GType g_type;
      guint i;

      g_type = bfc_codegen_arch_get_type ();
      klass = g_type_class_ref (g_type);
      g_string_append (str, "Arch: ");

      for (i = 0; i < klass->n_values; i++)
        {
          value = &(klass->values[i]);

          if (i > 0)
          g_string_append_c (str, ' ');
          g_string_append (str, value->value_nick);
        }

      g_string_append (str, "\r\n");
      g_type_class_unref (klass);
    }

    {
      g_string_append (str, "Targets: ");
      const gchar** targets = bfd_target_list ();
      guint i;

      for (i = 0; targets[i]; i++)
        {
          if (i > 0)
          g_string_append_c (str, ' ');
          g_string_append (str, targets[i]);
        }

      g_string_append (str, "\r\n");
    }

    description =
    g_string_free (str, FALSE);
  }

  {
    GString* str = g_string_sized_new (64);
    g_string_append (str, "Copyright 2021-2025 MarcosHCK");

    summary =
    g_string_free (str, FALSE);
  }

  context =
  g_option_context_new (NULL);
  g_option_context_add_main_entries (context, entries, "en_US");
  g_option_context_set_description (context, description);
  g_option_context_set_help_enabled (context, TRUE);
  g_option_context_set_ignore_unknown_options (context, FALSE);
  g_option_context_set_strict_posix (context, FALSE);
  g_option_context_set_summary (context, summary);
  g_option_context_set_translation_domain (context, "en_US");
  _g_free0 (description);
  _g_free0 (summary);

  /*
   *
   *
   */

    if (front == bfc_main)
      {
        const GOptionEntry entries[] =
        {
          { "compile", 'c', 0, G_OPTION_ARG_NONE, &(options.dontlink), "Compile only; do not assemble or link", "<file>" },
          { NULL, 0, 0, 0, NULL, NULL, NULL},
        };

        g_option_context_add_main_entries (context, entries, "en_US");
      }

  /*
   *
   *
   */

  g_option_context_parse (context, &argc, &argv, &tmp_err);
  g_option_context_free (context);

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

  /*
   *
   *
   */

  if (1 >= argc)
    {
      g_critical ("(%s: %i): No input files!", G_STRFUNC, __LINE__);
      return 0;
    }

  stream = bfc_options_open (output, BFC_OPEN_WRITE, &tmp_err);
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
  options.arch = (gchar*) arch;
  options.target = (gchar*) target;

  for (i = 0; i < argc - 1; i++)
    {
      stream = bfc_options_open (argv[i + 1], BFC_OPEN_READ, &tmp_err);
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
          bfc_options_clear (&options);
          return -1;
        }
      else
        {
          options.inputs[i] = stream;
          options.n_inputs++;
        }
    }

  /*
   *
   *
   */

  g_assert (front != NULL);

  result = front (&options, &tmp_err);
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
      bfc_options_clear (&options);
      return -1;
    }

  /*
   *
   *
   */

  bfc_options_clear (&options);
return result;
}

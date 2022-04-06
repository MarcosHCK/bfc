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

#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))
#define _g_close0(var) ((var == -1) ? -1 : (var = (close (var), -1)))

int
bfcc_main (BfcOptions* options, GError** error);
int
bfl_main (BfcOptions* options, GError** error);

static int
invoke_bfcc (BfcOptions* options, GError** error)
{
return bfcc_main (options, error);
}

static int
invoke_bfl (BfcOptions* options, GError** error)
{
return bfl_main (options, error);
}

int
bfc_main (BfcOptions* options, GError** error)
{
  GError* tmp_err = NULL;
  BfcOptions options2 = {0};
  gchar** vector = NULL;

  if (options->dontlink && options->n_inputs > 1)
    {
      g_warning ("cannot specify '-o' with '-c' with multiple files");
      return -1;
    }

  memcpy (&options2, options, sizeof (options2));
  options2.n_inputs = 0;
  options2.inputs = NULL;
  options2.output = NULL;

  if (!options->dontlink)
    {
      guint ni = options->n_inputs;
      gpointer stream = NULL;
      gchar** names = NULL;
      gchar* name = NULL;
      gint result = 0;
      guint i;

      names = g_new (gchar*, ni);
      for (i = 0; i < ni; i++)
        names[i] = NULL;

      options2.inputs = g_new (GInputStream*, 1);

      for (i = 0; i < ni; i++)
      {
        gint fd =
        g_file_open_tmp ("bfc-XXXXXX", &name, &tmp_err);
        _g_close0 (fd);

        if (G_UNLIKELY (tmp_err != NULL))
          {
            g_propagate_error (error, tmp_err);
            bfc_options_clear (&options2);
            g_strfreev (names);
            _g_free0 (name);
            return -1;
          }

        names[i] = name;

        stream =
        bfc_options_open (name, BFC_OPEN_WRITE, &tmp_err);
        if (G_UNLIKELY (tmp_err != NULL))
          {
            g_propagate_error (error, tmp_err);
            bfc_options_clear (&options2);
            g_strfreev (names);
            return -1;
          }

        options2.inputs[0] = g_object_ref (options->inputs[i]);
        options2.output = stream;

        result =
        invoke_bfcc (&options2, &tmp_err);
        if (G_UNLIKELY (tmp_err != NULL || result != 0))
          {
            if (tmp_err != NULL)
            g_propagate_error (error, tmp_err);
            bfc_options_clear (&options2);
            g_strfreev (names);
            return -1;
          }

        g_clear_object (& options2.inputs[0]);
        g_clear_object (& options2.output);
      }

      _g_free0 (options2.inputs);

      {
        options2.output = g_object_ref (options->output);
        options2.inputs = g_new (GInputStream*, ni);

        for (i = 0; i < ni; i++)
          options2.inputs[i] = NULL;

        for (i = 0; i < ni; i++)
        {
          stream =
          bfc_options_open (name, BFC_OPEN_WRITE, &tmp_err);
          if (G_UNLIKELY (tmp_err != NULL))
            {
              g_propagate_error (error, tmp_err);
              bfc_options_clear (&options2);
              g_strfreev (names);
              return -1;
            }
          else
            {
              options2.inputs[i] = stream;
            }
        }

        result =
        invoke_bfl (&options2, &tmp_err);
        if (G_UNLIKELY (tmp_err != NULL || result != 0))
          {
            if (tmp_err != NULL)
            g_propagate_error (error, tmp_err);
            bfc_options_clear (&options2);
            g_strfreev (names);
            return -1;
          }
      }

      for (i = 0; i < ni; i++)
      {
        g_clear_object (& options2.inputs[i]);

        GFile* file = g_file_new_for_path (names[i]);
        g_file_delete (file, NULL, &tmp_err);
        _g_object_unref0 (file);

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

      bfc_options_clear (&options2);
      g_strfreev (names);
    }
  else
    {
      return invoke_bfcc (options, error);
    }
return 0;
}

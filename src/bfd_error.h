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
#ifndef __BFC_BFD_ERROR__
#define __BFC_BFD_ERROR__ 1
#include <codegen.h>
#include <bfd.h>
#include <glib.h>

#ifndef throw_action
# define throw_action \
  G_STMT_START { \
    g_assert_not_reached (); \
  } G_STMT_END
#endif // throw_action

#define throw(code,...) \
  G_STMT_START { \
    g_print ("(%s: %i)\r\n", G_STRLOC, __LINE__); \
    g_set_error \
    (error, \
     BFC_CODEGEN_ERROR, \
     (code), \
     __VA_ARGS__); \
    throw_action; \
  } G_STMT_END

static const gchar*
bfd_strerror (enum bfd_error error)
{
  static char buf[32];
  g_snprintf (buf, sizeof (buf), "bfd error %i", (guint) error);
return bfd_errmsg (error);
}

#endif // __BFC_BFD_ERROR__

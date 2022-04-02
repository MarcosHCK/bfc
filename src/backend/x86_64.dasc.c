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

#include <dynasm.h>
#include <dynasm/dasm_x86.h>

|.arch x64
|.section code
|.globals globl_
|.actionlist actions
|.globalnames globl_names
|.externnames extern_names

const BfcCodegenBackend
backend_x64_64 =
{
  NULL,
  globl__MAX,
  globl__main,
  DASM_MAXSECTION,
  sizeof (guint64),
};

int
backend_x86_64_init (Dst_DECL)
{
  dasm_setup (Dst, actions);
  dasm_growpc (Dst, 0);

|.code
|->_main:
| push rbp
| mov rsp, rbp
}

int
backend_x86_64_fini (Dst_DECL)
{
| leave
| ret
}

int
backend_x86_64_consume (Dst_DECL, BfcCodegenBackend* backend, GError** error)
{
}

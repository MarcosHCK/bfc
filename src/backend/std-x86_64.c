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
#include <stdio.h>
#include <stdlib.h>
#include <x86_64.h>

static uint8_t
std_getc (BfState* state)
{
  return (uint8_t) getc (stdin);
}

static void
std_putc (BfState* state, uint8_t c)
{
  putc ((int) c, stdout);
}

int
main (int argc, char* argv[])
{
  BfState state;
  void* tape = malloc (TAPE_SIZE);
  state.tape = tape;
  state.getc = std_getc;
  state.putc = std_putc;

  _main (&state);
  free (tape);
return 0;
}

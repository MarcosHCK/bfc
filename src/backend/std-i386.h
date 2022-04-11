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
#ifndef __BACKEND_I386__
#define __BACKEND_I386__ 1
#include <stdint.h>

typedef struct _BfState BfState;
static const int TAPE_SIZE = 8192;

struct _BfState
{
  uint8_t *tape;
  uint8_t (*getc)(BfState *state);
  void (*putc)(BfState *state, uint8_t c);
} __attribute__((packed, aligned(1)));

/*
 * Entry point
 *
 */

void _main(BfState *state);

#endif // __BACKEND_I386__

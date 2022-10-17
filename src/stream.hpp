/* Copyright 2021-2025 MarcosHCK
 * This file is part of libabaco.
 *
 * libabaco is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libabaco is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libabaco. If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef __ASM_STREAM__
#define __ASM_STREAM__ 1
#include <llvm/Support/raw_ostream.h>
#include <gio/gio.h>

namespace Bfc
{
  class OStream : public llvm::raw_pwrite_stream
  {
  public:
    OStream (GOutputStream* stream);
    OStream (const OStream& stream);
    ~OStream ();

    virtual void pwrite_impl (const char* buffer, size_t size, uint64_t offset) override;
    virtual void write_impl (const char* Ptr, size_t Size) override;
    virtual uint64_t current_pos () const override;
  private:
    GOutputStream *stream;
  };
}

#endif // __ASM_STREAM__

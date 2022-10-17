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
#include <config.h>
#include <stream.hpp>

using namespace Bfc;

OStream::OStream (GOutputStream* stream)
{
  g_assert (G_IS_SEEKABLE (stream));
  this->stream = g_object_ref (stream);
}

OStream::OStream (const OStream& stream)
  : OStream (stream.stream)
{
}

OStream::~OStream ()
{
  this->flush ();
  g_object_unref (this->stream);
}

void
OStream::pwrite_impl (const char* buffer, size_t size, uint64_t offset)
{
  if (this->tell () != offset)
  {
    this->flush ();

    auto seekable = G_SEEKABLE (stream);
    if (!g_seekable_can_seek (seekable))
      g_error ("Unseekable stream");

    GError* tmperr = NULL;
    g_seekable_seek (seekable, (goffset) offset, G_SEEK_SET, nullptr, &tmperr);
    if (G_UNLIKELY (tmperr != nullptr))
    {
      g_critical
      ("(%s): %s: %i: %s",
       G_STRLOC,
       g_quark_to_string
       (tmperr->domain),
        tmperr->code,
        tmperr->message);
      g_error_free (tmperr);
      g_assert_not_reached ();
    }
  }

  this->write_impl (buffer, size);
}

void
OStream::write_impl (const char* buffer, size_t size)
{
  GError* tmperr = NULL;
  g_output_stream_write_all (stream, buffer, (gsize) size, nullptr, nullptr, &tmperr);
  if (G_UNLIKELY (tmperr != nullptr))
  {
    g_critical
    ("(%s): %s: %i: %s",
     G_STRLOC,
     g_quark_to_string
     (tmperr->domain),
      tmperr->code,
      tmperr->message);
    g_error_free (tmperr);
    g_assert_not_reached ();
  }
}

uint64_t
OStream::current_pos () const
{
  auto seekable = G_SEEKABLE (stream);
  return (uint64_t) g_seekable_tell (seekable);
}

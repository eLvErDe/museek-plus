/*  NewNet - A networking framework in C++
    Copyright (C) 2006-2007 Ingmar K. Steen (iksteen@gmail.com)
    Copyright 2008 little blue poney <lbponey@users.sourceforge.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */

#include "nnbuffer.h"
#include "platform.h"

#define CHUNK_SIZE 8192

NewNet::Buffer::Buffer() : m_Ptr(0), m_Pos(0), m_Count(0), m_Left(0)
{
}

NewNet::Buffer::Buffer(const Buffer & that) : m_Ptr(0), m_Pos(0), m_Count(0), m_Left(0)
{
  append(that.data(), that.count());
}

NewNet::Buffer &
NewNet::Buffer::operator=(const Buffer & that)
{
  m_Left += m_Pos + m_Count;
  m_Pos = m_Count = 0;
  append(that.data(), that.count());
  return *this;
}

NewNet::Buffer::~Buffer()
{
  free(m_Ptr);
}

void
NewNet::Buffer::append(const unsigned char * data, size_t n)
{
  if(m_Left < n)
  {
    if(m_Pos + m_Left >= n)
    {
      memmove(m_Ptr, m_Ptr + m_Pos, m_Count);
      m_Left += m_Pos;
      m_Pos = 0;
    }
    else
    {
      int newSize = (((n + m_Count + m_Pos) / CHUNK_SIZE) + 1) * CHUNK_SIZE;
      unsigned char * newPtr = (unsigned char *)realloc(m_Ptr, newSize);
      assert(newPtr != 0);
      m_Ptr = newPtr;
      m_Left = newSize - m_Count - m_Pos;
    }
  }

  memcpy(m_Ptr + m_Pos + m_Count, data, n);
  m_Count += n;
  m_Left -= n;
}

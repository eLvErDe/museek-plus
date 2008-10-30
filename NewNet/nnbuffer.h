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

#ifndef NEWNET_BUFFER_H
#define NEWNET_BUFFER_H

#include "nnobject.h"
#include <sys/types.h>
#include <assert.h>

namespace NewNet
{
  //! A character buffer class.
  /*! This class provides a simple character buffer that is used by
      ClientSocket to buffer incoming and outgoing network data. */
  class Buffer : public NewNet::Object
  {
  public:
    //! Create an empty buffer.
    /*! Create an empty buffer. */
    Buffer();

    //! Copy an existing buffer.
    /*! Copy an existing buffer. */
    Buffer(const Buffer & that);

    //! Copy an existing buffer.
    /*! Copy an exisiting buffer. */
    Buffer & operator=(const Buffer & that);

    //! Destructor.
    /*! Frees all memory allocated by the buffer. */
    ~Buffer();

    //! Get a pointer to the start of the buffer.
    /*! Get a pointer to the start of the character buffer. */
    unsigned char * data()
    {
      return m_Ptr + m_Pos;
    }

    //! Get a const pointer to the start of the buffer.
    /*! Get a const pointer to the start of the character buffer. */
    const unsigned char * data() const
    {
      return m_Ptr + m_Pos;
    }

    //! Get the number of bytes that are in the buffer.
    /*! Get the number of bytes that are currently stored in the character
        buffer. */
    size_t count() const
    {
      return m_Count;
    }

    //! Determine if the buffer is empty.
    /*! Determine if the character buffer is currently empty. */
    bool empty() const
    {
      return m_Count == 0;
    }

    //! Seek forward in the buffer.
    /*! Seek forward in the character buffer. Note: this asserts that there
        are enough bytes in the buffer for the seek operation. It will raise
        a signal if there's not. Note: calling this may move or reallocate
        the buffer. All earlier results of data() will be invalidated. */
    void seek(size_t n)
    {
      assert(n <= m_Count);
      m_Pos += n;
      m_Count -= n;
    }

    //! Append data to the buffer
    /*! Append data to the character buffer. Note: calling this may move
        or reallocate the buffer. All earlier results of data() will be
        invalidated. */
    void append(const unsigned char * data, size_t n);

    //! Clear the buffer
    /*! Seeks to the end of the character buffer essentially clearing it. */
    void clear()
    {
      seek(count());
    }

  private:
    unsigned char * m_Ptr;
    size_t m_Pos, m_Count, m_Left;
  };
}

#endif // NEWNET_BUFFER_H

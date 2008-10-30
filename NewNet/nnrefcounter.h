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

#ifndef NEWNET_REFCOUNTER_H
#define NEWNET_REFCOUNTER_H

#ifdef NN_PTR_DEBUG
 #ifdef NN_PTR_DEBUG_ASSERT
  #include <assert.h>
 #else
  #include <iostream>
 #endif // NN_PTR_DEBUG_ASSERT
#endif // NN_PTR_DEBUG

namespace NewNet
{
  //! Helper class for reference counting.
  /*! This class basically provides an int that's guaranteed to be 0 when
      the refcounter is constructed. Provides increment and decrement
       operators. The decrement operator will return true if the reference
       count drops to zero. */
  class RefCounter
  {
  public:
    //! Create a new reference counter object.
    /*! Creates a new reference counter object with the reference count
        initialized at 0. */
    RefCounter() : m_RefCount(0)
    {
    }

    //! Increment the reference count.
    /*! Increment the reference count by 1. */
    void operator++()
    {
      m_RefCount += 1;
    }

    //! Decrement the reference count.
    /*! Decrement the reference count by 1. Returns true if the reference
        count drops to zero. Delete the owner object if that happens. */
    bool operator--()
    {
#ifdef NN_PTR_DEBUG
 #ifdef NN_PTR_DEBUG_ASSERT
      assert(m_RefCount > 0);
 #else
      if(m_RefCount == 0)
      {
        std::cerr << "Warning: Attempting to decrement refcount of unreferenced object!" << std::endl;
        return true;
      }
 #endif // NN_PTR_DEBUG_ASSERT
#endif // NN_PTR_DEBUG
      m_RefCount -= 1;
      return m_RefCount == 0;
    }

    //! Return the reference count.
    /*! Return the value of this reference counter. */
    unsigned int count()
    {
      return m_RefCount;
    }

  private:
    unsigned int m_RefCount;
  };
}

#endif // NEWNET_REFCOUNTER_H

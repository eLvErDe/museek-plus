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

#ifndef NEWNET_BASEPTR_H
#define NEWNET_BASEPTR_H

#ifdef NN_PTR_DEBUG
 #include <assert.h>
 #ifdef NN_PTR_DEBUG_ASSERT
  #define NN_PTR_CHECK { assert(m_Ptr != 0); }
 #else
  #include <iostream>
  #define NN_PTR_CHECK { if(! m_Ptr) std::cerr << "Warning: Dereferencing NULL pointer" << std::endl; }
 #endif
#else
 #define NN_PTR_CHECK
#endif

namespace NewNet
{
  class Object;

  //! Base class for wrapped pointers.
  /*! This is a base wrapper class for pointers. This class is pretty
      useless when used directly, subclass it or use RefPtr or WeakRefPtr
      instead. */
  template<class T> class BasePtr
  {
  public:
    //! Create a new BasePtr.
    /*! Create a new BasePtr. The pointer always points at NULL. */
    BasePtr() : m_Ptr(0)
    {
    }

    //! Create a new initialized BasePtr.
    /*! Create a new BasePtr. The pointer will point at T * t. */
    BasePtr(T * t) : m_Ptr(t)
    {
    }

    //! Return the pointer.
    /*! Return the pointer. */
    T * ptr() const
    {
      return m_Ptr;
    }

    //! Cast operator to T *.
    /*! This provides a cast operator to T *. */
    operator T*() const
    {
      return m_Ptr;
    }

    //! Dereferencing operator to T &.
    /*! This provides a dereferencing operator to T &. Dereferencing a NULL
        pointer is checked if compiled with -DNN_PTR_DEBUG. */
    T & operator*()
    {
      NN_PTR_CHECK
      return *m_Ptr;
    }

    //! Dereferencing operator to const T &.
    /*! This provides a dereferencing operator to const T &. Dereferencing a
        NULL pointer is checked if compiled with -DNN_PTR_DEBUG. */
    const T & operator*() const
    {
      NN_PTR_CHECK
      return *m_Ptr;
    }

    //! Dereferencing operator to T *.
    /*! This provides a dereferencing operator to T *. Dereferencing a NULL
        pointer is checked if compiled with -DNN_PTR_DEBUG. */
    T * operator->()
    {
      NN_PTR_CHECK
      return m_Ptr;
    }

    //! Dereferencing operator to const T *.
    /*! This provides a dereferencing operator to const T *. Dereferencing a
        NULL pointer is checked if compiled with -DNN_PTR_DEBUG. */
    const T * operator->() const
    {
      NN_PTR_CHECK
      return m_Ptr;
    }

    //! Determine wether the pointer is valid (not NULL).
    /*! Return true if the pointer is not NULL. */
    bool isValid() const
    {
      return m_Ptr != 0;
    }

    //! Check if the pointer is NULL.
    /*! Returns true if the pointer is NULL. */
    bool operator!() const
    {
      return m_Ptr == 0;
    }

  protected:
    //! The pointer value.
    /*! Provides the current pointer value. */
    T * m_Ptr;
  };
}

#undef NN_PTR_CHECK

#endif // NEWNET_REFPTR_H

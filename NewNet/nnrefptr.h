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

#ifndef NEWNET_REFPTR_H
#define NEWNET_REFPTR_H

#include "nnbaseptr.h"

namespace NewNet
{
  class Object;

  //! A reference counting pointer.
  /*! This provides a pointer type which does automatic deletion of objects
      by tracking the reference count. If the reference count drops to 0, the
      containing object will be disposed of. */
  template<class T> class RefPtr : public BasePtr<T>
  {
  public:
    //! Create a new reference counter pointer that points at NULL.
    /*! Create a new reference counter pointer that points at NULL. */
    RefPtr() : BasePtr<T>()
    {
    }

    //! Create a new reference counter pointer that points at t.
    /*! Create a new reference counter pointer that points at t. */
    RefPtr(T * t) : BasePtr<T>(t)
    {
      if(t)
        ++(t->refCounter());
    }

    //! Create a new reference counter pointer that points at t.
    /*! Create a new reference counter pointer that points at t. */
    RefPtr(const RefPtr& t) : BasePtr<T>()
    {
      BasePtr<T>::m_Ptr = t.m_Ptr;
      if(t.m_Ptr)
        ++(t.m_Ptr->refCounter());
    }

    //! Assign operator.
    /*! Assign the object t is pointing at to this pointer as well. */
    RefPtr& operator=(const RefPtr& t)
    {
      if(BasePtr<T>::m_Ptr == t.m_Ptr)
        return *this;
      if(BasePtr<T>::m_Ptr)
      {
        if(--(BasePtr<T>::m_Ptr->refCounter()))
          delete BasePtr<T>::m_Ptr;
      }
      BasePtr<T>::m_Ptr = t.m_Ptr;
      if(t.m_Ptr)
      {
        ++(t.m_Ptr->refCounter());
      }
      return *this;
    }

    //! Assign operator.
    /*! Make this pointer point at t. */
    RefPtr& operator=(T * t)
    {
      if(BasePtr<T>::m_Ptr == t)
        return *this;
      if(BasePtr<T>::m_Ptr)
      {
        if(--(BasePtr<T>::m_Ptr->refCounter()))
          delete BasePtr<T>::m_Ptr;
      }
      BasePtr<T>::m_Ptr = t;
      if(t)
        ++(t->refCounter());
      return *this;
    }

    //! Destructor.
    /*! Decrements reference count of object and deletes it if the reference
        count drops to 0. */
    ~RefPtr()
    {
      if(BasePtr<T>::m_Ptr)
      {
        if(--(BasePtr<T>::m_Ptr->refCounter()))
          delete BasePtr<T>::m_Ptr;
      }
    }
  };
}

#endif // NEWNET_REFPTR_H

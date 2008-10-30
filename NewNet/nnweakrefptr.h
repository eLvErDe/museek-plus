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

#ifndef NEWNET_WEAKREFPTR_H
#define NEWNET_WEAKREFPTR_H

#include "nnbaseptr.h"
#include "nnguardobject.h"

namespace NewNet
{
  class Object;

  //! A weak reference pointer class.
  /*! A weak reference pointer as implemented in this class points at NULL
      after the object it once pointed as is deleted. This is done by
      registering a callback to the object's guard object so that the
      pointer class will be notified when the object is deleted. */
  template<class T> class WeakRefPtr : public BasePtr<T>
  {
  private:
#ifndef DOXYGEN_UNDOCUMENTED
    class GuardObjectCallback : public GuardObject::Callback
    {
    public:
      GuardObjectCallback(WeakRefPtr * ptr) : m_Ptr(ptr)
      {
      }

      void operator()(Object * p)
      {
        m_Ptr->objectDestroyed(p);
      }

    private:
      WeakRefPtr * m_Ptr;
    };

    GuardObjectCallback * m_GuardObjectCallback;
#endif // DOXYGEN_UNDOCUMENTED

  public:
    //! Create a new weak reference pointer that points at NULL.
    /*! Create a new weak reference pointer that points at NULL. */
    WeakRefPtr() : BasePtr<T>()
    {
      m_GuardObjectCallback = new GuardObjectCallback(this);
    }

    //! Create a new weak reference pointer that points at t.
    /*! Create a new weak reference pointer that points at t. */
    WeakRefPtr(T * t) : BasePtr<T>(t)
    {
      m_GuardObjectCallback = new GuardObjectCallback(this);
      if(t)
        t->guardObject() += m_GuardObjectCallback;
    }

    //! Create a new weak reference pointer that points at t.
    /*! Create a new weak reference pointer that points at t. */
    WeakRefPtr(const WeakRefPtr& t) : BasePtr<T>(t.m_Ptr)
    {
      m_GuardObjectCallback = new GuardObjectCallback(this);
      if(t.m_Ptr)
        t.m_Ptr->guardObject() += m_GuardObjectCallback;
    }

    //! Assign t's object to this pointer.
    /*! Assign t's object to this pointer. */
    WeakRefPtr& operator=(const WeakRefPtr& t)
    {
      if(BasePtr<T>::m_Ptr == t.m_Ptr)
        return *this;
      if(BasePtr<T>::m_Ptr)
        BasePtr<T>::m_Ptr->guardObject() -= m_GuardObjectCallback;
      BasePtr<T>::m_Ptr = t.m_Ptr;
      if(t.m_Ptr)
        BasePtr<T>::m_Ptr->guardObject() += m_GuardObjectCallback;
      return *this;
    }

    //! Assign object t to this pointer.
    /*! Assign object t to this pointer. */
    WeakRefPtr& operator=(T * t)
    {
      if(BasePtr<T>::m_Ptr == t)
        return *this;
      if(BasePtr<T>::m_Ptr)
        BasePtr<T>::m_Ptr->guardObject() -= m_GuardObjectCallback;
      BasePtr<T>::m_Ptr = t;
      if(t)
        t->guardObject() += m_GuardObjectCallback;
      return *this;
    }

#ifndef DOXYGEN_UNDOCUMENTED
    ~WeakRefPtr()
    {
      if(BasePtr<T>::m_Ptr)
        BasePtr<T>::m_Ptr->guardObject() -= m_GuardObjectCallback;
      delete m_GuardObjectCallback;
    }
#endif // DOXYGEN_UNDOCUMENTED

  protected:
#ifndef DOXYGEN_UNDOCUMENTED
    void objectDestroyed(Object * p)
    {
#ifdef NN_PTR_DEBUG
      assert(p == BasePtr<T>::m_Ptr);
#endif
      BasePtr<T>::m_Ptr = 0;
    }
#endif // DOXYGEN_UNDOCUMENTED
  };
}

#undef NN_PTR_CHECK

#endif // NEWNET_REFPTR_H

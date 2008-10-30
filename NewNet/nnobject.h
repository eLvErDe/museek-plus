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

#ifndef NEWNET_OBJECT_H
#define NEWNET_OBJECT_H

#include "nnguardobject.h"
#include "nnrefcounter.h"

#ifdef NN_PTR_DEBUG
 #ifdef NN_PTR_DEBUG_ASSERT
  #include <assert.h>
 #else
  #include <iostream>
 #endif // NN_PTR_DEBUG_ASSERT
#endif // NN_PTR_DEBUG

namespace NewNet
{
  //! Base class for objects.
  /*! This provides a base class for NewNet objects. It provides a reference
      counter and a guard object for detecting deletion of the object. */
  class Object
  {
  public:
    //! Constructor.
    /*! This constructs a new object. Note that you don't have to call this.
        It's an empty method. */
    Object()
    {
    }

    //! Copy constructor.
    /*! Ensures that the reference count of the copied object is 0 and that
        the guard object is empty. */
    Object(const Object &)
    {
    }

    //! Destructor.
    /*! When an object is destroyed, the guard object will be emitted so the
        registered callbacks get called. */
    virtual ~Object()
    {
      m_GuardObject.emit(this);
#ifdef NN_PTR_DEBUG
 #ifdef NN_PTR_DEBUG_ASSERT
      assert(m_RefCounter.count() == 0);
 #else
      if(m_RefCounter.count() != 0)
        std::cerr << "Warning: Object " << this << " deleted while refcount = " << m_RefCounter.count() << "." << std::endl;
 #endif // NN_PTR_DEBUG_ASSERT
#endif // NN_PTR_DEBUG
    }

    //! Reference to the guard object.
    /*! This returns a reference to the object's guard object so you can hook
        in your own callbacks if you want to. */
    GuardObject & guardObject()
    {
      return m_GuardObject;
    }

    //! Reference to the reference counter.
    /*! This returns a reference to the object's reference counter so you can
        manipulate it if you want to. */
    RefCounter & refCounter()
    {
      return m_RefCounter;
    }

  private:
    GuardObject m_GuardObject;
    RefCounter m_RefCounter;
  };
}

#endif // NEWNET_OBJECT_H

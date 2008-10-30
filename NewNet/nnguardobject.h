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

#ifndef NEWNET_GUARDOBJECT_H
#define NEWNET_GUARDOBJECT_H

#include <vector>
#include <algorithm>

namespace NewNet
{
  class Object;

  //! Helper class for detecting object deletion.
  /*! This class is used to detect when objects are deleted. You can add
      callbacks that will get invoked when the object is deleted. All
      objects have a guard object embedded. See NewNet::Object. */
  class GuardObject
  {
  public:
    //! The callback class for the guard object.
    /*! The callback class for the guard object. operator()(Object * p) will be
        invoked when the callback is added to a guard object and the guard
        object is deleted. */
    class Callback
    {
    public:
#ifndef DOXYGEN_UNDOCUMENTED
      virtual ~Callback() {}
#endif // DOXYGEN_UNDOCUMENTED

      //! Override this and place your callback code here.
      /*! Override this and place your callback code here. */
      virtual void operator()(Object * p) = 0;
    };

    //! Create a new guard object.
    /*! Create a new guard object. */
    GuardObject()
    {
    }

#ifndef DOXYGEN_UNDOCUMENTED
    void emit(Object * p)
    {
      std::vector<Callback *>::iterator it, end = m_Callbacks.end();
      for(it = m_Callbacks.begin(); it != end; ++it)
        (*it)->operator()(p);
    }
#endif // DOXYGEN_UNDOCUMENTED

    //! Add a callback to the guard object.
    /*! Add a callback to the guard object so the callback will be invoked
        when the guard object is deleted. Note: this stores a regular pointer
        and not a RefPtr. */
    GuardObject & operator+=(Callback * p)
    {
      m_Callbacks.push_back(p);
      return *this;
    }

    //! Remove a callback from the guard object.
    /*! Remove a callback from the guard object. It will no longer get invoked
        when the guard object is deleted. Note that since the guard object
        stores a regular pointer to the callback, it will not get deleted
        automatically. */
    GuardObject & operator-=(Callback * p)
    {
      std::vector<Callback *>::iterator it;
      it = std::find(m_Callbacks.begin(), m_Callbacks.end(), p);
      if (it != m_Callbacks.end())
        m_Callbacks.erase(it);

      return *this;
    }

  private:
    std::vector<Callback *> m_Callbacks;
  };
}

#endif // NEWNET_GUARDOBJECT_H

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

#ifndef NEWNET_EVENT_H
#define NEWNET_EVENT_H

#include "nnobject.h"
#include "nnrefptr.h"
#include <vector>
#include <functional>
#include <algorithm>

namespace NewNet
{
  //! Simple event dispatcher class.
  /*! The NewNet::Event class provides a simple and easy to use event
      aggregation solution. Callbacks can be registered to an event that
      will be invoked upon emitting the event. */
  template<typename T> class Event : public Object
  {
  public:
    //! Abstract callback type for Event.
    /*! A callback can be added to an event so that operator()(T t) will be
        called when the event is emitted. */
    class Callback : public Object
    {
    public:
      //! Constructor.
      Callback() { }

      //! Destructor.
      virtual ~Callback() { }

      //! Slot method.
      /*! Override this and implement your callback function. */
      virtual void operator()(T t) = 0;

#ifndef DOXYGEN_UNDOCUMENTED
      /* Callback was added to an Event */
      void onConnected(Event * event)
      {
        m_Events.push_back(event);
      }

      /* Callback was disconnected from an Event */
      void onDisconnected(Event * event)
      {
        typename std::vector<Event *>::iterator it;
        it = std::find(m_Events.begin(), m_Events.end(), event);
        if (it != m_Events.end())
            m_Events.erase(it);
      }
#endif // DOXYGEN_UNDOCUMENTED

      //! Disconnect this callback from all events.
      /*! Calling this will result in the disconnection of the callback from
          all the events it is registered to. Note: Events store a RefPtr to
          the callbacks. The callback class might be deleted when
          disconnecting from all registered events. */
      void disconnect()
      {
        /* Artificially increase our reference count. Otherwise we might get
           prematurely deleted when we get disconnected from the last Event */
        ++(this->refCounter());

        /* Disconnect from all the events we're registered to */
        while(! m_Events.empty())
          m_Events.front()->disconnect(this);

        /* Decrease the refrence count again and delete if necessary */
        if(--(this->refCounter()))
          delete this;
      }

    private:
      /* Private copy constructor, you don't want this happening */
      Callback(const Callback &) { }
      std::vector<Event *> m_Events;
    };

  private:
#ifndef DOXYGEN_UNDOCUMENTED
    /* Callback to a method of a NewNet::Object */
    template<class ObjectType, typename MethodType> class BoundCallback : public Callback
    {
    private:
      /* We use this to track deletion of our Object */
      class GuardObjectCallback : public GuardObject::Callback
      {
      public:
        GuardObjectCallback(BoundCallback * callback) : m_Callback(callback)
        {
        }

        /* If the Object is destroyed, notify our Callback */
        void operator()(Object *)
        {
          m_Callback->onObjectDeleted();
        }
      private:
        BoundCallback * m_Callback;
      };
      GuardObjectCallback * m_GuardObjectCallback;

    public:
      BoundCallback(ObjectType * object, MethodType method)
                   : m_Object(object), m_Method(method)
      {
        /* Register to the Object's delete guard */
        m_GuardObjectCallback = new GuardObjectCallback(this);
        m_Object->guardObject() += m_GuardObjectCallback;
      }

      ~BoundCallback()
      {
        /* If the object is still valid, remove our delete callback from
           its delete guard */
        if(m_Object)
          m_Object->guardObject() -= m_GuardObjectCallback;

        delete m_GuardObjectCallback;
      }

      void operator()(T t)
      {
        /* Since C++ methods are internally called as
           Class::Method(object, ...), this results in a valid C++ bound
           method call. */
        if(m_Object)
          std::bind1st(std::mem_fun(m_Method), m_Object)(t);
      }

    protected:
      /* Object we're bound to was deleted, reset pointer and disconnect
         from all Events we're registered to */
      void onObjectDeleted()
      {
        m_Object = 0;
        Callback::disconnect();
      }

    private:
      /* Private copy constructor, you don't want this happening */
      BoundCallback(const BoundCallback &) { }

      ObjectType * m_Object;
      MethodType m_Method;
    };
#endif // DOXYGEN_UNDOCUMENTED

  public:
    //! Constructor.
    /*! Create a new event to which you can register callbacks. */
    Event() { }

    //! Copy constructor.
    /*! When an event is copied, all the callbacks registered to the original
        event will also be connected to this event. */
    Event(const Event & that)
    {
      typename std::vector<RefPtr<Callback> >::const_iterator it, end;
      end = that.m_Callbacks.end();
      for(it = that.m_Callbacks.begin(); it != end; ++it)
        connect(*it);
    }

    //! Destructor.
    /*! Disconnects all callbacks from the event. Note: see clear(). */
    virtual ~Event()
    {
      /* Disconnect all Callbacks */
      clear();
    }

    //! Empty the event callback list.
    /*! Call this to remove all the callbacks from this event. Note: the
        event stores a RefPtr to all the callbacks registered. Clearing the
        event may delete the callback if there are no other references to
        it. */
    void clear()
    {
      while(! m_Callbacks.empty())
        disconnect(m_Callbacks.front());
    }

    //! Connect a callback to the event.
    /*! Add a callback to this event so that it will get invoked when the
        event is emitted. Note: stores a RefPtr to the callback. */
    Callback * connect(Callback * callback)
    {
      /* Push a RefPtr to the Callback to our list */
      m_Callbacks.push_back(callback);

      /* Notify the Callback that it's connected to us */
      callback->onConnected(this);

      return callback;
    }

    //! Create a callback to a bound method.
    /*! This will construct a callback object that will invoke a method
        of an object. */
    template<class ObjectType, typename MethodType>
    static Callback * bind(ObjectType * object, MethodType method)
    {
      return new BoundCallback<ObjectType, MethodType>(object, method);
    }

    //! Connect callback to a method of an object to the event.
    /*! Add a callback to a method of an object so that it will get
        invoked when the event is emitted. Note: stores a RefPtr to the newly
        created callback. */
    template<class ObjectType, typename MethodType>
    Callback * connect(ObjectType * object, MethodType method)
    {
      return connect(bind(object, method));
    }

    //! Disconnect a callback from the event.
    /*! Remove a callback from the invocation list. Note: the event stores
        a RefPtr to the callback. If the event holds the last RefPtr, the
        callback will be deleted. */
    void disconnect(Callback * callback)
    {
      /* Artificially increase the reference count so it doesn't get deleted
         prematurely */
      ++(callback->refCounter());

      /* Delete the Callback from our list */
      typename std::vector<RefPtr<Callback> >::iterator it;
      it = std::find(m_Callbacks.begin(), m_Callbacks.end(), callback);
      if (it != m_Callbacks.end())
        m_Callbacks.erase(it);

      /* Notify the Callback it was disconnected */
      callback->onDisconnected(this);

      /* Decrease the reference count of the Callback and delete if
         necessary */
      if(--(callback->refCounter()))
        delete callback;
    }

    //! Emit the event.
    /*! Emit the event, invokes the Callback::operator()(T t) method of all
        registered callbacks. */
    void operator()(T t)
    {
      std::vector<RefPtr<Callback> > callbacks(m_Callbacks);
      typename std::vector<RefPtr<Callback> >::iterator it, end = callbacks.end();
      for(it = callbacks.begin(); it != end; ++it)
      {
        (*(*it))(t);
      }
    }

  private:
    std::vector<RefPtr<Callback> > m_Callbacks;
  };
}

#endif // NEWNET_EVENT_H

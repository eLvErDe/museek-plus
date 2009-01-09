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

#ifndef NEWNET_REACTOR_H
#define NEWNET_REACTOR_H

#include "nnobject.h"
#include "nnsocket.h"
#include "nnrefptr.h"
#include "nnevent.h"
#include "util.h"
#include <vector>
#include <event.h>

/* Update timeout to 'ms' miliseconds after the current time if that's
   sooner than the current timeout, or if no timeout has been set yet. */
inline void fixtime(struct timeval & timeout, long ms, bool & timeout_set)
{
  struct timeval now;
  gettimeofday(&now, 0);
  if((! timeout_set) || (difftime(timeout, now) > ms))
  {
    timeout.tv_sec = now.tv_sec + (ms / 1000);
    timeout.tv_usec = now.tv_usec + (ms % 1000) * 1000;
    if(timeout.tv_usec >= 1000000)
    {
      timeout.tv_sec += 1;
      timeout.tv_usec -= 1000000;
    }
    timeout_set = true;
  }
}

namespace NewNet
{
  //! Monitors sockets and timeouts. This is what drives your application.
  /*! The Reactor class provides your application with a main-loop. It
      monitors the sockets and waits for timeouts to occur. */
  class Reactor : public Object
  {
  public:
    //! Constructor.
    /*! Create a new reactor */
    Reactor();

#ifndef DOXYGEN_UNDOCUMENTED
    ~Reactor();
#endif // DOXYGEN_UNDOCUMENTED

    //! Add a socket to the reactor.
    /*! Add a socket to the reactor's watch list so that the socket will be
        able to receive and process events. Note: stores a RefPtr to the
        socket object */
    void add(Socket * socket);

    //! Remove a socket from the reactor.
    /*! Remove a socket from the reactor's watch list. Note: The reactor
        stores a RefPtr to the socket, if the reactor holds the last reference
        to the socket, it will get deleted automatically. */
    void remove(Socket * socket);

    //! Start the main loop.
    /*! Call this to start the reactor's main loop. The reactor will start
        listening for events and waiting for timeouts to happen. This method
        will not return until there are no sockets and timeouts left, or when
        Reactor::stop() is called. */
    void run();

    //! Stop the main loop.
    /*! Call this to exit the reactor's main loop. Note that the reactor will
        first process any pending events before exiting. */
    void stop();

    //! Convenience definition for timeouts.
    /*! A convenience definition for timeout callback. Note: Timeouts aren't
        actually used as events, but the Event::Callback class is used to
        define the callback for a timeout. */
    typedef Event<long> Timeout;

    //! Add a new timeout to the reactor.
    /*! Add a new timeout callback to the reactor so that the callback
        will be invoked after approximately msec miliseconds. Note: stores
        a RefPtr to the callback object. */
    Timeout::Callback * addTimeout(long msec, Timeout::Callback * callback);

    //! Add a new timeout to the reactor.
    /*! Add a new timeout callback to the reactor so that the method of the
        specified object will be invoked after approximately msec miliseconds.
        Note: stores a RefPtr to the generated callback object. */
    template<class ObjectType, typename MethodType>
    Timeout::Callback * addTimeout(long msec, ObjectType * object, MethodType method)
    {
      return addTimeout(msec, Timeout::bind(object, method));
    }

    //! Remove a timeout from the reactor.
    /*! This removes all pending timeouts that have 'callback' as a callback
        and frees the RefPtr on the callback object. */
    void removeTimeout(Timeout::Callback * callback);

    //! Returns the maximum number of sockets that can be opened
    /*! On linux this is usually 1024 */
    int maxSocketNo();

    //! Returns the current number of sockets opened
    int currentSocketNo();

    //! Returns the highest file descriptor currently used
    int maxFileDescriptor();

    //! Invoked by libevent when a socket wakes up
    /*! Invoked by libevent when a socket wakes up */
    void eventCallback(int, short, void *);

  private:
    struct event mEvTimeout;

  protected:
    //! Prepare sockets to be watched by the reactor.
    /*! Prepare sockets to be watched by the reactor. */
    void checkSockets(struct timeval & timeout, bool & timeout_set);

    //! Check for timeouts and emit needed actions. Set up next reactor wake up.
    /*! Check for timeouts and emit needed actions. Set up next reactor wake up. */
    bool checkTimeouts(struct timeval & timeout, bool & timeout_set);

    //! Set up every data needed for the next reactor cycle.
    /*! Set up every data needed for the next reactor cycle. */
    bool prepareReactorData();

    int m_maxSocketNo;
    int m_maxFD;
    std::vector<RefPtr<Socket> > m_Sockets;

#ifndef DOXYGEN_UNDOCUMENTED
    struct Timeouts;
    struct Timeouts * m_Timeouts;
#ifdef WIN32
    void * m_WsaData;
#endif // WIN32
#endif
  };
}

#ifndef DOXYGEN_UNDOCUMENTED
typedef std::pair<struct timeval, NewNet::RefPtr<NewNet::Reactor::Timeout::Callback> > TimeoutItem;
struct NewNet::Reactor::Timeouts
{
  std::vector<TimeoutItem> timeouts;
};
#endif

#endif // NEWNET_REACTOR_H

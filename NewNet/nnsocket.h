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

#ifndef NEWNET_SOCKET_H
#define NEWNET_SOCKET_H

#include "nnobject.h"
#include "nnrefptr.h"
#include "nnweakrefptr.h"
#include "nnratelimiter.h"
#include <event.h>

namespace NewNet
{
  class Reactor;

  //! Base class for network sockets.
  /*! This provides a generic base class for both client and server sockets.
      The reactor doesn't differentiate between those two and just operates
      on this socket type. */
  class Socket : public Object
  {
  public:
    //! Enumeration to describe the state of the socket.
    /*! This defines the current state of the socket. The value can be
        retrieved with socketState() and set with setSocketState(). */
    typedef enum
    {
      SocketUninitialized, //!< The socket is uninitialized.
      SocketListening,     //!< The socket is listening for client connections.
      SocketConnecting,    //!< The socket is currently busy connecting.
      SocketConnected,     //!< The socket is connected.
      SocketDisconnecting, //!< The socket is currently busy disconnecting.
      SocketDisconnected,  //!< The socket is disconnected.
      SocketException      //!< An error occured, the socket is dead.
    } SocketState;

    //! Enumeration to describe pending network events.
    /*! This defines what kind of network events are currently pending on the
        socket. The value can be retrieved using readyState() and set with
        setReadyState(). */
    typedef enum
    {
      StateSend = 1,       //!< The socket is ready to send data.
      StateReceive = 2,    //!< Data was received, socket is ready to read.
      StateException = 4   //!< Out-of-band data was received.
    } ReadyState;

    //! Enumeration to describe the error that last occured.
    /*! This defines what kind of error made the state go into exception
        state. It can be retrieved using socketError() and set with
        setSocketError(). */
    typedef enum
    {
      ErrorNoError,        //!< No error at all, everything's fine.
      ErrorCannotResolve,  //!< The socket was unable to resolve the destination host.
      ErrorInvalidPath,    //!< The specified path was invalid.
      ErrorCannotConnect,  //!< The socket was unable to connect to the remote end.
      ErrorCannotBind,     //!< The socket couldn't bind to the specified address.
      ErrorCannotListen,   //!< The socket couldn't listen on the specified address.
      ErrorUnknown         //!< An unknown error occured.
    } SocketError;

    //! Create a new uninitialized socket.
    /*! This creates a new socket that has an invalid descriptor, is
        uninitialized, has no pending events, no error and no data waiting. */
    Socket() : m_Reactor(0), m_FD(-1), m_SocketState(SocketUninitialized),
              m_ReadyState(0), m_SocketError(ErrorNoError),
              m_DataWaiting(false)
    {
        m_EventData = new struct event;
        m_EventData->ev_flags = 0; // This event has not been initialized
    }

    //! Return the associated reactor.
    /*! Return the reactor this socket is associated to, if any. */
    Reactor * reactor() const
    {
      return m_Reactor;
    }

    //! Set the associated reactor.
    /*! This is called by the reactor when the socket is added to the
        reactor. Note: stores a regular pointer to the reactor. */
    void setReactor(Reactor * reactor)
    {
      m_Reactor = reactor;
    }

    //! Return the socket's descriptor.
    /*! This returns the socket's descriptor. */
    int descriptor() const
    {
      return m_FD;
    }

    //! Set the socket's descriptor.
    /*! This initializes the socket's descriptor. Usually called by
        subclasses when connecting or by socket factories when accepting
        a new client. */
    void setDescriptor(int fd)
    {
      m_FD = fd;
    }

    //! Return the current socket state.
    /*! Retrieves the current socket state. */
    SocketState socketState() const
    {
      return m_SocketState;
    }

    //! Set the current socket state.
    /*! Changes the current socket state. */
    void setSocketState(SocketState socketState)
    {
      m_SocketState = socketState;
    }

    //! Return the socket's ready state.
    /*! Returns what kind of events are pending on the socket. */
    int readyState() const
    {
      return m_ReadyState;
    }

    //! Set the socket's ready state.
    /*! Usually called by subclasses and the reactor to specify what kind
        of network events are pending. */
    void setReadyState(int readyState)
    {
      m_ReadyState = readyState;
    }

    //! Return the socket's error state.
    /*! Returns what kind of error last occured on the socket. */
    SocketError socketError() const
    {
      return m_SocketError;
    }

    //! Set the socket's error state.
    /*! Usually called by subclasses to specify what kind of error occured
        on the socket. */
    void setSocketError(SocketError socketError)
    {
      m_SocketError = socketError;
      setSocketState(SocketException);
    }

    //! Return wether there's data waiting to be sent.
    /*! Called by the reactor to determine wether the socket has data
        waiting to be sent. */
    bool dataWaiting() const
    {
      return m_DataWaiting;
    }

    //! Set the data waiting flag.
    /*! Called by subclasses to specify that there's data waiting to be sent */
    void setDataWaiting(bool dataWaiting)
    {
      m_DataWaiting = dataWaiting;
    }

    //! Return the current download rate limiter.
    /*! Return the current download rate limiter. */
    RateLimiter * downRateLimiter()
    {
      return m_DownRateLimiter;
    }

    //! Set the current download rate limiter.
    /*! Set the current download rate limiter. Note: stores a RefPtr to the
        rate limiter. */
    void setDownRateLimiter(RateLimiter * limiter)
    {
      m_DownRateLimiter = limiter;
    }

    //! Return the current upload rate limiter.
    /*! Return the current upload rate limiter. */
    RateLimiter * upRateLimiter()
    {
      return m_UpRateLimiter;
    }

    //! Set the current upload rate limiter.
    /*! Set the current upload rate limiter. Note: stores a RefPtr to the
        rate limiter. */
    void setUpRateLimiter(RateLimiter * limiter)
    {
      m_UpRateLimiter = limiter;
    }

    //! Processor function.
    /*! This is called by the reactor when there are pending network events. */
    virtual void process()
    {
    }

    //! Associate some libevent data to the socket.
    /*! Associate some libevent data to the socket. */
    void setEventData(struct event & evData) {
        *m_EventData = evData;
    }

    //! Returns libevent data associated with the socket.
    /*! Returns libevent data associated with the socket. */
    struct event * getEventData() {
        return m_EventData;
    }

  private:
    Reactor * m_Reactor;
    int m_FD;
    SocketState m_SocketState;
    int m_ReadyState;
    SocketError m_SocketError;
    bool m_DataWaiting;
    RefPtr<RateLimiter> m_DownRateLimiter, m_UpRateLimiter;
    struct event * m_EventData;
  };
}

#endif // NEWNET_SOCKET_H

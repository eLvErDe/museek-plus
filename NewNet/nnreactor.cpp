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

#include "nnreactor.h"
#include "nnlog.h"
#include "platform.h"
#include "util.h"
#include <algorithm>
#include <iostream>
#include <queue>
#include <assert.h>
#include <sys/resource.h>


void eventCallback(int fd, short event, void *arg) {
    static_cast<NewNet::Reactor *>(arg)->eventCallback(fd, event, arg);
}

NewNet::Reactor::Reactor()
{
    m_Timeouts = new Timeouts;
#ifdef WIN32
    m_WsaData = new WSADATA;
    WORD wVersionRequested = MAKEWORD(1, 1);
    assert(WSAStartup(wVersionRequested, (WSADATA *)m_WsaData) == 0);
#endif // WIN32

    // Set the FD limit to the maximum available
    // The maximum available can be modified in /etc/security/limits.conf (changing nofile parameter for your user)
    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) == 0) {
        rl.rlim_cur = rl.rlim_max;
        setrlimit(RLIMIT_NOFILE, &rl);
    }

    struct rlimit rlim;
    m_maxSocketNo = -1;
    if (getrlimit (RLIMIT_NOFILE, &rlim) >= 0)
        m_maxSocketNo = rlim.rlim_cur;


    NNLOG("newnet.net.debug", "%i file descriptors available for museekd.", m_maxSocketNo);

    event_init();
}

#ifndef DOXYGEN_UNDOCUMENTED
NewNet::Reactor::~Reactor()
{
#ifdef WIN32
  WSACleanup();
  delete (WSADATA *)m_WsaData;
#endif // WIN32
  delete m_Timeouts;
}
#endif // DOXYGEN_UNDOCUMENTED

/* Check if any timeouts have expired. If so, invoke them and remove them
   from the queue. Also, update the timeout if a timeout should be called
   before the currently set timeout. */
bool
NewNet::Reactor::checkTimeouts(struct timeval & timeout, bool & timeout_set)
{
  bool retVal = false;

  struct timeval now;
  gettimeofday(&now, 0);

  std::vector<TimeoutItem>::iterator it;

  std::vector<TimeoutItem> timeouts = m_Timeouts->timeouts; // copy the timeouts in case it is modified somewhere else while iterating

  it = timeouts.begin();
  while (it != timeouts.end())
  {
    // Has the timeout expired?
    if(timercmp(&now, &(*it).first, >=))
    {
      // Calculate how long the timeout is overdue
      gettimeofday(&now, 0);
      unsigned long diff = difftime(now, (*it).first);

      // Store the timeout callback and delete it from to-be-emitted list
      NewNet::RefPtr<NewNet::Reactor::Timeout::Callback> item = it->second;

      removeTimeout(item);

      it = timeouts.erase(it);

      // And emit it
      if (item.isValid())
        item->operator()(diff);

      retVal = true;
    }
    else {
      if((! timeout_set) || (timercmp(&(*it).first, &timeout, <))) {
          /* If the timeout expires before the next cycle timeout, adjust
             the cycle timeout. */
          timeout.tv_sec = (*it).first.tv_sec;
          timeout.tv_usec = (*it).first.tv_usec;
          timeout_set = true;
      }
      ++it;
    }
  }

  return retVal;
}

void NewNet::Reactor::add(Socket * socket)
{
  if(socket->reactor())
  {
    if(socket->reactor() == this)
      return;

    /* Proceed with caution here, otherwise the socket might get prematurely
       deleted: first create our shared reference, then remove it from the
       other reactor. */
    m_Sockets.push_back(socket);
    socket->reactor()->remove(socket);
  }
  else
    m_Sockets.push_back(socket);
  socket->setReactor(this);
}

void NewNet::Reactor::remove(Socket * socket)
{
  int fd = socket->descriptor();
  NNLOG("newnet.net.debug", "removing socket %u from reactor", socket->descriptor());
  // Removing a socket from the wrong reactor is a programming error, trap it.
  assert(socket->reactor() == this);

  socket->setReactor(0);
  std::vector<RefPtr<Socket> >::iterator it;
  it = std::find(m_Sockets.begin(), m_Sockets.end(), socket);
  if (it != m_Sockets.end()) {
    // See if there is another socket using the same FD
    std::vector<RefPtr<Socket> >::iterator itFD;
    bool found = false;
    for (itFD = m_Sockets.begin(); ((itFD != m_Sockets.end()) && !found); ++itFD) {
        if (((*itFD)->descriptor() == fd) && (socket != *itFD))
            found = true;
    }
    if (!found) {
        // No other socket is using this FD stop watching it
        struct event * ev = socket->getEventData();
        if (event_initialized(ev))
            event_del(ev);
    }

    m_Sockets.erase(it);
  }
}

void NewNet::Reactor::run()
{
    NNLOG("newnet.net.debug", "Running reactor. Libevent is using %s method.", event_get_method());

    bool loop = true;
    while (loop) {
        loop = prepareReactorData();
    }

    // Launch the main loop
    event_dispatch();
}

bool
NewNet::Reactor::prepareReactorData() {
    /* No timeout set yet */
    bool timeout_set = false;
    struct timeval timeout;

    // Update the sockets watched by the reactor
    checkSockets(timeout, timeout_set);

    // Update the timeouts and call expired ones
    if (checkTimeouts(timeout, timeout_set))
        return true;

    // Set a timer to come back here when needed
    if(timeout_set)
    {
      /* We know when we need to wake up, but how many sec/usec from
         now is that? */
      struct timeval now;
      gettimeofday(&now, 0);
      timeout.tv_sec -= now.tv_sec;
      timeout.tv_usec -= now.tv_usec;
      if(timeout.tv_usec < 0)
      {
        timeout.tv_sec -= 1;
        timeout.tv_usec += 1000000;
      }
      if(timeout.tv_sec < 0)
      {
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;
      }

      evtimer_del(&mEvTimeout); // delete potentially existing previous timeout
      evtimer_set(&mEvTimeout, ::eventCallback, this);
      evtimer_add(&mEvTimeout, &timeout);
    }

    if(timeout_set)
      NNLOG("newnet.net.debug", "Waiting at most %li ms until one of %i sockets wakes up (max FD: %i).", (timeout.tv_sec * 1000) + (timeout.tv_usec / 1000), currentSocketNo(), maxFileDescriptor());
    else
      NNLOG("newnet.net.debug", "Waiting indefinitely until one of %i sockets wakes up (max FD: %i).", currentSocketNo(), maxFileDescriptor());

    return false;
}

void
NewNet::Reactor::eventCallback(int fd, short event, void *arg) {
    NNLOG("newnet.net.debug", "Entering event callback for socket %i.", fd);

    bool loop = true;
    while (loop) {
        /* Make a copy of our socket list, as they might disappear because of
           events that occur and then our iterators go berserk */
        std::vector<RefPtr<Socket> > sockets(m_Sockets);

        std::vector<RefPtr<Socket> >::iterator it, end = sockets.end();

        for(it = sockets.begin(); it != end; ++it) {
            NewNet::Socket * sock = *it;
            struct event *evData = sock->getEventData();

            // Let the sockets do their job
            if ((sock->descriptor() >= 0) && ((evData->ev_res & EV_READ) || (evData->ev_res & EV_WRITE))) {
              // Update the socket's ready state
              long upLimit = (! sock->upRateLimiter()) ? 0 : sock->upRateLimiter()->nextWindow();
              long downLimit = (! sock->downRateLimiter()) ? 0 : sock->downRateLimiter()->nextWindow();

              int state = 0;
              if ((downLimit == 0) && (evData->ev_res & EV_READ))
                state |= NewNet::Socket::StateReceive;
              if ((upLimit == 0) && (evData->ev_res & EV_WRITE))
                state |= NewNet::Socket::StateSend;
              sock->setReadyState(state);

              // If we have something to report, make the socket process the events.
              if(state)
                sock->process();
            }
        }

        loop = prepareReactorData();
    }
}

void
NewNet::Reactor::checkSockets(struct timeval & timeout, bool & timeout_set) {
    /* Make a copy of our socket list, as they might disappear because of
       events that occur and then our iterators go berserk */
    std::vector<RefPtr<Socket> > sockets(m_Sockets);

    /* Check which events we want to hear about from which sockets */
    std::vector<RefPtr<Socket> >::iterator it, end = sockets.end();

    int nfds = 0;

    for(it = sockets.begin(); it != end; ++it)
    {
      /* Convenience... */
      NewNet::Socket * sock = *it;
      struct event *evData = sock->getEventData();
      int fd = sock->descriptor();
      if(fd == -1)
        continue;

      long n; // miliseconds to next window of opportunity
      short evFlags = 0; // event type flag to be used

      switch(sock->socketState())
      {
        /* The socket is dead, no events are interesting */
        case NewNet::Socket::SocketUninitialized:
        case NewNet::Socket::SocketDisconnecting:
        case NewNet::Socket::SocketDisconnected:
        case NewNet::Socket::SocketException:
          break;

        /* Listening socket, check for read-ready events */
        case NewNet::Socket::SocketListening:
          evFlags = EV_READ;
          nfds = std::max(nfds, fd + 1);
          break;

        /* Connecting socket, check for write-ready events */
        case NewNet::Socket::SocketConnecting:
          evFlags = EV_WRITE;
          nfds = std::max(nfds, fd + 1);
          break;

        /* Connected socket, if possible / allowed check for read, write */
        case NewNet::Socket::SocketConnected:
          /* Check if we're allowed to receive, and if not, when we might be. */
          n = (! sock->downRateLimiter()) ? 0 : sock->downRateLimiter()->nextWindow();
          if(n == 0)
            evFlags = EV_READ;
          else
          {
            NNLOG("newnet.net.debug", "Download limiter for socket %i recommends %li ms sleep.", fd, n);
            fixtime(timeout, n, timeout_set);
          }

          /* Check if we want to send, if we're allowed to send. And if we're
             not allowed to send, when we might be. */
          if(sock->dataWaiting())
          {
            n = (! sock->upRateLimiter()) ? 0 : sock->upRateLimiter()->nextWindow();
            if(n == 0)
                evFlags |= EV_WRITE;
            else
            {
              NNLOG("newnet.net.debug", "Upload rate limiter for socket %i reports next window in %li ms", fd, n);
              fixtime(timeout, n, timeout_set);
            }
          }

          nfds = std::max(nfds, fd + 1);
          break;
        }

      m_maxFD = nfds;

      if (evFlags > 0) {
        if (event_initialized(evData))
            event_del(evData);
        event_set(evData, fd, evFlags, ::eventCallback, this);
        event_add(evData, NULL);
      }
    }
}

void
NewNet::Reactor::stop()
{
  event_loopexit(NULL);
}

NewNet::Reactor::Timeout::Callback *
NewNet::Reactor::addTimeout(long msec, Timeout::Callback * callback)
{
  // Calculate when the event has to occur
  struct timeval tv;
  gettimeofday(&tv, 0);
  tv.tv_sec += (msec / 1000);
  tv.tv_usec += (msec % 1000) * 1000;
  if(tv.tv_usec >= 1000000)
  {
    tv.tv_sec += 1;
    tv.tv_usec -= 1000000;
  }

  // Push the timeout on our queue
  m_Timeouts->timeouts.push_back(TimeoutItem(tv, callback));

  // Return the callback, for convenience
  return callback;
}

void
NewNet::Reactor::removeTimeout(Timeout::Callback * callback)
{
  std::queue<std::vector<TimeoutItem>::iterator> purge;
  std::vector<TimeoutItem>::iterator it, end = m_Timeouts->timeouts.end();
  for(it = m_Timeouts->timeouts.begin(); it != end; ++it)
  {
    if((*it).second == callback)
    {
      purge.push(it);
    }
  }
  while(! purge.empty())
  {
    m_Timeouts->timeouts.erase(purge.front());
    purge.pop();
  }
}

int
NewNet::Reactor::maxSocketNo()
{
    return m_maxSocketNo;
}

int
NewNet::Reactor::currentSocketNo()
{
    return m_Sockets.size();
}

int
NewNet::Reactor::maxFileDescriptor()
{
    return m_maxFD;
}


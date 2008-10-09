/*  NewNet - A networking framework in C++
    Copyright (C) 2006-2007 Ingmar K. Steen (iksteen@gmail.com)

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

#ifndef DOXYGEN_UNDOCUMENTED
typedef std::pair<struct timeval, NewNet::RefPtr<NewNet::Reactor::Timeout::Callback> > TimeoutItem;
struct NewNet::Reactor::Timeouts
{
  std::vector<TimeoutItem> timeouts;
};
#endif

/* Check if any timeouts have expired. If so, invoke them and remove them
   from the queue. Also, update the timeout if a timeout should be called
   before the currently set timeout. */
static bool
checkTimeouts(std::vector<TimeoutItem> & timeouts, struct timeval & timeout, bool & timeout_set)
{
  bool retVal = false;

  struct timeval now;
  gettimeofday(&now, 0);

  std::vector<TimeoutItem>::iterator it, end = timeouts.end();

  for(it = timeouts.begin(); it != end; ++it)
  {
    // Has the timeout expired?
    if(timercmp(&now, &(*it).first, >=))
    {
      // Calculate how long the timeout is overdue
      gettimeofday(&now, 0);
      unsigned long diff = difftime(now, (*it).first);

      // Store the timeout callback and delete it from to-be-emitted list
      NewNet::RefPtr<NewNet::Reactor::Timeout::Callback> item = it->second;
      timeouts.erase(it);

      // And emit it
      if (item)
        item->operator()(diff);

      retVal = true;
    }
    else if((! timeout_set) || (timercmp(&(*it).first, &timeout, <)))
    {
      /* If the timeout expires before the next cycle timeout, adjust
         the cycle timeout. */
      timeout.tv_sec = (*it).first.tv_sec;
      timeout.tv_usec = (*it).first.tv_usec;
      timeout_set = true;
    }
  }

  return retVal;
}

NewNet::Reactor::Reactor()
{
  m_Timeouts = new Timeouts;
#ifdef WIN32
  m_WsaData = new WSADATA;
  WORD wVersionRequested = MAKEWORD(1, 1);
  assert(WSAStartup(wVersionRequested, (WSADATA *)m_WsaData) == 0);
#endif // WIN32
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
  NNLOG("newnet.net.debug", "removing socket %u from reactor", socket->descriptor());
  // Removing a socket from the wrong reactor is a programming error, trap it.
  assert(socket->reactor() == this);

  socket->setReactor(0);
  std::vector<RefPtr<Socket> >::iterator it;
  it = std::find(m_Sockets.begin(), m_Sockets.end(), socket);
  if (it != m_Sockets.end())
    m_Sockets.erase(it);
}

void NewNet::Reactor::run()
{
  int nfds = 0;
  fd_set readfds, writefds, exceptfds;

  m_StopReactor = false;
  while((! m_StopReactor) && (! (m_Timeouts->timeouts.empty() && m_Sockets.empty())))
  {
    /* No timeout set yet */
    bool timeout_set = false;
    struct timeval timeout;

    /* Zero the FD sets */
    nfds = 0;
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);

    /* Make a copy of our socket list, as they might disappear because of
       events that occur and then our iterators go berserk */
    std::vector<NewNet::RefPtr<NewNet::Socket> > sockets(m_Sockets);

    /* Check which events we want to hear about from which sockets */
    std::vector<NewNet::RefPtr<NewNet::Socket> >::iterator it, end = sockets.end();
    for(it = sockets.begin(); it != end; ++it)
    {
      /* Convenience... */
      NewNet::Socket * sock = *it;
      int fd = sock->descriptor();

      if(fd == -1)
        continue;

      long n; // miliseconds to next window of opportunity

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
          FD_SET(fd, &readfds);
          nfds = std::max(nfds, fd + 1);
          break;

        /* Connecting socket, check for write-ready events */
        case NewNet::Socket::SocketConnecting:
          FD_SET(fd, &writefds);
          nfds = std::max(nfds, fd + 1);
          break;

        /* Connected socket, if possible / allowed check for read, write
           and OOB events */
        case NewNet::Socket::SocketConnected:
          /* Check if we're allowed to receive, and if not, when we might be. */
          n = (! sock->downRateLimiter()) ? 0 : sock->downRateLimiter()->nextWindow();
          if(n == 0)
            FD_SET(fd, &readfds);
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
              FD_SET(fd, &writefds);
            else
            {
              NNLOG("newnet.net.debug", "Upload rate limiter for socket %i reports next window in %li ms", fd, n);
              fixtime(timeout, n, timeout_set);
            }
          }

          /* OOB data is not rate limited and we're always interested */
          FD_SET(fd, &exceptfds);

          nfds = std::max(nfds, fd + 1);
          break;
        }
    }

    if(checkTimeouts(m_Timeouts->timeouts, timeout, timeout_set))
      continue;

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
    }

    /* If we have nothing to do, just sleep a bit */
    if(nfds == 0)
    {
      /* If no timeout is set and there are no active descriptors, the reactor
         is dead and it should exit. */
      if(! timeout_set)
        break;

      NNLOG("newnet.net.debug", "Sleeping %li ms until next timeout.", (timeout.tv_sec * 1000) + (timeout.tv_usec / 1000));

#ifndef WIN32
      if(sleep(timeout.tv_sec) == 0)
        usleep(timeout.tv_usec);
#else
      Sleep(timeout.tv_sec * 1000 + timeout.tv_usec / 1000);
#endif // ! WIN32
      continue;
    }

    /* Wait for socket events */
    int r = 0;
    if(timeout_set)
    {
      NNLOG("newnet.net.debug", "Waiting at most %li ms until one of max. %i sockets wakes up.", (timeout.tv_sec * 1000) + (timeout.tv_usec / 1000), nfds);
      r = select(nfds, &readfds, &writefds, &exceptfds, &timeout);
    }
    else
    {
      NNLOG("newnet.net.debug", "Waiting indefinitely until one of max. %i sockets wakes up.", nfds);
      r = select(nfds, &readfds, &writefds, &exceptfds, 0);
    }

    if(r == -1) { // An error occured
        NNLOG("newnet.net.warn", "Error %d while selecting sockets", errno);
        continue; // Let's pretend nothing happened and just try again
    }

    for(it = sockets.begin(); (it != end) && (r > 0); ++it)
    {
      NewNet::Socket * sock = *it;
      int fd = sock->descriptor();
      if(fd == -1)
        continue;

      // Update the socket's ready state
      int state = 0;
      if(FD_ISSET(fd, &readfds))
        state |= NewNet::Socket::StateReceive;
      if(FD_ISSET(fd, &writefds))
        state |= NewNet::Socket::StateSend;
      if(FD_ISSET(fd, &exceptfds))
        state |= NewNet::Socket::StateException;
      sock->setReadyState(state);

      // If we have something to report, make the socket process the events.
      if(state)
      {
        --r;
        sock->process();
      }
    }
  }
}

void
NewNet::Reactor::stop()
{
  m_StopReactor = true;
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

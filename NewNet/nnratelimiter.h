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

#ifndef NEWNET_RATELIMITER_H
#define NEWNET_RATELIMITER_H

#include "nnobject.h"

namespace NewNet
{
  //! Helper class for transfer rate limiting.
  /*! This provides a transfer rate tracker and a method to calculate the
      next window of opportunity (the moment the maximum transfer rate is
      no longer broken) */
  class RateLimiter : public Object
  {
  public:
    //! Constructor.
    /*! Create a new rate limiter. The limit will be initialized to -1 which
        means that there will be no rate limiting. */
    RateLimiter();

#ifndef DOCYGEN_UNDOCUMENTED
    ~RateLimiter();
#endif // DOXYGEN_UNDOCUMENTED

    //! Get the current transfer rate limit.
    /*! This returns the current transfer rate limit in bytes per second. 0
        means that no traffic will be allowed to be sent. A value of -1 means
        that there is no limit set. */
    ssize_t limit() const
    {
      return m_Limit;
    }

    //! Set the transfer rate limit.
    /*! This changes the current transfer rate limit. The limit is measured in
        bytes per second. A value of 0 means that no traffic will be allowed
        to pass. A value of -1 means that no limit is enforced. */
    void setLimit(ssize_t limit)
    {
      m_Limit = limit;
    }

    //! Feed bytes to the collector.
    /*! This adds a frame of bytes to the rate limit collector.
        NewNet::ClientSocket calls this whenever it received or sent data
        of the socket. */
    void transferred(ssize_t bytes);

    //! Next window of opportunity.
    /*! This will predict when the rate limit will be 'unbreached' and when
        data will be allowed to be transferred again. If the limit is set to 0
        this always returns 60000 (60 seconds). If the limit is set to -1, it
        always returns 0. Otherwise, it returns the number of miliseconds
        until the next opportunity. */
    long nextWindow();

  private:
    /* Flush old data from the vector */
    void flush();

    ssize_t m_Limit;

#ifndef DOXYGEN_UNDOCUMENTED
    // The collection data depends on platform specific types, hide it
    struct Data;
    struct Data * m_Data;
#endif // DOXYGEN_UNDOCUMENTED
  };
}

#endif // NEWNET_RATELIMITER_H

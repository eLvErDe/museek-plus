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

#include "nnratelimiter.h"
#include "platform.h"
#include "util.h"

#include <vector>
#include <queue>
#include <algorithm>
#include <iostream>

/* Keep at most x seconds of data in the rate buffer */
#define MAX_HISTORY 5

#ifndef DOXYGEN_UNDOCUMENTED
typedef std::pair<struct timeval, ssize_t> RateData;
struct NewNet::RateLimiter::Data
{
  std::vector<RateData> rateData;
};
#endif // DOXYGEN_UNDOCUMENTED

NewNet::RateLimiter::RateLimiter()
{
  m_Limit = -1;
  m_Data = new NewNet::RateLimiter::Data;
}

#ifndef DOXYGEN_UNDOCUMENTED
NewNet::RateLimiter::~RateLimiter()
{
  delete m_Data;
}
#endif // DOXYGEN_UNDOCUMENTED

void
NewNet::RateLimiter::transferred(ssize_t n)
{
  struct timeval now;
  gettimeofday(&now, 0);
  m_Data->rateData.push_back(RateData(now, n));
}

#ifndef timercmp
#define timercmp(tvp, uvp, cmp)\
  ((tvp)->tv_sec cmp (uvp)->tv_sec ||\
  (tvp)->tv_sec == (uvp)->tv_sec &&\
  (tvp)->tv_usec cmp (uvp)->tv_usec)
#endif

void
NewNet::RateLimiter::flush()
{
  /* Flush all entries that are older than MAX_HISTORY second(s) */
  struct timeval tv;
  gettimeofday(&tv, 0);
  tv.tv_sec -= MAX_HISTORY;
  while((! m_Data->rateData.empty()) && timercmp(&tv, &m_Data->rateData.front().first, >))
    m_Data->rateData.erase(m_Data->rateData.begin());
}

long
NewNet::RateLimiter::nextWindow()
{
  flush();

  if(m_Limit == -1)
    return 0;
  else if(m_Limit == 0)
    return 60000;

  ssize_t total = 0;
  std::vector<RateData>::reverse_iterator it, end = m_Data->rateData.rend();
  for(it = m_Data->rateData.rbegin(); it != end; ++it)
  {
    total += (*it).second;
    if(total >= m_Limit)
    {
      /* Rate limit will be 'unbreached' one second after this frame. */

      struct timeval now;
      gettimeofday(&now, 0);

      struct timeval tv((*it).first);
      tv.tv_sec += 1;

      long d = difftime(tv, now);
      if(d <= 0)
        return 0;
      else
        return d;
    }
  }

  return 0;
}

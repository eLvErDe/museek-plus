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

#include "nnlog.h"
#include <stdarg.h>
#include <stdio.h>
#include <algorithm>

NewNet::Log NewNet::log;

void NewNet::Log::operator()(const std::string & domain, const char * fmt, ...)
{
  bool enabled = std::find(m_EnabledDomains.begin(), m_EnabledDomains.end(), domain) != m_EnabledDomains.end();
  if(! (enabled || m_AllEnabled))
    return;

  int size = 100;
  char * p, * np;
  if((p = (char *)malloc(size)) == 0)
    return;

  while(1)
  {
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(p, size, fmt, ap);
    va_end(ap);
    if(n > -1 && n < size)
    {
      LogNotify notice;
      notice.domain = domain;
      notice.message = p;
      logEvent(&notice);
      free(p);
      return;
    }
    if(n > -1)
      size = n + 1;
    else
      size *= 2;
    if((np = (char *)realloc(p, size)) == 0)
    {
      free(p);
      return;
    }
    else
      p = np;
  }
}

void NewNet::Log::enable(const std::string & domain)
{
  if(domain == "ALL")
    m_AllEnabled = true;
  else if(std::find(m_EnabledDomains.begin(), m_EnabledDomains.end(), domain) == m_EnabledDomains.end())
    m_EnabledDomains.push_back(domain);
}

void NewNet::Log::disable(const std::string & domain)
{
  if(domain == "ALL")
  {
    m_AllEnabled = false;
    return;
  }
  std::vector<std::string>::iterator it;
  it = std::find(m_EnabledDomains.begin(), m_EnabledDomains.end(), domain);
  if(it != m_EnabledDomains.end())
    m_EnabledDomains.erase(it);
}

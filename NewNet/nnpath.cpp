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

#include "nnpath.h"
#include <assert.h>
#include <limits.h>
#include "platform.h"

NewNet::Path::Path(const std::vector<std::string> & path)
{
  if(path.empty())
    return;

  std::vector<std::string>::const_iterator it, begin = path.begin(), end = path.end();
  for(it = begin; it != end; ++it)
  {
    if(it != begin)
      m_Path += separator();
    m_Path += (*it);
  }
}

bool
NewNet::Path::isAbsolute() const
{
#ifndef WIN32
  return ((! m_Path.empty()) && (m_Path[0] == '/'));
#else
  return ((m_Path.length() >= 3) && (m_Path.substr(1, 2) == ":\\"));
#endif // ! WIN32
}

std::vector<std::string>
NewNet::Path::split() const
{
  std::vector<std::string> ret;
  if(m_Path.empty())
    return ret;

  std::string path(m_Path);
  std::string::size_type ix = path.find(separator());
  while(ix != std::string::npos)
  {
    ret.push_back(path.substr(0, ix));
    path = path.substr(ix + 1);
    ix = path.find(separator());
  }
  ret.push_back(path);
  return ret;
}

NewNet::Path
NewNet::Path::simplified() const
{
  std::string::size_type minElements = isAbsolute() ? 1 : 0;
  std::vector<std::string> s(split()), r;
  std::vector<std::string>::iterator it, begin = s.begin(), end = s.end();
  for(it = begin; it != end; ++it)
  {
    if(((*it) == ".") || ((it != begin) && (*it).empty()))
      continue;
    else if((*it) == "..")
    {
      if(r.size() > minElements)
        r.pop_back();
      else if(minElements == 0)
        r.push_back(*it);
    }
    else
      r.push_back(*it);
  }
  return Path(r);
}

NewNet::Path
NewNet::Path::absolute(const std::string & base_) const
{
  if(isAbsolute())
    return *this;

  Path base(base_);
  if(base.path().empty())
    base = currentDir();
  else if(! base.isAbsolute())
    base = base.absolute();

  if(base.path().empty() || (base.path()[base.path().length() - 1] != separator()))
    return Path(base.path() + separator() + m_Path);
  else
    return Path(base.path() + m_Path);
}

NewNet::Path
NewNet::Path::currentDir()
{
  std::string path;
  char * buf = new char[PATH_MAX];
  assert(getcwd(buf, PATH_MAX) != 0);
  path = buf;
  delete [] buf;
  return Path(path);
}

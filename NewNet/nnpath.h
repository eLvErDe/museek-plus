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

#ifndef NEWNET_PATH_H
#define NEWNET_PATH_H

#include <string>
#include <vector>
#include "nnobject.h"

namespace NewNet
{
  //! Helper class for file-path manipulation
  /*! This class provides helper functions for dealing with file-path
      manipulation */
  class Path
  {
  public:
    //! Constructor
    /*! Default constructor leaves the path empty */
    Path()
    {
    }

    //! Constructor.
    /*! Point at a specific path. */
    Path(const std::string & path) : m_Path(path)
    {
    }

    //! Constructor.
    /*! Constructs the path from a vector of path pieces. */
    Path(const std::vector<std::string> & path);

    //! Return the current path as a string
    const std::string & path() const
    {
      return m_Path;
    }

    //! Return the platform's directory seperator.
    /*! Return the platform's directory seperator. */
    static char separator()
    {
#ifndef WIN32
      return '/';
#else
      return '\\';
#endif // ! WIN32
    }

    //! Check if path is absolute.
    /*! Checks if the current path is absolute. */
    bool isAbsolute() const;

    //! Split the path.
    /*! Split the path at the directory separator. */
    std::vector<std::string> split() const;

    //! Return a simplified version of the path.
    /*! This removes any unnecessary and resolvable '..', '.' and empty
        elements from the path. */
    Path simplified() const;

    //! Return an absolute version of the current path.
    /*! Return an absolute version of the current path. If the path
        isn't absolute already, base will be used as a base path. If
        base is an empty string, the current working directory will be used */
    Path absolute(const std::string & base = std::string()) const;

    //! Return the current working directory.
    /*! This returns the current working directory */
    static Path currentDir();

  private:
    std::string m_Path;
  };
}

#endif // NEWNET_PATH_H

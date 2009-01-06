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

#ifndef NEWNET_LOG_H
#define NEWNET_LOG_H

#include <string>
#include <vector>
#include <iostream>
#include "nnevent.h"

namespace NewNet
{
  //! Controllable logging class
  /*! This class will let you output messages to the console in a controlled
      fashion. It works by enabling and disabling domains where events
      can happen. */
  class Log {
  public:
    typedef struct
    {
      std::string domain;
      std::string message;
    } LogNotify;

    //! Print a message.
    /*! Print a message if the specified domain is enabled. */
    void operator() (const std::string & domain, const char*, ...);

    //! Enable a message domain.
    /*! This will enable printing messages of that domain. A special
        case is 'ALL' in which case all messages will be printed. */
    void enable(const std::string & domain);

    //! Disable a message domain.
    /*! This will disable printing messages of that domain. */
    void disable(const std::string & domain);

    //! Invoked when a message is logged.
    /*! This will be invoked when a message is logged in a domain that's
        enabled. */
    NewNet::Event<const LogNotify *> logEvent;

  private:
    bool m_AllEnabled;
    std::vector<std::string> m_EnabledDomains;
  };

  //! Console output class.
  /*! Connect an instance of this class to Log's logEvent event to output
      log messages to stderr. */
  class ConsoleOutput : public Event<const Log::LogNotify *>::Callback
  {
  public:
#ifndef DOXYGEN_UNDOCUMENTED
    void operator()(const Log::LogNotify * notice)
    {
      std::cout << "[" << notice->domain << "] " << notice->message << std::endl;
    }
#endif
  };

  //! Global logger instance.
  /*! Global logger instance, you can also reference this using the convenient
      NNLOG preprocessor definition. */
  extern Log log;
}

#define NNLOG NewNet::log

#endif // NEWNET_LOG_H

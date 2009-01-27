/*  Museek - A SoulSeek client written in C++
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

#ifndef MUSEEK_IFACESOCKET_H
#define MUSEEK_IFACESOCKET_H

#include "messageprocessor.h"
#include "ifacemessages.h"
#include <NewNet/nnclientsocket.h>
#include <NewNet/nnrefptr.h>

namespace NewNet
{
  class Reactor;
}

namespace Museek
{
  class IfaceSocket : public NewNet::ClientSocket, public MessageProcessor
  {
  public:
    IfaceSocket();
    ~IfaceSocket();

    const std::string & challenge()
    {
      return m_Challenge;
    }
    void setChallenge(const std::string & challenge)
    {
      m_Challenge = challenge;
    }

    bool authenticated() const
    {
      return m_Authenticated;
    }
    void setAuthenticated(bool authenticated)
    {
      m_Authenticated = authenticated;
    }

    unsigned int mask() const
    {
      return m_Mask;
    }
    void setMask(unsigned int mask)
    {
      m_Mask = mask;
    }

    void setCipherKey(const std::string & key)
    {
      cipherKeySHA256(m_CipherContext, (char *)key.data(), key.size());
    }

    CipherContext * cipherContext()
    {
      return m_CipherContext;
    }

    void sendMessage(const NewNet::Buffer & message);

    void onCannotConnect(NewNet::ClientSocket *);

    #define MAP_MESSAGE(ID, TYPE, EVENT) NewNet::Event<const TYPE *> EVENT;
    #define MAP_C_MESSAGE(ID, TYPE, EVENT) NewNet::Event<const TYPE *> EVENT;
    #include "ifaceeventtable.h"
    #undef MAP_MESSAGE
    #undef MAP_C_MESSAGE

  private:
    void onMessageReceived(const MessageData * data);

    bool m_Authenticated;
    unsigned int m_Mask;
    std::string m_Challenge;
    CipherContext * m_CipherContext;
  };
}

#endif // MUSEEK_IFACESOCKET_H

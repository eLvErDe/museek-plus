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

#ifndef MUSEEK_MESSAGEPROCESSOR_H
#define MUSEEK_MESSAGEPROCESSOR_H

#include "mutypes.h"
#include <NewNet/nnevent.h>

/* Forward declarations. */
namespace NewNet
{
  class ClientSocket;
}

namespace Museek
{
  /* MessageProcessor is a mix-in class that provides the means to process
     soulseek message packets coming in on a socket. */
  class MessageProcessor
  {
  public:
    /* Constructor. codeSize defines how wide the messageCode parameter will
       be. 1 is used for handshake socket, other types use 4. */
    MessageProcessor(uint codeSize) : m_CodeSize(codeSize)
    {
    }

    /* Data that's provided when messageReceivedEvent is emitted. */
    struct MessageData
    {
      /* Pointer to the socket that received the message. */
      NewNet::ClientSocket * socket;
      /* Message type / message code. Identifies what kind of message this
         is. */
      uint32 type;
      /* Length of the data body. */
      uint32 length;
      /* Pointer to the data that's part of the message. */
      const unsigned char * data;
    };

    /* Emitted when a complete message has been received and parsed. */
    NewNet::Event<const MessageData *> messageReceivedEvent;

    /* Data received event handler. Connect a client socket's
       dataReceivedEvent to this. */
    void onDataReceived(NewNet::ClientSocket * socket)
    {
      /* Keep parsing messages until we run out of data. */
      while(parseMessage(socket))
      {
      }
    }

  private:
    /* Size of the message type, see constructor comment for more info. */
    uint m_CodeSize;
    /* Try to parse a complete message from the socket's receive buffer. */
    bool parseMessage(NewNet::ClientSocket * socket);
  };
}

#endif // MUSEEK_MESSAGEPROCESSOR_H

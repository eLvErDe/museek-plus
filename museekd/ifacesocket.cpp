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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif // HAVE_CONFIG_H
#include "ifacesocket.h"
#include <NewNet/nnreactor.h>

Museek::IfaceSocket::IfaceSocket() : NewNet::ClientSocket(), MessageProcessor(4), m_Authenticated(false)
{
  m_CipherContext = new CipherContext();
  dataReceivedEvent.connect(this, &IfaceSocket::onDataReceived);
  messageReceivedEvent.connect(this, &IfaceSocket::onMessageReceived);
  cannotConnectEvent.connect(this, &IfaceSocket::onCannotConnect);
}

Museek::IfaceSocket::~IfaceSocket()
{
  NNLOG("museekd.iface.debug", "IfaceSocket destroyed");
  free(m_CipherContext);
}

void
Museek::IfaceSocket::sendMessage(const NewNet::Buffer & buffer)
{
  if(socketState() != SocketConnected)
  {
    NNLOG("museekd.iface.warn", "Trying to send message over closed socket...");
    return;
  }

  unsigned char buf[4];
  buf[0] = buffer.count() & 0xff;
  buf[1] = (buffer.count() >> 8) & 0xff;
  buf[2] = (buffer.count() >> 16) & 0xff;
  buf[3] = (buffer.count() >> 24) & 0xff;
  send(buf, 4);
  send(buffer.data(), buffer.count());
}

void
Museek::IfaceSocket::onMessageReceived(const MessageData * data)
{
  if((! authenticated()) && (data->type != 2))
  {
    NNLOG("museekd.iface.warn", "Attempt to bypass security detected, disconnecting interface.");
    data->socket->disconnect();
    return;
  }

  switch(data->type)
  {
    #define MAP_MESSAGE(ID, TYPE, EVENT) \
      case ID: \
      { \
        NNLOG("museek.messages.iface", "Received interface message " #TYPE "."); \
        TYPE msg; \
        msg.setIfaceSocket(this); \
        msg.parse_network_packet(data->data, data->length); \
        EVENT(&msg); \
        break; \
      }
    #define MAP_C_MESSAGE(ID, TYPE, EVENT) \
      case ID: \
      { \
        NNLOG("museek.messages.iface", "Received interface message " #TYPE "."); \
        TYPE msg(m_CipherContext); \
        msg.setIfaceSocket(this); \
        msg.parse_network_packet(data->data, data->length); \
        EVENT(&msg); \
        break; \
      }
    #include "ifaceeventtable.h"
    #undef MAP_MESSAGE
    #undef MAP_C_MESSAGE

    default:
        NNLOG("museekd.iface.warn", "Received unknown interface message, type: %u, length: %u", data->type, data->length);
        NetworkMessage msg;
        msg.parse_network_packet(data->data, data->length);
  }
}

void
Museek::IfaceSocket::onCannotConnect(NewNet::ClientSocket *)
{
  disconnect();
}

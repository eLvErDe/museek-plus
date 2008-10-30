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
#include "messageprocessor.h"
#include <NewNet/nnclientsocket.h>

bool
Museek::MessageProcessor::parseMessage(NewNet::ClientSocket * socket)
{
  /* How many bytes do we have in store? */
  size_t count = socket->receiveBuffer().count();
  /* Set up an easy-access pointer to the buffer. */
  uchar * inbuf = socket->receiveBuffer().data();

  /* If we have less than 4 + m_CodeSize bytes, bail out:
     4 bytes for the message length
     m_CodeSize bytes for the message type. */
  if(count < (4 + m_CodeSize))
    return false;

  /* Unpack the message length. 32bit little endian, that's the slsk way. */
  uint32 len = inbuf[0] + (inbuf[1] << 8) + (inbuf[2] << 16) + (inbuf[3] << 24);

  /* Unpack the message type. 8 or 32bit little endian depending on m_CodeSize */
  uint32 mtype = 0;
  for(uint j = 0; j < m_CodeSize; ++j)
    mtype += inbuf[4 + j] << (j * 8);

  /* 'len' includes the message type, so if we have less than len + 4 bytes in
     the buffer, bail out. */
  if(count < (len + 4))
    return false;

  /* A complete message is here. Set up the message data structure and emit
     messageReceivedEvent. */
  struct MessageData messageData;
  messageData.socket = socket;
  messageData.type = mtype;
  messageData.length = len - m_CodeSize;
  messageData.data = inbuf + 4 + m_CodeSize;
  messageReceivedEvent(&messageData);

  /* This happens if the socket descriptor is transferred to another socket
     instance. Bail out and terminate all processing, somebody else owns
     this buffer now. */
  if(socket->receiveBuffer().empty())
    return false;

  /* Seek to the end of the message. */
  socket->receiveBuffer().seek(len + 4);

  /* Is the buffer empty? Stop processing. If it's not, keep processing. */
  return ! socket->receiveBuffer().empty();
}

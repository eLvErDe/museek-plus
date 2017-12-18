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
#include <limits.h>

bool
Museek::MessageProcessor::parseMessage(NewNet::ClientSocket * socket)
{
    /* How many bytes do we have in store? */
    size_t count = socket->receiveBuffer().count();

    /* Set up an easy-access pointer to the buffer. */
    uchar * inbuf = socket->receiveBuffer().data();

    uint keySize = 0;
    if (obfuscated()) {
        keySize = 4;
    }

    /* If we have less than 4 + m_CodeSize bytes, bail out:
       4 bytes for the message length
       m_CodeSize bytes for the message type. */
    if(count < (4 + m_CodeSize + keySize))
        return false;

    uint32 len = getMessageLen(inbuf);

    /* 'len' includes the message type, so if we have less than len + 4 bytes in
       the buffer, bail out. */
    if(count < (len + 4 + keySize))
      return false;

    if (obfuscated()) {
        decodeMessage(inbuf, len);
    }

    /* Unpack the message type. 8 or 32bit little endian depending on m_CodeSize */
    uint32 mtype = 0;
    for(uint j = keySize; j < m_CodeSize + keySize; ++j)
        mtype += inbuf[4 + j] << (j * 8);

    /* A complete message is here. Set up the message data structure and emit
       messageReceivedEvent. */
    struct MessageData messageData;
    messageData.socket = socket;
    messageData.type = mtype;
    messageData.length = len - m_CodeSize;
    messageData.data = inbuf + 4 + m_CodeSize + keySize;
    messageReceivedEvent(&messageData);

    /* This happens if the socket descriptor is transferred to another socket
       instance. Bail out and terminate all processing, somebody else owns
       this buffer now. */
    if(socket->receiveBuffer().empty())
        return false;

    /* Seek to the end of the message. */
    socket->receiveBuffer().seek(len + 4 + keySize);

    /* Is the buffer empty? Stop processing. If it's not, keep processing. */
    return ! socket->receiveBuffer().empty();
}

void
Museek::MessageProcessor::rotrKey(unsigned char *buf, unsigned int c)
{
    const unsigned int mask = (CHAR_BIT*sizeof(buf) - 1);

    uint32_t key_orig = buf[0] + (buf[1] << 8) + (buf[2] << 16) + (buf[3] << 24);

    c &= mask;
    uint32_t key_rotated = (key_orig>>c) | (key_orig<<( (-c)&mask ));

    buf[0] = key_rotated & 0xff;
    buf[1] = (key_rotated >> 8) & 0xff;
    buf[2] = (key_rotated >> 16) & 0xff;
    buf[3] = (key_rotated >> 24) & 0xff;
}


/*
 * Decodes an unobfuscted message directly in the passed buffer
 * Only <len> bytes will be decoded (not counting the key size)
 */
void
Museek::MessageProcessor::decodeMessage(unsigned char *buf, uint32 len)
{
    // First read the key
    uint keySize = 4;
    uchar * key = new uchar[keySize]; // The key we will use to decode the msg

    for(uint j = 0; j < keySize; ++j) {
        key[j] = buf[j];
    }

    // Then decode the asked content length
    uint32 key_pos = 0;
    for (uint i = keySize; i < (len + keySize + 4); i++) {
        key_pos = i % keySize;
        if (key_pos == 0) {
            rotrKey(key, 31);
        }
        buf[i] = buf[i] ^ key[key_pos];
    }

    delete[] key;
}

uint32
Museek::MessageProcessor::getMessageLen(const unsigned char *inbuf)
{
    if (obfuscated()) {
        // Decode only the message length
        uchar * msg_len = new uchar[8]; // 8 = keysize + 4 bytes for message length
        for(uint j = 0; j < 8; ++j)
            msg_len[j] = inbuf[j];
        decodeMessage(msg_len, 4);

        /* Unpack the message length. 32bit little endian, that's the slsk way. */
        uint32 len = msg_len[4] + (msg_len[5] << 8) + (msg_len[6] << 16) + (msg_len[7] << 24);
        delete[] msg_len;

        return len;
    }
    else {
        /* Unpack the message length. 32bit little endian, that's the slsk way. */
        return inbuf[0] + (inbuf[1] << 8) + (inbuf[2] << 16) + (inbuf[3] << 24);
    }
}

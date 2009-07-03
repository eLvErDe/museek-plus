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
#include "networkmessage.h"
#include <cstdio>
#include <zlib.h>
#include <sstream>
#include <iomanip>

/* Pack a string. trslash indicates wether / to \ translation is in order,
   used to convert unix paths to slsk (win32) paths. */
void NetworkMessage::pack(const std::string& str, bool trslash)
{
  // Pack the string size
  pack((uint32)str.size());
  if (! trslash)
    // Append character data to the buffer directly
    buffer.append((unsigned char *)str.data(), str.size());
  else
    // Pack '/' as '\'. Pack everything else literally.
    for (uint32 i = 0; i < str.size(); i++)
      if(str[i] == '/')
        pack((uchar)'\\');
      else
        pack((uchar)str[i]);
}

/* Pack an IPv4 IP address. */
void NetworkMessage::pack_ip(const std::string& str)
{
    uint32 res = 0;
    const char *s, *s2;
    int i;
    long tmp;

    s = str.c_str();
    for (i = 0; i < 4; i++) {
        if (i > 0 && *s++ != '.')
            return;
        if (isspace(*s))  // because strtol() will skip whitespace
            return;
        tmp = strtol(s, (char **)&s2, 10);
        if (s2 == s || tmp < 0 || tmp > 255)
            return;
        res += ((uchar)tmp) << 8*i;
        s = s2;
    }

    pack(res);
}

/* Pack a raw byte array. */
void NetworkMessage::pack(const std::vector<uchar>& d)
{
  // Pack the array size.
  pack((uint32)d.size());
  // Pack the array data.
  std::vector<uchar>::const_iterator it, end = d.end();
  for(it = d.begin(); it != end; ++it)
    pack(*it);
}

/* Pack a 32bit unsigned integer (little-endian) */
void NetworkMessage::pack(uint32 i)
{
  unsigned char buf[4];
  buf[0] = i & 0xff;
  buf[1] = (i >> 8) & 0xff;
  buf[2] = (i >> 16) & 0xff;
  buf[3] = (i >> 24) & 0xff;
  buffer.append(buf, 4);
}

/* Pack a 32bit signed integer (little-endian) */
void NetworkMessage::pack(int32 i)
{
  unsigned char buf[4];
  buf[0] = i & 0xff;
  buf[1] = (i >> 8) & 0xff;
  buf[2] = (i >> 16) & 0xff;
  buf[3] = (i >> 24) & 0xff;
  buffer.append(buf, 4);
}

/* Pack a 64bit unsigned integer (file size / position). */
void NetworkMessage::pack(uint64 i)
{
  unsigned char buf[8];
  buf[0] = i & 0xff;
  buf[1] = (i >> 8) & 0xff;
  buf[2] = (i >> 16) & 0xff;
  buf[3] = (i >> 24) & 0xff;
  buf[4] = (i >> 32) & 0xff;
  buf[5] = (i >> 40) & 0xff;
  buf[6] = (i >> 48) & 0xff;
  buf[7] = (i >> 56) & 0xff;
  buffer.append(buf, 8);
}

/* Unpack a 32bit unsigned integer (little endian). */
uint32 NetworkMessage::unpack_int()
{
  // If we have less than 4 bytes, that's bad.
  if(buffer.count() < 4)
    return 0;
  unsigned char * buf = buffer.data();
  uint32 l = buf[0] + (buf[1] << 8) + (buf[2] << 16) + (buf[3] << 24);
  buffer.seek(4);
  return l;
}

/* Unpack a 32bit signed integer (little endian). */
int32 NetworkMessage::unpack_signed_int()
{
  // If we have less than 4 bytes, that's bad.
  if(buffer.count() < 4)
    return 0;
  unsigned char * buf = buffer.data();
  int32 l;
  if ((buf[3] & 0xf0) == 0xf0) // This is a negative int
    l = -1 - ((buf[0] ^ 0xff) + ((buf[1] ^ 0xff) << 8) + ((buf[2] ^ 0xff) << 16) + ((buf[3] ^ 0xff) << 24));
  else
    l = buf[0] + (buf[1] << 8) + (buf[2] << 16) + (buf[3] << 24);
  buffer.seek(4);
  return l;
}

/* Unpack a 64bit unsigned integer (file size / offset) */
uint64 NetworkMessage::unpack_off()
{
  // If we have less than 8 bytes, that's bad.
  if(buffer.count() < 8)
    return 0;
  unsigned char * buf = buffer.data();
  uint64 l = ((uint64)buf[0] << 0)  + ((uint64)buf[1] << 8)  +
            ((uint64)buf[2] << 16) + ((uint64)buf[3] << 24) +
            ((uint64)buf[4] << 32) + ((uint64)buf[5] << 40) +
            ((uint64)buf[6] << 48) + ((uint64)buf[7] << 56);
  buffer.seek(8);
  return l;
}

/* Unpack a string. */
std::string NetworkMessage::unpack_string()
{
  std::string x;

  // We need at least 4 bytes for the length.
  if(buffer.count() < 4)
    return x;

  // Unpack the string length.
  uint32 len = unpack_int();
  // Do we have enough bytes?
  if (buffer.count() < len)
    return x;

  // Copy the string data.
  x.assign((const char *)buffer.data(), len);
  buffer.seek(len);

  return x;
}

/* Unpack an IPv4 IP address. */
std::string NetworkMessage::unpack_ip()
{
  // We need at least 4 bytes of data.
  if(buffer.count() < 4)
    return "0.0.0.0";

  unsigned char * buf = buffer.data();
  char _ip[16];
  // Funky formatting.
  snprintf(_ip, 16, "%u.%u.%u.%u", buf[3], buf[2], buf[1], buf[0]);
  buffer.seek(4);
  return std::string(_ip);
}

/* Unpack a raw data array. */
std::vector<uchar> NetworkMessage::unpack_vector()
{
  std::vector<uchar> vec;

  // We need at least 4 bytes of data for the length.
  if(buffer.count() < 4)
    return vec;

  // Unpack the array length.
  uint32 len = unpack_int();
  // Bail out if we don't have enough data.
  if(buffer.count() < len)
    return vec;

  // Copy the array data.
  for(uint32 i = 0; i < len; i++)
    vec.push_back(unpack_char());

  return vec;
}

/* Unpack a raw data array of the rest of the message. */
std::vector<uchar> NetworkMessage::unpack_raw_message()
{
  std::vector<uchar> vec;

  // We need at least 4 bytes of data for the length.
  if(buffer.count() == 0)
    return vec;

  // Unpack the array length.
  uint32 len = buffer.count();

  // Copy the array data.
  for(uint32 i = 0; i < len; i++)
    vec.push_back(unpack_char());

  return vec;
}

/* Compress the message using zlib. */
void NetworkMessage::compress()
{
  // Pop the message type, that's not to be compressed.
  uint32 _mtype = unpack_int();

  // Calculate estimated output size and allocate buffer.
  uLong outbuf_len = (int)(buffer.count() * 1.1 + 12.0);
  uchar * outbuf = new uchar[outbuf_len];

  // Call zlib's compress function to compress the buffer.
  if (::compress((Bytef *)outbuf, &outbuf_len, (const Bytef *)buffer.data(), buffer.count()) == Z_OK)
  {
    // Seek to the end of the message buffer.
    buffer.seek(buffer.count());
    // Pack the message type.
    pack(_mtype);
    // Pack the compressed data.
    buffer.append(outbuf, outbuf_len);
  }
  else
  {
    // Ok, this might need some improvement.
    NNLOG("museekd.warn", "Corrupted message created (compression error).");
    buffer.seek(buffer.count());
  }

  // Delete the allocated buffer.
  delete [] outbuf;
}

// Allocate 1MByte as a default result buffer.
#define DEFAULTALLOC 1000000
// Decompress the network message data.
void NetworkMessage::decompress()
{
  // Intialize the zlib stream.
  z_stream zst;
  zst.zalloc = (alloc_func)NULL;
  zst.zfree = (free_func)NULL;

  // Set input buffer.
  zst.avail_in = buffer.count();
  zst.next_in = (Bytef*)buffer.data();

  // Allocate and set output buffer.
  NewNet::Buffer output_buffer;
  zst.avail_out = DEFAULTALLOC;
  uchar * outbuf = new uchar[DEFAULTALLOC];
  assert(outbuf != 0);
  zst.next_out = (Bytef*)outbuf;

  // Initialize zlib's decompression routines.
  int err = inflateInit(&zst);
  if (err != Z_OK)
  {
    // Something went horrible wrong.
    buffer.seek(buffer.count());
    delete [] outbuf;
    if (err != Z_MEM_ERROR)
      inflateEnd(&zst);
    NNLOG("museekd.warn", "Corrupted packet encountered (decompression error).");
    return;
  }

  do {
    // Try to decompress the buffer.
    err = inflate(&zst, Z_FINISH);
    switch(err)
    {
      // Cool, we're finished.
      case Z_STREAM_END:
        break;
      // Problem with the buffer
      case Z_BUF_ERROR:
        if (zst.avail_out > 0)
        {
          // Bad stuff...
          inflateEnd(&zst);
          buffer.seek(buffer.count());
          delete [] outbuf;
          NNLOG("museekd.warn", "Corrupted packet encountered (decompression error).");
          return;
        }
      case Z_OK:
        // We're out of memory. Push the data to the buffer and continue.
        output_buffer.append(outbuf, DEFAULTALLOC - zst.avail_out);
        zst.avail_out = DEFAULTALLOC;
        zst.next_out = (Bytef*)outbuf;
        break;
      default:
        // Bad stuff...
        NNLOG("museekd.warn", "Corrupted packet encountered (decompression error).");
        inflateEnd(&zst);
        buffer.seek(buffer.count());
        delete [] outbuf;
        return;
    }
  } while (err != Z_STREAM_END);

  // Append the rest of the decompressed data to the buffer.
  output_buffer.append(outbuf, DEFAULTALLOC - zst.avail_out);
  delete [] outbuf;

  // We're finished, clean up.
  inflateEnd(&zst);

  // Seek past the compressed data.
  buffer.seek(buffer.count());
  // Append the output buffer to the message buffer.
  buffer.append(output_buffer.data(), output_buffer.count());
}
#undef DEFAULTALLOC

void NetworkMessage::garbage_collector() {
    std::vector<uchar> raw = unpack_raw_message();
    if (raw.size() > 0) {
        std::vector<uchar>::const_iterator itraw = raw.begin();
        std::stringstream hexContent;
        hexContent.setf(std::ios_base::hex, std::ios::basefield);
        hexContent.setf(std::ios_base::uppercase);
        hexContent.fill('0');
        for (; itraw != raw.end(); itraw++) {
            hexContent << ' ' << std::setw(2) << (uint16_t) *itraw;
        }
        NNLOG("protocol.warn", "Unexpected message content for message %s: %s", get_name().c_str(), hexContent.str().c_str());
    }
}

void NetworkMessage::default_garbage_collector() {
    std::vector<uchar> raw = unpack_raw_message();
    std::vector<uchar>::const_iterator itraw = raw.begin();
    std::stringstream hexContent;
    hexContent.setf(std::ios_base::hex, std::ios::basefield);
    hexContent.setf(std::ios_base::uppercase);
    hexContent.fill('0');
    for (; itraw != raw.end(); itraw++) {
        hexContent << ' ' << std::setw(2) << (uint16_t) *itraw;
    }
    NNLOG("protocol.warn", "Unexploited data in message: %s", hexContent.str().c_str());
}

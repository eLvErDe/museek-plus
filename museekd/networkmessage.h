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

#ifndef MUSEEK_NETWORKMESSAGE_H
#define MUSEEK_NETWORKMESSAGE_H

#include "mutypes.h"

#include <string>
#include <vector>
#include <iostream>
#include <sys/types.h>
#include <NewNet/nnbuffer.h>
#include <NewNet/nnlog.h>

/* This declares a GenericMessage. It's not used at the moment, but it could
   be in the future. */
class GenericMessage
{
    public:
        GenericMessage()
        {
        }

        virtual ~GenericMessage()
        {
        }
};

/* Voodoo magic preprocessing: build packet suitable for transmission. */
#define MAKE virtual const NewNet::Buffer & make_network_packet() { pack(get_type());
#define END_MAKE return buffer; };
/* Voodoo magic preprocessing: extract values from a buffer. */
#define PARSE virtual void unsafe_parse_network_packet() {
#define END_PARSE garbage_collector();};

/* NetworkMessage class. Base class for all networked messages and provides
   handy functions for packing and unpacking values and compressing and
   decompressing network buffers. */
class NetworkMessage: public GenericMessage
{
public:
  /* This is where data gets stored when you call make_network_packet(). */
  NewNet::Buffer buffer;

  /* Default make_network_packet. Packs nothing. */
  MAKE
  END_MAKE

  /* Default unsafe_parse_network_packet. Parse raw data. */
  PARSE
    default_garbage_collector(); // This is used when an unknown message is received
  END_PARSE

  /* Wrapper around unsafe_parse_network_packet: catch out of memory
     exceptions. */
  virtual void parse_network_packet(const unsigned char * data, size_t count)
  {
    buffer.append(data, count);
    try
    {
      unsafe_parse_network_packet();
    }
    catch(std::bad_alloc e)
    {
      NNLOG("museekd.warn", "Ran out of memory while unpacking message!");
    }
  }

protected:
  /* Pack a string. */
  void pack(const std::string&, bool=false);
  /* Pack an IP address. */
  void pack_ip(const std::string&);
  /* Pack a raw data array. */
  void pack(const std::vector<uchar>&);
  /* Pack a 32bit unsigned integer. */
  void pack(uint32);
  /* Pack a 32bit signed integer. */
  void pack(int32);
  /* Pack a single raw 8bit element. */
  void pack(uchar c)
  {
    buffer.append(&c, 1);
  }
  /* Pack a 64bit unsigned integer. */
  void pack(uint64);

  /* Unpack a string. */
  std::string unpack_string();
  /* Unpack raw data. */
  std::vector<uchar> unpack_vector();
  /* Unpack raw data of the rest of the message. */
  std::vector<uchar> unpack_raw_message();
  /* Unpack an IP address. */
  std::string unpack_ip();
  /* Unpack a 32bit unsigned integer. */
  uint32 unpack_int();
  /* Unpack a 32bit signed integer. */
  int32 unpack_signed_int();
  /* Unpack a 64bit unsigned integer. */
  uint64 unpack_off();
  /* Unpack a raw 8bit element. */
  uchar unpack_char()
  {
    uchar c = 0; // Default value
    if(! buffer.empty())
    {
      c = buffer.data()[0]; // Grab next byte in buffer
      buffer.seek(1); // Seek forward one byte
    }
    else
    {
      // Buffer was empty, message is invalid, print message
      NNLOG("museekd.warn", "Corrupted message encountered (unpack from empty buffer).");
    }
    // Return the value.
    return c;
  }

  /* Compress the message. */
  void compress();
  /* Decompress the message. */
  void decompress();

  /* Handle unexpected data in messages */
  void garbage_collector();

  /* Handle unexploited data */
  virtual void default_garbage_collector();

  /* Return the message name. */
  virtual std::string get_name()
  {
    return std::string();
  }

private:
  /* Return the message type identifier. Note that if you use the MAKE and
     END_MAKE macros in your own messages you can redefine get_type to be of
     a different type than unsigned char. */
  unsigned char get_type()
  {
    return 0;
  }
};

/* Voodoo magic preprocessing: shortcut for defining a network message. */
#define NETWORKMESSAGE(parent, mtype, m_id) \
  class mtype : public parent \
  { \
  private: \
    uint32 get_type() { return m_id; } \
  protected: \
    std::string get_name() { return #mtype ; } \
  public:
#define END };

#endif // MUSEEK_NETWORKMESSAGE_H

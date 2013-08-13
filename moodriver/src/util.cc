//  MooDriver - Museekd C++ client library 
//  Copyright (C) 2006 M. Derezynski
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include <cstdlib>
#include <cstring>
#include <cctype>
#include <vector>
#include <string>

#include <glib.h>
#include "util.hh"
#include "md5.hh"

namespace Util
{
    char*
    hexify (char *pass, int len)
    {
      char *hash;
      char *bp;
      char hexchars[] = "0123456789abcdef";
      int i;

      hash = static_cast<char *> (g_new0 (char, 33));
      bp = hash;

      for(i = 0; i < len; i++)
        {
          *(bp++) = hexchars[(pass[i] >> 4) & 0x0f];
          *(bp++) = hexchars[pass[i] & 0x0f];
        }
      *bp = 0;
      return hash;
    }

    std::string
    md5_hex (char* const data, size_t data_size)
    {
        md5_state_t  md5state;
        char         md5pword[16];

        md5_init (&md5state);
        md5_append (&md5state, (unsigned const char *)data, data_size); 
        md5_finish (&md5state, (unsigned char *)md5pword);
        char *md5_hex = Util::hexify (md5pword, sizeof(md5pword));
        std::string md5_hex_std = md5_hex;
        free (md5_hex);
        return md5_hex_std;
    }
}

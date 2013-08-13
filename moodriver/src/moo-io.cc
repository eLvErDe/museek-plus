// MooDriver - (C) 2006 M.Derezynski
//
// Code based on;
//
// ---
//
// giFTcurs - curses interface to giFT
// Copyright (C) 2001, 2002, 2003 Göran Weinholt <weinholt@dtek.chalmers.se>
// Copyright (C) 2003 Christian Häggström <chm@c00.info>
//

#include <vector>
#include <glib.h>
#include <moo/mucipher.h>

#include "moo-io.hh" 

namespace Moo
{
  namespace BasicIO
  {
    void
    write_data (GIOChannel *source, unsigned char* data, gsize size, gsize &written)
    {
      GError *error = 0;
      g_io_channel_write_chars (source, (char*)data, size, &written, &error);

      if (error)
        {
          g_message ("%s: Error: %s", G_STRFUNC, error->message);
          g_error_free (error);
        }
    }

    void
    write_data (GIOChannel *source, unsigned char* data, gsize size)
    {
      GError *error = 0;
      g_io_channel_write_chars (source, (char*)data, size, NULL, &error);


      if (error)
        {
          g_message ("%s: Error: %s", G_STRFUNC, error->message);
          g_error_free (error);
        }
    }
  }

  namespace IO
  {
    // Write Methods
    void 
    write_bool (GIOChannel *source, unsigned char c)
    {
      unsigned char *data = g_new0 (unsigned char, 1); 
      data[0] = c; 
      BasicIO::write_data (source, data, 1); 
      g_free (data);
    }

    void 
    write_uint (GIOChannel *source, guint uint)
    {
      unsigned char *data = g_new0 (unsigned char, sizeof(guint));
      for (guint n = 0; n < sizeof(guint); ++n) 
        {
          data[n] = (uint >> (n * 8)) & 0xFF;
        }
      BasicIO::write_data  (source, data, sizeof(guint));
      g_free (data);
    }

    void 
    write_off_t (GIOChannel *source, gint64 int64)
    {
      unsigned char *data = g_new0 (unsigned char, sizeof(gint64));
      for (guint n = 0; n < sizeof(gint64); ++n) 
        {
          data[n] = (int64 >> (n * 8)) & 0xFF;
        }
      BasicIO::write_data  (source, data, sizeof(gint64));
      g_free (data);
    }

    void 
    write_string (GIOChannel *source, const char* str) 
    {
      write_uint (source, strlen(str));
      BasicIO::write_data (source, (unsigned char*)(str), strlen(str));
    }

    // Live Read Methods
    guint
    read_uint (GIOChannel *source)
    {
      GError       *error  = 0;
      char         *data   = g_new0 (char, sizeof(guint));
      gsize         length = 0;
      guint         uint   = 0;

      g_io_channel_read_chars (source, data, sizeof(guint), &length, &error);

      if (error)
        {
          g_message ("%s: Error: %s", G_STRFUNC, error->message);
          g_error_free (error);
          return (guint)(0);
        }

      if (length == sizeof(guint))
        {
          for (guint n = 0; n < sizeof(guint); ++n) 
            {
              uint += (unsigned char)(data[n]) << (n * 8); 
            }
          g_free (data);
          return uint;
        }
      else
        {
          g_message ("uint: expected %d bytes, but got %d", sizeof(guint), length);
          return 0;
        }
    }

    // Unpack Read Methods 
    guchar
    unpack_uchar (const Data& read_data, guint& offset)
    {
      guchar value = (unsigned char)(read_data[offset++]);
      return value;
    }

    guint 
    unpack_uint (const Data& read_data, guint& offset)
    {
      guint value = 0; 
      value += guint(unpack_uchar (read_data, offset)) << 0; 
      value += guint(unpack_uchar (read_data, offset)) << 8; 
      value += guint(unpack_uchar (read_data, offset)) << 16; 
      value += guint(unpack_uchar (read_data, offset)) << 24; 
      return value;
    }

    gint64 
    unpack_off_t (const Data& read_data, guint& offset)
    {
      gint64 value = 0;
      value += gint64(unpack_uchar (read_data, offset)) << 0; 
      value += gint64(unpack_uchar (read_data, offset)) << 8; 
      value += gint64(unpack_uchar (read_data, offset)) << 16; 
      value += gint64(unpack_uchar (read_data, offset)) << 24; 
      value += gint64(unpack_uchar (read_data, offset)) << 32; 
      value += gint64(unpack_uchar (read_data, offset)) << 40; 
      value += gint64(unpack_uchar (read_data, offset)) << 48; 
      value += gint64(unpack_uchar (read_data, offset)) << 56; 
      return value;
    }

    std::string
    unpack_string (const Data& read_data, guint& offset)
    {
      std::string value;
      guint len = unpack_uint (read_data, offset);
      for (guint n = 0; n < len; ++n)
      {
        value += unpack_uchar (read_data, offset);
      }
      return value;
    }

    bool 
    unpack_bool (const Data& read_data, guint& offset)
    {
      return unpack_uchar (read_data, offset);
    }
  }

  namespace CipherIO
  {
    std::string
    decipher (const Moo::Data& read_data, guint& offset, CipherContext *ctx)
    {
      unsigned int l  = Moo::IO::unpack_uint (read_data, offset); 
      unsigned int lc = CIPHER_BLOCK(l);

      unsigned char *temp1, *temp2;
      temp1 = g_new0 (unsigned char, lc+1);
      temp2 = g_new0 (unsigned char, lc+1);

      for (unsigned int n = 0; n < lc; ++n)
      {
        temp1[n] = Moo::IO::unpack_uchar (read_data, offset);
      }

      blockDecipher (ctx, temp1, lc, temp2);
      std::string dec = (char*)temp2;
      g_free (temp1);
      g_free (temp2);
      return dec.substr (0, l);
    }

    unsigned char*
    cipher (const char *str, CipherContext *ctx, gsize& size)
    {
      unsigned int i = strlen(str);
      unsigned char *crypted = g_new0 (unsigned char, CIPHER_BLOCK(i));
      blockCipher (ctx, (unsigned char*)str, i, crypted); 
//      write_data (source, temp, CIPHER_BLOCK(strlen(str))); 
//      g_free (temp);
      size = CIPHER_BLOCK(strlen(str));
      return crypted; 
    }

  }
}

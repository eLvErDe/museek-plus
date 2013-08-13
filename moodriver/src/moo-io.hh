#ifndef MOO_IO_H
#define MOO_IO_H

#include <vector>
#include <string>

#include <glib.h>
#include <moo/types.hh>

#include "moo-types-private.hh"

namespace Moo
{
  namespace BasicIO
  {
    void
    write_data (GIOChannel *source, unsigned char* data, gsize size, gsize &written);
    void
    write_data (GIOChannel *source, unsigned char* data, gsize size);
  }

  namespace IO
  {
    void 
    write_bool    (GIOChannel *source, unsigned char c);
    void 
    write_uint    (GIOChannel *source, guint uint);
    void 
    write_off_t   (GIOChannel *source, gint64 int64);
    void 
    write_string  (GIOChannel *source, const char* str);

    guint
    read_uint     (GIOChannel *source);

    guchar
    unpack_uchar  (const Moo::Data& read_data, guint& offset);

    guint 
    unpack_uint   (const Moo::Data& read_data, guint& offset);

    gint64 
    unpack_off_t  (const Moo::Data& read_data, guint& offset);

    std::string
    unpack_string (const Moo::Data& read_data, guint& offset);

    bool 
    unpack_bool   (const Moo::Data& read_data, guint& offset);
  }

  namespace CipherIO
  {
    std::string
    decipher (const Moo::Data& read_data, guint& offset, CipherContext *ctx);

    unsigned char*
    cipher (const char *str, CipherContext *ctx, gsize& size);
  }
 
}
#endif

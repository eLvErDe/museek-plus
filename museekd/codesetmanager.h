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

#ifndef MUSEEK_CODESETMANAGER_H
#define MUSEEK_CODESETMANAGER_H

#include <NewNet/nnobject.h>
#include <NewNet/nnweakrefptr.h>
#include <string>
#include <map>
#include <iconv.h>

namespace Museek
{
  class Museekd;

  class CodesetManager : public NewNet::Object
  {
  public:
    CodesetManager(Museekd * museekd);

    /* Frees all stored iconv contexts. */
    ~CodesetManager();

    /* Return pointer to museekd instance. */
    Museekd * museekd() const
    {
      return m_Museekd;
    }

    /* Convert 'str' from character set 'from' to character set 'to' */
    std::string convert(const std::string & from, const std::string & to, const std::string & str);
    /* Convert 'str' from character set 'from' to UTF8 */
    std::string toUtf8(const std::string & from, const std::string & str)
    {
      return convert(from, "UTF-8", str);
    }
    /* Convert 'str' from UTF8 to character set 'to' */
    std::string fromUtf8(const std::string & to, const std::string & str)
    {
      return convert("UTF-8", to, str);
    }

    /* Convert 'str' from character set set for room 'room' to UTF8 */
    std::string fromRoom(const std::string & room, const std::string & str);
    /* Convert values in map from character set for room 'room' to UTF8 */
    std::map<std::string, std::string> fromRoomMap(const std::string & room, const std::map<std::string, std::string> & map);
    /* Convert 'str' from UTF8 to character set for room 'room' */
    std::string toRoom(const std::string & room, const std::string & str);

    /* Convert 'str' from character set set for peer 'peer' to UTF8 */
    std::string fromPeer(const std::string & peer, const std::string & str);
    /* Convert 'str' from UTF8 to character set for peer 'peer' */
    std::string toPeer(const std::string & peer, const std::string & str);

    /* Convert 'str' from filesystem encoding to network encoding, slashes = should we replace slashes? */
    std::string fromFSToNet(const std::string & str, bool slashes = true);
    /* Convert 'str' from network encoding to filesystem encoding, slashes = should we replace slashes? */
    std::string fromNetToFS(const std::string & str, bool slashes = true);

    /* Convert 'str' from filesystem encoding to a peer's encoding, slashes = should we replace slashes? */
    std::string fromFSToPeer(const std::string & peer, const std::string & str, bool slashes = true);
    /* Convert 'str' from a peer's encoding to filesystem encoding, slashes = should we replace slashes? */
    std::string fromPeerToFS(const std::string & peer, const std::string & str, bool slashes = true);

    /* Convert 'str' from network encoding to utf8 */
    std::string fromNet(const std::string & str);
    /* Convert 'str' from utf8 to network encoding*/
    std::string toNet(const std::string & str);

    /* Convert a path as received from Iface to filesystem (slashes) */
    std::string fromUtf8ToFS(const std::string & str, bool slashes = true);
    /* Convert a filesystem path to a path as sent to Iface (slashes) */
    std::string fromFsToUtf8(const std::string & str, bool slashes = true);

    /* Convert 'str' from utf8 to the network encoding */
    std::string fromUtf8ToNet(const std::string & str);
    /* Convert 'str' from the network encoding to utf8 */
    std::string fromNetToUtf8(const std::string & str);

  private:
    /* Get the character set for an object from the configuration */
    std::string getNetworkCodeset(const std::string & domain, const std::string & key) const;
    /* Get an iconv conversion context from character set 'from' to 'to'. */
    iconv_t getContext(const std::string & from, const std::string & to);

    /* Weak reference to museekd instance. */
    NewNet::WeakRefPtr<Museekd> m_Museekd;
    /* Iconv context cache. */
    std::map<std::pair<std::string, std::string>, iconv_t> m_Contexts;
  };
}

#endif // MUSEEK_CODESETMANAGER_H

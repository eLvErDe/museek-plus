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

#ifndef MUSEEK_MUSEEKD_H
#define MUSEEK_MUSEEKD_H

namespace NewNet
{
  class Reactor;
}

#include "servermessages.h"
#include <NewNet/nnrefptr.h>

namespace Museek
{
  /* Forward definitions for classes we use for the class definition. */
  class ConfigManager;
  class CodesetManager;
  class ServerManager;
  class PeerManager;
  class DownloadManager;
  class UploadManager;
  class SharesDatabase;
  class SearchManager;
  class IfaceManager;

  class Museekd : public NewNet::Object
  {
  public:
    Museekd(NewNet::Reactor * reactor = 0);
    ~Museekd();

    /* Generate the next unique (within this session) token. Used for
       identifying peer sockets and transfers. */
    int token()
    {
      return ++m_Token;
    }

    /* Return a pointer to the reactor. */
    NewNet::Reactor * reactor() const
    {
      return m_Reactor;
    }

    /* Return a pointer to the config manager. */
    ConfigManager * config() const
    {
      return m_Config;
    }

    /* Return a pointer to the codeset manager (codeset translator). */
    CodesetManager * codeset() const
    {
      return m_Codeset;
    }

    /* Return a pointer to the server manager (connection to the soulseek
       server). */
    ServerManager * server() const
    {
      return m_Server;
    }

    /* Return a pointer to the peer manager (handles incoming connections
       and passive / reverse connection requests. */
    PeerManager * peers() const
    {
      return m_Peers;
    }

    /* Return a pointer to the download manager. */
    DownloadManager * downloads() const
    {
      return m_Downloads;
    }

    /* Return a pointer to the upload manager. */
    UploadManager * uploads() const
    {
      return m_Uploads;
    }

    /* Return a pointer to the interface manager. */
    IfaceManager * ifaces() const
    {
      return m_Ifaces;
    }

     SharesDatabase * shares() const
    {
      return m_Shares;
    }

    SharesDatabase * buddyshares() const
    {
      return m_BuddyShares;
    }

    SearchManager * searches() const
    {
      return m_Searches;
    }

    void LoadShares();
    void LoadDownloads();

    bool isBanned(const std::string u);
    bool isIgnored(const std::string u);
    bool isTrusted(const std::string u);
    bool isBuddied(const std::string u);
    bool isPrivileged(const std::string u);
    bool toBuddiesOnly();
    bool haveBuddyShares();
    bool trustingUploads();
    bool autoClearFinishedDownloads();
    bool autoClearFinishedUploads();
    bool autoRetryDownloads();
    bool privilegeBuddies();
    uint upSlots();
    uint downSlots();
    void addPrivilegedUser(const std::string & user);
    void setPrivilegedUsers(const std::vector<std::string> & users);
    void sendSharedNumber();
    bool isEnabledPrivRoom();

  private:
    /* Our strong references to the various components. */
    NewNet::RefPtr<NewNet::Reactor> m_Reactor;
    NewNet::RefPtr<ConfigManager> m_Config;
    NewNet::RefPtr<CodesetManager> m_Codeset;
    NewNet::RefPtr<ServerManager> m_Server;
    NewNet::RefPtr<PeerManager> m_Peers;
    NewNet::RefPtr<DownloadManager> m_Downloads;
    NewNet::RefPtr<UploadManager> m_Uploads;
    NewNet::RefPtr<IfaceManager> m_Ifaces;
    NewNet::RefPtr<SharesDatabase> m_Shares, m_BuddyShares;
    NewNet::RefPtr<SearchManager> m_Searches;

    int m_Token;

    std::vector<std::string> mPrivilegedUsers; // The list of privileged users
  };
}

#endif // MUSEEK_MUSEEKD_H

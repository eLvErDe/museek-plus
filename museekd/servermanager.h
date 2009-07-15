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

#ifndef MUSEEK_SERVERMANAGER_H
#define MUSEEK_SERVERMANAGER_H

#include <NewNet/nnobject.h>
#include <NewNet/nnbuffer.h>
#include <NewNet/nnrefptr.h>
#include <NewNet/nnweakrefptr.h>
#include "tcpmessagesocket.h"
#include "servermessages.h"
#include "configmanager.h"
#include <string>

namespace NewNet
{
  class ClientSocket;
}

namespace Museek
{
  class Museekd;

  class ServerManager : public NewNet::Object
  {
  public:
    ServerManager(Museekd * museekd);
    ~ServerManager();

    const std::string & username() const
    {
      return m_Username;
    }

    Museekd * museekd() const
    {
      return m_Museekd;
    }

    TcpMessageSocket * socket()
    {
      return m_Socket;
    }

    bool loggedIn() const
    {
      return m_LoggedIn;
    }

    long getServerTimeDiff() const {
        return mServerTimeDiff;
    }

    void connect();
    void disconnect();

    std::vector<std::string> joinedRooms() {return m_JoinedRooms;};

    void sendMessage(const NewNet::Buffer & buffer);

    // Emitted when the login state changes
    NewNet::Event<bool> loggedInStateChangedEvent;

    NewNet::Event<long> receivedServerTimeDiff;

    bool isServerTimeTestMessage(const std::string&, const std::string&);

    #define MAP_MESSAGE(ID, TYPE, EVENT) NewNet::Event<const TYPE *> EVENT;
    #include "servereventtable.h"
    #undef MAP_MESSAGE

  protected:
    void setLoggedIn(bool loggedIn)
    {
      if(loggedIn != m_LoggedIn)
      {
        m_LoggedIn = loggedIn;
        loggedInStateChangedEvent(m_LoggedIn);
      }
    }

    void onConfigKeySet(const ConfigManager::ChangeNotify * data);
    void onConfigKeyRemoved(const ConfigManager::RemoveNotify * data);

    void onCannotConnect(NewNet::ClientSocket * socket);
    void onConnected(NewNet::ClientSocket *);
    void onDisconnected(NewNet::ClientSocket * socket);
    void onMessageReceived(const TcpMessageSocket::MessageData * data);
    void onLoggedIn(const SLogin *);
    void onRoomJoined(const SJoinRoom * message);
    void onRoomLeft(const SLeaveRoom * message);
    void onPrivilegedUsersReceived(const SPrivilegedUsers * message);
    void onPrivilegedUserAddedReceived(const SAddPrivileged * message);
    void onKicked(const SKicked * message);
    void onServerPrivateMessageReceived(const SPrivateMessage * message);

  private:
    void pingServer(long);
    void reconnect(long);
    void launchServerTimeTest(long);
    void serverTimeTestFailed(long);

    NewNet::WeakRefPtr<Museekd> m_Museekd;

    std::string m_Username, m_Password;

    NewNet::WeakRefPtr<TcpMessageSocket> m_Socket;
    bool m_LoggedIn, m_AutoConnect;

    std::vector<std::string> m_JoinedRooms;
    NewNet::WeakRefPtr<NewNet::Event<long>::Callback> m_PingTimeout, m_ReconnectTimeout;
    int m_ConnectionTries;
    struct timeval mLastSentMessage, mLastServerTimeTestTime;
    std::string mLastServerTimeTestString;
    long mServerTimeDiff;
    bool mTestingServerTime;
  };
}

#endif // MUSEEK_SERVERMANAGER_H

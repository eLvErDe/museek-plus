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
#include "servermanager.h"
#include "museekd.h"
#include "codesetmanager.h"
#include "configmanager.h"
#include "peermanager.h"
#include <NewNet/nnreactor.h>
#include <NewNet/nnlog.h>
#include <iostream>
#include <sstream>
#include <NewNet/util.h>

Museek::ServerManager::ServerManager(Museekd * museekd) : m_Museekd(museekd), m_LoggedIn(false)
{
    // Connect config event handlers
    museekd->config()->keySetEvent.connect(this, &ServerManager::onConfigKeySet);
    museekd->config()->keyRemovedEvent.connect(this, &ServerManager::onConfigKeyRemoved);

    // Connect local event handlers
    loggedInEvent.connect(this, &ServerManager::onLoggedIn);
    roomJoinedEvent.connect(this, &ServerManager::onRoomJoined);
    roomLeftEvent.connect(this, &ServerManager::onRoomLeft);
    privilegedUsersReceivedEvent.connect(this, & ServerManager::onPrivilegedUsersReceived);
    privilegedUserAddedEvent.connect(this, & ServerManager::onPrivilegedUserAddedReceived);
    kickedEvent.connect(this, & ServerManager::onKicked);
    privateMessageReceivedEvent.connect(this, &ServerManager::onServerPrivateMessageReceived);

    m_ConnectionTries = 0;
    m_AutoConnect = true;
    mServerTimeDiff = 0.0L;
    mTestingServerTime = false;
}

Museek::ServerManager::~ServerManager()
{
    if (m_ReconnectTimeout.isValid())
        museekd()->reactor()->removeTimeout(m_ReconnectTimeout);
}

void
Museek::ServerManager::connect()
{
  if(m_Socket.isValid())
  {
    NNLOG("museekd.server.warn", "Already connected to server.");
    return;
  }

  m_LoggedIn = false;

  std::string host = museekd()->config()->get("server", "host", "server.slsknet.org");
  unsigned int port = museekd()->config()->getUint("server", "port", 2242);

  if(host.empty())
  {
    NNLOG("museekd.server.warn", "No hostname for server specified.");
    return;
  }

  if(port == 0)
  {
    NNLOG("museekd.server.warn", "No port for server specified.");
    return;
  }

  m_Username = museekd()->config()->get("server", "username");
  m_Password = museekd()->config()->get("server", "password");
  if(m_Username.empty())
  {
    NNLOG("museekd.server.warn", "No username for server specified.");
    return;
  }

  // Set up auto-join
  m_JoinedRooms = museekd()->config()->keys("autojoin");

  // Create the TcpMessageSocket
  m_Socket = new TcpMessageSocket();

  // Connect the event handlers
  m_Socket->cannotConnectEvent.connect(this, &ServerManager::onCannotConnect);
  m_Socket->connectedEvent.connect(this, &ServerManager::onConnected);
  m_Socket->disconnectedEvent.connect(this, &ServerManager::onDisconnected);
  m_Socket->messageReceivedEvent.connect(this, &ServerManager::onMessageReceived);

  // Add the socket to the reactor
  museekd()->reactor()->add(m_Socket);

  // Reconnect automatically if problems
  m_AutoConnect = true;

  // Connect to the soulseek server
  m_Socket->connect(host, port);
}

void
Museek::ServerManager::disconnect()
{
  // Don't try to reconnect automatically as we explicitly tell we want to stay disconnected
  m_AutoConnect = false;

  if(m_Socket.isValid())
    m_Socket->disconnect();
}

/**
  * Reconnect to the server
  */
void
Museek::ServerManager::reconnect(long) {
    if (m_AutoConnect)
        connect();
}

void
Museek::ServerManager::sendMessage(const NewNet::Buffer & buffer)
{
  if(! m_Socket)
  {
    NNLOG("museekd.server.warn", "Trying to send message over closed socket...");
    return;
  }

  gettimeofday(&mLastSentMessage, 0);

  unsigned char buf[4];
  buf[0] = buffer.count() & 0xff;
  buf[1] = (buffer.count() >> 8) & 0xff;
  buf[2] = (buffer.count() >> 16) & 0xff;
  buf[3] = (buffer.count() >> 24) & 0xff;
  m_Socket->send(buf, 4);
  m_Socket->send(buffer.data(), buffer.count());
}

#define SEND_MESSAGE(m) sendMessage(m.make_network_packet())

void
Museek::ServerManager::onCannotConnect(NewNet::ClientSocket * socket)
{
  // We'll try to reconnect in a few seconds
  bool wasAuto = m_AutoConnect;
  if (m_AutoConnect) {
      int length = 2000;
      if (m_ConnectionTries >= 2)
        length = 30000;
      if (m_ReconnectTimeout.isValid())
        museekd()->reactor()->removeTimeout(m_ReconnectTimeout);
      m_ReconnectTimeout = museekd()->reactor()->addTimeout(length, this, &ServerManager::reconnect);
      m_ConnectionTries++;
      NNLOG("museekd.server.warn", "Cannot connect to server... Will reconnect in %d ms.", length);
  }
  else
      NNLOG("museekd.server.warn", "Cannot connect to server... Will not try to reconnect.");

  setLoggedIn(false);
  disconnect();
  m_AutoConnect = wasAuto; // Restore m_AutoConnect as disconnect() will always set it to false
}

void
Museek::ServerManager::onConnected(NewNet::ClientSocket *)
{
  NNLOG("museekd.server.debug", "Connected to server.");
  NNLOG("museekd.server.debug", "Sending login message.");
  SEND_MESSAGE(SLogin(m_Username, m_Password));
}

void
Museek::ServerManager::onDisconnected(NewNet::ClientSocket * socket)
{
  // We'll try to reconnect in a few seconds
  if (m_AutoConnect) {
      int length = 2000;
      if (m_ConnectionTries >= 2)
        length = 30000;
      if (m_ReconnectTimeout.isValid())
        museekd()->reactor()->removeTimeout(m_ReconnectTimeout);
      m_ReconnectTimeout = museekd()->reactor()->addTimeout(length, this, &ServerManager::reconnect);
      m_ConnectionTries++;
    NNLOG("museekd.server.debug", "Disconnected from server. Will reconnect in %d ms.", length);
  }
  else
    NNLOG("museekd.server.debug", "Disconnected from server. Will not try to reconnect.");


  setLoggedIn(false);
  museekd()->reactor()->remove(socket);
  museekd()->reactor()->removeTimeout(m_PingTimeout);
}

void
Museek::ServerManager::onMessageReceived(const TcpMessageSocket::MessageData * data)
{
  switch(data->type)
  {

    #define MAP_MESSAGE(ID, TYPE, EVENT) \
      case ID: \
      { \
        NNLOG("museek.messages.server", "Received server message " #TYPE "."); \
        TYPE msg; \
        msg.parse_network_packet(data->data, data->length); \
        EVENT(&msg); \
        break; \
      }
    #include "servereventtable.h"
    #undef MAP_MESSAGE

    default:
        NNLOG("museekd.server.warn", "Received unknown server message, type: %u, length: %u", data->type, data->length);
        NetworkMessage msg;
        msg.parse_network_packet(data->data, data->length);
  }
}

void
Museek::ServerManager::onConfigKeySet(const ConfigManager::ChangeNotify * data)
{
  if(loggedIn())
  {
    if(data->domain == "interests.like")
      SEND_MESSAGE(SInterestAdd(data->key));
    else if(data->domain == "interests.hate")
      SEND_MESSAGE(SInterestHatedAdd(data->key));
    else if(data->domain == "buddies" || data->domain == "trusted" || data->domain == "ignored" || data->domain == "banned")
      museekd()->peers()->requestUserData(data->key);
  }
}

void
Museek::ServerManager::onConfigKeyRemoved(const ConfigManager::RemoveNotify * data)
{
  if(loggedIn())
  {
    if(data->domain == "interests.like")
      SEND_MESSAGE(SInterestRemove(data->key));
    else if(data->domain == "interests.hate")
      SEND_MESSAGE(SInterestHatedRemove(data->key));
  }
}

void
Museek::ServerManager::onLoggedIn(const SLogin * message)
{
  m_ConnectionTries = 0;
  if (m_ReconnectTimeout.isValid())
    museekd()->reactor()->removeTimeout(m_ReconnectTimeout);

  m_PingTimeout = museekd()->reactor()->addTimeout(60000, this, &ServerManager::pingServer);

  launchServerTimeTest(0.0L);
  museekd()->reactor()->addTimeout(10000, this, &ServerManager::serverTimeTestFailed); // Server time test needs to work in the first 10 seconds.

  setLoggedIn(message->success);
  if(message->success)
  {
    museekd()->peers()->requestUserData(username());
    std::vector<std::string>::const_iterator it, end = m_JoinedRooms.end();
    for(it = m_JoinedRooms.begin(); it != end; ++it)
      SEND_MESSAGE(SJoinRoom(*it));
    m_JoinedRooms.clear();

    std::vector<std::string> interests;
    interests = museekd()->config()->keys("interests.like");
    end = interests.end();
    for(it = interests.begin(); it != end; ++it)
      SEND_MESSAGE(SInterestAdd(*it));
    interests = museekd()->config()->keys("interests.hate");
    end = interests.end();
    for(it = interests.begin(); it != end; ++it)
      SEND_MESSAGE(SInterestHatedAdd(*it));

    // Get status and stats for every users we have in our lists
    std::vector<std::string> users, trusted, banned, ignored;
    users = museekd()->config()->keys("buddies");
    trusted = museekd()->config()->keys("trusted");
    banned = museekd()->config()->keys("banned");
    ignored = museekd()->config()->keys("ignored");
    users.insert(users.begin(), trusted.begin(), trusted.end());
    users.insert(users.begin(), banned.begin(), banned.end());
    users.insert(users.begin(), ignored.begin(), ignored.end());
    std::sort(users.begin(), users.end());
    std::unique(users.begin(), users.end());
    for(it = users.begin(); it != users.end(); ++it) {
      museekd()->peers()->requestUserData(*it);
    }

    museekd()->sendSharedNumber();

    SEND_MESSAGE(SPrivRoomToggle(museekd()->isEnabledPrivRoom()));
  }
}

void
Museek::ServerManager::launchServerTimeTest(long) {
    gettimeofday(&mLastServerTimeTestTime, 0);
    std::ostringstream oss;
    oss << "Testing server time " << mLastServerTimeTestTime.tv_sec << " " << mLastServerTimeTestTime.tv_usec;
    mLastServerTimeTestString = oss.str();
    SEND_MESSAGE(SPrivateMessage(username(), mLastServerTimeTestString));
    mTestingServerTime = true;
}

void
Museek::ServerManager::serverTimeTestFailed(long) {
    if (mServerTimeDiff <= 0)
        mServerTimeDiff = 0;
    NNLOG("museekd.server.debug", "The server time diff is considered as %ld.", mServerTimeDiff);
    //mTestingServerTime = true; // Keep it to true to avoid showing the test message if the server finally answers after a moment
    museekd()->reactor()->addTimeout(21600000, this, &ServerManager::launchServerTimeTest); // New test in 6 hours
    receivedServerTimeDiff(mServerTimeDiff);
}

bool
Museek::ServerManager::isServerTimeTestMessage(const std::string& user, const std::string& message) {
    return ((user == username()) && (mLastServerTimeTestString == message));
}

void
Museek::ServerManager::onServerPrivateMessageReceived(const SPrivateMessage * message)
{
    if (mTestingServerTime && isServerTimeTestMessage(message->user, message->message)) {
        mServerTimeDiff = message->timestamp - mLastServerTimeTestTime.tv_sec;
        NNLOG("museekd.server.debug", "The server time diff is %ld.", mServerTimeDiff);
        mTestingServerTime = false;
        SEND_MESSAGE(SAckPrivateMessage(message->ticket));
        museekd()->reactor()->addTimeout(21600000, this, &ServerManager::launchServerTimeTest); // New test in 6 hours
        receivedServerTimeDiff(mServerTimeDiff);
    }
}

/**
  * Ping the server and launch a timer for the next ping
  */
void
Museek::ServerManager::pingServer(long diff) {
    struct timeval now;
    gettimeofday(&now, 0);

    if (difftime(now, mLastSentMessage) > 59000) {
        // No data sent to server since 60 seconds. Ping the server
        NNLOG("museekd.server.debug", "Pinging the server (%dms delay)", diff);
        SPing msg;
        sendMessage(msg.make_network_packet());
        m_PingTimeout = museekd()->reactor()->addTimeout(60000, this, &ServerManager::pingServer);
    }
    else {
        // We've sent someting to the server recently. Wait 60 seconds vefore pinging.
        NNLOG("museekd.server.debug", "Delaying server ping");
        m_PingTimeout = museekd()->reactor()->addTimeout(60000, this, &ServerManager::pingServer);
    }
}

void
Museek::ServerManager::onRoomJoined(const SJoinRoom * message)
{
  m_JoinedRooms.push_back(message->room);

  std::string ticker(museekd()->config()->get("tickers", message->room));
  if(ticker.empty())
    ticker = museekd()->config()->get("default-ticker", "ticker");
  if(! ticker.empty())
  {
    ticker = museekd()->codeset()->toRoom(message->room, ticker);
    SEND_MESSAGE(SSetRoomTicker(message->room, ticker));
  }
}

void
Museek::ServerManager::onRoomLeft(const SLeaveRoom * message)
{
    std::vector<std::string>::iterator it;
    it = std::find(m_JoinedRooms.begin(), m_JoinedRooms.end(), message->value);
    if (it != m_JoinedRooms.end())
        m_JoinedRooms.erase(it);
}

void
Museek::ServerManager::onPrivilegedUsersReceived(const SPrivilegedUsers * message) {
    NNLOG("museekd.server.debug", "Received privileged users");
    m_Museekd->setPrivilegedUsers(message->values);
}

void
Museek::ServerManager::onPrivilegedUserAddedReceived(const SAddPrivileged * message) {
    NNLOG("museekd.server.debug", "Received a new privileged user");
    m_Museekd->addPrivilegedUser(message->value);
}

/**
  * Called when museekd is kicked from the server.
  */
void
Museek::ServerManager::onKicked(const SKicked * message) {
    // Don't try to reconnect
    m_AutoConnect = false;
}

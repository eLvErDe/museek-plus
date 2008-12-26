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
#include "ifacemanager.h"
#include "museekd.h"
#include "codesetmanager.h"
#include "sharesdatabase.h"
#include "servermanager.h"
#include "peersocket.h"
#include "downloadmanager.h"
#include "uploadmanager.h"
#include "searchmanager.h"
#include "peermanager.h"
#include <Muhelp/string_ext.hh>
#include <NewNet/nnunixfactorysocket.h>
#include <NewNet/nntcpfactorysocket.h>
#include <NewNet/nnlog.h>

#include <fstream>

#define SEND_MESSAGE(SOCKET, MESSAGE) (SOCKET)->sendMessage(MESSAGE.make_network_packet())
#define SEND_ALL(MESSAGE) \
  do { \
    NewNet::Buffer buffer(MESSAGE.make_network_packet()); \
    std::vector<NewNet::RefPtr<Museek::IfaceSocket> >::iterator it, end = m_Ifaces.end(); \
    for(it = m_Ifaces.begin(); it != end; ++it) \
      if((*it)->authenticated()) \
        (*it)->sendMessage(buffer); \
  } while(0)
#define SEND_MASK(MASK, MESSAGE) \
  do { \
    NewNet::Buffer buffer(MESSAGE.make_network_packet()); \
    std::vector<NewNet::RefPtr<Museek::IfaceSocket> >::iterator it, end = m_Ifaces.end(); \
    for(it = m_Ifaces.begin(); it != end; ++it) \
      if((*it)->authenticated() && ((*it)->mask() & MASK)) \
        (*it)->sendMessage(buffer); \
  } while(0)
#define SEND_C_MASK(MASK, MESSAGE) \
  do { \
    std::vector<NewNet::RefPtr<Museek::IfaceSocket> >::iterator it, end = m_Ifaces.end(); \
    for(it = m_Ifaces.begin(); it != end; ++it) \
      if((*it)->authenticated() && ((*it)->mask() & MASK)) \
        (*it)->sendMessage(MESSAGE.make_network_packet()); \
  } while(0)

static char challengemap[] = "0123456789abcdef";
static std::string challenge()
{
  std::string r;
  for(int i = 0; i < 64; i++)
    r += challengemap[rand() % 16];
  return r;
}

Museek::IfaceManager::IfaceManager(Museekd * museekd) : m_Museekd(museekd)
{
  m_AwayState = 0;

  NNLOG.logEvent.connect(this, &IfaceManager::onLog);

  museekd->config()->keySetEvent.connect(this, &IfaceManager::onConfigKeySet);
  museekd->config()->keyRemovedEvent.connect(this, &IfaceManager::onConfigKeyRemoved);

  museekd->server()->loggedInStateChangedEvent.connect(this, &IfaceManager::onServerLoggedInStateChanged);
  museekd->server()->loggedInEvent.connect(this, &IfaceManager::onServerLoggedIn);
  museekd->server()->kickedEvent.connect(this, &IfaceManager::onServerKicked);
  museekd->server()->peerAddressReceivedEvent.connect(this, &IfaceManager::onServerPeerAddressReceived);
  museekd->server()->userExistsReceivedEvent.connect(this, &IfaceManager::onServerUserExistsReceived);
  museekd->server()->userStatusReceivedEvent.connect(this, &IfaceManager::onServerUserStatusReceived);
  museekd->server()->userStatsReceivedEvent.connect(this, &IfaceManager::onServerUserStatsReceived);
  museekd->server()->privateMessageReceivedEvent.connect(this, &IfaceManager::onServerPrivateMessageReceived);
  museekd->server()->roomMessageReceivedEvent.connect(this, &IfaceManager::onServerRoomMessageReceived);
  museekd->server()->roomJoinedEvent.connect(this, &IfaceManager::onServerRoomJoined);
  museekd->server()->roomLeftEvent.connect(this, &IfaceManager::onServerRoomLeft);
  museekd->server()->userJoinedRoomEvent.connect(this, &IfaceManager::onServerUserJoinedRoom);
  museekd->server()->userLeftRoomEvent.connect(this, &IfaceManager::onServerUserLeftRoom);
  museekd->server()->roomListReceivedEvent.connect(this, &IfaceManager::onServerRoomListReceived);
  museekd->server()->privilegesReceivedEvent.connect(this, &IfaceManager::onServerPrivilegesReceived);
  museekd->server()->roomTickersReceivedEvent.connect(this, &IfaceManager::onServerRoomTickersReceived);
  museekd->server()->roomTickerAddedEvent.connect(this, &IfaceManager::onServerRoomTickerAdded);
  museekd->server()->roomTickerRemovedEvent.connect(this, &IfaceManager::onServerRoomTickerRemoved);
  museekd->server()->recommendationsReceivedEvent.connect(this, &IfaceManager::onServerRecommendationsReceived);
  museekd->server()->globalRecommendationsReceivedEvent.connect(this, &IfaceManager::onServerGlobalRecommendationsReceived);
  museekd->server()->similarUsersReceivedEvent.connect(this, &IfaceManager::onServerSimilarUsersReceived);
  museekd->server()->itemRecommendationsReceivedEvent.connect(this, &IfaceManager::onServerItemRecommendationsReceived);
  museekd->server()->itemSimilarUsersReceivedEvent.connect(this, &IfaceManager::onServerItemSimilarUsersReceived);
  museekd->server()->userInterestsReceivedEvent.connect(this, &IfaceManager::onServerUserInterestsReceived);

  museekd->downloads()->downloadAddedEvent.connect(this, &IfaceManager::onDownloadUpdated);
  museekd->downloads()->downloadUpdatedEvent.connect(this, &IfaceManager::onDownloadUpdated);
  museekd->downloads()->downloadRemovedEvent.connect(this, &IfaceManager::onDownloadRemoved);

  museekd->uploads()->uploadAddedEvent.connect(this, &IfaceManager::onUploadUpdated);
  museekd->uploads()->uploadUpdatedEvent.connect(this, &IfaceManager::onUploadUpdated);
  museekd->uploads()->uploadRemovedEvent.connect(this, &IfaceManager::onUploadRemoved);

  museekd->peers()->peerSocketUnavailableEvent.connect(this, &IfaceManager::onPeerSocketUnavailable);
  museekd->peers()->peerSocketReadyEvent.connect(this, &IfaceManager::onPeerSocketReady);
}

bool
Museek::IfaceManager::addListener(const std::string & path)
{
  if(path.empty())
    return false;

#ifndef WIN32 // No unix sockets on win32
  if(path[0] == '/')
  {
    NewNet::RefPtr<NewNet::UnixFactorySocket<IfaceSocket> > factory;
    factory = new NewNet::UnixFactorySocket<IfaceSocket>;
    factory->clientAcceptedEvent.connect(this, &IfaceManager::onIfaceAccepted);
    factory->serverSocket()->listen(path);
    if(factory->serverSocket()->socketState() != NewNet::Socket::SocketListening)
    {
      NNLOG("museekd.iface.warn", "Couldn't listen on unix path '%s'.", path.c_str());
      return false;
    }
    m_Factories[path] = factory;
    m_ServerSockets[path] = factory->serverSocket();
    museekd()->reactor()->add(factory->serverSocket());
  }
  else
#endif // WIN32
  {
    std::string::size_type ix = path.find(':');
    if(ix == std::string::npos)
    {
      NNLOG("museekd.iface.warn", "Invalid TCP description: '%s'.", path.c_str());
      return false;
    }
    std::string host = path.substr(0, ix);
    unsigned int port = atol(path.substr(ix + 1).c_str());
    NewNet::RefPtr<NewNet::TcpFactorySocket<IfaceSocket> > factory;
    factory = new NewNet::TcpFactorySocket<IfaceSocket>;
    factory->clientAcceptedEvent.connect(this, &IfaceManager::onIfaceAccepted);
    factory->serverSocket()->listen(host, port);
    if(factory->serverSocket()->socketState() != NewNet::Socket::SocketListening)
    {
      NNLOG("museekd.iface.warn", "Couldn't listen on '%s:%u'", host.c_str(), port);
      return false;
    }
    m_Factories[path] = factory;
    m_ServerSockets[path] = factory->serverSocket();
    museekd()->reactor()->add(factory->serverSocket());
  }

  NNLOG("museekd.iface.debug", "Listening on '%s'.", path.c_str());
  return true;
}

void
Museek::IfaceManager::removeListener(const std::string & path)
{
  std::map<std::string, NewNet::RefPtr<NewNet::ServerSocket> >::iterator it;
  it = m_ServerSockets.find(path);
  if(it == m_ServerSockets.end())
    return;
  (*it).second->disconnect();
  museekd()->reactor()->remove((*it).second);
  m_ServerSockets.erase(it);
  std::map<std::string, NewNet::RefPtr<NewNet::Object> >::iterator fit = m_Factories.find(path);
  if (fit != m_Factories.end())
    m_Factories.erase(fit);
}

void
Museek::IfaceManager::onLog(const NewNet::Log::LogNotify * log)
{
  SEND_MASK(EM_DEBUG, IDebugMessage(log->domain, log->message));
}

void
Museek::IfaceManager::onConfigKeySet(const ConfigManager::ChangeNotify * data)
{
  if(data->domain == "interfaces.bind")
  {
    if(m_Factories.find(data->key) != m_Factories.end())
      return;
    addListener(data->key);
  }
  SEND_C_MASK(EM_CONFIG, IConfigSet((*it)->cipherContext(), data->domain, data->key, data->value));
}

void
Museek::IfaceManager::onConfigKeyRemoved(const ConfigManager::RemoveNotify * data)
{
  if(data->domain == "interfaces.bind")
  {
    if(m_Factories.find(data->key) == m_Factories.end())
      return;
    removeListener(data->key);
  }
  SEND_C_MASK(EM_CONFIG, IConfigRemove((*it)->cipherContext(), data->domain, data->key));
}

void
Museek::IfaceManager::flushPrivateMessages()
{
  if(m_PrivateMessages.empty() || m_Ifaces.empty())
    return;

  bool sent = false;
  std::vector<PrivateMessage>::iterator it, end = m_PrivateMessages.end();

  std::vector<NewNet::RefPtr<IfaceSocket> >::iterator iit, iend = m_Ifaces.end();
  for(iit = m_Ifaces.begin(); iit != iend; ++iit)
  {
    if((*iit)->mask() & EM_PRIVATE)
    {
      sent = true;
      for(it = m_PrivateMessages.begin(); it != end; ++it)
        SEND_MESSAGE(*iit, IPrivateMessage(0, (*it).timestamp, (*it).user, (*it).message));
    }
  }

  if(! sent)
    return;

  for(it = m_PrivateMessages.begin(); it != end; ++it)
    SEND_MESSAGE(museekd()->server(), SAckPrivateMessage((*it).ticket));

  m_PrivateMessages.clear();
}

void
Museek::IfaceManager::sendNewSearchToAll(const std::string & query, uint token) {
    SEND_ALL(ISearch(query, token));
}

void
Museek::IfaceManager::onIfaceAccepted(IfaceSocket * socket)
{
  NNLOG("museekd.iface.debug", "Accepted new interface socket.");
  m_Ifaces.push_back(socket);

  // Connect the events
  socket->disconnectedEvent.connect(this, &IfaceManager::onIfaceDisconnected);
  socket->pingEvent.connect(this, &IfaceManager::onIfacePing);
  socket->loginEvent.connect(this, &IfaceManager::onIfaceLogin);
  socket->checkPrivilegesEvent.connect(this, &IfaceManager::onIfaceCheckPrivileges);
  socket->setStatusEvent.connect(this, &IfaceManager::onIfaceSetStatus);
  socket->setConfigEvent.connect(this, &IfaceManager::onIfaceSetConfig);
  socket->removeConfigEvent.connect(this, &IfaceManager::onIfaceRemoveConfig);
  socket->setUserImageEvent.connect(this, &IfaceManager::onIfaceSetUserImage);
  socket->getPeerExistsEvent.connect(this, &IfaceManager::onIfaceGetPeerExists);
  socket->getPeerStatusEvent.connect(this, &IfaceManager::onIfaceGetPeerStatus);
  socket->getPeerStatsEvent.connect(this, &IfaceManager::onIfaceGetPeerStats);
  socket->getUserInfoEvent.connect(this, &IfaceManager::onIfaceGetUserInfo);
  socket->getUserInterestsEvent.connect(this, &IfaceManager::onIfaceGetUserInterests);
  socket->getUserSharesEvent.connect(this, &IfaceManager::onIfaceGetUserShares);
  socket->getPeerAddressEvent.connect(this, &IfaceManager::onIfaceGetPeerAddress);
  socket->givePrivilegesEvent.connect(this, &IfaceManager::onIfaceGivePrivileges);
  socket->sendPrivateMessageEvent.connect(this, &IfaceManager::onIfaceSendPrivateMessage);
  socket->getRoomListEvent.connect(this, &IfaceManager::onIfaceGetRoomList);
  socket->joinRoomEvent.connect(this, &IfaceManager::onIfaceJoinRoom);
  socket->leaveRoomEvent.connect(this, &IfaceManager::onIfaceLeaveRoom);
  socket->sayRoomEvent.connect(this, &IfaceManager::onIfaceSayRoom);
  socket->setRoomTickerEvent.connect(this, &IfaceManager::onIfaceSetRoomTicker);
  socket->startGlobalSearchEvent.connect(this, &IfaceManager::onIfaceStartSearch);
  socket->startUserSearchEvent.connect(this, &IfaceManager::onIfaceStartUserSearch);
  socket->startWishListSearchEvent.connect(this, &IfaceManager::onIfaceStartWishListSearch);
  socket->getRecommendationsEvent.connect(this, &IfaceManager::onIfaceGetRecommendations);
  socket->getGlobalRecommendationsEvent.connect(this, &IfaceManager::onIfaceGetGlobalRecommendations);
  socket->getSimilarUsersEvent.connect(this, &IfaceManager::onIfaceGetSimilarUsers);
  socket->getItemRecommendationsEvent.connect(this, &IfaceManager::onIfaceGetItemRecommendations);
  socket->getItemSimilarUsersEvent.connect(this, &IfaceManager::onIfaceGetItemSimilarUsers);
  socket->addInterestEvent.connect(this, &IfaceManager::onIfaceAddInterest);
  socket->removeInterestEvent.connect(this, &IfaceManager::onIfaceRemoveInterest);
  socket->addHatedInterestEvent.connect(this, &IfaceManager::onIfaceAddHatedInterest);
  socket->removeHatedInterestEvent.connect(this, &IfaceManager::onIfaceRemoveHatedInterest);
  socket->addWishItemEvent.connect(this, &IfaceManager::onIfaceAddWishItem);
  socket->removeWishItemEvent.connect(this, &IfaceManager::onIfaceRemoveWishItem);
  socket->connectToServerEvent.connect(this, &IfaceManager::onIfaceConnectToServer);
  socket->disconnectFromServerEvent.connect(this, &IfaceManager::onIfaceDisconnectFromServer);
  socket->reloadSharesEvent.connect(this, &IfaceManager::onIfaceReloadShares);
  socket->downloadFileEvent.connect(this, &IfaceManager::onIfaceDownloadFile);
  socket->downloadFileToEvent.connect(this, &IfaceManager::onIfaceDownloadFileTo);
  socket->downloadFolderEvent.connect(this, &IfaceManager::onIfaceDownloadFolder);
  socket->downloadFolderToEvent.connect(this, &IfaceManager::onIfaceDownloadFolderTo);
  socket->updateTransferEvent.connect(this, &IfaceManager::onIfaceUpdateTransfer);
  socket->removeTransferEvent.connect(this, &IfaceManager::onIfaceRemoveTransfer);
  socket->abortTransferEvent.connect(this, &IfaceManager::onIfaceAbortTransfer);
  socket->uploadFolderEvent.connect(this, &IfaceManager::onIfaceUploadFolder);
  socket->uploadFileEvent.connect(this, &IfaceManager::onIfaceUploadFile);

  // Send the login challenge
  socket->setChallenge(challenge());
  SEND_MESSAGE(socket, IChallenge(4, socket->challenge()));
}

void
Museek::IfaceManager::onIfaceDisconnected(NewNet::ClientSocket * socket)
{
    std::vector<NewNet::RefPtr<IfaceSocket> >::iterator it;
    it = std::find(m_Ifaces.begin(), m_Ifaces.end(), static_cast<IfaceSocket *>(socket));
    if (it != m_Ifaces.end())
        m_Ifaces.erase(it);
    museekd()->reactor()->remove(socket);
}

void
Museek::IfaceManager::onIfacePing(const IPing * message)
{
  SEND_MESSAGE(message->ifaceSocket(), IPing(message->id));
}

void
Museek::IfaceManager::onIfaceLogin(const ILogin * message)
{
  std::string password = museekd()->config()->get("interfaces", "password");
  if(password.empty())
  {
    NNLOG("museekd.iface.warn", "Rejecting login attempt because of empty password.");
    message->ifaceSocket()->setChallenge(challenge());
    SEND_MESSAGE(message->ifaceSocket(), ILogin(false, "INVPASS", message->ifaceSocket()->challenge()));
    return;
  }

  std::string ch = message->ifaceSocket()->challenge() + password;

  unsigned char digest[32];
  uint digest_len = 0;

  if(message->algorithm == "SHA1")
  {
    shaBlock((unsigned char *)ch.data(), ch.size(), digest);
    digest_len = 20;
  }
  else if(message->algorithm == "SHA256")
  {
    sha256Block((unsigned char *)ch.data(), ch.size(), digest);
    digest_len = 32;
  }
  else if(message->algorithm == "MD5")
  {
    md5Block((unsigned char *)ch.data(), ch.size(), digest);
    digest_len = 16;
  }
  else
  {
    NNLOG("museekd.iface.warn", "Rejected login attempt because of unknown hash algorithm.");
    message->ifaceSocket()->setChallenge(challenge());
    SEND_MESSAGE(message->ifaceSocket(), ILogin(false, "INVHASH", message->ifaceSocket()->challenge()));
    return;
  }

  char hexdigest[65];
  hexDigest(digest, digest_len, hexdigest);
  if(message->chresponse != hexdigest)
  {
    NNLOG("museekd.iface.warn", "Rejected login attempt because of incorrect password.");
    message->ifaceSocket()->setChallenge(challenge());
    SEND_MESSAGE(message->ifaceSocket(), ILogin(false, "INVPASS", message->ifaceSocket()->challenge()));
  }
  else
  {
    NNLOG("museekd.iface.debug", "Interface successfully logged in.");
    IfaceSocket * socket = message->ifaceSocket();
    socket->setAuthenticated(true);
    socket->setMask(message->mask);
    socket->setCipherKey(password);
    SEND_MESSAGE(socket, ILogin(true, std::string(), std::string()));
    SEND_MESSAGE(socket, IServerState(museekd()->server()->loggedIn(), museekd()->server()->username()));
    if(socket->mask() & EM_CHAT)
      SEND_MESSAGE(socket, IRoomState(m_RoomList, m_RoomData, m_TickerData));
    if(socket->mask() & EM_TRANSFERS) {
      SEND_MESSAGE(socket, ITransferState(&museekd()->downloads()->downloads()));
      SEND_MESSAGE(socket, ITransferState(&museekd()->uploads()->uploads()));
    }
    if(socket->mask() & EM_PRIVATE)
      flushPrivateMessages();
    if(socket->mask() & EM_CONFIG)
      SEND_MESSAGE(socket, IConfigState(socket->cipherContext(), museekd()->config()->data()));
    if(museekd()->server()->loggedIn())
      SEND_MESSAGE(message->ifaceSocket(), ISetStatus(m_AwayState));

    if (socket->mask() & EM_USERINFO) {
        // send peers stats
        std::map<std::string, SGetUserStats>::const_iterator it = museekd()->peers()->userStats()->begin();
        for(; it != museekd()->peers()->userStats()->end(); it++) {
            SEND_MESSAGE(socket, IPeerStats(it->second.user, it->second.avgspeed, it->second.downloadnum, it->second.files, it->second.dirs));
        }
        std::map<std::string, uint32>::const_iterator sit = museekd()->peers()->userStatus()->begin();
        for(; sit != museekd()->peers()->userStatus()->end(); sit++) {
            SEND_MESSAGE(socket, IPeerStatus(sit->first, sit->second));
        }
    }
  }
}

void
Museek::IfaceManager::onIfaceCheckPrivileges(const ICheckPrivileges * message)
{
  SEND_MESSAGE(museekd()->server(), SCheckPrivileges());
}

void
Museek::IfaceManager::onIfaceSetStatus(const ISetStatus * message)
{
  SEND_MESSAGE(museekd()->server(), SSetStatus(message->status ? 1 : 2));
  // Send it without waiting for a response from the server.
  // Before 157, server used to send us SGetStatus after changing our status.
  // Starting from 157, it doesn't anymore when switching back to online.
  m_AwayState = message->status;
  SEND_ALL(ISetStatus(m_AwayState));
  museekd()->peers()->setUserStatus(museekd()->server()->username(), message->status ? 1 : 2);
  SEND_ALL(IPeerStatus(museekd()->server()->username(), message->status ? 1 : 2));
}

void
Museek::IfaceManager::onIfaceSetConfig(const IConfigSet * message)
{
  museekd()->config()->set(message->domain, message->key, message->value);
}

void
Museek::IfaceManager::onIfaceRemoveConfig(const IConfigRemove * message)
{
  museekd()->config()->removeKey(message->domain, message->key);
}

void
Museek::IfaceManager::onIfaceSetUserImage(const IConfigSetUserImage * message) {
    std::string path = museekd()->config()->get("userinfo", "image");

    std::ofstream ofs(path.c_str(), std::ofstream::binary | std::ofstream::trunc);
    if (!ofs.good())
        return;

    for (uint i =0; i < message->mData.size(); i++) {
        ofs.put(static_cast<const char>(message->mData[i]));
    }

    ofs.close();
}

void
Museek::IfaceManager::onIfaceGetPeerExists(const IPeerExists * message)
{
  SEND_MESSAGE(museekd()->server(), SAddUser(message->user));
}

void
Museek::IfaceManager::onIfaceGetPeerStatus(const IPeerStatus * message)
{
  SEND_MESSAGE(museekd()->server(), SGetStatus(message->user));
}

void
Museek::IfaceManager::onIfaceGetPeerStats(const IPeerStats * message)
{
  SEND_MESSAGE(museekd()->server(), SGetUserStats(message->user));
}

void
Museek::IfaceManager::onIfaceGetUserInfo(const IUserInfo * message)
{
  m_PendingInfo[message->user].push_back(message->ifaceSocket());
  if (std::find(m_PendingInfoWaiting.begin(), m_PendingInfoWaiting.end(), message->user) == m_PendingInfoWaiting.end())
    m_PendingInfoWaiting.push_back(message->user);

  museekd()->peers()->peerSocket(message->user);
}

void
Museek::IfaceManager::onIfaceGetUserInterests(const IUserInterests * message)
{
  // Check the user's Interests
  m_Museekd->server()->sendMessage(SUserInterests(message->user).make_network_packet());
}

void
Museek::IfaceManager::onIfaceGetUserShares(const IUserShares * message)
{
  m_PendingShares[message->user].push_back(message->ifaceSocket());
  if (std::find(m_PendingSharesWaiting.begin(), m_PendingSharesWaiting.end(), message->user) == m_PendingSharesWaiting.end())
    m_PendingSharesWaiting.push_back(message->user);

  museekd()->peers()->peerSocket(message->user);
}

void
Museek::IfaceManager::onIfaceGetPeerAddress(const IPeerAddress * message)
{
  SEND_MESSAGE(museekd()->server(), SGetPeerAddress(message->user));
}

void
Museek::IfaceManager::onIfaceGivePrivileges(const IGivePrivileges * message)
{
  SEND_MESSAGE(museekd()->server(), SGivePrivileges(message->user, message->days));
}

void
Museek::IfaceManager::onIfaceSendPrivateMessage(const IPrivateMessage * message)
{
  std::string line = museekd()->codeset()->toPeer(message->user, message->msg);

  // send one message per line
  std::vector<std::string> lines;
  std::vector<std::string>::const_iterator it;
  split(line, lines, "\n");

  for (it = lines.begin(); it != lines.end(); ++it) {
    SEND_MESSAGE(museekd()->server(), SPrivateMessage(message->user, *it));
  }

  // send one message per line
  std::vector<std::string> ilines;
  std::vector<std::string>::const_iterator iit;
  split(message->msg, ilines, "\n");

  for (iit = ilines.begin(); iit != ilines.end(); ++iit) {
    IPrivateMessage msg(1, time(NULL), message->user, *iit);

    const NewNet::Buffer & buffer = msg.make_network_packet();
    std::vector<NewNet::RefPtr<Museek::IfaceSocket> >::iterator fit;
    for(fit = m_Ifaces.begin(); fit != m_Ifaces.end(); ++fit) {
    if((*fit)->authenticated() && ((*fit)->mask() & EM_PRIVATE) && ((*fit) != message->ifaceSocket()))
      (*fit)->sendMessage(buffer);
    }
  }
}

void
Museek::IfaceManager::onIfaceGetRoomList(const IRoomList * message)
{
  SEND_MESSAGE(museekd()->server(), SRoomList());
}

void
Museek::IfaceManager::onIfaceJoinRoom(const IJoinRoom * message)
{
  SEND_MESSAGE(museekd()->server(), SJoinRoom(message->room));
}

void
Museek::IfaceManager::onIfaceLeaveRoom(const ILeaveRoom * message)
{
  SEND_MESSAGE(museekd()->server(), SLeaveRoom(message->room));
}

void
Museek::IfaceManager::onIfaceSayRoom(const ISayRoom * message)
{
  std::string line = museekd()->codeset()->toRoom(message->room, message->line);

  // send one message per line
  std::vector<std::string> lines;
  std::vector<std::string>::const_iterator it;
  split(line, lines, "\n");

  for (it = lines.begin(); it != lines.end(); ++it) {
    SEND_MESSAGE(museekd()->server(), SSayRoom(message->room, *it));
  }
}

void
Museek::IfaceManager::onIfaceSetRoomTicker(const IRoomTickerSet * message)
{
  std::string ticker = museekd()->codeset()->toRoom(message->room, message->message);
  ticker = str_replace(ticker, '\n', ' ');
  SEND_MESSAGE(museekd()->server(), SSetRoomTicker(message->room, ticker));
}

void
Museek::IfaceManager::onIfaceStartSearch(const ISearch * message)
{
    uint token = museekd()->token();

    if (message->type == 0) // Global search
        SEND_MESSAGE(museekd()->server(), SFileSearch(token, museekd()->codeset()->toNet(message->query)));
    else if (message->type == 1) // Buddies search
        museekd()->searches()->buddySearch(token, message->query);
    else if (message->type == 2) // Room search
        museekd()->searches()->roomsSearch(token, message->query);

    sendNewSearchToAll(message->query, token);
}

void
Museek::IfaceManager::onIfaceStartUserSearch(const IUserSearch * message)
{
    uint token = museekd()->token();

    SEND_MESSAGE(museekd()->server(), SUserSearch(message->user, token, museekd()->codeset()->toNet(message->query)));

    sendNewSearchToAll(message->query, token);
}

void
Museek::IfaceManager::onIfaceStartWishListSearch(const IWishListSearch * message) {
    museekd()->searches()->wishlistAdd(message->query);
}

void
Museek::IfaceManager::onIfaceGetRecommendations(const IGetRecommendations *)
{
  SEND_MESSAGE(museekd()->server(), SGetRecommendations());
}

void
Museek::IfaceManager::onIfaceGetGlobalRecommendations(const IGetGlobalRecommendations *)
{
  SEND_MESSAGE(museekd()->server(), SGetGlobalRecommendations());
}

void
Museek::IfaceManager::onIfaceGetSimilarUsers(const IGetSimilarUsers *)
{
  SEND_MESSAGE(museekd()->server(), SGetSimilarUsers());
}

void
Museek::IfaceManager::onIfaceGetItemRecommendations(const IGetItemRecommendations * message)
{
  SEND_MESSAGE(museekd()->server(), SGetItemRecommendations(message->item));
}

void
Museek::IfaceManager::onIfaceGetItemSimilarUsers(const IGetItemSimilarUsers * message)
{
  SEND_MESSAGE(museekd()->server(), SGetItemSimilarUsers(message->item));
}

void
Museek::IfaceManager::onIfaceAddInterest(const IAddInterest * message)
{
  museekd()->config()->set("interests.like", message->interest, "");
}

void
Museek::IfaceManager::onIfaceRemoveInterest(const IRemoveInterest * message)
{
  museekd()->config()->removeKey("interests.like", message->interest);
}

void
Museek::IfaceManager::onIfaceAddHatedInterest(const IAddHatedInterest * message)
{
  museekd()->config()->set("interests.hate", message->interest, "");
}

void
Museek::IfaceManager::onIfaceRemoveHatedInterest(const IRemoveHatedInterest * message)
{
  museekd()->config()->removeKey("interests.hate", message->interest);
}

void
Museek::IfaceManager::onIfaceAddWishItem(const IAddWishItem * message)
{
  museekd()->config()->set("wishlist", message->query, 0);
}

void
Museek::IfaceManager::onIfaceRemoveWishItem(const IRemoveWishItem * message)
{
  museekd()->config()->removeKey("wishlist", message->query);
}

void
Museek::IfaceManager::onIfaceConnectToServer(const IConnectServer * message)
{
  museekd()->server()->connect();
}

void
Museek::IfaceManager::onIfaceDisconnectFromServer(const IDisconnectServer * message)
{
  museekd()->server()->disconnect();
}


void
Museek::IfaceManager::onIfaceReloadShares(const IReloadShares * message)
{
  museekd()->LoadShares();
}

void
Museek::IfaceManager::onIfaceDownloadFile(const IDownloadFile * message)
{
    museekd()->downloads()->add(message->user, message->path);
    Download * newDownload = museekd()->downloads()->findDownload(message->user, message->path);
    newDownload->setSize(message->size);
}

void
Museek::IfaceManager::onIfaceDownloadFileTo(const IDownloadFileTo * message)
{
    museekd()->downloads()->add(message->user, message->path, museekd()->codeset()->fromUtf8ToFS(message->localpath));
    Download * newDownload = museekd()->downloads()->findDownload(message->user, message->path);
    newDownload->setSize(message->size);
}

void
Museek::IfaceManager::onIfaceDownloadFolder(const IDownloadFolder * message)
{
    museekd()->downloads()->addFolder(message->user, message->folder);
}

void
Museek::IfaceManager::onIfaceDownloadFolderTo(const IDownloadFolderTo * message)
{
    museekd()->downloads()->addFolder(message->user, message->folder, museekd()->codeset()->fromUtf8ToFS(message->localpath));
}

void
Museek::IfaceManager::onIfaceUpdateTransfer(const ITransferUpdate * message)
{
    museekd()->downloads()->update(message->user, message->path);
    museekd()->uploads()->update(message->user, museekd()->codeset()->fromUtf8ToFS(message->path));
}

void
Museek::IfaceManager::onIfaceRemoveTransfer(const ITransferRemove * message)
{
    if(! message->upload)
        museekd()->downloads()->remove(message->user, message->path);
    else {
        museekd()->uploads()->remove(message->user, museekd()->codeset()->fromUtf8ToFS(message->path));
    }
}

void
Museek::IfaceManager::onIfaceAbortTransfer(const ITransferAbort * message)
{
    if(! message->upload)
        museekd()->downloads()->abort(message->user, message->path);
    else
        museekd()->uploads()->abort(message->user, museekd()->codeset()->fromUtf8ToFS(message->path));
}

void
Museek::IfaceManager::onIfaceUploadFolder(const IUploadFolder * message) {
    std::string user = message->user;
    std::string path = message->path;
    museekd()->uploads()->addFolder(user, path);
}

void
Museek::IfaceManager::onIfaceUploadFile(const IUploadFile * message)
{
    std::string user = message->user;
    std::string path = museekd()->codeset()->fromUtf8ToFS(message->path);
    std::string pathInDb = museekd()->codeset()->toNet(message->path);
    std::string error;

    if (museekd()->uploads()->isUploadable(user, pathInDb, &error))
        museekd()->uploads()->add(user, path);
}

void
Museek::IfaceManager::onServerLoggedInStateChanged(bool loggedIn)
{
  m_AwayState = 0;
  SEND_ALL(IServerState(loggedIn, museekd()->server()->username()));
  if(loggedIn) {
    SEND_ALL(ISetStatus(m_AwayState));
    sendStatusMessage(true, std::string("Connected to the server"));
  }
  else
  {
    m_AwayState = 0;
    m_RoomList.clear();
    m_RoomData.clear();
    m_TickerData.clear();
    m_PrivateMessages.clear();
    sendStatusMessage(true, std::string("Disconnected from the server"));
  }
}

void
Museek::IfaceManager::onServerKicked(const SKicked* message) {
    sendStatusMessage(true, std::string("Kicked from soulseek server"));

    PrivateMessage msg;
    msg.ticket = 0;
    msg.timestamp = time(NULL);
    msg.user = "Server";
    msg.message = "You've been kicked from soulseek server. Maybe you tried to launch several museekd instances. Or you did something *wrong*. Try reconnecting in a moment (usually >30min).";
    m_PrivateMessages.push_back(msg);
    flushPrivateMessages();
}

void
Museek::IfaceManager::onServerPeerAddressReceived(const SGetPeerAddress * message)
{
  SEND_ALL(IPeerAddress(message->user, message->ip, message->port));
}

void
Museek::IfaceManager::onServerUserExistsReceived(const SAddUser * message)
{
  SEND_ALL(IPeerExists(message->user, message->exists));
}

void
Museek::IfaceManager::onServerUserStatusReceived(const SGetStatus * message)
{
  std::map<std::string, RoomData>::iterator it, end = m_RoomData.end();
  for(it = m_RoomData.begin(); it != end; ++it)
  {
    RoomData::iterator u_it = (*it).second.find(message->user);
    if(u_it == (*it).second.end())
      continue;
    (*u_it).second.status = message->status;
  }
  SEND_ALL(IPeerStatus(message->user, message->status));
  if(message->user == museekd()->server()->username())
  {
    m_AwayState = message->status & 1;
    SEND_ALL(ISetStatus(m_AwayState));
  }
}

void
Museek::IfaceManager::onServerUserStatsReceived(const SGetUserStats * message)
{
  std::map<std::string, RoomData>::iterator it, end = m_RoomData.end();
  for(it = m_RoomData.begin(); it != end; ++it)
  {
    RoomData::iterator u_it = (*it).second.find(message->user);
    if(u_it == (*it).second.end())
      continue;
    (*u_it).second.avgspeed = message->avgspeed;
    (*u_it).second.downloadnum = message->downloadnum;
    (*u_it).second.files = message->files;
    (*u_it).second.dirs = message->dirs;
  }
  SEND_ALL(IPeerStats(message->user, message->avgspeed, message->downloadnum, message->files, message->dirs));
}

void
Museek::IfaceManager::onServerPrivateMessageReceived(const SPrivateMessage * message)
{
  PrivateMessage msg;
  msg.ticket = message->ticket;
  msg.timestamp = message->timestamp - 9600; // Server's timestamps are wrong
  msg.user = message->user;
  msg.message = museekd()->codeset()->fromPeer(msg.user, message->message);
  m_PrivateMessages.push_back(msg);
  flushPrivateMessages();
}

void
Museek::IfaceManager::onServerRoomMessageReceived(const SSayRoom * message)
{
  std::string line = museekd()->codeset()->fromRoom(message->room, message->line);
  SEND_MASK(EM_CHAT, ISayRoom(message->room, message->user, line));
}

void
Museek::IfaceManager::onServerRoomJoined(const SJoinRoom * message)
{
  if(m_RoomData.find(message->room) != m_RoomData.end())
    return;
  m_RoomData[message->room] = message->users;
  m_TickerData[message->room];
  SEND_MASK(EM_CHAT, IJoinRoom(message->room, message->users));
}

void
Museek::IfaceManager::onServerRoomLeft(const SLeaveRoom * message)
{
  std::map<std::string, RoomData>::iterator it = m_RoomData.find(message->value);
  if(it == m_RoomData.end())
    return;
  m_RoomData.erase(it);
  std::map<std::string, Tickers>::iterator tit = m_TickerData.find(message->value);
  if (tit != m_TickerData.end())
    m_TickerData.erase(tit);
  SEND_MASK(EM_CHAT, ILeaveRoom(message->value));
}

void
Museek::IfaceManager::onServerUserJoinedRoom(const SUserJoinedRoom * message)
{
  std::map<std::string, RoomData>::iterator it = m_RoomData.find(message->room);
  if(it == m_RoomData.end())
    return;
  RoomData::iterator it2 = (*it).second.find(message->user);
  if(it2 != (*it).second.end())
    return;
  (*it).second[message->user] = message->userdata;
  SEND_MASK(EM_CHAT, IUserJoinedRoom(message->room, message->user, message->userdata));
}

void
Museek::IfaceManager::onServerUserLeftRoom(const SUserLeftRoom * message)
{
  std::map<std::string, RoomData>::iterator it = m_RoomData.find(message->room);
  if(it == m_RoomData.end())
    return;
  RoomData::iterator it2 = (*it).second.find(message->user);
  if(it2 == (*it).second.end())
    return;
  (*it).second.erase(it2);
  SEND_MASK(EM_CHAT, IUserLeftRoom(message->room, message->user));
}

void
Museek::IfaceManager::onServerRoomListReceived(const SRoomList * message)
{
  m_RoomList = message->roomlist;
  SEND_MASK(EM_CHAT, IRoomList(message->roomlist));
}

void
Museek::IfaceManager::onServerPrivilegesReceived(const SCheckPrivileges * message)
{
  SEND_ALL(ICheckPrivileges(message->time_left));
}

void
Museek::IfaceManager::onServerRoomTickersReceived(const SRoomTickers * message)
{
  if(m_TickerData.find(message->room) == m_TickerData.end())
    return;
  m_TickerData[message->room] = museekd()->codeset()->fromRoomMap(message->room, message->tickers);
  SEND_MASK(EM_CHAT, IRoomTickers(message->room, m_TickerData[message->room]));
}

void
Museek::IfaceManager::onServerRoomTickerAdded(const SRoomTickerAdd * message)
{
  if(m_TickerData.find(message->room) == m_TickerData.end())
    return;
  std::string ticker = museekd()->codeset()->fromRoom(message->room, message->ticker);
  m_TickerData[message->room][message->user] = ticker;
  SEND_MASK(EM_CHAT, IRoomTickerSet(message->room, message->user, ticker));
}

void
Museek::IfaceManager::onServerRoomTickerRemoved(const SRoomTickerRemove * message)
{
  std::map<std::string, Tickers>::iterator it = m_TickerData.find(message->room);
  if(it == m_TickerData.end())
    return;
  Tickers::iterator it2 = (*it).second.find(message->user);
  if(it2 == (*it).second.end())
    return;
  (*it).second.erase(it2);
  SEND_MASK(EM_CHAT, IRoomTickerSet(message->room, message->user, std::string()));
}

void
Museek::IfaceManager::onServerRecommendationsReceived(const SGetRecommendations * message)
{
  SEND_MASK(EM_INTERESTS, IGetRecommendations(message->recommendations));
}

void
Museek::IfaceManager::onServerGlobalRecommendationsReceived(const SGetGlobalRecommendations * message)
{
  SEND_MASK(EM_INTERESTS, IGetGlobalRecommendations(message->recommendations));
}

void
Museek::IfaceManager::onServerSimilarUsersReceived(const SGetSimilarUsers * message)
{
  SEND_MASK(EM_INTERESTS, IGetSimilarUsers(message->users));
}

void
Museek::IfaceManager::onServerItemRecommendationsReceived(const SGetItemRecommendations * message)
{
  SEND_MASK(EM_INTERESTS, IGetItemRecommendations(message->item, message->recommendations));
}

void
Museek::IfaceManager::onServerItemSimilarUsersReceived(const SGetItemSimilarUsers * message)
{
  SEND_MASK(EM_INTERESTS, IGetItemSimilarUsers(message->item, message->users));
}


void
Museek::IfaceManager::onServerUserInterestsReceived(const SUserInterests * message)
{
  NNLOG("museekd.iface.debug", "%s has %d likes and %d hates", message->user.c_str(), message->likes.size(), message->hates.size());
  SEND_MASK(EM_USERINFO, IUserInterests(message->user, message->likes, message->hates));
}

void
Museek::IfaceManager::onPeerSocketUnavailable(std::string user)
{
  std::map<std::string, std::vector<NewNet::WeakRefPtr<IfaceSocket> > >::iterator it;
  it = m_PendingInfo.find(user);
  std::vector<std::string>::iterator wit = std::find(m_PendingInfoWaiting.begin(), m_PendingInfoWaiting.end(), user);
  if(it != m_PendingInfo.end())
    m_PendingInfo.erase(it);
  if (wit != m_PendingInfoWaiting.end())
    m_PendingInfoWaiting.erase(wit);

  it = m_PendingShares.find(user);
  wit = std::find(m_PendingSharesWaiting.begin(), m_PendingSharesWaiting.end(), user);
  if(it != m_PendingShares.end())
    m_PendingShares.erase(it);
  if (wit != m_PendingSharesWaiting.end())
    m_PendingSharesWaiting.erase(wit);
}

void
Museek::IfaceManager::onPeerSocketReady(PeerSocket * socket)
{
    std::vector<std::string>::iterator iit = std::find(m_PendingInfoWaiting.begin(), m_PendingInfoWaiting.end(), socket->user());
    if (iit != m_PendingInfoWaiting.end()) {
        socket->infoReceivedEvent.connect(this, &IfaceManager::onPeerInfoReceived);
        PInfoRequest msgI;
        socket->sendMessage(msgI.make_network_packet());
        m_PendingInfoWaiting.erase(iit);
    }

    std::vector<std::string>::iterator sit = std::find(m_PendingSharesWaiting.begin(), m_PendingSharesWaiting.end(), socket->user());
    if (sit != m_PendingSharesWaiting.end()) {
        socket->sharesReceivedEvent.connect(this, &IfaceManager::onPeerSharesReceived);
        PSharesRequest msgS;
        socket->sendMessage(msgS.make_network_packet());
        m_PendingSharesWaiting.erase(sit);
    }
}

void
Museek::IfaceManager::onPeerInfoReceived(const PInfoReply * message)
{
    PeerSocket * socket = message->peerSocket();
    std::map<std::string, std::vector<NewNet::WeakRefPtr<IfaceSocket> > >::iterator it;
    std::vector<NewNet::WeakRefPtr<IfaceSocket> >::iterator fit;
    it = m_PendingInfo.find(socket->user());
    if (it != m_PendingInfo.end()) {
        for (fit = it->second.begin(); fit != it->second.end(); fit++) {
            if (fit->isValid()) {
                IUserInfo msg(socket->user(), museekd()->codeset()->fromPeer(socket->user(), message->description), message->picture, message->totalupl, message->queuesize, message->slotfree);
                (*fit)->sendMessage(msg.make_network_packet());
            }
        }
    }

    if (it != m_PendingInfo.end())
        m_PendingInfo.erase(it);

    // Delete also from waiting (in case it isn't already done)
    std::vector<std::string>::iterator sit = std::find(m_PendingInfoWaiting.begin(), m_PendingInfoWaiting.end(), socket->user());
    if (sit != m_PendingInfoWaiting.end())
        m_PendingInfoWaiting.erase(sit);
}

void
Museek::IfaceManager::onPeerSharesReceived(const PSharesReply * message)
{
    PeerSocket * socket = message->peerSocket();

    // We need to convert the shares with correct encoding
    Shares oriShares = message->shares;
    Shares encShares;
    Shares::iterator itFold;
    Folder::iterator itFile;
    for(itFold = oriShares.begin(); itFold != oriShares.end(); ++itFold) {
        Folder newFold;
        for(itFile = itFold->second.begin(); itFile != itFold->second.end(); ++itFile) {
            newFold[museekd()->codeset()->fromPeer(socket->user(), itFile->first)] = itFile->second;
        }
        encShares[museekd()->codeset()->fromPeer(socket->user(), itFold->first)] = newFold;
    }

    std::map<std::string, std::vector<NewNet::WeakRefPtr<IfaceSocket> > >::iterator it;
    std::vector<NewNet::WeakRefPtr<IfaceSocket> >::iterator fit;
    it = m_PendingShares.find(socket->user());
    if (it != m_PendingShares.end()) {
        for (fit = it->second.begin(); fit != it->second.end(); fit++) {
            if (fit->isValid()) {
                IUserShares msg(socket->user(), encShares);
                (*fit)->sendMessage(msg.make_network_packet());
            }
        }
    }

    if (it != m_PendingShares.end())
        m_PendingShares.erase(it);

    // Delete also from waiting (in case it isn't already done)
    std::vector<std::string>::iterator sit = std::find(m_PendingSharesWaiting.begin(), m_PendingSharesWaiting.end(), socket->user());
    if (sit != m_PendingSharesWaiting.end())
        m_PendingSharesWaiting.erase(sit);
}

void
Museek::IfaceManager::onServerLoggedIn(const SLogin * message)
{

  if(message->success)
  {
    sendStatusMessage(false, message->greet);
  }
}

void
Museek::IfaceManager::sendStatusMessage(bool type, std::string message)
{
  SEND_ALL(IStatusMessage(type, message));
}

void
Museek::IfaceManager::onDownloadUpdated(Download * download)
{
  SEND_MASK(EM_TRANSFERS, ITransferUpdate(download));
}

void
Museek::IfaceManager::onDownloadRemoved(Download * download)
{
  SEND_MASK(EM_TRANSFERS, ITransferRemove(false, download->user(), download->remotePath()));
}

void
Museek::IfaceManager::onUploadUpdated(Upload * upload)
{
  SEND_MASK(EM_TRANSFERS, ITransferUpdate(upload));
}

void
Museek::IfaceManager::onUploadRemoved(Upload * upload)
{
    SEND_MASK(EM_TRANSFERS, ITransferRemove(true, upload->user(), museekd()->codeset()->fromFsToUtf8(upload->localPath())));
}

void
Museek::IfaceManager::onSearchReply(uint ticket, const std::string & user, bool slotfree, uint avgspeed, uint queuelen, const Folder & folders)
{
  SEND_ALL(ISearchReply(ticket, user,slotfree, avgspeed, queuelen, folders));
}

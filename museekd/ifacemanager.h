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

#ifndef MUSEEK_IFACEMANAGER_H
#define MUSEEK_IFACEMANAGER_H

#include "ifacesocket.h"
#include "servermessages.h"
#include "configmanager.h"
#include "peermessages.h"
#include <NewNet/nnobject.h>
#include <NewNet/nnrefptr.h>
#include <NewNet/nnweakrefptr.h>
#include <NewNet/nnclientsocket.h>
#include <NewNet/nnserversocket.h>

namespace NewNet
{
  class Reactor;
}

namespace Museek
{
  class IfaceSocket;
  class Museekd;
  class ServerManager;

  class IfaceManager : public NewNet::Object
  {
  public:
    void sendStatusMessage(bool type, std::string message);
    enum
    {
      EM_CHAT = 1,
      EM_PRIVATE = 2,
      EM_TRANSFERS = 4,
      EM_USERINFO = 8,
      EM_USERSHARES = 16,
      EM_INTERESTS = 32,
      EM_CONFIG = 64,
      EM_DEBUG = 128
    };

    IfaceManager(Museekd * museekd);

    Museekd * museekd() const
    {
      return m_Museekd;
    }

    // Some search results received
    void onSearchReply(uint ticket, const std::string & user, bool slotfree, uint avgspeed, uint queuelen, const Folder & folders);

    void sendNewSearchToAll(const std::string & query, uint token);

  private:
    bool addListener(const std::string & path);
    void removeListener(const std::string & path);

    void flushPrivateMessages();

    // Log event handler:
    void onLog(const NewNet::Log::LogNotify * notice);

    // Config changed listener events:
    void onConfigKeySet(const ConfigManager::ChangeNotify * data);
    void onConfigKeyRemoved(const ConfigManager::RemoveNotify * data);

    // Interface event handlers:
    void onIfaceAccepted(IfaceSocket * socket);
    void onIfaceDisconnected(NewNet::ClientSocket * socket);
    void onIfacePing(const IPing * message);
    void onIfaceLogin(const ILogin * message);
    void onIfaceCheckPrivileges(const ICheckPrivileges * message);
    void onIfaceSetStatus(const ISetStatus * message);
    void onIfaceNewPassword(const INewPassword * message);
    void onIfaceSetConfig(const IConfigSet * message);
    void onIfaceRemoveConfig(const IConfigRemove * message);
    void onIfaceSetUserImage(const IConfigSetUserImage * message);
    void onIfaceGetPeerExists(const IPeerExists * message);
    void onIfaceGetPeerStatus(const IPeerStatus * message);
    void onIfaceGetPeerStats(const IPeerStats * message);
    void onIfaceGetUserInfo(const IUserInfo * message);
    void onIfaceGetUserInterests(const IUserInterests * message);
    void onIfaceGetUserShares(const IUserShares * message);
    void onIfaceGetPeerAddress(const IPeerAddress * message);
    void onIfaceGivePrivileges(const IGivePrivileges * message);
    void onIfaceSendPrivateMessage(const IPrivateMessage * message);
    void onIfaceGetRoomList(const IRoomList * message);
    void onIfaceJoinRoom(const IJoinRoom * message);
    void onIfaceLeaveRoom(const ILeaveRoom * message);
    void onIfaceSayRoom(const ISayRoom * message);
    void onIfaceSetRoomTicker(const IRoomTickerSet * message);
    void onIfaceMessageUsers(const IMessageUsers * message);
    void onIfaceMessageBuddies(const IMessageBuddies * message);
    void onIfaceMessageDownloading(const IMessageDownloading * message);
    void onIfaceAskPublicChat(const IAskPublicChat * message);
    void onIfaceStopPublicChat(const IStopPublicChat * message);
    void onIfacePrivRoomToggle(const IPrivRoomToggle * message);
    void onIfacePrivRoomAddUser(const IPrivRoomAddUser * message);
    void onIfacePrivRoomRemoveUser(const IPrivRoomRemoveUser * message);
    void onIfacePrivRoomAddOperator(const IPrivRoomAddOperator * message);
    void onIfacePrivRoomRemoveOperator(const IPrivRoomRemoveOperator * message);
    void onIfacePrivRoomDismember(const IPrivRoomDismember * message);
    void onIfacePrivRoomDisown(const IPrivRoomDisown * message);

    void onIfaceStartSearch(const ISearch * message);
    void onIfaceStartUserSearch(const IUserSearch * message);
    void onIfaceStartWishListSearch(const IWishListSearch * message);
    void onIfaceAddWishItem(const IAddWishItem * message);
    void onIfaceRemoveWishItem(const IRemoveWishItem * message);
    void onIfaceGetRecommendations(const IGetRecommendations * message);
    void onIfaceGetGlobalRecommendations(const IGetGlobalRecommendations * message);
    void onIfaceGetSimilarUsers(const IGetSimilarUsers * message);
    void onIfaceGetItemRecommendations(const IGetItemRecommendations * message);
    void onIfaceGetItemSimilarUsers(const IGetItemSimilarUsers * message);
    void onIfaceAddInterest(const IAddInterest * message);
    void onIfaceRemoveInterest(const IRemoveInterest * message);
    void onIfaceAddHatedInterest(const IAddHatedInterest * message);
    void onIfaceRemoveHatedInterest(const IRemoveHatedInterest * message);
    void onIfaceConnectToServer(const IConnectServer * message);
    void onIfaceDisconnectFromServer(const IDisconnectServer * message);
    void onIfaceReloadShares(const IReloadShares * message);
    void onIfaceDownloadFile(const IDownloadFile * message);
    void onIfaceDownloadFileTo(const IDownloadFileTo * message);
    void onIfaceDownloadFolder(const IDownloadFolder * message);
    void onIfaceDownloadFolderTo(const IDownloadFolderTo * message);
    void onIfaceRemoveTransfer(const ITransferRemove * message);
    void onIfaceUpdateTransfer(const ITransferUpdate * message);
    void onIfaceAbortTransfer(const ITransferAbort * message);
    void onIfaceUploadFile(const IUploadFile * message);
    void onIfaceUploadFolder(const IUploadFolder * message);

    // Server event handlers:
    void onServerLoggedIn(const SLogin * message);
    void onServerLoggedInStateChanged(bool loggedIn);
    void onServerTimeDiffReceived(long);
    void onServerKicked(const SKicked* message);
    void onServerPeerAddressReceived(const SGetPeerAddress * message);
    void onServerAddUserReceived(const SAddUser * message);
    void onServerUserStatusReceived(const SGetStatus * message);
    void onServerPrivateMessageReceived(const SPrivateMessage * message);
    void onServerRoomMessageReceived(const SSayRoom * message);
    void onServerRoomJoined(const SJoinRoom * message);
    void onServerRoomLeft(const SLeaveRoom * message);
    void onServerUserJoinedRoom(const SUserJoinedRoom * message);
    void onServerUserLeftRoom(const SUserLeftRoom * message);
    void onServerRoomListReceived(const SRoomList * message);
    void onServerPrivilegesReceived(const SCheckPrivileges * message);
    void onServerRoomTickersReceived(const SRoomTickers * message);
    void onServerRoomTickerAdded(const SRoomTickerAdd * message);
    void onServerRoomTickerRemoved(const SRoomTickerRemove * message);
    void onServerRecommendationsReceived(const SGetRecommendations * message);
    void onServerGlobalRecommendationsReceived(const SGetGlobalRecommendations * message);
    void onServerSimilarUsersReceived(const SGetSimilarUsers * message);
    void onServerItemRecommendationsReceived(const SGetItemRecommendations * message);
    void onServerItemSimilarUsersReceived(const SGetItemSimilarUsers * message);
    void onServerUserInterestsReceived(const SUserInterests * message);
    void onServerNewPasswordSet(const SNewPassword * message);
    void onServerPublicChatReceived(const SPublicChat * message);

    void onServerPrivRoomToggled(const SPrivRoomToggle * message);
    void onServerPrivRoomAlterableMembers(const SPrivRoomAlterableMembers * message);
    void onServerPrivRoomAlterableOperators(const SPrivRoomAlterableOperators * message);
    void onServerPrivRoomAddedUser(const SPrivRoomAddUser * message);
    void onServerPrivRoomRemovedUser(const SPrivRoomRemoveUser * message);
    void onServerPrivRoomAddedOperator(const SPrivRoomAddOperator * message);
    void onServerPrivRoomRemovedOperator(const SPrivRoomRemoveOperator * message);

    // Peer event handlers:
    void onPeerSocketUnavailable(std::string user);
    void onPeerSocketReady(PeerSocket * socket);
    void onPeerInfoReceived(const PInfoReply * message);
    void onPeerSharesReceived(const PSharesReply * message);

    // Download event handlers:
    void onDownloadUpdated(Download * download);
    void onDownloadRemoved(Download * download);

    // Upload event handlers:
    void onUploadUpdated(Upload * upload);
    void onUploadRemoved(Upload * upload);

    NewNet::WeakRefPtr<Museekd> m_Museekd;

    std::map<std::string, NewNet::RefPtr<NewNet::Object> > m_Factories;
    std::map<std::string, NewNet::RefPtr<NewNet::ServerSocket> > m_ServerSockets;
    std::vector<NewNet::RefPtr<IfaceSocket> > m_Ifaces;

    // Pending requests:
    std::map<std::string, std::vector<NewNet::WeakRefPtr<IfaceSocket> > > m_PendingInfo, m_PendingShares;
    std::vector<std::string> m_PendingInfoWaiting, m_PendingSharesWaiting; // List of users we don't have yet asked the pendinginfo/shares

    // Cached data:
    uint32 m_AwayState;

    RoomList m_RoomList;
    PrivRoomList m_PrivRoomList;
    PrivRoomOperators m_PrivRoomOperators;
    PrivRoomOwners m_PrivRoomOwners;
    std::map<std::string, RoomData> m_RoomData;
    std::map<std::string, Tickers> m_TickerData;

    std::map<std::string, std::vector<std::string> > m_PrivRoomAlterableMembers;
    std::map<std::string, std::vector<std::string> > m_PrivRoomAlterableOperators;

    bool m_ReceivedTimeDiff;

    struct PrivateMessage
    {
      uint32 ticket;
      uint32 timestamp;
      std::string user;
      std::string message;
    };
    std::vector<PrivateMessage> m_PrivateMessages;
  };
}

#endif // MUSEEK_IFACEMANAGER_H

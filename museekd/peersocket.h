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

#ifndef MUSEEK_PEERSOCKET_H
#define MUSEEK_PEERSOCKET_H

#include "usersocket.h"
#include "messageprocessor.h"
#include "peermessages.h"
#include "servermessages.h"

namespace Museek
{
  class Museekd;
  class IfaceManager;
  class HandshakeSocket;

  class PeerSocket : public UserSocket, public MessageProcessor
  {
  public:
    PeerSocket(Museekd * museekd);
    PeerSocket(HandshakeSocket * that);
    ~PeerSocket();

    void addSearchResultsOnlyTimeout(long length = 2000);
    void onSearchResultsOnly(long);
    void initiateOurself();
    void stopConnectOurself();

    #define MAP_MESSAGE(ID, TYPE, EVENT) NewNet::Event<const TYPE *> EVENT;
    #include "peereventtable.h"
    #undef MAP_MESSAGE

  private:
    void connectMessageSignals();

    void onMessageReceived(const MessageData * data);
    void onDataReceived(NewNet::ClientSocket * socket);

    void onInfoRequested(const PInfoRequest *);
    void onSharesRequested(const PSharesRequest *);
    void onUploadQueueNotificationReceived(const PUploadQueueNotification *);
    void onTransferRequested(const PTransferRequest *);
    void onFolderContentsRequested(const PFolderContentsRequest * message);
    void onFolderContentsReceived(const PFolderContentsReply * message);
    void onPlaceInQueueRequested(const PPlaceInQueueRequest * message);
    void onPlaceInQueueReplyReceived(const PPlaceInQueueReply * message);
    void onQueueDownloadRequested(const PQueueDownload * message);
    void onQueueFailedReceived(const PQueueFailed * message);
    void onUploadFailedReceived(const PUploadFailed * message);
    void onSearchResultsReceived(const PSearchReply * message);
    void onConnected(NewNet::ClientSocket *);
    void onDisconnected(NewNet::ClientSocket *);

    void onSocketTimeout(long);

	NewNet::WeakRefPtr<NewNet::Event<long>::Callback> m_SearchResultsOnlyTimeout;
	NewNet::WeakRefPtr<NewNet::Event<long>::Callback> m_SocketTimeout;
	NewNet::WeakRefPtr<NewNet::Event<NewNet::ClientSocket *>::Callback> m_CannotConnectOurselfCallback; // Callback to the transferreply event
  };
}

#endif // MUSEEK_PEERSOCKET_H

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
#include "peersocket.h"
#include "museekd.h"
#include "codesetmanager.h"
#include "configmanager.h"
#include "handshakesocket.h"
#include "handshakemessages.h"
#include "servermanager.h"
#include "peermanager.h"
#include "ifacemanager.h"
#include "downloadmanager.h"
#include "downloadsocket.h"
#include "uploadmanager.h"
#include "uploadsocket.h"
#include "searchmanager.h"
#include "sharesdatabase.h"
#include <Muhelp/string_ext.hh>
#include <fstream>
#include <NewNet/nnratelimiter.h>
#include <NewNet/nnpath.h>
#include <NewNet/nntcpserversocket.h>

Museek::PeerSocket::PeerSocket(Museek::Museekd * museekd) : Museek::UserSocket(museekd, "P"), Museek::MessageProcessor(4)
{
  connectMessageSignals();
}

Museek::PeerSocket::PeerSocket(Museek::HandshakeSocket * that) : Museek::UserSocket(that, "P"), Museek::MessageProcessor(4)
{
  connectMessageSignals();

  // If there's no activity within the next 130 seconds, then the socket should be closed (timeout)
  m_SocketTimeout = museekd()->reactor()->addTimeout(130000, this, &PeerSocket::onSocketTimeout);

  if(! receiveBuffer().empty())
    dataReceivedEvent(this);
}

Museek::PeerSocket::~PeerSocket()
{
    NNLOG("museekd.peers.debug", "PeerSocket %d destroyed for %s", descriptor(), user().c_str());
}

void
Museek::PeerSocket::connectMessageSignals()
{
    dataReceivedEvent.connect(this, &TcpMessageSocket::onDataReceived);
    dataReceivedEvent.connect(this, &PeerSocket::onDataReceived);
    messageReceivedEvent.connect(this, &PeerSocket::onMessageReceived);
    infoRequestedEvent.connect(this, &PeerSocket::onInfoRequested);
    sharesRequestedEvent.connect(this, &PeerSocket::onSharesRequested);
    uploadQueueNotificationReceivedEvent.connect(this, &PeerSocket::onUploadQueueNotificationReceived);
    transferRequestedEvent.connect(this, &PeerSocket::onTransferRequested);
    folderContentsRequestedEvent.connect(this, &PeerSocket::onFolderContentsRequested);
    folderContentsReceivedEvent.connect(this, &PeerSocket::onFolderContentsReceived);
    placeInQueueRequestedEvent.connect(this, & PeerSocket::onPlaceInQueueRequested);
    placeInQueueReceivedEvent.connect(this, & PeerSocket::onPlaceInQueueReplyReceived);
    queueDownloadRequestedEvent.connect(this, & PeerSocket::onQueueDownloadRequested);
    queueFailedEvent.connect(this, & PeerSocket::onQueueFailedReceived);
    uploadFailedEvent.connect(this, & PeerSocket::onUploadFailedReceived);
    searchResultsReceivedEvent.connect(this, & PeerSocket::onSearchResultsReceived);
    connectedEvent.connect(this, & PeerSocket::onConnected);
    disconnectedEvent.connect(this, & PeerSocket::onDisconnected);
}

void
Museek::PeerSocket::onConnected(NewNet::ClientSocket *)
{
    // If there's no activity within the next 130 seconds, then the socket should be closed (timeout)
    if (m_SocketTimeout.isValid())
        museekd()->reactor()->removeTimeout(m_SocketTimeout);

    m_SocketTimeout = museekd()->reactor()->addTimeout(130000, this, &PeerSocket::onSocketTimeout);
}

void
Museek::PeerSocket::onDisconnected(NewNet::ClientSocket *)
{
    if (m_SearchResultsOnlyTimeout.isValid())
        museekd()->reactor()->removeTimeout(m_SearchResultsOnlyTimeout);

    if (m_SocketTimeout.isValid())
        museekd()->reactor()->removeTimeout(m_SocketTimeout);
}

/*
    Some data has been received
*/
void
Museek::PeerSocket::onDataReceived(NewNet::ClientSocket * socket)
{
  if (m_SocketTimeout.isValid()) {
    museekd()->reactor()->removeTimeout(m_SocketTimeout);

    // If there's no activity in the next 130 seconds, then the socket should be closed (timeout)
    m_SocketTimeout = museekd()->reactor()->addTimeout(130000, this, &PeerSocket::onSocketTimeout);
  }
}

void
Museek::PeerSocket::onMessageReceived(const MessageData * data)
{
  if (m_SearchResultsOnlyTimeout.isValid())
    museekd()->reactor()->removeTimeout(m_SearchResultsOnlyTimeout);

  if (m_SocketTimeout.isValid()) {
    museekd()->reactor()->removeTimeout(m_SocketTimeout);

    // If there's no activity in the next 130 seconds, then the socket should be closed (timeout)
    m_SocketTimeout = museekd()->reactor()->addTimeout(130000, this, &PeerSocket::onSocketTimeout);
  }

  switch(data->type)
  {
    #define MAP_MESSAGE(ID, TYPE, EVENT) \
      case ID: \
      { \
        NNLOG("museek.messages.peer", "Received peer message " #TYPE "."); \
        TYPE msg; \
        msg.setPeerSocket(this); \
        msg.parse_network_packet(data->data, data->length); \
        EVENT(&msg); \
        break; \
      }
    #include "peereventtable.h"
    #undef MAP_MESSAGE

    default:
        NNLOG("museekd.peers.warn", "Received unknown peer message, type: %u, length: %u", data->type, data->length);
        NetworkMessage msg;
        msg.parse_network_packet(data->data, data->length);
  }
}

void
Museek::PeerSocket::onInfoRequested(const PInfoRequest *)
{
  std::string text = museekd()->config()->get("userinfo", "text");
  std::string path = museekd()->config()->get("userinfo", "image");
  std::vector<uchar> imgdata;
  if(path != "")
  {
    std::ifstream ifs(path.c_str(), std::ifstream::in);
    ifs.seekg (0, std::ios_base::end);
    uint length = ifs.tellg();
    ifs.seekg (0, std::ios_base::beg);
    if (length > 0) {
        while(ifs.good())
        {
          imgdata.push_back((unsigned char)ifs.get());
        }
    }
    ifs.close();
  }

  PInfoReply reply(museekd()->codeset()->toPeer(user(), text), imgdata, museekd()->upSlots(), museekd()->uploads()->queueTotalLength(), museekd()->uploads()->hasFreeSlots());
  sendMessage(reply.make_network_packet());
  museekd()->ifaces()->sendStatusMessage(true, std::string("User Info sent to: ") + user());
}

void
Museek::PeerSocket::onSharesRequested(const PSharesRequest *)
{
    SharesDatabase* db;
    if (museekd()->isBuddied(user()))
        db = museekd()->buddyshares();
    else
        db = museekd()->shares();

    PSharesReply reply(db->shares());
    sendMessage(reply.make_network_packet());

    museekd()->ifaces()->sendStatusMessage(true, std::string("Shares sent to: ") + user());
}

void
Museek::PeerSocket::onPlaceInQueueRequested(const PPlaceInQueueRequest * message) {
    size_t place = museekd()->uploads()->queueLength(user(), museekd()->codeset()->fromPeerToFS(user(), message->filename));

    PPlaceInQueueReply reply(message->filename, place);
    sendMessage(reply.make_network_packet());
}

void
Museek::PeerSocket::onPlaceInQueueReplyReceived(const PPlaceInQueueReply * message) {
    Download * download = museekd()->downloads()->findDownload(user(), museekd()->codeset()->fromPeer(user(), message->filename));

    if (download)
        download->setPlace(message->place);
}

void
Museek::PeerSocket::onUploadQueueNotificationReceived(const PUploadQueueNotification *)
{
  std::string state = " is not a buddy";
  if (museekd()->config()->hasKey("buddies", user()))
    state = " is a buddy";

  std::string isbuddy = user() + state;
  NNLOG("museekd.peers.debug", isbuddy.c_str());
}

void
Museek::PeerSocket::onTransferRequested(const PTransferRequest * request)
{
  std::string reason;
  uint64 size = 0;
  bool allowed = false;
  if (request->direction == 0)
    {
        // Since slsk 157, we're not supposed to received an upload request like this.
        // The peer sends us PQueueDownload and then WE send him transfer request.
        // So this code is not much useful anymore. That's why we haven't fixed the case problem (see Upload::setCaseProblem())
        std::string path = museekd()->codeset()->toNet(museekd()->codeset()->fromPeer(user(), request->filename));

        if (museekd()->uploads()->isUploadable(user(), path, &reason)) {
	        NNLOG("museekd.peers.debug", "shared");

            std::string pathFS = museekd()->codeset()->fromNetToFS(request->filename);
	        museekd()->uploads()->add(user(), pathFS);
	        Upload* upload = museekd()->uploads()->findUpload(user(), pathFS);
	        if (upload && museekd()->uploads()->hasFreeSlots() && !museekd()->uploads()->isUploadingTo(user())) {
		        NNLOG("museekd.peers.debug", "slot free");

		        if (!upload->openFile()) {
                    reason = "Remote file error";
			        NNLOG("museekd.peers.warn", "Local file error on %s", pathFS.c_str());
                }
                else {
                    UploadSocket * uploadSocket = new UploadSocket(museekd(), upload);
                    upload->setSocket(uploadSocket);
                    upload->setState(TS_Establishing);
                    upload->setTicket(request->ticket);
                    museekd()->reactor()->add(uploadSocket);
                    uploadSocket->wait();
			        NNLOG("museekd.peers.debug", "Initiating transfer, ticket %i", request->ticket);
			        size = upload->size();
                }
	        }
	        else {
		        NNLOG("museekd.peers.debug", "queued");
		        reason = "Queued";
	        }
        }

	    if (size > 0) {
            PUploadReply reply(request->ticket, size);
            sendMessage(reply.make_network_packet());
        }
	    else {
            PUploadReply reply(request->ticket, reason);
            sendMessage(reply.make_network_packet());
        }
    }
  else if (request->direction == 1)
    {
      NNLOG("museekd.peers.debug", "request for download %s %s %u", user().c_str(), request->filename.c_str(), request->ticket);
      // Starting a download
      std::string path = museekd()->codeset()->fromPeer(user(), request->filename);
      Download * download = museekd()->downloads()->findDownload(user(), path);
      if(download)
      {
        // Check that we don't already have this file downloaded in destination dir
        std::ifstream file(download->destinationPath().c_str(), std::fstream::in | std::fstream::binary);
        if(file.is_open()) {
            NNLOG("museekd.peers.debug", "%s has already been downloaded.", path.c_str());
            download->setState(TS_Finished);
            PDownloadReply reply(request->ticket, allowed, "Finished");
            sendMessage(reply.make_network_packet());
            file.close();
        }

        if (!museekd()->downloads()->hasFreeSlots()) {
          PDownloadReply reply(request->ticket, allowed, "Remotely Queued");
          sendMessage(reply.make_network_packet());
        }
        else if (download->state() == TS_QueuedRemotely
                || download->state() == TS_Initiating
                || download->state() == TS_ConnectionClosed
                || download->state() == TS_CannotConnect
                || download->state() == TS_RemoteError
                || download->state() == TS_LocalError
                || download->state() == TS_Offline) {
            // If we're already downloading/initiating another download from this user, stop it
            Download * downloading = museekd()->downloads()->isDownloadingFrom(user());
            if (downloading && downloading != download) {
                downloading->setSocket(0);
                downloading->setState(TS_QueuedRemotely);
                downloading->setEnqueued(false);
                museekd()->downloads()->enqueueDownload(downloading);
            }
            // Prepare the download
            download->setTicket(request->ticket);
            download->setSize(request->filesize);
            DownloadSocket * downloadSocket = new DownloadSocket(museekd(), download);
            download->setSocket(downloadSocket);
            museekd()->reactor()->add(downloadSocket);
            downloadSocket->wait();
            PDownloadReply reply(request->ticket, true);
            sendMessage(reply.make_network_packet());
        }
        else {
            PDownloadReply reply(request->ticket, allowed, "Cancelled");
            sendMessage(reply.make_network_packet());
        }
      }
      else if (museekd()->trustingUploads() && museekd()->isTrusted(user())) {
        // If we're already downloading/initiating another download from this user, stop it
        Download * downloading = museekd()->downloads()->isDownloadingFrom(user());
        if (downloading && downloading != download) {
            downloading->setSocket(0);
            downloading->setState(TS_QueuedRemotely);
            downloading->setEnqueued(false);
            museekd()->downloads()->enqueueDownload(downloading);
        }
        // Prepare the download
        std::stringstream localPath;
        // Construct the destination path
        localPath << museekd()->config()->get("transfers", "download-dir") << NewNet::Path::separator() << "upload" << NewNet::Path::separator()  << museekd()->codeset()->fromNetToFS(user());
        // Create the download and prepare it
        museekd()->downloads()->add(user(), path, localPath.str());
        Download * newDownload = museekd()->downloads()->findDownload(user(), path);
        if (!newDownload)
            return; // Check this just in case it disappers between add() and find() calls

        newDownload->setTicket(request->ticket);
        newDownload->setSize(request->filesize);

        if (!museekd()->downloads()->hasFreeSlots()) {
            PDownloadReply reply(request->ticket, allowed, "Remotely Queued");
            sendMessage(reply.make_network_packet());
        }
        else {
            DownloadSocket * downloadSocket = new DownloadSocket(museekd(), newDownload);
            newDownload->setSocket(downloadSocket);
            museekd()->reactor()->add(downloadSocket);
            downloadSocket->wait();
            PDownloadReply reply(request->ticket, true);
            sendMessage(reply.make_network_packet());
        }
      }
      else {
        PDownloadReply reply(request->ticket, allowed, "Forbidden");
        sendMessage(reply.make_network_packet());
      }
    }
}

/*
    The peer asks us to put an upload in queue. We don't need to check if it is allowed.
*/
void
Museek::PeerSocket::onQueueDownloadRequested(const PQueueDownload * message) {
    std::string reason, goodPath;

    NNLOG("museekd.peers.debug", "request for queued upload %s %s", user().c_str(), message->filename.c_str());

    if (museekd()->uploads()->isUploadable(user(), message->filename, &reason)) {
        museekd()->uploads()->add(user(), museekd()->codeset()->fromNetToFS(message->filename));
    }
    else if ((reason == "File not shared") && museekd()->uploads()->findUploadableNoCase(user(), message->filename, &goodPath)) {
        // We don't have this file!
        // This happens frequently after sending a PFolderContentReply to a peer.
        // If he's running the official client, we will get PQueueDownload with
        // the filename in lower case. If in our shares the path has upper letters, we'll throw "File not shared" error.
        // So let's try a case insensitive search: slower than previous one, with a risk to return a wrong file.
        NNLOG("museekd.peers.debug", "File not shared but found a case insensitive match");
        museekd()->uploads()->add(user(), museekd()->codeset()->fromNetToFS(goodPath),0 , true);
    }
    else {
        PQueueFailed msg(message->filename, reason);
        sendMessage(msg.make_network_packet());
    }
}

/*
    We asked to queue a download but there's a problem
*/
void
Museek::PeerSocket::onQueueFailedReceived(const PQueueFailed * message)
{
    std::string path = museekd()->codeset()->fromPeer(user(), message->filename);
    Download * download = museekd()->downloads()->findDownload(user(), path);
    if(download)
        download->setRemoteError(message->reason);
}

/*
    We asked to download a file but there's a problem
*/
void
Museek::PeerSocket::onUploadFailedReceived(const PUploadFailed * message)
{
    std::string path = museekd()->codeset()->fromPeer(user(), message->filename);
    Download * download = museekd()->downloads()->findDownload(user(), path);
    if(download && download->state() != TS_Aborted && download->state() != TS_LocalError)
        download->setRemoteError("Failed");
}

void
Museek::PeerSocket::onFolderContentsRequested(const PFolderContentsRequest * message)
{
    if (! museekd()->isBanned(user())) {
        std::vector<std::string>::const_iterator it;
        Folders reply;
        for (it = message->dirs.begin(); it != message->dirs.end(); it++) {
            std::string dir = *it;
            Shares content;
            if (museekd()->haveBuddyShares() && museekd()->isBuddied(user())) {
                content = museekd()->buddyshares()->folder_contents(dir);
                if (content.size() > 0)
                    reply[dir] = content;
            }
            else {
                content = museekd()->shares()->folder_contents(dir);
                if (content.size() > 0)
                    reply[dir] = content;
            }
        }

        if (reply.size() > 0) {
            PFolderContentsReply msg(reply);
            sendMessage(msg.make_network_packet());
        }
    }
}

void
Museek::PeerSocket::onFolderContentsReceived(const PFolderContentsReply * message) {
    museekd()->downloads()->addFolderContents(user(), message->folders);
}

void
Museek::PeerSocket::onSearchResultsReceived(const PSearchReply * message) {
    NNLOG("museekd.peers.debug", "Search result from %s", user().c_str());

    Folder folders;
    Folder::const_iterator it = message->results.begin();
    for(; it != message->results.end(); ++it) {
        std::string encodedFilename = museekd()->codeset()->fromPeer(user(), (*it).first);
        folders[encodedFilename] = (*it).second;
        }

    museekd()->searches()->searchReplyReceived(message->ticket, user(), message->slotfree, message->avgspeed, message->queuelen, folders);

    addSearchResultsOnlyTimeout(2000);
}

void
Museek::PeerSocket::addSearchResultsOnlyTimeout(long length) {
    // If we don't receive any other message in the next seconds, we should delete this socket as:
    // -we will probably not receive anything else soon
    // -there's a limit on the number of opened sockets (1024 for example): this can be a problem when doing big searches
    if (m_SearchResultsOnlyTimeout.isValid())
        museekd()->reactor()->removeTimeout(m_SearchResultsOnlyTimeout);
    m_SearchResultsOnlyTimeout = museekd()->reactor()->addTimeout(length, this, &PeerSocket::onSearchResultsOnly);
}

void
Museek::PeerSocket::onSearchResultsOnly(long) {
    NNLOG("museekd.peers.debug", "We only received or sent search results: close this socket");
    disconnect();
}

void
Museek::PeerSocket::onSocketTimeout(long) {
    NNLOG("museekd.peers.debug", "Ping timeout on a peer socket");
    disconnect();
}

void
Museek::PeerSocket::initiateOurself()
{
    setToken(museekd()->token());

    NNLOG("museekd.distrib.debug", "Initiating active connection to ourself.");

    HInitiate handshake(museekd()->server()->username(), type(), token());
    sendMessage(handshake.make_network_packet());

    m_CannotConnectOurselfCallback = cannotConnectEvent.connect(this, & PeerManager::onCannotConnectOurself);

    uint port = museekd()->peers()->peerFactory()->serverSocket()->listenPort();

    if(port == 0) {
        cannotConnectEvent(this);
        return;
    }
    connect("127.0.0.1", port);
}

void
Museek::PeerSocket::stopConnectOurself() {
    disconnect(false);

    if (m_CannotConnectOurselfCallback.isValid()) {
        cannotConnectEvent.disconnect(m_CannotConnectOurselfCallback);
        m_CannotConnectOurselfCallback = 0;
    }

    sendBuffer().clear(); // We have a HInitiate message still waiting in the buffer. We don't need it anymore
}

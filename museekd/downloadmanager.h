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

#ifndef MUSEEK_DOWNLOADMANAGER_H
#define MUSEEK_DOWNLOADMANAGER_H

#include <NewNet/nnobject.h>
#include <NewNet/nnweakrefptr.h>
#include <NewNet/nnrefptr.h>
#include <NewNet/nnevent.h>
#include "configmanager.h"
#include "mutypes.h"

/* Forward declarations. */
class SGetStatus;
class PTransferReply;

namespace NewNet
{
  class ClientSocket;
  class RateLimiter;
}

namespace Museek
{
  class Museekd;
  class PeerSocket;
  class TicketSocket;
  class DownloadSocket;

  /* Definition of the download structure. */
  class DownloadManager;
  class Download : public NewNet::Object
  {
  public:
    Download(Museekd * museekd, const std::string & user, const std::string & remotePath, const std::string & localDir = std::string());
    ~Download();

    void initiate(PeerSocket * socket);

    Museekd * museekd() const { return m_Museekd; }

    DownloadSocket * socket() const { return m_Socket; }
    void setSocket(DownloadSocket * socket);

    uint ticket() const { return m_Ticket; }
    void setTicket(uint ticket) { m_Ticket = ticket; }
    const std::string & user() const { return m_User; }
    bool enqueued() const { return m_Enqueued;}
    void setEnqueued(bool e) { m_Enqueued = e; }

    const std::string & remotePath() const { return m_RemotePath; }
    const std::string & localDir() const { return m_LocalDir; }
    void setLocalDir(const std::string & localDir) { m_LocalDir = localDir; }
    std::string filename() const;
    std::string incompletePath() const;
    void setIncompletePath(const std::string & path) const { m_IncompletePath = path;};
    std::string destinationPath(bool create = false) const;

    uint64 size() const { return m_Size; }
    void setSize(uint64 size);
    uint64 position() const { return m_Position; }
    void setPosition(uint64 position);
    void setPositionFromIncompleteFile();

    void received(uint bytes);

    TrState state() const { return m_State; }
    void setState(TrState state);
    const std::string & error() const { return m_Error; }
    void setRemoteError(const std::string & error);

    void retry(long);

    uint rate() const { return m_Rate; }
    uint place() const { return m_Place; }
    void setPlace(uint place);

    void setInitTimeout(NewNet::WeakRefPtr<NewNet::Event<long>::Callback> ref) {m_InitTimeout = ref;};
    void initTimedOut(long);

  private:
    NewNet::WeakRefPtr<Museekd>         m_Museekd; // Ref to the museekd

    NewNet::WeakRefPtr<DownloadSocket>  m_Socket; // Ref to the socket associated

    uint                                m_Ticket; // The download ticket with which this download is identified
    std::string                         m_User; // Name of the user
    bool                                m_Enqueued; // Have we already enqueued this download?

    std::string                         m_RemotePath; // Path of the file in user's shares
    std::string                         m_LocalDir; // Dir where we need to store downloaded file when finished
    mutable std::string                 m_Filename; // Name of the file
    mutable std::string                 m_IncompletePath; // Complete path where to store the incomplete file

    uint64                               m_Size; // Size of this file
    uint64                               m_Position; // Current position of this file

    TrState                             m_State; // Transfer state (see mutypes.h)
    std::string                         m_Error; // Error message if state = TR_Error

	std::vector<uint>                   m_RatePool; // 10 last download rates collected
	uint                                m_Rate; // The average dowload rate

	struct timeval                      m_CollectStart; // When did we start downloading this file
	uint                                m_Collected; // How many data has been sent since m_CollectStart ?

	uint                                m_Place; // The place in queue for this download

    NewNet::WeakRefPtr<NewNet::Event<long>::Callback> m_InitTimeout; // Used to avoir waiting too long in initiating mode
  };

  /* The download manager manages .. downloads. */
  class DownloadManager : public NewNet::Object
  {
  public:
    DownloadManager(Museekd * museekd);
    ~DownloadManager();

    /* Return pointer to museekd instance. */
    Museekd * museekd() const { return m_Museekd; }

    /* List all downloads. */
    const std::vector<NewNet::RefPtr<Download> > & downloads() const { return m_Downloads; }

    /* Add a new download or retry an existing one. */
    void add(const std::string & user, const std::string & path, const std::string & localPath = std::string(), const uint & ticket = 0);
    /* Download a folder: first get the folder contents, then download
       all the files it contains. */
    void addFolder(const std::string & user, const std::string & path, const std::string & localPath = std::string());
    /* Abort a download. */
    void abort(const std::string & user, const std::string & path);
    /* Remove a download. If necessary, the transfer will first be aborted. */
    void remove(const std::string & user, const std::string & path);
    /* Update the given transfer (if available) */
    void update(const std::string & user, const std::string & path);

    /* Do we have any free download slots at this time. */
    bool hasFreeSlots();

    /* Find a download by it's path. */
    Download * findDownload(const std::string & user, const std::string & path);
    /* Find a download by it's ticket. */
    Download * findDownload(const std::string & user, uint ticket);

    /* Check if there's some upload to start */
    void checkDownloads();

    void updateRates();

    /* Analyse the folder contents we've received and if we can start some download with it */
    void addFolderContents(const std::string & user, const Folders & folders);

    void enqueueDownload(Download * download);

    Download * isDownloadingFrom(const std::string & user);

    void onDownloadAdded(Download * download);
    void onDownloadUpdated(Download * download);
    void onDownloadRemoved(Download * download);

    void onPeerTransferReplyReceived(const PTransferReply * message);

    void setTransferReplyCallback(NewNet::Event<const PTransferReply *>::Callback * cb) {m_TransferReplyCallback = cb;};

    void loadDownloads();
    void saveDownloads();

    NewNet::RateLimiter * limiter() {return m_Limiter;}

    /* A transfer connection was initiated by a remote peer. */
    NewNet::Event<TicketSocket *> transferTicketReceivedEvent;

    /* A new download was created. */
    NewNet::Event<Download *> downloadAddedEvent;
    /* A download was removed. */
    NewNet::Event<Download *> downloadRemovedEvent;
    /* A download was updated. */
    NewNet::Event<Download *> downloadUpdatedEvent;

  private:
    void addDownloading(Download * download);
    void removeDownloading(const std::string& user);

    void addInitiating(Download * download);
    void removeInitiating(const std::string& user);
    Download * isInitiatingFrom(const std::string & user);

    void onServerLoggedInStateChanged(bool loggedIn);
    void onPeerSocketUnavailable(std::string user);
    void onPeerSocketReady(PeerSocket * socket);
    void onPeerOffline(std::string user);
    void onConfigKeySet(const ConfigManager::ChangeNotify * data);
    void onConfigKeyRemoved(const ConfigManager::RemoveNotify * data);

    /* Send the foldercontents request pending a peer socket */
    void askPendingFolderContents(PeerSocket * socket);
    /* Send the place in queue request pending a peer socket */
    void askPendingPlaces(PeerSocket * socket);
    /* Send the place in queue request pending a peer socket */
    void askPendingEnqueuing(PeerSocket * socket);

    bool                                                    m_AllowUpdate;      // Set it to false if you don't want downloads
                                                                                // to be enqueued and saved
    bool                                                    m_AllowSave;        // Set it to false if you don't want downloads
                                                                                // to be saved
    bool                                                    m_PendingDownloadsSave; // Should we save downloads soon?
    NewNet::RefPtr<NewNet::RateLimiter>                     m_Limiter;          // Rate limiter shared between downloads
    NewNet::WeakRefPtr<Museekd>                             m_Museekd;          // Ref to the museekd
    std::vector<NewNet::RefPtr<Download> >                  m_Downloads;        // List of all the downloads
    std::map<std::string, NewNet::WeakRefPtr<Download> >    m_Initiating;       // List of all the downloads currently being initiated
    std::map<std::string, NewNet::WeakRefPtr<Download> >    m_Downloading;      // List of user we're currently uploading
    std::map<std::string, std::map<std::string, std::string> > m_ContentsAsked;    // List of the folder contents asked (waiting for the reply)
    std::map<std::string, std::map<std::string, std::string> > m_ContentsPending;    // List of the folder contents pending a user socket
    std::map<std::string, std::vector<std::string> >        m_PlacesPending;    // List of place requests pending a peer socket
    std::map<std::string, std::vector<std::string> >        m_EnqueuingPending; // List of enqueuing request pending a peer socket
    NewNet::WeakRefPtr<NewNet::Event<const PTransferReply *>::Callback>
                                                            m_TransferReplyCallback; // Callback to the transferreply event
  };
}

#endif // MUSEEK_DOWNLOADMANAGER_H

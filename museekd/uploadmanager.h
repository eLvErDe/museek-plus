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

#ifndef MUSEEK_UPLOADMANAGER_H
#define MUSEEK_UPLOADMANAGER_H

#include <NewNet/nnobject.h>
#include <NewNet/nnweakrefptr.h>
#include <NewNet/nnrefptr.h>
#include <NewNet/nnevent.h>
#include <NewNet/nnbuffer.h>
#include "mutypes.h"
#include "servermessages.h"
#include "configmanager.h"

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
  class UploadSocket;

  /* Definition of the upload structure. */
  class UploadManager;
  class Upload : public NewNet::Object
  {
  public:
    Upload(Museekd * museekd, const std::string & user, const std::string & localPath);
    ~Upload();

    void initiate(PeerSocket * socket);

    bool openFile();
    void closeFile();
    bool seek(uint64 pos);
    bool read(NewNet::Buffer & buffer);
    void sent(uint count);
    void collect(uint bytes);

    Museekd * museekd() const { return m_Museekd; }

    UploadSocket * socket() const { return m_Socket; }
    void setSocket(UploadSocket * socket);

    const std::string & user() const { return m_User; }
    const std::string & localPath() const { return m_LocalPath; }
    uint64 size() const { return m_Size; }
    uint64 position() const { return m_Position; }
    void setPosition(uint64 position);

    uint ticket() const { return m_Ticket; }
    void setTicket(uint ticket) { m_Ticket = ticket; }
	inline bool ticket_valid() const { return m_TicketValid; };
	inline void invalidateTicket() { m_TicketValid = false; };
	inline void validateTicket() { m_TicketValid = false; };

    TrState state() const { return m_State; }
    void setState(TrState state);
    const std::string & error() const { return m_Error; }
    void setRemoteError(const std::string & error);
    void setLocalError(const std::string & error);

    uint rate() const { return m_Rate; }

    bool hasCaseProblem() const {return m_CaseProblem;}
    void setCaseProblem(bool problem) {m_CaseProblem = problem;}

    /* A transfer connection was initiated by a remote peer. */
    NewNet::Event<TicketSocket *> transferTicketReceivedEvent;

  private:
    void replyTimeout(long);

    NewNet::WeakRefPtr<Museekd>         m_Museekd; // Ref to the museekd

    std::ifstream *                     m_File; // The file we need to send
    NewNet::WeakRefPtr<UploadSocket>    m_Socket; // Ref to the socket associated

    std::string                         m_User; // Name of the user
    std::string                         m_LocalPath; // Path to the file we have to upload
    uint64                               m_Size; // Size of this file
    uint64                               m_Position; // Current position of this file

    uint                                m_Ticket; // The upload ticket with which this upload is identified
	bool                                m_TicketValid; // Is this ticket still valid ?

    TrState                             m_State; // Transfer state (see mutypes.h)
    std::string                         m_Error; // Error message if state = TR_Error

	std::vector<uint>                   m_RatePool; // 10 last upload rates collected
	uint                                m_Rate; // The average upload rate(send as statistic to the server)

	struct timeval                      m_CollectStart; // When did we start uploading this file
	uint                                m_Collected; // How many data has been sent since m_CollectStart ?

	bool                                m_CaseProblem; // If this is true, the peer is waiting for a lowercase path

    NewNet::WeakRefPtr<NewNet::Event<long>::Callback> m_WaitingTimeout;
  };

  /* The upload manager manages .. uploads. */
  class UploadManager : public NewNet::Object
  {
  public:
    UploadManager(Museekd * museekd);
    ~UploadManager();

    /* Return pointer to museekd instance. */
    Museekd * museekd() const { return m_Museekd; }

    /* List all uploads. */
    const std::vector<NewNet::RefPtr<Upload> > & uploads() const { return m_Uploads; }

    /* Add a new upload or retry an existing one. */
    void add(const std::string & user, const std::string & localPath, const uint & ticket = 0, const bool caseProblem = false, const bool forceEnqueue = false);
    /* Add a new folder to upload */
    void addFolder(const std::string & user, const std::string & localPath);
    /* Abort a upload. */
    void abort(const std::string & user, const std::string & path);
    /* Remove a upload. If necessary, the transfer will first be aborted. */
    void remove(const std::string & user, const std::string & path);
    /* Update the given transfer (if available) */
    void update(const std::string & user, const std::string & path);

    /* Do we have any free upload slots at this time. */
    bool hasFreeSlots();

    bool isUploadable(const std::string & user, const std::string & path, std::string * error);
    bool findUploadableNoCase(const std::string & user, const std::string & path, std::string * goodPath);

    /* Find a upload by it's path. */
    Upload * findUpload(const std::string & user, const std::string & path);
    /* Find a upload by it's ticket. */
    Upload * findUpload(const std::string & user, uint ticket);

    /* Check if there's some upload to start */
    void checkUploads();

    void updateRates();

    uint queueLength(const std::string& user, const std::string& stopAt);

    uint queueTotalLength();

    Upload * isUploadingTo(const std::string & user);

    /* Returns the list of users in the upload queue (currently downloading or in not) */
    std::vector<std::string> getAllUsersWithUpload();

    void onUploadAdded(Upload * upload);
    void onUploadUpdated(Upload * upload);
    void onUploadRemoved(Upload * upload);

    void onPeerTransferReplyReceived(const PTransferReply * message);

    void setTransferReplyCallback(NewNet::Event<const PTransferReply *>::Callback * cb) {m_TransferReplyCallback = cb;};

    NewNet::RateLimiter * limiter() {return m_Limiter;}

    /* A transfer connection was initiated by a remote peer. */
    NewNet::Event<TicketSocket *> transferTicketReceivedEvent;

    /* A new upload was created. */
    NewNet::Event<Upload *> uploadAddedEvent;
    /* A upload was removed. */
    NewNet::Event<Upload *> uploadRemovedEvent;
    /* A upload was updated. */
    NewNet::Event<Upload *> uploadUpdatedEvent;

  private:
    void addUploading(Upload * upload);
    void removeUploading(const std::string& user);

    void addInitiating(Upload * upload);
    void removeInitiating(const std::string& user);
    Upload * isInitiatingTo(const std::string & user);

    void onServerLoggedInStateChanged(bool loggedIn);
    void onPeerSocketUnavailable(std::string user);
    void onPeerSocketReady(PeerSocket * socket);
    void onPeerOffline(std::string user);
    void onConfigKeySet(const ConfigManager::ChangeNotify * data);
    void onConfigKeyRemoved(const ConfigManager::RemoveNotify * data);

    NewNet::WeakRefPtr<Museekd>                             m_Museekd;      // Ref to the museekd
    std::vector<NewNet::RefPtr<Upload> >                    m_Uploads;      // List of all the uploads
    std::map<std::string, NewNet::WeakRefPtr<Upload> >      m_Initiating;   // List of all the uploads currently being initiated
    std::map<std::string, NewNet::WeakRefPtr<Upload> >      m_Uploading;    // List of user we're currently uploading
    NewNet::RefPtr<NewNet::RateLimiter>                     m_Limiter;      // Rate limiter shared between uploads
    NewNet::WeakRefPtr<NewNet::Event<const PTransferReply *>::Callback>
                                                            m_TransferReplyCallback; // Callback to the transferreply event
  };
}

#endif // MUSEEK_UPLOADMANAGER_H

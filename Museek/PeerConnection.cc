/* Museek - Museek's 'core' library
 *
 * Copyright (C) 2003-2004 Hyriand <hyriand@thegraveyard.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <Museek/mu.hh>

#include <Museek/PeerConnection.hh>
#include <Museek/PeerMessages.hh>
#include <Museek/PeerManager.hh>
#include <Museek/Museek.hh>
#include <Museek/ShareBase.hh>
#include <Museek/TransferManager.hh>
#include <Museek/Recoder.hh>

#define MULOG_DOMAIN "Museek.PC"
#include <Muhelp/Mulog.hh>

#include <string>
using std::string;
using std::wstring;

PeerConnection::PeerConnection(Peer* peer, uint32 token)
               : BaseConnection(4, "P", peer, token), mLocked(false) {
	CT("PeerConnection %s, %u", peer->user().c_str(), token);
	
	mUser = peer->user();
	mMuseek = peer->manager()->museek();
}

#ifdef PARSE
# undef PARSE
#endif
#define PARSE(mtype) mtype s; s.parse_network_packet(message);

void PeerConnection::process_message(uint32 code) {
	CT("process_message %u (%s)", code, peer()->user().c_str());
	
	switch(code) {
	case 4: {
		// Recieved a Request for Shares List
		if( mMuseek->is_banned(mUser)) {
			mMuseek->cb_peer_banned(mUser);
			break;
		}
		
		if (mUser == mMuseek->username()) {
			DEBUG("you are attempting to browse yourself... or being spoofed, blocked");
			break;
		}
		if (mMuseek->is_receiving_shares(mUser)) {
			DEBUG("already sent shared files list %s", mUser.c_str());
		} else {
			// Unfinished receiving users list
			// mMuseek->add_receiving(mUser);

			if ( mMuseek->mBuddySharesHave && mMuseek->is_buddied(mUser)) {
				lock();
				DEBUG("sending buddy shared files list %s", mUser.c_str());
				PSharesReply r(mMuseek->buddyshares()->shares());
				send(r);
				mMuseek->cb_peer_sent_buddy_shares(mUser);
				}
			else {
				lock();
				DEBUG("sending normal shared files list %s", mUser.c_str());
				PSharesReply r(mMuseek->shares()->shares());
				send(r);
				mMuseek->cb_peer_sent_normal_shares(mUser);
				}
			
		}
		break;
	}
	case 5: {
		// Recieved a Shares List; Send Data to User Interfaces
		PARSE(PSharesReply)
		DEBUG("got shared file list %s with %u directories", mUser.c_str(), s.shares.size());
		mMuseek->cb_peer_shares(mUser, mMuseek->recoder()->decode_shares(mUser, s.shares));
		break;
	}
	case 8: {
		// Recieved a Search Request; Check shares for a match
		PARSE(PSearchRequest);
		DEBUG("peer search request from %s: %s", mUser.c_str(), s.query.c_str());
		mMuseek->cb_peer_search(this, mUser, s.ticket, mMuseek->recoder()->decode_user(mUser, s.query));
		break;
	}
	case 9: {
		// Recieved a Search Reply (Results); Send Data to User Interfaces
		PARSE(PSearchReply);
		DEBUG("search results %s, %i", mUser.c_str(), s.results.size());
		if(! s.results.empty() or ! s.failed)
			mMuseek->cb_peer_results(s.ticket, mUser, mMuseek->recoder()->decode_folder(mUser, s.results),
			s.avgspeed, s.queuelen, s.slotfree);
		else if (s.failed) 
			DEBUG("Parsing Search Reply failed");
		/* yes, this is one of those evil-yet-functionalisms: */
		if(inbuf.empty())
			disconnect();
		
		break;
	}
	case 15: {
		// A User Info Request was recieved; Craft a User Info Reply
		if(! mMuseek->is_banned(mUser)) {
			if (mUser == mMuseek->username()) {
				DEBUG("you are attempting to get your own userinfo... or being spoofed, blocked");
				break;
			}
			DEBUG("sending user info to %s", mUser.c_str());
			lock();
			string newline = "\n";
			
			string descr = mMuseek->recoder()->encode_network(mMuseek->userinfo());

			std::string::size_type match = 0;
			while  ((match = descr.find("\n", match)) != string::npos){
				descr.insert(match, "\r");
				match += 2;
				}
			PInfoReply r(descr,
				mMuseek->userpic(),
				mMuseek->upload_slots(),
				mMuseek->transfer_manager()->queue_length(mUser),
				mMuseek->transfer_manager()->slot_free());
			send(r);
			mMuseek->cb_peer_sent_user_info(mUser);
		}
		break;
	}
	case 16: {
		// Recieved a User Info Reply; Send Data to User Interfaces
		PARSE(PInfoReply);
		DEBUG("got user info %s", mUser.c_str());
		mMuseek->cb_peer_info(mUser, 
		mMuseek->recoder()->decode_user(mUser, s.description), 
		s.picture, s.totalupl, s.queuesize, s.slotfree);
	
		break;
	}
	case 36: {
		// Folder Contents Request; Check shares for this folder, and send contents
		if(! mMuseek->is_banned(mUser)) {
			PARSE(PFolderContentsRequest);
			if ( mMuseek->mBuddySharesHave && mMuseek->is_buddied(mUser))
				DEBUG("got buddy folder contents request %s, %d", mUser.c_str(),s.dirs.size());
			else
				DEBUG("got folder contents request %s, %d", mUser.c_str(),s.dirs.size());
			
			Folders res;
			StringList::iterator it = s.dirs.begin();
			for(; it != s.dirs.end(); ++it) {
				DEBUG("%s", (*it).c_str());
				if(res.find(*it) != res.end())
					continue;
				if ( mMuseek->mBuddySharesHave && mMuseek->is_buddied(mUser))
					res[*it] = mMuseek->buddyshares()->folder_contents(*it);
				else
					res[*it] = mMuseek->shares()->folder_contents(*it);
				}
			PFolderContentsReply r(res);
			send(r);

		}
		break;
	}
	case 37: {
		// Recieved Folder Contents Reply; Try to download these files
		PARSE(PFolderContentsReply);
		DEBUG("got folder contents reply from %s <...> (%d)", mUser.c_str(), s.folders.size());
		Folders::iterator it = s.folders.begin();
		for(; it != s.folders.end(); ++it)
			mMuseek->cb_peer_folder_contents(mUser, mMuseek->recoder()->decode_user(mUser, (*it).first),   mMuseek->recoder()->decode_shares(mUser, (*it).second));
		break;
	}
	case 40: {
		PARSE(PTransferRequest);
		
		if (s.direction == 0) {
			// Recieved Upload Request; Craft Upload Reply
			PUploadReply r;
			
			DEBUG("request for upload %s %s %u", mUser.c_str(), s.filename.c_str(), s.ticket);
			if ( mUser == mMuseek->username() ) {
				r = PUploadReply(s.ticket, "Cannot Transfer to yourself");

			} else if(mMuseek->is_banned(mUser)) {
				DEBUG("banned");
				if (mMuseek->mBuddiesOnly)
					r = PUploadReply(s.ticket, "Sharing Only to List");
				else
					r = PUploadReply(s.ticket, "Banned");
			} else if(! mMuseek->shares()->is_shared(s.filename) && ! mMuseek->buddyshares()->is_shared(s.filename) ) {
				DEBUG("not shared");
				r = PUploadReply(s.ticket, "File not shared");
			
			} else if(mMuseek->mBuddySharesHave && mMuseek->buddyshares()->is_shared(s.filename) && ! mMuseek->shares()->is_shared(s.filename) && ! mMuseek->is_buddied(mUser)) {
				DEBUG("shared only in buddy shares, but user is not a buddy");
				r = PUploadReply(s.ticket, "File not shared");
 			} else if( ! mMuseek->mBuddySharesHave  && mMuseek->buddyshares()->is_shared(s.filename)  && ! mMuseek->shares()->is_shared(s.filename) ) {
 				DEBUG("shared in buddy shares, but buddy shares not enabled");
 				r = PUploadReply(s.ticket, "File not shared");
			} else {
				DEBUG("shared");
				wstring path = mMuseek->recoder()->decode_network(s.filename);
				Transfer* upload = mMuseek->transfer_manager()->new_upload(mUser, path);
				if(! mPeer->uploading() && mMuseek->transfer_manager()->slot_free())  {
					DEBUG("slot free");
					if(! upload->start_upload(s.ticket)) {
						r = PUploadReply(s.ticket, "Remote file error");
						DEBUG("Local file error");
					} else {
						DEBUG("Initiating transfer");
						r = PUploadReply(s.ticket, upload->size());
					}
				} else {
					DEBUG("queued");
					r = PUploadReply(s.ticket, "Queued");
				}
			}
			send(r);
		} else if (s.direction == 1) {
			// Recieved Download Request; Craft Download Reply
			PDownloadReply r;
			
			DEBUG("request for download %s %s %u", mUser.c_str(), s.filename.c_str(), s.ticket);
			std::wstring fn = mMuseek->recoder()->decode_user(mUser, s.filename);
			std::string reason;
			bool allowed = false;
			if ( mUser == mMuseek->username() ) {	
				reason = "Cannot Transfer to yourself";
				r = PDownloadReply(s.ticket, allowed, reason);
				send(r);
				break;
			}
			Transfer* dl = peer()->download(fn);
			
			if(! dl && mMuseek->is_trusted(mUser))
				dl = mMuseek->transfer_manager()->new_download(mUser, fn, wstring(), wstring(), s.filesize);
			

			
			if(dl) {
				allowed = dl->start_download(s.ticket, s.filesize);
				if(! allowed)
					reason = "Remote file error";
			} else {
				reason = "Cancelled";
			}
			r = PDownloadReply(s.ticket, allowed, reason);
			send(r);
		} 
		
		break;
	}
	case 41: {
		PARSE(PTransferReply);
		Transfer* transfer = mPeer->upload(s.ticket);
		if(! transfer)
			transfer = mPeer->download(s.ticket);
		if(! transfer) {
			DEBUG("couldn't find transfers %u (%s)", s.ticket, mUser.c_str());
			break;
		}
		
		if (transfer->direction() == Transfer::Upload) {
			DEBUG("upload transfer reply %s, %u, %d", mUser.c_str(), s.ticket, s.allowed);
			
			if(transfer->state() != TS_Initiating) {
				DEBUG("weirdness, expected state TS_Initiating, found %i", transfer->state());
				break;
			}
			
			if(! s.allowed) {
				transfer->set_state(TS_Aborted);
			} else {
				transfer->start_upload();
			}
		} else {
			DEBUG("download transfer reply %s %u", mUser.c_str(), s.ticket);
			if(s.allowed) {
				transfer->start_download(s.filesize);
			} else {
				DEBUG("download denied %s, %s", mUser.c_str(), s.reason.c_str());
				transfer->invalidate_ticket();
				if(s.reason == "Queued") {
					transfer->set_state(TS_Queued);
				} else {
					transfer->set_error(s.reason);
				}
			}
		}
		break;
	}
	case 42: {
		PARSE(PUploadPlacehold);
		DEBUG("Upload Placehold %s, %s", mUser.c_str(), s.filename.c_str());
		break;
	}
	case 43: {
		PARSE(PQueueDownload);
		DEBUG("queue upload %s, %s", mUser.c_str(), s.filename.c_str());
		if(mMuseek->is_banned(mUser)) {
			PQueueFailed r(s.filename, "Banned or Sharing Only to List");
			send(r);
		} else if(! mMuseek->shares()->is_shared(s.filename)) {
			PQueueFailed r(s.filename, "File not shared");
			send(r);
		} else {
			mMuseek->transfer_manager()->new_upload(mUser, mMuseek->recoder()->decode_network(s.filename));
		}
		break;
	}
	case 44: {
		PARSE(PPlaceInQueueReply);
		DEBUG("place in queue %s, %s, %i", mUser.c_str(), s.filename.c_str(), s.place);
		mMuseek->transfer_manager()->set_place_in_queue(mUser, mMuseek->recoder()->decode_user(mUser, s.filename), s.place);
		break;
	}
	case 51: {
		PARSE(PPlaceInQueueRequest);
		PPlaceInQueueReply r(s.filename, 0);
		DEBUG("place in queue request %s, %s", mUser.c_str(), s.filename.c_str());
		Transfer *t = mPeer->upload(mMuseek->recoder()->decode_network(s.filename));
		if(t && t->state() == TS_Queued)
			r.place = mMuseek->transfer_manager()->queue_length(mUser, t);
		send(r);
	}
	case 46: {
		PARSE(PUploadFailed);
		DEBUG("got Upload Failed from %s, %s", mUser.c_str(), s.filename.c_str());
		break;
	}
	case 52: {
		PARSE(PUploadQueueNotification);
		DEBUG("got Upload Queue Notification from %s", mUser.c_str() );
		if(! mMuseek->is_trusted(mUser)) {
			mMuseek->cb_peer_upload_blocked(mUser);
			}
		break;
	}
	default:
		BaseConnection::process_message(code);
	}
}

void PeerConnection::shares() {
	CT("shares (%s)", mUser.c_str());
	
	lock();
	
	PSharesRequest r;
	send(r);
}

void PeerConnection::info() {
	CT("info (%s)", mUser.c_str());
	
	lock();
	
	PInfoRequest r;
	send(r);
}

void PeerConnection::folder_contents(const wstring& folder) {
	CT("folder_contents %s (%s)", folder.c_str(), mUser.c_str());
	
	lock();
	
	string _folder = mMuseek->recoder()->encode_user(mUser, folder);
	if(! _folder.empty()) {
		PFolderContentsRequest r(_folder);
		send(r);
	}
}

void PeerConnection::folder_contents(const WStringList& folders) {
	CT("folder_contents %u (%s)", folders.size(), mUser.c_str());
	
	lock();
	
	StringList _folders = mMuseek->recoder()->encode_user_list(mUser, folders);
	if(! _folders.empty()) {
		PFolderContentsRequest r(_folders);
		send(r);
	}
}

void PeerConnection::search(uint32 ticket, const wstring& query) {
	CT("search %u, %s (%s)", ticket, query.c_str(), mUser.c_str());
	
	string _query = mMuseek->recoder()->encode_user(mUser, query);
	if(_query.size() >= 3) {
		PSearchRequest r(ticket, _query);
		send(r);
	}
}

void PeerConnection::results(uint32 ticket, const Folder& results) {
	CT("results %u, %u (%s)", ticket, results.size(), mUser.c_str());
	
	const string& me = mMuseek->username();
	PSearchReply r(ticket, me, results,
	               mMuseek->peer_manager()->get_peer(me)->avgspeed(),
	               mMuseek->transfer_manager()->queue_length(mUser),
	               mMuseek->transfer_manager()->slot_free());
	send(r);
}

void PeerConnection::download(uint32 ticket, const wstring& path) {
	CT("download %s", path.c_str());
	
	string _path = mMuseek->recoder()->encode_user(mUser, path);
	if(! _path.empty()) {
		PTransferRequest r(ticket, _path);
		send(r);
	}
}

void PeerConnection::upload(Transfer *transfer) {
	CT("upload");
	
	string _path = mMuseek->recoder()->encode_network(transfer->path());
	if(! _path.empty()) {
		PTransferRequest r(transfer->ticket(), _path, transfer->size());
		send(r);
	}
}

void PeerConnection::upload_failed(const wstring& path) {
	CT("upload_failed %s", mPeer->manager()->museek()->recoder()->encode_filesystem(path).c_str());
	
	PUploadFailed r(mMuseek->recoder()->encode_network(path));
	send(r);
}

void PeerConnection::queue_failed(const wstring& path, const string& reason) {
	CT("queue_failed %s, %s", mPeer->manager()->museek()->recoder()->encode_filesystem(path).c_str(), reason.c_str());
	
	PQueueFailed r(mMuseek->recoder()->encode_network(path), reason);
	send(r);
}

void PeerConnection::place_in_queue(const wstring& path) {
	CT("place_in_queue %s", mPeer->manager()->museek()->recoder()->encode_filesystem(path).c_str());
	
	PPlaceInQueueRequest r(mMuseek->recoder()->encode_network(path));
	send(r);
}

void PeerConnection::upload_notification() {
	CT("upload_notification %s", mUser.c_str());
	
	PUploadQueueNotification r;
	send(r);
}

void PeerConnection::local_shares_request() {
	CT("local_shares_request %s", mUser.c_str());
	if ( mMuseek->mBuddySharesHave && mMuseek->is_buddied(mUser)) {
		PSharesReply r(mMuseek->buddyshares()->shares());
		send(r);
	} else {
		PSharesReply r(mMuseek->shares()->shares());
		send(r);
	}
}

void PeerConnection::local_userinfo_request() {
	CT("local_userinfo_request %s", mUser.c_str());
	string descr = mMuseek->recoder()->encode_network(mMuseek->userinfo());
	std::string::size_type match = 0;
	while  ((match = descr.find("\n", match)) != string::npos){
		descr.insert(match, "\r");
		match += 2;
		}

	PInfoReply r(descr,
		mMuseek->userpic(),
		mMuseek->upload_slots(),
		mMuseek->transfer_manager()->queue_length(mUser),
		mMuseek->transfer_manager()->slot_free());
	send(r);
}

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

#include <Museek/TransferManager.hh>

#include <Museek/Museek.hh>
#include <Museek/PeerManager.hh>
#include <Museek/ShareBase.hh>
#include <Museek/PeerConnection.hh>
#include <Museek/TransferConnection.hh>
#include <Museek/Recoder.hh>

#define MULOG_DOMAIN "Museek.TM"
#include <Muhelp/Mulog.hh>
#include <Muhelp/string_ext.hh>

#include <string>
#include <sstream>
#include <vector>
#include <iostream>

using std::string;
using std::wstring;
using std::vector;
using std::fstream;
using std::queue;
using std::map;

TransferManager::TransferManager(Museek* museek)
                : mMuseek(museek) {
}

Transfer* TransferManager::new_download(const string& user, const wstring& path, const wstring& localpath, const wstring& temppath, off_t size) {
	CT("new_download %s, %s", user.c_str(), path.c_str());
	
	Peer* peer = mMuseek->peer_manager()->get_peer(user);
	Transfer* transfer = peer->download(path);
	if(! transfer) {
		transfer = new Transfer(this, Transfer::Download, peer, path, localpath, temppath, size);
		mDownloads.push_back(transfer);
	} else {
		transfer->retry();
	}
	
	return transfer;
}

Transfer* TransferManager::send_upload(const string& user, const wstring& path) {
	CT("send_upload %s, %s", user.c_str(), path.c_str());
	
	Peer* peer = mMuseek->peer_manager()->get_peer(user);
	Transfer* transfer = peer->upload(path);
	if(! transfer) {
		transfer = new Transfer(this, Transfer::Upload, peer, path, path);
		mUploads.push_back(transfer);
		check_uploads();
	} else {
		transfer->set_state(TS_Queued);
	}
	
	return transfer;
}

Transfer* TransferManager::new_upload(const string& user, const wstring& path) {
	CT("new_upload %s, %s", user.c_str(), path.c_str());
	Peer* peer = mMuseek->peer_manager()->get_peer(user);
	Transfer* transfer = peer->upload(path);
	if(! transfer) {
// 		DEBUG("new_upload started %s, %s", user.c_str(), path.c_str());
		transfer = new Transfer(this, Transfer::Upload, peer, path);
		mUploads.push_back(transfer);
	} else {
// 		DEBUG("old upload? queued %s, %s", user.c_str(), path.c_str());
		transfer->set_state(TS_Queued); // BUG
	}
	
	return transfer;

}
void TransferManager::remove_transfer(Transfer* transfer) {
	if(transfer->direction() == Transfer::Download)
		mDownloads.erase(find(mDownloads.begin(), mDownloads.end(), transfer));
	else
		mUploads.erase(find(mUploads.begin(), mUploads.end(), transfer));
}

uint32 TransferManager::queue_length(const std::string& user, Transfer *stopAt) const {
	Peer* peer = mMuseek->peer_manager()->get_peer(user);
	bool priv = peer->is_privileged();
	
	vector<Transfer*>::const_iterator it, end = mUploads.end();
	uint32 uploads = 0;
	
	for(it = mUploads.begin(); it != end; ++it) {
		if((*it)->state() == TS_Queued && (! priv || (*it)->peer()->is_privileged()))
			uploads++;
		
		if(*it == stopAt)
			break;
	}
	
	if(stopAt && it == end)
		return 0;
	
	return uploads;
}

bool TransferManager::slot_free() const {
	vector<Transfer*>::const_iterator it = mUploads.begin();
	uint transfers = 0;
	uint32 slots = mMuseek->upload_slots();
	if(! slots)
		return false;
	
	for(; it != mUploads.end(); ++it) {
		switch((*it)->state()) {
		case TS_Negotiating:
		case TS_Establishing:
		case TS_Transferring:
		case TS_Initiating:
			if(++transfers >= slots)
				return false;
		default: ;
		}
	}
	return true;
}

void TransferManager::check_uploads() {
	if(! slot_free())
		return;
	
	Transfer* candidate = 0;
	vector<Transfer*>::const_iterator it = mUploads.begin();
	for(; it != mUploads.end(); ++it) {
		Peer* peer = (*it)->peer();
		if((*it)->state() == TS_Queued && ! peer->uploading()) {
			if(peer->is_privileged()) {
				candidate = *it;
				break;
			}
			if(! candidate)
				candidate = *it;
		}
	}
	if(candidate) {
		candidate->initiate_upload();
		check_uploads();
	}
}

void TransferManager::ban_user(Peer *peer) {
	const vector<Transfer*>* transfers = peer->transfers();
	queue<Transfer*> trash;
	
	vector<Transfer*>::const_iterator it = transfers->begin();
	for(; it != transfers->end(); ++it) {
		if((*it)->direction() == Transfer::Upload && (*it)->state() != TS_Finished)
			trash.push(*it);
	}
	
	PeerConnection *mConnection = peer->connection();
	while(! trash.empty()) {
		Transfer* transfer = trash.front();
		if(transfer->state() == TS_Queued)
			mConnection->queue_failed(transfer->path(), "List Only/Banned");
		delete transfer;
		trash.pop();
	}
}

void TransferManager::add_initiating(Transfer* transfer, BaseConnection* conn) {
	mInitiating[transfer] = conn;
	DEBUG("initiating stack length: %i", mInitiating.size());
}

void TransferManager::cannot_connect(BaseConnection* conn) {
	queue<Transfer*> remove;
	map<Transfer*, BaseConnection*>::iterator it, end = mInitiating.end();
	for(it = mInitiating.begin(); it != end; ++it)
		if((*it).second == conn)
			remove.push((*it).first);
	while(! remove.empty()) {
		mInitiating.erase(remove.front());
		remove.front()->set_state(TS_CannotConnect);
		remove.pop();
	}
	DEBUG("initiating stack length: %i", mInitiating.size());
}

void TransferManager::connected(BaseConnection* conn) {
	queue<Transfer*> remove;
	map<Transfer*, BaseConnection*>::iterator it, end = mInitiating.end();
	for(it = mInitiating.begin(); it != end; ++it)
		if((*it).second == conn)
			remove.push((*it).first);
	while(! remove.empty()) {
		mInitiating.erase(remove.front());
		remove.pop();
	}
	DEBUG("initiating stack length: %i", mInitiating.size());
}

void TransferManager::set_place_in_queue(const string& user, const wstring& path, uint32 place) {
	Transfer *dl = mMuseek->peer_manager()->get_peer(user)->download(path);
	if(dl) {
		dl->set_place_in_queue(place);
		mMuseek->cb_transfer_update(dl);
	}
}

Transfer::Transfer(TransferManager* manager, Direction direction, Peer* peer, const wstring& path, const wstring& local_path, const wstring& temp_path, off_t size)
         : mManager(manager), mDirection(direction), mTicketValid(false), mPeer(peer), mPath(path), mLocalPath(local_path), mTempPath(temp_path),
           mState((direction == Download) ? TS_Offline : TS_Queued), mSize(size), mPos(0), mRate(0), mPlaceInQueue((uint32)-1),
           mConnection(0), mFD(-1), mCollected(0) {
	CT("transfer %s, %s", peer->user().c_str(), path.c_str());
	
	mCollectStart.tv_sec = mCollectStart.tv_usec = 0;
	
	peer->add_transfer(this);
	
	if(direction == Upload) {
		mLocalPath = str_replace(mPath, '\\', '/');
		if(open_upload()) {
			close(mFD);
			mFD = -1;
		}
	}
	
	mManager->museek()->cb_transfer_update(this);
}

Transfer::~Transfer() {
	mManager->museek()->cb_transfer_delete(this);
	
	if(mConnection) {
		terminate();
		mConnection->disconnect();
		mConnection = 0;
	}
	
	mManager->remove_transfer(this);
	mPeer->remove_transfer(this);
}

void Transfer::retry() {
	if(mConnection) {
		mConnection->disconnect();
		mConnection = 0;
	}
	
	mState = TS_Offline;
	mTicketValid = false;
	
	mManager->museek()->cb_transfer_update(this);
	
	mPeer->remove_transfer(this);
	mPeer->add_transfer(this);
}

std::string Transfer::path_utf8() const {
	return mManager->museek()->recoder()->encode_utf8(mPath);
}

void Transfer::set_state(TrState state) {
	if(state == mState)
		return;
	
	switch(state) {
	case TS_Queued:
	case TS_CannotConnect:
	case TS_Aborted:
	case TS_Error:
		if(mConnection) {
			mConnection->disconnect();
			mConnection = 0;
		}
		
		if(mFD != -1) {
			close(mFD);
			mFD = -1;
		}
		
		if(mDirection == Upload) {
			if(mPeer->uploading() == this)
				mPeer->set_uploading(0);
			
			if(state != TS_Queued) {
				mState = state;
				mManager->check_uploads();
			}
		}
		
		break;
	
	case TS_Offline:
		if(mConnection)
			return; // Keep transferring until the connection is closed
	case TS_Finished:
	case TS_ConnectionClosed:
		mConnection = 0;
		
		if(mFD != -1) {
			close(mFD);
			mFD = -1;
			if(mDirection == Download && mPos == mSize) {
				if(! finish_download()) {
					set_error("Local file error");
					break;
				}
				state = TS_Finished;
				DEBUG("transfer speed for %s was %u", mPeer->user().c_str(), mRate);
				mManager->museek()->cb_server_send_user_speed(mPeer->user(), mRate);
			}
		}
		
		if(mPeer->uploading() == this)
			mPeer->set_uploading(0);
	default: ;
	}
	
	mState = state;
	mManager->museek()->cb_transfer_update(this);
}

void Transfer::set_status(uint32 status) {
	CT("set_status %u (%s, %s)", status, mPeer->user().c_str(), mPath.c_str());
	
	switch(mState) {
	case TS_Offline:
		if(mDirection == Download && status > 0) {
			set_state(TS_Connecting);
			mTicket = mManager->museek()->token();
			mTicketValid = true;
			mPeer->push_download(mTicket, mPath);
			break;
		}
		break;
	case TS_Queued:
	case TS_Initiating:
	case TS_Connecting:
		if(status == 0) {
			set_state(TS_Offline);
		}
		break;
	default: ;
	}
}

void Transfer::terminate() {
	if(! mConnection)
		return;
	if(mDirection == Upload)
		static_cast<UploadConnection*>(mConnection)->terminate();
	else
		static_cast<DownloadConnection*>(mConnection)->terminate();
}

bool Transfer::start_download(uint32 ticket, off_t filesize) {
	if(mState != TS_Queued)
		return false;
		
	mTicket = ticket;
	mTicketValid = true;
	mSize = filesize;
	DEBUG("starting download %u %lli %d", ticket, filesize, mState);
	
	if(open_temp_file()) {
		mConnection = new DownloadConnection(this);
		return true;
	}
	
	return false;
}

void Transfer::start_download(off_t filesize) {
	if(mState != TS_Connecting)
		return;
	
	mSize = filesize;
	DEBUG("starting download %u %lli %d", mTicket, filesize, mState);
	
	open_temp_file();
	
	mConnection = new DownloadConnection(this, mManager->museek()->token());
	if(mManager->museek()->connect_mode() == CM_Passive) {
		mConnection->pierce_firewall();
	} else {
		mConnection->init();
		if(! mPeer->have_address()) {
			mConnection->set_state(BS_Address);
			mManager->museek()->server_get_peer_address(peer()->user());
		} else {
			mConnection->connect();
		}
	}
}

bool Transfer::start_upload(uint32 ticket) {
	if(mState != TS_Queued)
		return false;
	
	mTicket = ticket;
	mTicketValid = true;
	DEBUG("starting upload %u", ticket);
	
	if(open_upload()) {
		mPeer->set_uploading(this);
		set_state(TS_Establishing);
		mConnection = new UploadConnection(this);
		mManager->add_initiating(this, mConnection);
		return true;
	}
	
	return false;
}

bool Transfer::initiate_upload() {
	if(mState != TS_Queued)
		return false;
	
	if(! open_upload()) {
		set_error("Local file error");
		return false;
	}
	
	mPeer->set_uploading(this);
	
	set_state(TS_Initiating);
	
	mTicket = mManager->museek()->token();
	mTicketValid = true;
	
	DEBUG("initiating upload sequence %u", mTicket);
	PeerConnection *conn = mPeer->connection();
	mManager->add_initiating(this, conn);
	conn->upload(this);
	
	return true;
}

void Transfer::start_upload() {
	if(mConnection) {
		mConnection->disconnect();
		mConnection = 0;
	}
	
	mConnection = new UploadConnection(this, mManager->museek()->token());
	mManager->add_initiating(this, mConnection);
	if(mManager->museek()->connect_mode() == CM_Passive) {
		mConnection->pierce_firewall();
	} else {
		mConnection->init();
		if(! mPeer->have_address()) {
			mConnection->set_state(BS_Address);
			mManager->museek()->server_get_peer_address(peer()->user());
		} else {
			mConnection->connect();
		}
	}
}


static inline double difftime(struct timeval& a, struct timeval& b) {
	struct timeval diff;
	diff.tv_sec = a.tv_sec - b.tv_sec;
	diff.tv_usec = a.tv_usec - b.tv_usec;
	while(diff.tv_usec < 0)
	{
		diff.tv_sec--;
		diff.tv_usec += 1000000L;
	}
	return diff.tv_sec + (diff.tv_usec / 1000000.0);
}

void Transfer::collect(uint32 bytes) {
	struct timeval now;
	gettimeofday(&now, NULL);
	
	if(mCollectStart.tv_sec == 0)
		mCollectStart = now;
	
	mCollected += bytes;
	double diff = difftime(now, mCollectStart);
	if(diff >= 1.0) {
		mRatePool.push_back((uint32)((double)mCollected / diff));
		while(mRatePool.size() > 10)
			mRatePool.erase(mRatePool.begin());
		
		vector<uint32>::iterator it, end = mRatePool.end();
		for(it = mRatePool.begin(); it != end; ++it)
			mRate += *it;
		mRate /= mRatePool.size();
		
		if(mRate < 0)
			mRate = 0;
		mCollected = 0;
		mCollectStart = now;
		mManager->museek()->cb_transfer_update(this);
	}
}


bool Transfer::make_dirs(const std::string& dir) {
	string part;
	string::const_iterator it = dir.begin();
	for(; it != dir.end(); ++it) {
		if(*it == '/' && ! part.empty()) {
			if(mkdir(part.c_str(), S_IRWXU | S_IRGRP | S_IWGRP | S_IROTH | S_IXOTH) == -1 && errno != EEXIST) {
				DEBUG("WARNING: Couldn't create path %s", part.c_str());
				return false;
			}
		}
		part += *it;
	}
	
	return true;
}

bool Transfer::open_temp_file() {
	std::wstringstream temp;
	temp << mManager->temp_dir() << "incomplete." << mSize << ".";
	
	wstring::size_type ix = mPath.rfind('\\');
	ix = (ix == wstring::npos) ? 0 : (ix + 1);
	mFilename = mPath.substr(ix);
	temp << mFilename;
	
	mTempPath = temp.str();
	
	if(mLocalPath.empty())
		mLocalPath = mManager->download_dir() + mFilename;
	
	string fn = mManager->museek()->recoder()->encode_filesystem(mTempPath);
	if(fn.empty() || ! make_dirs(fn))
		return false;
	
	mFD = open(fn.c_str(), O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if(mFD == -1) {
		perror(NULL);
		return false;
	}
	
	mPos = lseek(mFD, 0, SEEK_END);
	
	DEBUG("opened temp file %s (pos: %u)", fn.c_str(), mPos);
	
	return true;
}

bool Transfer::write(queue<unsigned char>& buffer) {
	if(mFD == -1)
		return false;
	
	set_state(TS_Transferring);
	
	collect(buffer.size());
	
	if(buffer.size()) {
		unsigned char* buf = new unsigned char[buffer.size()];
		ssize_t i = 0, written;
		while(! buffer.empty()) {
			buf[i++] = buffer.front();
			buffer.pop();
		}
		
		written = ::write(mFD, buf, i);
		
		delete [] buf;
		
		if(written > 0)
			mPos += written;
		
		if(written != i)
			return false;
	}
	
	return true;
}

bool Transfer::finish_download() {
start:
	std::string src = mManager->museek()->recoder()->encode_filesystem(mTempPath),
	           dest = mManager->museek()->recoder()->encode_filesystem(mLocalPath);
	
	make_dirs(dest);
	
	struct stat s;
	int ret = stat(dest.c_str(), &s);
	if(ret == -1) {
		switch(errno) {
		case ENOTDIR:
		case ELOOP:
		case EFAULT:
		case EACCES:
			DEBUG("Invalid path: %s", dest.c_str());
			perror(NULL);
			mLocalPath = mManager->temp_dir() + mFilename;
			goto start;
		default: ;
		}
	} else {
		int i = 1;
		while(1) {
			std::string ndest = dest + "." + itos(i);
			DEBUG("file exists, trying %s", ndest.c_str());
			ret = stat(ndest.c_str(), &s);
			if(ret == -1 && errno == ENOENT) {
				mLocalPath = mManager->museek()->recoder()->decode_filesystem(ndest);
				goto start;
			}
			++i;
		}
	}
	
	if(rename(src.c_str(), dest.c_str()) == -1) {
		if(errno == EXDEV) {
			DEBUG("having incomplete and downloaddir on separate filesystems is a bad idea");
			
			fstream fin, fout;
			fin.open(src.c_str(), fstream::in | fstream::binary);
			if(! fin.is_open()) {
				DEBUG("couldn't open temporary file for reading, bailing out");
				return false;
			}
			
			fout.open(dest.c_str(), fstream::out | fstream::binary);
			if(! fout.is_open()) {
				DEBUG("couldn't open destination file for writing, bailing out");
				fin.close();
				return false;
			}
			
			char buffer[1024 * 1024];
			bool ok = true;
			uint count;
			do {
				count = fin.readsome(buffer, 1024*1024);
				if(fin.fail()) {
					DEBUG("error while reading, bailing out");
					ok = false;
					break;
				}
				if(count) {
					fout.write(buffer, count);
					if(fout.fail()) {
						DEBUG("error while writing, bailing out");
						ok = false;
						break;
					}
				}
			} while(count > 0);
			fin.close();
			fout.close();
			
			if(ok) {
				if(remove(src.c_str()) == -1) {
					DEBUG("couldn't delete temporary file");
					perror(NULL);
				}
				DEBUG("finished transfer (saved to %s)", dest.c_str());
				return true;
			} else {
				DEBUG("couldn't copy temporary file to download dir");
				remove(dest.c_str());
				return false;
			}
		} else {
			DEBUG("RENAMING FAILED!");
			perror(NULL);
			return false;
		}
	} else {
		DEBUG("finished transfer (saved to %s)", dest.c_str());
	}
	
	return true;
}	

bool Transfer::open_upload() {
	mFD = ::open(mManager->museek()->recoder()->encode_filesystem(mLocalPath).c_str(), O_RDONLY);
	if(mFD == -1)
		return false;
	
	mSize = ::lseek(mFD, 0, SEEK_END);
	
	return true;
}

bool Transfer::seek(off_t pos) {
	DEBUG("seeking to %u", pos);
	set_state(TS_Transferring);
	
	if(lseek(mFD, pos, SEEK_SET) == (off_t)-1)
		return false;
		
	mPos = pos;
	
	return true; 
	
}

bool Transfer::read(queue<unsigned char>& buffer) {
	unsigned char buf[1024 * 1024];
	
	off_t count = ::read(mFD, buf, 1024 * 1024);
	if(count == -1)
		return false;
	
	for(off_t i = 0; i < count; ++i)
		buffer.push(buf[i]);
	
	return true;
}

void Transfer::sent(uint count) {
	mPos += count;
	collect(count);
}

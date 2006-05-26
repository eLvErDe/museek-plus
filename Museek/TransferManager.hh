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

#ifndef __TRANSFERMANAGER_HH__
#define __TRANSFERMANAGER_HH__

#include <string>
#include <vector>
#include <fstream>
#include <queue>

class TransferManager;
class Museek;
class Peer;
class BaseConnection;

class Transfer {
public:
	enum Direction {
		Upload,
		Download
	};
	
	Transfer(TransferManager* manager, Direction dir, Peer* peer, const std::wstring& path,
	         const std::wstring& local_path = std::wstring(), const std::wstring& temp_path = std::wstring(), off_t size = 0);
	~Transfer();
	
	inline TransferManager* manager() const { return mManager; }
	inline Direction direction() const { return mDirection; }
	inline Peer* peer() const { return mPeer; }
	
	void retry();
	
	inline bool ticket_valid() const { return mTicketValid; };
	inline uint32 ticket() const { return mTicket; }
	inline void invalidate_ticket() { mTicketValid = false; };
	
	inline const std::wstring& path() const { return mPath; }
	std::string path_utf8() const;
	inline const std::wstring& local_path() const { return mLocalPath; }
	inline const std::wstring& temp_path() const { return mTempPath; }
	
	inline TrState state() const { return mState; }
	void set_state(TrState state);
	
	inline std::string error() const { return (mState == TS_Error) ? mError : std::string(); }
	inline void set_error(const std::string& error) { mError = error; set_state(TS_Error); }
	
	inline off_t size() const { return mSize; }
	inline void set_size(off_t size) { mSize = size; }
	
	inline off_t pos() const { return mPos; }
	inline void set_pos(off_t pos) { mPos = pos; }
	
	inline uint32 rate() const { return mRate; }
	inline void set_rate(uint32 rate) { mRate = rate; }
	
	inline uint32 place_in_queue() const { return mPlaceInQueue; }
	inline void set_place_in_queue(uint32 place) { mPlaceInQueue = place; }
	
	void set_status(uint32 status);
	
	inline BaseConnection* connection() const { return mConnection; }
	void terminate();
	
	bool start_download(uint32 ticket, off_t filesize);
	void start_download(off_t filesize);
	
	bool start_upload(uint32 ticket);
	bool initiate_upload();
	void start_upload();
	
	void collect(uint32 count);
	
	/* for downloads */
	bool make_dirs(const std::string& path);
	bool open_temp_file();
	bool write(std::queue<unsigned char>& buffer);
	bool finish_download();
	
	/* for uploads */
	bool open_upload();
	bool seek(off_t pos);
	bool read(std::queue<unsigned char>& buffer);
	void sent(uint count);
		
private:
	TransferManager* mManager;
	Direction mDirection;
	bool mTicketValid;
	uint32 mTicket;
	Peer* mPeer;
	
	std::wstring mPath, mFilename, mLocalPath, mTempPath;
	
	TrState mState;
	std::string mError;
	off_t mSize, mPos;
	uint32 mRate, mPlaceInQueue;
	std::vector<uint32> mRatePool;
	
	BaseConnection* mConnection;
	
	int mFD;
	
	uint32 mCollected;
	struct timeval mCollectStart;
};


class TransferManager {
public:
	TransferManager(Museek* museek);
	
	inline Museek* museek() const { return mMuseek; }
	
	inline const std::wstring& temp_dir() const { return mTempDir; }
	inline void set_temp_dir(std::wstring temp_dir) { mTempDir = temp_dir; }
	
	inline const std::wstring& download_dir() const { return mDownloadDir; }
	inline void set_download_dir(std::wstring download_dir) { mDownloadDir = download_dir; }
	
	inline const std::vector<Transfer*>& uploads() const { return mUploads; }
	inline const std::vector<Transfer*>& downloads() const { return mDownloads; }
	
	Transfer* new_download(const std::string& user, const std::wstring& path, const std::wstring& localpath, const std::wstring& temppath, off_t size);
	Transfer* new_upload(const std::string& user, const std::wstring& path);
	Transfer* send_upload(const std::string& user, const std::wstring& path);
	
	void add_initiating(Transfer* transfer, BaseConnection* conn);
	void cannot_connect(BaseConnection* conn);
	void connected(BaseConnection* conn);
	void remove_transfer(Transfer* transfer);
	
	uint32 queue_length(const std::string& user, Transfer* stopAt = 0) const;
	bool slot_free() const;
	void check_uploads();
	
	void set_place_in_queue(const std::string&, const std::wstring& path, uint32 place);
	
	/* Use this to just cancel someones transfers */
	void ban_user(Peer *peer);
	
private:
	Museek* mMuseek;
	std::vector<Transfer*> mUploads, mDownloads;
	
	std::wstring mTempDir, mDownloadDir;
	
	std::map<Transfer*, BaseConnection*> mInitiating;
};

#endif // __TRANSFERMANAGER_HH__

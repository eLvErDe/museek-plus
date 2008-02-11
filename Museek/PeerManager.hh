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

#ifndef __PEERMANAGER_HH__
#define __PEERMANAGER_HH__

#include <string>
#include <map>
#include <queue>

class Museek;
class PeerManager;
class PeerConnection;
class Transfer;

class Peer {
public:
	Peer(PeerManager* manager, const std::string& user);
	~Peer();
	
	void server_connected();
	void server_disconnected();
	
	void inc_ref();
	void dec_ref(void* connection = 0);
	inline uint32 ref_count() const { return mRefCount; }
	
	void add_transfer(Transfer* transfer);
	void remove_transfer(Transfer* transfer);
	
	inline PeerManager* manager() const { return mManager; }
	
	inline std::string user() const { return mUser; }
	
	void set_exists(bool exists);
	void set_status(uint32 status);
	void set_address(const std::string& ip, uint32 port);
	void set_stats(uint32 avgspeed, uint32 downloadnum, uint32 files, uint32 dirs);
	
	void subscribe();
	inline bool subscribed() const { return mSubscribed; }
	inline bool have_exists() const { return mHaveExists; }
	inline bool exists() const { return mExists; }
	
	inline bool have_status() const { return mHaveStatus; }
	inline uint32 status() const { return mStatus; }
	
	inline bool have_address() const { return mHaveAddress; }
	inline std::string ip() const { return mIP; }
	inline uint32 port() const { return mPort; }
	
	inline bool have_stats() const { return mHaveStats; }
	inline uint32 avgspeed() const { return mAvgSpeed; }
	inline uint32 downloadnum() const { return mDownloadNum; }
	inline uint32 files() const { return mFiles; }
	inline uint32 dirs() const { return mDirs; }
	
	inline bool is_privileged() const { return mPrivileged; }
	inline void set_privileged(bool p) { mPrivileged = p; }
	inline bool is_trusted() const { return mAllowUploads; }
	inline void set_trusted(bool p) { mAllowUploads= p; }
	
	inline const std::vector<Transfer*>* transfers() const { return &mTransfers; }
	
	PeerConnection* connection();
	void set_socket(int sock, uint32 token, const std::queue<unsigned char>& data);
	void request_connect(const std::string& type, uint32 token);
	void cannot_connect(uint32 token);
	bool pierced_firewall(int sock, uint32 token);
	
	Transfer* download(const std::wstring& path);
	Transfer* download(uint32 ticket);
	void abort_download(const std::wstring& path);
	
	Transfer* upload(const std::wstring& path);
	Transfer* upload(uint32 ticket);
	void abort_upload(const std::wstring& path);
	
	inline Transfer* uploading() const { return mUploading; }
	inline void set_uploading(Transfer* uploading) { mUploading = uploading; }
	
	void push_download(uint path1, const std::wstring& path);
	void flush_downloads();
	
private:
	PeerManager* mManager;
	std::vector<PeerConnection*> mConnections;
	
	uint32 mRefCount;
	
	std::string mUser;
	bool mSubscribed, mHaveExists, mHaveStatus, mHaveAddress, mHaveStats, mPrivileged, mAllowUploads;
	
	bool mExists;
	std::string mIP;
	uint32 mPort,
	     mStatus,
	     mAvgSpeed,
	     mDownloadNum,
	     mFiles,
	     mDirs;
	
	std::vector<Transfer*> mTransfers;
	std::vector<std::pair<uint, std::wstring> > mDownloadQueue;
	Transfer* mUploading;
	bool mReceiving;
};

class PeerManager {
public:
	PeerManager(Museek*);
	
	inline Museek* museek() const { return mMuseek; }
	
	void server_connected();
	void server_disconnected();
	
	Peer* get_peer(const std::string& user, bool can_make = true);
	inline std::map<std::string, Peer*>& peers() { return mPeers; }
	void remove_peer(Peer* peer);
	void purge();
	
	void set_peer_exists(const std::string& user, bool exists);
	void set_peer_status(const std::string& user, uint32 status);
	void set_peer_address(const std::string& user, const std::string& ip, uint32 port);
	void set_peer_stats(const std::string& user, uint32 avgspeed, uint32 downloadnum, uint32 files, uint32 dirs);
	void request_peer_connect(const std::string& user, const std::string& type, const std::string& ip, uint32 port, uint32 token);
	void peer_cannot_connect(const std::string& user, uint32 token);
	bool pierced_firewall(int sock, uint32 token);
	
private:
	PeerManager* mManager;
	Museek* mMuseek;
	std::map<std::string, Peer*> mPeers;
};

#endif // __PEERMANAGER_HH__

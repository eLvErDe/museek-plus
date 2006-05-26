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

#ifndef __PEERCONNECTION_HH__
#define __PEERCONNECTION_HH__

#include <Museek/BaseConnection.hh>

#include <string>

class Museek;
class Transfer;

class PeerConnection : public BaseConnection {
public:
	PeerConnection(Peer* peer, uint32 token);
	
	void lock() { mLocked = true; }
	bool locked() const { return mLocked; }
	
	void shares();
	void info();
	void upload_notification();
	void folder_contents(const std::wstring& folder);
	void folder_contents(const WStringList& folders);
	void search(uint32 ticket, const std::wstring& query);
	void results(uint32 ticket, const Folder& results);
	void download(uint32 ticket, const std::wstring& path);
	void upload(Transfer *transfer);
	void upload_failed(const std::wstring& path);
	void queue_failed(const std::wstring& path, const std::string& reason);
	void place_in_queue(const std::wstring& path);
	
protected:
	void process_message(uint32 code);
	
private:
	bool mLocked;
	
	std::string mUser;
	Museek* mMuseek;
};

#endif // __PEERCONNECTION_HH__

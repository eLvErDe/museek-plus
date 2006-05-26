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

#ifndef __BASECONNECTION_HH__
#define __BASECONNECTION_HH__

#include <Museek/MessageConnection.hh>
#include <string>

class Peer;

class BaseConnection : public MessageConnection {
public:
	BaseConnection(int codesize, std::string type, Peer* peer, uint32 token);
	~BaseConnection();
	
	inline BaseConnState state() const { return mState; }
	void set_state(BaseConnState state) { mState = state; }
	
	void init();
	void connect();
	void pierce_firewall();
	void request_connect();
	void set_socket(int sock, const std::queue<unsigned char>& data);
	void pierced_firewall(int sock);
	
	inline Peer* peer() const { return mPeer; }
	inline uint32 token() const { return mToken; }
	
protected:
	virtual void connected();
	virtual void disconnected();
	virtual void cannot_connect();
	int get_poll_mask();
	void return_poll_mask(int);
	
protected:
	Peer* mPeer;
	bool mPiercing; // Are we the result of somebody asking us to connect
	
private:
	std::string mType; // Connection type "P", "T" or "D"
	uint32 mToken;
	BaseConnState mState;
	time_t mLastTraffic;
};

#endif // __BASECONNECTION_HH__

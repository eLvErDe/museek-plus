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

#include <Museek/BaseConnection.hh>
#include <Museek/PeerManager.hh>
#include <Museek/InitMessages.hh>
#include <Museek/Museek.hh>

#define MULOG_DOMAIN "Museek.BC"
#include <Muhelp/Mulog.hh>

#include <string>
#include <queue>

using std::string;
using std::queue;

BaseConnection::BaseConnection(int codesize, string type, Peer* peer, uint32 token) 
               : MessageConnection(codesize),
                 mPeer(peer),
                 mPiercing(false),
                 mType(type),
                 mToken(token),
                 mState(BS_Unknown) {
	CT("BaseConnection %i, %s, %s, %u", codesize, type.c_str(), peer->user().c_str(), token);
	
	timeout = 60;
	mLastTraffic = time(0);
	
	mPeer->manager()->museek()->add(this, false);
	mPeer->inc_ref();
}

BaseConnection::~BaseConnection() {
	CT("~BaseConnection %s", mPeer->user().c_str());
	
	mPeer->dec_ref(this);
}

void BaseConnection::init() {
	CT("init (%s)", mPeer->user().c_str());
	
	PInit i(mPeer->manager()->museek()->username(), mType, mToken);
	send(i);
}

void BaseConnection::connect() {
	DEBUG("connect (%s)", mPeer->user().c_str());
	
	mState = BS_Connecting;
	MessageConnection::connect(mPeer->ip(), mPeer->port(), 60);
	mPeer->manager()->museek()->subscribe(this);
}

void BaseConnection::connected() {
	DEBUG("connected (%s)", mPeer->user().c_str());
	
	mState = BS_Connected;
	mPiercing = false;
	mPeer->manager()->museek()->connected(this, mPeer->user());
}

void BaseConnection::disconnected() {
	CT("disconnected (%s)", mPeer->user().c_str());
	
	mState = BS_Disconnected;
}

void BaseConnection::cannot_connect() {
	CT("cannot_connect (%s)", mPeer->user().c_str());
	
	mState = BS_Error;
	if(mPiercing)
		mPeer->manager()->museek()->server_peer_cannot_connect(mToken, mPeer->user());
	mPeer->manager()->museek()->cannot_connect(this, mPeer->user());
}

void BaseConnection::pierce_firewall() {
	CT("pierce_firewall (%s)", mPeer->user().c_str());
	
	mState = BS_Waiting;
	mPeer->manager()->museek()->server_request_peer_connect(mToken, mPeer->user(), mType);
}

void BaseConnection::request_connect() {
	CT("request_connect (%s)", mPeer->user().c_str());
	
	mState = BS_Connecting;
	mPiercing = true;
	
	MessageConnection::connect(mPeer->ip(), mPeer->port(), 60);
	mPeer->manager()->museek()->subscribe(this);
	
	PPierceFW i(mToken);
	send(i);
}

void BaseConnection::set_socket(int sock, const queue<unsigned char>& data) {
	CT("set_socket %u (%s)", sock, mPeer->user().c_str());
	
	mState = BS_Connected;
	
	MessageConnection::set_socket(sock);
	
	if(sock != -1) {
		inbuf = data;
		
		mPeer->manager()->museek()->subscribe(this);
	
		if(inbuf.size())
			process();
	}
}

void BaseConnection::pierced_firewall(int sock) {
	CT("pierced_firewall %u (%s)", sock, mPeer->user().c_str());
	
	MessageConnection::set_socket(sock);
	
	if(sock != -1)
		mPeer->manager()->museek()->subscribe(this);
}

int BaseConnection::get_poll_mask() {
	CT("get_poll_mask");
	
	time_t curtime = time(0);
	if(ready && curtime >= mLastTraffic + 60) {
		DEBUG("Connection timed out.. bye bye %s %i %i", mPeer->user().c_str(), curtime, mLastTraffic);
		disconnect();
		error = ERR_DISCONNECTED;
		return 0;
	}
	
	return MessageConnection::get_poll_mask();
}

void BaseConnection::return_poll_mask(int mask) {
	CT("return_poll_mask %i", mask);
	
	mLastTraffic = time(0);
	next_event = mLastTraffic + 60;
	
	MessageConnection::return_poll_mask(mask);
}

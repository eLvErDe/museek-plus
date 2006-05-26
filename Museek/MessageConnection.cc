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

#include <Museek/MessageConnection.hh>
#include <Museek/Messages.hh>

#include <queue>
#include <string>

#define MULOG_DOMAIN "Museek.MC"
#include <Muhelp/Mulog.hh>

void MessageConnection::send(NetworkMessage& msg) {
	CT("send");
	
	std::queue<unsigned char>& buffer = msg.make_network_packet();
	ClientConnection::send((uint32)buffer.size());
	ClientConnection::send(buffer);
}

void MessageConnection::process() {
	CT("process");
	
	if (! mMsgLen) {
		if (inbuf.size() < 4)
			return;
		for(uint i=0; i < 4; ++i) {
			mMsgLen += inbuf.front() << (i * 8);
			inbuf.pop();
		}
	}

	if (mMsgLen < mCodeSize) {
		DEBUG("malformed package size %u\n", mMsgLen);
		disconnect();
		return;
	}

	if (inbuf.size() < mMsgLen)
		return;

	mMsgCode = 0;
	for(uint i=0; i < (uint)mCodeSize; i++) {
		mMsgCode += inbuf.front() << (i * 8);
		inbuf.pop();
	}
	mMsgLen -= mCodeSize;
	
	for(uint i = 0; i < mMsgLen; i++) {
		message.push(inbuf.front());
		inbuf.pop();
	}

	process_message(mMsgCode);
	while(! message.empty())
		message.pop();
	mMsgLen = 0;
	process();
}

void MessageConnection::process_message(uint32 code) {
	CT("process message");
	
	DEBUG("unknown message code %u, %u", code, mMsgLen);
}

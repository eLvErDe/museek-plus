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

#ifndef __MESSAGECONNECTION_HH__
#define __MESSAGECONNECTION_HH__

#include <Museekal/ClientConnection.hh>

#include <queue>
#include <string>

class NetworkMessage;

class MessageConnection : public ClientConnection {
public:
	MessageConnection(uint codesize)
		: ClientConnection(), mMsgLen(0), mMsgCode(0), mCodeSize(codesize)
		{ can_receive = true; };
	
protected:
	void send(NetworkMessage&);
	virtual void process();
	virtual void process_message(uint32 mMsgCode);
	
	std::queue<unsigned char> message;

private:
	uint32 mMsgLen, mMsgCode;
	uint mCodeSize;
};

#endif // __MESSAGECONNECTION_HH__

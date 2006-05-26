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

#include <Museek/PeerListener.hh>
#include <Museek/Museek.hh>
#include <Museek/MessageConnection.hh>
#include <Museek/InitMessages.hh>

#include <string>
using std::string;

#define MULOG_DOMAIN "Museek.PR"
#include <Muhelp/Mulog.hh>

class PreConnection : MessageConnection {
public:
	PreConnection(PeerListener* listener, int sock)
	             : MessageConnection(1), mListener(listener) {
		CT("PreConnection %u", sock);
		
		mLastRX = time(NULL);
		set_socket(sock);
		mListener->museek()->add(this);
	}
	
private:
	PeerListener* mListener;
	time_t mLastRX;
	
	int get_poll_mask() {
		if(mLastRX + 60 <= time(NULL)) {
			DEBUG("timeout");
			disconnect();
			return 0;
		}
		next_event = mLastRX + 60;
		return MessageConnection::get_poll_mask();
	}

	void process() {
		mLastRX = time(NULL);
		MessageConnection::process();
	}
	
	void process_message(uint32 code) {
		CT("process_message %u", code);
		
		switch(code) {
		case 0:
		    {
			PPierceFW m;
			m.parse_network_packet(message);
			DEBUG("got pierce firewall init %u, %u (%d left in buffer)", sock, m.token, inbuf.size());
			if(! mListener->museek()->cb_listen_pierce_firewall(sock, m.token)) {
				disconnect();
				return;
			}
			break;
		    }
		case 1:
		    {
			PInit m;
			m.parse_network_packet(message);
			DEBUG("got peer init message %u, %s, %s, %u, <...> (%d)", sock, m.user.c_str(), m.type.c_str(), m.token, inbuf.size());
			mListener->museek()->cb_listen_init(sock, m.user, m.type, m.token, inbuf);
			break;
		    }
		default:
			DEBUG("Unknown peer init code %u", code);
			disconnect();
			return;
		}
		
		is_disconnected = true;
		sock = -1;
		error = ERR_DISCONNECTED;
		
		while(! inbuf.empty())
			inbuf.pop();
	}
	
};

#undef MULOG_DOMAIN
#define MULOG_DOMAIN "Museek.PL"

PeerListener::PeerListener(Museek* museek)
             : ListenConnection(), mMuseek(museek) {
	CT("PeerListener");
	
}

void PeerListener::listen(const string& host, uint port) {
	CT("listen %s, %u", host.c_str(), port);
	
	ListenConnection::listen(host, port);
	if(error == ERR_NONE)
		mMuseek->add(this);
}

bool PeerListener::accept() {
	CT("accept");
	
	int sock = simple_accept();
	if(sock == -1)
		return false;
	
	new PreConnection(this, sock);
	
	return true;
}

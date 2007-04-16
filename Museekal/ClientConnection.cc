/* Museekal - Museek's Network Abstraction Layer
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

#include <system.h>

#include <Museekal/ClientConnection.hh>
#include <Museekal/TrafficTracker.hh>

#define MULOG_DOMAIN "Museekal.CC"
#include <Muhelp/Mulog.hh>

void ClientConnection::connect(const std::string& host, uint port, time_t _timeout) {
	CT("connect %s, %u, %i", host.c_str(), port, _timeout);

	timeout = _timeout;
	ready = false;
	
	sock = socket(PF_INET, SOCK_STREAM, 0);
	fcntl(sock, F_SETFL, O_NONBLOCK);

	DEBUG("resolving %s", host.c_str());
	struct hostent *h = gethostbyname(host.c_str());
	if(!h) {
		DEBUG("cannot resolve %s", host.c_str());
		error = ERR_LOOKUP_FAILED;
		cannot_resolve();
		return;
	}
	DEBUG("resolved host %s", host.c_str());

	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	memcpy(&(address.sin_addr.s_addr), *(h->h_addr_list), sizeof(address.sin_addr.s_addr));
	address.sin_port = htons(port);
	
	DEBUG("connecting to %s", host.c_str());
	if (::connect(sock, (struct sockaddr *)&address, sizeof(struct sockaddr_in)) == 0) {
		ready = true;
		connected();
	} else if (errno != EINPROGRESS) {
		DEBUG("cannot connect to %s (errno: %i)", host.c_str(), errno);
		error = ERR_CANNOT_CONNECT;
		cannot_connect();
	}
	start = time(NULL);
}

void ClientConnection::connect_unix(const std::string& path) {
	CT("connect_unix %s", path.c_str());
#ifdef HAVE_SYS_UN_H	
	timeout = 60;
	
	if(path.length() >= UNIX_PATH_MAX) {
		DEBUG("unix path too long %s", path.length());
		cannot_resolve();
		error = ERR_LOOKUP_FAILED;
		return;
	}
	
	sock = socket(PF_UNIX, SOCK_STREAM, 0);
	
	struct sockaddr_un address;
	memset(&address, 0, sizeof(address));
	address.sun_family = AF_UNIX;
	memcpy(address.sun_path, path.c_str(), path.length()+1);
	
	DEBUG("connecting to %s", path.c_str());
	if (::connect(sock, (struct sockaddr *)&address, sizeof(struct sockaddr_un)) == 0) {
		fcntl(sock, F_SETFL, O_NONBLOCK);
		
		ready = true;
		connected();
	} else if (errno != EINPROGRESS) {
		DEBUG("cannot connect to %s (errno: %i)", path.c_str(), errno);
		error = ERR_CANNOT_CONNECT;
		cannot_connect();
	}
#else
	DEBUG("unix sockets not available, cannot connect to %s", path.c_str());
	error = ERR_CANNOT_CONNECT;
	cannot_connect();
#endif
}

void ClientConnection::cannot_resolve() {
	CT("cannot_resolve");
}

void ClientConnection::get_ready(int mask) {
	CT("get_ready %08x", mask);

	if (is_disconnected || error != ERR_NONE)
		return;

	if (mask & MASKERR) {
		DEBUG("cannot connect to host");
		error = ERR_CANNOT_CONNECT;
		cannot_connect();
	} else if (mask & MASKOUT) {
		DEBUG("connected to host");
		ready = true;
		next_event = 0;
		connected();
	}
	return;
}

void ClientConnection::send(const unsigned char *data, size_t len) {
	CT("send '...' (%d)", len);

	for(size_t i = 0; i < len; i++)
		outbuf.push(data[i]);
}

void ClientConnection::send(uint32 i) {
	CT("send %u", i);

	for(uint j = 0; j < 4; j++) {
		outbuf.push((unsigned char)(i & 0xff));
		i = i >> 8;
	}
}

void ClientConnection::send(off_t i) {
	CT("send %lliL", (long long)i);

	for(uint j = 0; j < 8; j++) {
		outbuf.push((unsigned char)(i & 0xff));
		i = i >> 8;
	}
}

void ClientConnection::send(std::queue<unsigned char>& _data) {
	CT("send <...> (%u)", _data.size());
	std::queue<unsigned char>& data = _data;
	
	while(! data.empty()) {
		outbuf.push(data.front());
		data.pop();
	}
}

/*void ClientConnection::send(std::queue<unsigned char> data) {
	while(! data.empty()) {
		outbuf.push(data.front());
		data.pop();
	}
}*/

int ClientConnection::get_poll_mask() {
	CT("get_poll_mask");
	
	if (is_disconnected || error != ERR_NONE)
		return 0;
	
	if (! ready) {
		if (timeout > 0) {
			if (time(NULL) >= start + timeout) {
				DEBUG("timeout connecting to host");
				error = ERR_TIMEOUT;
				connection_timeout();
				return 0;
			} else
				next_event = start + timeout;
		}
		return (sock == -1) ? 0 : MASKOUT;
	}
	
	if(sock == -1)
		return 0;
	
	return (outbuf.empty() ? 0 : MASKOUT) |
	       (can_receive ? MASKIN : 0);
}

void ClientConnection::return_poll_mask(int mask) {
	CT("return_poll_mask %08x", mask);

	if (! ready) {
		get_ready(mask);
		return;
	}

	if (mask & MASKERR) {
		disconnect();
		return;
	}

	if (!outbuf.empty() && (mask & MASKOUT)) {
		char *data = new char[outbuf.size()];
		int i = 0, j;

		while (! outbuf.empty()) {
			data[i++] = outbuf.front();
			outbuf.pop();
		}

		j = ::send(sock, data, i, 0);

		errno = 0;
		if (j < 0) {
			if (errno != EAGAIN)
				disconnect();
		}
		else {
			for(int x = j; x < i; x++)
				outbuf.push(data[x]);
			TrafficTracker::instance()->collect(0, j);
			data_sent(j);
		}
		delete [] data;
	}

	if (mask & MASKIN) {
		char data[16384];
		int i = 1, read = 0, _errno = EAGAIN;

		while(i > 0) {
			i = recv(sock, data, 16384, 0);
			if (i > 0) {
				TrafficTracker::instance()->collect(1, i);
				for(int j = 0; j < i; j++)
					inbuf.push(data[j]);
				read += i;
			}
		}
		if (i < 0)
			_errno = errno;

		if (! inbuf.empty())
			process();

		if ((i == 0) || (_errno != EAGAIN))
			disconnect();
	}
}

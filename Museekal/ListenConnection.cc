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

#include <Museekal/ListenConnection.hh>

#define MULOG_DOMAIN "Museekal.LC"
#include <Muhelp/Mulog.hh>

ListenConnection::ListenConnection() : CoreConnection(), mDomain(PF_UNSPEC) { 
	CT("ListenConnection");
};

void ListenConnection::listen(const std::string& local, uint port) {
	CT("listen %s, %u", local.c_str(), port);
	
	mDomain = PF_INET;
	
	sock = socket(PF_INET, SOCK_STREAM, 0);
	fcntl(sock, F_SETFL, O_NONBLOCK);

	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));

	address.sin_family = AF_INET;
	address.sin_port = htons(port);

	if (local != "") {
		DEBUG("resolving up %s", local.c_str());
		struct hostent *h = gethostbyname(local.c_str());
		if (! h) {
			DEBUG("cannot resolve %s", local.c_str());
			error = ERR_CANNOT_CONNECT;
			close(sock);
			return;
		}
		DEBUG("resolved host %s", local.c_str());
		memcpy(&(address.sin_addr.s_addr), *(h->h_addr_list), sizeof(address.sin_addr.s_addr));
	} else
		address.sin_addr.s_addr = INADDR_ANY;
	
	int socket_option = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &socket_option, sizeof(int));
	
	DEBUG("binding to %s:%u", local.c_str(), port);
	if (bind(sock, (struct sockaddr *)&address, sizeof(struct sockaddr_in)) != 0) {
		DEBUG("cannot bind to %s:%u", local.c_str(), port);
		error = ERR_CANNOT_CONNECT;
		close(sock);
		return;
	}
	DEBUG("bound to %s:%u", local.c_str(), port);
	
	DEBUG("starting to listen on %s:%u", local.c_str(), port);
	
	if (::listen(sock, 3) != 0) {
		DEBUG("cannot listen on %s:%u", local.c_str(), port);
		error = ERR_CANNOT_CONNECT;
		close(sock);
		return;
	}
	DEBUG("listening on %s:%u", local.c_str(), port);
}

void ListenConnection::listen_unix(const std::string& path) {
	CT("listen_unix %s", path.c_str());

#ifdef HAVE_SYS_UN_H	
	mDomain = PF_UNIX;
	
	if(path.length() >= UNIX_PATH_MAX) {
		DEBUG("unix path too long %s", path.length());
		error = ERR_CANNOT_CONNECT;
		return;
	}
	
	unlink(path.c_str());
	
	mode_t old_mode = umask(0177);
	
	sock = socket(PF_UNIX, SOCK_STREAM, 0);
	fcntl(sock, F_SETFL, O_NONBLOCK);
	
	struct sockaddr_un address;
	memset(&address, 0, sizeof(address));

	address.sun_family = AF_UNIX;
	memcpy(address.sun_path, path.c_str(), path.length());

	DEBUG("binding to %s", path.c_str());
	if (bind(sock, (struct sockaddr *)&address, sizeof(struct sockaddr_un)) != 0) {
		umask(old_mode);
		DEBUG("cannot bind to %s", path.c_str());
		error = ERR_CANNOT_CONNECT;
		close(sock);
		return;
	}
	DEBUG("bound to %s", path.c_str());
	
	DEBUG("starting to listen on %s", path.c_str());
	
	if (::listen(sock, 3) != 0) {
		umask(old_mode);
		DEBUG("cannot listen on %s:%u", path.c_str());
		error = ERR_CANNOT_CONNECT;
		close(sock);
		return;
	}
	DEBUG("listening on %s", path.c_str());
	umask(old_mode);
#else
	DEBUG("unix sockets not available, cannot listen on %s", path.c_str());
	error = ERR_CANNOT_CONNECT;
#endif
}

void ListenConnection::return_poll_mask(int mask) {
	CT("return_poll_mask %08x", mask);
	if (sock == -1 || is_disconnected || error != ERR_NONE)
		return;

	if (mask & MASKIN) {
		while (accept());
	}
	else if (mask & MASKERR) {
		disconnect();
		error = ERR_CANNOT_CONNECT;
	}
}

int ListenConnection::simple_accept() {
	CT("simple_accept");
	
	int client = ::accept(sock, 0, 0);
	if (client != -1)
		fcntl(client, F_SETFL, O_NONBLOCK);
	return client;
}

bool ListenConnection::accept() {
	CT("accept");
	
	int client = simple_accept();
	if (client != -1) {
		close(client);
		error = ERR_DISCONNECTED;
		return true;
	}
	error = ERR_DISCONNECTED;
	return false;
}

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

#ifndef __CORECONNECTION_HH__
#define __CORECONNECTION_HH__

#define MASKIN  1 << 0
#define MASKOUT 1 << 1
#define MASKERR 1 << 2

typedef enum {
	ERR_NONE = 0,
	ERR_LOOKUP_FAILED = -1,
	ERR_CANNOT_CONNECT = -2,
	ERR_TIMEOUT = -3,
	ERR_DISCONNECTED = -4
} ConnectionError;

class CoreConnection {
protected:
	int sock;
	bool is_disconnected;
	bool is_shaped;
	ConnectionError error;
	time_t next_event;

public:
	CoreConnection() : sock(-1), is_disconnected(false), is_shaped(false),
	                  error(ERR_NONE), next_event(0) { };
	virtual ~CoreConnection() { disconnect(); };
	
	virtual time_t get_next_event() { return next_event; };
	virtual void disconnect() {
		if (sock != -1 && ! is_disconnected) {
			is_disconnected = true;
			close(sock);
			sock = -1;
			error = ERR_DISCONNECTED;
			disconnected();
		}
	}

	virtual void disconnected() {};

	ConnectionError get_error() { return error; };
	int get_fd() { return sock; };

	virtual int get_poll_mask() { return 0; };
	virtual void return_poll_mask(int) {};
	bool get_shaped() const { return is_shaped; }
};

#endif // __CORECONNECTION_HH__

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

#ifndef __CLIENTCONNECTION_HH__
#define __CLIENTCONNECTION_HH__

#include <Museekal/CoreConnection.hh>

#include <string>
#include <queue>

class ClientConnection : public CoreConnection {
public:
	ClientConnection()
		: CoreConnection(), ready(false), can_receive(false), timeout(0), start(time(NULL)) { }
	
	void connect(const std::string&, uint, time_t = 0);
	void connect_unix(const std::string&);
	void set_socket(int _sock) { sock = _sock; ready = true; connected(); }
	
	virtual void connected() {};
	virtual void cannot_resolve();
	virtual void cannot_connect() {};
	virtual void connection_timeout() { cannot_connect(); };

	void send(const unsigned char *, size_t);
	void send(uint32);
	void send(off_t);
	void send(std::queue<unsigned char>&);
	void send(const std::basic_string<unsigned char>& s) { send(s.data(), s.size()); };

	virtual void data_sent(uint r) {};

	virtual void process() {};

	virtual int get_poll_mask();
	virtual void return_poll_mask(int);

protected:
	void get_ready(int);

	bool ready, can_receive;
	time_t timeout, start;
	std::queue<unsigned char> outbuf, inbuf;
};

#endif // __CLIENTCONNECTION_HH__

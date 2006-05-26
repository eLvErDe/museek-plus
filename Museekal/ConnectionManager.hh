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

#ifndef __CONNECTIONMANAGER_HH__
#define __CONNECTIONMANAGER_HH__

#include <Museekal/CoreConnection.hh>

#include <vector>
#include <map>

class ConnectionManager {
public:
	ConnectionManager();
	virtual ~ConnectionManager();

	void die() { wants_to_die = true; };

	virtual void add(CoreConnection*, bool=true);
	virtual void subscribe(CoreConnection*);
	virtual void remove(CoreConnection*);

	void cycle(int timeout = 60000);
#ifdef HAVE_EPOLL_CTL
	void cycle_epoll(int timeout = 60000);
#endif
	virtual void cycle_callback() {};
	virtual void loop(int timeout = 60000);

protected:
	std::vector<CoreConnection *> connections;
	std::map<int, CoreConnection *> fd_map;
	bool wants_to_die;
#ifdef HAVE_EPOLL_CTL
	int epoll_fd;
#endif
};

#endif // __CONNECTIONMANAGER_HH__

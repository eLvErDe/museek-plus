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

#include <Museekal/ConnectionManager.hh>
#include <Museekal/TrafficTracker.hh>

#define MULOG_DOMAIN "Museekal.CM"
#include <Muhelp/Mulog.hh>

ConnectionManager::ConnectionManager() {
	CT("ConnectionManager");
	
	wants_to_die = false;
#ifdef HAVE_EPOLL_CTL
	if ((epoll_fd = epoll_create(1024)) < 0) {
		DEBUG("error creating epoll fd");
	} else {
		DEBUG("created epoll fd");
	}
#endif
}

ConnectionManager::~ConnectionManager() {
	CT("~ConnectionManager");
	
	std::vector<CoreConnection *>::iterator it = connections.begin();
	for(; it != connections.end(); ++it)
		delete (*it);
#ifdef HAVE_EPOLL_CTL
	if (epoll_fd >= 0)
		close(epoll_fd);
#endif
}

void ConnectionManager::add(CoreConnection *conn, bool reg) {
	CT("add %lu, %d", (unsigned long) conn, reg);
	
	if (reg)
		subscribe(conn);
	connections.push_back(conn);
}

void ConnectionManager::subscribe(CoreConnection *conn) {
	CT("subscribe %ul", (unsigned long) conn);
	
	int fd = conn->get_fd();
#ifdef HAVE_EPOLL_CTL
	if(epoll_fd >= 0) {
		struct epoll_event ev;
		ev.events = EPOLLIN | EPOLLOUT;
		ev.data.fd = fd;
		ev.data.ptr = conn;
		if ((epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) < 0) && (errno != EEXIST)) {
			DEBUG("epoll insertion error (fd=%i)", fd);
			conn->disconnect();
			return;
		}
	}
#endif
	fd_map[fd] = conn;
}

void ConnectionManager::remove(CoreConnection *conn) {
	CT("remove %ul", (unsigned long) conn);
	
	int fd = conn->get_fd();

	if (fd != -1) {
#ifdef HAVE_EPOLL_CTL
		if(epoll_fd > 0)
			epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
#endif
		fd_map.erase(fd);
	}
	std::vector<CoreConnection *>::iterator it = connections.begin();
	for(; it != connections.end(); ++it)
		if (*it == conn) {
			connections.erase(it);
			return;
		}
}

#ifdef HAVE_EPOLL_CTL
void ConnectionManager::cycle_epoll(int timeout) {
	CT("cycle_epoll %d", timeout);
	
	struct epoll_event events[1024];
	std::vector<CoreConnection *>::iterator it = connections.begin();
	
	time_t curtime = time(NULL);
	
	bool up_breached = TrafficTracker::instance()->average(0) > 204800,
	   down_breached = TrafficTracker::instance()->average(1) > 204800;
	
	TT("Average upload   rate: %u", TrafficTracker::instance()->average(0));
	TT("Average download rate: %u", TrafficTracker::instance()->average(1));
	
	int i = 0;
	for (; it != connections.end(); ++it) {
		int fd = (*it)->get_fd(),
		    _mask = (*it)->get_poll_mask(),
		    mask = ((_mask & MASKERR) ? (EPOLLERR|EPOLLHUP) : 0);
		// if(! (*it)->get_shaped() || ! down_breached)
			mask |= ((_mask & MASKIN) ? EPOLLIN : 0);
		// if(! (*it)->get_shaped() || ! up_breached)
			mask |= ((_mask & MASKOUT) ? EPOLLOUT : 0);
		           
		if ((*it)->get_error() != ERR_NONE) {
			epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
			if(fd != -1)
				fd_map.erase(fd);
			delete *it;
			connections.erase(it);
			return;
		}
		if (fd == -1)
			continue;
		
		time_t next_ev = (*it)->get_next_event();
		if((next_ev != 0) && ((curtime + (timeout / 1000)) > next_ev)) {
			timeout = (next_ev - curtime) * 1000;
			TT("changing timeout to %i", timeout);
		}
		
		struct epoll_event ev;
		ev.data.fd = fd;
		ev.events = mask;
		ev.data.ptr = (*it);
		epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
	}
	
	if (connections.empty())
		return;

	i = epoll_wait(epoll_fd, events, 1024, timeout);
	if (i <= 0)
		return;

	for (int j = 0; j < i; j++) {
		if (! events[j].events)
			return;
		CoreConnection* conn = static_cast<CoreConnection*>(events[j].data.ptr);
		int mask = ((events[j].events & EPOLLIN) ? MASKIN : 0) |
		           ((events[j].events & EPOLLOUT) ? MASKOUT : 0) |
		           ((events[j].events & (EPOLLERR|EPOLLHUP)) ? MASKERR : 0);
		conn->return_poll_mask(mask);
	}

}
#endif

#ifdef HAVE_SYS_POLL_H
void ConnectionManager::cycle(int timeout) {
	CT("poll_cycle %d", timeout);
	
	struct pollfd *p_fd = new struct pollfd[connections.size()];
	std::vector<CoreConnection *>::iterator it = connections.begin();

	int i = 0;
	time_t curtime = time(NULL);

	for (; it != connections.end(); ++it) {
		int fd = (*it)->get_fd();
		int mask = (*it)->get_poll_mask();
		
		if ((*it)->get_error() != ERR_NONE) {
			if(fd != -1)
				fd_map.erase(fd);
			delete *it;
			connections.erase(it);
			delete [] p_fd;
			return;
		}

		if (fd == -1)
			continue;

		time_t next_ev = (*it)->get_next_event();
		if((next_ev != 0) && ((curtime + (timeout / 1000)) > next_ev)) {
			timeout = (next_ev - curtime) * 1000;
		}
		if (mask) {
			p_fd[i].fd = fd;
			p_fd[i].events = ((mask & MASKIN) ? POLLIN : 0) | 
			                 ((mask & MASKOUT) ? POLLOUT : 0) |
			                 ((mask & MASKERR) ? (POLLERR|POLLHUP|POLLNVAL) : 0);
			p_fd[i].revents = 0;
			i++;
		}
	}

	if (connections.empty()) {
		delete [] p_fd;
		return;
	}

	int ret = poll(p_fd, i, timeout);
	if (ret <= 0) {
		delete [] p_fd;
		return;
	}

	for (int j = 0; j < i; j++) {
		if (! p_fd[j].revents)
			continue;

		std::map<int, CoreConnection *>::iterator c_it;
		c_it = fd_map.find(p_fd[j].fd);

		int mask = ((p_fd[j].revents & POLLIN) ? MASKIN : 0) |
		           ((p_fd[j].revents & POLLOUT) ? MASKOUT : 0) |
		           ((p_fd[j].revents & (POLLERR|POLLHUP|POLLNVAL)) ? MASKERR : 0);

		if (c_it != fd_map.end())
			(*c_it).second->return_poll_mask(mask);

		ret--;
		if (! ret)
			break;
	}

	delete [] p_fd;
}
#else // ! HAVE_SYS_POLL_H
void ConnectionManager::cycle(int timeout) {
	CT("select_cycle %d", timeout);

	int i = 0;
	struct timeval tv;
	tv.tv_sec = timeout;
	tv.tv_usec = 0;
	
	time_t curtime = time(NULL);
	
	fd_set rfds, wfds, efds;
	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_ZERO(&efds);
	
	std::vector<CoreConnection *>::iterator it = connections.begin();
	for (; it != connections.end(); ++it) {
		int fd = (*it)->get_fd();

		int mask = (*it)->get_poll_mask();
		if ((*it)->get_error() != ERR_NONE) {
			if(fd != -1)
				fd_map.erase(fd);
			delete *it;
			connections.erase(it);
			return;
		}

		if (fd == -1)
			continue;

		time_t next_ev = (*it)->get_next_event();
		if((next_ev != 0) && ((curtime + tv.tv_sec) > next_ev))
			tv.tv_sec = next_ev - curtime;
		
		if (mask) {
			if(mask & MASKIN)
				FD_SET(fd, &rfds);
			if(mask & MASKOUT)
				FD_SET(fd, &wfds);
			if(mask & MASKERR)
				FD_SET(fd, &efds);
			if(fd > i)
				i = fd;
		}
	}

	if (connections.empty())
		return;
	
	int ret = select(i + 1, &rfds, &wfds, &efds, &tv);
	if (ret <= 0)
		return;

	int j = 0;
	while(ret) {
		int mask = 0;
		if(FD_ISSET(j, &rfds))
			mask |= MASKIN;
		if(FD_ISSET(j, &wfds))
			mask |= MASKOUT;
		if(FD_ISSET(j, &efds))
			mask |= MASKERR;
		if (! mask) {
			j++;
			continue;
		}
		
		std::map<int, CoreConnection *>::iterator c_it;
		c_it = fd_map.find(j);
		if (c_it != fd_map.end())
			(*c_it).second->return_poll_mask(mask);
		
		j++;
		ret--;
	}
}
#endif // ! HAVE_SYS_POLL_H

void ConnectionManager::loop(int timeout) {
	CT("loop %d", timeout);
	
	while ((! wants_to_die) && (! connections.empty())) {
#ifdef HAVE_EPOLL_CTL
		if(epoll_fd >= 0)
			cycle_epoll(timeout);
		else
			cycle(timeout);
#else
		cycle(timeout);
#endif
		cycle_callback();
#ifdef MULOG_CYCLE
		mulog("Museekal.CM.CYCLE", "cycle %i", connections.size());
#endif
	}
}

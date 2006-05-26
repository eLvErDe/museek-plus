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

#ifndef __LISTENCONNECTION_HH__
#define __LISTENCONNECTION_HH__

#include <Museekal/CoreConnection.hh>
#include <string>

class ListenConnection : public CoreConnection {
public:
	ListenConnection();
	void listen(const std::string&, uint);
	void listen_unix(const std::string&);

	int domain() const { return mDomain; }
	
	virtual int get_poll_mask() { return MASKIN; };
	virtual void return_poll_mask(int mask);

protected:
	int simple_accept();
	virtual bool accept();
	
	int mDomain;
};

#endif // __LISTENCONNECTION_HH__

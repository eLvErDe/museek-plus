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

#ifndef __DISTRIBMANAGER_HH__
#define __DISTRIBMANAGER_HH__

class Museek;
class DistribConnection;

class DistribManager {
public:
	DistribManager(Museek* museek);
	
	inline Museek* museek() const { return mMuseek; }
	
	void server_disconnected();
	void add_parents(const NetInfo& net_info);
	
	void check_cache();
	
	void add(DistribConnection* connection);
	void remove(DistribConnection* connection);
	
	void cycle();
	bool pierced_firewall(uint32 sock, uint32 token);
	
private:
	void connect_to_parent();
	
	Museek *mMuseek;
	NetInfo mCache;
	
	DistribConnection *mParent;
	std::vector<DistribConnection*> mChildren;
};

#endif // __DISTRIBMANAGER_HH__


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

#ifndef __DISTRIBCONNECTION_HH__
#define __DISTRIBCONNECTION_HH__

#include <Museek/BaseConnection.hh>

#include <string>

class DistribManager;

class DistribConnection : public BaseConnection {
public:
	DistribConnection(Peer* manager, uint32 token, bool parent);
	~DistribConnection();
	
	inline bool parent() const { return mParent; }
	
protected:
	void process_message(uint32 code);
	
private:
	DistribManager* mManager;
	bool mParent;
};

#endif // __PEERCONNECTION_HH__

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

#ifndef __DISTRIBMESSAGES_HH__
#define __DISTRIBMESSAGES_HH__

#include <Museek/Messages.hh>

class DistribMessage : public NetworkMessage {};
#define DISTRIBMESSAGE(mtype, m_id) NETWORKMESSAGE(DistribMessage, mtype, m_id)

DISTRIBMESSAGE(DPing, 0)
	DPing() { };
	
	MAKE
	END_MAKE
	
	PARSE
	END_PARSE
END

DISTRIBMESSAGE(DSearchRequest, 3)
	DSearchRequest() { };
	DSearchRequest(uint32 _uu, const std::string& _u, uint32 _t, const std::string& _q)
	              : user(_u), query(_q), unknown(_uu), ticket(_t) { };
	
	MAKE
		pack(unknown);
		pack(user);
		pack(ticket);
		pack(query);
	END_MAKE
	
	PARSE
		unknown = unpack_int();
		user = unpack_string();
		ticket = unpack_int();
		query = unpack_string();
	END_PARSE
	
	std::string user, query;
	uint32 unknown, ticket;
END

#endif // __DISTRIBMESSAGES_HH__

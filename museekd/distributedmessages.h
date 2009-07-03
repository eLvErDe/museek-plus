/* Museek - Museek's 'core' library
 *
 * Copyright (C) 2003-2007 Hyriand <hyriand@thegraveyard.org>
 * Copyright 2008 little blue poney <lbponey@users.sourceforge.net>
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

#ifndef MUSEEK_DISTRIBUTEDMESSAGES_H
#define MUSEEK_DISTRIBUTEDMESSAGES_H

#include "networkmessage.h"
#include <string>
#include <vector>
#include <queue>

namespace Museek
{
	class DistributedSocket;
}

class DistributedMessage : public NetworkMessage
{
public:
	void setDistributedSocket(Museek::DistributedSocket * distributedSocket)
	{
		m_DistributedSocket = distributedSocket;
	}
	Museek::DistributedSocket * distributedSocket() const
	{
		return m_DistributedSocket;
	}
protected:
    void default_garbage_collector() { }
private:
	Museek::DistributedSocket * m_DistributedSocket;
};

/* Voodoo magic preprocessing: shortcut for defining a distributed message. */
#define DISTRIBUTEDMESSAGE(mtype, m_id) \
  class mtype : public DistributedMessage \
  { \
  private: \
    uchar get_type() { return m_id; } \
  protected: \
    std::string get_name() { return #mtype ; } \
  public:
#define END };


DISTRIBUTEDMESSAGE(DPing, 0)
    DPing() {unknown = 0;};
    DPing(uint _u) {unknown = _u;};

	MAKE
        if (unknown > 0)
            pack((uint32) unknown);
	END_MAKE

	PARSE
        unknown = unpack_int();
	END_PARSE

	uint unknown;
END

DISTRIBUTEDMESSAGE(DSearchRequest, 3)
    DSearchRequest() {};
	DSearchRequest(uint _u, const std::string & _n, uint _t, const std::string& _q) : unknown(_u), ticket(_t), username(_n), query(_q) { };

	MAKE
        pack(unknown);
        pack(username);
		pack(ticket);
		pack(query);
	END_MAKE

	PARSE
		unknown = unpack_int();
		username = unpack_string();
		ticket = unpack_int();
		query = unpack_string();
	END_PARSE

	uint unknown, ticket;
	std::string username, query;
END

DISTRIBUTEDMESSAGE(DBranchLevel, 4)
    DBranchLevel() {};
    DBranchLevel(uint _l) : level(_l) {};

	MAKE
        pack(level);
	END_MAKE

	PARSE
        level = unpack_int();
	END_PARSE

	uint level;
END

DISTRIBUTEDMESSAGE(DBranchRoot, 5)
    DBranchRoot() {};
    DBranchRoot(const std::string & _r) : root(_r) {};

	MAKE
        pack(root);
	END_MAKE

	PARSE
        root = unpack_string();
	END_PARSE

	std::string root;
END

DISTRIBUTEDMESSAGE(DChildDepth, 7)
    DChildDepth() {};
    DChildDepth(uint _d) : depth(_d) {};

	MAKE
        pack(depth);
	END_MAKE

	PARSE
        depth = unpack_int();
	END_PARSE

	uint depth;
END

#endif // MUSEEK_DISTRIBUTEDMESSAGES_H

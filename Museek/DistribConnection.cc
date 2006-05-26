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

#include <Museek/mu.hh>

#include <Museek/DistribConnection.hh>
#include <Museek/DistribManager.hh>
#include <Museek/DistribMessages.hh>
#include <Museek/PeerManager.hh>
#include <Museek/Recoder.hh>
#include <Museek/Museek.hh>

#define MULOG_DOMAIN "Museekd.DC"
#include <Muhelp/Mulog.hh>

DistribConnection::DistribConnection(Peer* peer, uint32 token, bool parent)
                  : BaseConnection(1, "D", peer, token), mParent(parent)
{
	CT("DistribConnection::DistribConnection %s, %d", peer->user().c_str(), parent);
	
	mManager = peer->manager()->museek()->distrib_manager();
	mManager->add(this);
}

DistribConnection::~DistribConnection()
{
	mManager->remove(this);
}

#ifdef PARSE
# undef PARSE
#endif
#define PARSE(mtype) mtype s; s.parse_network_packet(message);

void
DistribConnection::process_message(uint32 code)
{
	CT("process_message %u (%s)", code, peer()->user().c_str());
	switch(code) {
	case 0: {
		PARSE(DPing);
		DEBUG("Got distrib ping");
		break;
	}
	case 3: {
		PARSE(DSearchRequest);
		DEBUG("Got distrib search request %u, %s, %u, %s", s.unknown, s.user.c_str(), s.ticket, s.query.c_str());
		mManager->museek()->cb_distrib_search(s.user, s.ticket, mManager->museek()->recoder()->decode_user(s.user, s.query));
		break;
	}
	default:
		BaseConnection::process_message(code);
	}
		
}

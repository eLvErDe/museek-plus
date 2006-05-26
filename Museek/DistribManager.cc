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

#include <Museek/DistribManager.hh>
#include <Museek/DistribConnection.hh>
#include <Museek/Museek.hh>
#include <Museek/PeerManager.hh>

#define MULOG_DOMAIN "Museek.DM"
#include <Muhelp/Mulog.hh>

using std::vector;

DistribManager::DistribManager(Museek *museek)
               : mMuseek(museek), mParent(0)
{
}

void
DistribManager::server_disconnected()
{
	mCache.clear();
	
	if(mParent)
	{
		mParent->disconnect();
		mParent = 0;
	}
	
	vector<DistribConnection*>::iterator it, end = mChildren.end();
	for(it = mChildren.begin(); it != end; ++it)
		(*it)->disconnect();
}

void
DistribManager::add_parents(const NetInfo& net_info)
{
	NetInfo::const_iterator it, end = net_info.end();
	for(it = net_info.begin(); it != end; ++it)
		mCache[(*it).first] = (*it).second;
	
	if(mCache.size() >= mMuseek->min_parents_in_cache())
		mMuseek->server_no_parent(false);
	
	cycle();
}

void
DistribManager::add(DistribConnection *connection)
{
	if(connection->parent())
		mParent = connection;
	else
		mChildren.push_back(connection);
}

void
DistribManager::check_cache()
{
	if(mCache.size() < mMuseek->min_parents_in_cache())
		mMuseek->server_no_parent(true);
}

void
DistribManager::connect_to_parent()
{
	if(mParent)
	{
		mParent->disconnect();
		mParent = 0;
	}
	
	if(mCache.empty())
		return;
	
	NetInfo::iterator it = mCache.begin();
	
	DEBUG("making distrib connection to parent %s", (*it).first.c_str());
	
	Peer *peer = mMuseek->peer_manager()->get_peer((*it).first);
	mParent = new DistribConnection(peer, mMuseek->token(), true);
	if(mMuseek->connect_mode() == CM_Passive)
		mParent->pierce_firewall();
	else
	{
		mParent->init();
		if(! peer->have_address())
		{
			mParent->set_state(BS_Address);
			peer->set_address((*it).second.first, (*it).second.second);
		}
		else
			mParent->connect();
	}
	
	mCache.erase(it);
	check_cache();
}


void
DistribManager::remove(DistribConnection *connection)
{
	if(connection == mParent)
	{
		DEBUG("Lost connection to distrib. parent");
		mParent = 0;
	} else {
		vector<DistribConnection*>::iterator it = find(mChildren.begin(), mChildren.end(), connection);
		if(it != mChildren.end())
			mChildren.erase(it);
	}
}

void
DistribManager::cycle()
{
	if(! (mCache.empty() || mParent))
		connect_to_parent();
}

bool
DistribManager::pierced_firewall(uint32 sock, uint32 token)
{
	if(mParent && mParent->state() == BS_Waiting && mParent->token() == token)
	{
		mParent->pierced_firewall(sock);
		return true;
	}
	
	vector<DistribConnection*>::iterator it, end = mChildren.end();
	for(it = mChildren.begin(); it != end; ++it)
		if((*it)->state() == BS_Waiting && mParent->token() == token)
		{
			(*it)->pierced_firewall(sock);
			return true;
		}
	
	return false;
}

/* museekd - The Museek daemon
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
#include <museekd/Buddies.hh>
#include <museekd/museekd.hh>
#include <Museek/PeerManager.hh>

#define MULOG_DOMAIN "museekd.BL"
#include <Muhelp/Mulog.hh>

void Buddies::add(const std::string& user) {
	if(has(user))
		return;
	
	mBuddies.push_back(user);
	
	Peer* peer = mMuseekd->peer_manager()->get_peer(user);
	peer->inc_ref();
	
	if(mMuseekd->privilege_buddies())
		peer->set_privileged(true);
	
	if(! peer->subscribed())
		peer->subscribe();
}

void Buddies::del(const std::string& user) {
	std::vector<std::string>::iterator it = find(mBuddies.begin(), mBuddies.end(), user);
	if(it == mBuddies.end())
		return;
	
	mBuddies.erase(it);
	
	Peer *peer = mMuseekd->peer_manager()->get_peer(user);
	
	if(mMuseekd->privilege_buddies())
		peer->set_privileged(mMuseekd->is_privileged(user));
	
	peer->dec_ref();
}

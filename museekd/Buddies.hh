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

#ifndef __BUDDIES_HH__
#define __BUDDIES_HH__

#include <vector>
#include <string>
#include <algorithm>

class Museekd;

class Buddies {
public:
	Buddies(Museekd* museekd) : mMuseekd(museekd) { };
	
	void add(const std::string& user);
	void del(const std::string& user);
	
	inline bool has(const std::string& user) const
	       { return find(mBuddies.begin(), mBuddies.end(), user) != mBuddies.end(); }
	inline operator std::vector<std::string>() const
	       { return mBuddies; }
	
private:
	Museekd* mMuseekd;
	std::vector<std::string> mBuddies;
};

#endif // __BUDDIES_HH__

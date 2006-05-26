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

#include <Museekal/TrafficTracker.hh>

#define MULOG_DOMAIN "Museekal.TT"
#include <Muhelp/Mulog.hh>

TrafficTracker * TrafficTracker::mInstance = 0;

static inline double difftime(struct timeval& a, struct timeval& b) {
	struct timeval diff;
	diff.tv_sec = a.tv_sec - b.tv_sec;
	diff.tv_usec = a.tv_usec - b.tv_usec;
	while(diff.tv_usec < 0)
	{
		diff.tv_sec--;
		diff.tv_usec += 1000000L;
	}
	return diff.tv_sec + (diff.tv_usec / 1000000.0);
}

uint
TrafficTracker::average(int channel)
{
	size_t len = mWindow[channel].size();
	if(! len)
		return 0;
	
	struct timeval now;
	gettimeofday(&now, 0);
	while(difftime(now, mWindow[channel].front().first) > mWinLength || len > mMaxWinSize)
	{
		mWindow[channel].pop_front();
		len -= 1;
		if(! len)
			break;
	}
	
	if(! len)
		return 0;
	
	uint count = 0;
	
	std::deque<std::pair<struct timeval, uint> >::iterator it, end;
	end = mWindow[channel].end();
	for(it = mWindow[channel].begin(); it != end; ++it)
		count += (*it).second;
	
	double dt = difftime(now, mWindow[channel].front().first);
	if(dt < 1.0)
		dt = 1.0;
	
	TT("%u frames, total count: %u, td: %.1f, avg: %.1f", len, count, dt, (double)count / dt);
	
	return static_cast<uint>(static_cast<double>(count) / dt);
}

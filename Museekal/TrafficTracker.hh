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

#ifndef __TRAFFICTRACKER_HH__
#define __TRAFFICTRACKER_HH__

#include <deque>
#include <iostream>

class TrafficTracker {
public:
	TrafficTracker(double winlength = 10.0, int maxwinsize = 1000)
	  : mWinLength(winlength), mMaxWinSize(maxwinsize) {}
	
	inline void collect(int channel, uint count);
	uint average(int channel);
	
	inline static TrafficTracker * instance();
	inline static void setInstance(TrafficTracker * tracker);
	
private:
	std::deque<std::pair<struct timeval, uint> > mWindow[2];
	double mWinLength;
	uint mMaxWinSize;
	
	static TrafficTracker * mInstance;
};

void TrafficTracker::collect(int channel, uint count)
{
	struct timeval now;
	gettimeofday(&now, 0);
	mWindow[channel].push_back(std::pair<struct timeval, uint>(now, count));
}

TrafficTracker * TrafficTracker::instance()
{
	if(! mInstance)
		mInstance = new TrafficTracker;
	return mInstance;
}

void TrafficTracker::setInstance(TrafficTracker * tracker)
{
	if(mInstance)
	{
		if(tracker)
		{
			tracker->mWindow[0] = mInstance->mWindow[0];
			tracker->mWindow[1] = mInstance->mWindow[1];
		}
		delete mInstance;
	}
	mInstance = tracker;
}

#endif // __TRAFFICTRACKER_HH__

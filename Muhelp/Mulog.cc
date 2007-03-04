/* Muhelp - Helper library for Museek
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
#include <syslog.h>

#include <Muhelp/Mulog.hh>

void Mulog::operator() (const std::string& _class, const char* fmt, ...) {
	va_list ap;
	int n, size = 1024;
	char *p;

	if((p = (char *)malloc(size)) == NULL)
		return;

	while(1) {
		va_start(ap, fmt);
		n = vsnprintf(p, size, fmt, ap);
		va_end(ap);
		if(n > -1 && n < size) {
			std::list<MulogSink *>::iterator it = sinks.begin();
			for(; it != sinks.end(); ++it)
				(*it)->log(_class, p);
			free(p);
			return;
		}
		if(n > -1)
			size = n+1;
		else
			size *= 2;
		if((p = (char *)realloc(p, size)) == NULL)
			return;
	}
}

void Mulog::setOutput(const int o) {
	if ((o < 0) || (o > 1)) {
		DEBUG("illegal output value in Mulog::setOutput()");
		return;
	}
	std::list<MulogSink *>::iterator it = sinks.begin();
	while (sinks.size() > 0) {
		MulogSink *oldSink = sinks.front();
		sinks.pop_front();
		delete(oldSink);
	}
	switch (o) {
		case 1 :
			add( new MulogSyslog(this) );
			mOutput = 1;
			break;
		case 0 :
			add (new MulogConsole() );
			mOutput = 0;
			break;
	}
}

const int Mulog::output() {
	std::cerr << "Mulog::output()" << std::endl;
	return(mOutput);
}

void Mulog::setSyslogFacility(int f) {
	// hard-coded limits from syslog.h
	if ((f < 0) || (f > (23<<3))) {
		return;
	}
	mSyslogFacility = f;
}

const int Mulog::syslogFacility() {
	return(mSyslogFacility);
}

void Mulog::setSyslogPriority(int p) {
	// hard-coded limits from syslog.h
	if ((p < 0) || (p > 7)) {
		return;
	}
	mSyslogPriority = p;
}

const int Mulog::syslogPriority() {
	return(mSyslogPriority);
}

void MulogSyslog::log(const std::string& _c, const std::string& _m) {
	openlog("museekd", LOG_PID, mParent->syslogFacility());
	syslog(mParent->syslogPriority(), "%s", _m.data());
	closelog();
}

Mulog mulog;

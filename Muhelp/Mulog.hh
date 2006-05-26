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

#ifndef __MULOG_HH__
#define __MULOG_HH__

#include <string>
#include <map>
#include <list>
#include <iostream>

class MulogSink {
public:
	virtual ~MulogSink() {}
	virtual void log(const std::string&, const std::string&) {};
};

class MulogConsole : public MulogSink {
public:
	virtual void log(const std::string& _c, const std::string& _m) 
		{ std::cerr << "[" << _c << "] " << _m << std::endl; };
};

class Mulog {
public:
	Mulog() { add(new MulogConsole()); };
	void operator() (const std::string&, const char*, ...);
	void add(MulogSink *_s) { sinks.push_back(_s); };
	void remove(MulogSink *_s) { sinks.remove(_s); };
protected:
	std::list<MulogSink *> sinks;
};

extern Mulog mulog;

#ifndef MULOG_DOMAIN
# define MULOG_DOMAIN "UNDEFINED"
#endif

#ifdef CPP99_VARARGS
# ifdef MULOG_CALLTRACE
#  define CT(...) mulog(MULOG_DOMAIN ".CT", __VA_ARGS__)
# else
#  define CT(...)
# endif

# ifdef MULOG_DEBUG
#  define DEBUG(...) mulog(MULOG_DOMAIN ".DBG", __VA_ARGS__)
# else
#  define DEBUG(...)
# endif

# ifdef MULOG_TT
#  define TT(...) mulog(MULOG_DOMAIN ".TT", __VA_ARGS__)
# else
#  define TT(...)
# endif

#else // ! CPP99_VARARGS

# ifdef MULOG_CALLTRACE
#  define CT(args...) mulog(MULOG_DOMAIN ".CT", args)
# else
#  define CT(args...)
# endif

# ifdef MULOG_DEBUG
#  define DEBUG(args...) mulog(MULOG_DOMAIN ".DBG", args)
# else
#  define DEBUG(args...)
# endif

# ifdef MULOG_TT
#  define TT(args...) mulog(MULOG_DOMAIN ".TT", args)
# else
#  define TT(args...)
# endif

#endif // CPP99_VARARGS

#endif // __MULOG_HH__

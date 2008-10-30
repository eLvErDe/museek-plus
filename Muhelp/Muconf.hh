/* Muhelp - Helper library for Museek
 *
 * Copyright (C) 2003-2004 Hyriand <hyriand@thegraveyard.org>
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

#ifndef __MUCONF_HH__
#define __MUCONF_HH__

#include <string>
#include <map>
#include <vector>
#include <libxml++/libxml++.h>

class MuconfDomain;
class Muconf;

class MuconfKey {
public:
	MuconfKey() { };
	MuconfKey(const std::string&);

	
	MuconfDomain* domain() const;
	
	uint asUint() const;
	int asInt() const;
	double asDouble() const;
	bool asBool() const;
	
	void operator=(const std::string&);
	void operator=(const char*);
	void operator=(uint);
	void operator=(int);
	void operator=(double);
	void operator=(bool);

	bool operator==(const std::string&) const;
	bool operator!=(const std::string&) const;
	bool operator!() const;
	operator const char*() const;
	operator std::string() const;
	
private:
	std::string mKey, mValue;
};

class MuconfDomain {
public:
	MuconfDomain() : mDomain("") { };
	MuconfDomain(const std::string&);

	std::string domain() const;
	
	void store(xmlpp::Element*) const;
	void restore(const xmlpp::Element*);
	
	bool hasKey(const std::string&) const;
	std::vector<std::string> keys() const;
	
	void remove(const std::string&);
	
	MuconfKey& operator[](const std::string&);
	operator std::map<std::string, std::string>() const;
	
private:
	std::string mDomain;
	std::map<std::string, MuconfKey> mKeys;
};


class Muconf {
public:
	Muconf();
	Muconf(const std::string&);

	std::string filename() const;

	void store();
	void restore(const xmlpp::Element*);
	
	bool hasDomain(const std::string&) const;
	std::vector<std::string> domains() const;
	
	MuconfDomain& operator[](const std::string&);
	operator std::map<std::string, std::map<std::string, std::string> >() const;
	
private:
	std::map<std::string, MuconfDomain> mDomains;
	std::string mFilename;
};

#endif // __MUCONF_HH__

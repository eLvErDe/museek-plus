/* Tools - Tools for Museek (muscan)
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

#ifndef __SCANNER_HH__
#define __SCANNER_HH__

#include <Muhelp/DirEntry.hh>

#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

class DirScanner : public DirEntry {
public:
	DirScanner(bool _f = true) : DirEntry(_f) { };
	DirScanner(const std::string& _p) : DirEntry(_p) { };
	
	void add(const std::string& path);
	
	virtual DirEntry* new_folder(bool fake);
	virtual DirEntry* new_folder(const std::string& path);
	void scan(const struct stat* = NULL);
	
	FileEntry scan_file(const std::string&);
	void real_scan();
};

extern int Scanner_Verbosity;

#endif // __SCANNER_HH__

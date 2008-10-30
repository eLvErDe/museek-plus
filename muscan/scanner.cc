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

#include <system.h>

#include <cstdlib>

extern "C" {
# include "mp3.h"
}

#ifndef HAVE_SCANDIR
# include "scandir.hh"
#endif

#ifdef HAVE_VORBIS
# include <vorbis/vorbisfile.h>
#endif

#include <Muhelp/DirEntry.hh>
#include <Muhelp/string_ext.hh>
#include <NewNet/nnlog.h>

#include <iostream>

#include "scanner.hh"

using std::map;
using std::vector;
using std::string;
using std::cerr;
using std::cout;
using std::endl;

#ifdef HAVE_VORBIS
# ifdef RELAYED_LIBVORBISFILE
extern int libvorbisfile_is_present;
# else
static int libvorbisfile_is_present = 1;
# endif
#endif

int Scanner_Verbosity = 0;

void DirScanner::add(const string& path) {
	if(folders.find(path) == folders.end())
		folders[path] = new_folder(path);
}

DirEntry* DirScanner::new_folder(bool fake) {
	NNLOG("museek.dirscanner", "new_folder %i", fake);
	
	return new DirScanner(fake);
}

DirEntry* DirScanner::new_folder(const string& path) {
	NNLOG("museek.dirscanner", "new_folder %s", path.c_str());
	
	return new DirScanner(path);
}


void DirScanner::scan(const struct stat* s) {
	NNLOG("museek.dirscanner", "scan <...>");
	
	bool uptodate = false;
	if (s != NULL) {
		uptodate = s->st_mtime == mtime;
		mtime = s->st_mtime;
	}
	if (! fake && ! uptodate) {
		real_scan();
		return;
	}
	
	if(Scanner_Verbosity > 1 && ! fake)
		cout << "Skipping " << path << endl;
	
	map<string, DirEntry*>::iterator dit = folders.begin();
	for(; dit != folders.end(); ++dit) {
		struct stat s2;
		if (stat((*dit).first.c_str(), &s2)) {
			(*dit).second->files.clear();
			for(map<string,DirEntry*>::iterator it = (*dit).second->folders.begin(); it != (*dit).second->folders.end(); ++it)
				delete (*it).second;
			(*dit).second->folders.clear();
			continue;
		}
		static_cast<DirScanner*>((*dit).second)->scan(&s2);
	}
}


FileEntry DirScanner::scan_file(const string& path) {
	NNLOG("museek.dirscanner", "scan file %s", path.c_str());
	
	if(Scanner_Verbosity > 2)
		cout << "Identifying " << path << endl;
	
	FileEntry fe;
	mp3info info;
	int l = path.length();
	if (l > 4) {
		if(tolower(path.substr(l-4, l)) == ".mp3") {
			if (mp3_scan(path.c_str(), &info)) {
				fe.attrs.push_back(info.bitrate);
				fe.attrs.push_back(info.length);
				fe.attrs.push_back(info.vbr);
				fe.ext = "mp3";
			} else {
				cerr << "Invalid mp3 " << path << endl;
			}
#ifdef HAVE_VORBIS
		} else if(libvorbisfile_is_present && tolower(path.substr(l-4, l)) == ".ogg") {
			FILE *f = fopen(path.c_str(), "r");
			if (f == NULL)
				return fe;
			OggVorbis_File ovf;
			if(ov_open(f, &ovf, NULL, 0) != 0) {
				fclose(f);
				return fe;
			}
			vorbis_info *vinfo = ov_info(&ovf, -1);
			if (vinfo != NULL) {
				long bitrate = ov_bitrate(&ovf, -1);
				fe.attrs.push_back(bitrate / 1000);
				fe.attrs.push_back((int)ov_time_total(&ovf, -1));
				if ((bitrate == vinfo->bitrate_nominal) && (bitrate == vinfo->bitrate_upper) && (bitrate == vinfo->bitrate_lower))
					fe.attrs.push_back(0);
				else
					fe.attrs.push_back(1);
				fe.ext = "mp3";
			}
			ov_clear(&ovf);
#endif // HAVE_VORBIS
		}
	}
	return fe;
}

void DirScanner::real_scan() {
	NNLOG("museek.dirscanner", "real_scan");
	
	if(Scanner_Verbosity > 0)
		cout << "Scanning " << path << endl;
	
	files.clear();

	struct SCANDIR_ENTRY **temp;
	struct stat s;
	int n;
        if(path[path.size() - 1] == '/')
		path = path.substr(0, path.size() - 1);
        
        if (path.substr(path.rfind('/')+1).rfind(".") == 0 ) {
            cout << "Warning: " << path.c_str() << " is a hidden directory, not sharing." << endl;
            return;
        }

#if SCANDIR_ENTRY != dirent
	char *x = strdup(path.c_str());
	if((n = scandir(x, &temp, NULL, NULL)) < 0) {
		free(x);
		folders.clear();
		return;
	}
	free(x);
#else // SCANDIR_ENTRY == dirent
	if((n = scandir(path.c_str(), &temp, NULL, NULL)) < 0) {
		folders.clear();
		return;
	}
#endif
	
	map<string, DirEntry*>newfolders;

	while (n--) {
		string fn = temp[n]->d_name,
			full = path + "/" + fn;
		free(temp[n]);

		if ((fn == ".") || (fn == "..") || (stat(full.c_str(), &s) != 0))
			continue;
		if ( fn.rfind(".") == 0 )
		        // Ignore dot-files
		        continue;
		if(S_ISREG(s.st_mode)) {
			FileEntry fe = scan_file(full);
			fe.size = s.st_size;
			files[fn] = fe;
		} else if (S_ISDIR(s.st_mode)) {
			map<string, DirEntry*>::iterator dit = folders.find(full);
			if (dit != folders.end()) {
				newfolders[full] = (*dit).second;
				static_cast<DirScanner*>(newfolders[full])->scan(&s);
			} else {
				DirEntry* de = new_folder(full);
				static_cast<DirScanner*>(de)->scan(&s);
				newfolders[full] = de;
			}
			
		}
	}
	folders = newfolders;
	free(temp);
}

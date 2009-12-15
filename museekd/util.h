/*  Museek - A SoulSeek client written in C++
    Copyright (C) 2006-2007 Ingmar K. Steen (iksteen@gmail.com)
    Copyright 2008 little blue poney <lbponey@users.sourceforge.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */

#ifndef MUSEEK_UTIL_H
#define MUSEEK_UTIL_H

#include <NewNet/nnpath.h>
#include <Muhelp/string_ext.hh>
#ifndef WIN32
#include <sys/stat.h>
#include <sys/types.h>
#include <regex.h>
#else
#include <io.h>
#endif // ! WIN32
#include <errno.h>
#include <stdint.h>
#include <fstream>

/* Make sure a directory exists by creating all the pieces from the ground
   up. Returns true if all went ok, false if things went bad. */
static inline bool makedirs(const std::string & dir)
{
  // Split the path into pieces.
  std::vector<std::string> pieces = NewNet::Path(dir).split(), pieces_;
  // Iterate over all the pieces.
  std::vector<std::string>::iterator it, end = pieces.end();
  for(it = pieces.begin(); it != end; ++it)
  {
    // Add piece to our piece vector.
    pieces_.push_back(*it);
    // Join them into a full path.
    std::string path = NewNet::Path(pieces_).path();
    if(path.empty())
      continue;
    // Create the piece of the path.
#ifndef WIN32
    if(mkdir(path.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == -1 && errno != EEXIST)
#else
    if(mkdir(path.c_str()) == -1 && errno != EEXIST)
#endif // ! WIN32
    {
      // Error and path didn't already exist. Issue a warning.
      NNLOG("museekd.user.warn", "Couldn't create directory '%s'.", path.c_str());
      return false;
    }
  }

  // Success.
  return true;
}

#ifdef WIN32

#include <shlobj.h>
#include <string>

static inline
std::string getConfigPath(const std::string & appName)
{
  LPITEMIDLIST pidl;
  if(SUCCEEDED(SHGetSpecialFolderLocation(0, CSIDL_APPDATA, &pidl)))
  {
    char path[MAX_PATH];
    if(SUCCEEDED(SHGetPathFromIDList(pidl, path)))
    {
      CoTaskMemFree(pidl);
      return std::string(path) + "\\" + appName;
    }
    CoTaskMemFree(pidl);
  }

  char path[MAX_PATH];
  GetModuleFileName(0, path, sizeof(path) - 1);
  std::string strPath(path);
  return strPath.substr(0, strPath.rfind('\\')) + "\\config";
}

#endif // WIN32

static inline int write_int(std::ofstream * ofs, uint32 i) {
	if(ofs->fail() || !ofs->is_open())
		return -1;

	unsigned char d[4];
	for(int j = 0; j < 4; j++) {
		d[j] = i & 0xff;
		i = i >> 8;
	}
	ofs->write((char *) d, 4);
	if(ofs->fail() || !ofs->is_open())
		return -1;
    return 4;
}

static inline int write_off(std::ofstream * ofs, uint64 i) {
	if(ofs->fail() || !ofs->is_open())
		return -1;

	unsigned char d[8];
	for(int j = 0; j < 8; j++) {
		d[j] = i & 0xff;
		i = i >> 8;
	}
	ofs->write((char *) d, 8);
	if(ofs->fail() || !ofs->is_open())
		return -1;
    return 8;
}

static inline int write_str(std::ofstream * ofs, const std::string& str) {
	if(!write_int(ofs, str.size()))
		return -1;

	const char* d = str.data();
	ofs->write(d, str.size());
	if(ofs->fail() || !ofs->is_open())
		return -1;
    return str.size();
}

static inline int read_int(std::ifstream * ifs, uint32* r) {
    *r = 0;
	if(ifs->fail() || !ifs->is_open())
		return -1;

	unsigned char d[4];
	ifs->read((char *)d, 4);
	if(ifs->fail() || !ifs->is_open())
		return -1;

	for(uint32 j = 0; j < 4; j++)
		(*r) += d[j] << (j * 8);
    return 4;
}

static inline int read_off(std::ifstream * ifs, uint64* r) {
    (*r) = 0;
	if(ifs->fail() || !ifs->is_open())
		return -1;

	unsigned char d[8];
	ifs->read((char *)d, 8);
	if(ifs->fail() || !ifs->is_open())
		return -1;

	for(uint64 j = 0; j < 8; j++) {
		(*r) += d[j] << (j * 8);
	}
	return 8;
}

static inline int read_str(std::ifstream * ifs, std::string& r) {
	uint32 len;
	if(!read_int(ifs, &len))
		return -1;

	char d[len];
	ifs->read(d, len);
	if(ifs->fail() || !ifs->is_open())
		return -1;

	r.append(d, len);

	return len + 4;
}

/**
 * Returns true if stringStr matched the given pattern (wildStr). False otherwise.
 * Authorized special characters are: * (any chars) and ? (exactly one char)
 */
static inline bool wildcmp(const std::string & wildStr, const std::string & stringStr) {
#ifndef WIN32
    int err;
    regex_t preg;

    std::string regex = wildStr;
    regex = str_replace(regex, "\\", "\\\\");
    regex = str_replace(regex, "+", "\\+");
    regex = str_replace(regex, ".", "\\.");
    regex = str_replace(regex, "{", "\\{");
    regex = str_replace(regex, "}", "\\}");
    regex = str_replace(regex, "|", "\\|");
    regex = str_replace(regex, "(", "\\(");
    regex = str_replace(regex, ")", "\\)");
    regex = str_replace(regex, "^", "\\^");
    regex = str_replace(regex, "$", "\\$");

    regex = str_replace(regex, "*", ".*");
    regex = str_replace(regex, "?", ".{1}"); // FIXME doesn't work

    regex = "^"+regex+"$";

    const char *str_regex = regex.c_str();
    const char *str_request = stringStr.c_str();

    err = regcomp (&preg, str_regex, REG_NOSUB | REG_EXTENDED | REG_ICASE);

    if (err == 0)
    {
      int match;

      match = regexec (&preg, str_request, 0, NULL, 0);

      regfree (&preg);

      if (match == 0)
         return true;
      else
          return false;
    }

    NNLOG("museekd.util.warn", "Error while using regex %s.", regex.c_str());

    return false;

#endif // WIN32

return false; // For Win32
}
#endif // MUSEEK_UTIL_H

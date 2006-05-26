/* Museek - Museek's 'core' library
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

#ifndef __SHAREBASE_HH__
#define __SHAREBASE_HH__

#include <string>
#include <vector>
#include <Muhelp/DirEntry.hh>

class Museek;

class ShareBase {
public:
	ShareBase(Museek* museek);
	
	void load(const std::string& db, const std::string& type);
	
	inline uint32 folders() const { return mNumFolders; }
	inline uint32 files() const { return mNumFiles; }
	
	bool is_shared(const std::string& path) const;
	
	inline const std::vector<unsigned char>& shares() const { return mCompressed; }
	void search(const std::wstring& query, Folder& result);
	Shares folder_contents(const std::string& _f);
	
protected:
	void update(const std::string& type);
	void recode();
	void update_flat();
	void update_compressed();
	void update_word_maps();
	
private:
	inline std::map<std::string, FileEntry> fetch(const std::wstring& word) const;
	
	Museek* mMuseek;
	
	uint32 mNumFolders, mNumFiles;
	
	DirEntry mShares, mRecoded;
	Folder mFlat;
	
	std::vector<unsigned char> mCompressed;
	
	std::map<wchar_t, std::map<std::wstring, std::map<std::string, FileEntry> > > mCharMap;
};

#endif // __SHAREBASE_HH__

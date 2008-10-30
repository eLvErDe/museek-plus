/* Museek - Museek's 'core' library
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

#ifndef MUSEEK_SHARESDATABASE_H
#define MUSEEK_SHARESDATABASE_H

#include <NewNet/nnobject.h>
#include <NewNet/nnweakrefptr.h>
#include <string>
#include <vector>
#include <Muhelp/DirEntry.hh>

namespace Museek
{
class Museekd;
class SharesDatabase : public NewNet::Object {
public:
	SharesDatabase(Museekd * museekd);

	void load(const std::string& db, bool add = false);

	inline uint32 folders() const { return mNumFolders; }
	inline uint32 files() const { return mNumFiles; }

	bool is_shared(const std::string& path) const;
	std::string find_shared_nocase(const std::string& path) const;

	inline const std::vector<unsigned char>& shares() const { return mCompressed; }
	void search(const std::string& query, Folder& result);
	Shares folder_contents(const std::string& _f);

protected:
	void update( bool add = false );
	void recode( bool add = false );
	void update_flat();
	void update_compressed();
	void update_word_maps();

private:
	inline Folder fetch(const std::string& word) const;

	NewNet::WeakRefPtr<Museekd> mMuseekd;

	uint32 mNumFolders, mNumFiles;

	DirEntry mShares, mRecoded;
	Folder mFlat;

	std::vector<unsigned char> mCompressed;

	std::map<wchar_t, Shares> mCharMap;
};
}
#endif // MUSEEK_SHARESDATABASE_H

/* Museek - Museek's 'core' library
 *
 * Copyright (C) 2003-2007 Hyriand <hyriand@thegraveyard.org>
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

#ifndef MUSEEK_PEERMESSAGES_H
#define MUSEEK_PEERMESSAGES_H

#include "networkmessage.h"
#include <string>
#include <vector>
#include <queue>

namespace Museek
{
	class PeerSocket;
}

class PeerMessage : public NetworkMessage
{
public:
	void setPeerSocket(Museek::PeerSocket * peerSocket)
	{
		m_PeerSocket = peerSocket;
	}
	Museek::PeerSocket * peerSocket() const
	{
		return m_PeerSocket;
	}
private:
	Museek::PeerSocket * m_PeerSocket;
};
#define PEERMESSAGE(mtype, m_id) NETWORKMESSAGE(PeerMessage, mtype, m_id)


PEERMESSAGE(PSharesRequest, 4)
	MAKE
	END_MAKE

	PARSE
	END_PARSE
END

PEERMESSAGE(PSharesReply, 5)
	PSharesReply() { data = NULL; };
	PSharesReply(const std::vector<uchar>& _data) {
		data_len = _data.size();
		std::vector<uchar>::const_iterator it = _data.begin();
		int i = 0;
		data = new uchar[data_len];
		for(; it != _data.end(); ++it)
			data[i++] = *it;
	};

	~PSharesReply() {
		if(data)
			delete [] data;
	};

	MAKE
		for(uint i = 0; i < data_len; i++)
			pack(data[i]);
	END_MAKE

	PARSE
		decompress();
		uint n = unpack_int();
		while(n) {
			std::string dirname = unpack_string();
			uint f = unpack_int();
			Folder files;
			while(f) {
				unpack_char();
				std::string filename = unpack_string();
				FileEntry fe;
				fe.size = unpack_off();
				fe.ext = unpack_string();
				uint attrs = unpack_int();
				while(attrs) {
					unpack_int();
					fe.attrs.push_back(unpack_int());
					attrs--;
				}
				files[filename] = fe;
				f--;
			}
			shares[dirname] = files;
			n--;
		}
	END_PARSE

	uchar *data;
	uint data_len;
	Shares shares;
END

PEERMESSAGE(PSearchRequest, 8)
	PSearchRequest() {};
	PSearchRequest(uint _t, const std::string& _q) : query(_q) { ticket = _t; };

	MAKE
		pack(ticket);
		pack(query);
	END_MAKE

	PARSE
		ticket = unpack_int();
		query = unpack_string();
	END_PARSE

	std::string query;
	uint ticket;
END

PEERMESSAGE(PSearchReply, 9)
	PSearchReply() {};
	PSearchReply(uint _t, const std::string& _u, const Folder& _r, uint _spe, uint _que, bool _fre)
                    : user(_u), results(_r), ticket(_t), avgspeed(_spe), queuelen(_que), slotfree(_fre) {}

	MAKE
		pack(user);
		pack(ticket);
		pack((uint32)results.size());
		Folder::iterator it = results.begin();
		for(; it != results.end(); ++it) {
			pack((uchar)1);
			pack((*it).first, true);
			pack((*it).second.size);
			pack((*it).second.ext);
			pack((uint32)(*it).second.attrs.size());
			std::vector<uint>::iterator ait = (*it).second.attrs.begin();
			for(uint j = 0; ait != (*it).second.attrs.end(); ++ait) {
				pack(j++);
				pack(*ait);
			}
		}
		pack((uchar)slotfree);
		pack(avgspeed);
		pack(queuelen);

		compress();
	END_MAKE

	PARSE
		decompress();

		user = unpack_string();
		ticket = unpack_int();
		uint n = unpack_int();
		while(n) {
			buffer.seek(1);
			std::string fn = unpack_string();
			FileEntry fe;
			fe.size = unpack_off();
			fe.ext = unpack_string();
			int attrs = unpack_int();
			while(attrs) {
				unpack_int();
				fe.attrs.push_back(unpack_int());
				attrs--;
			}
			results[fn] = fe;
			n--;
		}
		slotfree = unpack_char();
		avgspeed = unpack_int();
		queuelen = unpack_int();
	END_PARSE

	std::string user;
	Folder results;
	uint ticket, avgspeed, queuelen;
	bool slotfree;
END

PEERMESSAGE(PInfoRequest, 15)
	PInfoRequest() {};
	MAKE
	END_MAKE

	PARSE
	END_PARSE
END

PEERMESSAGE(PInfoReply, 16)
	PInfoReply() {};
	PInfoReply(const std::string& _d, const std::vector<uchar>& _p, uint _t, uint _q, bool _s)
		: description(_d), picture(_p) { totalupl = _t; queuesize = _q; slotfree = _s; };
	MAKE
		pack(description);
		if (! picture.empty()) {
			pack((uchar)1);
			pack(picture);
		} else
			pack((uchar)0);
		pack(totalupl);
		pack(queuesize);
		pack((uint32)slotfree);
	END_MAKE

	PARSE
		description = unpack_string();
		bool has_picture = unpack_char();
		if (has_picture)
			picture = unpack_vector();
		totalupl = unpack_int();
		queuesize = unpack_int();
		slotfree = unpack_int();
	END_PARSE

	std::string description;
	std::vector<uchar> picture;
	bool slotfree;
	uint totalupl, queuesize;
END

PEERMESSAGE(PFolderContentsRequest, 36)
	PFolderContentsRequest() {};
	PFolderContentsRequest(const std::vector<std::string>& _d) : dirs(_d) {};
	PFolderContentsRequest(const std::string& _d) { dirs.push_back(_d); };

	MAKE
		pack((uint32)dirs.size());
		std::vector<std::string>::iterator it = dirs.begin();
		for(; it != dirs.end(); ++it)
			pack(*it);
	END_MAKE

	PARSE
		uint n = unpack_int();
		while(n) {
			dirs.push_back(unpack_string());
			n--;
		}
	END_PARSE

	std::vector<std::string> dirs;
END

PEERMESSAGE(PFolderContentsReply, 37)
	PFolderContentsReply() {};
	PFolderContentsReply(const Folders _f)
                           : folders(_f) {};

	MAKE
		pack((uint32)folders.size());
		Folders::iterator fit = folders.begin();
		for(; fit != folders.end(); ++fit) {
			pack((*fit).first);
			pack((uint32)(*fit).second.size());
			Shares::iterator dit = (*fit).second.begin();
			for(; dit != (*fit).second.end(); ++dit) {
				pack((*dit).first, true);
				pack((uint32)(*dit).second.size());
				Folder::iterator it = (*dit).second.begin();
				for(; it != (*dit).second.end(); ++it) {
					pack((uchar)1);
					pack((*it).first);
					pack((*it).second.size);
					pack((*it).second.ext);
					pack((uint32)(*it).second.attrs.size());
					std::vector<uint>::iterator ait = (*it).second.attrs.begin();
					for(uint j = 0; ait != (*it).second.attrs.end(); ++ait) {
						pack(j++);
						pack(*ait);
					}
				}
			}
		}

		compress();
	END_MAKE

	PARSE
		decompress();

		uint n = unpack_int();
		while(n) {
			std::string _folder = unpack_string();
			uint o = unpack_int();
			while(o) {
				std::string _dir = unpack_string();
				uint p = unpack_int();
				folders[_folder][_dir].clear();
				while(p) {
					FileEntry fe;
					unpack_char();
					std::string _name = unpack_string();
					fe.size = unpack_off();
					fe.ext = unpack_string();
					uint q = unpack_int();
					while(q) {
						unpack_int();
						fe.attrs.push_back(unpack_int());
						q--;
					}
					folders[_folder][_dir][_name] = fe;
					p--;
				}
				o--;
			}
			n--;
		}
	END_PARSE

	Folders folders;
END

PEERMESSAGE(PTransferRequest, 40)
	PTransferRequest() {};
	PTransferRequest(uint _t, const std::string& _f)
                            : filename(_f) { direction = 0; ticket = _t; };
	PTransferRequest(uint _t, const std::string& _f, off_t _s)
                            : filename(_f) { direction = 1; ticket = _t; filesize = _s; };

	MAKE
		pack(direction);
		pack(ticket);
		pack(filename);
		if (direction == 1)
			pack(filesize);
	END_MAKE

	PARSE
		direction = unpack_int();
		ticket = unpack_int();
		filename = unpack_string();
		if (direction == 1)
			filesize = unpack_off();
	END_PARSE

	uint direction, ticket;
	off_t filesize;
	std::string filename;
END

PEERMESSAGE(PUploadReply, 41)
	PUploadReply() { }
	PUploadReply(uint _t, const std::string& _r) : ticket(_t), reason(_r), allowed(false) { }
	PUploadReply(uint _t, off_t _f) : ticket(_t), filesize(_f), allowed(true) { }
	MAKE
		pack(ticket);
		if(allowed) {
			pack((uchar)1);
			pack(filesize);
		} else {
			pack((uchar)0);
			pack(reason);
		}
	END_MAKE

	uint ticket;
	std::string reason;
	off_t filesize;
	bool allowed;
END

PEERMESSAGE(PDownloadReply, 41)
	PDownloadReply() { }
	PDownloadReply(uint _t, bool _a, const std::string& _r = std::string()) : ticket(_t), allowed(_a), reason(_r) { }
	MAKE
		pack(ticket);
		pack((uchar)allowed);
		if(! allowed)
			pack(reason);
	END_MAKE

	uint ticket;
	bool allowed;
	std::string reason;
END

PEERMESSAGE(PTransferReply, 41)
	PTransferReply() { };

	PARSE
		ticket = unpack_int();
		allowed = unpack_char() == 1;
		if (buffer.count()) {
			if (allowed)
				filesize = unpack_off();
			else
				reason = unpack_string();
		}
	END_PARSE

	uint ticket;
	bool allowed;
	std::string reason;
	off_t filesize;

END

PEERMESSAGE(PUploadPlacehold, 42)
	PUploadPlacehold() { };
	PUploadPlacehold(const std::string& _f) : filename(_f) { }

	MAKE
		pack(filename);
	END_MAKE

	PARSE
		filename = unpack_string();
	END_PARSE

	std::string filename;
END


PEERMESSAGE(PQueueDownload, 43)
	PQueueDownload() { };
	PQueueDownload(const std::string& _f) : filename(_f) { };

	MAKE
		pack(filename);
	END_MAKE

	PARSE
		filename = unpack_string();
	END_PARSE

	std::string filename;
END

PEERMESSAGE(PPlaceInQueueReply, 44)
	PPlaceInQueueReply() { }
	PPlaceInQueueReply(const std::string& _f, uint32 _p) : filename(_f), place(_p) { }

	MAKE
		pack(filename);
		pack(place);
	END_MAKE

	PARSE
		filename = unpack_string();
		place = unpack_int();
	END_PARSE

	std::string filename;
	uint32 place;
END

PEERMESSAGE(PUploadFailed, 46)
	PUploadFailed() { };
	PUploadFailed(const std::string& _f) : filename(_f) { }

	MAKE
		pack(filename);
	END_MAKE

	PARSE
		filename = unpack_string();
	END_PARSE

	std::string filename;
END

PEERMESSAGE(PQueueFailed, 50)
	PQueueFailed() { };
	PQueueFailed(const std::string& _f, const std::string& _r) : filename(_f), reason(_r) { }

	MAKE
		pack(filename);
		pack(reason);
	END_MAKE

	PARSE
		filename = unpack_string();
		reason = unpack_string();
	END_PARSE

	std::string filename, reason;
END

PEERMESSAGE(PPlaceInQueueRequest, 51)
	PPlaceInQueueRequest() { }
	PPlaceInQueueRequest(const std::string& _f) : filename(_f) { }

	MAKE
		pack(filename);
	END_MAKE

	PARSE
		filename = unpack_string();
	END_PARSE

	std::string filename;
END


PEERMESSAGE(PUploadQueueNotification, 52)
	PUploadQueueNotification() { }

	MAKE
	END_MAKE

	PARSE
	END_PARSE
END


#endif // MUSEEK_PEERMESSAGES_H

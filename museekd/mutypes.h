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

#ifndef MUSEEK_MUTYPES_H
#define MUSEEK_MUTYPES_H

#include <map>
#include <string>
#include <vector>
#include <stdint.h>
#include <sys/types.h>

typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int32_t int32;
typedef unsigned int uint;
typedef unsigned char uchar;

struct _UserData {
  uint32 status, avgspeed, files, dirs;
  uint64 downloadnum;
  bool slotsfull;
  std::string country;
};

enum _TrState {
  TS_Finished = 0,
  TS_Transferring,
  TS_Negotiating,
  TS_Waiting,
  TS_Establishing,
  TS_Initiating,
  TS_Connecting,
  TS_QueuedRemotely,
  TS_Address_UNUSED,
  TS_Status_UNUSED,
  TS_Offline,
  TS_ConnectionClosed,
  TS_CannotConnect,
  TS_Aborted,
  TS_RemoteError,
  TS_LocalError,
  TS_QueuedLocally
};

enum _BaseConnState {
  BS_Unknown = 0,
  BS_Error,
  BS_Status,
  BS_Address,
  BS_Connecting,
  BS_Waiting,
  BS_Connected,
  BS_Disconnected
};

struct _FolderRequest {
  enum _TrState state;
  std::string user;
  std::string folder;
};

struct _FileEntry {
  uint64 size;
  std::string ext;
  std::vector<uint32> attrs;
};

typedef std::map<std::string, std::string> StringMap;
typedef std::vector<std::string> StringList;
typedef std::vector<std::wstring> WStringList;

typedef struct _UserData UserData;
typedef enum _TrState TrState;
typedef enum _BaseConnState BaseConnState;

typedef struct _FileEntry FileEntry;

typedef struct _FolderRequest FolderRequest;
typedef StringMap Tickers;
typedef std::map<std::string, std::wstring> WTickers;
typedef std::map<std::string, int32> Recommendations;
typedef std::map<std::string, uint32> SimilarUsers;
typedef std::map<std::string, std::pair<std::string, uint32> > NetInfo;
typedef std::map<std::string, uint32> RoomList;
typedef std::map<std::string, std::pair<uint32, uint32> > PrivRoomList;
typedef std::map<std::string, UserData> RoomData;
typedef std::map<std::string, std::vector<std::string> > PrivRoomOperators;
typedef std::map<std::string, std::string> PrivRoomOwners;

typedef std::map<std::string, FileEntry> Folder;
typedef std::map<std::wstring, FileEntry> WFolder;

typedef std::map<std::string, Folder> Shares;
typedef std::map<std::wstring, WFolder> WShares;

typedef std::map<std::string, Shares> Folders;
typedef std::map<std::wstring, WShares> WFolders;

#endif // MUSEEK_MUTYPES_H

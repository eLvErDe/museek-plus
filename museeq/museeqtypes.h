/* museeq - a Qt client to museekd
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

#ifndef MUSEEQTYPES_H
#define MUSEEQTYPES_H

#include <qobject.h>

typedef struct NFileData {
	Q_INT64 size;
	uint length;
	uint bitrate;
	bool vbr;
};

typedef struct NUserData {
	uint status;
	uint speed;
	uint files;
};

typedef struct NTransfer {
	QString user;
	QString filename;
	uint placeInQueue, state;
	QString error;
	Q_INT64 filepos, filesize;
	uint rate;
};

typedef QMap<QString, NFileData> NFolder;
typedef QMap<QString, NFolder> NShares;

typedef QMap<QString, NUserData> NRoom;
typedef QMap<QString, NRoom> NRooms;
typedef QMap<QString, uint> NRoomList;
typedef QMap<QString, uint> NGlobalRecommendations;
typedef QMap<QString, uint> NRecommendations;
typedef QMap<QString, uint> NItemRecommendations;
typedef QMap<QString, uint> NItemSimilarUsers;
typedef QMap<QString, uint> NSimilarUsers;

typedef QValueList<NTransfer> NTransfers;
typedef QMap<QString, QString> NTickers;
typedef QMap<QString, NTickers> NTickerMap;

#endif // MUSEEQTYPES_H

/* museeq - a Qt client to museekd
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

#ifndef MUSEEQTYPES_H
#define MUSEEQTYPES_H

#include <QList>
#include <QMap>
#include <QString>
#include <QPair>

typedef struct {
	qint64 size;
	uint length;
	uint bitrate;
	bool vbr;
} NFileData;

typedef struct {
	uint status, speed, downloadnum, files, dirs;
	bool slotsfull;
	QString country;
} NUserData;

typedef struct {
	QString user;
	QString filename;
	uint placeInQueue, state;
	QString error;
	qint64 filepos, filesize;
	uint rate;
} NTransfer;

typedef QMap<QString, NFileData> NFolder;
typedef QMap<QString, NFolder> NShares;

typedef QMap<QString, NUserData> NRoom;
typedef QMap<QString, NRoom> NRooms;
typedef QMap<QString, uint> NRoomList;
typedef QMap<QString, QString> NPrivRoomOwners;
typedef QMap<QString, QStringList> NPrivRoomOperators;
typedef QMap<QString, QPair<uint, uint> > NPrivRoomList;
typedef QMap<QString, int> NGlobalRecommendations;
typedef QMap<QString, int> NRecommendations;
typedef QMap<QString, int> NItemRecommendations;
typedef QMap<QString, uint> NItemSimilarUsers;
typedef QMap<QString, uint> NSimilarUsers;

typedef QList<NTransfer> NTransfers;
typedef QMap<QString, QString> NTickers;
typedef QMap<QString, NTickers> NTickerMap;

#endif // MUSEEQTYPES_H

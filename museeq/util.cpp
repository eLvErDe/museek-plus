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

#include "util.h"

QString Util::makeSize(Q_INT64 i) {
	double d = i;
	if(d < 1000)
		return QString().sprintf("%.2f B", d);
	d /= 1024;
	if(d < 1024)
		return QString().sprintf("%.2f KiB", d);
	d /= 1024;
	if(d < 1024)
		return QString().sprintf("%.2f MiB", d);
	d /= 1024;
	if(d < 1024)
		return QString().sprintf("%.2f GiB", d);
	d /= 1024;
	if(d < 1024)
		return QString().sprintf("%.2f TiB", d);
	d /= 1024;
	return QString().sprintf("%.2f PB", d);
}

int Util::cmp(Q_INT64 a, Q_INT64 b) {
	if(a > b)
		return 1;
	if(a == b)
		return 0;
	return -1;
}

QString Util::makeTime(uint length) {
	if(! length)
		return QString::null;
	else
		if(length < 3600)
			return QString().sprintf("%i:%02i", length / 60, length % 60);
		else
			return QString().sprintf("%i:%02i:%02i", length / 3600, (length / 60) % 60, length % 60);
}

QString Util::makeBitrate(uint bitrate, bool vbr) {
	if(! bitrate)
		return QString::null;
	else
		if(vbr)
			return QString("(vbr) %1").arg(bitrate);
		else
			return QString::number(bitrate);
}

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

#ifndef SLSKDRAG_H
#define SLSKDRAG_H

#include <qdragobject.h>

class SlskDrag : public QUriDrag {
	Q_OBJECT
public:
	SlskDrag(QWidget*);
	void append(const QString&, const QString& = QString::null, Q_INT64 = 0);
	
	int count() const;
	
	static bool canDecode(const QMimeSource*, bool = false);
	static bool decode(const QMimeSource*, QStringList&);
	
protected:
	QStringList mUris;
};


#endif // SLSKDRAG_H

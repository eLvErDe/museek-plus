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

#ifndef USERMENU_H
#define USERMENU_H

#include <qpopupmenu.h>

class Usermenu : public QPopupMenu {
	Q_OBJECT
public:
	Usermenu(QWidget* = 0, const char* = 0);
	QString user() const;
	void setup(const QString&);
	
public slots:
	void exec(const QString&);
	void exec(const QString&, const QPoint&);

protected slots:
	void slotActivated(int id);
	
private:
	QString mUser;
};

#endif // USERMENU_H

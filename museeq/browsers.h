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

#ifndef BROWSERS_H
#define BROWSERS_H

#include "museeqtypes.h"
#include "usertabwidget.h"

class Browsers : public UserTabWidget {
	Q_OBJECT
public:
	Browsers(QWidget* = 0, const char* = 0);

public slots:
	void setShares(const QString&, const NShares&);

protected:
	UserWidget* makeNewPage(const QString&);

protected slots:
	void closeCurrent();
};

#endif // BROWSERS_H

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

#include "browsers.h"
#include "browser.h"
#include "museeq.h"

Browsers::Browsers(QWidget* parent, const char* name)
          : UserTabWidget(parent, name) {

	connect(museeq, SIGNAL(userShares(const QString&, const NShares&)), SLOT(setShares(const QString&, const NShares&)));
}

UserWidget* Browsers::makeNewPage(const QString& user) {
	UserWidget* w = new Browser(user, this);
	museeq->getUserShares(user);
	return w;
}

void Browsers::setShares(const QString& user, const NShares& shares) {
	Browser* browser = dynamic_cast<Browser*>(page(user));
	if(! browser)
		return;
	browser->setShares(shares);
}

void Browsers::closeCurrent() {
    Browser * page = dynamic_cast<Browser*>(currentWidget());
    if (!page->isLoading())
        TabWidget::closeCurrent();
}

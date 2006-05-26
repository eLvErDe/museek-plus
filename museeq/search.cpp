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

#include "search.h"

#include <qcombobox.h>
#include <qlistview.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlabel.h>

#include "searchlistview.h"
#include "searchfilter.h"
#include "museeq.h"

Search::Search(const QString& query, QWidget* parent, const char* name)
       : QVBox(parent, name), mQuery(query) {
	
	setSpacing(5);
	
	QHBox* box = new QHBox(this);
	box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	mShowFilters = new QCheckBox("Enable filters", box);
	(new QWidget(box))->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	new QPushButton("Ignore", box);
	
	mFilters = new SearchFilter(this);
	
	mResults = new SearchListView(mFilters, this);
	
	connect(mShowFilters, SIGNAL(toggled(bool)), mFilters, SLOT(setShown(bool)));
	mFilters->hide();
	
	connect(mFilters, SIGNAL(filterChanged()), SLOT(refilter()));
}

Search::~Search() {
	QValueList<uint>::const_iterator it = mTokens.begin();
	for(; it != mTokens.end(); ++it)
		museeq->terminateSearch(*it);
}

QString Search::query() const {
	return mQuery;
}

void Search::setToken(uint token) {
	mTokens << token;
}

bool Search::hasToken(uint token) const {
	return mTokens.find(token) != mTokens.end();
}

void Search::append(const QString& u, bool f, uint s, uint q, const NFolder& r) {
	mResults->append(u, f, s, q, r);
	emit highlight(1);
}

void Search::refilter() {
	mFilters->refilter(mResults);
}

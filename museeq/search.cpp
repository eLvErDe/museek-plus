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

#include "searchlistview.h"
#include "searchfilter.h"
#include "museeq.h"
#include "search.h"

#include <QCheckBox>
#include <QPushButton>
#include <QList>
#include <QLayout>

Search::Search(const QString& query, QWidget* parent, const char* name)
       : QWidget(parent), mHighlight(0), mQuery(query) {

	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	QVBoxLayout* MainLayout = new QVBoxLayout(this);
	QHBoxLayout* ButtonLayout = new QHBoxLayout;
	MainLayout->addLayout(ButtonLayout);
	mShowFilters = new QCheckBox(tr("Enable filters"), this);
	ButtonLayout->addWidget(mShowFilters);
	mIgnore = new QPushButton(tr("Stop search"), this);
	ButtonLayout->addWidget(mIgnore);

	mFilters = new SearchFilter(this);
	MainLayout->addWidget(mFilters);


	mResults = new SearchListView(mFilters, this);
	MainLayout->addWidget(mResults);

	connect(mShowFilters, SIGNAL(toggled(bool)), mFilters, SLOT(setShown(bool)));
	mFilters->hide();

	connect(mFilters, SIGNAL(filterChanged()), SLOT(refilter()));
	connect(mIgnore, SIGNAL(clicked()), SLOT(ignoreSearch()));
}

Search::~Search() {
	QList<uint>::const_iterator it = mTokens.begin();
	for(; it != mTokens.end(); ++it)
		museeq->terminateSearch(*it);
}

QString Search::query() const {
	return mQuery;
}

void Search::setToken(uint token) {
	mTokens.push_back( token);
	mIgnore->setDisabled(false);
}

bool Search::hasToken(uint token) const {
	return (mTokens.indexOf(token) != -1);
}

void Search::append(const QString& u, bool f, uint s, uint q, const NFolder& r) {
	mResults->append(u, f, s, q, r);
	emit highlight(1, this);
}

void Search::ignoreSearch() {
	mTokens.clear();
	mIgnore->setDisabled(true);
}

void Search::refilter() {
	mFilters->refilter(mResults);
}

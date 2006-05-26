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

#ifndef SEARCH_H
#define SEARCH_H

#include "museeqtypes.h"

#include <qvbox.h>
#include <qdict.h>

class QHBox;
class QComboBox;
class QCheckBox;
class SearchListView;
class SearchFilter;
        
class Search : public QVBox {
	Q_OBJECT
public:
	Search(const QString&, QWidget* = 0, const char* = 0);
	~Search();
	
	QString query() const;
	bool hasToken(uint) const;
	
signals:
	void highlight(int);
	
public slots:
	void setToken(uint);
	void append(const QString&, bool, uint, uint, const NFolder&);
	
protected slots:
	void refilter();
	
private:
	QString mQuery;
	QValueList<uint> mTokens;
	
	QCheckBox* mShowFilters;
	SearchFilter* mFilters;
	SearchListView* mResults;
};

#endif // SEARCH_H

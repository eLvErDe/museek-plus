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

#ifndef ACLINEEDIT_H
#define ACLINEEDIT_H

#include <QLineEdit>

class ACLineEdit : public QLineEdit {
	Q_OBJECT
public:
	ACLineEdit(QWidget * parent = 0, const char * name = 0);

	void reset();

	void pushCompletor(const QString& i);
	void setCompletors(const QStringList &l);
	void addCompletor(const QString& text);
	void removeCompletor(const QString& text);

protected slots:
	void autoComplete();

protected:
	void keyPressEvent(QKeyEvent *e);

private:
	QStringList mCompletors, mMatches;
	QString mLeft, mRight;
};

#endif // ACLINEEDIT_H

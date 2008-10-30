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

#include "aclineedit.h"

#include <QStringList>
#include <QShortcut>

ACLineEdit::ACLineEdit(QWidget * parent, const char * name)
           : QLineEdit(name, parent)
{
	new QShortcut(Qt::Key_Tab, this, SLOT(autoComplete()));
}

void
ACLineEdit::pushCompletor(const QString& t)
{
	mCompletors.removeAt(mCompletors.indexOf(t));
	mCompletors.prepend(t);
}

void
ACLineEdit::autoComplete()
{
	if(mMatches.size()) {
		QString t = mMatches.front();
		mMatches.pop_front();
		mMatches.append(t);

		pushCompletor(t);

		if(mLeft.isEmpty()) {
			setText(t + ": " + mRight);
			setCursorPosition(t.length() + 2);
		} else {
			setText(mLeft + t + mRight);
			setCursorPosition(mLeft.length() + t.length());
		}
		return;
	}

	QString _text = text(),
		t = _text.left(cursorPosition());
	int ix = t.lastIndexOf(' ') + 1;
	t = t.mid(ix);

	QStringList::iterator it, end = mCompletors.end();
	for(it = mCompletors.begin(); it != end; it ++)
		if((*it).startsWith(t, Qt::CaseInsensitive))
			mMatches << *it;

	mLeft = _text.left(ix);
	mRight = _text.mid(ix + t.length());

	if(mMatches.size())
		autoComplete();
}

void
ACLineEdit::setCompletors(const QStringList &l)
{
	mCompletors = l;
}

void
ACLineEdit::addCompletor(const QString& text)
{
	if(! mCompletors.contains(text))
		mCompletors << text;
}

void
ACLineEdit::removeCompletor(const QString& text)
{
	mMatches.removeAll(text);
	mCompletors.removeAll(text);
}

void
ACLineEdit::keyPressEvent(QKeyEvent *e)
{
	mMatches.clear();
	QLineEdit::keyPressEvent(e);
}

void
ACLineEdit::reset()
{
	setText(QString::null);
	mMatches.clear();
}

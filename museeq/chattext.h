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

#ifndef CHATTEXT_H
#define CHATTEXT_H

#include <QTextBrowser>

class ChatText : public QTextBrowser {
	Q_OBJECT
public:
	ChatText(const QString& tf, QWidget* = 0);
	QString colorRemote, colorNickname, colorMe, colorBuddy, colorBanned, colorTime, fontTime, fontMessage;

public slots:
	void append(const QString&, const QString&);
	void append(const QString&, const QString&, const QString&);
	void append(uint, const QString&, const QString&);
	void append(uint, const QString&, const QString&, const QString&);
	void append(uint, const QString&);
	void setSource(const QString&) { };
	void backward() { };
	void forward() { };
	void home() { };
	void reload() { };

protected slots:
	void setNickname(const QString&);

protected:
	QString postProcess(const QString&, const QString&, const QString&);

private:
	QString mTimeFormat, mNickname;
};

#endif // CHATTEXT_H

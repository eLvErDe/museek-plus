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

#ifndef CHATPANEL_H
#define CHATPANEL_H

#include <QLayout>
#include <QWidget>

class ChatText;
class ACLineEdit;

class ChatPanel : public QWidget {
	Q_OBJECT
public:
	ChatPanel(const QString&, QWidget* = 0);

	inline QWidget* box() const { return mBox; };
	inline QHBoxLayout* boxLayout() const { return BoxLayout; };
	inline ACLineEdit* entry() const { return mEntry; }

public slots:
	void append(const QString&, const QString&);
	void append(uint, const QString&, const QString&);
	void append(uint, const QString&);
	void clear();
signals:
	void send(const QString&);

protected:
	void showEvent(QShowEvent*);

protected slots:
	void slotSendMessage();
	void slotConnected();
	void slotDisconnected();

private:
	ChatText* mScroll;
	ACLineEdit* mEntry;
	QHBoxLayout* BoxLayout;
	QWidget* mBox;
};


#endif // CHATPANEL_H

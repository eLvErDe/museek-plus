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

#ifndef SCRIPT_H
#define SCRIPT_H

#include <QObject>
#include <QList>
#ifdef HAVE_QTSCRIPT
    #include <QtScript>
    #ifdef HAVE_QTUITOOLS
        #include <QtUiTools>
        #include <QUiLoader>
    #endif // HAVE_QTUITOOLS
#endif // HAVE_QTSCRIPT

class QMenu;

class Script : public QObject {
	Q_OBJECT

public:
	Script(const QString& script, const QString&);
	~Script();

    void init();

	void reportScriptError();

	QString scriptName();

	QString handleInput(bool, const QString&, QString);

	Q_INVOKABLE QScriptValue loadUI(const QString&);
	Q_INVOKABLE QString path();
	Q_INVOKABLE QVariant config(const QString&);
	Q_INVOKABLE QVariant launchProcess(const QString&);

public slots:

	void registerMenu(const QString&, QMenu *);
	void addMenu(const QString&, const QString&, const QString&);
	void addInputHandler(const QString&);
	void showWarning(const QString&);
	void setConfig(const QString&, const QVariant&);
	void sayRoom(const QString&);

protected slots:
	void slotMenuActivated(QAction*);

private:
#ifdef HAVE_QTSCRIPT
    bool mInitiated;
    QString mScriptName;
    QString mPath; // Path where the script and all its needed files are stored
	QMap<QString, QPair<QMenu*, QList<QAction*> > > mMenus; // QString: menu name, QMenu: menu object, QAction: script's actions in this menu
	QScriptEngine* mEngine; // Engine for this script
	QScriptValue mSelf; //
	QMap<int, QString> mCallbacks; // List of callbacks : <menu id, callback function name>
	QStringList mInputHandlers; // Script functions that will be called when we send some text (room/private) (QString is function name.)
#endif //HAVE_QTSCRIPT
};

#endif // SCRIPT_H

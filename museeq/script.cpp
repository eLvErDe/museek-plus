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

#include "script.h"
#include "mainwin.h"
#include "museeq.h"
#include "chatrooms.h"

#include <QMenuBar>
#include <QMessageBox>

/**
  * Constructor: open the script file and see if it's ok.
  */
Script::Script(const QString& script, const QString& path)
      : QObject(0) {
#ifdef HAVE_QTSCRIPT
    mPath = path;

    mInitiated = false;

	mEngine = new QScriptEngine(this);
	QScriptValue global = mEngine->globalObject();
	mSelf = mEngine->newQObject( this );
    mSelf.setScope( global );
    global.setProperty("muscript", mSelf);

	QScriptValue museeqSV = mEngine->newQObject( museeq );
    global.setProperty("museeq", museeqSV);

	mEngine->evaluate(script);
	if ( mEngine->hasUncaughtException()) {
        reportScriptError();
        return;
	}
#endif // HAVE_QTSCRIPT
}

/**
  * Destructor.
  */
Script::~Script()
{
#ifdef HAVE_QTSCRIPT
	/* Call script destructor */
    QScriptValue fun = mSelf.property("destroy");
    if ( !fun.isFunction() )
        qDebug() << "Script: destroy is not a function, " << fun.toString();

    QScriptContext *ctx = mEngine->pushContext();
    ctx->setActivationObject( mSelf );
    QScriptValue result = fun.call( mSelf );
    mEngine->popContext();

    if ( mEngine->hasUncaughtException() )
        reportScriptError();

    if (result.toBoolean())
        return;

    /* Erase menu entries and empty menus */
    QMap<QString, QPair<QMenu*, QList<QAction*> > >::iterator it = mMenus.begin();
    for(; it != mMenus.end(); ++it)
    {
        QMenu* m = it.value().first;
        // Remove this script's actions from menu
        QList<QAction*>::iterator ait = it.value().second.begin();
        for (; ait != it.value().second.end();) {
            m->removeAction(*ait);
            delete *ait;
            it.value().second.erase(ait++);
        }

        // Remove the menu if it is empty
        if (m->isEmpty())
            delete m;
    }
#endif // HAVE_QTSCRIPT
}

/**
  * Launches the script. Call it once after creating the Script object.
  */
void Script::init() {
#ifdef HAVE_QTSCRIPT
    QScriptValue fun = mSelf.property("init");
    if ( !fun.isFunction() )
        qDebug() << "Script: init is not a function, " << fun.toString();

    QScriptContext *ctx = mEngine->pushContext();
    ctx->setActivationObject( mSelf );
    QScriptValue name = fun.call( mSelf );
    mEngine->popContext();

    if ( mEngine->hasUncaughtException() ) {
        reportScriptError();
        mScriptName = "";
    }
    else
        mScriptName = name.toString();

	if(mScriptName.isEmpty())
		mScriptName = tr("Unknown script");

    qDebug() << "Adding new script: " << mScriptName;
    mInitiated = true;
#endif // HAVE_QTSCRIPT
}

/**
  * Debugging: print script errors
  */
void Script::reportScriptError() {
#ifdef HAVE_QTSCRIPT
    qDebug() << "Error in script: " << mEngine->uncaughtException().toString()
                << " at line " << mEngine->uncaughtExceptionLineNumber() << endl;
    qDebug() << mEngine->uncaughtExceptionBacktrace();
#endif // HAVE_QTSCRIPT
}

/**
  * Returns the name of this script.
  */
QString Script::scriptName() {
    #ifdef HAVE_QTSCRIPT
        return mScriptName;
    #endif //HAVE_QTSCRIPT
    return QString();
}

/**
  * Returns the path of this script.
  */
Q_INVOKABLE QString Script::path() {
    #ifdef HAVE_QTSCRIPT
        return mPath;
    #endif //HAVE_QTSCRIPT
    return QString();
}

/**
  * Call this from script when you want to add a new menu item.
  * menu is the name of the menu where the item should be placed (File, ...). This can be a new menu.
  * item is the name of the new menu item
  * callback is the name of the script function that should be called when the menu item is clicked.
  * Every menu item added by a script will be erased when unloading the script
  */
void Script::addMenu(const QString& menu, const QString& item, const QString& callback) {
#ifdef HAVE_QTSCRIPT
	QMenu * m;
	QAction * action;
	QMap<QString, QPair<QMenu*, QList<QAction*> > >::iterator it = mMenus.find(menu);
	if(it == mMenus.end()) {
	    // This menu doesn't exist yet: create it
		m = museeq->mainwin()->menuBar()->addMenu(menu);
		museeq->registerMenu(menu, m);
	} else
		m = it.value().first; // Found the menu where we want to add an action

    // Create the action, add it to the menu and register it
    action = m->addAction(item);
    int id = museeq->scriptCallbackId();
    action->setData(id);
    mCallbacks[id] = callback;
    mMenus[menu].second.push_back(action);
#endif // HAVE_QTSCRIPT
}

/**
  * Store this new QMenu.
  */
void Script::registerMenu(const QString& title, QMenu* menu) {
#ifdef HAVE_QTSCRIPT
	mMenus[title] = QPair<QMenu*, QList<QAction*> >(menu, QList<QAction*>());
	connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(slotMenuActivated(QAction*)));
#endif // HAVE_QTSCRIPT
}

/**
  * This method is called when a menu defined by this script is clicked.
  */
void Script::slotMenuActivated(QAction* action) {
#ifdef HAVE_QTSCRIPT
    int id = action->data().toInt();
	QMap<int, QString>::iterator it = mCallbacks.find(id);
	if(it != mCallbacks.end())
	{
        // Call script callback
        QScriptValue fun = mSelf.property(it.value());
        if ( !fun.isFunction() ) {
            qDebug() << "Script: menu callback " << it.value() << " is not a function, " << fun.toString();
            return;
        }

        QScriptContext *ctx = mEngine->pushContext();
        ctx->setActivationObject( mSelf );
        QScriptValue result = fun.call( mSelf );
        mEngine->popContext();

        if ( mEngine->hasUncaughtException() )
            reportScriptError();
	}
#endif // HAVE_QTSCRIPT
}

/**
  * Add a new input handler for this script.
  * An input handler is a callback script function that will be called everytime the user sends a message in room
  * or private chat.
  */
void Script::addInputHandler(const QString& callback) {
#ifdef HAVE_QTSCRIPT
	mInputHandlers.push_back(callback);
#endif // HAVE_QTSCRIPT
}

/**
  * Pass the given line to any input handler defined by this script.
  */
QString Script::handleInput(bool privateMessage, const QString& target, QString line) {
#ifdef HAVE_QTSCRIPT
    QStringList::iterator it, end = mInputHandlers.end();
    for(it = mInputHandlers.begin(); it != end; ++it)
    {
        QScriptValueList args;
        args << QScriptValue(mEngine, privateMessage) << QScriptValue(mEngine, target) << QScriptValue(mEngine, line);
        // Call script handler
        QScriptValue fun = mSelf.property(*it);
        if ( !fun.isFunction() ) {
            qDebug() << "Script: input handler " << *it << " is not a function, " << fun.toString();
            break;
        }

        QScriptContext *ctx = mEngine->pushContext();
        ctx->setActivationObject( mSelf );
        QScriptValue result = fun.call( mSelf, args );
        mEngine->popContext();

        if ( mEngine->hasUncaughtException() )
            reportScriptError();

        QString newline = result.toString();

        if (!newline.isEmpty())
            line = newline;
    }
#endif // HAVE_QTSCRIPT
	return line;
}

/**
  * Use this method in scripts to show a warning MessageBox to user
  */
void Script::showWarning(const QString& msg) {
#ifdef HAVE_QTSCRIPT
    QMessageBox::warning(NULL, tr("Warning in ") + scriptName() + tr(" script"), msg, QMessageBox::Ok, QMessageBox::NoButton);
#endif // HAVE_QTSCRIPT
}

/**
  * Call this from a script to show the dialog defined in the given .ui file
  */
Q_INVOKABLE QScriptValue Script::loadUI(const QString& file) {
#ifdef HAVE_QTSCRIPT
    #ifdef HAVE_QTUITOOLS
    QUiLoader loader;
    QFile uiFile(file);
    QWidget *ui;
    QScriptValue scriptUi;
    if (uiFile.open(QIODevice::ReadOnly)) {
        ui = loader.load(&uiFile);
        uiFile.close();

        scriptUi = mEngine->newQObject(ui);
    }
    return scriptUi;
    #endif // HAVE_QTUITOOLS
#endif // HAVE_QTSCRIPT
    return QScriptValue();
}

/**
  * Scripts use this to store their data.
  */
void Script::setConfig(const QString& key, const QVariant& value) {
#ifdef HAVE_QTSCRIPT
    if (mInitiated)
        museeq->settings()->setValue("scripts/" + scriptName() + "/" + key, value);
#endif // HAVE_QTSCRIPT
}

/**
  * Scripts use this to access their stored data.
  */
Q_INVOKABLE QVariant Script::config(const QString& key) {
#ifdef HAVE_QTSCRIPT
    if (mInitiated)
        return museeq->settings()->value("scripts/" + scriptName() + "/" + key);
#endif // HAVE_QTSCRIPT
    return QVariant();
}

/**
  * This will write the given msg inside the current room if any
  */
void Script::sayRoom(const QString& msg) {
#ifdef HAVE_QTSCRIPT
    QString room = museeq->mainwin()->chatRooms()->getCurrentPage();
    if (!room.isEmpty())
        museeq->sayRoom(room, msg);
#endif // HAVE_QTSCRIPT
}

/**
  * Scripts use this to launch other processes.
  * This method is blocked while performing the command.
  */
Q_INVOKABLE QVariant Script::launchProcess(const QString& cmd) {
#ifdef HAVE_QTSCRIPT
    QProcess proc;
    proc.start(cmd);

    if (!proc.waitForFinished())
       return false;

    QString result = proc.readAll();
    result = result.left(result.length() - 1);

    return result;
#endif // HAVE_QTSCRIPT
    return QVariant();
}

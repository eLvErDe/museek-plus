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

#include "museeq.h"
#include "codeccombo.h"

static QString _list[] = {
	"UTF-8",
	"UTF-7",
	"UTF-16",
	"UTF-32",
	"KOI8-R",
	"ISO8859-1",
	"ISO8859-2",
	"ISO8859-3",
	"ISO8859-4",
	"ISO8859-5",
	"ISO8859-6",
	"ISO8859-7",
	"ISO8859-8",
	"ISO8859-9",
	"ISO8859-10",
	"ISO8859-11",
	"ISO8859-13",
	"ISO8859-14",
	"ISO8859-15",
	"ISO8859-16",
	"CP1250",
	"CP1251",
	"CP1252",
	"CP1253",
	"CP1254",
	"CP1255",
	"CP1256",
	"CP1257",
	"CP1258",
	"CP874",
	"Big5",
	"EUC-CN",
	"EUC-KR",
	"EUC-JP",
	"Shift-JIS",
	QString::null // Empty string, important!
};

CodecCombo::CodecCombo(const QString& _d, const QString& _k, QWidget* _p, const char* _n)
           : QComboBox(_p), mDomain(_d), mKey(_k)
{
	QStringList l;

	QString *charset = _list;
	while(! charset->isEmpty()) {
		l.append(*charset);
		charset++;
	}

	addItems(l);
	setEditable(false);

	QString _i = museeq->config(_d, _k);
	if(_i.isEmpty())
		_i = museeq->config("encoding", "network");

	setCharset(_i);

	connect(museeq, SIGNAL(configChanged(const QString&, const QString&, const QString&)), SLOT(slotConfigChanged(const QString&, const QString&, const QString&)));
	connect(this, SIGNAL(activated(const QString&)), SLOT(slotActivated(const QString&)));
}

void
CodecCombo::slotActivated(const QString& charset)
{
	if(charset != museeq->config(mDomain, mKey))
		museeq->setConfig(mDomain, mKey, charset);
}

void
CodecCombo::slotConfigChanged(const QString& domain, const QString& key, const QString& value)
{
	if(domain == mDomain && key == mKey)
		setCharset(value);
}

void
CodecCombo::setCharset(const QString& charset)
{
	int c = count();
	int pos = findText ( charset,  Qt::MatchExactly);
	if (pos != -1)
	{
		setCurrentIndex(pos);
		return;
	}
	addItem(charset);
	setCurrentIndex(c);
}

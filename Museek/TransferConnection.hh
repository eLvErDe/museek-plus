/* Museek - Museek's 'core' library
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

#ifndef __DOWNLOADCONNECTION_HH__
#define __DOWNLOADCONNECTION_HH__

#include <Museek/BaseConnection.hh>

class Transfer;
class Museek;

class TransferPreConnection : public BaseConnection {
public:
	TransferPreConnection(Peer* peer, uint32 token);
	
protected:
	void process();
};

class DownloadConnection : public BaseConnection {
public:
	DownloadConnection(Transfer* transfer);
	DownloadConnection(Transfer* transfer, uint32 token);
	
	void terminate() { mTransfer = 0; }
	
protected:
	void disconnected();
	void connected();
	void process();
	void cannot_connect();
	
private:
	Transfer* mTransfer;
	bool mSendTicket;
};

class UploadConnection : public BaseConnection {
public:
	UploadConnection(Transfer* transfer);
	UploadConnection(Transfer* transfer, uint32 token);
	
	void terminate() { mTransfer = 0; }
	
protected:
	void disconnected();
	void connected();
	void process();
	void cannot_connect();
	void data_sent(unsigned int);
	
private:
	Transfer* mTransfer;
	bool mSendTicket;
	bool mHavePos;
	
};

#endif // __DOWNLOADCONNECTION_HH__

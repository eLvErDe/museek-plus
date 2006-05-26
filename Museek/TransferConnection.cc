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

#include <Museek/mu.hh>

#include <Museek/TransferConnection.hh>
#include <Museek/TransferManager.hh>
#include <Museek/PeerManager.hh>
#include <Museek/PeerConnection.hh>

#include <vector>
using std::vector;

#define MULOG_DOMAIN "Museek.TP"
#include <Muhelp/Mulog.hh>

TransferPreConnection::TransferPreConnection(Peer* peer, uint32 token)
                      : BaseConnection(0, "F", peer, token) {
	
	DEBUG("TransferPreConnection");
}

void TransferPreConnection::process() {
	DEBUG("process");
	
	if(inbuf.size() < 4)
		return;
	
	uint32 ticket = 0;
	for(uint i = 0; i < 4; i++) {
		ticket += inbuf.front() << (i * 8);
		inbuf.pop();
	}
	
	DEBUG("ticket: %u", ticket);
	
	const std::vector<Transfer*>* transfers = peer()->transfers();
	vector<Transfer*>::const_iterator it = transfers->begin();
	for(; it != transfers->end(); ++it) {
		Transfer* transfer = *it;
		if(transfer->state() == TS_Establishing && transfer->ticket() == ticket) {
			DEBUG("found transfer: %s, %s", transfer->peer()->user().c_str(), transfer->path().c_str());
			transfer->invalidate_ticket();
			transfer->connection()->set_socket(sock, inbuf);
			is_disconnected = true;
			sock = -1;
			error = ERR_DISCONNECTED;
			return;
		}
	}
	DEBUG("couldn't find transfer for ticket %u", ticket);
	disconnect();
}

#undef MULOG_DOMAIN
#define MULOG_DOMAIN "Museek.DC"

DownloadConnection::DownloadConnection(Transfer* transfer)
                   : BaseConnection(0, "F", transfer->peer(), 0),
                     mTransfer(transfer), mSendTicket(false) {
	
	transfer->set_state(TS_Establishing);
}

DownloadConnection::DownloadConnection(Transfer* transfer, uint32 token)
                   : BaseConnection(0, "F", transfer->peer(), token),
                     mTransfer(transfer), mSendTicket(true) {
	
	transfer->set_state(TS_Establishing);
}

void DownloadConnection::connected() {
	BaseConnection::connected();
	
	if(! mTransfer)
		return;
	
	if(mSendTicket) {
		DEBUG("sending ticket");
		ClientConnection::send(mTransfer->ticket());
		mTransfer->invalidate_ticket();
	}
	
	DEBUG("sending position");
	ClientConnection::send((off_t)mTransfer->pos());
	
	DEBUG("%u", outbuf.size());
	
	if(mTransfer->pos() == mTransfer->size()) {
		mTransfer->set_state(TS_Finished);
		disconnect();
	} else {
		mTransfer->set_state(TS_Waiting);
	}
}

void DownloadConnection::cannot_connect() {
	BaseConnection::cannot_connect();
	
	if(! mTransfer)
		return;
	
	mTransfer->set_state(TS_CannotConnect);
}

void DownloadConnection::disconnected() {
	if(! mTransfer)
		return;
	
	switch(mTransfer->state()) {
	case TS_Finished:
	case TS_Error:
		break;
	default:
		mTransfer->set_state(TS_ConnectionClosed);
	}
}

void DownloadConnection::process() {
	DEBUG("downloaded %u bytes", inbuf.size());
	
	is_shaped = true;
	
	if(! mTransfer)
		return;
	
	if(inbuf.size() > 0 && ! mTransfer->write(inbuf)) {
		mTransfer->set_error("Error writing to file");
		disconnect();
		return;
	}
	
	if(mTransfer->pos() == mTransfer->size()) {
		mTransfer->set_state(TS_Finished);
		disconnect();
	}
}


#undef MULOG_DOMAIN
#define MULOG_DOMAIN "Museek.UC"

UploadConnection::UploadConnection(Transfer* transfer)
                   : BaseConnection(0, "F", transfer->peer(), 0),
                     mTransfer(transfer), mSendTicket(false), mHavePos(false) {
	
	transfer->set_state(TS_Establishing);
}

UploadConnection::UploadConnection(Transfer* transfer, uint32 token)
                   : BaseConnection(0, "F", transfer->peer(), token),
                     mTransfer(transfer), mSendTicket(true), mHavePos(false) {
	
	transfer->set_state(TS_Establishing);
}

void UploadConnection::connected() {
	BaseConnection::connected();
	
	if(! mTransfer)
		return;
	
	if(mSendTicket) {
		DEBUG("sending ticket");
		ClientConnection::send(mTransfer->ticket());
		mTransfer->invalidate_ticket();
	}
	
	mTransfer->set_state(TS_Negotiating);
}

void UploadConnection::cannot_connect() {
	BaseConnection::cannot_connect();
	
	if(! mTransfer)
		return;
	
	mTransfer->set_state(TS_CannotConnect);
	if(! mSendTicket)
		mPeer->connection()->upload_failed(mTransfer->path());
}

void UploadConnection::disconnected() {
	if(! mTransfer)
		return;
	
	if(mTransfer->state() == TS_Error)
		return;
	
	if(mTransfer->pos() == mTransfer->size())
		mTransfer->set_state(TS_Finished);
	else
		mTransfer->set_state(TS_ConnectionClosed);
	
	DEBUG("connection closed");
	mTransfer->manager()->check_uploads();
}

void UploadConnection::process() {
	DEBUG("got %u bytes", inbuf.size());
	if(! mTransfer)
		return;
	
	if(! mHavePos) {
		if(inbuf.size() >= 8) {
			off_t pos = 0;
			for(int i = 0; i < 8; i++) {
				pos += inbuf.front() << (i*8);
				inbuf.pop();
			}
			if(! mTransfer->seek(pos)) {
				DEBUG("seek error");
				mTransfer->set_error("Local file error");
				disconnect();
				return;
			}
			mHavePos = true;
			if(! mTransfer->read(outbuf)) {
				DEBUG("read error");
				mTransfer->set_error("Local file error");
				disconnect();
				return;
			}
			DEBUG("have size %i", outbuf.size());
			is_shaped = true;
		}
	}
	if(mHavePos)
		while(inbuf.size())
			inbuf.pop();
}

void UploadConnection::data_sent(uint sent) {
	if(! mTransfer)
		return;
	
	mTransfer->sent(sent);
	if(outbuf.size() < 10240 && mTransfer->pos() + (off_t)outbuf.size() < mTransfer->size()) {
		if(! mTransfer->read(outbuf)) {
			DEBUG("read error");
			mTransfer->set_error("Local file error");
			disconnect();
		}
	}
}

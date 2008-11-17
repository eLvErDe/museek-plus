/*  Museek - A SoulSeek client written in C++
    Copyright (C) 2006-2007 Ingmar K. Steen (iksteen@gmail.com)
    Copyright 2008 little blue poney <lbponey@users.sourceforge.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */

#ifndef MUSEEK_UPLOADSOCKET_H
#define MUSEEK_UPLOADSOCKET_H

#include "usersocket.h"
#include <fstream>

namespace Museek
{
  class Museekd;
  class Upload;
  class TicketSocket;

  class UploadSocket : public UserSocket
  {
  public:
    UploadSocket(Museekd * museekd, Upload * upload);
    ~UploadSocket();

    void wait();
    void stop();
    void send(const unsigned char * data, size_t n);
    void sendTicket();

  private:
    void onDisconnected(NewNet::ClientSocket * socket);
    void onCannotConnect(NewNet::ClientSocket * socket);
    void onTransferTicketReceived(TicketSocket * socket);
    void onDataSent(NewNet::ClientSocket * socket);
    void onDataReceived(NewNet::ClientSocket * socket);
    void findPosition();
    void dataTimeout(long);

    NewNet::RefPtr<Upload>  m_Upload; // Reference to the upload
	bool                    mHavePos; // Have we already received the position sent by the downloader?
	size_t                  m_lastDataSentCount; // What was the last data count in the buffer?
    NewNet::WeakRefPtr<NewNet::Event<long>::Callback> m_DataTimeout;
  };
}

#endif // MUSEEK_UPLOADSOCKET_H

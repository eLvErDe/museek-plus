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

#ifndef MUSEEK_DOWNLOADSOCKET_H
#define MUSEEK_DOWNLOADSOCKET_H

#include "usersocket.h"
#include <fstream>

namespace Museek
{
  class Museekd;
  class Download;
  class TicketSocket;

  class DownloadSocket : public UserSocket
  {
  public:
    DownloadSocket(Museekd * museekd, Download * download);
    ~DownloadSocket();

    void pickUp();
    void wait();
    void stop();

  private:
    bool openIncompleteFile();
    void onConnected(NewNet::ClientSocket * socket);
    void onDisconnected(NewNet::ClientSocket * socket);
    void onCannotConnect(NewNet::ClientSocket * socket);
    void onTransferTicketReceived(TicketSocket * socket);
    void onDataReceived(NewNet::ClientSocket * socket);
    void finish();
    void dataTimeout(long);

    NewNet::RefPtr<Download> m_Download;
    std::ofstream m_Output;
    NewNet::WeakRefPtr<NewNet::Event<long>::Callback> m_DataTimeout;
  };
}

#endif // MUSEEK_DOWNLOADSOCKET_H

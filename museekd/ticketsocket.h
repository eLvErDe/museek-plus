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

#ifndef MUSEEK_TICKETSOCKET_H
#define MUSEEK_TICKETSOCKET_H

#include "usersocket.h"

namespace Museek
{
  class TicketSocket : public UserSocket
  {
  public:
    TicketSocket(HandshakeSocket * that);
    TicketSocket(Museekd * museekd);
    ~TicketSocket();
    void findTicket();

    uint ticket() const { return m_Ticket; }

  private:
    void onDataReceived(NewNet::ClientSocket *);

    uint m_Ticket;
  };
}

#endif // MUSEEK_TICKETSOCKET_H

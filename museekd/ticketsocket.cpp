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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif // HAVE_CONFIG_H
#include "ticketsocket.h"
#include "downloadmanager.h"
#include "uploadmanager.h"
#include "museekd.h"
#include <NewNet/nnreactor.h>

Museek::TicketSocket::TicketSocket(Museek::HandshakeSocket * that) : UserSocket(that, "F")
{
    // Sometimes, the ticket is sent at the connection of the socket so the data received event is not called.
    // You should call findTicket when you create a new TicketSocket, after adding it to the reactor
    dataReceivedEvent.connect(this, &TicketSocket::onDataReceived);
}

Museek::TicketSocket::TicketSocket(Museek::Museekd * museekd) : UserSocket(museekd, "F")
{
  dataReceivedEvent.connect(this, &TicketSocket::onDataReceived);
}

Museek::TicketSocket::~TicketSocket()
{
  NNLOG("museekd.ticket.debug", "TicketSocket destroyed");
}

void
Museek::TicketSocket::onDataReceived(NewNet::ClientSocket *) {
    findTicket();
}

void
Museek::TicketSocket::findTicket() {
    if(receiveBuffer().count() < 4)
        return;

    NNLOG("museekd.ticket.debug", "TicketSocket got %u bytes", receiveBuffer().count());
    // Unpack the ticket
    if (receiveBuffer().count() >= 4 ) {
        unsigned char * data = receiveBuffer().data();
        m_Ticket = data[0] + (data[1] << 8) + (data[2] << 16) + (data[3] << 24);
        receiveBuffer().seek(4);
    }

    // Notify our waiting downloadsockets
    NNLOG("museekd.ticket.debug", "Yay! We received ticket %u.. Now what..", m_Ticket);
    museekd()->downloads()->transferTicketReceivedEvent(this);
    museekd()->uploads()->transferTicketReceivedEvent(this);

    // Self-terminate
    receiveBuffer().clear();
    museekd()->reactor()->remove(this);
}

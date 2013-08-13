// MooDriver - (C) 2006 M.Derezynski
//
// Code based on;
//
// ---
//
// giFTcurs - curses interface to giFT
// Copyright (C) 2001, 2002, 2003 Göran Weinholt <weinholt@dtek.chalmers.se>
// Copyright (C) 2003 Christian Häggström <chm@c00.info>
//

#ifndef MOO_TYPES_PRIVATE_H
#define MOO_TYPES_PRIVATE_H

#include <glib.h>
#include <map>
#include <vector>

namespace Moo
{
    typedef std::vector<unsigned char> Data;
    enum MuseekMessages
    {
        M_PING                  = 0x0000,
        M_CHALLENGE             = 0x0001,
        M_LOGIN                 = 0x0002,
        M_SERVER_STATE          = 0x0003,
        M_PRIVILEGES            = 0x0004,
        M_SET_STATUS            = 0x0005, 

        M_STATUSMSG             = 0x0010,
        M_CONFSTATE             = 0x0100,
        M_CONFSET               = 0x0101,
        M_CONFREMOVE            = 0x0102,
        M_SETUSRIMG             = 0x0103,
        
        M_PEER_EXISTS           = 0x0201,
        M_PEER_STATUS           = 0x0202,
        M_PEER_STATS            = 0x0203,
        M_PEER_INFO             = 0x0204,
        M_PEER_SHARES           = 0x0205,
        M_PEER_ADDRESS          = 0x0206,
        M_PEER_GIVEPRIVS        = 0x0207,
        
        M_ROOM_STATE            = 0x0300,
        M_ROOM_LIST             = 0x0301,
        M_PRIVMSG               = 0x0302,
        M_ROOM_JOIN             = 0x0303,
        M_ROOM_LEAVE            = 0x0304,
        M_ROOM_USER_JOIN        = 0x0305,
        M_ROOM_USER_LEAVE       = 0x0306,
        M_ROOM_SAY              = 0x0307,
        M_ROOM_TICKER           = 0x0308,
        M_ROOM_TICKER_SET       = 0x0309,

        M_SEARCH                = 0x0401,
        M_SEARCH_REPLY          = 0x0402,
        M_USER_SEARCH           = 0x0403,
        M_WISHLIST_SEARCH       = 0x0405,

        M_XFER_STATE            = 0x0500,
        M_XFER_UPDATE           = 0x0501,
        M_XFER_REMOVE           = 0x0502,
        M_XFER_DOWNLOAD         = 0x0503,
        M_XFER_DOWN_FOLDER      = 0x0504,
        M_XFER_ABORT            = 0x0505,
        M_XFER_UPLOAD           = 0x0506,

        M_GET_RECOMMENDS        = 0x0600,
        M_GET_RECOMMENDSALL     = 0x0601,
        M_GET_FRIENDS           = 0x0602,
        M_GET_ITEM_RECOMMEND    = 0x0603,
        M_GET_ITEM_PEERS        = 0x0604,
        
        M_INTEREST_ADD          = 0x0610,
        M_INTEREST_REMOVE       = 0x0611,
        M_INTEREST_HATE_ADD     = 0x0612,
        M_INTEREST_HATE_REMOVE  = 0x0613,

        M_SERVER_CONNECT        = 0x0700,
        M_SERVER_DISCONNECT     = 0x0701,

        M_RELOAD_SHARES         = 0x0703
    };
}

#endif

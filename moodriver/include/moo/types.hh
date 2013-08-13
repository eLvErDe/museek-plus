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

#ifndef MOO_TYPES_H
#define MOO_TYPES_H

#include <glib.h>
#include <map>
#include <vector>
#include <sigc++/sigc++.h>

namespace Moo
{
    // Transfer
    enum TransferState
    {
      XFER_STATE_FINISHED,
      XFER_STATE_RUNNING,
      XFER_STATE_NEGOTIATE,
      XFER_STATE_WAITING,
      XFER_STATE_ESTABLISHING,
      XFER_STATE_INITIATING,
      XFER_STATE_CONNECTING,
      XFER_STATE_QUEUED,
      XFER_STATE_GET_ADDRESS,
      XFER_STATE_GET_STATUS,
      XFER_STATE_NO_USER,
      XFER_STATE_CLOSED,
      XFER_STATE_UNABLE_TO_CONNECT,
      XFER_STATE_ABORTED,
      XFER_STATE_ERROR
    };
    enum DaemonMessageFlags
    {
      M_DAEMON_FLAG_NONE       = 0,
      M_DAEMON_FLAG_CHAT       = 1 << 0,
      M_DAEMON_FLAG_PRIVATE    = 1 << 1,
      M_DAEMON_FLAG_TRANSFERS  = 1 << 2,
      M_DAEMON_FLAG_USERINFO   = 1 << 3,
      M_DAEMON_FLAG_USERSHARES = 1 << 4,
      M_DAEMON_FLAG_INTERESTS  = 1 << 5,
      M_DAEMON_FLAG_CONFIG     = 1 << 6
    };
    struct Transfer
    {
      bool          upload;
      std::string   user;
      std::string   path;
      unsigned int  queuepos;
      unsigned int  state;
      std::string   error;
      gint64        position;
      gint64        size;
      unsigned int  rate;
    };
    struct PeerStatistics
    {
      std::string username;
      unsigned int speed;
      // Average Transfer Speed (set by client after uploading a file)
      unsigned int downloads;
      // Number of Files other users have downloaded from this user
      unsigned int files;
      // Number of Files this user reports as Shared
      unsigned int dirs;
      // Number of Directories this user reports as Shared
    };
    struct PeerStatus
    {
      std::string username;
      unsigned int status;
      // 0 Offline; 1 Away; 2 Online
    };
    struct PeerExists
    {
      std::string username;
      bool exists;
      // Is this user in the Server's list of users?
      // Inactive users get removed from the server's database
      // after a certain length of time (around two months)
    };
    struct PeerAddress
    {
      std::string username;
      std::string ipaddress;
      // IP Address, supplied by the server
      unsigned int port;
      // Port Number of user's Soulseek Client
      // If 0, then Server didn't recieve a correct port number
      // and connections will fail.
    };
    struct PeerInfo
    {
      std::string username;
      std::string info;
      // Text of user's info. May contain newlines.
      std::string picture;
      // User's image or empty string
      unsigned int uploads;
      // Number of concurrent uploads; how many uploads can be sent at once.
      unsigned int queuelength;
      // How many files or users are in the user's queue, this number depends
      // on which client and queue type the user is using. 
      // Queue types include Round-Robin, FIFO and randomly selected.
      bool slotsfree;
      // TRUE if the user has a free upload slot and send the file immediately.
      
    };
    typedef std::pair<std::string, std::string> XFERKey;
    typedef std::map<XFERKey, Transfer> Transfers;
    
    // Rooms
    typedef std::map<std::string, std::string> TickersList;
    struct _UserData {
	unsigned int status, avgspeed, downloadnum, files, dirs;
	bool slotsfull;
    };
    struct PrivateMessage {
      unsigned int direction;
      // Direction the message is travelling
      // 0 If another user has sent you this message
      // 1 If another museek client has sent this message to the user
      // You will not receive a message if you send a private msssage
      // from this client.
      unsigned int timestamp;
      // Unix Time timestamp, set by the Server
      std::string username;
      // The user who you receiving or sending a Private to or from.
      std::string message;
      // The message, itself
      // Note that messages starting with "/me " should be replaced
      // with "* username "
    };
    struct RoomMessage {
      std::string room;
      // Room in which the message was said.
      std::string username;
      // The user who said the message.
      std::string message;
      // The message, itself
      // Note that messages starting with "/me " should be replaced
      // with "* username "
    };
    struct Interest {
      std::string item;
      // An Interest item, for recommending similar users and other interests
      bool opinion;
      // 0 Like; 1 Hate;
    };
    typedef struct _UserData UserData;
    struct Room { std::string name; unsigned int n_users; };

    typedef std::vector<Room> RoomList;

    typedef std::map<std::string, UserData> RoomData;
    typedef std::map<std::string, TickersList> RoomsTickers;
    typedef std::map<std::string, RoomData> RoomsLists;
    
    struct SimilarUser {
      std::string username;
      unsigned int status;
      // 0 Offline; 1 Away; 2 Online
    };
    typedef std::vector<SimilarUser> SimilarUsers;
    
    
    // Search Result
    typedef std::vector<unsigned int> FileAttrs;
    struct File
    {
        std::string       name;
        gint64            size;
        std::string       ext;
        FileAttrs         attrs;
    };
    typedef std::vector<File> FileList;
    struct SearchResult
    {
        unsigned int      ticket;
        std::string       user;
        bool              slot;
        unsigned int      speed;
        unsigned int      queue;
        FileList          files;
    };
    typedef std::map<std::string, std::string> Domain;
    typedef std::map<std::string, Domain> Config;
    
    // Signals
    typedef sigc::signal<void>                            SDisconnected;
    typedef sigc::signal<void>                            SConnected;
    typedef sigc::signal<void, bool, std::string>         SStatusMessage;
    typedef sigc::signal<void, bool, std::string>         SServerState;
    typedef sigc::signal<void, bool, std::string>         SLogin;
    typedef sigc::signal<void, std::string, unsigned int> SSearch;
    typedef sigc::signal<void, SearchResult>              SSearchResult;
    typedef sigc::signal<void, XFERKey, Transfer>         STransferUpdate;
    typedef sigc::signal<void, RoomMessage>               SRoomSay;
    typedef sigc::signal<void, PrivateMessage>            SPrivateSay;
    typedef sigc::signal<void, RoomList, RoomsLists, RoomsTickers> SRoomState;
    typedef sigc::signal<void, RoomList>                  SRoomList;
    typedef sigc::signal<void, std::string, RoomData>     SRoomJoin;
    typedef sigc::signal<void, std::string>               SRoomLeave;
    typedef sigc::signal<void, std::string, std::string, UserData>     SRoomUserJoin;
    typedef sigc::signal<void, std::string, std::string>               SRoomUserLeave;
    typedef sigc::signal<void, unsigned int>              SSetStatus;
    typedef sigc::signal<void, unsigned int>              SPrivilegesLeft;
    typedef sigc::signal<void, PeerExists>                SPeerExists;
    typedef sigc::signal<void, PeerStatus>                SPeerStatus;
    typedef sigc::signal<void, PeerStatistics>            SPeerStats;
    typedef sigc::signal<void, PeerInfo>                  SPeerInfo;
    typedef sigc::signal<void, PeerAddress>               SPeerAddress;
    typedef sigc::signal<void, Interest>                  SInterestAdd;
    typedef sigc::signal<void, Interest>                  SInterestRemove;
    typedef sigc::signal<void, Config>                    SConfigState;
    typedef sigc::signal<void, std::string, std::string, std::string> SConfigSet;
    typedef sigc::signal<void, std::string, std::string>  SConfigRemove;
    enum MuseekSearchType
    {
        SEARCH_GLOBAL,
        SEARCH_BUDDIES,
        SEARCH_ROOM
    };

    typedef std::map<std::string, guint> SearchesBySearchT;
    typedef std::map<guint, std::string> SearchesByTicketT;

}

#endif

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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>

#include <glib.h>
#include <glib/gi18n.h>

#include <iostream>
#include <istream>
#include <ostream>
#include <sstream>

#include "xcommon.hh"
#include "util.hh"

#include <moo/types.hh>
#include <moo/driver.hh>
#include <moo/mucipher.h>

#include "moo-io.hh"
#include "moo-types-private.hh"

namespace
{
    const char* xfer_states[] =
    {
      N_("Finished"),
      N_("Running"),
      N_("Negotiate"),
      N_("Waiting"),
      N_("Establishing"),
      N_("Initiating"),
      N_("Connecting"),
      N_("Queued"),
      N_("Getting Address"),
      N_("Getting Status"),
      N_("User Offline"),
      N_("Transfer Closed"),
      N_("Unable To Connect"),
      N_("Transfer Aborted"),
      N_("Error"),
    };
}

namespace Moo
{
    //Helper Stuff
    std::string
    Driver::xfer_status_description (Moo::TransferState state)
    {
      if ( state > int(G_N_ELEMENTS (xfer_states))) return std::string();
      return std::string(xfer_states[state]);
    } 

    gboolean
    Driver::m_stream_read (GIOChannel     *source,
                           GIOCondition    condition,
                           gpointer        data)
    {
      Moo::Driver *driver = reinterpret_cast<Moo::Driver *>(data);

      if (!g_mutex_trylock (driver->lock)) return FALSE;

      if (condition == G_IO_HUP)
        {
          g_mutex_unlock (driver->lock);
          driver->disconnect ();
          return FALSE;
        }
      else if (condition == G_IO_ERR)
        {
          g_message ("Condition: %d", condition); 
        }
      else if (condition == G_IO_IN)
        {
          guint msize = Moo::IO::read_uint (source);

          if (!msize)
            {
              g_mutex_unlock (driver->lock);
              driver->disconnect ();
              return FALSE;
            }

          size_t  expect = msize,
                  length = 0;
          char   *data   = g_new0 (char, expect); 

          GError *error = 0;
          g_io_channel_read_chars (source, data, expect, &length, &error);

          if (error)
            {
              g_warning ("Error occured reading from the iochannel: %s", error->message);
              g_error_free (error);
              abort ();
            }

          Moo::Data read_data;
          if (expect != length)
            {
              g_free (data);
              g_mutex_unlock (driver->lock);
              driver->disconnect ();
              return FALSE;
            }
          else
            {
              for (guint n = 0; n < length; ++n)
                {
                  read_data.push_back ((unsigned char)data[n]);
                }
              g_free (data);
            }

          guint offset = 0;    
          guint mtype = Moo::IO::unpack_uint (read_data, offset);

          if (msize) switch (mtype)
            {
              case Moo::M_CHALLENGE:
              {
                Moo::IO::unpack_uint (read_data, offset); //NOTE: We need to read off those 4 first bytes or the rest data is scrambled
                std::string challenge = Moo::IO::unpack_string (read_data, offset);

                challenge += driver->m_password; 
                
                std::string resp      = Util::md5_hex ((char*)challenge.c_str(), strlen (challenge.c_str())); 
                std::string algo      = "MD5";

                // Attempt Login
                guint response_size =  (sizeof(guint)+strlen(algo.c_str())) +  // Algorithm; we use MD5
                                       (sizeof(guint)+strlen(resp.c_str())) +  // Challenge response
                                       (sizeof(guint)) +                       // Mask
                                       (sizeof(guint));                        // Message ID;

                Moo::IO::write_uint (source, response_size); 
                Moo::IO::write_uint (source, (guint)(Moo::M_LOGIN));
                Moo::IO::write_string (source, algo.c_str());
                Moo::IO::write_string (source, resp.c_str()); 
                Moo::IO::write_uint (source, driver->m_daemon_flags); 
                g_io_channel_flush (source, NULL);
                break;
              }
              case Moo::M_LOGIN:
              { 
                bool success = Moo::IO::unpack_bool (read_data, offset);
                std::string reason;
                if (!success)
                  {
                    reason = Moo::IO::unpack_string (read_data, offset);
                  }
                else
                  {
                    cipherKeySHA256(&(driver->mContext), (char*)driver->m_password.c_str(), strlen(driver->m_password.c_str()));
                  }
                driver->s_login_.emit (success, reason);
                break;
              }

              case Moo::M_SERVER_STATE:     
              {
                bool connected = Moo::IO::unpack_bool (read_data, offset);
                std::string username  = Moo::IO::unpack_string (read_data, offset);
                driver->s_server_state_.emit (connected, username);
                break;
              }
              case Moo::M_PRIVILEGES:
              {
                unsigned int days = Moo::IO::unpack_uint (read_data, offset);
                driver->s_privileges_left_.emit(days);
                break;
              }

              case Moo::M_SET_STATUS:
              {
                unsigned int status = Moo::IO::unpack_uint (read_data, offset);
                driver->s_set_status_.emit (status);
                break;
              }

              case Moo::M_STATUSMSG:
              {
                bool type = Moo::IO::unpack_bool (read_data, offset);
                std::string message = Moo::IO::unpack_string (read_data, offset);
                driver->s_status_message_.emit (type, message);
                break;
              }

              case Moo::M_CONFSTATE:
              {
                unsigned int n = Moo::IO::unpack_uint (read_data, offset);

                while (n)
                {
                  std::string domain = Moo::CipherIO::decipher (read_data, offset, &driver->mContext);
                  driver->config[domain];
                  unsigned int o = Moo::IO::unpack_uint (read_data, offset);
                  while (o)
                  {
                    std::string key = Moo::CipherIO::decipher (read_data, offset, &driver->mContext);
                    std::string val = Moo::CipherIO::decipher (read_data, offset, &driver->mContext);
                    driver->config[domain][key] = val;
                    o--;
                  }
                  n--;
                }
                driver->s_config_state_.emit(driver->config);
                break;
              };
              case Moo::M_CONFSET:
              {
                std::string domain = Moo::CipherIO::decipher (read_data, offset, &driver->mContext);
                std::string key = Moo::CipherIO::decipher (read_data, offset, &driver->mContext);
                std::string val = Moo::CipherIO::decipher (read_data, offset, &driver->mContext);
                driver->config[domain][key] = val;
                driver->s_config_set_.emit(domain, key, val);
                break;
              }
              case Moo::M_CONFREMOVE:
              {
                std::string domain = Moo::CipherIO::decipher (read_data, offset, &driver->mContext);
                std::string key = Moo::CipherIO::decipher (read_data, offset, &driver->mContext);
                driver->config[domain].erase(key);
                driver->s_config_remove_.emit(domain, key);
                break;
              }

              case Moo::M_PEER_EXISTS:
              {
                PeerExists peerexists;
                
                peerexists.username = Moo::IO::unpack_string (read_data, offset);
                peerexists.exists = Moo::IO::unpack_bool (read_data, offset);
                
                driver->s_peer_exists_.emit (peerexists);
                break;
              }
              case Moo::M_PEER_STATUS:
              {
                PeerStatus peerstatus;
                
                peerstatus.username = Moo::IO::unpack_string (read_data, offset);
                peerstatus.status = Moo::IO::unpack_uint (read_data, offset);
                
                driver->s_peer_status_.emit (peerstatus);
                break;
              }
              case Moo::M_PEER_STATS:
              {
                PeerStatistics statistics;
                
                statistics.username  = Moo::IO::unpack_string (read_data, offset);
                statistics.speed = Moo::IO::unpack_uint (read_data, offset);
                statistics.downloads = Moo::IO::unpack_uint (read_data, offset);
                statistics.files = Moo::IO::unpack_uint (read_data, offset);
                statistics.dirs = Moo::IO::unpack_uint (read_data, offset);
                
                driver->s_peer_stats_.emit (statistics);
                break;
              }
              case Moo::M_PEER_INFO:
              {
                PeerInfo peerinfo;
                
                peerinfo.username  = Moo::IO::unpack_string (read_data, offset);
                peerinfo.info  = Moo::IO::unpack_string (read_data, offset);
                peerinfo.picture  = Moo::IO::unpack_string (read_data, offset);
                peerinfo.uploads = Moo::IO::unpack_uint (read_data, offset);
                peerinfo.queuelength = Moo::IO::unpack_uint (read_data, offset);
                peerinfo.slotsfree = Moo::IO::unpack_bool (read_data, offset);
                
                driver->s_peer_info_.emit (peerinfo);
                break;
              }
              case Moo::M_PEER_SHARES:
              {
                
                std::string username  = Moo::IO::unpack_string (read_data, offset);
                unsigned int numFolders = Moo::IO::unpack_uint (read_data, offset);
                for (guint n = 0; n < numFolders; ++n) {
                        std::string folder = Moo::IO::unpack_string (read_data, offset);
                        unsigned int numFiles = Moo::IO::unpack_uint (read_data, offset);
                        for (guint n = 0; n < numFiles; ++n) {
                            File file;
                            file.name = Moo::IO::unpack_string (read_data, offset);
                            file.size = Moo::IO::unpack_off_t (read_data, offset);
                            file.ext  = Moo::IO::unpack_string (read_data, offset);
                            guint numAttrs = Moo::IO::unpack_uint (read_data, offset);
                            if (numAttrs) for (guint n = 0; n < numAttrs; ++n)
                            {
                              file.attrs.push_back (Moo::IO::unpack_uint (read_data, offset));
                            }
//                             result.files.push_back (file); 
                        }

                }
                break;
              }
              case Moo::M_PEER_ADDRESS:
              {
                PeerAddress peeraddress;
                
                peeraddress.username = Moo::IO::unpack_string (read_data, offset);
                peeraddress.ipaddress = Moo::IO::unpack_string (read_data, offset);
                peeraddress.port = Moo::IO::unpack_uint (read_data, offset);
                
                driver->s_peer_address_.emit(peeraddress);
                break;
              }

              case Moo::M_ROOM_STATE: 
              {
                RoomList roomlist;
                
                guint numRooms = Moo::IO::unpack_uint (read_data, offset); 
                for (guint n = 0; n < numRooms; ++n) {
                        Room room;
                        room.name  = Moo::IO::unpack_string (read_data, offset);
                        room.n_users = Moo::IO::unpack_uint (read_data, offset);
                        roomlist.push_back(room);
                        
                }
                
                RoomsLists joinedrooms;
                RoomsTickers roomstickers;
                
                guint numJoined = Moo::IO::unpack_uint (read_data, offset); 
                for (guint n = 0; n < numJoined; ++n) {
                        RoomData roomdata;
                        std::string room  = Moo::IO::unpack_string (read_data, offset);
                        unsigned int numUsers = Moo::IO::unpack_uint (read_data, offset);
//                         std::vector<UserData> _d;
                        for (guint n = 0; n < numUsers; ++n) {
                                UserData _data;

                                std::string user  = Moo::IO::unpack_string (read_data, offset);
                                _data.status = Moo::IO::unpack_uint (read_data, offset);
                                _data.avgspeed = Moo::IO::unpack_uint (read_data, offset);
                                _data.downloadnum = Moo::IO::unpack_uint (read_data, offset);
                                _data.files = Moo::IO::unpack_uint (read_data, offset);
                                _data.dirs = Moo::IO::unpack_uint (read_data, offset);
                                _data.slotsfull = Moo::IO::unpack_bool (read_data, offset);

                                roomdata[user] = _data;
                        }
                        TickersList tickers;
                        unsigned int numTickers = Moo::IO::unpack_uint (read_data, offset);
                        for (guint n = 0; n < numTickers; ++n) {
                                std::string user  = Moo::IO::unpack_string (read_data, offset);
                                std::string ticker = Moo::IO::unpack_string (read_data, offset);
                                tickers[user] = ticker;
                        }
                        roomstickers[room] = tickers;
                        joinedrooms[room] = roomdata;
                }

                driver->s_room_state_.emit (roomlist, joinedrooms, roomstickers);
                break;
              }
              case Moo::M_ROOM_LIST: 
              {
                RoomList roomlist;
                guint numRooms = Moo::IO::unpack_uint (read_data, offset); 
                for (guint n = 0; n < numRooms; ++n) {
                  Room room;
                  room.name  = Moo::IO::unpack_string (read_data, offset);
                  room.n_users = Moo::IO::unpack_uint (read_data, offset);
                  roomlist.push_back(room);
                }
                
                 driver->s_room_list_.emit(roomlist);
                break;
              }
              case Moo::M_PRIVMSG: 
              {
                PrivateMessage privatemessage;
                privatemessage.direction = Moo::IO::unpack_uint (read_data, offset);
                privatemessage.timestamp = Moo::IO::unpack_uint (read_data, offset);
                privatemessage.username = Moo::IO::unpack_string (read_data, offset);
                privatemessage.message = Moo::IO::unpack_string (read_data, offset);
                driver->s_private_say_.emit (privatemessage);
                break;
              }
              case Moo::M_ROOM_JOIN:
              {
                std::string room  = Moo::IO::unpack_string (read_data, offset);
                unsigned int numUsers = Moo::IO::unpack_uint (read_data, offset);
                RoomData roomdata;
                for (guint n = 0; n < numUsers; ++n) {
                        UserData _data;

                        std::string user  = Moo::IO::unpack_string (read_data, offset);
                        _data.status = Moo::IO::unpack_uint (read_data, offset);
                        _data.avgspeed = Moo::IO::unpack_uint (read_data, offset);
                        _data.downloadnum = Moo::IO::unpack_uint (read_data, offset);
                        _data.files = Moo::IO::unpack_uint (read_data, offset);
                        _data.dirs = Moo::IO::unpack_uint (read_data, offset);
                        _data.slotsfull = Moo::IO::unpack_bool (read_data, offset);

                        roomdata[user] = _data;
                }
                driver->s_room_join_.emit (room, roomdata);
                break;
              }
              case Moo::M_ROOM_LEAVE:
              {
                std::string room  = Moo::IO::unpack_string (read_data, offset);
                driver->s_room_leave_.emit (room);
                break;
              }
              case Moo::M_ROOM_USER_JOIN:
              {
                std::string room  = Moo::IO::unpack_string (read_data, offset);
                std::string username  = Moo::IO::unpack_string (read_data, offset);
                
                UserData _data;

                _data.status = Moo::IO::unpack_uint (read_data, offset);
                _data.avgspeed = Moo::IO::unpack_uint (read_data, offset);
                _data.downloadnum = Moo::IO::unpack_uint (read_data, offset);
                _data.files = Moo::IO::unpack_uint (read_data, offset);
                _data.dirs = Moo::IO::unpack_uint (read_data, offset);
                _data.slotsfull = Moo::IO::unpack_bool (read_data, offset);
                
                driver->s_room_user_join_.emit (room, username, _data);
                break;
              }
              case Moo::M_ROOM_USER_LEAVE:
              {
              std::string room  = Moo::IO::unpack_string (read_data, offset);
              std::string username  = Moo::IO::unpack_string (read_data, offset);
              
              driver->s_room_user_leave_.emit (room, username);
              break;
              }
              case Moo::M_ROOM_SAY: 
              {
                RoomMessage roommessage;
                
                roommessage.room  = Moo::IO::unpack_string (read_data, offset);
                roommessage.username = Moo::IO::unpack_string (read_data, offset);
                roommessage.message = Moo::IO::unpack_string (read_data, offset);
                
                driver->s_room_say_.emit (roommessage);
                break;
              }
              case Moo::M_ROOM_TICKER:
              {
                TickersList tickers;
                std::string room  = Moo::IO::unpack_string (read_data, offset);
                unsigned int numTickers = Moo::IO::unpack_uint (read_data, offset);
                for (guint n = 0; n < numTickers; ++n) {
                  std::string user  = Moo::IO::unpack_string (read_data, offset);
                  std::string ticker = Moo::IO::unpack_string (read_data, offset);
                  tickers[user] = ticker;
                }
               break;
              }

              case Moo::M_ROOM_TICKER_SET:
              {
                 std::string room  = Moo::IO::unpack_string (read_data, offset);
                 std::string username  = Moo::IO::unpack_string (read_data, offset);
                 std::string message  = Moo::IO::unpack_string (read_data, offset);
                break;
              }

              case Moo::M_SEARCH:
              {
                std::string search = Moo::IO::unpack_string (read_data, offset);
                guint ticket = Moo::IO::unpack_uint (read_data, offset);
                
                driver->m_searches_by_search.insert (std::make_pair (search, ticket));
                driver->m_searches_by_ticket.insert (std::make_pair (ticket, search));
                driver->s_search_.emit (search, ticket);
                break;
              }

              case Moo::M_SEARCH_REPLY:
              {
                SearchResult result;

                result.ticket   = Moo::IO::unpack_uint    (read_data, offset);
                result.user     = Moo::IO::unpack_string  (read_data, offset);
                result.slot     = Moo::IO::unpack_bool    (read_data, offset);
                result.speed    = Moo::IO::unpack_uint    (read_data, offset);
                result.queue    = Moo::IO::unpack_uint    (read_data, offset);
                
                guint numFiles = Moo::IO::unpack_uint (read_data, offset); 
                for (guint n = 0; n < numFiles; ++n)
                {
                    File file;
                    file.name = Moo::IO::unpack_string (read_data, offset);
                    file.size = Moo::IO::unpack_off_t (read_data, offset);
                    file.ext  = Moo::IO::unpack_string (read_data, offset);
                    guint numAttrs = Moo::IO::unpack_uint (read_data, offset);
                    if (numAttrs) for (guint n = 0; n < numAttrs; ++n)
                      {
                        file.attrs.push_back (Moo::IO::unpack_uint (read_data, offset));
                      }
                    result.files.push_back (file); 
                }
                driver->s_search_result_.emit (result);
                break;
              }

              case Moo::M_XFER_STATE:
              {
                guint nTransfers = Moo::IO::unpack_uint (read_data, offset);
                Transfers xfers; 

                for (guint n = 0; n < nTransfers; ++n)
                {
                  Transfer xfer;

                  xfer.upload     = Moo::IO::unpack_bool (read_data, offset);
                  xfer.user       = Moo::IO::unpack_string (read_data, offset);
                  xfer.path       = Moo::IO::unpack_string (read_data, offset);
                  xfer.queuepos   = Moo::IO::unpack_uint (read_data, offset);
                  xfer.state      = Moo::IO::unpack_uint (read_data, offset);
                  xfer.error      = Moo::IO::unpack_string (read_data, offset);
                  xfer.position   = Moo::IO::unpack_off_t (read_data, offset);
                  xfer.size       = Moo::IO::unpack_off_t (read_data, offset);
                  xfer.rate       = Moo::IO::unpack_uint (read_data, offset);

                  XFERKey key (xfer.user, xfer.path); 
                  driver->m_transfers[key] = xfer; 
                  driver->s_transfer_update_.emit (key, xfer);
                }
                break;
              }
              case Moo::M_XFER_REMOVE:
              {
//                 Transfer xfer;
                bool upload = Moo::IO::unpack_bool (read_data, offset);
                std::string user  = Moo::IO::unpack_string (read_data, offset);
                std::string path  = Moo::IO::unpack_string (read_data, offset);
                if (upload);
//                 XFERKey key (user, path); 
//                 driver->m_transfers[key] = xfer; 
//                 driver->s_transfer_remove_.emit (key, xfer);

                break;
              }
              case Moo::M_GET_RECOMMENDS:
              {
              std::string room  = Moo::IO::unpack_string (read_data, offset);
              break;
              }
              case Moo::M_GET_RECOMMENDSALL:
              {
              std::string room  = Moo::IO::unpack_string (read_data, offset);
              break;
              }
              case Moo::M_GET_FRIENDS:
              {
                SimilarUsers similarusers;
                guint numUsers = Moo::IO::unpack_uint (read_data, offset); 
                for (guint n = 0; n < numUsers; ++n) {
                        SimilarUser similaruser;
                        similaruser.username  = Moo::IO::unpack_string (read_data, offset);
                        similaruser.status = Moo::IO::unpack_uint (read_data, offset);
                        similarusers.push_back (similaruser); 
                 }
                 
                break;
              }
              case Moo::M_GET_ITEM_RECOMMEND:
              {
              std::string interest  = Moo::IO::unpack_string (read_data, offset);
              break;
              }
              case Moo::M_GET_ITEM_PEERS:
              {
                
                std::string interest  = Moo::IO::unpack_string (read_data, offset);
                guint numUsers = Moo::IO::unpack_uint (read_data, offset); 
                SimilarUsers similarusers;
                for (guint n = 0; n < numUsers; ++n) {
                        SimilarUser similaruser;
                        similaruser.username  = Moo::IO::unpack_string (read_data, offset);
                        similaruser.status = Moo::IO::unpack_uint (read_data, offset);
                        similarusers.push_back (similaruser); 
                 }
                break;
              }
              case Moo::M_INTEREST_ADD:
              {
                Interest interest;
                interest.item = Moo::IO::unpack_string (read_data, offset);
                interest.opinion = 0;
                
                driver->s_interest_add_.emit (interest);
                break;
              }
              case Moo::M_INTEREST_REMOVE:
              {
                Interest interest;
                interest.item = Moo::IO::unpack_string (read_data, offset);
                interest.opinion = 0;
                
                driver->s_interest_remove_.emit (interest);
                break;
              }
              case Moo::M_INTEREST_HATE_ADD:
              {
                Interest interest;
                interest.item = Moo::IO::unpack_string (read_data, offset);
                interest.opinion = 1;
                
                driver->s_interest_add_.emit (interest);
                break;
              }
              case Moo::M_INTEREST_HATE_REMOVE:
              {
                Interest interest;
                interest.item = Moo::IO::unpack_string (read_data, offset);
                interest.opinion = 1;
                
                driver->s_interest_remove_.emit (interest);
                break;
              }
            }
        }
      g_mutex_unlock (driver->lock);
      return TRUE;
    } 
}

namespace Moo
{
 // Chatting
    void
    Driver::room_say ( const std::string &room,   const std::string &message)  
    {
      guint mSize = ( 3*sizeof(guint) ) + strlen( room.c_str() ) + strlen( message.c_str() ); 
      Moo::IO::write_uint   (m_stream, mSize);
      
      Moo::IO::write_uint   (m_stream, (guint)(M_ROOM_SAY));
      Moo::IO::write_string (m_stream, room.c_str());
      Moo::IO::write_string (m_stream, message.c_str());
      g_io_channel_flush    (m_stream, NULL);
    }
    void
    Driver::room_join ( const std::string &room)
    {
      guint mSize = ( 2*sizeof(guint) ) + strlen( room.c_str() ) ; 
      Moo::IO::write_uint   (m_stream, mSize);
      
      Moo::IO::write_uint   (m_stream, (guint)(M_ROOM_JOIN));
      Moo::IO::write_string (m_stream, room.c_str());
      
      g_io_channel_flush    (m_stream, NULL);
    }
    void
    Driver::room_leave ( const std::string &room)
    {
      guint mSize = ( 2*sizeof(guint) ) + strlen( room.c_str() ); 
      Moo::IO::write_uint   (m_stream, mSize);
      
      Moo::IO::write_uint   (m_stream, (guint)(M_ROOM_LEAVE));
      Moo::IO::write_string (m_stream, room.c_str());
      
      g_io_channel_flush    (m_stream, NULL);
    }
    void
    Driver::refresh_room_list ()
    {
      guint mSize = (1*sizeof(guint));
      Moo::IO::write_uint   (m_stream, mSize); 
      Moo::IO::write_uint   (m_stream, (guint)(M_ROOM_LIST));
      g_io_channel_flush    (m_stream, NULL);
    }
    void
    Driver::private_say ( const std::string &username,   const std::string &message)  
    {
      guint mSize = ( 3*sizeof(guint) ) + strlen( username.c_str() ) + strlen( message.c_str() ); 
      Moo::IO::write_uint   (m_stream, mSize);
      
      Moo::IO::write_uint   (m_stream, (guint)(M_PRIVMSG));
      Moo::IO::write_string (m_stream, username.c_str());
      Moo::IO::write_string (m_stream, message.c_str());
      g_io_channel_flush    (m_stream, NULL);
    }
    //------------ Searching
    void
    Driver::search_start (MuseekSearchType        type,
                          const std::string      &query)  
    {
      guint mSize = ((3*sizeof(guint))+strlen(query.c_str())); 
      Moo::IO::write_uint   (m_stream, mSize); 

      Moo::IO::write_uint   (m_stream, (guint)(M_SEARCH));
      Moo::IO::write_uint   (m_stream, (guint)(type));
      Moo::IO::write_string (m_stream, query.c_str());
      g_io_channel_flush    (m_stream, NULL);
    }

    void
    Driver::set_status (unsigned int status) 
    {
      guint mSize = (2*sizeof(guint));
      Moo::IO::write_uint   (m_stream, mSize); 

      Moo::IO::write_uint   (m_stream, (guint)(M_SET_STATUS));
      Moo::IO::write_uint   (m_stream, (guint)(status));

      g_io_channel_flush    (m_stream, NULL);

    }
    
    void
    Driver::search_cancel (unsigned int ticket) 
    {
      guint mSize = (2*sizeof(guint));
      Moo::IO::write_uint   (m_stream, mSize); 

      Moo::IO::write_uint   (m_stream, (guint)(M_SEARCH_REPLY));
      Moo::IO::write_uint   (m_stream, (guint)(ticket));

      g_io_channel_flush    (m_stream, NULL);

      std::string search;
      SearchesByTicketT::iterator i = m_searches_by_ticket.find (ticket);
      if (i != m_searches_by_ticket.end())
        {
          search = i->second;
          m_searches_by_ticket.erase (i);
        }
      m_searches_by_search.erase (search);
    }

    void
    Driver::request_transfers ()
    {
      guint mSize = (1*sizeof(guint));
      Moo::IO::write_uint   (m_stream, mSize); 
      Moo::IO::write_uint   (m_stream, (guint)(M_XFER_STATE));
      g_io_channel_flush    (m_stream, NULL);
    }

    //------------ Downloads 
    void
    Driver::download_start (const XFERKey& key)
    {
      guint mSize = (3*sizeof(guint)) + strlen(key.first.c_str()) + strlen (key.second.c_str());
      Moo::IO::write_uint   (m_stream, mSize); 

      Moo::IO::write_uint   (m_stream, (guint)(M_XFER_DOWNLOAD)); 
      Moo::IO::write_string (m_stream, key.first.c_str()); 
      Moo::IO::write_string (m_stream, key.second.c_str()); 

      g_io_channel_flush    (m_stream, NULL);
    }

    void
    Driver::transfer_abort (const XFERKey& key, bool upload)
    {
      guint mSize = (3*sizeof(guint)) + strlen(key.first.c_str()) + strlen (key.second.c_str()) + (sizeof(unsigned char));
      Moo::IO::write_uint   (m_stream, mSize); 

      Moo::IO::write_uint   (m_stream, (guint)(M_XFER_ABORT)); 
      Moo::IO::write_bool   (m_stream, (unsigned char)(upload)); 
      Moo::IO::write_string (m_stream, key.first.c_str()); 
      Moo::IO::write_string (m_stream, key.second.c_str()); 

      g_io_channel_flush    (m_stream, NULL);
    }

    void
    Driver::transfer_remove (const XFERKey& key, bool upload)
    {
      guint mSize = (3*sizeof(guint)) + strlen(key.first.c_str()) + strlen (key.second.c_str()) + (sizeof(unsigned char));
      Moo::IO::write_uint   (m_stream, mSize); 

      Moo::IO::write_uint   (m_stream, (guint)(M_XFER_REMOVE)); 
      Moo::IO::write_bool   (m_stream, (unsigned char)(upload)); 
      Moo::IO::write_string (m_stream, key.first.c_str()); 
      Moo::IO::write_string (m_stream, key.second.c_str()); 

      g_io_channel_flush    (m_stream, NULL);
    }
    void
    Driver::config_set (const std::string &domain, const std::string &key, const std::string &value)  
    {
      guint mSize = 0;
      unsigned char *cdomain, *ckey, *cvalue;
      gsize cdomain_size = 0, ckey_size = 0, cvalue_size = 0;

      cdomain = Moo::CipherIO::cipher (domain.c_str(), &mContext, cdomain_size);
      ckey = Moo::CipherIO::cipher (key.c_str(), &mContext, ckey_size);
      cvalue = Moo::CipherIO::cipher (value.c_str(), &mContext, cvalue_size);

      mSize += sizeof(guint); // message code
      mSize += cdomain_size;
      mSize += ckey_size;
      mSize += cvalue_size;

      mSize += 3*sizeof(guint); // +3 times the size of the unencrypted string

      Moo::IO::write_uint (m_stream, mSize); 
      Moo::IO::write_uint (m_stream, (guint)(M_CONFSET));

      Moo::IO::write_uint (m_stream, (unsigned int)(strlen(domain.c_str())));
      Moo::BasicIO::write_data (m_stream, cdomain, cdomain_size); 

      Moo::IO::write_uint (m_stream, (unsigned int)(strlen(key.c_str())));
      Moo::BasicIO::write_data (m_stream, ckey, ckey_size); 

      Moo::IO::write_uint (m_stream, (unsigned int)(strlen(value.c_str())));
      Moo::BasicIO::write_data (m_stream, cvalue, cvalue_size); 

      g_io_channel_flush (m_stream, NULL);

      g_free (cdomain);
      g_free (ckey);
      g_free (cvalue);
    }

    //------------ Connection with museekd 
    bool
    Driver::connect ()
    {
      if (m_stream) return true;

      int fd = xconnect_ip (m_server_host.c_str(), m_server_port.c_str());
      if (fd < 0)
      {
        g_log (MOO_LOG_DOMAIN,
               G_LOG_LEVEL_INFO,
               _("Can't connect to the daemon at %s:%s, is museekd running?"), m_server_host.c_str(), m_server_port.c_str());
        s_disconnected_.emit();
        return false; 
      }

      m_stream = g_io_channel_unix_new (fd);
      g_io_channel_set_encoding (m_stream, NULL, NULL);
      m_stream_source_id = g_io_add_watch (m_stream, GIOCondition (G_IO_IN | G_IO_ERR | G_IO_HUP), Driver::m_stream_read, this);

      g_log (MOO_LOG_DOMAIN,
             G_LOG_LEVEL_INFO,
             _("Connected to museekd at %s:%s"), m_server_host.c_str(), m_server_port.c_str());

      s_connected_.emit();
      return true;
    }

    void
    Driver::disconnect ()
    {
      if (!m_stream) return;
      if (!g_mutex_trylock(lock))  
        {
          while (!g_mutex_trylock(lock)) while (g_main_context_pending(NULL)) g_main_context_iteration (NULL, TRUE);
        }
      g_source_destroy (g_main_context_find_source_by_id (NULL, m_stream_source_id));
      g_io_channel_shutdown (m_stream, TRUE, NULL);
      g_io_channel_unref (m_stream);
      m_stream = NULL;
      g_mutex_unlock (lock);
      s_disconnected_.emit();
    }
}

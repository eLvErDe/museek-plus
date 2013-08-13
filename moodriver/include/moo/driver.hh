#ifndef MOO_DRIVER_HH
#define MOO_DRIVER_HH

#include <string>
#include <glib.h>
#include "types.hh"
#include "mucipher.h"

namespace Moo
{
  class Driver 
  {
    public:

      
      
      Driver (const std::string& hostname, const std::string& port, const std::string& password, unsigned int& daemon_flags ) 
      : m_stream (0), m_server_host (hostname), m_server_port (port), m_password (password), m_daemon_flags(daemon_flags)
      {
        lock = g_mutex_new ();
      }

      ~Driver () { if (m_stream) disconnect(); }

      // Core Functions
      void credentials (const std::string& hostname, const std::string& port, const std::string& password, bool reconnect = false) 
      { 
        m_server_host = hostname;
        m_server_port = port;
        m_password    = password;
        
        if (reconnect)
          {
            disconnect ();
            connect ();
          }
      }

      bool connect    ();
      void disconnect ();

      void search_start (MuseekSearchType type, std::string const&query);
      void room_say (std::string const&room, std::string const&message);
      void room_join (std::string const&room);
      void room_leave (std::string const&room);
      
      void private_say (std::string const&username, std::string const&message);
      void set_status (unsigned int status);
      void search_cancel (unsigned int ticket);

      void download_start  (XFERKey const& key);

      void transfer_abort  (XFERKey const& key, bool upload = false);
      void transfer_remove (XFERKey const& key, bool upload = false);
      
      void config_state(Config& config);
      void config_set(std::string const&domain, std::string const&key, std::string const&value);
      void config_remove(std::string const&domain, std::string const&key);
      
      void request_transfers  ();
      void privileges_left    ();
      void refresh_room_list  ();

      //Config
      std::string
      get_key (const std::string& domain, const std::string& key)
      {
        Moo::Config::iterator i = config.find (domain);
        if (i == config.end()) return std::string();

        Moo::Domain::iterator d = i->second.find (key);        
        if (d == i->second.end ()) return std::string();

        return d->second;
      }

      //Helper Stuff
      static std::string xfer_status_description (Moo::TransferState state);

      // Signals
      SConnected&           s_connected               () { return s_connected_; }
      SDisconnected&        s_disconnected            () { return s_disconnected_; }
      SStatusMessage&       s_status_message          () { return s_status_message_; }
      SServerState&         s_server_state            () { return s_server_state_; }
      SLogin&               s_login                   () { return s_login_; }
      SSearch&              s_search                  () { return s_search_; }
      SSearchResult&        s_search_result           () { return s_search_result_; }
      STransferUpdate&      s_transfer_update         () { return s_transfer_update_; }
      SRoomSay&             s_room_say                () { return s_room_say_; }
      SPrivateSay&          s_private_say             () { return s_private_say_; }
      SRoomState&           s_room_state              () { return s_room_state_; }
      SRoomList&            s_room_list               () { return s_room_list_; }
      SRoomJoin&            s_room_join               () { return s_room_join_; }
      SRoomLeave&           s_room_leave              () { return s_room_leave_; }
      SRoomUserJoin&        s_room_user_join          () { return s_room_user_join_; }
      SRoomUserLeave&       s_room_user_leave         () { return s_room_user_leave_; }
      SSetStatus&           s_set_status              () { return s_set_status_; }
      SPrivilegesLeft&      s_privileges_left         () { return s_privileges_left_; }
      SPeerExists&          s_peer_exists             () { return s_peer_exists_; }
      SPeerInfo&            s_peer_info               () { return s_peer_info_; }
      SPeerStats&           s_peer_stats              () { return s_peer_stats_; }
      SPeerStatus&          s_peer_status             () { return s_peer_status_; }
      SPeerAddress&         s_peer_address            () { return s_peer_address_; }
      SInterestAdd&         s_interest_add            () { return s_interest_add_; }
      SInterestRemove&      s_interest_remove         () { return s_interest_remove_; }
      SConfigState&         s_config_state            () { return s_config_state_; }
      SConfigSet&           s_config_set              () { return s_config_set_; }
      SConfigRemove&        s_config_remove           () { return s_config_remove_; }
      
    private:

      SConnected          s_connected_;
      SDisconnected       s_disconnected_;
      SStatusMessage      s_status_message_;
      SServerState        s_server_state_;
      SLogin              s_login_;
      SSearch             s_search_;
      SSearchResult       s_search_result_;
      STransferUpdate     s_transfer_update_;
      SRoomSay            s_room_say_;
      SPrivateSay         s_private_say_;
      SRoomState          s_room_state_;
      SRoomList           s_room_list_;
      SRoomJoin           s_room_join_;
      SRoomLeave          s_room_leave_;
      SRoomUserJoin       s_room_user_join_;
      SRoomUserLeave      s_room_user_leave_;
      SSetStatus          s_set_status_;
      SPrivilegesLeft     s_privileges_left_;
      SPeerExists         s_peer_exists_;
      SPeerInfo           s_peer_info_;
      SPeerStats          s_peer_stats_;
      SPeerStatus         s_peer_status_;
      SPeerAddress        s_peer_address_;
      SInterestAdd        s_interest_add_;
      SInterestRemove     s_interest_remove_;
      SConfigState        s_config_state_;
      SConfigSet          s_config_set_;
      SConfigRemove       s_config_remove_;
      
      static gboolean
      m_stream_read (GIOChannel     *source,
                     GIOCondition    condition,
                     gpointer        data);

      GIOChannel  *m_stream;
      guint        m_stream_source_id;
      GMutex      *lock; 

      std::string  m_server_host;
      std::string  m_server_port;
      std::string  m_password;
      guint  m_daemon_flags;

      Transfers         m_transfers;

      
      SearchesBySearchT m_searches_by_search;
      SearchesByTicketT m_searches_by_ticket;

      CipherContext mContext; 

      Moo::Config config;
  };
}
#endif

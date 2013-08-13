// MooDriver test code (C) 2006 mderezynski

#include <glib.h>
#include <glib-object.h>
#include <sigc++/sigc++.h>
#include <moo/driver.hh>
#include <moo/types.hh>

#include <iostream>

Moo::Driver *driver;
std::string  query;

void
status_message  (bool         server_peer,
                 std::string  message)
{
  g_message ("Status Message: %s", message.c_str());
}

void
server_state    (bool         connected,
                 std::string  username)
{
  if (connected)
    {
      g_message ("Connected as %s.", username.c_str());
      if (!query.empty()) driver->search_start (Moo::SEARCH_GLOBAL, query);
    }
  else
    {
      g_message ("Disconnected.");
    }
}

void
login           (bool         success,
                 std::string  reason)
{
  if (!success)
    {
      g_message ("Not Logged in. Reason: %s", reason.c_str());
    }
  else
    {
      g_message ("Logged In.");
    }
}

void
search          (std::string  query,
                 unsigned int ticket)
{
  g_message ("Search: %s, ticket_id: %u", query.c_str(), ticket); 
}

void
search_result   (Moo::SearchResult result)
{
  g_message ("User: %s", result.user.c_str());
  for (Moo::FileList::const_iterator i = result.files.begin (); i != result.files.end(); ++i)
    {
      g_message ("\t\tFile: '%s'", i->name.c_str());
    }

  if (result.slot)
    {
      //driver->search_cancel (result.ticket);
      Moo::FileList::const_iterator i = result.files.begin ();
      Moo::XFERKey key (result.user, i->name);
      driver->download_start (key);
    }
}

void
transfer_update (Moo::XFERKey key, Moo::Transfer xfer)
{
  std::cout << "Filename ["<< key.second <<"]\n\tSize ["<< xfer.size <<"]\n\tPosition ["<< xfer.position <<"]\n\tRate ["<< xfer.rate/1024. <<" KB/s]\n\tState ["<< Moo::Driver::xfer_status_description (Moo::TransferState (xfer.state)) <<"]\n\n";
}

int
main (int n_args, char **args)
{
    g_type_init ();
    g_thread_init (NULL);

    if (n_args < 4)
      {
        std::cerr << "Usage: moo <hostname> <port> <password> [keywords...]\n";
        exit (0);
      }
      unsigned int daemonflags = Moo::M_DAEMON_FLAG_CHAT | Moo::M_DAEMON_FLAG_CONFIG | Moo::M_DAEMON_FLAG_TRANSFERS;
    driver = new Moo::Driver (args[1], args[2], args[3], daemonflags);

    for (int n = 4; n < n_args; ++n)
      {
        query += args[n];
        if (n != (n_args-1)) query += " ";
      }

    driver->s_status_message().connect (sigc::ptr_fun (&status_message));
    driver->s_server_state().connect (sigc::ptr_fun (&server_state));
    driver->s_login().connect (sigc::ptr_fun (&login));
    driver->s_search().connect (sigc::ptr_fun (&search));
    driver->s_search_result().connect (sigc::ptr_fun (&search_result));
    driver->s_transfer_update().connect (sigc::ptr_fun (&transfer_update));

    driver->connect ();

    GMainLoop *loop = g_main_loop_new (NULL, FALSE);
    g_main_loop_run (loop);
}

/* museekd - The Museek daemon
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

#include <museekd/museekd.hh>
#include <string>

#define MULOG_DOMAIN "museekd.MA"
#include <Muhelp/Mulog.hh>

using std::string;

Museekd* museekd = 0;

RETSIGTYPE reconnect(int signal) {
	if(museekd != 0) {
		if (museekd->mKicked) museekd->mKicked = false;
		DEBUG("reconnecting...");
		museekd->do_reconnect();
	}
	else if (museekd->mKicked) {
		museekd->mKicked = false;
		museekd->do_reconnect();
	}
	
#if RETSIGTYPE != void
	return 0;
#endif
}

RETSIGTYPE terminate(int signal) {
	if(museekd != 0) {
		DEBUG("terminating...");
		museekd->die();
	}
#if RETSIGTYPE != void
	return 0;
#endif
}

RETSIGTYPE hangup(int signal) {
	if(museekd != 0) {
		DEBUG("reloading shares...");
		museekd->load_shares();
	}
#if RETSIGTYPE != void
	return 0;
#endif
}

int main(int argc, char **argv) {
	std::string config = string(getenv("HOME")) + "/.museekd/config.xml";
	std::string version = string("museekd :: Version 0.1.9 :: Museek Daemon Plus");
	for(int i = 1; i < argc; i++) {
		string arg = argv[i];
		if(arg == "--config" || arg == "-c") {
			if(i + 1 != argc) {
				config = argv[i+1];
			} else {
				std::cerr << "Missing config file path" << std::endl;
				return -1;
			}
		} else if(arg == "--version" || arg == "-V" ) {
			std::cout << version << std::endl << std::endl;
			return 0;
		} else if(arg == "--help" || arg == "-h") {
			std::cout << version << std::endl;
			std::cout << "Syntax: museekd [options]" << std::endl << std::endl;
			std::cout << "Options:" << std::endl;
			std::cout << "-c --config <config file>\tUse alternative config file" << std::endl;
			std::cout << "-h --help\t\t\tDisplay this message and quit" << std::endl;
			std::cout << "-V --version\t\t\tDisplay museekd version and quit" << std::endl << std::endl; 
			std::cout << "Signals:" << std::endl;
			std::cout << "kill -HUP \tReload Shares Database(s)" << std::endl;
			std::cout << "kill -ALRM \tReconnect to Server" << std::endl;
			std::cout << std::endl;
			return 0;
		}
			
	}
	std::cout << version << std::endl << std::endl;
	signal(SIGALRM, reconnect);
	signal(SIGHUP, hangup);
	signal(SIGINT, terminate);
	signal(SIGTERM, terminate);
	
	museekd = new Museekd(config);
	if(! museekd->load_config()) {
		delete museekd;
		std::cerr << "Error loading configuration! Please run musetup and configure any unset options." << std::endl;
		return -1;
	}
	
	museekd->init();
	
	museekd->server_connect();
	museekd->loop();
	
	delete museekd;
	
	return 0;
}

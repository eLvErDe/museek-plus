/* Tools - Tools for Museek (muscan)
 *
 * Copyright (C) 2003-2004 Hyriand <hyriand@thegraveyard.org>
 * Copyright 2008 little blue poney <lbponey@users.sourceforge.net>
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

#include <system.h>

#include <muscan/scanner.hh>
#include <Muhelp/Muconf.hh>
#include <NewNet/nnlog.h>

#include <iostream>
#include <cstdlib>

using std::vector;
using std::string;
using std::cout;
using std::cerr;
using std::endl;
using std::map;

void help() {
	cout << "muscan [-c --config PATH] [-b --buddy] [-v --verbose] [-r --rescan] [-n --noscan] [-s --share PATH]... [-u --unshare PATH]..." << endl;
	cout << "or:" << endl;
	cout << "muscan [-c --config PATH] -l --list" << endl;
	cout << "Version 0.2.0" << endl;
	exit(-1);
}

int main(int argc, char **argv) {
	string config_file = string(getenv("HOME")) + "/.museekd/config.xml";
	vector<string> add, remove;
	bool doList = false, rescan = false, noscan = false, doBuddy = false;

	for(int i = 1; i < argc; i++) {
		string arg = argv[i];
		if(arg == "-c" || arg == "--config") {
			++i;
			if(i == argc)
				help();
			config_file = string(argv[i]);
		} else if(arg == "-s" || arg == "--share") {
			++i;
			if(i == argc)
				help();
			add.push_back(string(argv[i]));
		} else if(arg == "-u" || arg == "--unshare") {
			++i;
			if(i == argc)
				help();
			remove.push_back(string(argv[i]));
		} else if(arg == "-b" || arg == "--buddy") {
			doBuddy = true;
		} else if(arg == "-l" || arg == "--list") {
			doList = true;
		} else if(arg == "-v" || arg == "--verbose") {
			Scanner_Verbosity += 1;
		} else if(arg == "-r" || arg == "--rescan") {
			rescan = true;
		} else if(arg == "-n" || arg == "--noscan") {
			noscan = true;
		} else
			help();
	}
	
	if (Scanner_Verbosity >= 2){
	    NNLOG.logEvent.connect(new NewNet::ConsoleOutput);
    	NNLOG.enable("ALL");
    }
	
	Muconf config(config_file);
	if(! config.hasDomain("shares") || ! config["shares"].hasKey("database")) {
		cerr << "config file '" << config_file << "' has incomplete or corrupt shares database" << endl;
		exit(-1);
	}
	if(! config.hasDomain("buddy.shares") || ! config["buddy.shares"].hasKey("database")) {
		cerr << "config file '" << config_file << "' has incomplete or corrupt buddy shares database" << endl;
		exit(-1);
	}
	
	DirScanner root;
	string state;
	
	if (doBuddy) {
		string s = config["buddy.shares"]["database"];
		state += s + ".state";
	} else {
		string s = config["shares"]["database"];
		state += s + ".state";
	}
		
	root.load(state);

	if(doList) {
		map<string, DirEntry*>::iterator fit = root.folders.begin();
		for(; fit != root.folders.end(); ++fit)
			cout << (*fit).second->path << endl;
		
		return 0;
	}
	
	vector<string>::iterator it = add.begin();
	for(; it != add.end(); ++it)
		root.add(*it);

	it = remove.begin();
	for(; it != remove.end(); ++it) {
		map<string, DirEntry*>::iterator fit = root.folders.find(*it);
		if(fit != root.folders.end()) {
			delete (*fit).second;
			root.folders.erase(fit);
		}
	}

	if(rescan) {

		add.clear();

		map<string, DirEntry*>::iterator fit = root.folders.begin();
		for(; fit != root.folders.end(); ++fit)
			add.push_back((*fit).first);
		it = add.begin();
		
		root = DirScanner();
		
		for(; it != add.end(); ++it)
			root.add(*it);
	}
	if(! noscan)
		root.scan();
	root.save(state);
	DirScanner folded;
	root.fold(&folded);
		
	if (doBuddy) { 
		folded.save(config["buddy.shares"]["database"]);
	} else {
		folded.save(config["shares"]["database"]);
	}

	return 0;
}

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
#include <museekd/IfaceConnection.hh>
#include <Museek/Recoder.hh>
#include <Museek/ShareBase.hh>
#include <Museek/TransferManager.hh>
#include <Museek/PeerManager.hh>
#include <Museek/PeerConnection.hh>
#include <Mucipher/mucipher.h>
#include <museekd/Buddies.hh>
#include <Muhelp/string_ext.hh>

#define MULOG_DOMAIN "museekd.MD"
#include <Muhelp/Mulog.hh>

#include <set>

using std::vector;
using std::set;
using std::string;
using std::wstring;
using std::map;
using std::cerr;
using std::endl;

Museekd::Museekd(const string& config, const string& version)
        : Museek(),  mVersion(version), mKicked(false), 
	mHaveBuddyShares(false), mReconnectTime(15), mConfigFile(config), mConfig(mConfigFile), 
	mStatus(0), mPrivilegeBuddies(false), mOnlyBuddies(false),  mTrustedUploads(false), 
	mJoined(false), mDontSaveDownloads(false), mSaveDownloads(false), mLastSave(0),
	mPonged(false), mPingTime(0) {
	CT("Museekd");
}

bool Museekd::load_config() {
	mConfig = Muconf(mConfigFile);
	
	string base;
	int ix1 = mConfigFile.rfind('.'),
	    ix2 = mConfigFile.rfind('/');
        if(ix1 > ix2)
		base = mConfigFile.substr(0, ix1 + 1);
	else
		base = mConfigFile + ".";
	
	mConfig["interfaces"]["password"];
	
	if(! mConfig.hasDomain("interfaces.bind")) {
		mConfig["interfaces.bind"]["localhost:2240"];
#ifdef HAVE_SYS_UN_H
# ifdef HAVE_PWD_H
		struct passwd *pw = getpwuid(getuid());
		if(pw)
			mConfig["interfaces.bind"][string("/tmp/museekd.") + pw->pw_name];
# endif
#endif
	}
	
	// added logging defaults (darrik@mythofbutterfly.com)
	if (! mConfig["logging"]["output"]) {
		// console output
		mConfig["logging"]["output"] = 0;
	}
	if (! mConfig["logging"]["syslog_facility"]) {
		// LOG_LOCAL6 facility (see syslog.h)
		mConfig["logging"]["syslog_facility"] = 176;
	}
	if (! mConfig["logging"]["syslog_priority"]) {
		// LOG_DEBUG priority (see syslog.h)
		mConfig["logging"]["syslog_priority"] = 7;
	}
	// end logging defaults

	if(! mConfig["shares"]["database"])
		mConfig["shares"]["database"] = base + "shares";

	if(! mConfig["buddy.shares"]["database"])
		mConfig["buddy.shares"]["database"] = base + "buddyshares";
	
	if(! mConfig["transfers"]["downloads"])
		mConfig["transfers"]["downloads"] = base + "downloads";
	
	if(! mConfig["server"]["host"])
		mConfig["server"]["host"] = "server.slsknet.org";
	
	if(! mConfig["server"]["port"])
		mConfig["server"]["port"] = 2240;
	
	mConfig["server"]["username"];
	mConfig["server"]["password"];
	
	if(! mConfig["clients"]["connectmode"])
		mConfig["clients"]["connectmode"] = "passive";
	if(! mConfig["clients.bind"]["first"])
		mConfig["clients.bind"]["first"] = "2234";
	if(! mConfig["clients.bind"]["last"])
		mConfig["clients.bind"]["last"] = "2240";
	
	if(! mConfig["encoding"]["network"])
		mConfig["encoding"]["network"] = "utf-8";
	if(! mConfig["encoding"]["filesystem"])
		mConfig["encoding"]["filesystem"] = "latin1";
	
	if(! mConfig["transfers"]["upload_slots"])
		mConfig["transfers"]["upload_slots"] = 1;
	
	if(! mConfig["transfers"]["privilege_buddies"])
		mConfig["transfers"]["privilege_buddies"] = true;
	if(! mConfig["transfers"]["user_warnings"])
		mConfig["transfers"]["user_warnings"] = true;
	if(! mConfig["transfers"]["only_buddies"])
		mConfig["transfers"]["only_buddies"] = false;
	if(! mConfig["transfers"]["trusting_uploads"])
		mConfig["transfers"]["trusting_uploads"] = false;
	if(! mConfig["transfers"]["have_buddy_shares"])
		mConfig["transfers"]["have_buddy_shares"] = false;
	mConfig["transfers"]["download-dir"];
	mConfig["transfers"]["incomplete-dir"];
	
	mConfig["encoding.rooms"];
	mConfig["encoding.users"];

	mConfig["buddies"];
	mConfig["banned"];
	mConfig["ignored"];
	mConfig["trusted"];
	
	mConfig["userinfo"]["text"];
	if(! mConfig["userinfo"]["image"])
		mConfig["userinfo"]["image"] = base + "image";
	
	mConfig["interests.like"];
	mConfig["interests.hate"];

	mConfig["tickers"];
	mConfig["default-ticker"]["ticker"];

	mConfig.store();
	
	
	bool incomplete = false;
	
	if(! mConfig["interfaces"]["password"]) {
		cerr << "FATAL: Interface password missing in " << mConfigFile << endl;
		incomplete = true;
	}
	if(! mConfig["server"]["username"]) {
		cerr << "FATAL: Username missing in " << mConfigFile << endl;
		incomplete = true;
	}
	if(! mConfig["server"]["password"]) {
		cerr << "FATAL: Password missing in " << mConfigFile << endl;
		incomplete = true;
	}
	if(! mConfig["transfers"]["download-dir"]) {
		cerr << "FATAL: Download dir missing in " << mConfigFile << endl;
		incomplete = true;
	}
	
	return ! incomplete;
}


void Museekd::init() {
	// initial output sink for mulog
	DEBUG("changing logging output to %d",mConfig["logging"]["output"].asUint());
	mulog.setSyslogFacility(mConfig["logging"]["syslog_facility"].asUint());
	mulog.setSyslogPriority(mConfig["logging"]["syslog_priority"].asUint());
	mulog.setOutput(mConfig["logging"]["output"].asUint());

	set_port_range(mConfig["clients.bind"]["first"].asUint(), mConfig["clients.bind"]["last"].asUint());
	
	Museek::init();
	
	load_downloads();
	
	init_iface_listeners();
	
	load_shares();
	load_buddy_shares();

	init_buddies();

	mHaveBuddyShares = mConfig["transfers"]["have_buddy_shares"].asBool();
	set_have_buddy_shares(mHaveBuddyShares);
	mTrustedUploads = mConfig["transfers"]["trusting_uploads"].asBool();
	mPrivilegeBuddies = mConfig["transfers"]["privilege_buddies"].asBool();
	mOnlyBuddies = mConfig["transfers"]["only_buddies"].asBool();
	mUserWarnings = mConfig["transfers"]["user_warnings"].asBool();
	mu_set_user_warnings(mUserWarnings);
	set_user_warnings(mUserWarnings);
	set_only_buddies(mOnlyBuddies);
	mu_set_only_buddies(mOnlyBuddies);
	update_buddies();

	set_server(mConfig["server"]["host"],
	           mConfig["server"]["port"].asUint());
	
	set_username(mConfig["server"]["username"]);
	set_password(mConfig["server"]["password"]);
	
	if(mConfig["clients"]["connectmode"] == string("active"))
		set_connect_mode(CM_Active);
	else
		set_connect_mode(CM_Passive);
	
	set_upload_slots(mConfig["transfers"]["upload_slots"].asInt());
	
	mUserinfo = mRecoder->decode_utf8(mConfig["userinfo"]["text"]);
	set_iface_password(mConfig["interfaces"]["password"]);
	
	FILE *f = fopen(mConfig["userinfo"]["image"], "r");
	if(f) {
		vector<unsigned char> tmpimg;
		int read;
		unsigned char buf[8192];
		while((read = fread(buf, 1, 8192, f))) {
			for(int i = 0; i < read; ++i)
				tmpimg.push_back(buf[i]);
		}
		fclose(f);
		DEBUG("loaded userimage, %i bytes", tmpimg.size());
		set_userpic(tmpimg);
	} else 
		perror("cannot load userinfo image: ");

	vector<string> autojoin = mConfig["autojoin"].keys();
	vector<string>::const_iterator it = autojoin.begin();
	for(; it != autojoin.end(); ++it) {
		mJoinedRooms[*it];
		mTickers[*it];
	}

}

void Museekd::load_shares() {
	mShares->load(mConfig["shares"]["database"], "normal");
}
void Museekd::load_buddy_shares() {
	mBuddyShares->load(mConfig["buddy.shares"]["database"], "buddy");
}

void Museekd::init_recoder() {
	mRecoder = new Recoder(mConfig["encoding"]["filesystem"], mConfig["encoding"]["network"]);
	
	map<string, string> codings = mConfig["encoding.users"];
	map<string, string>::iterator it = codings.begin();
	for(; it != codings.end(); ++it)
		mRecoder->set_user((*it).first, (*it).second);
	
	codings = mConfig["encoding.rooms"];
	for(it = codings.begin(); it != codings.end(); ++it)
		mRecoder->set_room((*it).first, (*it).second);
}

void Museekd::init_transfer_manager() {
	Museek::init_transfer_manager();
	
	wstring temp_dir, download_dir = mRecoder->decode_utf8(mConfig["transfers"]["download-dir"]);
	
	if(! mConfig["transfers"]["incomplete-dir"])
		temp_dir = download_dir;
	else
		temp_dir = mRecoder->decode_utf8(mConfig["transfers"]["incomplete-dir"]);
	
	if(download_dir[download_dir.size() - 1] != '/')
		download_dir += '/';
	if(temp_dir[temp_dir.size() - 1] != '/')
		temp_dir += '/';
	
	mTransferManager->set_download_dir(download_dir);
	mTransferManager->set_temp_dir(temp_dir);
	
	vector<string> banned = mConfig["banned"].keys();
	vector<string>::iterator it = banned.begin();
	for(; it != banned.end(); ++it)
		ban(*it);

	vector<string> trusted = mConfig["trusted"].keys();
	vector<string>::iterator tit = trusted.begin();
	for(; tit != trusted.end(); ++tit)
		trust(*tit);
}

void Museekd::init_iface_listeners() {
	StringMap ifaces = mConfig["interfaces.bind"];
	StringMap::const_iterator it = ifaces.begin();
	for(; it != ifaces.end(); ++it) {
		IfaceListener* listener = new IfaceListener(this, (*it).first);
		if(listener->get_error() == ERR_NONE)
			add(listener);
	}
}

void Museekd::init_buddies() {
	mBuddies = new Buddies(this);
	
	map<string, string> buddies = mConfig["buddies"];
	map<string, string>::const_iterator it = buddies.begin();
	for(; it != buddies.end(); ++it)
		mBuddies->add((*it).first);
}

static char challengemap[] = "0123456789abcdef";
string Museekd::challenge() {
	string r;
	for(int i = 0; i < 64; i++)
		r += challengemap[rand() % 16];
	return r;
}

void Museekd::new_iface(IfaceConnection* conn) {
	mIfaces.push_back(conn);
	add(conn);
	conn->challenge(4, challenge());
}

void Museekd::remove_iface(IfaceConnection* conn) {
	vector<IfaceConnection*>::iterator it;
	it = find(mIfaces.begin(), mIfaces.end(), conn);
	mIfaces.erase(it);
	
	vector<uint> searches;
	map<uint, IfaceConnection*>::const_iterator s_it = mSearches.begin();
	for(; s_it != mSearches.end(); ++s_it)
		if((*s_it).second == conn)
			searches.push_back((*s_it).first);
	vector<uint>::const_iterator _s_it = searches.begin();
	for(; _s_it != searches.end(); ++_s_it)
		mSearches.erase(*_s_it);
}


#define ALL_IFACES(code) \
	{ vector<IfaceConnection*>::iterator if_it = mIfaces.begin(); \
	  for(; if_it != mIfaces.end(); ++if_it) { \
	  (*if_it)->code; } \
	}

#define ALL_IFACES_EX(conn, code) \
	{ vector<IfaceConnection*>::iterator if_it = mIfaces.begin(); \
	  for(; if_it != mIfaces.end(); ++if_it) \
	   if(*if_it != conn) \
	    (*if_it)->code; \
	}


void Museekd::cb_iface_ping(IfaceConnection* conn, uint32 id) {
	conn->ping(id);
}

void Museekd::cb_iface_login(IfaceConnection* conn, const string& algorithm, const string& chresponse) {
	string ch = conn->get_challenge() + mIfacePassword;
	
	unsigned char digest[32];
	uint digest_len = 0;
	
	if(algorithm == "SHA1") {
		shaBlock((unsigned char*)ch.data(), ch.size(), digest);
		digest_len = 20;
	} else if(algorithm == "SHA256") {
		sha256Block((unsigned char*)ch.data(), ch.size(), digest);
		digest_len = 32;
	} else if(algorithm == "MD5") {
		md5Block((unsigned char*)ch.data(), ch.size(), digest);
		digest_len = 16;
	} else {
		conn->login_reply(false, "INVHASH", challenge());
		return;
	}
	
	char hexdigest[65];
	hexDigest(digest, digest_len, hexdigest);
	if(chresponse != hexdigest) {
		conn->login_reply(false, "INVPASS", challenge());
		return;
	}
	
	conn->login_reply(true, string(), mIfacePassword);
	
	conn->server_state(logged_in(), mUsername);
	conn->config_state(mConfig);
	conn->room_state(mRoomList, mJoinedRooms,  mRecoder->utf8_tickermap(mTickers));
	conn->transfer_state(mTransferManager->uploads(), mTransferManager->downloads());
	
	if(mLoggedIn) {
		conn->status_set(mPeerManager->get_peer(mUsername)->status() & 1);
		vector<string> buddies = *mBuddies;
		vector<string>::const_iterator it = buddies.begin();
		for(; it != buddies.end(); ++it) {
			Peer* peer = mPeerManager->get_peer(*it);
			if(peer->have_exists())
				conn->peer_exists(*it, peer->exists());
			if(peer->have_status())
				conn->peer_status(*it, peer->status());
			if(peer->have_stats())
				conn->peer_stats(*it, peer->avgspeed(), peer->downloadnum(), peer->files(), peer->dirs());
		}
		vector<string>::const_iterator itt = mTrusted.begin();
		for(; itt != mTrusted.end(); ++itt) {
			Peer* peer = mPeerManager->get_peer(*itt);
			if(peer->have_exists())
				conn->peer_exists(*itt, peer->exists());
			if(peer->have_status())
				conn->peer_status(*itt, peer->status());
			if(peer->have_stats())
				conn->peer_stats(*itt, peer->avgspeed(), peer->downloadnum(), peer->files(), peer->dirs());
		}
		if(conn->mask() & EM_PRIVATE) {
			vector<PrivateMessage>::iterator it = mPrivateMessages.begin();
			for(; it != mPrivateMessages.end(); ++it) {
				conn->private_message(0, (*it).timestamp + (mPonged ? mPingTime : 0), (*it).user, mRecoder->encode_utf8((*it).message));
				server_ack_private((*it).ticket);
			}
			mPrivateMessages.clear();
		}
		string _msg = ("Museek Daemon Version: "+ mVersion );
		conn->status_message(0, _msg );
	}
}


void Museekd::cb_iface_check_privileges(IfaceConnection*) {
	server_check_privileges();
}

void Museekd::cb_iface_set_status(IfaceConnection *conn, uint32 status) {
	server_set_status(status ? 1 : 2);
}

void Museekd::cb_iface_config_set(IfaceConnection* conn, const string& domain, const string& key, const string& value) {
	mConfig[domain][key] = value;
	mConfig.store();
	
	// added logging config change (darrik@mythofbutterfly.com)
	if (domain == "logging")
	{
		if (key == "output") {
			if (value == "1") {
				mulog.setOutput(1);
			} else {
				mulog.setOutput(0);
			}
		} else if (key == "syslog_facility") {
			mulog.setSyslogFacility(atoi(value.data()));
		} else if (key == "syslog_priority") {
			mulog.setSyslogPriority(atoi(value.data()));
		}
	// end logging config change
	} else if(domain == "buddies")
	{
		ALL_IFACES(config_set(domain, key, value));
		mBuddies->add(key);
		
		if(Peer* peer = mPeerManager->get_peer(key, false))
		{
			if(peer->have_exists())
				ALL_IFACES(peer_exists(key, peer->exists()));
			if(peer->have_status())
				ALL_IFACES(peer_status(key, peer->status()));
			if(peer->have_stats())
				ALL_IFACES(peer_stats(key, peer->avgspeed(), peer->downloadnum(), peer->files(), peer->dirs()));
		}
		
		return;
	} else if(domain == "trusted") {
		trust(key);
	} else if(domain == "encoding.users")
		mRecoder->set_user(key, value);
	else if(domain == "encoding.rooms")
		mRecoder->set_room(key, value);
	else if(domain == "banned")
		ban(key);
	else if(domain == "transfers") {
		if(key == "upload_slots") {
			set_upload_slots(mConfig[domain][key].asInt());
			mTransferManager->check_uploads();
		} else if(key == "privilege_buddies") {
			set_privilege_buddies(mConfig[domain][key].asBool());
		} else if(key == "user_warnings") {
			set_user_warnings(mConfig[domain][key].asBool());
		} else if(key == "only_buddies") {
			set_only_buddies(mConfig[domain][key].asBool());
			mu_set_only_buddies(mConfig[domain][key].asBool());
		} else if(key == "trusting_uploads") {
			set_trusting_uploads(mConfig[domain][key].asBool());
		} else if(key == "have_buddy_shares") {
			mHaveBuddyShares = mConfig[domain][key].asBool();
			set_have_buddy_shares(mHaveBuddyShares);
		}
	} else if(domain == "clients" && key == "connectmode") {
		if (value == "active")
			set_connect_mode(CM_Active);
		else if (value == "passive")
			set_connect_mode(CM_Passive);
	} else if(domain == "userinfo" && key == "text")
		mUserinfo = mRecoder->decode_utf8(value);
	
	ALL_IFACES(config_set(domain, key, value));
}

void Museekd::cb_iface_config_remove(IfaceConnection* conn, const string& domain, const string& key) {
	mConfig[domain].remove(key);
	mConfig.store();
	
	if(domain == "buddies")
		mBuddies->del(key);
	else if(domain == "encoding.users")
		mRecoder->unset_user(key);
	else if(domain == "encoding.rooms")
		mRecoder->unset_room(key);
	else if(domain == "banned")
		unban(key);
	else if(domain == "trusted")
		untrust(key);
	
	ALL_IFACES(config_remove(domain, key));
}

void Museekd::cb_iface_config_set_user_image(IfaceConnection*, const vector<unsigned char>& data) {
	set_userpic(data);
	FILE *f = fopen(mConfig["userinfo"]["image"], "w");
	if(f) {
		char *buf = new char[data.size()];
		vector<unsigned char>::const_iterator it, end = data.end();
		int i = 0;
		for(it = data.begin(); it != end; ++it)
			buf[i++] = *it;
		fwrite(buf, 1, data.size(), f);
		fclose(f);
		delete [] buf;
	}
}



void Museekd::cb_iface_peer_exists(IfaceConnection* conn, const string& user) {
	Peer* peer = mPeerManager->get_peer(user);
	if(peer->have_exists()) {
		conn->peer_exists(user, peer->exists());
	} else {
		peer->subscribe();
	}
}

void Museekd::cb_iface_peer_status(IfaceConnection* conn, const string& user) {
	Peer* peer = mPeerManager->get_peer(user);
	if(peer->have_status()) {
		conn->peer_status(user, peer->status());
	} else {
		server_get_peer_status(user);
	}
}

void Museekd::cb_iface_peer_stats(IfaceConnection* conn, const string& user) {
	Peer* peer = mPeerManager->get_peer(user);
	if(peer->have_stats()) {
		conn->peer_stats(user, peer->avgspeed(), peer->downloadnum(), peer->files(), peer->dirs());
	} else {
		server_get_peer_stats(user);
	}
}

void Museekd::cb_iface_peer_address(IfaceConnection* conn, const string& user) {
	Peer* peer = mPeerManager->get_peer(user);
// 	if(peer->have_address()) {
// 		conn->peer_address(user, peer->ip(), peer->port());
// 	} else {
	server_get_peer_address(user);
	}
}

void Museekd::cb_iface_info(IfaceConnection* conn, const string& user) {
	if(user == mUsername) {
		mPeerManager->get_peer(user)->connection()->local_userinfo_request();
		server_get_peer_interests(user);
	} else {
		mPeerManager->get_peer(user)->connection()->info();
		server_get_peer_interests(user);
	}
}

void Museekd::cb_iface_shares(IfaceConnection* conn, const string& user) {
	if(user == mUsername) {
		mPeerManager->get_peer(user)->connection()->local_shares_request();
	} else {
		mPeerManager->get_peer(user)->connection()->shares();
	}
}

void Museekd::cb_iface_give_privileges(IfaceConnection* conn, const string& user, uint32 days) {
	server_give_privileges(user, days);
}


void Museekd::cb_iface_room_list(IfaceConnection* coon) {
	server_room_list();
}

void Museekd::cb_iface_get_global_recommendations(IfaceConnection* coon) {
	server_get_global_recommendations();
}

void Museekd::cb_iface_get_recommendations(IfaceConnection* coon) {
	server_get_recommendations();
}

void Museekd::cb_iface_get_similar_users(IfaceConnection* coon) {
	server_get_similar_users();
}

void Museekd::cb_iface_get_item_recommendations(IfaceConnection* coon, const string& item) {
	server_get_item_recommendations(item);
}

void Museekd::cb_iface_get_item_similar_users(IfaceConnection* coon, const string& item) {
	server_get_item_similar_users(item);
}
void Museekd::cb_iface_private(IfaceConnection* conn, const string& user, const string& message) {
	server_send_private(user, mRecoder->decode_utf8(message));
	ALL_IFACES_EX(conn, private_message(1, time(NULL), user, message));
}

void Museekd::cb_iface_join_room(IfaceConnection* conn, const string& room) {
	map<string, RoomData>::iterator it = mJoinedRooms.find(room);
	if(it != mJoinedRooms.end())
		return;
	
	if(mJoined)
		server_join_room(room);
	else {
		mJoinedRooms[room];
		mTickers[room];
		ALL_IFACES(joined_room(room, mJoinedRooms[room]));
	}
}

void Museekd::cb_iface_leave_room(IfaceConnection* conn, const string& room) {
	map<string, RoomData>::iterator it = mJoinedRooms.find(room);
	if(it == mJoinedRooms.end())
		return;
	
	if(mJoined)
		server_leave_room(room);
	else {
		mJoinedRooms.erase(it);
		mTickers.erase(room);
		ALL_IFACES(left_room(room));
	}
		
}

void Museekd::cb_iface_remove_interest(IfaceConnection* conn, const string& interest) {
	server_remove_interest(interest);
	mConfig["interests.like"].remove(interest);
	mConfig.store();
	ALL_IFACES(config_remove("interests.like", interest));
}
void Museekd::cb_iface_remove_hated_interest(IfaceConnection* conn, const string& interest) {
	server_remove_hated_interest(interest);
	mConfig["interests.hate"].remove(interest);
	mConfig.store();
	ALL_IFACES(config_remove("interests.hate", interest));
}
void Museekd::cb_iface_add_interest(IfaceConnection* conn, const string& interest) {
	server_add_interest(interest);
	mConfig["interests.like"][interest];
	mConfig.store();
	ALL_IFACES(config_set("interests.like", interest, ""));
}
void Museekd::cb_iface_add_hated_interest(IfaceConnection* conn, const string& interest) {
	server_add_hated_interest(interest);
	mConfig["interests.hate"][interest];
	mConfig.store();
	ALL_IFACES(config_set("interests.hate", interest, ""));
}

void Museekd::cb_server_remove_interest(const string& interest) {

}
void Museekd::cb_server_remove_hated_interest(const string& interest) {

}

void Museekd::cb_server_add_interest(const string& interest) {

}

void Museekd::cb_server_add_hated_interest(const string& interest) {

}

void Museekd::cb_peer_transfer_finished(const wstring& path, const string& user) {
	string _path = mRecoder->encode_utf8(path);
	string _msg = ("Finished downloading "+ _path +" from "+ user );
	ALL_IFACES(status_message(1, _msg ));
}

void Museekd::cb_peer_banned(const string& user) {
	string _msg = ("Refused to send shares to: "+ user );
	ALL_IFACES(status_message(1, _msg ));
	peer_banned(user);
}
void Museekd::cb_peer_upload_blocked(const string& user) {
	string _msg = ("Refused to accept file from non-trusted user: "+ user );
	ALL_IFACES(status_message(1, _msg ));
	peer_upload_blocked(user);
}

void Museekd::cb_peer_sent_buddy_shares(const string& user) {
	string _msg = ("Buddy Shares sent to: "+ user );
	ALL_IFACES(status_message(1, _msg ));
	
}
void Museekd::cb_peer_sent_normal_shares(const string& user) {
	string _msg = ("Normal Shares sent to: "+ user );
	ALL_IFACES(status_message(1, _msg ));
	
}
void Museekd::cb_peer_sent_user_info(const string& user) {
	string _msg = ("User Info sent to: "+ user );
	ALL_IFACES(status_message(1, _msg ));
}
void Museekd::cb_server_global_message(const string& message) {
	string _msg = ("Global Message: "+ message );
	ALL_IFACES(status_message(0, _msg ));
}

void Museekd::msg_server_privileged_users(uint32 _size) {
	string _msg = string("Privileged users: " + itos(_size) );
	ALL_IFACES(status_message(0, _msg));
}


void Museekd::cb_iface_say_room(IfaceConnection* conn, const string& room, const string& message) {
	if(mJoinedRooms.find(room) == mJoinedRooms.end() || message.empty())
		return;
	server_say_room(room, mRecoder->decode_utf8(message));
}

void Museekd::cb_iface_room_ticker_set(IfaceConnection* conn, const string& room, const string& message) {
	if(mJoinedRooms.find(room) == mJoinedRooms.end())
		return;
	server_set_ticker(room, mRecoder->decode_utf8(message));
}


void Museekd::cb_iface_user_search(IfaceConnection* conn, const string& user, const string& query) {
	uint32 ticket = token();
	mSearches[ticket] = conn;
	conn->search(query, ticket);
	wstring wquery = mRecoder->decode_utf8(query);
	server_user_search(user, ticket, wquery);
}
void Museekd::cb_iface_wishlist_search(IfaceConnection* conn, const string& query) {
	uint32 ticket = token();
	mSearches[ticket] = conn;
	conn->search(query, ticket);
	wstring wquery = mRecoder->decode_utf8(query);
	server_wishlist_search(ticket, wquery);
}

void Museekd::cb_iface_search(IfaceConnection* conn, uint32 type, const string& query) {
	uint32 ticket = token();
	mSearches[ticket] = conn;
	conn->search(query, ticket);
	
	wstring wquery = mRecoder->decode_utf8(query);
	if(type == 0)
	{
		server_search(ticket, wquery);
	} else if(type == 1) {
		vector<string> buddies = *mBuddies;
		vector<string>::iterator it, end = buddies.end();
		for(it = buddies.begin(); it != end; ++it)
			if (mConfig["server"]["username"] != *it)
				server_user_search(*it, ticket, wquery);
				//mPeerManager->get_peer(*it)->connection()->search(ticket, wquery);
	} else if (type == 2) {
		
		DEBUG ("Room search : %s", query.c_str ());

		set<string> room_users;
		map<string, RoomData>::iterator it, end = mJoinedRooms.end ();

		for (it = mJoinedRooms.begin() ; it != end ; ++it)
			{ 
			DEBUG ("Room search : %s", (*it).first.c_str());
			server_room_search( (*it).first,  ticket, wquery);
// 			DEBUG ("Adding users from room %s...", (*it).first.c_str ());
// 			RoomData rdata = (*it).second;
// 
// 			map<string, UserData>::iterator it2, end2 = rdata.end ();
// 
// 			for (it2 = rdata.begin () ; it2 != end2 ; ++it2)
// 				{
// 				UserData udata = (*it2).second;
// 
// 				if (mConfig["server"]["username"] != (*it2).first)
// 					room_users.insert (room_users.begin(), (*it2).first);
// 				}
 			}
// 
// 		set<string>::iterator it3, end3 = room_users.end();
// 		DEBUG ("Sending messages to the %d users...", room_users.size ());
// 		for(it3 = room_users.begin() ; it3 != end3 ; ++it3)
// 			mPeerManager->get_peer(*it3)->connection()->search(ticket, wquery);
	}
}

void Museekd::cb_iface_terminate(IfaceConnection* conn, const uint32 ticket) {
	map<uint32, IfaceConnection*>::const_iterator it = mSearches.find(ticket);
	if(it == mSearches.end() || (*it).second != conn)
		return;
	mSearches.erase(ticket);
}

	
void Museekd::cb_iface_transfer_update(IfaceConnection* conn, const string& user, const string& path) {
	mPeerManager->get_peer(user)->connection()->place_in_queue(mRecoder->decode_utf8(path));
}

void Museekd::cb_iface_transfer_delete(IfaceConnection* conn, bool upload, const string& user, const string& path) {
	Transfer* transfer;
	
	if(upload) {
		transfer = mPeerManager->get_peer(user)->upload(mRecoder->decode_utf8(path));
	} else {
		transfer = mPeerManager->get_peer(user)->download(mRecoder->decode_utf8(path));
	}
	
	if(transfer) {
		transfer->set_state(TS_Aborted);
		delete transfer;
	}
}

void Museekd::cb_iface_upload_file(IfaceConnection* conn, const string& user, const string& path) {
	Peer *peer = mPeerManager->get_peer(user, true);
	if(peer) { 
		mPeerManager->get_peer(user)->connection()->upload_notification();
// 		const string pa = str_replace(path, '/', '\\');
		mTransferManager->send_upload(user, mRecoder->decode_utf8(path));
	}
}


void Museekd::cb_iface_download_file(IfaceConnection* conn, const string& user, const string& path) {
	if(! (user.empty() || path.empty()))
		mTransferManager->new_download(user, mRecoder->decode_utf8(path), wstring(), wstring(), 0);
}
void Museekd::cb_iface_download_file_to(IfaceConnection* conn, const string& user, const string& path, const string& dpath) {
	if(! (user.empty() || path.empty()))
		mTransferManager->new_download(user, mRecoder->decode_utf8(path), mRecoder->decode_utf8(dpath), wstring(), 0);
}

void Museekd::cb_iface_download_folder(IfaceConnection* conn, const string& user, const string& folder) {
	if(! (user.empty() || folder.empty()))
		mPeerManager->get_peer(user)->connection()->folder_contents(mRecoder->decode_utf8(folder));
}

void Museekd::cb_iface_transfer_abort(IfaceConnection*, bool upload, const string& user, const string& path) {
	Peer* peer = mPeerManager->get_peer(user, false);
	if(peer) {
		if(upload)
			peer->abort_upload(mRecoder->decode_utf8(path));
		else
			peer->abort_download(mRecoder->decode_utf8(path));
	}
}


void Museekd::reconnect() {
	struct itimerval ival;
	ival.it_value.tv_sec = mReconnectTime;
	
	ival.it_interval.tv_sec = 0;
	ival.it_interval.tv_usec = ival.it_value.tv_usec =  0;
	
	setitimer(ITIMER_REAL, &ival, NULL);
}


void Museekd::cb_iface_disconnect_server(IfaceConnection*) {
	mKicked = true;
	server_disconnect();
}
void Museekd::cb_iface_connect_server(IfaceConnection*) {
	do_reconnect();
}

void Museekd::cb_iface_reload_shares(IfaceConnection*) {
	DEBUG("reloading shares...");
	load_shares();
	DEBUG("reloading buddy shares...");
	load_buddy_shares();
	ALL_IFACES(status_message(0, "Reloaded Shares.." ));
}


void Museekd::do_reconnect() {
	mKicked = true;
	server_disconnect();
	DEBUG("Reconnecting: reloading server config");
	set_server(mConfig["server"]["host"],
	           mConfig["server"]["port"].asUint());
	
	set_username(mConfig["server"]["username"]);
	set_password(mConfig["server"]["password"]);
	server_connect();
	mKicked = false;
}
void Museekd::cb_server_disconnected() {
	Museek::cb_server_disconnected();
	
	/* NOTE: The private messages haven't been acked yet, we will get them again when we re-connect */
	mPrivateMessages.clear();
	
	map<string, RoomData>::iterator it = mJoinedRooms.begin();
	for(; it != mJoinedRooms.end(); ++it)
		(*it).second.clear();
	map<string, WTickers>::iterator tit = mTickers.begin();
	for(; tit != mTickers.end(); ++tit)
		(*tit).second.clear();
	mJoined = false;
	
	ALL_IFACES(server_state(false, ""));
	
	mReconnectTime = 15;
	
	
	if (! mKicked) {
		DEBUG("*sniff* server disconnected us, trying again in %i seconds", mReconnectTime);
		ALL_IFACES(status_message(0, "Server disconnected us, reconnecting in a bit."));
		reconnect();
		}
	else 
		ALL_IFACES(status_message(0, "Server disconnected us, not reconnecting."));
		

}

void Museekd::cb_server_kicked() {
	Museek::cb_server_kicked();
	
	mKicked = true;
	DEBUG("Forced the server to disconnect us, giving up");
	ALL_IFACES(status_message(0, "Disconnected from Server, not reconnecting."));
}	

void Museekd::cb_server_cannot_connect() {
	Museek::cb_server_cannot_connect();
	
	DEBUG("cannot connect to server, trying again in %i seconds", mReconnectTime);
	ALL_IFACES(status_message(0, "Could not connect to server, trying again in a bit."));
	reconnect();
	
	if(mReconnectTime < 60)
		mReconnectTime *= 2;
}

void Museekd::cb_server_cannot_login() {
	Museek::cb_server_cannot_login();
	
	DEBUG("timeout while logging in to the server", mReconnectTime);
	ALL_IFACES(status_message(0, "Timed out while logging into the server."));
	
	/* don't call reconnect here, since we're disconnected after we get here,
	   cb_server_disconnected() will do that for us */
}

void Museekd::cb_server_cannot_resolve() {
	Museek::cb_server_cannot_resolve();
	
	if(mReconnectTime < 60)
		mReconnectTime *= 2;
	
	DEBUG("cannot resolve server, trying again in %i seconds", mReconnectTime);
	ALL_IFACES(status_message(0, "cannot resolve server, trying again in a bit"));
	reconnect();
}

void Museekd::cb_server_logged_in(bool success, const string& message) {
	Museek::cb_server_logged_in(success, message);
	
	if(success)
	{
		mPonged = false;
		mPingTime = time(0);
		ALL_IFACES(server_state(true, mUsername));
		ALL_IFACES(status_message(0, "Logged into Server as "+ mUsername));
		ALL_IFACES(status_message(0, message));
		server_send_private(mUsername, wstring(L"\001"));
		
		vector<string> interests_like = mConfig["interests.like"].keys();
		vector<string> interests_hate = mConfig["interests.hate"].keys();

		vector<string>::const_iterator lit = interests_like.begin();
		for(; lit != interests_like.end(); ++lit) {
			server_add_interest(*lit);
		}
		
		vector<string>::const_iterator hit = interests_hate.begin();
		for(; hit != interests_hate.end(); ++hit) {
			server_add_hated_interest(*hit);
		}
	}
}

void Museekd::cb_server_privileges_left(uint32 time_left) {
	ALL_IFACES(privileges_left(time_left));
}

void Museekd::cb_server_peer_status(const string& user, uint32 status) {
	Museek::cb_server_peer_status(user, status);
	
	map<string, RoomData>::iterator r_it = mJoinedRooms.begin();
	for(; r_it != mJoinedRooms.end(); ++r_it) {
		RoomData::iterator u_it = (*r_it).second.find(user);
		if(u_it == (*r_it).second.end())
			continue;
		(*u_it).second.status = status;
	}
	
	ALL_IFACES(peer_status(user, status));
	
	if(user == mUsername)
		ALL_IFACES(status_set(status & 1));
}

void Museekd::cb_server_peer_stats(const string& user, uint32 avgspeed, uint32 downloadnum, uint32 files, uint32 dirs) {
	Museek::cb_server_peer_stats(user, avgspeed, downloadnum, files, dirs);
	
	map<string, RoomData>::iterator r_it = mJoinedRooms.begin();
	for(; r_it != mJoinedRooms.end(); ++r_it) {
		RoomData::iterator u_it = (*r_it).second.find(user);
		if(u_it == (*r_it).second.end())
			continue;
		(*u_it).second.avgspeed = avgspeed;
		(*u_it).second.downloadnum = downloadnum;
		(*u_it).second.files = files;
		(*u_it).second.dirs = dirs;
	}
	
	ALL_IFACES(peer_stats(user, avgspeed, downloadnum, files, dirs));
}

void Museekd::cb_server_peer_address(const string& user, const string& ip, uint32 port) {
	Museek::cb_server_peer_address(user, ip, port);
	
	ALL_IFACES(peer_address(user, ip, port));
}

void Museekd::cb_server_private(uint32 msg_id, uint32 timestamp, const string& user, const wstring& message) {
	if(! mPonged && user == mUsername && message == wstring(L"\001"))
	{
		mPonged = true;
		mPingTime -= timestamp;
		server_ack_private(msg_id);
		
		vector<IfaceConnection*> ifaces;
		
		vector<IfaceConnection*>::iterator iface = mIfaces.begin();
		for(; iface != mIfaces.end(); ++iface)
			if((*iface)->mask() & EM_PRIVATE)
				ifaces.push_back(*iface);
		
		if(ifaces.empty())
			return;
		
		vector<PrivateMessage>::iterator it, end = mPrivateMessages.end();
		for(it = mPrivateMessages.begin(); it != end; ++it)
		{
			for(iface = ifaces.begin(); iface != ifaces.end(); ++iface)
				(*iface)->private_message(0, (*it).timestamp + mPingTime, (*it).user, mRecoder->encode_utf8((*it).message));
			server_ack_private((*it).ticket);
		}
		
		return;
	}
	
	if(mConfig["ignored"].hasKey(user)) {
		server_ack_private(msg_id);
		return;
	}
	
	bool sent = false;
	
	if(mPonged)
	{
		vector<IfaceConnection*>::iterator it = mIfaces.begin();
		for(; it != mIfaces.end(); ++it) {
			if((*it)->mask() & EM_PRIVATE) {
				sent = true;
				(*it)->private_message(0, timestamp + mPingTime, user, mRecoder->encode_utf8(message));
			}
		}
	}
	
	if(sent) {
		server_ack_private(msg_id);
	} else {
		PrivateMessage pm;
		pm.ticket = msg_id;
		pm.timestamp = timestamp;
		pm.user = user;
		pm.message = message;
		mPrivateMessages.push_back(pm);
	}
}

void Museekd::cb_server_room_list(const RoomList& rooms) {
	mRoomList = rooms;
	
	if(! mJoined) {
		map<string, RoomData>::iterator it = mJoinedRooms.begin();
		for(; it != mJoinedRooms.end(); ++it) {
			(*it).second.clear();
			server_join_room((*it).first);
// 			if (mConfig["tickers"][(*it).first] ) {
// 				wstring tricky = mRecoder->decode_utf8(mConfig["tickers"][(*it).first]);
// 				server_set_ticker((*it).first.c_str(), tricky);
// 			} else {
// 				wstring tricky = mRecoder->decode_utf8(mConfig["default-ticker"]["ticker"]);
// 				server_set_ticker((*it).first.c_str(), tricky);
// 			}
		}
		mJoined = true;
	}
	
	ALL_IFACES(room_list(mRoomList));
}

void Museekd::cb_server_get_global_recommendations(const Recommendations& recommendations) {
	mRecommendations = recommendations;
	
	ALL_IFACES(get_global_recommendations(mRecommendations));
}

void Museekd::cb_server_get_recommendations(const string& interest, const Recommendations& recommendations) {
	mRecommendations = recommendations;
	
	ALL_IFACES(get_recommendations(mRecommendations));
}

void Museekd::cb_server_get_similar_users(const SimilarUsers& similarusers) {
	mSimilarUsers = similarusers;
	
	ALL_IFACES(get_similar_users(mSimilarUsers));
}

void Museekd::cb_server_get_item_similar_users(const string& item, const SimilarUsers& similarusers) {
	mSimilarUsers = similarusers;
	
	ALL_IFACES(get_item_similar_users(item, mSimilarUsers));
}

void Museekd::cb_server_get_item_recommendations(const string& item, const Recommendations& recommendations) {
	mRecommendations = recommendations;
	
	ALL_IFACES(get_item_recommendations(item, mRecommendations));
}

void Museekd::cb_server_joined_room(const string& room, const RoomData& users) {
	mJoinedRooms[room] = users;
	mTickers[room].clear();
	ALL_IFACES(joined_room(room, users));
	if ( mConfig["tickers"][room]) {
		wstring tricky = mRecoder->decode_utf8(mConfig["tickers"][room]);
		if (tricky.empty()) {
			wstring tricky = mRecoder->decode_utf8(mConfig["default-ticker"]["ticker"]);
			server_set_ticker(room.c_str(), tricky); }
		else
			server_set_ticker(room.c_str(), tricky);
	} else {
		wstring tricky = mRecoder->decode_utf8(mConfig["default-ticker"]["ticker"]);
		server_set_ticker(room.c_str(), tricky);
	}
}

void Museekd::cb_server_left_room(const string& room) {
	if(mJoinedRooms.find(room) == mJoinedRooms.end())
		return;
	
	mJoinedRooms.erase(room);
	mTickers.erase(room);
	ALL_IFACES(left_room(room));
}

void Museekd::cb_server_say_room(const string& room, const string& user, const wstring& message) {
	if(! mConfig["ignored"].hasKey(user)) {
		string _msg = mRecoder->encode_utf8(message);
		ALL_IFACES(say_room(room, user, _msg));
	}
}

void Museekd::cb_server_user_joined_room(const string& room, const string& user, const UserData& userdata) {
	map<string, RoomData>::iterator rit = mJoinedRooms.find(room);
	if(rit == mJoinedRooms.end())
		return;
	RoomData::const_iterator uit = (*rit).second.find(user);
	if(uit != (*rit).second.end())
		return;
	
	(*rit).second[user] = userdata;
	ALL_IFACES(user_joined_room(room, user, userdata));
}

void Museekd::cb_server_user_left_room(const string& room, const string& user) {
	map<string, RoomData>::iterator rit = mJoinedRooms.find(room);
	if(rit == mJoinedRooms.end())
		return;
	RoomData::const_iterator uit = (*rit).second.find(user);
	if(uit == (*rit).second.end())
		return;
	
	(*rit).second.erase(user);
	mTickers[room].erase(user);
	ALL_IFACES(user_left_room(room, user));
}

void Museekd::cb_server_room_tickers(const string& room, const WTickers& tickers) {
	if(mTickers.find(room) == mTickers.end())
		return;
	
	mTickers[room] = tickers;
	ALL_IFACES(room_tickers(room, mRecoder->utf8_tickers(tickers)));
}

void Museekd::cb_server_add_room_ticker(const string& room, const string& user, const wstring& message) {
	if(message.empty()) {
		cb_server_del_room_ticker(room, user);
		return;
	}
	
	map<string, WTickers>::iterator it = mTickers.find(room);
	if(it == mTickers.end())
		return;
	
	(*it).second[user] = message;
	ALL_IFACES(room_ticker_set(room, user, mRecoder->encode_utf8(message)));
}

void Museekd::cb_server_del_room_ticker(const string& room, const string& user) {
	map<string, WTickers>::iterator it = mTickers.find(room);
	if(it == mTickers.end())
		return;
	WTickers::iterator tit = (*it).second.find(user);
	if(tit == (*it).second.end())
		return;
	
	(*it).second.erase(tit);
	ALL_IFACES(room_ticker_set(room, user, ""));
}



void Museekd::cb_peer_shares(const string& user, const WShares& shares) {
	ALL_IFACES(shares(user, mRecoder->utf8_shares(shares)));
}

void Museekd::cb_peer_info(const string& user, const wstring& info, const vector<unsigned char>& pic, uint32 totalupl, uint32 queuelen, bool slotfree) {
	ALL_IFACES(info(user, mRecoder->encode_utf8(info), pic, totalupl, queuelen, slotfree));
}

void Museekd::cb_peer_results(uint32 ticket, const string& user, const WFolder& results, uint32 avgspeed, uint32 queuelen, bool slotfree) {
	map<uint32, IfaceConnection*>::const_iterator it = mSearches.find(ticket);
	if(it == mSearches.end()) {
		DEBUG("couldn't find search ticket %u", ticket);
	} else {
		DEBUG("found search ticket %u", ticket);
		(*it).second->results(ticket, user, mRecoder->utf8_folder(results), avgspeed, queuelen, slotfree);
	}
}

void Museekd::cb_peer_folder_contents(const string& user, const wstring& dir, const WShares& contents) {
	wstring path = dir;
	
	if(path[path.size() - 1] == '\\')
		path = path.substr(0, path.size() - 1);
	path = transfer_manager()->download_dir() + path.substr(path.rfind('\\') + 1);
	DEBUG("folder: %s", mRecoder->encode_filesystem(path).c_str());
	
	WShares::const_iterator it = contents.begin();
	for(; it != contents.end(); ++it) {
		wstring subdir = (*it).first;
		if(! subdir.size())
			subdir = '\\';
		if(subdir[subdir.size()-1] != '\\')
			subdir += '\\';
		wstring subpath = path;
		subpath += '/';
		if(subdir.size() > dir.size()) {
			subpath += str_replace(subdir.substr(dir.size()), '\\', '/');
		}
		DEBUG(" - %s", mRecoder->encode_filesystem(subpath).c_str());
		
		WFolder::const_iterator fit = (*it).second.begin();
		for(; fit != (*it).second.end(); ++fit) {
			DEBUG("  - %s", mRecoder->encode_filesystem(subpath + (*fit).first).c_str());
			transfer_manager()->new_download(user, subdir + (*fit).first, subpath + (*fit).first, wstring(), (*fit).second.size);
		}
	}
}

void Museekd::cb_transfer_update(const Transfer* transfer) {
	ALL_IFACES(transfer_update(transfer));
	
	if(! mDontSaveDownloads && transfer->direction() == Transfer::Download && transfer->state() != TS_Transferring) {
		mLastSave = time(0);
		mSaveDownloads = true;
	}
}

void Museekd::cb_transfer_delete(const Transfer* transfer) {
	ALL_IFACES(transfer_delete(transfer));
	if(! mDontSaveDownloads && transfer->direction() == Transfer::Download) {
		mLastSave = time(0);
		mSaveDownloads = true;
	}
}

static inline int write_int(int fd, uint32 i) {
	unsigned char d[4];
	for(int j = 0; j < 4; j++) {
		d[j] = i & 0xff;
		i = i >> 8;
	}
	return write(fd, d, 4);
}

static inline int write_off(int fd, off_t i) {
	unsigned char d[8];
	for(int j = 0; j < 8; j++) {
		d[j] = i & 0xff;
		i = i >> 8;
	}
	return write(fd, d, 8);
}

static inline int write_str(int fd, const string& str) {
	if(write_int(fd, str.size()) == -1)
		return -1;
	const char* d = str.data();
	return write(fd, d, str.size());
}

static inline int read_int(int fd, uint32* r) {
	unsigned char d[4];
	*r = 0;
	if(read(fd, d, 4) == -1)
		return -1;
	for(uint32 j = 0; j < 4; j++)
		(*r) += d[j] << (j * 8);
	return 4;
}

static inline int read_off(int fd, off_t* r) {
	unsigned char d[8];
	(*r) = 0;
	if(read(fd, d, 8) == -1)
		return -1;
	for(off_t j = 0; j < 8; j++)
		(*r) += d[j] << (j * 8);
	return 8;
}

static inline int read_str(int fd, string& r) {
	uint32 len;
	if(read_int(fd, &len) == -1)
		return -1;
	
	char d[len];
	if(read(fd, d, len) == -1)
		return -1;
	r.append(d, len);
	
	return len + 4;
}

void Museekd::load_downloads() {
	int fd = open(mConfig["transfers"]["downloads"], 0);
	if(fd == -1) {
		DEBUG("Cannot load downloads. Bailing out");
		return;
	}
	uint32 n;
	if(read_int(fd, &n) == -1) {
		DEBUG("Cannot load downloads. Bailing out");
		return;
	}
	while(n) {
		uint32 state;
		off_t size;
		string user, path, localpath, temppath;
		if(read_int(fd, &state) == -1 || \
		   read_str(fd, user) == -1 || \
		   read_off(fd, &size) == -1 || \
		   read_str(fd, path) == -1 || \
		   read_str(fd, localpath) == -1 || \
		   read_str(fd, temppath) == -1) {
			DEBUG("Cannot load downloads. Bailing out");
			return;
		}
		mDontSaveDownloads = true;
		Transfer* tr = mTransferManager->new_download(user, mRecoder->decode_utf8(path), mRecoder->decode_utf8(localpath), mRecoder->decode_utf8(temppath), size);
		if(state == 0)
			tr->set_state(TS_Aborted);
		mDontSaveDownloads = false;
		n--;
	}
}

void Museekd::save_downloads() {
	int fd = creat(mConfig["transfers"]["downloads"], S_IRUSR | S_IWUSR);
	if(fd == -1) {
		DEBUG("Cannot save downloads, trying again later");
		return;
	}
	
	uint32 transfers = 0;
	
	vector<Transfer*>::const_iterator it = mTransferManager->downloads().begin();
	for(; it != mTransferManager->downloads().end(); ++it)
		if((*it)->state() != TS_Finished)
			transfers++;
	
	if(write_int(fd, transfers) == -1) {
		DEBUG("Cannot save downloads, trying again later. Leaving downloads file in corrupted state");
		close(fd);
		return;
	}
	
	for(it = mTransferManager->downloads().begin(); it != mTransferManager->downloads().end(); ++it) {
		uint32 state;
		switch((*it)->state()) {
		case TS_Finished:
			continue;
		case TS_Aborted:
			state = 0;
			break;
		default:
			state = 1;
			break;
		}
		
		if(write_int(fd, state) == -1 || \
		   write_str(fd, (*it)->peer()->user()) == -1 || \
		   write_off(fd, (*it)->size()) == -1 || \
		   write_str(fd, (*it)->path_utf8()) == -1 || \
		   write_str(fd, mRecoder->encode_utf8((*it)->local_path())) == -1 || \
		   write_str(fd, mRecoder->encode_utf8((*it)->temp_path())) == -1) {
			DEBUG("Cannot save downloads, trying again later. Leaving downloads file in corrupted state");
			close(fd);
			return;
		}
	}
	close(fd);
	mSaveDownloads = false;
}

bool Museekd::is_privileged(const string& user) {
	if((mOnlyBuddies || mPrivilegeBuddies) && mBuddies->has(user))
		return true;
	else
		return Museek::is_privileged(user);
}

bool Museekd::is_buddied(const string& user) const {
	if( mBuddies->has(user))
		return true;
	else
		return Museek::is_buddied(user);
}

bool Museekd::is_trusted(const string& user) const {
	if(mTrustedUploads )
		return find(mTrusted.begin(), mTrusted.end(), user) != mTrusted.end();
	else
		return false;
}


void Museekd::trust(const string& user) {
	if(find(mTrusted.begin(), mTrusted.end(), user) != mTrusted.end())
		return;
	mTrusted.push_back(user);
	

}

void Museekd::untrust(const string& user) {
	vector<string>::iterator it = find(mTrusted.begin(), mTrusted.end(), user);
	if(it != mTrusted.end())
		mTrusted.erase(it);
}

void Museekd::ban(const string& user) {
	if(find(mBanned.begin(), mBanned.end(), user) != mBanned.end())
		return;
	mBanned.push_back(user);
	
	Peer *peer = mPeerManager->get_peer(user, false);
	if(peer)
		mTransferManager->ban_user(peer);
}

void Museekd::unban(const string& user) {
	vector<string>::iterator it = find(mBanned.begin(), mBanned.end(), user);
	if(it != mBanned.end())
		mBanned.erase(it);
}

void Museekd::remove_receiving(const string& user) {
	vector<string>::iterator it = find(mReceiving.begin(), mReceiving.end(), user);
	if(it != mReceiving.end())
		mReceiving.erase(it);
// 	remove_receiving(user);
}
void Museekd::add_receiving(const string& user) {
	DEBUG ("Add to Receiving list: %s", user.c_str ());
	if(find(mReceiving.begin(), mReceiving.end(), user) != mReceiving.end())
		return;
	mReceiving.push_back(user);
}
bool Museekd::is_receiving_shares(const string& user) const {
	return find(mReceiving.begin(), mReceiving.end(), user) != mReceiving.end();
}
bool Museekd::is_banned(const string& user) const {
	if(mOnlyBuddies && ! mBuddies->has(user))
		return true;
	else
		return find(mBanned.begin(), mBanned.end(), user) != mBanned.end();
}

void Museekd::set_privilege_buddies(bool b) {
	if(mPrivilegeBuddies == b)
		return;
	mPrivilegeBuddies = b;
	update_buddies();
}

void Museekd::set_user_warnings(bool b) {
	if(mUserWarnings == b)
		return;
	mUserWarnings = b;
}

void Museekd::set_only_buddies(bool b) {
	if(mOnlyBuddies == b)
		return;
	mOnlyBuddies = b;
	update_buddies();
}
void Museekd::set_trusting_uploads(bool b) {
	if(mTrustedUploads == b)
		return;
	mTrustedUploads = b;
}


void Museekd::update_buddies() {
	map<string, Peer*>& peers = mPeerManager->peers();
	map<string, Peer*>::iterator it, end = peers.end();
	for(it = peers.begin(); it != end; ++it) {
		(*it).second->set_privileged(is_privileged((*it).first));
		if(mOnlyBuddies && ! mBuddies->has((*it).first))
			mTransferManager->ban_user((*it).second);
	}
}


void Museekd::cycle_callback() {
	Museek::cycle_callback();
	
	time_t curtime = time(0);
	if(mSaveDownloads && curtime > mLastSave) {
		save_downloads();
		mLastSave = curtime;
	}
}

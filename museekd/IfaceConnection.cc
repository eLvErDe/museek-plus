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

#include <museekd/IfaceConnection.hh>
#include <museekd/IfaceMessages.hh>
#include <museekd/museekd.hh>
#include <museekd/MuseekdTypes.hh>
#include <Mucipher/mucipher.h>

#include <string>
#include <map>
#include <vector>

#define MULOG_DOMAIN "museekd.IL"
#include <Muhelp/Mulog.hh>

#ifdef PARSE
# undef PARSE
#endif
#define PARSE(mtype) mtype s; s.parse_network_packet(message);
#define CPARSE(mtype) mtype s(&mContext); s.parse_network_packet(message);

using std::string;
using std::map;
using std::vector;

IfaceListener::IfaceListener(Museekd *_museekd, const string& _path)
              : ListenConnection(), mMuseekd(_museekd) {
	if(_path.size() == 0) {
		error = ERR_LOOKUP_FAILED;
		return;
	}
	
	if(_path[0] == '/') {
#ifdef HAVE_SYS_UN_H
		listen_unix(_path.c_str());
#else
		DEBUG("no unix socket support");
#endif
 	} else {
 		int ix = _path.find(":"),
 		    port = atoi(_path.substr(ix+1).c_str());
 		string host = _path.substr(0, ix);
 		if(port != 0)
 			listen(host, port);
 		else {
 			DEBUG("invalid tcp host: %s", _path.c_str());
 			error = ERR_LOOKUP_FAILED;
 		}
	}
}

bool IfaceListener::accept() {
	DEBUG("IFACELISTENER::accept");
	
	int sock = simple_accept();
	if(sock == -1)
		return false;
	
	new IfaceConnection(mMuseekd, sock);
	return true;
}



#undef MULOG_DOMAIN
#define MULOG_DOMAIN "museekd.IC"

IfaceConnection::IfaceConnection(Museekd *museekd, int sock)
                : MessageConnection(4), mMuseekd(museekd), mAuthenticated(false) {
	CT("IfaceConnection %i", sock);
	
	set_socket(sock);
	mMuseekd->new_iface(this);
}

void IfaceConnection::disconnected() {
	CT("disconnected");
	
	mMuseekd->remove_iface(this);
}

void IfaceConnection::process_message(uint32 code) {
	CT("process_message");
	
	if(! mAuthenticated && code != 0x0002) {
		DEBUG("attempted message while not authenticated");
		disconnect();
		return;
	}
	switch(code) {
	case 0x0000: {
		PARSE(IPing)
		DEBUG("got ping request %u", s.id);
		
		mMuseekd->cb_iface_ping(this, s.id);
		break;
	}
	case 0x0002: {
		PARSE(ILogin)
		DEBUG("got login attempted %s, %s, %08x", s.algorithm.c_str(), s.chresponse.c_str(), s.mask);
		
		mMask = s.mask;
		mMuseekd->cb_iface_login(this, s.algorithm, s.chresponse);
		break;
	}
	case 0x0004: {
		DEBUG("got check privileges");
		mMuseekd->cb_iface_check_privileges(this);
		break;
	}
	case 0x0005: {
		PARSE(ISetStatus)
		DEBUG("got set status %u", s.status);
		mMuseekd->cb_iface_set_status(this, s.status);
		break;
	}
	
	case 0x0101: {
		CPARSE(IConfigSet)
		DEBUG("got config set %s, %s, %s", s.domain.c_str(), s.key.c_str(), s.value.c_str());
		
		mMuseekd->cb_iface_config_set(this, s.domain, s.key, s.value);
		break;
	}
	case 0x0102: {
		CPARSE(IConfigRemove)
		DEBUG("got config remove key %s, %s", s.domain.c_str(), s.key.c_str());
		
		mMuseekd->cb_iface_config_remove(this, s.domain, s.key);
		break;
	}
	case 0x0103: {
		PARSE(IConfigSetUserImage)
		DEBUG("got user image <...> (%d)", s.mData.size());
		
		mMuseekd->cb_iface_config_set_user_image(this, s.mData);
		break;
	}
	
	case 0x0201: {
		PARSE(IPeerExists)
		DEBUG("got get peer exists %s", s.user.c_str());
		
		mMuseekd->cb_iface_peer_exists(this, s.user);
		break;
	}
	case 0x0202: {
		PARSE(IPeerStatus)
		DEBUG("got get peer status %s", s.user.c_str());
		
		mMuseekd->cb_iface_peer_status(this, s.user);
		break;
	}
	case 0x0203: {
		PARSE(IPeerStats)
		DEBUG("got get user stats %s", s.user.c_str());
		
		mMuseekd->cb_iface_peer_stats(this, s.user);
		break;
	}
	case 0x0204: {
		PARSE(IUserInfo)
		DEBUG("got get user info %s", s.user.c_str());
		
		mMuseekd->cb_iface_info(this, s.user);
		break;
	}
	case 0x0205: {
		PARSE(IUserShares)
		DEBUG("got get user shares %s", s.user.c_str());
		
		mMuseekd->cb_iface_shares(this, s.user);
		break;
	}
	case 0x0206: {
		PARSE(IPeerAddress)
		DEBUG("got get user address %s", s.user.c_str());
		
		mMuseekd->cb_iface_peer_address(this, s.user);
		break;
	}
	case 0x0207: {
		PARSE(IGivePrivileges)
		DEBUG("got give privileges %s. %i", s.user.c_str(), s.days);
		
		mMuseekd->cb_iface_give_privileges(this, s.user, s.days);
		break;
	}
	
	case 0x0301: {
		DEBUG("got update room list");
		
		mMuseekd->cb_iface_room_list(this);
		break;
	}
	case 0x0302: {
		PARSE(IPrivateMessage)
		DEBUG("got private message %s, %s", s.user.c_str(), s.msg.c_str());
		
		mMuseekd->cb_iface_private(this, s.user, s.msg);
		break;
	}
	case 0x0303: {
		PARSE(IJoinRoom)
		DEBUG("got join room %s", s.room.c_str());
		
		mMuseekd->cb_iface_join_room(this, s.room);
		break;
	}
	case 0x0304: {
		PARSE(ILeaveRoom)
		DEBUG("got leave room %s", s.room.c_str());
		
		mMuseekd->cb_iface_leave_room(this, s.room);
		break;
	}
	case 0x0307: {
		PARSE(ISayRoom)
		DEBUG("got say room %s, %s", s.room.c_str(), s.line.c_str());
		
		mMuseekd->cb_iface_say_room(this, s.room, s.line);
		break;
	}
	case 0x0309: {
		PARSE(IRoomTickerSet)
		DEBUG("got set room ticker %s, %s", s.room.c_str(), s.message.c_str());
		
		mMuseekd->cb_iface_room_ticker_set(this, s.room, s.message);
		break;
	}
	
	case 0x0401: {
		PARSE(ISearch)
		DEBUG("got search request %u, %s", s.type, s.query.c_str());
		
		mMuseekd->cb_iface_search(this, s.type, s.query);
		break;
	}
	case 0x0402: {
		PARSE(ISearchReply)
		DEBUG("got terminate search request %u", s.ticket);
		
		mMuseekd->cb_iface_terminate(this, s.ticket);
		break;
	}
	case 0x0403: {
		PARSE(IUserSearch)
		DEBUG("got user search request %u, %s", s.user.c_str(), s.query.c_str());
		
		mMuseekd->cb_iface_user_search(this, s.user, s.query);
		break;
	}
	case 0x0405: {
		PARSE(IWishListSearch)
		DEBUG("got wishlist search request %s",  s.query.c_str());
		
		mMuseekd->cb_iface_wishlist_search(this,  s.query);
		break;
	}
	case 0x501: {
		PARSE(ITransferUpdate)
		DEBUG("got update transfer %s, %s", s.user.c_str(), s.path.c_str());
		mMuseekd->cb_iface_transfer_update(this, s.user, s.path);
		break;
	}
	case 0x0502: {
		PARSE(ITransferRemove);
		DEBUG("got remove transfer %i, %s, %s", s.upload, s.user.c_str(), s.path.c_str());
		
		mMuseekd->cb_iface_transfer_delete(this, s.upload, s.user, s.path);
		break;
	}
	case 0x0503: {
		PARSE(IDownloadFile)
		DEBUG("got download file %s. %s", s.user.c_str(), s.path.c_str());
		
		mMuseekd->cb_iface_download_file(this, s.user, s.path, s.size);
		break;
	}
	case 0x0504: {
		PARSE(IDownloadFolder)
		DEBUG("got get folder contents %s, %s", s.user.c_str(), s.folder.c_str());
		
		mMuseekd->cb_iface_download_folder(this, s.user, s.folder);
		break;
	}
	case 0x0505: {
		PARSE(ITransferAbort)
		DEBUG("got abort transfer %d, %s, %s", s.upload, s.user.c_str(), s.path.c_str());
		
		mMuseekd->cb_iface_transfer_abort(this, s.upload, s.user, s.path);
		break;
	}
	case 0x0506: {
		PARSE(IUploadFile)
		DEBUG("got upload file %s, %s", s.user.c_str(), s.path.c_str());
		
		mMuseekd->cb_iface_upload_file(this, s.user, s.path);
		break;
	}
	case 0x0600: {
		DEBUG("get update recommendations " );
		mMuseekd->cb_iface_get_recommendations(this);
		break;
	}
	case 0x0601: {
		DEBUG("get update global recommendations");
		mMuseekd->cb_iface_get_global_recommendations(this);
		break;
	}
	case 0x0602: {
		DEBUG("get similar users " );
		mMuseekd->cb_iface_get_similar_users(this);
		break;
	}
	case 0x0603: {
		PARSE(IGetItemRecommendations)
		DEBUG("get item recommendations %s ", s.item.c_str() );
		mMuseekd->cb_iface_get_item_recommendations(this, s.item);
		break;
	}
	case 0x0604: {
		PARSE(IGetItemSimilarUsers)
		DEBUG("get item similar users %s ", s.item.c_str() );
		mMuseekd->cb_iface_get_item_similar_users(this, s.item);
		break;
	}
	case 0x0610: {
		PARSE(IAddInterest)
		DEBUG("get add liked interest: %s", s.interest.c_str());
		mMuseekd->cb_iface_add_interest(this, s.interest);
		break;
	}
	case 0x0611: {
		PARSE(IRemoveInterest)
		DEBUG("remove liked interest: %s", s.interest.c_str());
		mMuseekd->cb_iface_remove_interest(this, s.interest);
		break;
	}
	case 0x0612: {
		PARSE(IAddHatedInterest)
		DEBUG("add hated interest: %s", s.interest.c_str());
		mMuseekd->cb_iface_add_hated_interest(this, s.interest);
		break;
	}
	case 0x0613: {
		PARSE(IRemoveHatedInterest)
		DEBUG("remove hated interest: %s", s.interest.c_str());
		mMuseekd->cb_iface_remove_hated_interest(this, s.interest);
		break;
	}
	case 0x0700: {
		DEBUG("got do connect server");
		mMuseekd->cb_iface_connect_server(this);
		break;
	}
	case 0x0701: {
		DEBUG("got do disconnect server");
		mMuseekd->cb_iface_disconnect_server(this);
		break;
	}
	case 0x0703: {
		DEBUG("got reload shares");
		mMuseekd->cb_iface_reload_shares(this);
		break;
	}
	default:
		DEBUG("received invalid message code %u", code);
		disconnect();
	}
}


void IfaceConnection::ping(uint32 id) {
	CT("ping");
	
	if(mAuthenticated) {
		IPing s(id);
		send(s);
	}
}

void IfaceConnection::challenge(uint version, const std::string& challenge) {
	CT("challenge");
	
	mChallenge = challenge;
	
	IChallenge s(version, challenge);
	send(s);
}

void IfaceConnection::login_reply(bool ok, const std::string& message, const std::string& new_challenge) {
	CT("login_reply %i, %s, %s", ok, message.c_str(), new_challenge.c_str());
	
	ILogin s;
	
	mAuthenticated = ok;
	if(! ok) {
		mChallenge = new_challenge;
		s = ILogin(ok, message, new_challenge);
	} else {
		cipherKeySHA256(&mContext, (char*)new_challenge.data(), new_challenge.size());
		s = ILogin(ok, string(), string());
	}
	
	send(s);
}

void IfaceConnection::server_state(bool logged_in, const std::string& username) {
	CT("server_state %i, %s", logged_in, username.c_str());
	
	if(mAuthenticated) {
		IServerState s(logged_in, username);
		send(s);
	}
}

void IfaceConnection::status_message(bool type, const std::string& message) {
	CT("status_message %i, %s", type, message.c_str());
	
	if(mAuthenticated) {
		IStatusMessage s(type, message);
		send(s);
	}
}

void IfaceConnection::room_state(const RoomList& roomlist, const map<string, RoomData>& rooms, const map<string, Tickers>& tickers) {
	CT("send_room_state %i, %i", roomlist.size(), rooms.size());
	
	if(mAuthenticated && mMask & EM_CHAT) {
		IRoomState cs = IRoomState(roomlist, rooms, tickers);
		send(cs);
	}
}


void IfaceConnection::config_state(const map<string, StringMap>& _n) {
	CT("config_state <...> (%d)", _n.size());
	
	if(mAuthenticated && mMask & EM_CONFIG) {
		IConfigState s(&mContext, _n);
		send(s);
	}
}

void IfaceConnection::config_set(const string& domain, const string& key, const string& value) {
	CT("config_set %s, %s, %s", domain.c_str(), key.c_str(), value.c_str());
	
	if(mAuthenticated && mMask & EM_CONFIG) {
		IConfigSet s(&mContext, domain, key, value);
		send(s);
	}
}

void IfaceConnection::config_remove(const string& domain, const string& key) {
	CT("config_remove %s, %s", domain.c_str(), key.c_str());
	
	if(mAuthenticated && mMask & EM_CONFIG) {
		IConfigRemove s(&mContext, domain, key);
		send(s);
	}
}

void IfaceConnection::search(const string& query, uint32 ticket) {
	CT("search %s, %u", query.c_str(), ticket);
	
	if(mAuthenticated) {
		ISearch s(query, ticket);
		send(s);
	}
}

void IfaceConnection::results(uint32 ticket, const string& user, const Folder& results, uint32 avgspeed, uint32 queuelen, bool slotfree) {
	CT("results %u, %s, %i, %i, %i, %i", ticket, user.c_str(), results.size(), avgspeed, queuelen, slotfree);
	
	if(mAuthenticated) {
		ISearchReply s(ticket, user, slotfree, avgspeed, queuelen, results);
		send(s);
	}
}

void IfaceConnection::room_list(const RoomList& rooms) {
	CT("room_list <...> (%i)", rooms.size());
	
	if(mAuthenticated && mMask & EM_CHAT) {
		IRoomList s(rooms);
		send(s);
	}
}

void IfaceConnection::get_recommendations(const Recommendations& recommendations) {
	CT("get_recommendations <...> (%i)", recommendations.size());
	
	if(mAuthenticated && mMask &  EM_INTERESTS) {
		IGetRecommendations s(recommendations);
		send(s);
	}
}

void IfaceConnection::get_global_recommendations(const Recommendations& recommendations) {
	CT("get_global_recommendations <...> (%i)", recommendations.size());
	
	if(mAuthenticated && mMask &  EM_INTERESTS) {
		IGetGlobalRecommendations s(recommendations);
		send(s);
	}
}

void IfaceConnection::get_similar_users(const SimilarUsers& similarusers) {
	CT("get_similar_users <...> (%i)", similarusers.size());
	
	if(mAuthenticated && mMask &  EM_INTERESTS) {
		IGetSimilarUsers s(similarusers);
		send(s);
	}
}
void IfaceConnection::get_item_similar_users(const string& item, const SimilarUsers& similarusers) {
	CT("get_item_similar_users for %s <...> (%i)", item.c_str(), similarusers.size());
	
	if(mAuthenticated && mMask &  EM_INTERESTS) {
		IGetItemSimilarUsers s(item, similarusers);
		send(s);
	}
}
void IfaceConnection::get_item_recommendations(const string& item, const Recommendations& recommendations) {
	CT("get_item_recommendations for %s <...> (%i)", item.c_str(),  recommendations.size());
	
	if(mAuthenticated && mMask &  EM_INTERESTS) {
		IGetItemRecommendations s(item, recommendations);
		send(s);
	}
}
void IfaceConnection::add_interest(const string& interest) {
	CT("add_interest (%s)", interest.c_str());
	
	if(mAuthenticated && mMask &  EM_INTERESTS) {
		IAddInterest s(interest);
		send(s);
	}
}

void IfaceConnection::add_hated_interest(const string& interest) {
	CT("add_hated_interest (%s)", interest.c_str());
	
	if(mAuthenticated && mMask &  EM_INTERESTS) {
		IAddHatedInterest s(interest);
		send(s);
	}
}

void IfaceConnection::remove_interest(const string& interest) {
	CT("remove_interest (%s)", interest.c_str());
	
	if(mAuthenticated && mMask &  EM_INTERESTS) {
		IRemoveInterest s(interest);
		send(s);
	}
}

void IfaceConnection::remove_hated_interest(const string& interest) {
	CT("remove_hated_interest (%s)", interest.c_str());
	
	if(mAuthenticated && mMask &  EM_INTERESTS) {
		IRemoveHatedInterest s(interest);
		send(s);
	}
}
void IfaceConnection::say_room(const string& room, const string& user, const string& line) {
	CT("say_room %s, %s, %s", room.c_str(), user.c_str(), line.c_str());
	
	if(mAuthenticated && mMask & EM_CHAT) {
		ISayRoom s(room, user, line);
		send(s);
	}
}

void IfaceConnection::joined_room(const string& room, const RoomData& users) {
	CT("joined_room %s, <...> (%d)", room.c_str(), users.size());
	
	if(mAuthenticated && mMask & EM_CHAT) {
		IJoinRoom s(room, users);
		send(s);
	}
}

void IfaceConnection::left_room(const string& room) {
	CT("left_room %s", room.c_str());
	
	if(mAuthenticated && mMask & EM_CHAT) {
		ILeaveRoom s(room);
		send(s);
	}
}

void IfaceConnection::user_joined_room(const string& room, const string& user, const UserData& data) {
	CT("user_joined_room %s, %s, <...>", room.c_str(), user.c_str());
	
	if(mAuthenticated && mMask & EM_CHAT) {
		IUserJoinedRoom s(room, user, data);
		send(s);
	}
}

void IfaceConnection::user_left_room(const string& room, const string& user) {
	CT("user_left_room %s, %s", room.c_str(), user.c_str());
	
	if(mAuthenticated && mMask & EM_CHAT) {
		IUserLeftRoom s(room, user);
		send(s);
	}
}

void IfaceConnection::private_message(uint32 direction, uint32 timestamp, const string& user, const string& message) {
	CT("private %u, %u, %s, %s", direction, timestamp, user.c_str(), message.c_str());
	
	if(mAuthenticated && mMask & EM_PRIVATE) {
		IPrivateMessage s(direction, timestamp, user, message);
		send(s);
	}
}

void IfaceConnection::info(const string& user, const string& info, const vector<unsigned char>& picture, uint32 avgspeed, uint32 queuelen, bool slotfree) {
	CT("info %s, %s, <...> (%d), %u, %u, %d", user.c_str(), info.c_str(), picture.size(), avgspeed, queuelen, slotfree);
	
	if(mAuthenticated && mMask & EM_USERINFO) {
		IUserInfo s(user, info, picture, avgspeed, queuelen, slotfree);
		send(s);
	}
}

void IfaceConnection::shares(const string& user, const Shares& shares) {
	CT("shares %s, <...> (%d)", user.c_str(), shares.size());
	
	if (mAuthenticated && mMask & EM_USERSHARES) {
		IUserShares s(user, shares);
		send(s);
	}
}

void IfaceConnection::transfer_state(const vector<Transfer*>& uploads, const vector<Transfer*>& downloads) {
	CT("transfer_state");
	
	if(mAuthenticated && mMask & EM_TRANSFERS) {
		ITransferState s(uploads, downloads);
		send(s);
	}
}

void IfaceConnection::transfer_update(const Transfer* transfer) {
	CT("transfer_update <...>");
	
	if(mAuthenticated && mMask & EM_TRANSFERS) {
		ITransferUpdate s(transfer);
		send(s);
	}
}

void IfaceConnection::transfer_delete(const Transfer* transfer) {
	CT("transfer_remove <...>");
	
	if(mAuthenticated && mMask & EM_TRANSFERS) {
		ITransferRemove s(transfer);
		send(s);
	}
}

void IfaceConnection::peer_exists(const string& user, bool exists) {
	CT("peer_exists %s, %d", user.c_str(), exists);
	
	if(mAuthenticated) {
		IPeerExists s(user, exists);
		send(s);
	}
}

void IfaceConnection::peer_status(const string& user, uint32 status) {
	CT("peer_status %s, %d", user.c_str(), status);
	
	if(mAuthenticated) {
		IPeerStatus s(user, status);
		send(s);
	}
}

void IfaceConnection::peer_stats(const string& user, uint32 avgspeed, uint32 numdownloads, uint32 files, uint32 dirs) {
	CT("peer_stats %s %u %u %u %u", user.c_str(), avgspeed, numdownloads, files, dirs);
	
	if(mAuthenticated) {
		IPeerStats s(user, avgspeed, numdownloads, files, dirs);
		send(s);
	}
}

void IfaceConnection::room_tickers(const string& room, const Tickers& tickers) {
	CT("room_tickers %s <...> (%d)", room.c_str(), tickers.size());
	
	if(mAuthenticated && mMask & EM_CHAT) {
		IRoomTickers s(room, tickers);
		send(s);
	}
}

void IfaceConnection::room_ticker_set(const string& room, const string& user, const string& message) {
	CT("room_ticker_set %s, %s, %s", room.c_str(), user.c_str(), message.c_str());
	
	if(mAuthenticated && mMask & EM_CHAT) {
		IRoomTickerSet s(room, user, message);
		send(s);
	}
}

void IfaceConnection::privileges_left(uint32 time_left) {
	CT("privileges_left %u", time_left);
	
	if(mAuthenticated) {
		ICheckPrivileges s(time_left);
		send(s);
	}
}

void IfaceConnection::peer_address(const std::string& user, const std::string& ip, uint32 port) {
	CT("peer_address %s, %s, %u", user.c_str(), ip.c_str(), port);
	
	if(mAuthenticated) {
		IPeerAddress s(user, ip, port);
		send(s);
	}
}

void IfaceConnection::status_set(uint32 status) {
	CT("status_set %u", status);
	
	if(mAuthenticated) {
		ISetStatus s(status);
		send(s);
	}
}

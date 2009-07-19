# pymuseekd - Python tools for museekd
#
# Copyright (C) 2003-2004 Hyriand <hyriand@thegraveyard.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

import socket
import struct
import random

try:
	from Crypto.Hash import SHA256
	from Crypto.Cipher import AES

	class Cipher:
		def __init__(self, key):
			self.ctx = AES.new(SHA256.new(key).digest())
		def decipher(self, value):
			return self.ctx.decrypt(value)
		def cipher(self, value):
			block = value
			while len(block) % 16:
				block += chr(random.randint(0, 255))
			return self.ctx.encrypt(block)
	sha256Block = SHA256.new
except ImportError:
	from mucipher import Cipher, sha256Block

class InvalidHostException(Exception):
	pass

class InvalidMessageException(Exception):
	pass

class UnknownMessageException(Exception):
	pass

# Extract message codes and classes from messages.py and add them to MSGTAB
import messages
MSGTAB = {}
for _message in dir(messages):
	message = getattr(messages, _message)
	if not hasattr(message, 'code'):
		continue
	MSGTAB[message.code] = message

class Driver:
	def __init__(self, callback=None):
		self.socket = None
		self.connected = False
		self.loggedin = False
		self.password = None
		self.mask = None
		self.cipher = None
		self.sync_id = 0
		self.callback = callback
	# Connect to museekd, host in the form of "/tmp/museekd.user" for unix sockets
	# or "somehostname:port" for TCP sockets. Mask is an event mask (see messages.py)
	def connect(self, host, password, mask = 0):
		self.password = password
		self.mask = mask
		if host[:1] == '/':
			# Connect to a unix socket
			self.socket = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
			try:
				self.socket.connect(host)
			except Exception, e:
				self.socket = None
				raise e
		else:
			# Connect to a TCP socket
			ix = host.rfind(":")
			if(ix == -1):
				raise InvalidHostException, 'hostname "%s" is missing a port' % host
			addr = (host[:ix], int(host[ix+1:]))
			self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
			try:
				self.socket.connect(addr)
			except Exception, e:
				self.socket = None
				raise e
	
	# Fetch and parse a message from museekd, blocks until an entire message is read
	def fetch(self):
		## Unpack the first 8 bytes of the message
		data = ""
		while len(data) < 8:
			buf = self.socket.recv(8 - len(data))
			if not buf:
				self.cb_disconnected()
				self.socket = None
				return
			data += buf
	
		## First 4 bytes are the length
		length = struct.unpack("<i", data[:4])[0]
		
		if length < 4:
			raise InvalidMessageException, 'received invalid message length (%i)' % length
		## Second 4 bytes are the message code
		code = struct.unpack("<I", data[4:])[0]
		## If message is longer than it's code, unpack all data

		data = ''
		if length > 4:
			length -= 4
			while len(data) < length:
				recv = self.socket.recv(length  - len(data))
				if not recv:
					self.cb_disconnected()
					self.socket = None
					return
				
				data += recv
	
		## If message doesn't match known messages, raise an error
		if not code in MSGTAB:
			raise UnknownMessageException, 'received unknown message tyoe 0x%04X' % code
		## Parse message with the message's class parse function
		m = MSGTAB[code]()
		m.cipher = self.cipher
		try:
			newmessage = m.parse(data)
		except Exception, e:
			self.PassError(e)
			return None
		else:
			return newmessage
			
	def PassError(self, message):
		if self.callback is not None:
			self.callback(message)
			
	# Send a message to museekd
	def send(self, message):
		message.cipher = self.cipher
		data = message.make()
		self.socket.send(message.pack_int(len(data)) + data)
	
	def close(self):
		self.socket.shutdown(1)
		
	# Fetch and process a message from museekd
	def process(self, message = None):
		if message is None:
			message = self.fetch()
			if not message:
				return
		if not self.socket:
			return
		
		if message.__class__ is messages.Ping:
			self.cb_ping()
		elif message.__class__ is messages.Challenge:
			chresp = sha256Block(message.challenge + self.password).hexdigest()
			self.send(messages.Login("SHA256", chresp, self.mask))
		elif message.__class__ is messages.Login:
			self.logged_in = message.result
			if not self.logged_in:
				self.cb_login_error(message.msg)
			else:
				self.cb_login_ok()
				self.cipher = Cipher(self.password)
		elif message.__class__ is messages.ServerState:
			self.cb_server_state(message.state, message.username)
		elif message.__class__ is messages.CheckPrivileges:
			self.cb_server_privileges(message.time_left)
		elif message.__class__ is messages.SetStatus:
			self.cb_server_status_set(message.status)
		elif message.__class__ is messages.StatusMessage:
			self.cb_status_message(message.type, message.message)
		elif message.__class__ is messages.DebugMessage:
			self.cb_debug_message(message.domain, message.message)
		elif message.__class__ is messages.ChangePasword:
			self.cb_change_password(message.password)
		elif message.__class__ is messages.ConfigState:
			self.cb_config_state(message.config)
		elif message.__class__ is messages.ConfigSet:
			self.cb_config_set(message.domain, message.key, message.value)
		elif message.__class__ is messages.ConfigRemove:
			self.cb_config_remove(message.domain, message.key)
		elif message.__class__ is messages.PeerExists:
			self.cb_peer_exists(message.user, message.exists)
		elif message.__class__ is messages.PeerStatus:
			self.cb_peer_status(message.user, message.status)
		elif message.__class__ is messages.PeerStats:
			self.cb_peer_stats(message.user, message.avgspeed, message.numdownloads, message.numfiles, message.numdirs, message.slotsfull, message.country)
		elif message.__class__ is messages.UserInfo:
			self.cb_user_info(message.user, message.info, message.picture, message.uploads, message.queue, message.slotsfree)
		elif message.__class__ is messages.UserShares:
			self.cb_user_shares(message.user, message.shares)
		elif message.__class__ is messages.PeerAddress:
			self.cb_peer_address(message.user, message.ip, message.port)
		elif message.__class__ is messages.RoomState:
			self.cb_room_state(message.roomlist, message.joined_rooms, message.tickers)
		elif message.__class__ is messages.RoomList:
			self.cb_room_list(message.roomlist)
		elif message.__class__ is messages.PrivateMessage:
			self.cb_private_message(message.direction, message.timestamp, message.user, message.message)
		elif message.__class__ is messages.JoinRoom:
			self.cb_room_joined(message.room, message.users, message.private, message.owner, message.operators)
		elif message.__class__ is messages.LeaveRoom:
			self.cb_room_left(message.room)
		elif message.__class__ is messages.UserJoinedRoom:
			self.cb_room_user_joined(message.room, message.user, message.userdata)
		elif message.__class__ is messages.UserLeftRoom:
			self.cb_room_user_left(message.room, message.user)
		elif message.__class__ is messages.SayRoom:
			self.cb_room_said(message.room, message.user, message.line)
		elif message.__class__ is messages.RoomTickers:
			self.cb_room_tickers(message.room, message.tickers)
		elif message.__class__ is messages.RoomTickerSet:
			self.cb_room_ticker_set(message.room, message.user, message.message)
		elif message.__class__ is messages.AskPublicChat:
			self.cb_public_chat_ask()
		elif message.__class__ is messages.StopPublicChat:
			self.cb_public_chat_stop()
		elif message.__class__ is messages.PublicChat:
			self.cb_public_chat(message.room, message.user, message.message)
		elif message.__class__ is messages.PrivateRoomToggle:
			self.cb_private_room_toggle(message.enabled)
		elif message.__class__ is messages.PrivateRoomList:
			self.cb_private_room_list(message.rooms)
		elif message.__class__ is messages.PrivateRoomAddUser:
			self.cb_private_room_add_user(message.room, message.user)
		elif message.__class__ is messages.PrivateRoomRemoveUser:
			self.cb_private_room_remove_user(message.room, message.user)
		elif message.__class__ is messages.RoomMembers:
			self.cb_room_members(message.roomlist)
		elif message.__class__ is messages.RoomsTickers:
			self.cb_rooms_tickers(message.tickers)
		elif message.__class__ is messages.PrivateRoomAlterableMembers:
			self.cb_private_room_alterable_members(message.members)
		elif message.__class__ is messages.PrivateRoomAlterableOperators:
			self.cb_private_room_alterable_operators(message.operators)
		elif message.__class__ is messages.PrivateRoomAddOperator:
			self.cb_private_room_add_operator(message.room, message.user)
		elif message.__class__ is messages.PrivateRoomRemoveOperator:
			self.cb_private_room_remove_operator(message.room, message.user)
		elif message.__class__ is messages.Search:
			self.cb_search_ticket(message.query, message.ticket)
		elif message.__class__ is messages.SearchReply:
			self.cb_search_results(message.ticket, message.user, message.free, message.speed, message.queue, message.results)
		elif message.__class__ is messages.AddWishListItem:
			self.cb_wishlist_add(message.query, message.lastSearched)
		elif message.__class__ is messages.RemoveWishListItem:
			self.cb_wishlist_remove(message.query)
		elif message.__class__ is messages.TransferState:
			self.cb_transfer_state(message.downloads, message.uploads)
		elif message.__class__ is messages.TransferUpdate:
			self.cb_transfer_update(message.transfer)
		elif message.__class__ is messages.TransferRemove:
			self.cb_transfer_remove(message.transfer)
		elif message.__class__ is messages.TransferAbort:
			self.cb_transfer_abort(message.transfer)
		elif message.__class__ is messages.GetRecommendations:
			self.cb_get_recommendations(message.recommendations)
		elif message.__class__ is messages.GetGlobalRecommendations:
			self.cb_get_global_recommendations(message.recommendations)
		elif message.__class__ is messages.GetSimilarUsers:
			self.cb_get_similar_users(message.users)
		elif message.__class__ is messages.GetItemRecommendations:
			self.cb_get_item_recommendations(message.item, message.recommendations)
		elif message.__class__ is messages.GetItemSimilarUsers:
			self.cb_get_item_similar_users( message.item, message.users)
		elif message.__class__ is messages.AddInterest:
			self.cb_add_interest(message.interest)
		elif message.__class__ is messages.RemoveInterest:
			self.cb_remove_interest(message.interest)
		elif message.__class__ is messages.AddHatedInterest:
			self.cb_add_hated_interest(message.interest)
		elif message.__class__ is messages.RemoveHatedInterest:
			self.cb_remove_hated_interest(message.interest)
		else:
			print 'Unhandled message:', message
	
	
	# Sync with museekd
	def sync(self, ignore = False):
#		print "syncing"
		id = self.sync_id
		self.sync_id += 1
		self.send(messages.Ping(id))
		while 1:
			try:
				message = self.fetch()
			except Exception, e:
				print e
				continue
			if message.__class__ == messages.Ping and message.id == id:
				break
			elif not ignore:
				self.process(message)
#		print "synced"
	
	# disconnected
	def cb_disconnected(self):
		pass
#		print 'disconnected'
	
	# Ping
	def cb_ping(self):
		pass
#		print "pong"
	
	# Couldn't log in
	def cb_login_error(self, error):
		pass
#		print 'couldn\'t login:', error
		
	# Logged in successfully
	def cb_login_ok(self):
		pass
#		print 'logged in'
	
	# Server state
	def cb_server_state(self, state, username):
		pass
	
#		if not state:
#			print 'Not',
#		print 'connected to server.',
#		if state:
#			print 'Username: ', `username`,
#		print
	def cb_status_message(self, type, message):
		pass
	
	def cb_debug_message(self, domain, message):
		pass

	def cb_change_password(self, password):
		pass
	
	# Seconds of privileges left
	def cb_server_privileges(self, time_left):
		pass
#		print "%i seconds of privileges left" % time_left
	
	# Online / away
	def cb_server_status_set(self, status):
		pass
#		print "you are now %s" % (status and "away" or "online")
	
	# Config state
	def cb_config_state(self, config):
		pass
#		print config
	
	# Config changed
	def cb_config_set(self, domain, key, value):
		pass
#		print "Config changed:", domain, "--", key, "--", value
	
	# Config key removed
	def cb_config_remove(self, domain, key):
		pass
#		print "Removed key:", domain, "--", key
		
	# Does a peer exist?
	def cb_peer_exists(self, user, exists):
		pass
#		print 'User', user,
#		if exists:
#			print 'exists'
#		else:
#			print 'does not exist'
	
	# Peer status
	def cb_peer_status(self, user, status):
		pass
#		print 'User', user, 'is', ['offline', 'away', 'online'][status]
	
	# Peer stats
	def cb_peer_stats(self, user, avgspeed, numdownloads, numfiles, numdirs, slotsfull, country):
		pass
#		print 'User', user, 'avgspeed:', avgspeed,'files:', numfiles
	
	# Peer address
	def cb_peer_address(self, user, ip, port):
		pass
#		print 'User', user, 'ip:', ip, 'port:', port
	
	# Room state
	def cb_room_state(self, roomlist, joined_rooms, tickers):
		pass
#		print 'Joined', len(joined_rooms), 'room(s)'
	
	# Room list
	def cb_room_list(self, roomlist):
		pass
#		print len(roomlist), 'rooms'
	def cb_get_global_recommendations(self, recommendations):
		pass
	def cb_get_similar_users(self, users):
		pass
	def cb_get_recommendations(self, recommendations):
		pass	
	def cb_get_item_similar_users(self, item, users):
		pass
	def cb_get_item_recommendations(self, item, recommendations):
		pass	
	def cb_add_interest(self, interest):
		pass
	
	def cb_remove_interest(self, interest):
		pass
	
	def cb_add_hated_interest(self, interest):
		pass
	
	def cb_remove_hated_interest(self, interest):
		pass
	
	# Joined room
	def cb_room_joined(self, room, users, private, owner, operators):
		pass
#		print 'Joined room', room, len(users), 'users'
	
	# Left room
	def cb_room_left(self, room):
		pass
#		print 'Left room', room
	
	# User joined a room
	def cb_room_user_joined(self, room, user, userdata):
		pass
#		print 'User', user, 'joined room', room
	
	# User left a room
	def cb_room_user_left(self, room, user):
		pass
#		print 'User', user, 'left room', room
	
	# Someone said something
	def cb_room_said(self, room, user, message):
		pass
#		print '%s [%s] %s' % (room, user, message)

	# See Room tickers
	def cb_room_tickers(self, room, tickers):
		pass
#		print "Ticker set for room '%s', user '%s': %s" % (room, user, message)

	# Someone set a ticker message
	def cb_room_ticker_set(self, room, user, message):
		pass
#		print "Ticker set for room '%s', user '%s': %s" % (room, user, message)
	
	def cb_public_chat_ask(self):
		pass

	def cb_public_chat_stop(self):
		pass

	def cb_public_chat(self, room, user, message):
		pass

	def cb_private_room_toggle(self, enabled):
		pass

	def cb_private_room_list(self, rooms):
		pass

	def cb_private_room_add_user(self, room, user):
		pass

	def cb_private_room_remove_user(self, room, user):
		pass

	def cb_room_members(self, roomlist):
		pass

	def cb_rooms_tickers(self, tickers):
		pass

	def cb_private_room_alterable_members(self, members):
		pass

	def cb_private_room_alterable_operators(self, operators):
		pass

	def cb_private_room_add_operator(self, room, user):
		pass

	def cb_private_room_remove_operator(self, room, user):
		pass

	# Private message
	def cb_private_message(self, direction, timestamp, user, message):
		pass
#		print '[%s] %s' % (user, message)
	
	# Search ticket
	def cb_search_ticket(self, query, ticket):
		pass
#		print '%s: %i' % (query, ticket)
	
	# Search results
	def cb_search_results(self, ticket, user, free, speed, queue, results):
		pass
#		print len(results), 'search results from user', user

	def cb_wishlist_add(self, query, lastSearched):
		pass

	def cb_wishlist_remove(self, query):
		pass

	# User info
	def cb_user_info(self, user, info, picture, uploads, queue, slotsfree):
		pass
#		print 'user info for', user, '\n' + info
	
	# User shares
	def cb_user_shares(self, user, shares):
		pass
#		print 'user shares for', user, len(shares), 'directories'
	
	# Transfer state
	def cb_transfer_state(self, downloads, uploads):
		pass
#		print len(downloads), 'downloads,', len(uploads), 'uploads'
	
	# Transfer update
	def cb_transfer_update(self, transfer):
		pass
#		print ['download', 'upload'][transfer.is_upload], transfer.user, transfer.path, 'updated'
	def cb_transfer_remove(self, transfer):
		pass

	def cb_transfer_abort(self, transfer):
		pass

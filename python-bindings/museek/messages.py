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

import struct
import sys
# Event mask
EM_CHAT		= 1 << 0
EM_PRIVATE	= 1 << 1
EM_TRANSFERS	= 1 << 2
EM_USERINFO	= 1 << 3
EM_USERSHARES	= 1 << 4
EM_INTERESTS	= 1 << 5
EM_CONFIG	= 1 << 6
EM_DEBUG	= 1 << 7

# Transfer state
TS_Finished	= 0
TS_Transferring = 1
TS_Negotiating	= 2
TS_Waiting	= 3
TS_Establishing	= 4
TS_Initiating	= 5
TS_Connecting	= 6
TS_QueuedRemotely	= 7
TS_Address	= 8
TS_Status	= 9
TS_Offline	= 10
TS_ConnectionClosed	= 11
TS_CannotConnect	= 12
TS_Aborted	= 13
TS_RemoteError	= 14
TS_LocalError	= 15
TS_QueuedLocally	= 16

class BaseMessage:
	cipher = None
	def __init__(self):
		pass
	
	def make(self):
		print "make called on unsupported message type"
		sys.stdout.flush()
		return ""

	def parse(self, msg):
		print "parse called on unsupported message type"
		sys.stdout.flush()
		return None

	def pack_int(self, i):
		return struct.pack("<i", i)

	def pack_uint(self, i):
		return struct.pack("<I", i)

	def pack_off(self, i):
		return struct.pack("<Q", i)

	def pack_string(self, s):
		return self.pack_int(len(s)) + s

	def unpack_int(self, d):
		return struct.unpack("<i", d[:4])[0], d[4:]

	def unpack_uint(self, d):
		return struct.unpack("<I", d[:4])[0], d[4:]

	def unpack_off(self, d):
		return struct.unpack("<Q", d[:8])[0], d[8:]

	def unpack_string(self, d):
		l, d = self.unpack_uint(d)
		return d[0:l], d[l:]
	
	def unpack_pos_off(self, d, pos):
		return struct.unpack("<Q", d[pos:pos+8])[0], pos+8
	
	def unpack_pos_uint(self, d, pos):
		return struct.unpack("<I", d[pos:pos+4])[0], pos+4
	
	def unpack_pos_string(self, data, position):
		lenstring, position = self.unpack_pos_uint(data, position)
		string = data[position:position+lenstring]
		return string, position+lenstring
	
	def unpack_cipher(self, d):
		l, d = self.unpack_uint(d);
		if(l % 16) != 0:
			l_c = ((l / 16) + 1) * 16
		else:
			l_c = l
		return self.cipher.decipher(d[0:l_c])[:l], d[l_c:]
	
	def pack_cipher(self, s):
		return self.pack_uint(len(s)) + self.cipher.cipher(s)
	

class Transfer:
	def __init__(self, is_upload, user, path, state, error, filepos, filesize, rate, place):
		self.is_upload = is_upload
		self.user = user
		self.path = path
		self.state = state
		self.error = error
		self.filepos = filepos
		self.filesize = filesize
		self.rate = rate
		self.place = place

class Ping(BaseMessage):
	code = 0x0000
	def __init__(self, id = None):
		self.id = id
	
	def make(self):
		return self.pack_uint(self.code) + self.pack_uint(self.id)
	
	def parse(self, data):
		self.id, data = self.unpack_uint(data)
		return self
	
class Challenge(BaseMessage):
	code = 0x0001
	
	def __init__(self):
		self.version = None
		self.challenge = None
		
	def parse(self, data):
		self.version, data = self.unpack_uint(data)
		self.challenge, data = self.unpack_string(data)
		return self

class Login(BaseMessage):
	code = 0x0002
	
	def __init__(self, algorithm = None, chresponse = None, mask = None):
		self.algorithm = algorithm
		self.chresponse = chresponse
		self.mask = mask
		self.result = None
		self.msg = None
		self.challenge = None
	
	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.algorithm) + \
			self.pack_string(self.chresponse) + \
			self.pack_uint(self.mask)
	
	def parse(self, data):
		self.result, data = ord(data[0]), data[1:]
		self.msg, data = self.unpack_string(data)
		self.challenge, data = self.unpack_string(data)
		return self

class ServerState(BaseMessage):
	code = 0x0003
	
	def __init__(self):
		self.state = None
		self.username = None

	def parse(self, data):
		self.state, data = ord(data[0]), data[1:]
		self.username, data = self.unpack_string(data)
		return self

class CheckPrivileges(BaseMessage):
	code = 0x0004
	
	def __init__(self):
		self.time_left = None
	
	def make(self):
		return self.pack_uint(self.code)
	
	def parse(self, data):
		self.time_left, data = self.unpack_uint(data)
		return self
	
class SetStatus(BaseMessage):
	code = 0x0005
	
	def __init__(self, status = None):
		self.status = status
	
	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_uint(self.status)
	
	def parse(self, data):
		self.status, data = self.unpack_uint(data)
		return self

class StatusMessage(BaseMessage):
	code = 0x0010
	
	def __init__(self):
		self.type = None
		self.message = None

	def parse(self, data):
		self.type, data = ord(data[0]), data[1:]
		self.message, data = self.unpack_string(data)
		return self

class DebugMessage(BaseMessage):
	code = 0x0011
	
	def __init__(self):
		self.domain = None
		self.message = None

	def parse(self, data):
		self.domain, data = self.unpack_string(data)
		self.message, data = self.unpack_string(data)
		return self

class ChangePasword(BaseMessage):
	code = 0x0012
	
	def __init__(self, password = None):
		self.password = password
	
	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_cipher(self.password)
	
	def parse(self, data):
		self.password, data = self.unpack_cipher(data)
		return self

class ConfigState(BaseMessage):
	code = 0x0100
	
	def __init__(self):
		self.config = None
		
	def parse(self, data):
		self.config = {}
		domains, data = self.unpack_uint(data)
		for i in range(domains):
			domain, data = self.unpack_cipher(data)
			values = {}
			keys, data = self.unpack_uint(data)
			for j in range(keys):
				key, data = self.unpack_cipher(data)
				value, data = self.unpack_cipher(data)
				values[key] = value
			self.config[domain] = values
		return self

class ConfigSet(BaseMessage):
	code = 0x0101
	
	def __init__(self, domain = None, key = None, value = None):
		self.domain = domain
		self.key = key
		self.value = value
	
	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_cipher(self.domain) + \
			self.pack_cipher(self.key) + \
			self.pack_cipher(self.value)
	
	def parse(self, data):
		self.domain, data = self.unpack_cipher(data)
		self.key, data = self.unpack_cipher(data)
		self.value, data = self.unpack_cipher(data)
		return self

class ConfigRemove(BaseMessage):
	code = 0x0102
	
	def __init__(self, domain = None, key = None):
		self.domain = domain
		self.key = key
	
	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_cipher(self.domain) + \
			self.pack_cipher(self.key)
	
	def parse(self, data):
		self.domain, data = self.unpack_cipher(data)
		self.key, data = self.unpack_cipher(data)
		return self

class ConfigSetUserImage(BaseMessage):
	code = 0x0103
	
	def __init__(self, image = ""):
		self.image = image
	
	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.image)
	
class PeerExists(BaseMessage):
	code = 0x0201
	
	def __init__(self, user = None):
		self.user = user
		self.exists = None
	
	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.user)
	
	def parse(self, data):
		self.user, data = self.unpack_string(data)
		self.exists = ord(data[0])
		return self
	
class PeerStatus(BaseMessage):
	code = 0x0202
	
	def __init__(self, user = None):
		self.user = user
		self.status = None
	
	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.user)
	
	def parse(self, data):
		self.user, data = self.unpack_string(data)
		self.status, data = self.unpack_uint(data)
		return self
		
class PeerStats(BaseMessage):
	code = 0x0203
	
	def __init__(self, user = None):
		self.user = user
		self.avgspeed = None
		self.numdownloads = None
		self.numfiles = None
		self.numdirs = None
		self.slotsfull = None
		self.country = None
	
	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.user)
	
	def parse(self, data):
		self.user, data = self.unpack_string(data)
		self.avgspeed, data = self.unpack_uint(data)
		self.numdownloads, data = self.unpack_uint(data)
		self.numfiles, data = self.unpack_uint(data)
		self.numdirs, data = self.unpack_uint(data)
		self.slotsfull, data = ord(data[0]), data[1:]
		self.country, data = self.unpack_string(data)
		return self

class UserInfo(BaseMessage):
	code = 0x0204
	
	def __init__(self, user = None):
		self.user = user
		self.info = None
		self.picture = None
		self.uploads = None
		self.queue = None
		self.slotsfree = None

	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.user)

	def parse(self, data):
		self.user, data = self.unpack_string(data)
		self.info, data = self.unpack_string(data)
		self.picture, data = self.unpack_string(data)
		self.uploads, data = self.unpack_uint(data)
		self.queue, data = self.unpack_uint(data)
		self.slotsfree, data = ord(data[0]), data[1:]
		return self

class UserShares(BaseMessage):
	code = 0x0205
	
	def __init__(self, user = None):
		self.user = user
		self.shares = None

	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.user)

	def parse(self, data):
		self.shares = {}
		pos = 0
		
		self.user, pos = self.unpack_pos_string(data, pos)
		dirs, pos = self.unpack_pos_uint(data, pos)
		for i in range(dirs):
			dir, pos = self.unpack_pos_string(data, pos)

			self.shares[dir] = {}
			
			files, pos = self.unpack_pos_uint(data, pos)

			for j in range(files):
				filename, pos = self.unpack_pos_string(data, pos)
				size, pos = self.unpack_pos_off(data, pos)
				extension, pos = self.unpack_pos_string(data, pos)
				attrs, pos = self.unpack_pos_uint(data, pos)
				attributes = []
				for k in range(attrs):
					a, pos = self.unpack_pos_uint(data, pos)
					attributes.append(a)
					
				self.shares[dir][filename] = [size, extension, attributes]
		return self

class PeerAddress(BaseMessage):
	code = 0x0206
	
	def __init__(self, user = None):
		self.user = user
		self.ip = None
		self.port = None
	
	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.user)
	
	def parse(self, data):
		self.user, data = self.unpack_string(data)
		self.ip, data = self.unpack_string(data)
		self.port, data = self.unpack_uint(data)
		return self


class GivePrivileges(BaseMessage):
	code = 0x0207

	def __init__(self, user = None, days=None):
		self.user = user
		self.days = days

	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.user) + \
			self.pack_uint(self.days)


class RoomState(BaseMessage): # deprecated since 0.3
	code = 0x0300
	
	def __init__(self):
		self.roomlist = None
		self.joined_rooms = None
		self.tickers = None

	def parse(self, data):
		self.roomlist = {}
		pos = 0
 
		nr, pos = self.unpack_pos_uint(data, pos)
		for i in range(nr):
			room, pos = self.unpack_pos_string(data, pos)
			users, pos = self.unpack_pos_uint(data, pos)
			self.roomlist[room] = users

		self.joined_rooms = {}
		self.tickers = {}
		n, pos = self.unpack_pos_uint(data, pos)
		for i in range(n):
			room, pos = self.unpack_pos_string(data, pos)
			self.joined_rooms[room] = {}
			n2, pos = self.unpack_pos_uint(data, pos) 
			for j in range(n2):
				user, pos = self.unpack_pos_string(data, pos)
				status, pos = self.unpack_pos_uint(data, pos)
				avgspeed, pos = self.unpack_pos_uint(data, pos)
				downloadnum, pos = self.unpack_pos_uint(data, pos)
				files, pos = self.unpack_pos_uint(data, pos)
				dirs, pos = self.unpack_pos_uint(data, pos)

				slotsfull = ord(data[pos]); pos += 1
				self.joined_rooms[room][user] = [status, avgspeed, downloadnum, files, dirs, slotsfull]
				
			n3, pos = self.unpack_pos_uint(data, pos) 
			self.tickers[room] = {}
			for j in range(n3):
				user, pos = self.unpack_pos_string(data, pos)
				message, pos = self.unpack_pos_string(data, pos)
				self.tickers[room][user] = message
		return self

class RoomList(BaseMessage):
	code = 0x0301
	
	def __init__(self):
		self.roomlist = None
	
	def make(self):
		return self.pack_uint(self.code)
	
	def parse(self, data):
		self.roomlist = {}
		n, data = self.unpack_uint(data);
		for i in range (n):
			r, data = self.unpack_string(data)
			u, data = self.unpack_uint(data)
			self.roomlist[r] = u
		return self

class PrivateMessage(BaseMessage):
	code = 0x0302
	
	def __init__(self, direction = None, user = None, message = None):
		self.direction = direction
		self.timestamp = None
		self.user = user
		self.message = message

	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.user) + \
			self.pack_string(self.message)

	def parse(self, data):
		self.direction, data = self.unpack_uint(data)
		self.timestamp, data = self.unpack_uint(data)
		self.user, data = self.unpack_string(data)
		self.message, data = self.unpack_string(data)
		return self

class JoinRoom(BaseMessage):
	code = 0x0303
	
	def __init__(self, room = None, private = None):
		self.room = room
		self.private = private
		self.users = None
		self.owner = None
		self.operators = []

	def make(self):
		if self.private:
			private = chr(1)
		else:
			private = chr(0)
		return self.pack_uint(self.code) + \
			self.pack_string(self.room) + \
			private

	def parse(self, data):
		self.users = {}
		pos = 0
		self.room, pos = self.unpack_pos_string(data, pos)
		n, pos = self.unpack_pos_uint(data, pos)
		for i in range(n):
			user, pos = self.unpack_pos_string(data, pos)
			status, pos = self.unpack_pos_uint(data, pos)
			avgspeed, pos = self.unpack_pos_uint(data, pos)
			downloadnum, pos = self.unpack_pos_uint(data, pos)
			files, pos = self.unpack_pos_uint(data, pos)
			dirs, pos = self.unpack_pos_uint(data, pos)

			free = ord(data[pos]); pos += 1
			self.users[user] = [status, avgspeed, downloadnum, files, dirs, free]
		self.private = False
		self.owner = ''
		self.operators = []
		if len(data) > 0:
			self.private = True
			self.owner, pos = self.unpack_pos_string(data, pos)
			no, pos = self.unpack_pos_uint(data, pos)
			for i in range(no):
				operator, pos = self.unpack_pos_string(data, pos)
				self.operators.append(operator)
		return self

class LeaveRoom(BaseMessage):
	code = 0x0304
	
	def __init__(self, room = None):
		self.room = room

	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.room)

	def parse(self, data):
		self.room, data = self.unpack_string(data)
		return self

class UserJoinedRoom(BaseMessage):
	code = 0x0305
	
	def __init__(self):
		self.room = None
		self.user = None
		self.userdata = None

	def parse(self, data):
		self.room, data = self.unpack_string(data)
		self.user, data = self.unpack_string(data)
		status, data = self.unpack_uint(data)
		avgspeed, data = self.unpack_uint(data)
		downloadnum, data = self.unpack_uint(data)
		files, data = self.unpack_uint(data)
		dirs, data = self.unpack_uint(data)
		free, data = ord(data[0]), data[1:]
		self.userdata = [status, avgspeed, downloadnum, files, dirs, free]
		return self

class UserLeftRoom(BaseMessage):
	code = 0x0306
	
	def __init__(self):
		self.room = None
		self.user = None

	def parse(self, data):
		self.room, data = self.unpack_string(data)
		self.user, data = self.unpack_string(data)
		return self
	
class SayRoom(BaseMessage):
	code = 0x0307
	
	def __init__(self, room = None, line = None):
		self.room = room
		self.user = None
		self.line = line

	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.room) + \
			self.pack_string(self.line)

	def parse(self, data):
		self.room, data = self.unpack_string(data)
		self.user, data = self.unpack_string(data)
		self.line, data = self.unpack_string(data)
		return self

class RoomTickers(BaseMessage):
	code = 0x0308
	
	def __init__(self, room = None, tickers = None):
		self.room = room
		self.tickers = None
	def make(self):
		return self.pack_uint(self.code)

	def parse(self, data):
		self.room, data = self.unpack_string(data)
		self.tickers = {}
		numtickers, data = self.unpack_uint(data)
		for i in range(numtickers):
			message, data = self.unpack_string(data)
			user, data = self.unpack_string(data)
			self.tickers[user] = message
		return self

class RoomTickerSet(BaseMessage):
	code = 0x0309
	
	def __init__(self, room = None, message = None):
		self.room = room
		self.message = message
		self.user = None
	
	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.room) + \
			self.pack_string(self.message)
	
	def parse(self, data):
		self.room, data = self.unpack_string(data)
		self.user, data = self.unpack_string(data)
		self.message, data = self.unpack_string(data)
		return self

class MessageUsers(BaseMessage):
	code = 0x0310
	
	def __init__(self, users = None, message = None):
		self.users = users
		self.message = message
	
	def make(self):
		for u in self.users:
			user_list += self.pack_string(u);
		return self.pack_uint(self.code) + \
			self.pack_uint(self.users.len()) + \
			user_list + \
			self.pack_string(self.message)

class MessageBuddies(BaseMessage):
	code = 0x0311
	
	def __init__(self, message = None):
		self.message = message
	
	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.message)

class MessageDownloading(BaseMessage):
	code = 0x0312
	
	def __init__(self, message = None):
		self.message = message
	
	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.message)

class AskPublicChat(BaseMessage):
	code = 0x0313
	
	def __init__(self):
		pass
	
	def make(self):
		return self.pack_uint(self.code)

	def parse(self, data):
		return self

class StopPublicChat(BaseMessage):
	code = 0x0314
	
	def __init__(self):
		pass
	
	def make(self):
		return self.pack_uint(self.code)

	def parse(self, data):
		return self

class PublicChat(BaseMessage):
	code = 0x0315
	
	def __init__(self):
		self.room = None
		self.user = None
		self.message = None
	
	def parse(self, data):
		self.room, data = self.unpack_string(data)
		self.user, data = self.unpack_string(data)
		self.message, data = self.unpack_string(data)
		return self

class PrivateRoomToggle(BaseMessage):
	code = 0x0320
	
	def __init__(self, enabled = None):
		self.enabled = enabled

	def make(self):
		if self.enabled:
			enabled = chr(1)
		else:
			enabled = chr(0)
		return self.pack_uint(self.code) + \
			self.pack_uint(self.enabled) + \
			enabled
	
	def parse(self, data):
		self.enabled, data = ord(data[0]), data[1:]
		return self

class PrivateRoomList(BaseMessage):
	code = 0x0321
	
	def __init__(self):
		self.rooms = None
	
	def parse(self, data):
		n, data = self.unpack_uint(data)
		self.rooms = {}
		for i in range(n):
			room, data = self.unpack_string(data)
			numusers, data = self.unpack_uint(data)
			status, data = self.unpack_uint(data)
			self.rooms[room] = (numusers, status)
		return self

class PrivateRoomAddUser(BaseMessage):
	code = 0x0322
	
	def __init__(self, room = None, user = None):
		self.room = room
		self.user = user

	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.room) + \
			self.pack_string(self.user)
	
	def parse(self, data):
		self.room, data = self.unpack_string(data)
		self.user, data = self.unpack_string(data)
		return self

class PrivateRoomRemoveUser(BaseMessage):
	code = 0x0323
	
	def __init__(self, room = None, user = None):
		self.room = room
		self.user = user

	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.room) + \
			self.pack_string(self.user)
	
	def parse(self, data):
		self.room, data = self.unpack_string(data)
		self.user, data = self.unpack_string(data)
		return self

class RoomMembers(BaseMessage):
	code = 0x0324
	
	def __init__(self):
		self.roomlist = None
	
	def parse(self, data):
		self.roomlist = {}
		pos = 0

		n, pos = self.unpack_pos_uint(data, pos)
		for i in range(n):
			room, pos = self.unpack_pos_string(data, pos)
			self.roomlist[room] = {}
			n2, pos = self.unpack_pos_uint(data, pos) 
			for j in range(n2):
				user, pos = self.unpack_pos_string(data, pos)
				status, pos = self.unpack_pos_uint(data, pos)
				avgspeed, pos = self.unpack_pos_uint(data, pos)
				downloadnum, pos = self.unpack_pos_uint(data, pos)
				files, pos = self.unpack_pos_uint(data, pos)
				dirs, pos = self.unpack_pos_uint(data, pos)
				slotsfull = ord(data[pos]); pos += 1
				country, pos = self.unpack_pos_string(data, pos)
				
				roomstatus, pos = self.unpack_pos_uint(data, pos)
				self.roomlist[room][user] = [status, avgspeed, downloadnum, files, dirs, slotsfull, country, roomstatus]
		return self

class RoomsTickers(BaseMessage):
	code = 0x0325
	
	def __init__(self):
		self.tickers = None
	
	def parse(self, data):
		self.tickers = {}
		pos = 0

		n, pos = self.unpack_pos_uint(data, pos)
		for i in range(n):
			room, pos = self.unpack_pos_string(data, pos)
			n2, pos = self.unpack_pos_uint(data, pos) 

			self.tickers[room] = {}
			for j in range(n2):
				user, pos = self.unpack_pos_string(data, pos)
				message, pos = self.unpack_pos_string(data, pos)
				self.tickers[room][user] = message
		return self

class PrivateRoomAlterableMembers(BaseMessage):
	code = 0x0326
	
	def __init__(self):
		self.members = None
	
	def parse(self, data):
		room, data = self.unpack_string(data)
		n, data = self.unpack_uint(data)
		self.members = {}
		self.members[room] = []
		for i in range(n):
			member, data = self.unpack_string(data)
			self.members[room].append(member)
		return self

class PrivateRoomAlterableOperators(BaseMessage):
	code = 0x0327
	
	def __init__(self):
		self.operators = None
	
	def parse(self, data):
		room, data = self.unpack_string(data)
		n, data = self.unpack_uint(data)
		self.operators = {}
		self.operators[room] = []
		for i in range(n):
			operator, data = self.unpack_string(data)
			self.operators[room].append(operator)
		return self

class PrivateRoomAddOperator(BaseMessage):
	code = 0x0328
	
	def __init__(self, room = None, user = None):
		self.room = room
		self.user = user

	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.room) + \
			self.pack_string(self.user)
	
	def parse(self, data):
		self.room, data = self.unpack_string(data)
		self.user, data = self.unpack_string(data)
		return self

class PrivateRoomRemoveOperator(BaseMessage):
	code = 0x0329
	
	def __init__(self, room = None, user = None):
		self.room = room
		self.user = user

	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.room) + \
			self.pack_string(self.user)
	
	def parse(self, data):
		self.room, data = self.unpack_string(data)
		self.user, data = self.unpack_string(data)
		return self

class PrivateRoomDismember(BaseMessage):
	code = 0x0330
	
	def __init__(self, room = None):
		self.room = room

	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.room)

class PrivateRoomDisown(BaseMessage):
	code = 0x0331
	
	def __init__(self, room = None):
		self.room = room

	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.room)
		
class Search(BaseMessage):
	code = 0x0401
	
	def __init__(self, type = None, query = None):
		self.type = type
		self.query = query
		self.ticket = None

	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_uint(self.type) + \
			self.pack_string(self.query)

	def parse(self, data):
		self.query, data = self.unpack_string(data)
		self.ticket, data = self.unpack_uint(data)
		return self

class SearchReply(BaseMessage):
	code = 0x0402
	
	def __init__(self):
		self.ticket = None
		self.user = None
		self.free = None
		self.speed = None
		self.queue = None
		self.results = None

	def parse(self, data):
		self.ticket, data = self.unpack_uint(data)
		self.user, data = self.unpack_string(data)
		self.free, data = ord(data[0]), data[1:]
		self.speed, data = self.unpack_uint(data)
		self.queue, data = self.unpack_uint(data)
		n, data = self.unpack_uint(data)
		self.results = []
		for i in range(n):
			fn, data = self.unpack_string(data)
			sz, data = self.unpack_off(data)
			ex, data = self.unpack_string(data)
			an, data = self.unpack_uint(data)
			at = []
			for j in range(an):
				a, data = self.unpack_uint(data)
				at.append(a)
			self.results.append((fn, sz, ex, at))
		return self

class UserSearch(BaseMessage):
	code = 0x0403
	
	def __init__(self, user = None, query = None):
		self.user = user
		self.query = query
		self.ticket = None

	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.user) + \
			self.pack_string(self.query)


class WishListSearch(BaseMessage):
	code = 0x0405
	
	def __init__(self,  query = None):
		self.query = query
		self.ticket = None

	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.query)


class AddWishListItem(BaseMessage):
	code = 0x0406
	
	def __init__(self,  query = None):
		self.query = query
		self.lastSearched = None

	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.query)

	def parse(self, data):
		self.query, data = self.unpack_string(data)
		self.lastSearched, data = self.unpack_uint(data)
		return self


class RemoveWishListItem(BaseMessage):
	code = 0x0407
	
	def __init__(self,  query = None):
		self.query = query

	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.query)

	def parse(self, data):
		self.query, data = self.unpack_string(data)
		return self

class TransferState(BaseMessage):
	code = 0x0500
	
	def __init__(self):
		self.downloads = None
		self.uploads = None

	def unpack_transfer(self, data):
		is_upload, data = ord(data[0]), data[1:]
		user, data = self.unpack_string(data)
		path, data = self.unpack_string(data)
		place, data = self.unpack_uint(data)
		state, data = self.unpack_uint(data)
		error, data = self.unpack_string(data)
		filepos, data = self.unpack_off(data)
		filesize, data = self.unpack_off(data)
		rate, data = self.unpack_uint(data)
		return Transfer(is_upload, user, path, state, error, filepos, filesize, rate, place), data
	
	def parse(self, data):
		self.uploads = []
		self.downloads = []
		n, data = self.unpack_uint(data)
		for i in range(n):
			tf, data = self.unpack_transfer(data)
			if tf.is_upload:
				self.uploads.append(tf)
			else:
				self.downloads.append(tf)
		return self

class TransferUpdate(TransferState):
	code = 0x0501
	
	def __init__(self, user = None, path = None):
		self.user = user
		self.path = path
		self.transfer = None
		
	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.user) + \
			self.pack_string(self.path)
			
	def parse(self, data):
		self.transfer, data = self.unpack_transfer(data)
		return self
		
class TransferRemove(BaseMessage):
	code = 0x0502

	def __init__(self, upload = None, user = None, path = None):
		self.upload = upload
		self.user = user
		self.path = path

	def make(self):
		if self.upload:
			upload = chr(1)
		else:
			upload = chr(0)
		return self.pack_uint(self.code) + \
			upload + \
			self.pack_string(self.user) + \
			self.pack_string(self.path)

	def parse(self, data):
		self.upload, data = ord(data[0]), data[1:]
		self.user, data = self.unpack_string(data)
		self.path, data = self.unpack_string(data)
		self.transfer = self.upload, self.user, self.path
		return self

class DownloadFile(BaseMessage):
	code = 0x0503
	
	def __init__(self, user = None, path = None, size = 0):
		self.user = user
		self.path = path
		self.size = size

	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.user) + \
			self.pack_string(self.path) + \
			self.pack_off(self.size)

class GetFolderContents(BaseMessage):
	code = 0x0504
	
	def __init__(self, user = None, folder = None):
		self.user = user
		self.folder = folder

	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.user) + \
			self.pack_string(self.folder)

class TransferAbort(BaseMessage):
	code = 0x0505

	def __init__(self, upload = None, user = None, path = None):
		self.upload = upload
		self.user = user
		self.path = path

	def make(self):
		if self.upload:
			upload = chr(1)
		else:
			upload = chr(0)
		return self.pack_uint(self.code) + \
			upload + \
			self.pack_string(self.user) + \
			self.pack_string(self.path)

	def parse(self, data):
		self.upload, data = ord(data[0]), data[1:]
		self.user, data = self.unpack_string(data)
		self.path, data = self.unpack_string(data)
		self.transfer = self.upload, self.user, self.path
		return self

class UploadFile(BaseMessage):
	code = 0x0506
	
	def __init__(self, user = None, path = None):
		self.user = user
		self.path = path

	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.user) + \
			self.pack_string(self.path)

class DownloadFileTo(BaseMessage):
	code = 0x0507
	
	def __init__(self, user = None, path = None, dpath = None, size = 0):
		self.user = user
		self.path = path
		self.dpath = dpath
		self.size = size

	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.user) + \
			self.pack_string(self.path) + \
			self.pack_string(self.dpath) + \
			self.pack_off(self.size)

class DownloadFolderTo(BaseMessage):
	code = 0x0508
	
	def __init__(self, user = None, path = None, dpath = None):
		self.user = user
		self.path = path
		self.dpath = dpath

	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.user) + \
			self.pack_string(self.path) + \
			self.pack_string(self.dpath)

class UploadFolder(BaseMessage):
	code = 0x0509
	
	def __init__(self, user = None, path = None):
		self.user = user
		self.path = path

	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.user) + \
			self.pack_string(self.path)

	
class GetRecommendations(BaseMessage):
	code = 0x0600
	
	def __init__(self):
		self.recommendations = None
	
	def make(self):
		return self.pack_uint(self.code)
	
	def parse(self, data):
		self.recommendations = {}
		n, data = self.unpack_uint(data);
		for i in range (n):
			r, data = self.unpack_string(data)
			u, data = self.unpack_int(data)
			self.recommendations[r] = u
		return self


class GetGlobalRecommendations(BaseMessage):
	code = 0x0601
	
	def __init__(self):
		self.recommendations = None
	
	def make(self):
		return self.pack_uint(self.code)
	
	def parse(self, data):
		self.recommendations = {}
		n, data = self.unpack_uint(data);
		for i in range (n):
			r, data = self.unpack_string(data)
			u, data = self.unpack_int(data)
			self.recommendations[r] = u
		return self

class GetSimilarUsers(BaseMessage):
	code = 0x0602
	
	def __init__(self):
		self.users = None
	
	def make(self):
		return self.pack_uint(self.code)
	
	def parse(self, data):
		self.users = {}
		n, data = self.unpack_uint(data);
		for i in range (n):
			r, data = self.unpack_string(data)
			u, data = self.unpack_uint(data)
			self.users[r] = u
		return self

class GetItemRecommendations(BaseMessage):
	code = 0x0603
	
	def __init__(self, item = None):
		self.item = item
		self.recommendations = None
	
	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.item)
	
	def parse(self, data):
		self.recommendations = {}
		self.item, data = self.unpack_string(data)
		n, data = self.unpack_uint(data);
		for i in range (n):
			r, data = self.unpack_string(data)
			u, data = self.unpack_int(data)
			self.recommendations[r] = u
		return self


class GetItemSimilarUsers(BaseMessage):
	code = 0x0604
	
	def __init__(self, item = None):
		self.item = item
		self.users = None
	
	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.item)
	
	def parse(self, data):
		self.users = {}
		self.item, data = self.unpack_string(data)
		n, data = self.unpack_uint(data);
		for i in range (n):
			r, data = self.unpack_string(data)
			u, data = self.unpack_uint(data)
			self.users[r] = u
		return self


class AddInterest(BaseMessage):
	code = 0x0610
	
	def __init__(self, interest = None):
		self.interest = interest

	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.interest)

	def parse(self, data):
		self.interest, data = self.unpack_string(data)
		return self

class RemoveInterest(BaseMessage):
	code = 0x0611
	
	def __init__(self, interest = None):
		self.interest = interest

	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.interest)

	def parse(self, data):
		self.interest, data = self.unpack_string(data)
		return self

class AddHatedInterest(BaseMessage):
	code = 0x0612
	
	def __init__(self, interest = None):
		self.interest = interest

	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.interest)

	def parse(self, data):
		self.interest, data = self.unpack_string(data)
		return self

class RemoveHatedInterest(BaseMessage):
	code = 0x0613
	
	def __init__(self, interest = None):
		self.interest = interest

	def make(self):
		return self.pack_uint(self.code) + \
			self.pack_string(self.interest)

	def parse(self, data):
		self.interest, data = self.unpack_string(data)
		return self


class ConnectServer(BaseMessage):
	code = 0x0700
	
	def make(self):
		return self.pack_uint(self.code)
	
class DisconnectServer(BaseMessage):
	code = 0x0701
	
	def make(self):
		return self.pack_uint(self.code)

class ReloadShares(BaseMessage):
	code = 0x0703
	
	def make(self):
		return self.pack_uint(self.code)



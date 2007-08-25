#!/usr/bin/env python
from museek import messages, driver
import socket
import os, sys, select, urllib

class museekDriver(driver.Driver):
	def __init__(self, client):
		driver.Driver.__init__(self)
		self.client = client
	
	def cb_disconnected(self):
		self.client.privmsg(msg = "[musirc] disconnected from museekd")
	
	def cb_login_error(self, error):
		self.client.privmsg(msg = "[musirc] login error connecting to museekd: " + error)
		try:
			self.socket.close()
		except Exception, e:
			pass
		self.socket = None
	
	def cb_server_state(self, state, username):
		if not state:
			self.client.privmsg(msg = "[musirc] not connected to soulseek")
		else:
			self.client.privmsg(msg = "[musirc] connected to soulseek")
		self.client.nick(newnick = username)
	
	def cb_private_message(self, timestamp, user, message):
		self.client.privmsg(whom = user, msg = message)
	
	def cb_room_state(self, roomlist, joined_rooms, tickers):
		for key in joined_rooms.keys():
			self.client.join(key, joined_rooms[key].keys())
	
	def cb_room_said(self, room, user, message):
		if user != self.client.nickname:
			self.client.roommsg(whom = user, room = room, msg = message)
	
class clientHandler:
	def __init__(self, socket, hostname, password, clients):
		self.socket = socket
		self.socket.setblocking(0)
		self.inbuf = ""
		self.authenticated = False
		self.hostname = hostname
		self.password = password
		self.nickname = None
		self.login_password = None
		self.userAlias = {}
		self.userAliasR = {}
		self.driver = museekDriver(self)
		self.clients = clients
	
	def fixname(self, name, lookup = True):
		if lookup and self.userAlias.has_key(name):
			return self.userAlias[name]
		return urllib.quote(name).replace("%", "^")
	
	def unfixname(self, name, lookup = True):
		if lookup and self.userAliasR.has_key(name):
			return self.userAliasR[name]
		return urllib.unquote(name.replace("^", "%"))
	
	def read(self):
		data = self.socket.recv(8192)
		if not data:
			if self.driver.socket:
				self.driver.socket.close()
				self.driver.socket = None
			return False
		
		inbuf = self.inbuf + data
		s = inbuf.split("\r\n")
		self.inbuf = s[-1]
		s = s[:-1]
		for line in s:
			if not line:
				continue
			s = line.split(" ", 1)
			cmd = s[0].upper()
			if len(s) > 1:
				args = s[1]
			else:
				args = None
			if not hasattr(self, cmd):
				print "unhandled command:", line
				continue
			else:
				print "<", line
			getattr(self, cmd)(args)
		return True
	
	def send(self, whom = None, cmd = "", args = [], lookup = True):
		if whom is None:
			whom = "musirc"
		else:
			whom = self.fixname(whom, lookup)
		if isinstance(cmd, int):
			cmd = "%03i" % cmd
		if isinstance(args, str):
			args = [args]
		line = ":%s %s %s" % (whom, cmd, " ".join(args))
		print ">", line
		self.socket.setblocking(1)
		self.socket.send(line + "\r\n")
		self.socket.setblocking(0)
	
	def sendto(self, whom = None, cmd = "", to = None, args = []):
		if to is None:
			to = self.nickname
		if isinstance(args, str):
			args = [args]
		self.send(whom, cmd, args = [self.fixname(to)] + args)
	
	def notenough(self, cmd):
		self.send(cmd = 461, args = [cmd, ":Not enough parameters"])
	
	def PASS(self, args):
		if self.authenticated:
			self.send(cmd = 462, args = ":Unauthorized command (already registered)")
			return
		
		if args is None:
			self.notenough("PASS")
			return
		
		self.login_password = args
	
	def nick(self, oldnick = None, newnick = ""):
		if oldnick is None:
			oldnick = self.nickname
		
		self.send(whom = oldnick, cmd = "NICK", args = self.fixname(newnick, False), lookup = False)
		
		if oldnick == self.nickname:
			self.nickname = newnick
	
	def NICK(self, args):
		if not args:
			self.send(cmd = 431, args = ":No nickname given")
		
		if not self.authenticated:
			if self.password != self.login_password:
				self.send(cmd = 464, args = ":Password incorrect")
				return
			else:
				self.nickname = args
				self.authenticated = True
				self.sendto(cmd = 001, args = ":Welcome to musirc")
				self.login = None
	
	def USER(self, args):
		if not self.authenticated:
			return
		
		if self.login:
			self.send(cmd = 462, args = ":Unauthorized command (already registered)")
			return
		
		login = args.split(" ")
		if len(login) != 4:
			self.notenough("USER")
			return
		
		self.login = login
		self.privmsg(msg = "[musirc] Hello there, through this query you can order musirc around")
		self.privmsg(msg = "[musirc] To connect to museekd, send: CONNECT <path>|<host:port> <password>")
		
	def PING(self, args):
		self.sendto(cmd = "PONG %s" % args)
	
	def WHOIS(self, args):
		pass
	
	def privmsg(self, whom = None, to = None, msg = ""):
		if whom is None:
			whom = self.nickname
		self.sendto(whom = whom, cmd = "PRIVMSG", to = to, args = ":" + msg)
	
	def roommsg(self, whom = None, room = None, msg = None):
		if whom is None:
			whom = self.nickname
		self.send(whom = whom, cmd = "PRIVMSG", args = ["#" + self.fixname(room, False), ":" + msg])
	
	def PRIVMSG(self, args):
		s = args.split(" ", 1)
		if len(s) != 2:
			self.notenough("PRIVMSG")
			return
		
		if self.unfixname(s[0]) == self.nickname:
			s = s[1][1:].split(" ", 1)
			cmd = s[0].upper()
			if len(s) == 2:
				args = s[1]
			else:
				args = ""
			if cmd == "PING":
				self.privmsg(msg = "PONG")
			elif cmd == "ALIAS":
				s = args.split(" ")
				if len(s) != 2:
					self.privmsg(msg = "[musirc] usage: ALIAS <shortname> <longname>")
				else:
					if self.userAlias.has_key(s[0]):
						self.nick(oldnick = s[0], newnick = self.userAlias[s[0]])
						del self.userAliasR[self.usersAlias[s[0]]]
						del self.userAlias[s[0]]
					if self.userAliasR.has_key(s[1]):
						self.nick(oldnick = self.userAliasR[s[1]], newnick = s[1])
						del self.userAlias[self.userAliasR[s[1]]]
						del self.userAliasR[s[1]]
					self.userAlias[s[0]] = s[1]
					self.userAliasR[s[1]] = s[0]
					self.privmsg(msg = "[musirc] added alias")
					if s[1] != self.nickname:
						self.nick(oldnick = s[1], newnick = s[0])
			elif cmd == "UNALIAS":
				if not self.userAlias.has_key(args):
					self.privmsg(msg = "[musirc] no such alias")
				else:
					old = self.userAlias[args]
					if args != self.nickname:
						self.nick(oldnick = args, newnick = old)
					del self.userAlias[args]
					del self.userAliasR[old]
					self.privmsg(msg = "[musirc] alias removed")
			elif cmd == "CONNECT":
				if self.driver.socket:
					self.privmsg(msg = "[musirc] already connected to museekd")
				else:
					s = args.split(" ")
					if len(s) != 2:
						self.privmsg(msg = "[musirc] usage: CONNECT <path>|<host:port> <password>")
					else:
						try:
							self.driver.connect(s[0], s[1], messages.EM_PRIVATE|messages.EM_CHAT)
							self.privmsg(msg = "[musirc] connected to museekd")
						except Exception, e:
							self.privmsg(msg = "[musirc] couldn't connect to museekd: " + str(e))
			else:
				self.privmsg(msg = "[musirc] unknown command: %s" % cmd)
		else:
			if s[0][0] == "#":
				self.driver.send(messages.SayRoom(self.unfixname(s[0][1:], False), s[1][1:]))
				for client in self.clients:
					if client != self and client.driver.socket:
						client.roommsg(whom = self.nickname, room = self.unfixname(s[0][1:]), msg = s[1][1:])
			else:
				self.driver.send(messages.PrivateMessage(self.unfixname(s[0]), s[1][1:]))
	
	def join(self, room, users):
		room = "#" + self.fixname(room)
		
		self.send(whom = self.nickname, cmd = "JOIN", args = ":" + room)
		self.sendto(cmd = 331, args = [room, ":No topic is set"])
		users = ["+" + self.fixname(user) for user in users]
		self.sendto(cmd = 353, args = ["=", room, ":" + " ".join(users)])
		self.sendto(cmd = 366, args = [room, ":End of NAMES list."])
	
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.bind(("", 2241))
s.listen(5)
clients = []
while 1:
	socks = [s]
	for client in clients:
		if client.socket:
			socks.append(client.socket)
		if client.driver.socket:
			socks.append(client.driver.socket)
	
	r, w, x = select.select(socks, [], [], None)
	if s in r:
		sock, addr = s.accept()
		clients.append(clientHandler(sock, addr[0], "secret", clients))
	remove = []
	for client in clients:
		if client.driver.socket in r:
			client.driver.process()
		if client.socket in r:
			if not client.read():
				remove.append(client)
	for client in remove:
		clients.remove(client)

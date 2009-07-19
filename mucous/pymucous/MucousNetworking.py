# This is part of the Mucous Museek Client, and distributed under the GPLv2
# Copyright (c) 2006 daelstorm. 
try:
	import messages, driver
except:
	try:
		from museek import messages, driver
	except:
		print "WARNING: The Museek Message-Parsing modules, messages.py and/or driver.py  were not found. Please install them into your '/usr/lib/python2.X/site-packages/museek' directory, or place them in a 'museek' subdirectory of the directory that contains the mucous python script."
		sys.exit()
import threading
import os, sys, time
import curses.wrapper
import select, socket
## Driver handle
## @param driver Museek driver bindings
class Networking(driver.Driver):
	## Constructor
	# @param self Networking (Driver Class)
	# @param mucous Mucous (Class)
	def __init__(self, mucous):
		driver.Driver.__init__(self, self.Error)
		## @var mucous
		# Mucous (Class)
		self.mucous = mucous
		
	## Connect to museekd, password, a museekd socket need to be set
	# @param self Networking (Driver Class)
	# If mucous cannot connect, enter a loop which allows commands to be inputted
	def connect(self):
		try:

		
			if not self.mucous.invalidpass:
				if self.mucous.Config["connection"]["passw"] != None:
					
					self.mucous.timers["nick"].cancel()
					self.mucous.timers["nick"] = threading.Timer(10.0, self.mucous.ThreadNickCheck)
					self.mucous.timers["nick"].start()
					
					driver.Driver.connect(self, self.mucous.Config["connection"]["interface"],  self.mucous.Config["connection"]["passw"], messages.EM_CHAT |  messages.EM_USERINFO| messages.EM_PRIVATE| messages.EM_TRANSFERS  | messages.EM_USERSHARES | messages.EM_CONFIG |  messages.EM_INTERESTS | messages.EM_DEBUG)
					
					#break
				else:
					raise Exception,  "No Password Set"
			else:
				raise Exception,  "INVPASS"
				

		except KeyboardInterrupt, e:
			# Ctrl-C Exits Mucous
			raise KeyboardInterrupt,  ""
		except select.error, e:
			self.mucous.Help.Log("status", "Connection Error2 "+str( e) )
			raise select.error, e
		except Exception, e:
			self.mucous.Help.Mode()
			if e == "INVPASS":
				self.mucous.Help.Log("status", "Incorrect Password, try another.")
			elif str(e) == str((111, 'Connection refused')):
				
				self.mucous.Help.Log("status", e[1] +", make sure the daemon is running, or change the interface.")
			else:
				self.mucous.Help.Log("status", "Connection Error "+str( e) )
			if self.mucous.timers["nick"] != None:
				self.mucous.timers["nick"].cancel()

	def Error(self, message):
		self.mucous.Help.Log("debug", message)
		
	## Recieve Messages from Museekd and collect new key presses
	# @param self Networking (Driver Class)
	def processWrap(self):

		while self.mucous._run:

			if self.socket is None:
				time.sleep(0.5)
				continue
			
			read, write, exception = select.select([self.socket], [], [self.socket], 0)
			if self.socket in read:
				self.process()
		
			time.sleep(0.05)

	## Recieved Ping from museekd
	# @param self Networking (Driver Class)
	def cb_ping(self):
		self.mucous.Help.Log("debug", "Recieved ping from daemon...")
		
	## Recieved Login Error from museekd
	# @param self Networking (Driver Class)
	# @param reason is a string containing the reason for login failure
	def cb_login_error(self, reason):
		try:
			self.mucous.timers["nick"].cancel()
			self.mucous.Spl["connected"] = 0
			self.close()
			if reason == "INVPASS":
				self.mucous.invalidpass = True
				self.mucous.Help.Mode()
				self.mucous.Help.Log("status", "Couldn't log in to Museekd: Invalid Password")
				
				#self.connect()
			else:
				self.mucous.invalidpass = False
				self.mucous.Help.Log("status", "Couldn't log in to Museekd: " + reason)
		except Exception,e:
			self.mucous.Help.Log("debug", "cb_login_error: " + str(e))
			
	## Recieved Login Okay from museekd
	# @param self Networking (Driver Class)
	def cb_login_ok(self):
		try:
			self.mucous.invalidpass = False
			self.mucous.Spl["connected"] = 1
			self.mucous.Help.Log("status", "Logging into Museek at "+ self.mucous.Config["connection"]["interface"])
			self.mucous.timers["timeout"] = threading.Timer(self.mucous.timeout_time, self.mucous.AwayTimeout)
			self.mucous.timers["timeout"].start()
		except Exception,e:
			self.mucous.Help.Log("debug", "cb_login_ok: " + str(e))
		
	## Museekd notified that we are disconnected
	# @param self Networking (Driver Class)
	def cb_disconnected(self):
		try:
			if self.mucous.Spl["connected"] == 1:
				try:
					#driver.Driver.close(self)
					self.D.close()
				except:
					pass
				self.mucous.Spl["connected"] = 0
			self.mucous.logs["onlinestatus"]="Closed"
			self.mucous.Muscan.timer.cancel()
			self.mucous.timers["nick"].cancel()
			self.mucous.ChatRooms.ticker_timer.cancel()
			self.mucous.timers["retry"].cancel()
			self.mucous.timers["clear"].cancel()
			self.mucous.timers["timeout"].cancel()
			self.mucous.username = None
			self.mucous.DrawOnlineStatus()

			
			for room in self.mucous.ChatRooms.rooms.keys():
				msg = ("Disconnected from the Museek Daemon")
				self.mucous.ChatRooms.AppendChat("Status", room, '', msg)
				self.mucous.ChatRooms.rooms[room] = {}
				self.mucous.ChatRooms.tickers[room] = {}
				
			uploadlist = []
			self.mucous.Transfers.uploads = {}
			self.mucous.Transfers.downloads = {}
			self.mucous.Transfers.transfers["downloads"] = {}
			self.mucous.Transfers.transfers["uploads"] = {}
			self.mucous.config = {}
			

			if self.mucous.mode == "chat":
				self.mucous.ChatRooms.Mode()
			elif self.mucous.mode == "transfer":
				self.mucous.Transfers.ModeTransfers()
			self.mucous.TerminalTitle()
			self.mucous.Transfers.DrawUploadCount("0")
			self.mucous.Transfers.DrawDownloadCount("0")

			self.mucous.refresh_windows()
		except Exception,e:
			self.mucous.Help.Log("debug", "cb_disconnected: " + str(e))
			
	## Museekd sent us a status message
	# @param self Networking (Driver Class)
	# @param type is a bool; False if the message relates to the Server / True for Peer
	# @param message is the message string
	def cb_status_message(self, type, message):
		try:
			if type == 1:
				stype = "Peer"
			elif type == 0:
				stype = "Server"
			self.mucous.Help.Log("status", "%s Message: %s" % (stype, message))
		except Exception,e:
			self.mucous.Help.Log("debug", "cb_status_message: " +str( e) )
			
	## Museekd sent us a debug message
	# @param self Networking (Driver Class)
	# @param domain is a string the value of which is a debug type
	# @param message is the message string
	def cb_debug_message(self, domain, message):
		try:
			if domain in ["museek.note", "museek.warn"] :
				self.mucous.Help.Log("status", "%s Message: %s" % (domain, message))
		except Exception,e:
			self.mucous.Help.Log("debug", "cb_debug_message: " +str( e) )
			
	## Museekd sent us the server state and our username
	# @param self Networking (Driver Class)
	# @param state is a bool; False if disconnected from the server / True if connected
	# @param username is a string
	def cb_server_state(self, state, username):
		try:
			self.mucous.username = username
			un = self.mucous.windows["border"]["username"]
			un.erase()
			un.addstr(self.mucous.dlang(self.mucous.username[:15]), self.mucous.colors["blackwhite"] )
			un.refresh()
		
			#self.mucous.Transfers.DrawUploadCount("0")
			#self.mucous.Transfers.DrawDownloadCount("0")
			#self.mucous.Search.Count(0)
		
			if state:
# 				self.mucous.Help.Log("status", "Connected to Server, username: " + username)
				
				self.mucous.logs["onlinestatus"]="Online"
				
	
				if self.mucous.ChatRooms.rooms.keys():
					for room in self.mucous.ChatRooms.rooms.keys():
						msg = ("Connected")
						self.mucous.ChatRooms.AppendChat("Status", room, '', msg)
				
			else:
				self.mucous.Help.Log("status", "Museek is not connected to Soulseek")
	
				self.mucous.logs["onlinestatus"]="Offline"
				
				
	
				if self.mucous.ChatRooms.rooms.keys():
					for room in self.mucous.ChatRooms.rooms.keys():
						msg = ("Disconnected from the Server")
						self.mucous.ChatRooms.AppendChat("Status", room, '', msg)
				#uploadlist = []
				#self.mucous.Transfers.uploads = {}
				#self.mucous.Transfers.transfers["downloads"] = {}
				#self.mucous.Transfers.transfers["uploads"] = {}
				
				# Clear users from rooms
				for room in self.mucous.ChatRooms.rooms.keys():
					self.mucous.ChatRooms.rooms[room] = []
				if self.mucous.mode == "chat":
					self.mucous.ChatRooms.Mode()
				elif self.mucous.mode == "transfer":
					self.mucous.Transfers.ModeTransfers()
			self.mucous.DrawOnlineStatus()

			
		except Exception,e:
			self.mucous.Help.Log("debug", "cb_server_state: " +str( e) )
		self.mucous.TerminalTitle()
		
	## Time left of server privileges
	# @param self Networking (Driver Class)
	# @param time_left Seconds of privileges left
	def cb_server_privileges(self, time_left):
		try: 
			time = time_left
			hours_i = time/3600
			minutes_i = time/60
			seconds_i = time - (60 * minutes_i)
			if minutes_i > 59:
				minutes_i = time/60 - (60 * hours_i)
				
			days = hours_i/24
			hours = hours_i - (days*24)
			if time:
				stime = 'You have %d Days, %2.2d:%2.2d:%2.2d of privileges left' % (days, hours, minutes_i, seconds_i)
			else:
				stime = 'You have no global privileges.'
			self.mucous.Help.Log("status", stime)
		except Exception,e:
			self.mucous.Help.Log("debug", "cb_server_privileges: " +str( e) )
		
	## Recieved Room List
	# @param self Networking (Driver Class)
	# @param roomlist dict of [rooms][users][stats]
	def cb_room_list(self, roomlist):
		try:
			alpha_list = self.mucous.SortedDict()
			for name in  roomlist:
				alpha_list[name] = roomlist[name]
			
			self.mucous.RoomsList.rooms = {}
			
			for x, y in alpha_list.items():
				self.mucous.RoomsList.rooms[x] = y
				
			if self.mucous.mode=="roomlist":
				self.mucous.RoomsList.Mode()
		except Exception, e:
			self.mucous.Help.Log("debug", "CB Room List" + str(e))
			
	## Got Global Recommendations
	# @param self Networking (Driver Class)
	# @param recommendations list of recommendations [item, number of recommends] 
	def cb_get_global_recommendations(self, recommendations):
		try:
			self.mucous.Recommendations.data["recommendations"] = self.mucous.SortedDict()
			for rec, num in recommendations.items():
				self.mucous.Recommendations.data["recommendations"] [rec] = num
			if self.mucous.mode == "lists" and self.mucous.UsersLists.current == "interests":
				self.mucous.Recommendations.DrawInterests()
		except Exception, e:
			self.mucous.Help.Log("debug", "CB Get Global Recommendations" + str(e))
	
	## Got Similar Users list
	# @param self Networking (Driver Class)
	# @param users List of format [username, status=(0,1,2)]
	def cb_get_similar_users(self, users):
		try:
			
			self.mucous.Recommendations.data["similar_users"] = self.mucous.SortedDict()
			for rec, num in users.items():
				self.mucous.Recommendations.data["similar_users"][rec] = num
			if self.mucous.mode == "lists" and self.mucous.UsersLists.current == "interests":
				self.mucous.Recommendations.DrawInterests()
		except Exception, e:
			self.mucous.Help.Log("debug", "CB Similar Users" + str(e))
	
	## Got Personal Recommendations
	# @param self Networking (Driver Class)
	# @param recommendations list of recommendations [item, number of recommends]
	def cb_get_recommendations(self, recommendations):
		try:
			self.mucous.Recommendations.data["recommendations"] = self.mucous.SortedDict()
			for rec, num in recommendations.items():
				self.mucous.Recommendations.data["recommendations"] [rec] = num
			if self.mucous.mode == "lists" and self.mucous.UsersLists.current == "interests":
				self.mucous.Recommendations.DrawInterests()
		except Exception, e:
			self.mucous.Help.Log("debug", "CB Get  Recommendations" + str(e))
	
	## Got Similar Users list for an Item
	# @param self Networking (Driver Class)
	# @param item string
	# @param users List of format [username, status=(0,1,2)]
	def cb_get_item_similar_users(self, item, users):
		try:
			self.mucous.Recommendations.data["similar_users"] = self.mucous.SortedDict()
			for rec, num in users.items():
				self.mucous.Recommendations.data["similar_users"][rec] = num
			if self.mucous.mode == "lists" and self.mucous.UsersLists.current == "interests":
				self.mucous.Recommendations.DrawInterests()
		except Exception, e:
			self.mucous.Help.Log("debug", "CB Item Similar Users" + str(e))
	
	## Got Recommendations for an Item
	# @param self Networking (Driver Class)
	# @param item string
	# @param recommendations list of recommendations [item, number of recommends]
	def cb_get_item_recommendations(self, item, recommendations):
		try:

			self.mucous.Recommendations.data["recommendations"] = self.mucous.SortedDict()
			for rec, num in recommendations.items():
				self.mucous.Recommendations.data["recommendations"] [rec] = num
			if self.mucous.mode == "lists" and self.mucous.UsersLists.current == "interests":
				self.mucous.Recommendations.DrawInterests()
		except Exception, e:
			self.mucous.Help.Log("debug", "CB Get Item Recommendations" + str(e))

	## Someone said something in a Chat Room
	# @param self Networking (Driver Class)
	# @param room Chat Room
	# @param user Username
	# @param text message
	def cb_room_said(self, room, user, text):
		try:
			#text = text.replace('\n', " ").replace('\t', "     ")
			self.mucous.ChatRooms.SaidInRoom(room, user, text)

		except Exception, e:
			self.mucous.Help.Log("debug", "CB Room Said" + str(e))
	
	## Recieved Room state 
	# @param self Networking (Driver Class)
	# @param roomlist Dict of rooms and the number of users in them
	# @param joinedrooms Dict of Rooms we have joined
	# @param tickers Dict of tickers [room][user] = ticker 
	def cb_room_state(self, roomlist, joinedrooms, tickers):
		try:
			for rooms1, numbers in roomlist.items():
				self.mucous.RoomsList.rooms[rooms1] = numbers
			
			for room in joinedrooms:
				self.mucous.ChatRooms.Joined(room, joinedrooms[room], tickers[room])
			
			joined = self.mucous.ChatRooms.rooms.keys()
			joined.sort(key=str.lower)
			
			if joined == []:
				return
			
			if self.mucous.Config["rooms"]["default_room"] != None:
				if self.mucous.Config["rooms"]["default_room"] in joined:
					self.mucous.ChatRooms.current = self.mucous.Config["rooms"]["default_room"]
					self.mucous.ChatRooms.Change(self.mucous.Config["rooms"]["default_room"])
				else:
					self.mucous.ChatRooms.JoinRoom(self.mucous.Config["rooms"]["default_room"])
					if len(joined) != 0:
						self.mucous.ChatRooms.Change(joined[0])

			else:
				self.mucous.ChatRooms.current = joined[0]
				self.mucous.ChatRooms.Change(joined[0])

		except Exception, e:
			self.mucous.Help.Log("debug", "CB Room state" + str(e))
				
	## We Joined a room
	# @param self Networking (Driver Class)
	# @param room Room name
	# @param users Dict of users in the room
	def cb_room_joined(self, room, users, private, owner, operators):
		try:
			self.mucous.ChatRooms.Joined(room, users)

			if self.mucous.ChatRooms.current == None or self.mucous.ChatRooms.current == room:
				self.mucous.ChatRooms.Change(room)
			curses.doupdate()
		except Exception, e:
			self.mucous.Help.Log("debug", "CB Room Joined: " + str(e))
			
	## We left a room
	# @param self Networking (Driver Class)
	# @param room Room name
	def cb_room_left(self, room):
		try:
			self.mucous.ChatRooms.Left(room)
		except Exception, e:
			self.mucous.Help.Log("debug", "CB Room Left: " + str(e))
			
	## A user joined a room we are in
	# @param self Networking (Driver Class)
	# @param room Room name
	# @param user User name
	# @param data status, speed, downloads, files, dirs, other
	def cb_room_user_joined(self, room, user, data):
		try:
			self.mucous.ChatRooms.UserJoined( room, user, data)

		except Exception, e:
			self.mucous.Help.Log("debug", "CB Room User Joined" + str(e))
			
	## A user left a room we are in
	# @param self Networking (Driver Class)
	# @param room Room name
	# @param user User name
	def cb_room_user_left(self, room, user):
		try:
			did = "left"
			what = None
			if self.mucous.config !=  {}: 
				if self.mucous.config.has_key("ignored") and user not in self.mucous.config["ignored"].keys():
					self.mucous.ChatRooms.AppendStatus(user, room, did, what)
				else:
					pass
			# correct placement in roombox
			if  self.mucous.mode == "chat" and self.mucous.ChatRooms.selected  == "roombox":
				self.mucous.ChatRooms.rooms[room].sort(key=str.lower)
				if self.mucous.ChatRooms.rooms[room].index(user) < self.mucous.ChatRooms.scrolling[self.mucous.ChatRooms.selected]:
					self.mucous.ChatRooms.scrolling[self.mucous.ChatRooms.selected] -= 1
			if user in self.mucous.ChatRooms.rooms[room]:
				self.mucous.ChatRooms.rooms[room].remove(user)
			if room in self.mucous.ChatRooms.tickers:
				if user in self.mucous.ChatRooms.tickers[room]:
					del self.mucous.ChatRooms.tickers[room][user]
			if self.mucous.mode == "chat" and self.mucous.ChatRooms.current == room:
				self.mucous.ChatRooms.DrawBox()
				for lines in self.mucous.ChatRooms.logs["rooms"][room][len(self.mucous.ChatRooms.logs["rooms"][self.mucous.ChatRooms.current]) - self.mucous.ChatRooms.dimensions["chat"]["height"]:]:
					# Update Chat history if user changes status
					if lines[2] == user:
						self.mucous.ChatRooms.Change(room)
						break
				curses.doupdate()
		except Exception, e:
			self.mucous.Help.Log("debug", "CB Room User Left" + str(e))
			
	## A user's status changed
	# @param self Networking (Driver Class)
	# @param user User name
	# @param status (1=away|2=online|3=offline)
	def cb_peer_status(self, user, status):
		try:
			if status == 1: what = "away"
			elif status == 2: what = "online"
			elif status == 0: what = "offline"
			
			if user in self.mucous.user["status"]:
				if self.mucous.user["status"][user] == status:
					return
				else: 
					self.mucous.user["status"][user] = status
			else:
				self.mucous.user["status"][user] = status

			room = None
			did = "change"
			if self.mucous.config !=  {}: 
				if "ignored" in self.mucous.config.keys(): 
					if user not in self.mucous.config["ignored"].keys():
						self.mucous.ChatRooms.AppendStatus(user, room, did, what)
						
			if self.mucous.mode == "chat":
				if self.mucous.ChatRooms.current != None:
					if user in self.mucous.ChatRooms.rooms[self.mucous.ChatRooms.current]:
						self.mucous.ChatRooms.DrawBox()
						curses.doupdate()
			elif self.mucous.mode in ("private", "info", "browse", "lists"):
				self.mucous.ModeReload(user)
		except Exception, e:
			self.mucous.Help.Log("debug", "CB Peer Status" + str(e))
			
	## Recieved a user's info
	# @param self Networking (Driver Class)
	# @param user the Username
	# @param info the description
	# @param picture the image, if it exists
	# @param uploads number of uploads slots
	# @param queue Length of queue
	# @param slotsfree has free slots?	
	def cb_user_info(self, user, info, picture, uploads, queue, slotsfree):
		try:
			
			self.mucous.UserInfo.Recieved(user, info, picture, uploads, queue, slotsfree)

		except Exception, e:
			self.mucous.Help.Log( "debug", "cb_user_info: " + str(e))
	
	## Recieved a user's ip and port from the server
	# @param self Networking (Driver Class)
	# @param user the Username
	# @param ip ip address
	# @param port user's listen port
	def cb_peer_address(self, user, ip, port):
		try:
			if user not in self.mucous.requests["ip"]:
				return
			self.mucous.requests["ip"].remove(user)
			
			if self.mucous.geoip_fail==0:
				try:
					country =  self.mucous.gi.country_name_by_addr( str(ip) )
					self.mucous.Help.Log("status", "%s's IP: %s Port: %s Country: %s"  % (user, str(ip), str(port), country) )
				except Exception, e:
					self.mucous.Help.Log("debug", "CB Peer Address: " + str(e))
			else:
				self.mucous.Help.Log("status","%s's IP: %s Port: %s"  % (user, str(ip), str(port)) )
			if self.mucous.mode not in ("status", "debug"):
				self.mucous.Alerts.setStatus("New IP")
		except Exception, e:
			self.mucous.Help.Log( "debug", "cb_peer_address: " + str(e))
			
	## Recieved peer stats from museekd
	# @param self Networking (Driver Class)
	# @param user the Username
	# @param avgspeed Average Speed of user
	# @param numdownloads Number of files user has downloaded
	# @param numfiles Number of files share
	# @param numdirs Number of directories shared
	def cb_peer_stats(self, user, avgspeed, numdownloads, numfiles, numdirs, slotsfull, country):
		try:
			self.mucous.user["statistics"][user] = avgspeed, numdownloads, numfiles, numdirs
			if user in self.mucous.requests["statistics"]:
	
				self.mucous.Help.Log("status", "Peer Stats for: %s Speed: %.2f Kbyte/s :: Downloads: %s :: Files: %s :: Directories: %s" % (user, (avgspeed/1024.0), numdownloads, numfiles, numdirs) )
				
				self.mucous.requests["statistics"].remove(user)

			if user == self.mucous.username:
				self.mucous.data["mystats"] = user,  avgspeed, numdownloads, numfiles, numdirs
				if self.mucous.mode == "setup":
					self.mucous.Setup.Mode()
		except Exception, e:
			self.mucous.Help.Log( "debug", "cb_peer_stats: " + str(e))
			
	## Recieved private message from the server
	# @param self Networking (Driver Class)
	# @param direction 0 == incoming; 1 == outgoing
	# @param timestamp (we use our own)
	# @param user username
	# @param message text
	def cb_private_message(self, direction, timestamp, user, message):
		try:
			
			self.mucous.PrivateChat.Recieved(direction, timestamp, user, message)
			
				
		except Exception, e:
			self.mucous.Help.Log( "debug", "cb_private_message: " + str(e))
				
	## Recieved a notification from the server of your away status
	# @param self Networking (Driver Class)
	# @param status away=1/online=0
	def cb_server_status_set(self, status):
		try:
			self.mucous.Spl["status"] = status
			if status:
				stat = "Away"
			else:
				stat = "Online"
			
			self.mucous.logs["onlinestatus"]=stat
			self.mucous.DrawOnlineStatus()

			self.mucous.TerminalTitle()
		except Exception, e:
			self.mucous.Help.Log( "debug", "cb_server_status_set: " + str(e))
			
	## Recieved list of tickers for room
	# @param self Networking (Driver Class)
	# @param room Room name
	# @param tickers Dict of Users and Tickers
	def cb_room_tickers(self, room, tickers):
		
		try:
			for message, user in tickers.items():
				if "room" not in self.mucous.ChatRooms.tickers:
					self.mucous.ChatRooms.tickers[room] = {}
				self.mucous.ChatRooms.tickers[room][user] = message
		except Exception, e:
			self.mucous.Help.Log( "debug", "cb_room_tickers: " + str(e))
			
	## A user in room set their ticker to message
	# @param self Networking (Driver Class)
	# @param room Room name
	# @param user User name
	# @param message ticker
	def cb_room_ticker_set(self, room, user, message):
		try:
			what = message
			did = "ticker"
			if self.mucous.config !=  {}: 
				if "ignored" in self.mucous.config.keys(): 
					if user not in self.mucous.config["ignored"].keys():
						self.mucous.ChatRooms.AppendStatus(user, room, did, what)
			if room in self.mucous.ChatRooms.tickers.keys():
				self.mucous.ChatRooms.tickers[room][user] = message
					
		except Exception, e:
			self.mucous.Help.Log( "debug", "cb_room_ticker_set: " + str(e))
			
	## New Search ticket recieved
	# @param self Networking (Driver Class)
	# @param query string searched for
	# @param ticket unique number associated with search
	def cb_search_ticket(self, query, ticket):
		try:
			self.mucous.Search.NewTicket(query, ticket)

		except Exception, e:
			self.mucous.Help.Log( "debug", "cb_search_ticket: " + str(e))
	
	## Recieved search results from a user
	# @param self Networking (Driver Class)
	# @param ticket unique number (used to organize results)
	# @param user username of user with results
	# @param free is there a free slot open? (True/False)
	# @param speed average speed of user
	# @param queue length of queue
	# @param results list of files [path, size, extension, list of attributes(bitrate, length, unused)]
	def cb_search_results(self, ticket, user, free, speed, queue, results):
		# search results
		try:
			self.mucous.Search.NewResults(ticket, user, free, speed, queue,results)
		except Exception, e:
			self.mucous.Help.Log("debug", "CB User Shares: " + str(e))

	## Recieved a user's shares
	# @param self Networking (Driver Class)
	# @param user Username
	# @param shares Dict of shares
	def cb_user_shares(self, user, shares):
		try:
			self.mucous.BrowseShares.Recieved(user, shares)
					
		except Exception, e:
			self.mucous.Help.Log("debug", "cb_search_results: " + str(e))

	## Recieved Transfer State from museekd
	# @param self Networking (Driver Class)
	# @param downloads list of instances of transfers
	# @param uploads list of instances of transfers
	def cb_transfer_state(self, downloads, uploads):
		try:
			for transfer in uploads:
				self.mucous.Transfers.transfers["uploads"][(transfer.user, transfer.path)] =  [transfer.is_upload, transfer.user, transfer.path, int(transfer.state), transfer.error, transfer.filepos, transfer.filesize, transfer.rate, transfer.place]
			self.mucous.Transfers.DrawUploadCount(str(len(self.mucous.Transfers.transfers["uploads"].keys())))
			
			for transfer in downloads:
				self.mucous.Transfers.transfers["downloads"][(transfer.user, transfer.path)] =  [transfer.is_upload, transfer.user, transfer.path, int(transfer.state), transfer.error, transfer.filepos, transfer.filesize, transfer.rate, transfer.place]
	
			self.mucous.Transfers.DrawDownloadCount(str(len(self.mucous.Transfers.transfers["downloads"].keys())))		
			if self.mucous.mode == "transfer":
				if self.mucous.Config["mucous"]["transbox"] == "split":
					self.mucous.Transfers.UploadManager()
					self.mucous.Transfers.DownloadManager()
					curses.doupdate()
				else:
					if self.mucous.Transfers.Transfers.current == "uploads":
						self.mucous.Transfers.UploadManager()
						curses.doupdate()
					else:
						self.mucous.Transfers.DownloadManager()
						curses.doupdate()
			if self.mucous.Config["mucous"]["auto-retry"] == "yes":
				self.mucous.timers["retry"].cancel()
				self.mucous.timers["retry"] = threading.Timer(30.0, self.mucous.ThreadTransfersRetry)
				self.mucous.timers["retry"].start()
			if self.mucous.Config["mucous"]["auto-clear"] == "yes":
				self.mucous.timers["clear"].cancel()
				self.mucous.timers["clear"] = threading.Timer(30.0, self.mucous.ThreadTransfersClear)
				self.mucous.timers["clear"].start()
		except Exception, e:
			self.mucous.Help.Log("debug", "cb_transfer_state: " + str(e))
		
	## Recieved A Transfer update
	# @param self Networking (Driver Class)
	# @param transfer transfer instance
	def cb_transfer_update(self, transfer):
		try:
			if transfer.is_upload:
				self.mucous.Transfers.transfers["uploads"][(transfer.user, transfer.path)] =   [transfer.is_upload, transfer.user, transfer.path, int(transfer.state), transfer.error, transfer.filepos, transfer.filesize, transfer.rate, transfer.place]
				if self.mucous.mode == "transfer":
					if self.mucous.Config["mucous"]["transbox"] == "split":
						self.mucous.Transfers.UploadManager()
						curses.doupdate()
					else:
						if self.mucous.Transfers.current == "uploads":
							self.mucous.Transfers.UploadManager()
							curses.doupdate()
				self.mucous.Transfers.DrawUploadCount(str(len(self.mucous.Transfers.transfers["uploads"].keys())))
			else:
		
				self.mucous.Transfers.transfers["downloads"][(transfer.user, transfer.path)] =  [transfer.is_upload, transfer.user, transfer.path, int(transfer.state), transfer.error, transfer.filepos, transfer.filesize, transfer.rate, transfer.place]
				if self.mucous.mode == "transfer":
					if self.mucous.Config["mucous"]["transbox"] == "split":
						self.mucous.Transfers.DownloadManager()
						curses.doupdate()
					else:
						if self.mucous.Transfers.current == "uploads":
							pass
						else:
							self.mucous.Transfers.DownloadManager()
							curses.doupdate()
				self.mucous.Transfers.DrawDownloadCount(str(len(self.mucous.Transfers.transfers["downloads"].keys())))
			if self.mucous.mode == "transfer":
				if self.mucous.PopupMenu.show == True:
					self.mucous.PopupMenu.Create()
		except Exception, e:
			self.mucous.Help.Log("debug", "cb_transfer_update: " + str(e))
	## Removed a transfer
	# @param self Networking (Driver Class)
	# @param transfer transfer instance
	def cb_transfer_remove(self, transfer):
		try:
			user_path = transfer[1], transfer[2]
			if transfer[0]:
				del self.mucous.Transfers.transfers["uploads"][user_path]
				if self.mucous.mode == "transfer":
					if self.mucous.Config["mucous"]["transbox"] == "split":
						self.mucous.Transfers.UploadManager()
						curses.doupdate()
					else:
						if self.mucous.Transfers.current == "uploads":
							self.mucous.Transfers.UploadManager()
							curses.doupdate()
				self.mucous.Transfers.DrawUploadCount(str(len(self.mucous.Transfers.transfers["uploads"].keys())))
						
			else:
				del self.mucous.Transfers.transfers["downloads"][user_path]
				if self.mucous.mode == "transfer":
					if self.mucous.Config["mucous"]["transbox"] == "split":
						self.mucous.Transfers.DownloadManager()
						curses.doupdate()
					else:
						if self.mucous.Transfers.current == "uploads":
							pass
						else:
							self.mucous.DownloadManager()
							curses.doupdate()
				self.mucous.Transfers.DrawDownloadCount(str(len(self.mucous.Transfers.transfers["downloads"].keys())))
		except Exception, e:
			self.mucous.Help.Log("debug", "cb_transfer_remove: " + str(e))
	
	## Set a key/value in Mucous.config
	# @param self Networking (Driver Class)
	# @param domain parent of key
	# @param key to be modified
	# @param value key's new value
	def cb_config_set(self, domain, key, value):
		try:
			if self.mucous.config.has_key(domain) and key in self.mucous.config[domain].keys():
				if not domain.startswith("museeq"):
					self.mucous.Help.Log("status", "Modified <"+key+"> in <" +domain+"> to <"+value + ">")
			else:
				if value == '' and domain is not "userinfo" and not domain.startswith("museeq"):
					self.mucous.Help.Log("status", "Added <"+key+"> to <" +domain+">")
				else:
					self.mucous.Help.Log("status", "Added <"+key+"> to <" +domain+"> and set to <"+value+">")
			if not self.mucous.config.has_key(domain):
				self.mucous.config[domain] = {}
			self.mucous.config[domain][key] = value
			self.mucous.ConfigUpdateDisplay(domain)
		except Exception, e:
			self.mucous.Help.Log("debug", "cb_config_set: " + str(e))
			
	## Delete a key from Mucous.config
	# @param self Networking (Driver Class)
	# @param domain parent of key 
	# @param key to be removed
	def cb_config_remove(self, domain, key):
		try:
			if key in self.mucous.config[domain].keys():
				self.mucous.Help.Log("status", "Removed <"+key+"> from <" +domain+">")
				del self.mucous.config[domain][key]
			self.mucous.ConfigUpdateDisplay(domain)
		except Exception, e:
			self.mucous.Help.Log("debug", "cb_config_remove: " + str(e))
	
	## Recieved a copy of museekd's config
	# copy it to Mucous.config at connection
	# @param self Networking (Driver Class)
	# @param museek_config copy of config for internal use
	def cb_config_state(self, museek_config):
		try:
			self.mucous.config = museek_config.copy()
			self.mucous.Help.Log("status", "Server is at: "+self.mucous.config["server"]["host"]+":"+self.mucous.config["server"]["port"])
			self.mucous.UsersLists.ListBuddy()
			self.mucous.UsersLists.ListBan()
			self.mucous.UsersLists.ListIgnore()
			self.mucous.refresh_windows()
			self.mucous.Spl["museekconfigfile"] = os.path.expanduser("~/.museekd/config.xml")
			if self.mucous.config["shares"]["database"] != "":
				pos = self.mucous.config["shares"]["database"].rfind(".")
				file = self.mucous.config["shares"]["database"][:33]+".xml"
				if os.path.exists(file):
					self.mucous.Spl["museekconfigfile"] = file
		except Exception, e:
			self.mucous.Help.Log("debug", "cb_config_state: " + str(e))
			
	# -- ^ Recieved Messages from Museekd^
	
	
	## Failsafe Sending of messages to Museekd
	# @param self Networking (class)
	# @param message messages instance
	def SendMessage(self, message):
		try:
			if self.mucous.Spl["connected"] == 0: return
			self.send(message)
		except Exception, e:
			self.mucous.Help.Log("debug", "SendMessage: " + str(e))
	## Abort transfer (remains in transfer list)
	# @param self Networking (class)
	# @param direction (1: upload, 0:download)
	# @param user username
	# @param path file to be transfered
	def TransferAbort(self, direction, user, path):
		## Transfer messages
		message = messages.TransferAbort(direction, user, path)
		self.SendMessage(message)
	
	## Remove transfer from transfer list
	# @param self Networking (class)
	# @param direction (1: upload, 0:download)
	# @param user username
	# @param path file to be transfered
	def TransferRemove(self, direction, user, path):
		message = messages.TransferRemove(direction, user, path)
		self.SendMessage(message)
	
	## Check place in queue
	# @param self Networking (class)
	# @param user username
	# @param path file in queue
	def TransferUpdate(self, user, path):
		message = messages.TransferUpdate(user, path)	
		self.SendMessage(message)
	
	## Download a file
	# @param self Networking (class)
	# @param user username
	# @param path file to be transfered
	def DownloadFile(self, user, path):
		message = messages.DownloadFile(user, path)
		self.SendMessage(message)
	## Download a file to a local directory
	# @param self Networking (class)
	# @param user username
	# @param path file to be transfered
	# @param directory local save directory
	def DownloadFileTo(self, user, path, directory):
		message = messages.DownloadFileTo(user, path, directory)
		self.SendMessage(message)
	## Ask user to send the contents of a directory
	# @param self Networking (class)
	# @param user username
	# @param directory Directory to recieve
	def GetFolderContents(self, user, directory):
		message = messages.GetFolderContents(user, directory)
		self.SendMessage(message)
	## Upload a file to user from path
	# @param self Networking (class)
	# @param user username
	# @param path file to be transfered	
	def UploadFile(self, user, path):
		message = messages.UploadFile(user, path) 
		self.SendMessage(message)
	## Say message in room
	# @param self Networking (class)
	# @param room Room
	# @param message text
	def SayRoom(self, room, message):
		messages.SayRoom(room, message) 
		self.SendMessage(message)
	## Leave a Room
	# @param self Networking (class)
	# @param room leave this room
	def LeaveRoom(self, room):
		message = messages.LeaveRoom(room) 
		self.SendMessage(message)
	## Get Room List
	# @param self Networking (class)
	def RoomList(self):
		message = messages.RoomList()
		self.SendMessage(message)
	## Say line in room
	# @param self Networking (class)
	# @param room A Room you are in
	# @param line message
	def SayRoom(self, room, line):
		message = messages.SayRoom(room, line) 
		self.SendMessage(message)
	## Set your ticker in room
	# @param self Networking (class)
	# @param room Room name
	# @param ticker message
	def RoomTickerSet(self, room, ticker):
		message = messages.RoomTickerSet(room, ticker)
		self.SendMessage(message)
	## Join a room
	# @param self Networking (class)
	# @param room Room name
	def JoinRoom(self, room):
		message = messages.JoinRoom(room)
		self.SendMessage(message)
	## Get a user's IP address and listen port
	# @param self Networking (class)
	# @param user Username
	def PeerAddress(self, user):
		message = messages.PeerAddress(user)
		self.SendMessage(message)
	
	## Private Chat messages
	# @param self Networking (class)
	# @param direction 1 if outgoing 0 if incoming
	# @param user username of user message is coming from or going to
	# @param line message
	def PrivateMessage(self, direction, user, line):
		message = messages.PrivateMessage(direction, user, line) 
		self.SendMessage(message)
		
	## Get a user's away status 
	# @param self Networking (class)
	# @param user username 
	def PeerStatus(self, user):
		message = messages.PeerStatus(user)
		self.SendMessage(message)
	## Does a user exist in the server's database? 
	# @param self Networking (class)
	# @param user username 
	def PeerExists(self, user):
		self.SendMessage(messages.PeerExists(user))
	## Get a user's statistics
	# @param self Networking (class)
	# @param user username
	def PeerStats(self, user):
		message = messages.PeerStats(user)
		self.SendMessage(message)
	## Get a user's userinfo
	# @param self Networking (class)
	# @param user username
	def UserInfo(self, user):
		message = messages.UserInfo(user)
		self.SendMessage(message)
	## Get a user's shares
	# @param self Networking (class)
	# @param user username
	def UserShares(self, user):
		message = messages.UserShares(user)
		self.SendMessage(message)
	## Search for a string in one of three methods
	# @param self Networking (class)
	# @param searchtype (0:Global, 1:Buddy, 2:Rooms)
	# @param query Search string
	def Search(self, searchtype, query):
		message = messages.Search(searchtype, query )
		self.SendMessage(message)
	## Search a user's shares for a string
	# @param self Networking (class)
	# @param user Username
	# @param query Search string
	def UserSearch(self, user, query):
		message = messages.UserSearch(user, query )
		self.SendMessage(message)
	## Search via the Wishlist
	# @param self Networking (class)
	# @param query Search Query
	def WishListSearch(self, query):
		message = messages.WishListSearch(query )
		self.SendMessage(message)
	## Get a list of users with your interests
	# @param self Networking (class)
	def GetSimilarUsers(self):
		message = messages.GetSimilarUsers()
		self.SendMessage(message)
	## Get a list of recommendations related to your interests
	# @param self Networking (class)
	def GetRecommendations(self):
		message = messages.GetRecommendations()
		self.SendMessage(message)
	## Get a list of recommendations related based on popularity
	# @param self Networking (class)
	def GetGlobalRecommendations(self):
		message = messages.GetGlobalRecommendations()
		self.SendMessage(message)
	## Add A liked interest
	# @param self Networking (class)
	# @param interest string
	def AddInterest(self, interest):
		message = messages.AddInterest(interest)
		self.SendMessage(message)
	## Add a hated interest
	# @param self Networking (class)
	# @param interest string
	def AddHatedInterest(self, interest):
		message = messages.AddHatedInterest(interest)
		self.SendMessage(message)
	## Remove a liked interest
	# @param self Networking (class)
	# @param interest string
	def RemoveInterest(self, interest):
		message = messages.RemoveInterest(interest)
		self.SendMessage(message)
	## Remove a hated interest
	# @param self Networking (class)
	# @param interest string
	def RemoveHatedInterest(self, interest):
		message = messages.RemoveHatedInterest(interest)
		self.SendMessage(message)
	## Check the amount of time of server privileges we have left
	# @param self Networking (class)
	def CheckPrivileges(self):
		message = messages.CheckPrivileges()
		self.SendMessage(message)
	## Give a number of days of privileges ot a user (Must have privileges to give them)
	# @param self Networking (class)
	# @param user Username
	# @param days days of privileges
	def GivePrivileges(self, user, days):
		message = messages.GivePrivileges(user, days)
		self.SendMessage(message)
		
	## Set your away status
	# @param self Networking (class)
	# @param status (1:away, 0:online)
	def SetStatus(self, status):
		message = messages.SetStatus(status)
		self.SendMessage(message)
		
	## Museekd connect to server (Reconnect if connected)
	# @param self Networking (class)
	def ConnectServer(self):
		message = messages.ConnectServer()
		self.SendMessage(message)
	## Museekd disconnect from server
	# @param self Networking (class)
	def DisconnectServer(self):
		message = messages.DisconnectServer()
		self.SendMessage(message)
	## Museekd reload the shares db from disk
	# @param self Networking (class)	
	def ReloadShares(self):
		message = messages.ReloadShares()
		self.SendMessage(message)
	## Set an option in Museekd's config
	# @param self Networking (class)
	# @param domain parent of key
	# @param key key being changed
	# @param value value of key
	def ConfigSet(self, domain, key, value):
		message = messages.ConfigSet(domain, key, value)
		self.SendMessage(message)
	## Remove an option from Museekd's config
	# @param self Networking (class)
	# @param domain parent of key
	# @param key key being changed
	def ConfigRemove(self, domain, key):
		message = messages.ConfigRemove(domain, key)
		self.SendMessage(message)
	## Ping Museekd
	# @param self Networking (class)
	# @param num number that will be echoed back to us
	def Ping(self, num):
		message = messages.Ping(num)
		self.SendMessage(message)
		


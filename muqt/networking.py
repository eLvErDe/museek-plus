from PyQt4 import QtCore, QtGui
#from PyQt4.QtCore import *
#from PyQt4.QtGui import *
from threading import Thread

import threading, time, select, sys
DEBUG=1

try:
	from Crypto.Hash import SHA256
	
except ImportError:	
	
	try:
		import mucipher
	except ImportError:
		self.Output( "WARNING: The Mucipher Module for Python wasn't found and neither was PyCrypto. One of these encryption modules is necessary to allow Murmur to connect to the Museek Daemon.\nDownload mucipher here: http://thegraveyard.org/files/pymucipher-0.0.1.tar.gz\nExtract the tarball, and as Root or sudo, run:\npython setup.py install\nYou'll need GCC, Python and SWIG.\nOr download PyCrypto from here: http://www.amk.ca/python/code/crypto.html")
		sys.exit()
try:
	import messages, driver
except ImportError, error:
	
	try:
		from museek import messages, driver
	except ImportError, error:
		self.Output("WARNING: The Museek Message-Parsing modules, messages.py and/or driver.py  were not found. Please install them into your '/usr/lib/python2.X/site-packages/museek' directory, or place them in a 'museek' subdirectory of the directory that contains the murmur python scipt.", error)
		sys.exit()
		
class Networking(driver.Driver, QtCore.QThread):
	def __init__(self, xapp):
		driver.Driver.__init__(self)
		QtCore.QThread.__init__(self)
		self.frame = self.app=xapp
		self.config = {}
		self.socket = None
		self.timer = None
		self.connected = False
		
	def Output(self,*args):
		message = ""
		
		for arg in args:
			if type(arg) is list:
				for a in arg:
					message += a
				
			else:
				#if type(arg) is str:
					#arg = arg.split("\n")
				message += str(arg) + " "
		self.emit(QtCore.SIGNAL("Log(PyQt_PyObject)"), message)
		
	def connect_to_museekd(self, string):

	
		try:
			if self.socket is not None:
				return
			if self.frame.Config["connection"]["passw"] != None:
				if self.timer is not None: self.timer.cancel()
				self.timer = threading.Timer(10.0, self.CheckNick)
				self.timer.start()

				self.connect(self.frame.Config["connection"]["interface"],  self.frame.Config["connection"]["passw"], messages.EM_CHAT |  messages.EM_USERINFO| messages.EM_PRIVATE | messages.EM_TRANSFERS  | messages.EM_USERSHARES | messages.EM_CONFIG |  messages.EM_INTERESTS|  messages.EM_DEBUG)
					
				if DEBUG:
					self.Output("connect attempt")

			else:
				raise Exception,  "NOPASS"


		except Exception, e:
			if self.timer is not None: self.timer.cancel()

			
			#if DEBUG:
				#if "INVPASS" in e:
					#message = "Incorrect Password for the museek daemon. Please try another."
					#self.SetToolTip( "Couldn't connect to Museek: %s" % "Invalid Password")
			if  "NOPASS" in e:
				message = "Set a Password in Murmur Settings."
				#self.SetToolTip( "Couldn't connect to Museek: %s" % "Set a Password")
			elif  "Connection refused" in e:
				message = e[1] +", make sure the daemon is running, or change the interface."
				#self.SetToolTip( "Make sure Museek is running, or try a different Interface.")
			self.Output("Connect to museekd Error: ", e)
			#gobject.idle_add(self.frame.AppendToLogWindow, message)
			
			self.connected = False
			
	def cb_login_error(self, reason):
		#
		try:
			if reason == "INVPASS":
				message = "Couldn't connect to Museek: %s" % "Invalid Password"

			else:
				message = "Couldn't Login to Museek: %s" % reason
				
			#gobject.idle_add(self.frame.TrayApp.SetToolTip, message)
			self.frame.AppendToLogWindow(message)
			#gobject.idle_add(self.frame.TrayApp.SetImage,"red")
			if self.timer is not None: self.timer.cancel()
		except Exception,e:
			if DEBUG: self.Output("RECEIVED: cb_login_error ERROR", e)
		self.connected = False
		#

	def cb_login_ok(self):
		
		try:
			message = "Logging in to Museek..."
			#gobject.idle_add(self.frame.AppendToLogWindow, message)
		except Exception,e:
			if DEBUG: self.Output("RECEIVED: cb_login_ok ERROR", e)
		self.connected = True
		
	def disconnect(self, string=""):
		print "disconnect"
		try:
			if self.socket != None:
				self.close()
				self.connected = False
				self.config = {}
				self.frame.ui.Downloads.clear()
				self.frame.ui.Uploads.clear()
			if self.timer is not None: self.timer.cancel()
			self.cb_disconnected()
		except Exception,e:
			if DEBUG: self.Output("disconnect ERROR", e)
		
	def cb_disconnected(self):
		
		if DEBUG: self.Output("RECEIVED: cb_disconnected")
		try:
			
			#gobject.idle_add(self.frame.TrayApp.SetImage,"red")

			#gobject.idle_add(self.frame.TrayApp.SetToolTip, "Disconnected from Museek")
			
			#gobject.idle_add(self.frame.update_statusbar, "Disconnected from Museek")
			
			self.frame.username = None
			self.frame.status = 2
			self.config = {}

			#self.frame.ChatRooms.ConnClose()
			self.emit(QtCore.SIGNAL("ConnClose()"))
			self.connected = False
			if self.timer is not None: self.timer.cancel()
		except Exception, e:
			if DEBUG: self.Output("RECEIVED: cb_disconnected ERROR", e)
		
		
	def CheckNick(self):
		try:

			if self.frame.username != None:
				# We have a username, so we probably logged in successfully
				return
			message = "Connection is taking a while to start, maybe you are trying to connect to something besides a museek daemon, or you museek daemon is not started? Closing socket to %s.." % self.frame.Config["connection"]["interface"]
			if DEBUG:
				
				self.Output(message)
			#gobject.idle_add(self.frame.AppendToLogWindow, message)
			if self.socket is not None:
				self.close()
			self.timer.cancel()
			
		except Exception,e:
			if DEBUG: self.Output("CheckNick ERROR", e)

			
	## Ping
	def cb_ping(self):
		if DEBUG: self.Output("RECEIVED: cb_ping")
		pass

	def cb_server_privileges(self, time_left):
		if DEBUG: self.Output("RECEIVED: cb_server_privileges: %i" % time_left)
		
	def cb_config_set(self, domain, key, value):
		
		try:
			if self.config.has_key(domain) and key in self.config[domain].keys():
				if DEBUG: self.Output("Modified <"+key+"> in <" +domain+"> to <"+value + ">")
				self.config[domain][key] = value
				if domain == "banned":
					self.frame.userlists.banned.update(key, value)
				elif domain == "buddies":
					self.frame.userlists.buddies.update(key, value)
				elif domain == "ignored":
					self.frame.userlists.ignored.update(key, value)
				elif domain == "trusted":
					self.frame.userlists.trusted.update(key, value)
			
				
					
			else:
				if domain not in self.config:
					if DEBUG: self.Output("Created domain <" +domain+">")
					self.config[domain] = {}
				if value == '':
					if DEBUG: self.Output("Added <"+key+"> to <" +domain+">")
				else:
					if DEBUG: self.Output("Added <"+key+"> to <" +domain+"> and set to <"+value+">")
				
				self.config[domain][key] = value
				if domain == "banned":
					self.frame.userlists.banned.append(key, value)
				elif domain in "buddies":
					self.frame.userlists.buddies.append(key, value)
				elif domain in "ignored":
					self.frame.userlists.ignored.append(key, value)
				elif domain in "trusted":
					self.frame.userlists.trusted.append(key, value)
			if domain in ("banned", "buddies", "ignored", "trusted"):
				# Update lists
				self.frame.ChatRooms.GetListStatus(key)
				self.frame.userlists.updateListIcon(key)
				if domain == "buddies":
					for widget in self.frame.BuddiesComboEntries:
						#gobject.idle_add(widget.Append, key)
						pass
					
			elif domain == "interests.hate":
				self.frame.Recommendations.AddedHatedInterest(key)
			elif domain == "interests.like":
				self.frame.Recommendations.AddedLikedInterest(key)
			elif domain == "autojoin":
				self.frame.ChatRooms.AutoJoin(key, True)
		except Exception,e:
			if DEBUG: self.Output("RECEIVED: cb_config_set ERROR", e)

		

	## Delete keys from self.config
	def cb_config_remove(self, domain, key):
		
		
		if key in self.config[domain].keys():
			if DEBUG: self.Output("Removed <"+key+"> from <" +domain+">")
			if domain == "banned":
				self.frame.userlists.banned.remove(key)
			elif domain in "buddies":
				self.frame.userlists.buddies.remove(key)
			elif domain in "ignored":
				self.frame.userlists.ignored.remove(key)
			elif domain in "trusted":
				self.frame.userlists.trusted.remove(key)
			elif domain == "interests.hate":
				self.frame.Recommendations.RemovedHatedInterest(key)
			elif domain == "interests.like":
				self.frame.Recommendations.RemovedLikedInterest(key)
			elif domain == "autojoin":
				self.frame.ChatRooms.AutoJoin(key, False)
			if domain in ("banned", "buddies", "ignored", "trusted"):
				#gobject.idle_add(self.frame.ChatRooms.GetListStatus, key)
				#gobject.idle_add(self.frame.userlists.updateListIcon, key)
				if domain == "buddies":
					for widget in self.frame.BuddiesComboEntries:
						#gobject.idle_add(widget.Remove, key)
						pass
			del self.config[domain][key]
		
		#self.display_config_update(domain)
	
	## Copy config to self.config at connection
	def cb_config_state(self, config):
		
		
		self.config = config.copy()
		if self.config.keys() != []:
			if self.config.has_key("buddies" ):
				#gobject.idle_add(self.frame.userlists.buddies.Fill, self.config["buddies"])

				#for widget in self.frame.BuddiesComboEntries:
					#gobject.idle_add(widget.Fill)
					#pass
				pass
			
			if self.config.has_key("banned"):

				#gobject.idle_add(self.frame.userlists.banned.Fill, self.config["banned"])
				
				for user in self.config["banned"].keys():
				
					if not self.frame.user_stats.has_key(user):
						self.PeerStats(user)
						self.PeerStatus(user)
						
			if self.config.has_key("ignored"):
				#gobject.idle_add(self.frame.userlists.ignored.Fill, self.config["ignored"])
				for user in self.config["ignored"].keys():
				
					if not self.frame.user_stats.has_key(user):
						self.PeerStats(user)
						self.PeerStatus(user)
					
	
			if self.config.has_key("trusted"):
				#gobject.idle_add(self.frame.userlists.trusted.Fill, self.config["trusted"])
				pass
			#if self.config.has_key("interests.like"):
				#for interest in self.config["interests.like"].keys():
					#self.frame.Recommendations.AddedLikedInterest(interest)
			#if self.config.has_key("interests.hate"):
				#for interest in self.config["interests.hate"].keys():
					#self.frame.Recommendations.AddedHatedInterest(interest)
		#self.frame.Muscan.GetConfig()
		
				
	## Add new/replace old keys to self.config
	def mod_config(self, changetype, username, value=''):
		#self.Output(changetype, username, value
		if changetype == "buddy":
			if not self.config.has_key("banned") or username not in self.config["buddies"].keys():
				self.ConfigSet("buddies", username, value)
				if not self.frame.user_stats.has_key(username):
					self.PeerExists(username)
		elif changetype == "unbuddy":
			if self.config.has_key("buddies") and username in self.config["buddies"].keys():
				self.ConfigRemove("buddies", username)
		
		elif changetype == "trust":
			if not self.config.has_key("trusted") or username not in self.config["trusted"].keys():
				self.ConfigSet("trusted", username, value)
				if not self.frame.user_stats.has_key(username):
					self.PeerExists(username)
		elif changetype == "distrust":
			if self.config.has_key("trusted") and username in self.config["trusted"].keys():
				self.ConfigRemove("trusted", username)
	
		elif changetype == "ban":
			if not self.config.has_key("banned") or username not in self.config["banned"].keys():
				self.ConfigSet("banned", username, value)
				if not self.frame.user_stats.has_key(username):
					self.PeerExists(username)
		elif changetype == "unban":
			if self.config.has_key("buddies") and username in self.config["buddies"].keys():
				self.ConfigRemove("banned", username)
		
		elif changetype == "ignore":
			if not self.config.has_key("ignored") or username not in self.config["ignored"].keys():
				self.ConfigSet("ignored", username, value)
	
				if not self.frame.user_stats.has_key(username):
					self.PeerExists(username)
		elif changetype == "unignore":
			if self.config.has_key("ignored") and username in self.config["ignored"].keys():
				self.ConfigRemove("ignored", username)
			
		elif changetype == "autojoin":
			room = username
			if not self.config.has_key("autojoin") or room not in self.config["autojoin"].keys():
				self.ConfigSet("autojoin", room, "")
		
		elif changetype == "unautojoin":
			room = username
			if self.config.has_key("autojoin") and room in self.config["autojoin"].keys():
				self.ConfigRemove("autojoin", room)
		
	def Send(self, message):
		if self.frame.Networking.connected == False:
			return
		try:
			self.send( message ) 
		except Exception, e:
			if DEBUG: self.Output("Sending message failed", message, e)
			
	## Got Global Recommendations
	# @param self Networking (Driver Class)
	# @param recommendations list of recommendations [item, number of recommends] 
	def cb_get_global_recommendations(self, recommendations):
		try:
		
			#gobject.idle_add(self.frame.Recommendations.UpdateRecommendations, recommendations)
			self.Output(recommendations)
		except Exception, e:
			self.Output("CB Get Global Recommendations" + str(e))
	
	## Got Similar Users list
	# @param self Networking (Driver Class)
	# @param users List of format [username, status=(0,1,2)]
	def cb_get_similar_users(self, users):
		try:
			
			
			#gobject.idle_add(self.frame.Recommendations.UpdateSimilarUsers, users)
			self.Output( users)
		except Exception, e:
			self.Output("CB Similar Users" + str(e))
			self.frame.PrintTraceBack()
	
	## Got Personal Recommendations
	# @param self Networking (Driver Class)
	# @param recommendations list of recommendations [item, number of recommends]
	def cb_get_recommendations(self, recommendations):
		try:
			
			#gobject.idle_add(self.frame.Recommendations.UpdateRecommendations, recommendations)
			self.Output(recommendations)
		except Exception, e:
			self.Output("CB Get  Recommendations" + str(e))
	
	## Got Similar Users list for an Item
	# @param self Networking (Driver Class)
	# @param item string
	# @param users List of format [username, status=(0,1,2)]
	def cb_get_item_similar_users(self, item, users):
		try:
			
			#gobject.idle_add(self.frame.Recommendations.UpdateSimilarUsers, users)
			self.Output( users)
		except Exception, e:
			self.Output("CB Item Similar Users" + str(e))
	
	## Got Recommendations for an Item
	# @param self Networking (Driver Class)
	# @param item string
	# @param recommendations list of recommendations [item, number of recommends]
	def cb_get_item_recommendations(self, item, recommendations):
		try:
			#gobject.idle_add(self.frame.Recommendations.UpdateRecommendations, recommendations)
			self.Output(recommendations)
		except Exception, e:
			self.Output("CB Get Item Recommendations" + str(e))

        def cb_room_list(self, roomlist):
		#

		if DEBUG: self.Output("RECEIVED: cb_room_list", len(roomlist))
	
		#gobject.idle_add(self.frame.ChatRooms.UpdateRoomList, roomlist)
		#

	def cb_room_tickers(self, room, tickers):
		if DEBUG: self.Output("RECEIVED: cb_room_ticker")
		
		

	def cb_room_ticker_set(self, room, user, message):
		
		if DEBUG: self.Output("RECEIVED: cb_room_ticker_set")
		

	def cb_search_ticket(self, query, ticket):
		
		
		if DEBUG: self.Output("RECEIVED: cb_search_ticket", query, ticket)
		#gobject.idle_add(self.frame.Searches.NewSearch, query, ticket)
		
		


	def cb_search_results(self, ticket, user, free, speed, queue, results):
		
		if DEBUG: self.Output("RECEIVED: cb_search_results")

		#gobject.idle_add(self.frame.Searches.NewResults,ticket, user, free, speed, queue, results)
		



	def cb_transfer_state(self, downloads, uploads):
		#
		if DEBUG: self.Output("RECEIVED: cb_transfer_state")
		try:
			pass
			#for transfer in uploads:
				#gobject.idle_add(self.frame.uploads.append, transfer.user, None, transfer.rate, transfer.state, transfer.filepos, transfer.filesize, transfer.path, transfer.error, transfer.place)
			#for transfer in downloads:
				#gobject.idle_add(self.frame.downloads.append, transfer.user, None, transfer.rate, transfer.state, transfer.filepos, transfer.filesize, transfer.path, transfer.error, transfer.place)
		except Exception, e:
			self.Output(e)
		#


	def cb_transfer_update(self, transfer):
		#
		try:
			
			if DEBUG: self.Output("RECEIVED: cb_transfer_update", transfer.user,  transfer.path)
			pass
			#if transfer.is_upload:
				#gobject.idle_add(self.frame.uploads.update , transfer.user, None, transfer.rate, transfer.state, transfer.filepos, transfer.filesize, transfer.path, transfer.error, transfer.place)
			#else:
				#gobject.idle_add(self.frame.downloads.update, transfer.user, None, transfer.rate, transfer.state, transfer.filepos, transfer.filesize, transfer.path, transfer.error, transfer.place)
		except Exception,e:
			if DEBUG: self.Output("RECEIVED: cb_transfer_update ERROR", e)
		#
			
	def cb_transfer_remove(self, transfer):
		
		
		if DEBUG: self.Output("RECEIVED: cb_transfer_remove")
		#user_path = transfer[1], transfer[2]
		if transfer[0]:
			pass
			#gobject.idle_add(self.frame.uploads.remove, transfer)
		else:
			pass
			#gobject.idle_add(self.frame.downloads.remove, transfer)
		
		
	def cb_user_shares(self, user, shares):
		
		if DEBUG: self.Output("RECEIVED: cb_user_shares")
		#self.frame.get_shares(user, shares)
		try:
			pass
			#gobject.idle_add(self.frame.userbrowses.GotUserBrowse, user, shares)
		except Exception,e:
			if DEBUG: self.Output("RECEIVED: cb_user_shares ERROR", e)
		
			
	def cb_user_info(self, user, info, picture, uploads, queue, slotsfree):
		
		try:
			if DEBUG:
				self.Output("got %s's userinfo" % user)

			#gobject.idle_add(self.frame.userinfos.GotUserInfo, user, info, picture, uploads, queue, slotsfree)
	
		except Exception,e:
			if DEBUG: self.Output("RECEIVED: cb_user_info ERROR", e)
		
		
	def cb_room_said(self, room, user, text):
		
		try:
			self.emit(QtCore.SIGNAL("SayChatRoom(PyQt_PyObject,PyQt_PyObject, PyQt_PyObject)"), room, user, text)
		
			#gobject.idle_add(self.frame.ChatRooms.SayChatRoom, room, user, text)
			#gobject.idle_add(self.frame.Logging.ChatRoomLog, room, user, text)
		except Exception,e:
			if DEBUG: self.Output("RECEIVED: cb_room_said ERROR", e)
		
			
	def cb_room_state(self, roomlist, joined, tickers):
		if DEBUG: self.Output("Logged in, getting Rooms")
		try:
		
			#gobject.idle_add(self.frame.ChatRooms.UpdateRoomList, roomlist)
			joined_rooms = list(joined.keys())
			joined_rooms.sort(key=str.lower)
			for room in joined_rooms:
#				for users, stats in joined[room].items():
					#self.Output(stats)
#					self.frame.user_stats[users] = [stats[0], stats[1], stats[2], stats[3], stats[4]]
				#self.frame.ChatRooms.JoinRoom( str(room), joined[room])
				self.emit(QtCore.SIGNAL("JoinRoom(PyQt_PyObject, PyQt_PyObject)"), str(room), joined[room])
				
				#gobject.idle_add(self.frame.ChatRooms.JoinRoom, str(room), joined[room])
				pass
		except Exception, e:
			if DEBUG: self.Output("Room State bug", e)
			self.frame.PrintTraceBack()

		
		
	def cb_room_joined(self, room, list_of_users, private, owner, operators):
		
		if DEBUG:
			self.Output("Joined room: %s" % room)
		try:
			
			self.emit(QtCore.SIGNAL("JoinRoom(PyQt_PyObject, PyQt_PyObject)"), str(room), list_of_users)
			#gobject.idle_add(self.frame.ChatRooms.JoinRoom, room, list_of_users)
			pass
				
		except Exception, e:
			if DEBUG: self.Output("Join room bug", e)
		
		
	def cb_room_left(self, room):
		if DEBUG: self.Output("RECEIVED: cb_room_left %s"% room)
		try:
			#gobject.idle_add(self.frame.ChatRooms.LeaveRoom, room)
			self.emit(QtCore.SIGNAL("LeaveRoom(PyQt_PyObject)"), room)
		except Exception, e:
			if DEBUG: self.Output("Leave room bug", e)
		
			
	def cb_room_user_joined(self, room, user, stats):
		#if DEBUG: self.Output("User Joined room",user)
		
		try:

			#self.frame.rooms[room][user] = stats
			self.frame.user_stats[user] = [stats[0], stats[1],stats[2],stats[3],stats[4]]
			self.frame.user_exists[user] = stats[5]
			self.emit(QtCore.SIGNAL("UserJoinedRoom(PyQt_PyObject, PyQt_PyObject)"), room, user)
			
		except Exception, e:
			if DEBUG: self.Output("User Joined room bug", e)
		

	
	def cb_room_user_left(self, room, user):
		#if DEBUG: self.Output("User Left room",user)
		
		try:
			pass
			#gobject.idle_add(self.frame.ChatRooms.UserLeftRoom, room, user)
			self.emit(QtCore.SIGNAL("UserLeftRoom(PyQt_PyObject, PyQt_PyObject)"), room, user)

		except Exception, e:
			if DEBUG: self.Output("User Left room bug", e)
		

	def cb_private_message(self, direction, timestamp, user, message):
		
		#if DEBUG: self.Output("RECEIVED: cb_private_message", user)
		#
		#if DEBUG: self.Output(direction, timestamp, user, message)
		try:
			
			

			#gobject.idle_add(self.frame.PrivateChats.ShowMessage, direction, user, message)
			self.emit(QtCore.SIGNAL("ShowMessage(PyQt_PyObject,PyQt_PyObject, PyQt_PyObject)"), direction, user, message)
			##gobject.idle_add(self.frame.Logging.PrivateChatLog, direction, user, message)
			
			if DEBUG:
				if direction:
					user = self.frame.username
					self.Output("You PM'd to %s: %s" % (user, message))
				else:
					self.Output("%s PM'd to you: %s" % (user, message))
		except Exception,e:
			if DEBUG: self.Output("RECEIVED: cb_private_message ERROR", e)
		#
			
	def cb_peer_address(self, user, ip, port):
		
		if DEBUG: self.Output("RECEIVED: cb_peer_address")
		#
		try:
			if user in self.frame.ip_requested:
				self.frame.ip_requested.remove(user)
				if geoip_fail==0:
					country =  gi.country_name_by_addr( str(ip) )
					message="%s's IP: %s Port: %s Country: %s"  % (user, str(ip), str(port), country)
					#s= self.frame.display_box(title="IP Address", message = message  )
					
					if DEBUG: self.Output(message)
				else:
					if DEBUG: self.Output("No Geoip")
					message="%s's IP: %s Port: %s" % (user, str(ip), str(port) )
				#gobject.idle_add(self.frame.AppendToLogWindow, message)
		except Exception,e:
			if DEBUG: self.Output("RECEIVED: cb_peer_address ERROR", e)
		#
			

		
	def cb_server_state(self, state, username):
		
		try:
			
			if state != self.frame.status:
				if DEBUG:
					self.Output( str(state), username)
			
				self.frame.status = state
				if self.frame.status:
					message = "Connected to Museek: %s Status: Away" % username
					
					#gobject.idle_add(self.frame.TrayApp.SetToolTip, message)
					#gobject.idle_add(self.frame.update_statusbar, message)
					#gobject.idle_add(self.frame.TrayApp.SetImage, "yellow")
					
				elif self.frame.status  == 2:
					message = "Connected to Museek: %s Status: Online" % username
					
					#gobject.idle_add(self.frame.TrayApp.SetToolTip, message )
					#gobject.idle_add(self.frame.update_statusbar, message )
					#gobject.idle_add(self.frame.TrayApp.SetImage, "green")
					
				else:
					#gobject.idle_add(self.frame.TrayApp.SetImage, "red")
					pass

			self.frame.username = username
					
		except Exception,e:
			if DEBUG: self.Output("RECEIVED: cb_server_state ERROR", e)
		
		
	def cb_server_status_set(self, status):
		
		#if DEBUG: self.Output("RECEIVED: cb_server_status_set ", status)
		try:
			
			if status != self.frame.status:
				self.frame.status = status
				if self.frame.status:
					message = "Connected to Museek: %s Status: Away" % self.frame.username
					
					#gobject.idle_add(self.frame.TrayApp.SetImage,"yellow")
				else:
					message = "Connected to Museek: %s Status: Online" % self.frame.username
					
					#gobject.idle_add(self.frame.TrayApp.SetImage, "green")
				#gobject.idle_add(self.frame.TrayApp.SetToolTip, message ) 
				#gobject.idle_add(self.frame.update_statusbar, message )
			
		except Exception,e:
			if DEBUG: self.Output("RECEIVED: cb_server_status_set ERROR", e)
		
                
	def cb_peer_exists(self, user, exists):
		#
		#if DEBUG: self.Output("%s exists? %i" % (user, exists))
		try:
			
			self.frame.user_exists[user] = exists
			if self.frame.user_stats.has_key(user):
				status = self.frame.user_stats[user][0]
				#gobject.idle_add(self.frame.ChatRooms.GetUserStatus, user, status)
				#gobject.idle_add(self.frame.userlists.updateListIcon, user)
				#gobject.idle_add(self.frame.PrivateChats.updateStatus, user, status)
				
		except Exception,e:
			if DEBUG: self.Output("RECEIVED: cb_peer_exists ERROR", e)
		#
                
	def cb_peer_stats(self, user, speed, downloads, files, dirs, slotsfull, country):
		#if DEBUG: self.Output("RECEIVED: cb_peer_stats", user, speed, downloads,) files, dirs

		try:
			if user in self.frame.user_stats:
				stats = self.frame.user_stats[user]
				#if stats[1] == speed and stats[2] == downloads and stats[3] == files and stats[4] == dirs:
					#self.Output("same")
					#return
				self.frame.user_stats[user] = [stats[0], speed, downloads, files, dirs]
				
			else:
				self.frame.user_stats[user] = [0, speed, downloads, files, dirs]
		except Exception,e:
			if DEBUG: self.Output("RECEIVED: cb_peer_stats ERROR", e)
		#
		
		#gobject.idle_add(self.frame.ChatRooms.GetUserStats, user, speed, files)
		#gobject.idle_add(self.frame.userlists.updateStats, user, self.frame.user_stats[user])

		#
			
	def cb_peer_status(self, user, status):
		#if DEBUG: self.Output("RECEIVED: cb_peer_status", user, status)
		try:
			if user in self.frame.user_stats:

				self.frame.user_stats[user][0] = status
				
	
			else:
				self.frame.user_stats[user] = [status, 0, 0, 0, 0]
			
			## Update Lists with new status information
			self.emit(QtCore.SIGNAL("UserStatus(PyQt_PyObject, PyQt_PyObject)"), user, status)
			#gobject.idle_add(self.frame.ChatRooms.GetUserStatus, user, status)
			#gobject.idle_add(self.frame.userlists.updateStatus, user, status)
			#gobject.idle_add(self.frame.PrivateChats.updateStatus, user, status)
			
		except Exception,e:
			if DEBUG: self.Output("RECEIVED: cb_peer_status ERROR1", e)
			
	## Museekd sent us a special message
	#
	# @param self is the Networking class
	# @param type is a bool; False if the message relates to the Server / True for Peer
	# @param message is the message string
	def cb_status_message(self, type, message):
		#
		try:
			if type == 1:
				stype = "Peer"
			elif type == 0:
				stype = "Server"
			
			#gobject.idle_add(self.frame.AppendToLogWindow, "%s Message: %s" % (stype, message))
			
		except Exception,e:
			self.Output("cb_status_message: " +str( e))
		#
	## Museekd sent us a debug message
	# @param self Networking (Driver Class)
	# @param domain is a string the value of which is a debug type
	# @param message is the message string
	def cb_debug_message(self, domain, message):

		
		try:
			if domain in ["museek.note", "museek.warn"] :
				self.frame.AppendToLogWindow( "%s Message: %s" % (domain, message))
				
		except Exception,e:
			self.Output("cb_debug_message: " +str( e))
		


	## Process Socket Data
	def run(self):
		if DEBUG:  self.Output(self.frame.pid)
		self.connected = True
		while 1:
		#if 1:
			try:
				d = 0
				#while self.frame.Config["connection"]["passw"]  == None or self.frame.Config["connection"]["passw"] == "None":
					#sleep(0.1)
				if self.socket != None:

					r, w, x = select.select([self.socket,], [], [self.socket], d)
					
					if self.socket in r:
						#try:
							#self.Output(r[0])
						#except:
							#pass
						driver.Driver.process(self)
				else:
					
					if self.connected == True:
						self.connect_to_museekd("")
					
				time.sleep(0.001)
				
				#self.frame.MurmurWindow.show_all() 

			except Exception, exception:
				if DEBUG: self.Output("Process Exception", Exception, exception)
				import traceback
				tb = traceback.format_tb(sys.exc_info()[2])
				self.Output(tb)
			except e:
				if DEBUG: self.Output(e)
		
	## Failsafe Sending of messages to Museekd
	# @param self Networking (class)
	# @param message messages instance
	def SendMessage(self, message):
		print self.frame.Networking.connected, message
		if self.frame.Networking.connected is False:
			return
		try:
			self.send(message)
		except Exception, e:
			if  e.args[0] == 10054:
				self.cb_disconnected()
			self.Output("SendMessage: " + str(e) + " " + str(message))
			
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
	# @param room A Room you are in
	# @param message text
	def SayRoom(self, room, message):
		message = messages.SayRoom(room, message )
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
	def Search(self, searchtype, query, user=None):

		if searchtype in (0, 1, 2):
			message = messages.Search(searchtype, query )
			self.SendMessage(message)
		elif searchtype == 3 and user is not None:
			self.UserSearch(user, query)
			
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


# This is part of the Mucous Museek Client, and distributed under the GPLv2
# Copyright (c) 2006 daelstorm. 

import curses.wrapper
## UserInfo text and user statistics
# Tabbed buttons display along the top
# Left window contains userinfo description
# Right window contains statistics
class UserInfo:
	## Constructor
	# @param self UserInfo (class)
	# @param mucous Mucous (class)
	def __init__(self, mucous):
		## @var mucous 
		# Mucous (Class)
		self.mucous = mucous
		## @var requests 
		# Users whose info we've requested (Discard other requests)
		self.requests = []
		## @var current
		# Currently shown user's info
		self.current = None
		## @var users 
		# Users whose info we have
		self.users = []
		## @var logs
		# dict containing descriptions and stats of users
		self.logs = {}
		## @var windows
		# dict containing instances of curses windows
		self.windows = {}
		## @var dimensions
		# dict containing placement data for windows
		self.dimensions = {}
		## @var scrolling
		# dict of users contain vertical position in their user info
		self.scrolling =  {}
		## @var scrolling_status
		# int of scroll position in instructions 
		self.scrolling_status = 0  
		
	## Request userinfo from a user
	# @param self UserInfo (class)
	# @param user the Username
	def Get(self, user):
		if user not in self.requests:
			self.requests.append(user)
		self.mucous.D.UserInfo(user)
		
	## Recieved userinfo
	# If a picture is included, save it to disk as username.image
	# @param self UserInfo (class)
	# @param user the Username
	# @param info the description
	# @param picture the image, if it exists
	# @param uploads number of uploads slots
	# @param queue Length of queue
	# @param slotsfree has free slots?
	def Recieved(self, user, info, picture, uploads, queue, slotsfree):
		if user not in self.requests:
			return
		self.requests.remove(user)
		self.scrolling[user] = 0
		message = info.replace('\r', "").split('\n')
		pic = False
		if picture != '':
			import imghdr
			format = imghdr.what(None, picture)
			r = file("%s.%s" % (self.mucous.config_dir+str(user), format), 'wb')
			r.write(picture)
			r.close()
			self.StatsLog( "Saved UserImage as: %s.%s" % (str(user), format))
			pic = True
		self.Log(user, message, uploads, queue, slotsfree, pic)
		
		if self.mucous.mode != "info":
			#self.mucous.Alerts.setStatus("New UserInfo")
			#self.mucous.Alerts.alert["INFO"].append(user)
			self.mucous.Alerts.Add(user, "INFO")
			self.mucous.HotKeyBar()
			
	## Draw windows
	# @param self UserInfo (class)
	def Mode(self):
		try:
			self.mucous.mode = "info"
			self.mucous.UseAnotherEntryBox()
			self.mucous.PopupMenu.show = False
			# Cleanup stale windows
			if "text" in self.windows:
				del self.windows["text"]
			if "border" in self.windows: 
				del self.windows["border"]
			if "infostats" in self.windows:	
				del self.windows["infostats"]
			if "statsborder" in self.windows:
				del self.windows["statsborder"]
			
			w = self.dimensions["info"] = {"height": self.mucous.h-10, "width": self.mucous.w-20, "top": 5, "left": 1}
			mw = self.windows["border"] = curses.newwin(w["height"]+2, w["width"]+2, w["top"]-1, w["left"]-1)
			mw.attron(self.mucous.colors["green"])
			mw.border()
			mw.attroff(self.mucous.colors["green"])
			try:
				mw.addstr(0, 3, "< Info Mode >",  self.mucous.colors["green"] | curses.A_BOLD)
			except:
				pass
	
			tw =  self.windows["info"] = mw.subwin(w["height"], w["width"], w["top"], w["left"])
			tw.scrollok(0)
			tw.idlok(1)
			
			#self.scrolling = -1
	
			self.mucous.SetEditTitle("Get info about user:")
			
			sw = self.dimensions["infostats"]= {"height": self.mucous.h-10, "width": 16, "top": 5, "left": self.mucous.w-17}
			isw = self.windows["statsborder"] = curses.newwin(sw["height"]+2, sw["width"]+2, sw["top"]-1, sw["left"]-1)
			isw.border()
			isw.addstr(0, 2, "< Stats >")
			isw.noutrefresh()
			itw = self.windows["infostats"] = isw.subwin(sw["height"], sw["width"], sw["top"], sw["left"])
			itw.scrollok(1)
	
			mw.noutrefresh()
			
			self.DrawText()
			itw.noutrefresh()
			# queue, uploads, speed, downloads, files, directories, freeslots

			self.mucous.DrawTabs(self.users, self.current)
			self.DrawStats()
			self.mucous.Alerts.Check()
			curses.doupdate()
			#self.HotKeyBar()
		except Exception, e:
			self.mucous.Help.Log("debug", "UserInfo.Mode: " + str(e))
			
	## Select another user
	# @param self UserInfo (class)
	# @param direction left/right list scrolling
	def Select(self, direction):
		try:
			if self.current == None:
				self.current = self.users[0]
				self.Mode()
				return
			place = self.mucous.FormatData.RotateList(direction, self.users, self.current, "yes" )
			if self.current != place:
				self.current = place
				self.Mode()
		except Exception, e:
			self.mucous.Help.Log("debug", "UserInfo.Select: " + str(e))
	
	## Draw text in text window
	# @param self UserInfo (class)
	def DrawText(self):
		try:
			scrolltext = "info"
			w = self.dimensions["info"] 
			lang = self.mucous.Config["mucous"]["language"]
			tw = self.windows["info"]
			if self.current != None:
				self.mucous.DrawInstructionsButtons()
				# Display UserInfo & Stats
				clipped_list, self.scrolling[self.current], w["start"] = self.mucous.FormatData.wrap_n_clip(self.logs[self.current][0], self.scrolling[self.current], w)
				
			else:
				# Display instructions, IP info, and stats
				clipped_list, self.scrolling_status, w["start"] = self.mucous.FormatData.wrap_n_clip( self.mucous.Help.log["userinfo"], self.scrolling_status, w)
				
	
			attrs = curses.A_BOLD; attr = curses.A_NORMAL
			count = 0
			tw.erase()
			if self.current == None:
				scroll = self.scrolling_status
			else:
				scroll = self.scrolling[self.current]
			
			for lines in clipped_list:
				try:
					lines, ls = self.mucous.FormatData.StringAddBlanks(lines, w)
					if count + w["start"] == scroll:
						tw.addstr(self.mucous.dlang(lines), attrs)
					else:
						tw.addstr(self.mucous.dlang(lines), attr)
					count += 1
				except Exception, e:
					pass
			tw.noutrefresh()
		except Exception, e:
			self.mucous.Help.Log("debug", "UserInfo.DrawText: " + str(e))

					
	## Draw statistics in info stats windo
	# @param self UserInfo (class)
	def DrawStats(self):
		try:
			itw = self.windows["infostats"]
			itw.erase()
			if self.current != None and self.mucous.mode=="info":
				userinfo = self.logs[self.current]
				if int(userinfo[1][2]):
					slots = "Yes"
				else:
					slots = "No"
				if userinfo[1][3]:
					image = "Yes"
				else:
					image = "No"
				itw.addstr('Slots: %s' % str(userinfo[1][0]) + \
                                          '\nQueue: %s' % str(userinfo[1][1]) + \
                                          '\nFree: %s' % slots +\
					  '\nImage: %s' % image)

				if self.current in self.mucous.user["statistics"].keys():
					try:
                                                stats = self.mucous.user["statistics"][self.current]
						itw.addstr('\nSpeed: %.2fKB' % (stats[0]/1024.0))
						itw.addstr('\nDown: %s' % str(stats[1]))
						itw.addstr('\nFiles: %s' % str(stats[2]))
						itw.addstr('\nDirs: %s' % str(stats[3]))
					except:
						pass
			itw.noutrefresh()
		except Exception, e:
			self.mucous.Help.Log("debug", "UserInfo.DrawStats: " + str(e))
			
	## Mouse Coordinates in the User Info Mode
	# @param self is UserInfo (class)
	# @param x is the horizontal position from the left
	# @param y is the vertical postion from the top
	# @param z is unimportant
	# @param event is the mouse event (button, doubleclick, etc) represented by a number
	def Mouse(self, x,y,z,event):
		try:
			if y in (2, 3, 4):
				if len(self.users) >= 1:
					
					if self.current == None:
						self.current = self.users[0]
					self.current, match = self.mucous.edit.MouseClickTab(x, self.current)
					if match == None:
						s = self.users.index(self.current)
						self.current = self.users[s-1]
					#self.DrawStats()
					self.Mode()
					
			if self.current != None:
				if y  in (5,6):
					if x >=self.mucous.w-19-2-16 and x < self.mucous.w-12:
						self.current=None
						self.Mode()
				elif y in ( self.mucous.h-3, self.mucous.h-4, self.mucous.h-5):
					if x >=self.mucous.w-10 and x < self.mucous.w-1:
						self.Close(self.current)
		except Exception, e:
			self.mucous.Help.Log("debug", "MouseUserInfo: " +str(e) )
			
	## Close userinfo
	# @param self UserInfo (class)
	# @param user the Username
	def Close(self, user):
		try:
			if user in self.users:
				self.users.remove(user)
			if self.users != []:
				for users in self.users:
					self.current = users
					break
			else:
				self.current = None
			if user in self.mucous.Alerts.alert["INFO"]:
				self.mucous.Alerts.alert["INFO"].remove(user)
			if self.mucous.mode == 'info':
				self.Mode()
		except Exception, e:
			self.mucous.Help.Log("debug", "UserInfo.Close: " + str(e))
	
	## Append stats to text log
	# @param self UserInfo (class)
	# @param s string
	def StatsLog(self, s):
		try:
			s= self.mucous.dlang(s)
			if "\n" in s:
				lis = s.split("\n")
				for line in lis:
					self.mucous.Help.log["userinfo"].append("%s" % line)
			else:
				self.mucous.Help.log["userinfo"].append("%s" % s)
			if self.mucous.mode == "info" and self.current == None:
				self.DrawText()
		except Exception, e:
			self.mucous.Help.Log("debug", "StatsLog: " + str(e))
			
	## Store UserInfo, stats in Log
	# @param self UserInfo (class)
	# @param user username
	# @param description user's info/description
	# @param user username
	# @param uploads number of uploads slots
	# @param queue Length of queue
	# @param slotsfree has free slots (True/False)
	# @param pic has a picture (True/False)
	def Log(self, user, description, uploads, queue, slotsfree, pic):
		try:
			if self.current == None:
				self.current = user
			if user not in self.logs:
				self.logs[user] = []
			if user not in self.users:
				self.users.append(user)
			self.logs[user] = description, [uploads, queue, slotsfree, pic]
			if user not in self.mucous.user["statistics"].keys():
				self.mucous.D.PeerStats(user)
			if self.mucous.mode == 'info':
				self.Mode()
		except Exception, e:
			self.mucous.Help.Log("debug", "UserInfo.Log: " + str(e))
		

# This is part of the Mucous Museek Client, and distributed under the GPLv2
# Copyright (c) 2006 daelstorm. 
import curses.wrapper			
## Users Lists
#			
class UsersLists:
	## Constructor
	# @param self UsersLists (class)
	# @param mucous Mucous (class)
	def __init__(self, mucous):
		## @var mucous 
		# Mucous (Class)
		self.mucous = mucous
		## @var logs
		# holds temporary data, until the next Mucous.config update
		self.logs = {"buddied": [], "banned": [], "ignored": [], "trusted": []}
		## @var scrolling
		# dict containing scroll position for buddied, banned, ignored ad trusted lists 
		self.scrolling = {"buddied": 0, "banned": 0, "ignored": 0, "trusted": 0}
		## @var current
		# default list is buddied
		self.current = "buddied"
		## @var windows
		# dict containing curses window instances
		self.windows = {"text":{}, "border":{} }
		## @var dimensions
		# dict containing placement data for windows
		self.dimensions = {}
		
	## Pick List to display
	# @param self UsersLists (class)	
	def ModeLists(self):
		self.mucous.mode = "lists"
		self.mucous.PopupMenu.show = False
		if self.current == "buddied":
			self.ModeBuddy()
		elif self.current == "banned":
			self.ModeBan()
		elif self.current == "ignored":
			self.ModeIgnore()
		elif self.current == "trusted":
			self.ModeTrust()	
		elif self.current == "interests":
			self.mucous.Recommendations.ModeInterests()
				
	## Display Trusted users list
	# @param self UsersLists (class)	
	def ModeTrust(self):
		try:
			self.mucous.UseAnotherEntryBox()
			self.current = "trusted"
			#self.DestroyOldWindows()
			self.mucous.PopupMenu.show = False
			
			s = self.dimensions[self.current] = {"height": self.mucous.h-7, "top": 2, "left": 1, "width": self.mucous.w-2, "start": 0}
			self.ListTrust()
			self.DrawListsWindows()
			
			self.FormatLists(s, "trusted")
			
			self.mucous.HotKeyBar()
			curses.doupdate()
		except Exception, e:
			self.mucous.Help.Log("debug", "ModeTrust: " + str(e))
			
	## Display Buddied users list
	# @param self UsersLists (class)
	def ModeBuddy(self):
		try:
			self.mucous.UseAnotherEntryBox()
			self.current = "buddied"
			self.mucous.PopupMenu.show = False
			
			s = self.dimensions[self.current] = {"height": self.mucous.h-7, "top": 2, "left": 1, "width": self.mucous.w-2, "start": 0}
			self.ListBuddy()
			self.DrawListsWindows()
			
			self.FormatLists(s, "buddied")
			
			self.mucous.HotKeyBar()
			curses.doupdate()
		except Exception, e:
			self.mucous.Help.Log("debug", "ModeBuddy: " + str(e))
	
	## Display Banned users list
	# @param self UsersLists (class)
	def ModeBan(self):
		try:
			self.mucous.UseAnotherEntryBox()
			self.current = "banned"
			self.mucous.PopupMenu.show = False
			
			s = self.dimensions[self.current] = {"height": self.mucous.h-7, "top": 2, "left": 1, "width": self.mucous.w-2, "start": 0}
			self.DrawListsWindows()
			self.ListBan()
			self.FormatLists(s, "banned")
			self.mucous.HotKeyBar()
			curses.doupdate()
		except Exception, e:
			self.mucous.Help.Log("debug", "ban_mode: " + str(e))
	
	## Display Ignored users list
	# @param self UsersLists (class)
	def ModeIgnore(self):
		try:
			self.mucous.UseAnotherEntryBox()
			self.current = "ignored"
			self.mucous.PopupMenu.show = False
			
			s = self.dimensions[self.current] = {"height": self.mucous.h-7, "top": 2, "left": 1, "width": self.mucous.w-2, "start": 0}
			self.DrawListsWindows()
			self.ListIgnore()
			self.FormatLists(s, "ignored")
			self.mucous.HotKeyBar()
			curses.doupdate()
		except Exception, e:
			self.mucous.Help.Log("debug", "ModeIgnore: " + str(e))
	
	## Format lists to fit inside window dimensions
	# @param self UsersLists (class)
	# @param window Window dimensions dict
	# @param mode list name
	def FormatLists(self, window, mode):
		try:
			if self.logs[mode] != None and self.logs[mode] != []:
				clipped_list, self.scrolling[mode], self.dimensions[self.current]["start"] = self.mucous.FormatData.scrollbox(self.logs[mode], self.scrolling[mode], window["height"])
				count = 0 
				try:
					self.windows["border"][self.current].addstr(self.mucous.h-6, self.mucous.w-18, "< "+str(len(self.logs[mode]))+" >", self.mucous.colors["green"] | curses.A_BOLD)
					self.windows["border"][self.current].noutrefresh()	
				except:
					pass
				self.windows["text"][self.current].erase()
				for lines in clipped_list:
					self.DrawLists(lines, count, self.current)
					count += 1
				
				self.windows["text"][self.current].noutrefresh()
		except Exception, e:
			self.mucous.Help.Log("debug", "FormatLists: " + str(e))
			
	## Draw List Window borders
	# @param self UsersLists (class)
	def DrawListsWindows(self):
		try:
			# Cleanup stale windows
			if self.current in self.windows["text"]: 
				del self.windows["text"][self.current]
			if self.current in self.windows["border"]:
				del self.windows["border"][self.current]
				
			s = self.dimensions[self.current]
			mw = self.windows["border"][self.current] = curses.newwin(s["height"]+2, s["width"]+2, s["top"]-1, s["left"]-1)
			mw.attron(self.mucous.colors["green"])
			mw.border()
			mw.attroff(self.mucous.colors["green"])

			if self.current =="buddied":
				mw.addstr(0, 3, "< Buddied >",  self.mucous.colors["green"] | curses.A_BOLD)
				mw.addstr(0, 16, "< Banned >", self.mucous.colors["green"])
				mw.addstr(0, 28, "< Ignored >", self.mucous.colors["green"])
				mw.addstr(0, 40, "< Trusted >", self.mucous.colors["green"])
				mw.addstr(0, 52, "< Interests >", self.mucous.colors["green"])
				
				self.mucous.SetEditTitle("Add Buddy:")
			elif self.current =="banned":
				mw.addstr(0, 3, "< Buddied >",  self.mucous.colors["green"] )
				mw.addstr(0, 16, "< Banned >", self.mucous.colors["green"]| curses.A_BOLD)
				mw.addstr(0, 28, "< Ignored >", self.mucous.colors["green"])
				mw.addstr(0, 40, "< Trusted >", self.mucous.colors["green"])
				mw.addstr(0, 52, "< Interests >", self.mucous.colors["green"])
				self.mucous.SetEditTitle("Ban User:")
			elif self.current =="ignored":
				mw.addstr(0, 3, "< Buddied >",  self.mucous.colors["green"] )
				mw.addstr(0, 16, "< Banned >", self.mucous.colors["green"])
				mw.addstr(0, 28, "< Ignored >", self.mucous.colors["green"]| curses.A_BOLD)
				mw.addstr(0, 40, "< Trusted >", self.mucous.colors["green"])
				mw.addstr(0, 52, "< Interests >", self.mucous.colors["green"])
				self.mucous.SetEditTitle("Ignore User:")
			elif self.current =="trusted":
				mw.addstr(0, 3, "< Buddied >",  self.mucous.colors["green"] )
				mw.addstr(0, 16, "< Banned >", self.mucous.colors["green"])
				mw.addstr(0, 28, "< Ignored >", self.mucous.colors["green"])
				mw.addstr(0, 40, "< Trusted >", self.mucous.colors["green"] | curses.A_BOLD)
				mw.addstr(0, 52, "< Interests >", self.mucous.colors["green"])
				self.mucous.SetEditTitle("Add Trusted:")
		
			mw.noutrefresh()
			tw = self.windows["text"][self.current] = mw.subwin(s["height"], s["width"], s["top"], s["left"])
			tw.scrollok(0)
			tw.idlok(1)
		except Exception, e:
			self.mucous.Help.Log("debug", "DrawLists: " + str(e))
	
	## Draw List Contents
	# @param self UsersLists (class)
	# @param line list of [attributes, username, note]
	# @param count number in list (Add to window["start"] to get the current line)
	# @param window window name (buddied, banned, ignored, trusted)
	def DrawLists(self, line, count, window):
		try:
			start = self.dimensions[window]["start"]
			tw = self.windows["text"][self.current]
			# attributes can contain ['trusted', 'banned', 'ignored', and 'buddied']
			attributes, username, note = line
		
			tabbeduser = self.mucous.dlang(username[:20])

			while len(tabbeduser) < 24:
				tabbeduser += ' '
			try:
				if username in self.mucous.user["status"].keys():
					if self.mucous.user["status"][username] == 1:
						tw.addstr('* ', self.mucous.colors["yellow"]|curses.A_BOLD)
					elif self.mucous.user["status"][username] == 2:
						tw.addstr('* ', self.mucous.colors["green"]|curses.A_BOLD)
					elif self.mucous.user["status"][username] == 0:
						tw.addstr('* ', self.mucous.colors["red"]|curses.A_BOLD)
				else: 
					tw.addstr('* ',  curses.A_BOLD )	
				pos = 2
				
				if self.current == "buddied":
					attrcount = 0
					try:
						if 'trusted' in attributes:
							color = self.mucous.colors["cyan"] | curses.A_BOLD 
							tw.addstr('^', color)
						else: tw.addstr(' ')
						color = self.mucous.colors["green"] | curses.A_BOLD 
						tw.addstr('^', color)
						if 'banned' in attributes:
							color = self.mucous.colors["red"] | curses.A_BOLD 
							tw.addstr('v', color)
						else: tw.addstr(' ')
						
						if 'ignored' in attributes:
							color = self.mucous.colors["yellow"] | curses.A_BOLD 
							tw.addstr('v ', color)
						else: tw.addstr('  ')
							
					except Exception, e:
						self.mucous.Help.Log("debug", "display list text" + str(e))
						pass
					
					
				elif self.current == "banned":
					
					try:
						if 'trusted' in attributes:
							color = self.mucous.colors["cyan"] | curses.A_BOLD 
							tw.addstr('^', color)
						else: tw.addstr(' ')
						
						if 'buddies' in attributes:
							color = self.mucous.colors["green"] | curses.A_BOLD 
							tw.addstr('^', color)
						else: tw.addstr(' ')
						color = self.mucous.colors["red"] | curses.A_BOLD 
						tw.addstr('v', color)
						if 'ignored' in attributes:
							color = self.mucous.colors["yellow"] | curses.A_BOLD 
							tw.addstr('v ', color)
						else: tw.addstr('  ')
						

					except:
						self.mucous.Help.Log("debug", "display list text" + str(e))
						
				elif self.current == "trusted":
					try:
						color = self.mucous.colors["cyan"] | curses.A_BOLD 
						tw.addstr('^', color)
						
						if 'buddies' in attributes:
							color = self.mucous.colors["green"] | curses.A_BOLD 
							tw.addstr('^', color)
						else: tw.addstr(' ')
						if 'banned' in attributes:
							color = self.mucous.colors["red"] | curses.A_BOLD 
							tw.addstr('v', color)
						else: tw.addstr(' ')
						if 'ignored' in attributes:
							color = self.mucous.colors["yellow"] | curses.A_BOLD 
							tw.addstr('v ', color)
						else: tw.addstr('  ')
					except Exception, e:
						self.mucous.Help.Log("debug", "display list text" + str(e))
						
				elif self.current == "ignored":
					try:
						if 'trusted' in attributes:
							color = self.mucous.colors["cyan"] | curses.A_BOLD 
							tw.addstr('^', color)
						else: tw.addstr(' ')
						if 'buddies' in attributes:
							color = self.mucous.colors["green"] | curses.A_BOLD 
							tw.addstr('^', color)
						else: tw.addstr(' ')
						if 'banned' in attributes:
							color = self.mucous.colors["red"] | curses.A_BOLD 
							tw.addstr('v', color)
						else: tw.addstr(' ')
						color = self.mucous.colors["yellow"] | curses.A_BOLD 
						tw.addstr('v ', color)
					except Exception, e:
						self.mucous.Help.Log("debug", "display list text" + str(e))
				#else:
					#tw.addstr(tabbeduser)
					#stats = note = ''
				color = curses.A_NORMAL
				pos +=5
				
				if count + start == self.scrolling[self.current]:
					attrib = curses.A_BOLD | curses.A_REVERSE | color
					attrib2 = curses.A_BOLD | curses.A_REVERSE 
				else:
					attrib = curses.A_BOLD | color
					attrib2 = curses.A_BOLD 
					
				tw.addstr(tabbeduser, attrib)
							
				if username in self.mucous.user["statistics"]:
					stats = " %sKB/s" % str(self.mucous.user["statistics"][username][0]/1024)
					while len(stats) < 9:
						stats += " "
					files = str(self.mucous.user["statistics"][username][2])
					while len(files) < 7:
						files = " " + files
					stats += files
					while len(stats) < 18:
						stats += " "
					tw.addstr( stats, attrib2)
				else:
					stats  = " 0KB/s         0  "
					tw.addstr(stats, attrib2)
					
				width = len(tabbeduser) + len(stats) + len(note) + 5
				subtract = self.mucous.w - width
				if subtract < 0:
					tw.addstr(note[:len(note)+subtract], attrib2)
				else:
					tw.addstr(note, attrib2)
					
					
				pos += len(tabbeduser) + len(stats) + len(note)
				if self.dimensions[window]["width"] - pos > 0:
					spaces = " " * (self.dimensions[window]["width"] - pos)
					tw.addstr(spaces, attrib2)
			except Exception, e:
				pass

		except Exception, e:
			self.mucous.Help.Log("debug", "DrawLists: " + str(e))

	## Mouse Coordinates in the Users Lists
	# @param self is mucous
	# @param x is the horizontal position from the left
	# @param y is the vertical postion from the top
	# @param z is unimportant
	# @param event is the mouse event (button, doubleclick, etc) represented by a number
	# @return 
	def Mouse(self, x,y,z,event):
		try:
			
			if self.current == "interests":
				w = self.mucous.Recommendations.dimensions[self.mucous.Recommendations.selected]
			else:
				w = self.dimensions[self.current]
			if y == 1:
				if x >= 4 and x <= 15:
					self.current="buddied"
					self.ModeLists()
				elif x >= 17 and x <= 27:
					self.current="banned"
					self.ModeLists()
				elif x >= 29 and x <= 40:
					self.current="ignored"
					self.ModeLists()
				elif x >= 42 and x <= 51:
					self.current="trusted"
					self.ModeLists()
				elif x >= 52 and x <= 64:
					self.current="interests"
					self.ModeLists()
				return
			if self.current == "interests":
				return self.mucous.Recommendations.MouseInterests(x,y,z,event)
			elif self.current in ("buddied", "banned", "ignored", "trusted"):
				# clicking on items in lists
				if y >= w["top"] and y < w["top"] + w["height"] and x >= w["left"] and x < w["left"] +w["width"]:
					y -= w["top"]
					
					this_list = self.logs[self.current]
					if y  + w["start"] in range(len(this_list)):
						self.scrolling[self.current]  = y  + w["start"]
						
						if event in ( 4096, 16384):
							self.SelectLists()
							self.mucous.PopupMenu.Create("lists", 0, True)
						else:
							self.SelectLists()
		except Exception, e:
			self.mucous.Help.Log("debug", "UsersLists.Mouse: " +str(e) )
	
	## ReDraw the current List
	# @param self UsersLists (class)
	def SelectLists(self):
		try:
			this_list = self.logs[self.current]
			if self.scrolling[self.current] > len(this_list):
				self.scrolling[self.current] = len(this_list)
			clipped_list, self.scrolling[self.current], self.dimensions[self.current]["start"] = self.mucous.FormatData.scrollbox(this_list, self.scrolling[self.current], self.mucous.h-7)
			count = 0
			self.windows["text"][self.current].erase()
			for lines in clipped_list:
				self.DrawLists(lines, count, self.current)
				count += 1
			self.windows["text"][self.current].refresh()
		except Exception, e:
			self.mucous.Help.Log("debug", "SelectLists: " + str(e))
			
	## Rebuild Buddied List from Mucous.config
	# @param self UsersLists (class)
	def ListBuddy(self):
		try:
			if not self.mucous.config.has_key("buddies"):
				return
			self.logs["buddied"] = []
			alpha_list = self.mucous.config["buddies"].keys()
			alpha_list.sort(key=str.lower)
			for user in alpha_list:
				note = self.mucous.config["buddies"][user]
				attributes = []
				if self.mucous.config.has_key("trusted") and self.mucous.config["trusted"].has_key(user):
					attributes.append("trusted")
				if self.mucous.config.has_key("ignored") and self.mucous.config["ignored"].has_key(user):
					attributes.append("ignored")
				if self.mucous.config.has_key("banned") and self.mucous.config["banned"].has_key(user):
					attributes.append("banned")
				self.logs["buddied"].append([attributes, user, note])
		except Exception, e:
			self.mucous.Help.Log("debug", "ListBuddy: " + str(e))
			
	## Rebuild Trusted List from Mucous.config
	# @param self UsersLists (class)
	def ListTrust(self):
		try:
			if not self.mucous.config.has_key("trusted"):
				return
			self.logs["trusted"] = []
			alpha_list = self.mucous.config["trusted"].keys()
			alpha_list.sort(key=str.lower)
			for user in alpha_list:
				note = self.mucous.config["trusted"][user]
				attributes = []
				if self.mucous.config.has_key("ignored") and self.mucous.config["ignored"].has_key(user):
					attributes.append("ignored")
				if self.mucous.config.has_key("banned") and self.mucous.config["banned"].has_key(user):
					attributes.append("banned")
				if self.mucous.config.has_key("buddies") and self.mucous.config["buddies"].has_key(user):
					attributes.append("buddies")
				self.logs["trusted"].append([attributes, user, note])
				
		except Exception, e:
			self.mucous.Help.Log("debug", "ListTrust: " + str(e))
			
	## Rebuild ListBan from Mucous.config
	# @param self UsersLists (class)
	def ListBan(self):
		try:
			if not self.mucous.config.has_key("banned"):
				return
			self.logs["banned"] = []
			alpha_list = self.mucous.config["banned"].keys()
			alpha_list.sort(key=str.lower)
			for user in alpha_list:
				note = self.mucous.config["banned"][user]
				attributes = []
				if self.mucous.config.has_key("ignored") and self.mucous.config["ignored"].has_key(user):
					attributes.append("ignored")
				if self.mucous.config.has_key("buddies") and self.mucous.config["buddies"].has_key(user):
					attributes.append("buddies")
				if self.mucous.config.has_key("trusted") and self.mucous.config["trusted"].has_key(user):
					attributes.append("trusted")
				self.logs["banned"].append([attributes, user, note])
		except Exception, e:
			self.mucous.Help.Log("debug", "ListBan: " + str(e))
			
	## Rebuild ListIgnore from Mucous.config
	# @param self UsersLists (class)
	def ListIgnore(self):
		try:
			if not self.mucous.config.has_key("ignored"):
				return
			self.logs["ignored"] = []
			alpha_list = self.mucous.config["ignored"].keys()
			alpha_list.sort(key=str.lower)
			for user in alpha_list:
				note = self.mucous.config["ignored"][user]
				attributes = []
				if self.mucous.config.has_key("banned") and self.mucous.config["banned"].has_key(user):
					attributes.append("banned")
				if self.mucous.config.has_key("buddies") and self.mucous.config["buddies"].has_key(user):
					attributes.append("buddies")
				if self.mucous.config.has_key("trusted") and self.mucous.config["trusted"].has_key(user):
					attributes.append("trusted")
				self.logs["ignored"].append([attributes, user, note])
				
							
		except Exception, e:
			self.mucous.Help.Log("debug", "ListIgnore: " + str(e))	
			
			

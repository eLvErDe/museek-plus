# This is part of the Mucous Museek Client, and distributed under the GPLv2
# Copyright (c) 2006 daelstorm. 
import curses.wrapper
import time	
## Alerts (messages in top-right of terminal)
#
class Alerts:
	## Constructor
	# @param self Alerts (class)
	# @param mucous Mucous (Class)
	def __init__(self, mucous):
		## @var mucous
		# Mucous (Class)
		self.mucous = mucous
		## @var alert
		# dict containing all the alert messages
		self.alert = { "CHAT": {}, "PRIVATE": [], "TRANSFERS" : [],  "SEARCH": [], "INFO": [], "BROWSE": [],  "HELP": [] }
		## @var log
		# log string containing current alert
		self.log = ""
		## @var windows
		# dict containing instances of curses windows
		self.windows = {}
		
	## Create and Draw alert box	
	# @param self Alerts (class)
	def Mode(self):
		try:
			if "border" in self.windows:
				del self.windows["border"]
			aw = self.windows["border"] = curses.newwin(1, 15, 0, self.mucous.w-15)
			aw.bkgdset(" ", self.mucous.colors["blackwhite"]  | curses.A_REVERSE | curses.A_BOLD)
			aw.idlok(1)
			
			self.setStatus(self.log)
		except:
			pass
			
	## Remove Alert from place	
	# @param self Alerts (class)
	# @param string alert text
	# @param place where the alert is stored
	def Remove(self, string, place):
		if place in self.alert:
			if string in self.alert[place]:
				self.alert[place].remove(string)
	## Add Alert to place
	# @param self Alerts (Class)
	# @param string alert text
	# @param place where the alert is stored
	def Add(self, string, place):
		if place in self.alert:
			if string not in self.alert[place]:
				self.alert[place].append(string)
			else:
				self.alert[place].remove(string)
				self.alert[place].append(string)
			self.Check()
			
	## Check alerts, remove old ones, and call Next
	# @param self Alerts (class)
	def Check(self):
		try:
			if self.mucous.mode == "chat":
				if self.mucous.ChatRooms.current in self.alert["CHAT"].keys():
					del self.alert["CHAT"][self.mucous.ChatRooms.current]
			elif self.mucous.mode == "private":
				if self.mucous.PrivateChat.current in self.alert["PRIVATE"]:
					self.alert["PRIVATE"].remove(self.mucous.PrivateChat.current)
			elif self.mucous.mode == "browse":
				if self.mucous.BrowseShares.current in self.alert["BROWSE"] and self.mucous.BrowseShares.current != "__default":
					self.alert["BROWSE"].remove(self.mucous.BrowseShares.current)
			elif self.mucous.mode == "search":
				if self.mucous.Search.current in self.alert["SEARCH"]:
					self.alert["SEARCH"].remove(self.mucous.Search.current)
			elif self.mucous.mode == "info":
				if self.mucous.UserInfo.current in self.alert["INFO"]:
					self.alert["INFO"].remove(self.mucous.UserInfo.current)
			elif self.mucous.mode in ("help", "debug", "status"):
				#if ("debug", "help", "status") in self.alert["HELP"]:
				self.alert["HELP"] = []
			self.Next()
			self.mucous.HotKeyBar()
		except Exception, e:
			self.mucous.Help.Log("debug", "Alert.Check: " + str(e))
			
	## Display old alerts
	# @param self Alerts (class)
	def Next(self):
		try:
			if self.mucous.mode ==  "search":
				if self.alert["SEARCH"] != []:
					for s in self.alert["SEARCH"]:
						self.setStatus(self.mucous.Search.tickets[s])
						return
				else:
					self.Pick()
			if self.mucous.mode in ("info", "private",  "browse"):
			#if self.mode.upper() in ("PRIVATE", "TRANSFERS",  "SEARCH", "INFO", "BROWSE"):
			
				if self.alert[self.mucous.mode.upper()] != []:
					for s in self.alert[self.mucous.mode.upper()]:
						self.setStatus(s)
						return
				else:
					self.Pick()
			elif self.mucous.mode.upper()  == "CHAT":
				if self.alert[self.mucous.mode.upper()] != {}:
					for m, l in self.alert["CHAT"].items():
						self.setStatus(m)
						return
					self.setStatus("")
				else:
					self.Pick()
			elif self.mucous.mode in ("help", "debug", "status"):
				for s in self.alert["HELP"]:
					if s == "help":
						self.setStatus("New Help")
					elif s == "debug":
						self.setStatus("New Bug")
					elif s == "status":	
						self.setStatus("New Status")
					break
				else:
					self.Pick()
			else:
				self.Pick()
		except Exception, e:
			self.mucous.Help.Log( "debug", "Alert.Next: " + str(e))
			
	## Display old alerts
	# @param self Alerts (class)
	def Pick(self):
		try:
			for mode, lists in self.alert.items():
				#self.log["help"].append(str(mode)+" " +str(l))
				if lists == []:
					continue
				elif lists == {}:
					continue
				
				if mode == "HELP":
					for s in self.alert["HELP"]:
						if s == "help":
							self.setStatus("New Help")
						elif s == "debug":
							self.setStatus("New Bug")
						elif s == "status":	
							self.setStatus("New Status")
						break
					return
				elif mode == "SEARCH":
					for s in self.alert["SEARCH"]:
	
						self.setStatus(self.mucous.Search.tickets[s])
						return

				for i in lists:
					if i != "":
						self.setStatus(i)
						return
			self.setStatus("")
		except Exception, e:
			self.mucous.Help.Log( "debug", "Pick: " + str(e))
			
	## Change Alert contents directly
	# shrunk to 14 characters
	# @param self Alerts (class)
	# @param s string
	def setStatus(self, s):
		try:
			self.log = s
			aw = self.windows["border"]
			aw.erase()
			
			if len(self.log[:14]) < 13 and len(self.log[:14]) > 0:
				line = " "+self.mucous.dlang( self.log[:14] )+" "
			else:
				line = self.mucous.dlang( self.log[:14] )
			aw.addstr(line, self.mucous.colors["yellow"] )
			aw.refresh()
		except Exception, e:
			self.mucous.Help.Log( "debug", "Alert.setStatus: " + str(e))
				
				

# This is part of the Mucous Museek Client, and distributed under the GPLv2
# Copyright (c) 2006 daelstorm. 
import threading
import os
import curses.wrapper
## Setup and view mucous and museekd config options
#			
class Setup:
	## Constructor
	# @param self Setup (class)
	# @param mucous Mucous (class)
	def __init__(self, mucous):
		## @var mucous 
		# Mucous (Class)
		self.mucous = mucous
		## @var windows
		# dict containing instances of curses windows
		self.windows = {"border":{}, "option":{}}
		## @var input
		# determines how to parse input enter in Setup
		self.input = "default"
		## @var current
		# Currently shown settings
		self.current = "mucous"
		## @var dimensions
		# dict containing placement data for windows
		self.dimensions = {}
		## @var modes
		# Setup modes
		self.modes = ["mucous", "museek", "shares", "userinfo", "logs"]
		## @var password
		# display password
		self.password = False
		self.numberboxes = ["cycletime", "scrolltime", "roomsize", "slots"]
		self.switchorder = []
		
	## Reset input to 'default' and redraw setup
	# @param self Setup (class)
	def Default(self):
		try:
			self.input = "default"
			self.Mode()
			self.mucous.ModeTopbar()
		except Exception, e:
			self.mucous.Help.Log("debug", "Setup.Default: "+ str(e))
			
	## Create window, display contents
	# @param self is Setup (Class)
	def Mode(self):
		self.mucous.UseAnotherEntryBox()
		self.mucous.mode = "setup"
		self.mucous.PopupMenu.show = False
		try:
			# Cleanup stale windows
			if "setup" in self.windows["border"]:
				del self.windows["border"]["setup"]
				
			w = self.dimensions["setup"] = {"height": self.mucous.h-5, "width": self.mucous.w, "top": 1, "left": 0}	
			mw = self.windows["border"]["setup"] = curses.newwin(w["height"], w["width"], w["top"], w["left"])
			mw.erase()
			mw.border()
			mw.addstr(w["height"]-1, 2, "< Use the mouse, if possible. Otherwise: /help setup >")

			if self.current == "mucous":
				#self.mucous.SetEditTitle("Mucous Setup")
				try:
					mw.addstr(0, 1, "< Mucous >",  curses.A_BOLD)
					mw.addstr(0, 15, "< Museek >")
					mw.addstr(0, 30, "< Shares >")
					mw.addstr(0, 45, "< Userinfo >")
					mw.addstr(0, 60, "< Logs >")
				except:
					pass
			elif self.current == "museek":
				try:
					mw.addstr(0, 1, "< Mucous >")
					mw.addstr(0, 15, "< Museek >",  curses.A_BOLD)
					mw.addstr(0, 30, "< Shares >")
					mw.addstr(0, 45, "< Userinfo >")
					mw.addstr(0, 60, "< Logs >")
				except:
					pass
			elif self.current == "shares":
				try:
					mw.addstr(0, 1, "< Mucous >")
					mw.addstr(0, 15, "< Museek >")
					mw.addstr(0, 30, "< Shares >",  curses.A_BOLD)
					mw.addstr(0, 45, "< Userinfo >")
					mw.addstr(0, 60, "< Logs >")
				except:
					pass
			elif self.current == "userinfo":
				try:
					mw.addstr(0, 1, "< Mucous >")
					mw.addstr(0, 15, "< Museek >")
					mw.addstr(0, 30, "< Shares >")
					mw.addstr(0, 45, "< Userinfo >",  curses.A_BOLD)
					mw.addstr(0, 60, "< Logs >")
				except:
					pass
			elif self.current == "logs":
				try:
					mw.addstr(0, 1, "< Mucous >")
					mw.addstr(0, 15, "< Museek >")
					mw.addstr(0, 30, "< Shares >")
					mw.addstr(0, 45, "< Userinfo >")
					mw.addstr(0, 60, "< Logs >",  curses.A_BOLD)
				except:
					pass
			self.InputTitle()
			mw.noutrefresh()
			# Create buttons, settings and immediately Delete them to aviod leaks
			if self.current in ("shares"):
				self.DrawShares()
			elif self.current in ("userinfo"):
				self.DrawUserinfo()
			elif self.current in ("logs"):
				self.DrawLogs()
			elif self.current == "mucous":
				self.DrawMucous()
			elif self.current == "museek":
				self.DrawMuseek()
				

		except Exception, e:
			self.mucous.Help.Log("debug", "Setup.Mode: " + str(e) )
		
		self.mucous.HotKeyBar()
		curses.doupdate()

		
	## Draw Mucous settings
	# @param self is Setup (Class)
	def DrawMucous(self):
		w = self.dimensions["setup"]
		mw = self.windows["border"]["setup"]
		self.switchorder = ["default", "interface",  "interface-password", "showtickers", "tickertype",  "cycletime", "scrolltime", "autoclear", "autoretry", "autobuddy", "beep", "roomsize", "encoding",  "url-custom", "url-reader", "save"]
		# Interface
		interface =""
		if "connection" in self.mucous.Config:
			interface = self.mucous.dlang( self.mucous.Config["connection"]["interface"] )
		
		self.SetupButton("Museek Interface", interface ,2,1, 3,32, edit=(self.input=="interface"))
		
		
		if "connection" in self.mucous.Config:
			password = self.mucous.dlang( str(self.mucous.Config["connection"]["passw"]) )
			if self.password == False and not (self.input=="interface-password"):
				value = '*' * len(password)
			else:
				value = password
		else:
			value = ""
			
		self.SetupButton("Interface Password", value, 5,1,3,32, edit=(self.input=="interface-password"))
		

		self.SetupCheck("Show Tickers:         /showtickers", self.mucous.Config["tickers"]["tickers_enabled"] ,2,34,1,45, True, selected=(self.input=="showtickers"))
		self.SetupCheck("Cycle Tickers:        /tickers", self.mucous.Config["tickers"]["ticker_cycle"] ,3,34,1,45, True, True, selected=(self.input=="tickertype"))
		self.SetupCheck("Scroll Tickers:       /tickers", self.mucous.Config["tickers"]["ticker_scroll"] ,4,34,1,45, True, True, selected=(self.input=="tickertype"))

		self.SetupCheck("Auto-clear Transfers: /autoclear", self.mucous.Config["mucous"]["auto-clear"],9,1,1,45, True, selected=(self.input=="autoclear"))

		self.SetupCheck("Auto-retry Transfers: /autoretry", self.mucous.Config["mucous"]["auto-retry"],10,1, 1,45, True, selected=(self.input=="autoretry"))

		self.SetupCheck("Auto-Buddy downloads: /autobuddy", self.mucous.Config["mucous"]["autobuddy"],11,1,1,45, True, selected=(self.input=="autobuddy"))
		
		self.SetupCheck("Beep:                 /beep", self.mucous.Config["mucous"]["beep"],12,1,1,45, True, selected=(self.input=="beep"))
		
		self.SetupButton("< Ticker Time >", " -          +\n -          +", 5,39, 4,17, optionbold=(self.input in ("scrolltime", "cycletime")) , titlepad=False)
		
		self.EditLine("%.2f" % float(self.mucous.Config["tickers"]["cycletime"]), 6,43, 1,5, edit=(self.input=="cycletime") )
		self.EditLine("%.2f" % float(self.mucous.Config["tickers"]["scrolltime"]), 7,43, 1,5, edit=(self.input=="scrolltime") )
		# Minimum Room size for Roomlist
		
		self.SetupButton("<Min RoomSize>", " -          +", 9,39, 3,17, optionbold=(self.input=="roomsize"), titlepad=False)
	
		self.EditLine(str(self.mucous.Config["mucous"]["roomlistminsize"]), 10,43, 1,8, edit=(self.input=="roomsize") )

		self.SetupButton("Encoding", self.mucous.Config["mucous"]["language"], 12,39,3,17, optionbold=(self.input=="encoding"))
		
		# Custom URL
		prefix =""
		if "url custom" in self.mucous.Config["mucous"]:
			prefix = self.mucous.dlang(self.mucous.Config["mucous"]["url custom"])
		self.SetupButton("Custom URL Reader", prefix,15,1,3,38,  edit=(self.input=="url-custom"))
		self.SetupButton("URL Reader", self.mucous.dlang(self.mucous.Config["mucous"]["url reader"]), 15,39,3,17, optionbold=(self.input=="url-reader"))


		# Save button
		
		#self.SetupButton(None, " Save Config", 15,49,3,16, optionbold=(self.input=="save"))
		self.SetupButton(None, " Save Config", self.mucous.h-8,self.mucous.w-17,3,16, optionbold=(self.input=="save"))
		
	## Draw Museek settings
	# @param self is Setup (Class)
	def DrawMuseek(self):
		w = self.dimensions["setup"]
		mw = self.windows["border"]["setup"]
		self.switchorder = ["default", "server-host",  "server-port",  "soulseek-username", "soulseek-password", "museek-interface-password", "connectmode", "only_buddies", "privilege_buddies", "have_buddy_shares", "trusting_uploads", "user_warnings", "slots", "download-dir", "incomplete-dir", "save"]
		# Server
		
		self.SetupButton("Server", "Host:\nPort:\nName:\nPass:", 2,1,6,38, optionbold=(self.input in("server-host",  "server-port",  "soulseek-username", "soulseek-password")))

		
		if "server" in self.mucous.config:
			value = self.mucous.dlang( self.mucous.config["server"]["host"])
		else: value = ""
		self.EditLine(value, 3,8,1,30, edit=(self.input=="server-host") ) 

		if "server" in self.mucous.config:
			value = self.mucous.dlang( self.mucous.config["server"]["port"])
		else: value = ""
		self.EditLine(value, 4,8,1,30, edit=(self.input=="server-port") ) 
		
		if "server" in self.mucous.config:
			value = self.mucous.dlang(self.mucous.config["server"]["username"])
		else: value = ""
		self.EditLine(value, 5,8,1,30, edit=(self.input=="soulseek-username") )

		if "server" in self.mucous.config:
			value = self.mucous.dlang(self.mucous.config["server"]["password"])
			if self.password == False and not (self.input=="soulseek-password") :
				value = "*" * len(value)
		else: value = ""
		self.EditLine(value, 6,8,1,30, edit=(self.input=="soulseek-password") )
		

		if "interfaces" in self.mucous.config:
			if self.password == True or self.input == "museek-interface-password":
				value = self.mucous.dlang( self.mucous.config["interfaces"]["password"] )
			else:
				value = "*" * len(self.mucous.config["interfaces"]["password"])
		else:
			value = ""

		self.SetupButton("< Interface Password >", value ,2,39,3,26, False, False, edit=(self.input == "museek-interface-password") )
		#------------------------
		value = ""
		if self.mucous.config.has_key("transfers"):
			value = self.mucous.config["transfers"]["only_buddies"]
		self.SetupCheck("Share to Buddies-Only", value, 8,1,1,30, True, selected=(self.input=="only_buddies"))
		if self.mucous.config.has_key("transfers"):
			value = self.mucous.config["transfers"]["privilege_buddies"]
		else: value = ""
		self.SetupCheck("Buddies get Privileges", value, 9,1,1,30, True, selected=(self.input=="privilege_buddies"))
		
		if self.mucous.config.has_key("transfers"):
			value = self.mucous.config["transfers"]["have_buddy_shares"]
		else: value = ""
		self.SetupCheck("Enable Buddy-Only shares", value, 10,1,1,30, True, selected=(self.input=="have_buddy_shares"))
		
		if self.mucous.config.has_key("transfers"):
			value = self.mucous.config["transfers"]["trusting_uploads"]
		else: value = "" 
		self.SetupCheck("Allow Trusted users to send you files", value, 11,1,1,self.mucous.w-3, True, selected=(self.input=="trusting_uploads"))
		
		if self.mucous.config.has_key("transfers"):
			value = self.mucous.config["transfers"]["user_warnings"]
		else: value = ""
		self.SetupCheck("Send automatic warnings via Private Chat", value, 12,1,1,self.mucous.w-3, True, selected=(self.input=="user_warnings"))
		
		if self.mucous.config.has_key("clients"):
			value = self.mucous.config["clients"]["connectmode"]
		else: value = ""
		self.SetupButton("<Connections >", value ,5,49,3,16, optionbold=(self.input=="connectmode"), titlepad=False)
		
		self.SetupButton("<Upload Slots>", " -          +", 8,49,3,16, optionbold=(self.input=="slots"), titlepad=False)
		if self.mucous.config.has_key("transfers"):
			value = str(self.mucous.config["transfers"]["upload_slots"])
		else:
			value = 0
		self.EditLine(value, 9,53, 1,8, edit=(self.input=="slots") )
		
		dirwin = curses.newwin(4,self.mucous.w-2,13,1)
		dirwin.border()
		dirwin.addstr(0, 1, "< Download/Incomplete Directories >",  curses.A_BOLD)
		dirwin.noutrefresh()
		
		if self.mucous.config.has_key("transfers"):
			
			value = self.mucous.dlang(self.mucous.config["transfers"]["download-dir"])
			if (self.input=="download-dir"):
				value = "/" + value
		else: value = ""
		self.EditLine(value, 14,2,1,self.mucous.w-4, edit=(self.input=="download-dir") )
		
		if self.mucous.config.has_key("transfers"):
			
			value = self.mucous.dlang(self.mucous.config["transfers"]["incomplete-dir"])
			if (self.input=="incomplete-dir"):
				value = "/" + value
		else: value = ""
		self.EditLine(value, 15,2,1,self.mucous.w-4, edit=(self.input=="incomplete-dir") )


		del dirwin
		
	## Draw Shares config options
	# @param self is Setup (Class)
	def DrawShares(self):
		w = self.dimensions["setup"]
		mw = self.windows["border"]["setup"]
		self.switchorder = ["default", "listnormal",  "adddir", "rmdir", "rescannormal", "updatenormal", "listbuddy", "addbuddydir", "rmbuddydir", "rescanbuddy", "updatebuddy", "reloadshares"]
		
		# First Row
		self.SetupButton("Normal", "List Shared", 3,1,3,16, optionbold=(self.input=="listnormal"))
		self.SetupButton(None, "Add Directory", 3,17,3,16, optionbold=(self.input=="adddir"))
		self.SetupButton(None, "Remove Dir", 3,33,3,16, optionbold=(self.input=="rmdir"))
		self.SetupButton(None, "Click on the buttons to run the local muscan.\nWhen you click on Add/Remove Directory, type in the directory below, and start with '//' instead of just a '/'", 2,49,10,self.mucous.w-50)
		# Second Row
		self.SetupButton(None, "Rescan Shares",6,1,3,16, optionbold=(self.input=="rescannormal"))
		self.SetupButton(None, "Update Shares", 6,17,3,16, optionbold=(self.input=="updatenormal"))
		
		# Third Row
		self.SetupButton("Buddy-only", "List Shared", 9,1,3,16, optionbold=(self.input=="listbuddy"))
		self.SetupButton(None, "Add Directory", 9,17,3,16, optionbold=(self.input=="addbuddydir"))
		self.SetupButton(None, "Remove Dir", 9,33,3,16, optionbold=(self.input=="rmbuddydir"))
		
		# Fourth Row
		self.SetupButton(None, "Rescan Shares", 12,1,3,16, optionbold=(self.input=="rescanbuddy"))
		self.SetupButton(None, "Update Shares", 12,17,3,16, optionbold=(self.input=="updatebuddy"))
		self.SetupButton(None, "Reload Shares", 12,49,3,16, optionbold=(self.input=="reloadshares"))
		
		museekconfigfile = self.mucous.Spl["museekconfigfile"]
		# Fifth Row
		self.SetupButton("Museek Config File", museekconfigfile, 15,1,3,self.mucous.w-2)
		
		
	## Draw Log file options
	# @param self is Setup (Class)
	def DrawLogs(self):
		w = self.dimensions["setup"]
		mw = self.windows["border"]["setup"]
		self.switchorder = ["default", "logging",  "logdir", "save"]
		self.SetupCheck("Log Chat messages? ", self.mucous.Config["mucous"]["logging"],2,1,1,30, True, selected=(self.input=="logging"))
		value = os.path.expanduser(self.mucous.Config["mucous"]["log_dir"])
		if self.input=="logdir":
			value = "/" + value
		
		self.SetupButton("Log Directory", value, 5,1,3,self.mucous.w-2, edit=(self.input=="logdir") )
		self.SetupButton(None, " Save Config", self.mucous.h-8,self.mucous.w-17,3,16, optionbold=(self.input=="save"))
		
	## Draw User Info 
	# @param self is Setup (Class)
	def DrawUserinfo(self):
		w = self.dimensions["setup"]
		mw = self.windows["border"]["setup"]
		self.switchorder = ["default", "userinfo",  "userimage"]
		info = ""
		if self.mucous.config.has_key("userinfo"):
			if "text" in self.mucous.config["userinfo"]:
				info = self.mucous.config["userinfo"]["text"]

		self.SetupButton("Your Userinfo", info, 2,1,self.mucous.h-8,self.mucous.w-18, edit=(self.input == "userinfo"))
		
		bwin = curses.newwin(8,16,2,self.mucous.w-17)
		bwin.border()
		try:
			bwin.addstr(0, 1, "< Stats >",  curses.A_BOLD)
		except:
			pass
		bwin.noutrefresh()
		statswin = bwin.subwin(6,14,3,self.mucous.w-16)

		statswin.scrollok(1)

		if self.mucous.data["mystats"] != []:
			try:
				statswin.addstr("Files: %s\nDirs:  %s\nSpeed: %sKB/s\nDownloads: %s" % (str(self.mucous.data["mystats"][3]), str(self.mucous.data["mystats"][4]), str(self.mucous.data["mystats"][1]/1024), str(self.mucous.data["mystats"][2])), self.mucous.colors["cyan"] )
				#self.data["mystats"] = user,  avgspeed, numdownloads, numfiles, numdirs
			except:
				pass
		else:
			if self.mucous.username != None:
				self.mucous.D.PeerStats(self.mucous.username)
		statswin.noutrefresh()

		del statswin

		del bwin
		
		inputimage = curses.newwin(1,13,self.mucous.h-6,1)
		inputimage.erase()
		inputimage.addstr( "Your Image: ")
		inputimage.noutrefresh()
		del inputimage
		
		if self.mucous.config.has_key("userinfo") and self.mucous.config["userinfo"].has_key("image"):
			if (self.input=="userimage"):
				value = "/" + self.mucous.config["userinfo"]["image"]
			else: value = self.mucous.config["userinfo"]["image"]
			self.EditLine(value, self.mucous.h-6,14,1,self.mucous.w-2-14, edit=(self.input=="userimage") )

		
	## Update input line's title
	# @param self is Setup (Class)
	def InputTitle(self):
		si = self.input
		inputmodes = ["default","interface", "custom-url", "interface-password", "museek-interface-password", "museek-interface-bind", "server-host","server-port", "soulseek-username", "soulseek-password", "slots", "download-dir", "incomplete-dir", "userinfo", "userimage", "adddir", "rmdir", "addbuddydir", "rmbuddydir"]
		if si == "default":
			self.mucous.SetEditTitle("Setup Mode")
		elif si == "interface":
			self.mucous.SetEditTitle("Set Interface")
		elif si == "interface-password":
			self.mucous.SetEditTitle("Set Mucous's Interface password")
		elif si == "url-reader":
			self.mucous.SetEditTitle("Set a program to open HTTP URLs")
		elif si == "url-custom":
			self.mucous.SetEditTitle("Set custom URL handler: command$command")
		elif si == "save":
			self.mucous.SetEditTitle("Save Mucous's Config")
		elif si == "museek-interface-password":
			self.mucous.SetEditTitle("Set Museek's Interface password")
		elif si == "museek-interface-bind":
			self.mucous.SetEditTitle("Add a Museek Interface")
		elif si == "server-host":
			self.mucous.SetEditTitle("Set Server Address")
		elif si == "server-port":
			self.mucous.SetEditTitle("Set Server Port")
		elif si == "soulseek-username":
			self.mucous.SetEditTitle("Set Soulseek Username")
		elif si == "soulseek-password":
			self.mucous.SetEditTitle("Set Soulseek Password")
		elif si == "slots":
			self.mucous.SetEditTitle("Set Number of Upload Slots to:")
		elif si == "download-dir":
			self.mucous.SetEditTitle("Set completed download directory")
		elif si == "incomplete-dir":
			self.mucous.SetEditTitle("Set incompleted download directory")
		elif si == "userinfo":
			self.mucous.SetEditTitle("Set Your UserInfo")
		elif si == "userimage":
			self.mucous.SetEditTitle("Set Your UserImage")
		elif si == "adddir":
			self.mucous.SetEditTitle("Add directory to your normal shares")
		elif si == "rmdir":
			self.mucous.SetEditTitle("Remove directory from your normal shares")
		elif si == "logdir":
			self.mucous.SetEditTitle("Set the directory Chat logs are saved in")
		elif si == "logging":
			self.mucous.SetEditTitle("Toggle Logging")
		elif si == "addbuddydir":
			self.mucous.SetEditTitle("Add directory to your buddy shares")
		elif si == "rmbuddydir":
			self.mucous.SetEditTitle("Remove directory from your buddy shares")
		else:
			self.mucous.SetEditTitle("Setup Mode")
	## Parse input entry line for interest and match it with Setup.input
	# @param self is Setup (Class)
	# @param line is a text string
	def InputSetup(self, line):
		try:
			inputs  = ["default", "interface",  "interface-password", "showtickers", "tickertype", "autoclear", "autoretry", "autobuddy", "beep", "cycletime", "scrolltime", "encoding", "roomsize", "url-custom", "url-reader", "save",  "server-host", "server-port", "soulseek-username", "soulseek-password", "museek-interface-password", "connectmode", "only_buddies", "privilege_buddies", "have_buddy_shares", "trusting_uploads", "user_warnings", "slots", "download-dir", "incomplete-dir", "listnormal", "adddir", "rmdir", "rescannormal", "updatenormal", "listbuddy", "addbuddydir", "rmbuddydir", "rescanbuddy", "updatebuddy", "reloadshares", "logging", "logdir", "userinfo", "userimage"]
			
			if self.input not in inputs:
				self.mucous.SetEditTitle(self.input)
				return
			
			line = self.mucous.dlang(line)
			# Mucoous
			if self.input == "interface":
				self.mucous.Config["connection"]["interface"] = line
			elif self.input=="interface-password":
				self.mucous.Config["connection"]["passw"] = line
			elif self.input=="showtickers":
				self.mucous.ChatRooms.ToggleTickersDisplay()
			elif self.input=="tickertype":
				self.mucous.ChatRooms.ToggleTickers()
			elif self.input=="autoclear":
				self.ToggleAutoClear()
				return
			elif self.input=="autoretry":
				self.ToggleAutoRetry()
				return
			elif self.input=="autobuddy":
				self.ToggleAutoBuddy()
				return
			elif self.input=="beep":
				self.mucous.ToggleBeep()
				self.Mode()
				return
			elif self.input=="cycletime":
				#if line.isdigit():
				try:self.mucous.Config["tickers"]["cycletime"] = str(float(line))
				except:pass
			elif self.input=="scrolltime":
				try:self.mucous.Config["tickers"]["scrolltime"] = str(float(line))
				except:pass
			elif self.input=="encoding":
				self.mucous.Config["mucous"]["language"]=self.mucous.FormatData.RotateList("right", self.mucous.encodings, self.mucous.Config["mucous"]["language"], "no")
				self.Mode()
				return
			elif self.input=="roomsize":
				if line.isdigit():
					self.mucous.Config["mucous"]["roomlistminsize"] = int(line)
			elif self.input=="url-custom":
				if "$" in line:
					self.mucous.Config["mucous"]["url custom"] = line
			elif self.input=="url-reader":
				_list = ["lynx", "links", "elinks", "firefox", "custom"]
				self.mucous.Config["mucous"]["url reader"] = self.mucous.FormatData.RotateList("right", _list, self.mucous.Config["mucous"]["url reader"], "no")
				self.Mode()
				return
			elif self.input=="save":
				self.mucous.config_manager.update_config()
			# Museek
			elif self.input=="museek-interface-password":
				self.mucous.D.ConfigSet("interfaces", "password", line)
			elif self.input=="museek-interface-bind":
				self.mucous.D.ConfigSet("interfaces.bind", line)
			elif self.input=="connectmode":
				self.ToggleConnectMode()
				return
			elif self.input=="server-host":
				self.mucous.D.ConfigSet("server", "host", line)
			elif self.input=="server-port":
				self.mucous.D.ConfigSet("server", "port", line)
			elif self.input=="soulseek-username":
				self.mucous.D.ConfigSet("server", "username", line)
			elif self.input=="soulseek-password":
				self.mucous.D.ConfigSet("server", "password", line)
				
			elif self.input == "only_buddies":
				self.ToggleOnlyBuddies()
			elif self.input == "privilege_buddies":
				self.TogglePrivilegeBuddies()
			elif self.input == "have_buddy_shares":
				self.ToggleHaveBuddyShares()
			elif self.input == "trusting_uploads":
				self.ToggleTrustedUploads()
			elif self.input == "user_warnings":
				self.ToggleUserWarnings()
			elif self.input == "slots":
				if line.isdigit():
					self.mucous.D.ConfigSet("transfers", "upload_slots", line)
				
			elif self.input=="download-dir":
				self.mucous.D.ConfigSet("transfers", "download-dir", line)
			elif self.input=="incomplete-dir":
				
				self.mucous.D.ConfigSet("transfers", "incomplete-dir", line)
			# Shares
			elif self.input=="listnormal":
				self.mucous.Muscan.ListNormal()
			elif self.input=="rescannormal":
				self.mucous.Muscan.RescanNormal()
			elif self.input=="updatenormal":
				self.mucous.Muscan.UpdateNormal()
			elif self.input=="listbuddy":
				self.mucous.Muscan.ListBuddy()
			elif self.input=="rescanbuddy":
				self.mucous.Muscan.RescanBuddy()
			elif self.input=="updatebuddy":
				self.mucous.Muscan.UpdateBuddy()
			elif self.input=="reloadshares":
				self.mucous.D.ReloadShares()
			elif self.input=="adddir":
				if line == "": return
				self.mucous.Muscan.Command(["muscan", "-s", line])
				self.mucous.Help.Log("status", "Adding "+line+" to normal shares. Scanning will begin.")
			elif self.input=="rmdir":
				if line == "": return
				self.mucous.Muscan.Command(["muscan", "-u", line])
				self.mucous.Help.Log("status", "Removing "+line+" from normal shares. Please rescan or update.")
			elif self.input=="addbuddydir":
				if line == "": return
				self.mucous.Muscan.Command(["muscan", "-b", "-s", line])
				self.mucous.Help.Log("status", "Adding "+line+" to buddy shares. Scanning will begin.")
			elif self.input=="rmbuddydir":
				if line == "": return
				self.mucous.Muscan.Command(["muscan", "-b", "-u", line])
				self.mucous.Help.Log("status", "Removing "+line+" from buddy shares. Please rescan or update.")
				# Userinfo
			elif self.input=="userinfo":
				try:
					line = line.replace('\\n', '\n')
					self.mucous.D.ConfigSet("userinfo", "text", line)
				except Exception, e:
					self.mucous.Help.Log("debug", "set userinfo: "+str( e))
			elif self.input == "userimage":
				
				try:
					self.mucous.D.ConfigSet("userinfo", "image", line)
				except Exception, e:
					self.mucous.Help.Log("debug", "set userinfo image: "+str( e))
			
			# Logging
			elif self.input == "logdir":
				self.mucous.Config["mucous"]["log_dir"] = line
				self.Mode()
			elif self.input == "logging":
				self.mucous.ToggleLogging()
				self.Mode()
				return
			if self.input != "default":
				self.SetInput()
			#self.input = "default"
			#self.Mode()
		except Exception,e:
			self.mucous.Help.Log("debug", "InputSetup: " + str(e))
	
	## Draw Mucous settings
	# @param self is Setup (Class)
	
	def Switch(self, key):
		if key == "KEY_DOWN":
			self.input = self.mucous.FormatData.RotateList("right", self.switchorder, self.input, "no")
		elif key == "KEY_UP":
			self.input = self.mucous.FormatData.RotateList("left", self.switchorder, self.input, "no")
			
		else:
			return
		self.Mode()
		
	def ChangeSize(self, direction):
		#self.numberboxes = ["cycletime", "scrolltime", "roomsize", "slots"]
		if self.input == "cycletime":
			difference = 0.5
			value = float(self.mucous.Config["tickers"]["cycletime"])
			if direction == "+":
				value += difference
			elif direction == "-":
				value -= difference
			if float(value) < 1.0:
				value = str(1.0)
			self.mucous.Config["tickers"]["cycletime"] = value
		elif self.input == "scrolltime":
			difference = 0.1
			value = float(self.mucous.Config["tickers"]["scrolltime"])
			if direction == "+":
				value += difference
			elif direction == "-":
				value -= difference
			if float(value) < 0.1:
				value = str(0.1)
			self.mucous.Config["tickers"]["scrolltime"] = value
		elif self.input == "roomsize":
			difference = 1
			value = int(self.mucous.Config["mucous"]["roomlistminsize"])
			if direction == "+":
				value += difference
			elif direction == "-":
				value -= difference
			if value < 1:
				value = 1
			self.mucous.Config["mucous"]["roomlistminsize"] = value
		elif self.input == "slots":
			difference = 1
			value = int(self.mucous.config["transfers"]["upload_slots"])
			if direction == "+":
				value += difference
			elif direction == "-":
				value -= difference
			if value < 0:
				value = 0
			value = str(value)
			self.mucous.D.ConfigSet("transfers", "upload_slots", value)
		self.Mode()
			
	## Toggle Museekd's Connection Method
	# @param self is Setup (Class)
	def ToggleConnectMode(self):
		if "clients" in self.mucous.config:
			if self.mucous.config["clients"]["connectmode"] == "passive":
				self.mucous.D.ConfigSet("clients", "connectmode", "active")
			elif self.mucous.config["clients"]["connectmode"] == "active":
				self.mucous.D.ConfigSet("clients", "connectmode", "passive")
		self.Mode()
		
	def ToggleOnlyBuddies(self):
		if self.mucous.config["transfers"]["only_buddies"] == "true":
			self.mucous.D.ConfigSet("transfers", "only_buddies", "false")
		elif self.mucous.config["transfers"]["only_buddies"] == "false":
			self.mucous.D.ConfigSet("transfers", "only_buddies", "true")
			
	def ToggleHaveBuddyShares(self):
		if self.mucous.config["transfers"]["have_buddy_shares"] == "true":
			self.mucous.D.ConfigSet("transfers", "have_buddy_shares", "false")
				
		elif self.mucous.config["transfers"]["have_buddy_shares"] == "false":
			self.mucous.D.ConfigSet("transfers", "have_buddy_shares", "true")
			
	def TogglePrivilegeBuddies(self):
		if self.mucous.config["transfers"]["privilege_buddies"] == "true":
			self.mucous.D.ConfigSet("transfers", "privilege_buddies", "false")
				
		elif self.mucous.config["transfers"]["privilege_buddies"] == "false":
			self.mucous.D.ConfigSet("transfers", "privilege_buddies", "true")
			
	def ToggleTrustedUploads(self):
		if self.mucous.config["transfers"]["trusting_uploads"]== "true":
			self.mucous.D.ConfigSet("transfers", "trusting_uploads", "false")
				
		elif self.mucous.config["transfers"]["trusting_uploads"] == "false":
			self.mucous.D.ConfigSet("transfers", "trusting_uploads", "true")
			
	def ToggleUserWarnings(self):
		if self.mucous.config["transfers"]["user_warnings"] == "true":
			self.mucous.D.ConfigSet("transfers", "user_warnings", "false")
				
		elif self.mucous.config["transfers"]["user_warnings"] == "false":
			self.mucous.D.ConfigSet("transfers", "user_warnings", "true")
			
	def ToggleAutoClear(self):
		if self.mucous.Config["mucous"]["auto-clear"] == "yes":
			self.mucous.Config["mucous"]["auto-clear"] = "no"
			self.mucous.timers["clear"].cancel()
			self.mucous.timers["clear"] = threading.Timer(30.0, self.mucous.ThreadTransfersClear)
			
		else:
			self.mucous.Config["mucous"]["auto-clear"] ="yes"
			self.mucous.timers["clear"].cancel()
			self.mucous.timers["clear"] = threading.Timer(30.0, self.mucous.ThreadTransfersClear)
			self.mucous.timers["clear"].start()
		self.Mode()
		
	def ToggleAutoBuddy(self):
		if self.mucous.Config["mucous"]["autobuddy"]  == "yes":
			self.mucous.Config["mucous"]["autobuddy"] = "no"

		elif self.mucous.Config["mucous"]["autobuddy"]  == "no":
			self.mucous.Config["mucous"]["autobuddy"] = "yes"
		self.Mode()
		
	def ToggleAutoRetry(self):
		if str(self.mucous.Config["mucous"]["auto-retry"]) == "yes":
			self.mucous.Config["mucous"]["auto-retry"] = "no"
			self.mucous.timers["retry"].cancel()
		else:
			self.mucous.Config["mucous"]["auto-retry"] ="yes"
			self.mucous.timers["retry"].cancel()
			self.mucous.timers["retry"] = threading.Timer(30.0, self.mucous.ThreadTransfersRetry)
			self.mucous.timers["retry"].start()
		self.Mode()

	## Create a Button
	# @param self Setup (class)
	# @param option
	# @param x
	# @param y
	# @param height
	# @param width
	# @param edit If False, delete references to curses windows; If True, use as input window
	
	def EditLine(self, option, x, y, height, width, edit=False):
		try:
			window = curses.newwin(height,width,x,y)
			
			if edit:
				window.attron(self.mucous.colors["green"]  | curses.A_BOLD)
				color = self.mucous.colors["green"] | curses.A_BOLD
			else:
				color = self.mucous.colors["cyan"]
			#inputimage2 = curses.newwin(1, self.mucous.w-2-14, self.mucous.h-6, 14)
			window.erase()
			window.bkgdset("_")
			window.scrollok(0)
			try:
				window.addstr( option[:width] , color)
			except Exception,e:
				pass
				#self.mucous.Help.Log("debug", "EditLine: "+str(e))
			window.noutrefresh()
			if edit:
				self.mucous.UseAnotherEntryBox(window, height, width, x, y, option, wrap=False)
			else:
				del window
		except Exception,e:
			self.mucous.Help.Log("debug", "EditLine: "+str(e))
		
	## Create a Button
	# @param self Setup (class)
	# @param title
	# @param option
	# @param x
	# @param y
	# @param height
	# @param width
	# @param optionbold True/False
	# @param titlepad True/False
	# @param border Draw a border around button
	# @param edit If False, delete references to curses windows; If True, use as input window
	def SetupButton(self, title, option, x, y, height, width, optionbold=False, titlepad=True, border=True, edit=False):
		try:
			winborder = curses.newwin(height,width,x,y)
			
			if edit or optionbold:
				winborder.attron(self.mucous.colors["green"])
				color = self.mucous.colors["green"] | curses.A_BOLD
			else:
				color = curses.A_BOLD
			winborder.erase()
			if border:
				winborder.border()
			if title != None:

				if titlepad:
					winborder.addstr(0,1, "< %s >" % title, color)
				else:
					winborder.addstr(0,1, title, color)
			winborder.noutrefresh()
			win = curses.newwin(height-2,width-2,x+1,y+1)
			if edit:
				win.attron(self.mucous.colors["green"] | curses.A_BOLD)
			win.scrollok(1)
			if option != None:
				if optionbold:
					win.addstr(option, self.mucous.colors["cyan"] | curses.A_BOLD)
				else:
					win.addstr(option, self.mucous.colors["cyan"])
			win.noutrefresh()
			if edit:
				#self.mucous.UseAnotherEntryBox(win, height-2, width-2, x+1, y+1, option, wrap=(height-2>1))
				self.mucous.UseAnotherEntryBox(win, height-2, width-2, x+1, y+1, option, wrap=True)
			else:
				del win
			del winborder
				
		except Exception,e:
			self.mucous.Help.Log("debug", "SetupButton: "+title+" "+ str(e))
			
	## Create a Check box or a toggle button
	# @param self Setup (class)
	# @param title
	# @param option
	# @param x
	# @param y
	# @param height
	# @param width
	# @param titlebold True/False
	# @param toggle True/False (is a togglebutton)
	# @param selected
	def SetupCheck(self, title, option, x, y, height, width, titlebold=False, toggle=False, selected=False, underlined=False):
		try:
			checked = "[x]"
			unchecked = "[ ]"
			toggled = "(*)"
			nottoggled = "( )"
			
			win = curses.newwin(height,width,x,y)
			if selected:
				if underlined:
					attr = self.mucous.colors["green"] | curses.A_UNDERLINE
				else:
					attr = self.mucous.colors["green"] 
			else:
				attr = curses.A_NORMAL
			win.erase()
			if option in (True, "True", "true", "yes"):
				if toggle:
					z = toggled
				else:
					z = checked
			else:
				if toggle:
					z = nottoggled
				else:
					z = unchecked
			win.addstr(z+" ", self.mucous.colors["cyan"])
			if title != None:
				if titlebold:
					win.addstr(title, attr | curses.A_BOLD)
				else:
					win.addstr(title, attr)
			win.noutrefresh()
			
			del win

		except Exception,e:
			self.mucous.Help.Log("debug", "SetupButton: "+title+" "+ str(e))
			
	## Mouse Coordinates in the Setup Mode
	# @param self is Setup (Class)
	# @param x is the horizontal position from the left
	# @param y is the vertical postion from the top
	# @param z is unimportant
	# @param event is the mouse event (button, doubleclick, etc) represented by a number
	def Mouse(self, x,y,z,event):
		try:
			
			if y == 1:
				if x >=1 and x <=12:
					if self.current != "mucous":
						self.current="mucous"
						self.input = "default"
				elif x >=16 and x <=26:
					if self.current != "museek":
						self.current = "museek"
						self.input = "default"
				elif x >=31 and x <=41:
					if self.current != "shares":
						self.current="shares"
						self.input = "default"
				elif x >=46 and x <=57:
					if self.current != "userinfo":
						self.current="userinfo"
						self.input = "default"
				elif x >=60 and x <=69:
					if self.current != "logs":
						self.current="logs"
						self.input = "default"
				self.SetInput()
				return
			
			if self.current=="mucous":
				if y in (2, 3, 4) and x >=1 and x <=35:
					self.SetInput("interface")
				elif y in (5, 6, 7) and x >=1 and x <=35:
					self.SetInput("interface-password")
				elif y == 2 and x >=39 and x <=75:
					self.input="showtickers"
					self.mucous.ChatRooms.ToggleTickersDisplay()
				elif y in (3, 4) and x >=39 and x <=75:
					self.input="tickertype"
					self.mucous.ChatRooms.ToggleTickers()
				
				elif y in (5, 6, 7, 8) and x >= 39 and x <= 55:
					# Minimum size of rooms displayed in room list
					if y in (5, 6):
						self.input="cycletime"
						
						if x >=40 and x <=43:
							self.ChangeSize("-")
						elif x >=49 and x <=55:
							self.ChangeSize("+")
						else:
							self.Mode()
					elif y in (7, 8):
						self.input="scrolltime"
						if x >=40 and x <=43:
							self.ChangeSize("-")
						elif x >=49 and x <=55:
							self.ChangeSize("+")
						else:
							self.Mode()
				elif y == 9 and x >=1 and x <=38:
					self.input="autoclear"
					self.ToggleAutoClear()
				elif y in (9, 10, 11) and x >= 39 and x <= 55:
					# Minimum size of rooms displayed in room list
					self.input="roomsize"
					if x >=39 and x <43:
						self.ChangeSize("-")
					elif x >=51 and x <=55:
						self.ChangeSize("+")
					else:
						self.Mode()
				
				elif y == 10 and x >=1 and x <=38:
					self.input="autoretry"
					self.ToggleAutoRetry()
					
				elif y ==11  and x >=1 and x <=38:
					# Toggle Autobuddy
					self.input="autobuddy"
					self.ToggleAutoBuddy()
				elif y == 12 and  x >=1 and x <=38:
					self.input="beep"
					self.mucous.ToggleBeep()
					
				elif y in (12, 13, 14) and x >=39 and x <=55:
					# Change charset, encoding, language that text is piped thru
					if "language" in self.mucous.Config["mucous"]:
						self.input="encoding"
						self.mucous.Config["mucous"]["language"]=self.mucous.FormatData.RotateList("right", self.mucous.encodings, self.mucous.Config["mucous"]["language"], "no")
						self.Mode()
			
				elif y in (15, 16, 17) and x >= 1 and x <= 55:
					if x >=1 and x <=38:
						# Edit custom URL handler process
						self.SetInput("url-custom")

					elif x >=39 and x <=55:
						self.input="url-reader"
						# Cycle thru list of URL handlers
						_list = ["lynx", "links", "elinks", "firefox", "custom"]
						self.mucous.Config["mucous"]["url reader"] = self.mucous.FormatData.RotateList("right", _list, self.mucous.Config["mucous"]["url reader"], "no")
						self.Mode()
					else:
						self.SetInput()
				elif y in (self.mucous.h-8, self.mucous.h-7, self.mucous.h-6):
					if x >= self.mucous.w-17:
					#self.mucous.h-8,self.mucous.w-17
						# Save
						self.input="save"
						self.mucous.config_manager.update_config()
						self.Mode()
					else:
						self.SetInput()
				else:
					self.SetInput()
			
			elif self.current=="museek" and "transfers" in self.mucous.config:
				if y in (2, 3) and x >=1 and x <=35:
					self.SetInput("server-host")
				elif y == 4 and x >=1 and x <=35:
					self.SetInput("server-port")
				elif y == 5 and x >=1 and x <=35:
					self.SetInput("soulseek-username")
				elif y == 6 and x >=1 and x <=35:
					self.SetInput("soulseek-password")
				elif y in (2, 3, 4) and x >=40 and x <=63:
					self.SetInput("museek-interface-password")
				elif y in (5,6,7) and x >=50 and x <=63:
					self.input="connectmode"
					self.ToggleConnectMode()
					return
				elif y==8 and x >=1 and x <=30:
					self.input="only_buddies"
					self.ToggleOnlyBuddies()
				elif y==9 and x >=1 and x <=30:
					self.input="privilege_buddies"
					self.TogglePrivilegeBuddies()
					
				elif y==10 and x >=1 and x <=30:
					self.input="have_buddy_shares"
					self.ToggleHaveBuddyShares()
					
				elif y==11 and x >=1 and x <=50:
					self.input="trusting_uploads"
					self.ToggleTrustedUploads()
					
				elif y==12 and x >=1 and x <=50:
					self.input="user_warnings"
					self.ToggleUserWarnings()
				
				elif y in (8,9,10)  and x >=49 and x <=64:
					self.input="slots"
					if x >=49 and x <=53:
						self.ChangeSize("-")
					elif x >=60 and x <=64:
						self.ChangeSize("+")
					else:
						self.Mode()

				elif y in (13, 14):
					# Download Directory
					self.SetInput("download-dir")
				elif y in (15, 16):
					# Incomplete Download Directory
					self.SetInput("incomplete-dir")
				else:
					self.SetInput()
			
			elif self.current=="shares":
				if y in ( 3, 4, 5):
					if x >= 1 and x <= 16:
						self.input="listnormal"
						self.mucous.Muscan.ListNormal()
					elif x >= 17 and x <= 32:
						self.input="adddir"
					elif x >= 33 and x < 49:
						self.input="rmdir"

				elif y in ( 6, 7, 8):
					if x >= 1 and x <= 16:
						self.input="rescannormal"
						self.mucous.Muscan.RescanNormal()
					elif x >= 17 and x <= 32:
						self.input="updatenormal"
						self.mucous.Muscan.UpdateNormal()
				elif y in ( 9, 10, 11):
					if x >= 1 and x <= 16:
						self.input="listbuddy"
						self.mucous.Muscan.ListBuddy()
					elif x >= 17 and x <= 32:
						self.input="addbuddydir"
					elif x >= 33 and x < 49:
						self.input="rmbuddydir"
				elif y in ( 12, 13, 14):
					if x >= 1 and x <= 16:
						self.input="rescanbuddy"
						self.mucous.Muscan.RescanBuddy()
					elif x >= 17 and x <= 32:
						self.input="updatebuddy"
						self.mucous.Muscan.UpdateBuddy()
					elif x >= 49 and x <= 66:
						self.input="reloadshares"
						self.mucous.D.ReloadShares()
				else:
					self.SetInput()
					return
				self.Mode()
				
			elif self.current=="userinfo":
				if y > 1 and y < self.mucous.h-7 and x < self.mucous.w-17:
					self.SetInput("userinfo")
				elif y <= self.mucous.h-5 and y >= self.mucous.h-7:
					self.SetInput("userimage")
				else:
					self.SetInput()
				
			elif self.current=="logs":
				if y >= 2 and y <= 3 and x < 37:
					self.input="logging"
					self.mucous.ToggleLogging()
					self.Mode()
				elif y >= 5 and y <= 7 :
					self.SetInput("logdir")
				elif y in (self.mucous.h-8, self.mucous.h-7, self.mucous.h-6):
					if x >= self.mucous.w-17:
						# Save
						self.input="save"
						self.mucous.config_manager.update_config()
						self.Mode()
					else:
						self.SetInput()
				else:
					self.SetInput()
					
			
			
					
		except Exception, e:
			self.mucous.Help.Log("debug", "Setup.Mouse: " +str(e) )
		
		
	def SetInput(self, input=None):
		if input == None:
			self.input = "default"
			self.mucous.UseAnotherEntryBox()
		else:
			self.input = input
		self.Mode()
		#self.InputTitle()
		

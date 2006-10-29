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
		self.windows = {"border":{}, }
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
		
	## Reset input to 'default' and redraw setup
	# @param self Setup (class)
	def Default(self):
		try:
			self.input = "default"
			self.Mode()
		except Exception, e:
			self.mucous.Help.Log("debug", "Setup.Default: "+ str(e))
			
	## Create window, display contents
	# @param self is Setup (Class)
	def Mode(self):
		
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
				#self.mucous.set_edit_title("Mucous Setup")
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
		self.mucous.ModeTopbar()
		self.mucous.HotKeyBar()
		curses.doupdate()
		
	## Update input line's title
	# @param self is Setup (Class)
	def InputTitle(self):
		si = self.input
		if si == "default":
			self.mucous.set_edit_title("Setup Mode")
		elif si == "interface":
			self.mucous.set_edit_title("Set Interface")
		elif si == "custom-url":
			self.mucous.set_edit_title("Set custom URL handler: command$command")
		elif si == "interface-password":
			self.mucous.set_edit_title("Set Mucous's Interface password")
		elif si == "museek-interface-password":
			self.mucous.set_edit_title("Set Museek's Interface password")
		elif si == "museek-interface-bind":
			self.mucous.set_edit_title("Add a Museek Interface")
		elif si == "server-host":
			self.mucous.set_edit_title("Set Server Address")
		elif si == "server-port":
			self.mucous.set_edit_title("Set Server Port")
		elif si == "soulseek-username":
			self.mucous.set_edit_title("Set Soulseek Username")
		elif si == "soulseek-password":
			self.mucous.set_edit_title("Set Soulseek Password")
		elif si == "slots":
			self.mucous.set_edit_title("Set Number of Upload Slots to:")
		elif si == "download-dir":
			self.mucous.set_edit_title("Set completed download directory")
		elif si == "incomplete-dir":
			self.mucous.set_edit_title("Set incompleted download directory")
		elif si == "userinfo":
			self.mucous.set_edit_title("Set UserInfo")
		elif si == "userimage":
			self.mucous.set_edit_title("Set UserImage")
		elif si == "adddir":
			self.mucous.set_edit_title("Add directory to your normal shares")
		elif si == "rmdir":
			self.mucous.set_edit_title("Remove directory from your normal shares")
		elif si == "addbuddydir":
			self.mucous.set_edit_title("Add directory to your buddy shares")
		elif si == "rmbuddydir":
			self.mucous.set_edit_title("Remove directory from your buddy shares")
			
	## Parse input entry line for interest and match it with Setup.input
	# @param self is Setup (Class)
	# @param line is a text string
	def InputSetup(self, line):
		try:
			line = self.mucous.dlang(line)
			if self.input == "interface":
				self.mucous.Config["connection"]["interface"] = line
			elif self.input=="interface-password":
				self.mucous.Config["connection"]["passw"] = line
			elif self.input=="custom-url":
				if "$" in line:
					custom = line.split("$")
					if len(custom) > 1 and len(custom) < 3:
						self.mucous.Config["mucous"]["url custom prefix"] = custom[0]
						self.mucous.Config["mucous"]["url custom suffix"] = custom[1]
					elif len(custom) == 1:
						self.mucous.Config["mucous"]["url custom prefix"] = custom[0]
						self.mucous.Config["mucous"]["url custom suffix"] = ''
			elif self.input=="museek-interface-password":
				
				self.mucous.D.ConfigSet("interfaces", "password", line)
			elif self.input=="museek-interface-bind":
				
				self.mucous.D.ConfigSet("interfaces.bind", line)
			elif self.input=="server-host":
				
				self.mucous.D.ConfigSet("server", "host", line)
			elif self.input=="server-port":
				
				self.mucous.D.ConfigSet("server", "port", line)
			elif self.input=="soulseek-username":
				self.mucous.D.ConfigSet("server", "username", line)
			elif self.input=="soulseek-password":
				self.mucous.D.ConfigSet("server", "password", line)
			elif self.input=="download-dir":
				self.mucous.D.ConfigSet("transfers", "download-dir", line)
			elif self.input=="incomplete-dir":
				
				self.mucous.D.ConfigSet("transfers", "incomplete-dir", line)
			elif self.input=="userinfo":
				
				try:
					if '\\n' in line:
						line = line.replace('\\n', '\n')
					self.mucous.D.ConfigSet("userinfo", "text", line)
				except Exception, e:
					self.mucous.Help.Log("debug", "set userinfo: "+str( e))
			elif self.input=="userimage":
				
				try:
					self.mucous.D.ConfigSet("userinfo", "image", line)
				except:
					pass
			elif self.input=="adddir":
				self.mucous.Muscan.Command(["muscan", "-s", line])
				self.mucous.Help.Log("status", "Adding "+line+" to normal shares. Scanning will begin.")
			elif self.input=="rmdir":
				self.mucous.Muscan.Command(["muscan", "-u", line])
				self.mucous.Help.Log("status", "Removing "+line+" from normal shares. Please rescan or update.")
			elif self.input=="addbuddydir":
				
				self.mucous.Muscan.Command(["muscan", "-b", "-s", line])
				self.mucous.Help.Log("status", "Adding "+line+" to buddy shares. Scanning will begin.")
			elif self.input=="rmbuddydir":
				self.mucous.Muscan.Command(["muscan", "-b", "-u", line])
				self.mucous.Help.Log("status", "Removing "+line+" from buddy shares. Please rescan or update.")
			self.input = "default"
			self.Mode()
		except Exception,e:
			self.mucous.Help.Log("debug", "InputSetup: " + str(e))
			
	## Draw Mucous settings
	# @param self is Setup (Class)
	def DrawMucous(self):
		w = self.dimensions["setup"]
		mw = self.windows["border"]["setup"]
		# Interface
		interface =""
		if "connection" in self.mucous.Config:
			interface = self.mucous.dlang( self.mucous.Config["connection"]["interface"][:28] )
		
		self.SetupButton("Museek Interface", interface ,2,1, 3,32)
		
		
		if "connection" in self.mucous.Config:
			if self.password == True:
				password = self.mucous.dlang( str(self.mucous.Config["connection"]["passw"]) )
			else:
				password = '*********'
		else:
			password = "NOT set"
			
		self.SetupButton("Interface Password", password, 5,1,3,32)
		
		bwin = curses.newwin(6,22,2,33)
		bwin.border()
		try:
			bwin.addstr(0, 1, "< Stats >",  curses.A_BOLD)
		except:
			pass
		bwin.noutrefresh()
		inbwin1_1 = bwin.subwin(1,20,3,34)
		inbwin1_2 = bwin.subwin(1,20,4,34)
		inbwin1_3 = bwin.subwin(1,20,5,34)
		inbwin1_4 = bwin.subwin(1,20,6,34)
		inbwin1_1.scrollok(1)
		inbwin1_2.scrollok(1)
		inbwin1_3.scrollok(1)
		inbwin1_4.scrollok(1)
		if self.mucous.data["mystats"] != []:
			try:
				inbwin1_1.addstr("Files: "+str(self.mucous.data["mystats"][3]), self.mucous.colors["cyan"] )
				inbwin1_2.addstr("Dirs: "+str(self.mucous.data["mystats"][4]), self.mucous.colors["cyan"] )
				inbwin1_3.addstr("Downloads: "+str(self.mucous.data["mystats"][2]), self.mucous.colors["cyan"] )
				inbwin1_4.addstr("Speed: "+str(self.mucous.data["mystats"][1]/1024)+"KB/s", self.mucous.colors["cyan"] )
				#self.data["mystats"] = user,  avgspeed, numdownloads, numfiles, numdirs
			except:
				pass
		else:
			if self.mucous.username != None:
				self.mucous.D.PeerStats(self.mucous.username)
		inbwin1_1.noutrefresh()
		inbwin1_2.noutrefresh()
		inbwin1_3.noutrefresh()
		inbwin1_4.noutrefresh()
		del inbwin1_1
		del inbwin1_2
		del inbwin1_3
		del inbwin1_4 
		del bwin
		
		self.SetupCheck("Show Tickers:         /showtickers", self.mucous.Config["tickers"]["tickers_enabled"] ,8,1,1,45, True)

		self.SetupCheck("Cycle Tickers:        /tickers", self.mucous.Config["tickers"]["ticker_cycle"] ,9,1,1,45, True, True)
		self.SetupCheck("Scroll Tickers:       /tickers", self.mucous.Config["tickers"]["ticker_scroll"] ,10,1,1,45, True, True)
		self.SetupCheck("Auto-clear Transfers: /autoclear", self.mucous.Config["mucous"]["auto-clear"],11,1,1,45, True)

		self.SetupCheck("Auto-retry Transfers: /autoretry", self.mucous.Config["mucous"]["auto-retry"],12,1, 1,45, True)

		self.SetupCheck("Auto-Buddy downloads: /autobuddy", self.mucous.Config["mucous"]["autobuddy"],13,1,1,45, True)
		
		self.SetupCheck("Beep:                 /beep", self.mucous.Config["mucous"]["beep"],14,1,1,45, True)
		self.SetupButton("< Ticker Time >", " -  %.2f +\n -  %.2f +" % ( float(self.mucous.Config["tickers"]["cycletime"]), float(self.mucous.Config["tickers"]["scrolltime"])), 8,39, 4,16, False, False)
		# Minimum Room size for Roomlist
		
		self.SetupButton("<Min Roomlist>", " -  "+str(self.mucous.Config["mucous"]["roomlistminsize"])+" +", 8,55, 3,16, False, False)

		self.SetupButton("Encoding", self.mucous.Config["mucous"]["language"], 12,39,3,16)
		
		# Custom URL
		prefix =""
		if "url custom prefix" in self.mucous.Config["mucous"] and "url custom suffix" in self.mucous.Config["mucous"]:
			prefix = self.mucous.dlang(self.mucous.Config["mucous"]["url custom prefix"])+"$"+self.mucous.dlang(self.mucous.Config["mucous"]["url custom suffix"])
		self.SetupButton("Custom URL Reader", prefix,15,1,3,32, optionbold=True)
		self.SetupButton("URL Reader", self.mucous.dlang(self.mucous.Config["mucous"]["url reader"]), 15,33,3,16)


		# Save button
		self.SetupButton(None, " Save Config", 15,49,3,16, True)
		
	## Draw Museek settings
	# @param self is Setup (Class)
	def DrawMuseek(self):
		w = self.dimensions["setup"]
		mw = self.windows["border"]["setup"]
		# Server
		bw = curses.newwin(5,38,2,1)
		bw.border()
		bw.addstr(0, 1, "< Server >",  curses.A_BOLD)
			
		bw.noutrefresh()
		inbw = bw.subwin(1,36,3,2)
		inbw.scrollok(1)
		inbw.addstr("Host: ")
		if "server" in self.mucous.config:
			try:
				inbw.addstr(self.mucous.dlang( self.mucous.config["server"]["host"][:22] )+":"+self.mucous.config["server"]["port"], self.mucous.colors["cyan"] )
			except:
				pass
		inbw.noutrefresh()
		del inbw
		

		
		inusernamewin2 = bw.subwin(1,36,4,2)
		inusernamewin2.scrollok(1)
		inusernamewin2.addstr("Name: ")
		if "server" in self.mucous.config:
			a = self.mucous.dlang(self.mucous.config["server"]["username"])
			try:
				inusernamewin2.addstr(a, self.mucous.colors["cyan"])
			except:
				pass

		inusernamewin2.noutrefresh()
		del inusernamewin2

		inifacepasswin = bw.subwin(1,36,5,2)
		inifacepasswin.addstr("Pass: ")
		if self.password == True:
			if "server" in self.mucous.config:
				inifacepasswin.addstr(self.mucous.dlang( self.mucous.config["server"]["password"] ), self.mucous.colors["cyan"])
				
		else:
			if "server" in self.mucous.config:
				inifacepasswin.addstr("*********", self.mucous.colors["cyan"])
		inifacepasswin.noutrefresh()
		del inifacepasswin
		del bw

		ifacepasswin = curses.newwin(3,26,2,39)
		ifacepasswin.border()
		ifacepasswin.addstr(0, 1, "< Interface Password >",  curses.A_BOLD)

		ifacepasswin.noutrefresh()
		inifacepasswin = ifacepasswin.subwin(1,24,3,40)
		inifacepasswin.scrollok(1)
		if "interfaces" in self.mucous.config:
			if self.password == True:
				try:
					inifacepasswin.addstr(self.mucous.dlang( self.mucous.config["interfaces"]["password"] ), self.mucous.colors["cyan"])
				except:
					pass
			else:
				inifacepasswin.addstr("*********", self.mucous.colors["cyan"])
		inifacepasswin.noutrefresh()
		del inifacepasswin

		#------------------------

		self.SetupCheck("Share to Buddies-Only", self.mucous.config["transfers"]["only_buddies"],8,1,1,30, True)

		self.SetupCheck("Buddies get Privileges", self.mucous.config["transfers"]["privilege_buddies"],9,1,1,30, True)

		self.SetupCheck("Enable Buddy-Only shares",self.mucous.config["transfers"]["have_buddy_shares"],10,1,1,30, True)

		self.SetupCheck("Allow Trusted users to send you files",self.mucous.config["transfers"]["trusting_uploads"],11,1,1,self.mucous.w-3, True)

		self.SetupCheck("Send automatic warnings via Private Chat",self.mucous.config["transfers"]["user_warnings"],12,1,1,self.mucous.w-3, True)

		self.SetupButton("<Connections >", self.mucous.config["clients"]["connectmode"],5,49,3,16, True, False)

		self.SetupButton("<Upload Slots>", " - " + self.mucous.config["transfers"]["upload_slots"]+ " +", 8,49,3,16, True, False)

		dirwin = curses.newwin(4,self.mucous.w-2,13,1)
		dirwin.border()
		dirwin.addstr(0, 1, "< Download/Incomplete Directories >",  curses.A_BOLD)
		dirwin.noutrefresh()
		dircompletewin = dirwin.subwin(1,self.mucous.w-4,14,2)
		try:
			dircompletewin.addstr(self.mucous.dlang(self.mucous.config["transfers"]["download-dir"]), self.mucous.colors["cyan"])
		except: pass
		dircompletewin.noutrefresh()
		dirincompletewin = dirwin.subwin(1,self.mucous.w-4,15,2)
		try:
			dirincompletewin.addstr(self.mucous.dlang(self.mucous.config["transfers"]["incomplete-dir"]), self.mucous.colors["cyan"])
		except: pass
		dirincompletewin.noutrefresh()
		del dirincompletewin
		del dircompletewin
		del dirwin
		
	## Draw Shares config options
	# @param self is Setup (Class)
	def DrawShares(self):
		w = self.dimensions["setup"]
		mw = self.windows["border"]["setup"]
		# First Row
		self.SetupButton("Normal", "List Shared", 3,1,3,16)
		self.SetupButton(None, "Add Directory", 3,17,3,16)
		self.SetupButton(None, "Remove Dir", 3,33,3,16)
		self.SetupButton(None, "Click on the buttons to run the local muscan.\nWhen you click on Add/Remove Directory, type in the directory below, and start with '//' instead of just a '/'", 2,49,10,self.mucous.w-50)
		# Second Row
		self.SetupButton(None, "Rescan Shares",6,1,3,16)
		self.SetupButton(None, "Update Shares", 6,17,3,16)
		
		# Third Row
		self.SetupButton("Buddy-only", "List Shared", 9,1,3,16)
		self.SetupButton(None, "Add Directory", 9,17,3,16)
		self.SetupButton(None, "Remove Dir", 9,33,3,16)
		
		# Fourth Row
		self.SetupButton(None, "Rescan Shares", 12,1,3,16)
		self.SetupButton(None, "Update Shares", 12,17,3,16)
		self.SetupButton(None, "Reload Shares", 12,49,3,16, optionbold=True)
		
		museekconfigfile = self.mucous.Spl["museekconfigfile"]
		# Fifth Row
		self.SetupButton("Museek Config File", museekconfigfile[:self.mucous.w-2], 15,1,3,self.mucous.w-2)
		
		
	## Draw Log file options
	# @param self is Setup (Class)
	def DrawLogs(self):
		w = self.dimensions["setup"]
		mw = self.windows["border"]["setup"]
		self.SetupCheck("Log Chat messages? ", self.mucous.Config["mucous"]["logging"],2,1,1,30, True)
		self.SetupButton("Log Directory", os.path.expanduser(self.mucous.Config["mucous"]["log_dir"])[:37], 5,1,3,self.mucous.w-2)
	## Draw User Info 
	# @param self is Setup (Class)
	def DrawUserinfo(self):
		w = self.dimensions["setup"]
		mw = self.windows["border"]["setup"]
		info = ""
		if "userinfo" in self.mucous.config.keys():
			if "text" in self.mucous.config["userinfo"]:
				info = self.mucous.config["userinfo"]["text"]
				#for line in self.mucous.config["userinfo"]["text"]:
					#info += line.decode(self.mucous.Config["mucous"]["language"], "replace") +"\n"
		
		self.SetupButton("Your Userinfo", info, 2,1,self.mucous.h-8,self.mucous.w-2)
		
		inputimage = curses.newwin(1,13,self.mucous.h-6,1)
		inputimage.erase()
		inputimage.addstr( "Your Image: ")
		inputimage.noutrefresh()
		
		if "userinfo" in self.mucous.config.keys():
			if "image" in self.mucous.config["userinfo"]:
				inputimage2 = curses.newwin(1,self.mucous.w-2-14,self.mucous.h-6,14)
				inputimage2.erase()
				inputimage2.scrollok(1)
				inputimage2.addstr( str(self.mucous.config["userinfo"]["image"][:self.mucous.w-3-14]))
				inputimage2.noutrefresh()
		del inputimage2
		del inputimage
			
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
	# @param border
	def SetupButton(self, title, option, x, y, height, width, optionbold=False, titlepad=True, border=True):
		try:
			winborder = curses.newwin(height,width,x,y)
			if border:
				winborder.border()
			if title != None:
				if titlepad:
					winborder.addstr(0,1, "< %s >" % title, curses.A_BOLD)
				else:
					winborder.addstr(0,1, title, curses.A_BOLD)
			winborder.noutrefresh()
			win = winborder.subwin(height-2,width-2,x+1,y+1)
			win.scrollok(1)
			if option != None:
				if optionbold:
					win.addstr(option, self.mucous.colors["cyan"] | curses.A_BOLD)
				else:
					win.addstr(option, self.mucous.colors["cyan"])
			win.noutrefresh()
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
	def SetupCheck(self, title, option, x, y, height, width, titlebold=False, toggle=False):
		try:
			enabled = "[x]"
			disabled = "[ ]"
			selected = "(*)"
			notselected = "( )"
			
			win = curses.newwin(height,width,x,y)
			win.erase()
			if option in ("True", "true", "yes"):
				if toggle:
					z = selected
				else:
					z = enabled
			else:
				if toggle:
					z = notselected
				else:
					z = disabled
			win.addstr(z+" ", self.mucous.colors["cyan"])
			if title != None:
				if titlebold:
					win.addstr(title,  curses.A_BOLD)
				else:
					win.addstr(title)
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
			
			if y in (0, 1):
				if x >=1 and x <=12:
					if self.current != "mucous":
						self.current="mucous"
						self.input = "default"
				elif x >=16 and x <=26:
					if self.current != "museek":
						self.current="museek"
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
				self.Mode()

			if self.current=="shares":
				if y in ( 3, 4, 5):
					if x >= 1 and x <= 16:
						self.mucous.Muscan.ListNormal()
					elif x >= 17 and x <= 32:
						self.mucous.Muscan.ChangeInput("adddir")
					elif x >= 33 and x < 49:
						self.mucous.Muscan.ChangeInput("rmdir")

				if y in ( 6, 7, 8):
					if x >= 1 and x <= 16:
						self.mucous.Muscan.RescanNormal()
					elif x >= 17 and x <= 32:
						self.mucous.Muscan.UpdateNormal()
				elif y in ( 9, 10, 11):
					if x >= 1 and x <= 16:
						self.mucous.Muscan.ListBuddy()
					elif x >= 17 and x <= 32:
						self.mucous.Muscan.ChangeInput("addbuddydir")
					elif x >= 33 and x < 49:
						self.mucous.Muscan.ChangeInput("rmbuddydir")
				elif y in ( 12, 13, 14):
					if x >= 1 and x <= 16:
						self.mucous.Muscan.RescanBuddy()
					elif x >= 17 and x <= 32:
						self.mucous.Muscan.UpdateBuddy()
					elif x >= 49 and x <= 66:
						self.mucous.D.ReloadShares()
			elif self.current=="userinfo":
				if y >= 1 and y < self.mucous.h-7:
					self.input="userinfo"
				elif y <= self.mucous.h-5 and y >= self.mucous.h-7:
					self.input="userimage"
				self.Mode()
			elif self.current=="logs":
				if y >= 2 and y <= 3 and x < 37:
					self.mucous.ToggleLogging()
					self.Mode()
				
					
			if self.current not in ("museek", "mucous"):
				return
			if y not in (2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17):
				return
			if self.current=="museek" and "transfers" in self.mucous.config:
				if y in (2, 3) and x >=1 and x <=35:
					self.input="server-host"
					self.Mode()
				elif y == 4 and x >=1 and x <=35:
					self.input="soulseek-username"
					self.Mode()
				elif y == 5 and x >=1 and x <=35:
					self.input="soulseek-password"
					self.Mode()
				elif y in (2, 3, 4) and x >=40 and x <=63:
					self.input="museek-interface-password"
					self.Mode()
				elif y in (5,6,7) and x >=50 and x <=63:
					# Connect Mode
					if "clients" in self.mucous.config:
						
						if self.mucous.config["clients"]["connectmode"] == "passive":
							self.mucous.D.ConfigSet("clients", "connectmode", "active")
						elif self.mucous.config["clients"]["connectmode"] == "active":
							self.mucous.D.ConfigSet("clients", "connectmode", "passive")
							
				elif y==8 and x >=1 and x <=30:
					#Buddies-only
					
					if self.mucous.config["transfers"]["only_buddies"] == "true":
						self.mucous.D.ConfigSet("transfers", "only_buddies", "false")
					elif self.mucous.config["transfers"]["only_buddies"] == "false":
						self.mucous.D.ConfigSet("transfers", "only_buddies", "true")
				elif y==9 and x >=1 and x <=30:
					
					if self.mucous.config["transfers"]["privilege_buddies"] == "true":
						self.mucous.D.ConfigSet("transfers", "privilege_buddies", "false")
							
					elif self.mucous.config["transfers"]["privilege_buddies"] == "false":
						self.mucous.D.ConfigSet("transfers", "privilege_buddies", "true")
				elif y==10 and x >=1 and x <=30:
					if self.mucous.config["transfers"]["have_buddy_shares"] == "true":
						self.mucous.D.ConfigSet("transfers", "have_buddy_shares", "false")
							
					elif self.mucous.config["transfers"]["have_buddy_shares"] == "false":
						self.mucous.D.ConfigSet("transfers", "have_buddy_shares", "true")
				elif y==11 and x >=1 and x <=50:
					if self.mucous.config["transfers"]["trusting_uploads"]== "true":
						self.mucous.D.ConfigSet("transfers", "trusting_uploads", "false")
							
					elif self.mucous.config["transfers"]["trusting_uploads"] == "false":
						self.mucous.D.ConfigSet("transfers", "trusting_uploads", "true")
				elif y==12 and x >=1 and x <=50:
					if self.mucous.config["transfers"]["user_warnings"] == "true":
						self.mucous.D.ConfigSet("transfers", "user_warnings", "false")
							
					elif self.mucous.config["transfers"]["user_warnings"] == "false":
						self.mucous.D.ConfigSet("transfers", "user_warnings", "true")
				
				elif y in (8,9,10)  and x >=49 and x <=64:
					s = int(self.mucous.config["transfers"]["upload_slots"]) 
					if x >=49 and x <=53:
						s  -= 1
					elif x >=54 and x <=64:
						s  += 1
					if s < 0:
						s = 0
					s = str(s)
					self.mucous.D.ConfigSet("transfers", "upload_slots", s)
					self.Mode()
				elif y in (13, 14):
					# Download Directory
					self.input="download-dir"
					self.Mode()
				elif y in (15, 16):
					# Incomplete Download Directory
					self.input="incomplete-dir"
					#if x >=1 and x <=61:
					self.Mode()
					
			elif self.current=="mucous":
				if y in (2, 3, 4):
					if x >=1 and x <=35:
						self.input="interface"
						self.Mode()
				elif y in (5, 6, 7):
					if x >=1 and x <=35:
						self.input="interface-password"
					self.Mode()
				elif y == 8 and x >=1 and x <=40:
					self.mucous.ChatRooms.ToggleTickersDisplay()
				elif y in (9, 10) and  x >=1 and x <=38:
					self.mucous.ChatRooms.ToggleTickers()
						
				elif y == 11 and x >=1 and x <=38:
					if self.mucous.Config["mucous"]["auto-clear"] == "yes":
						self.mucous.Config["mucous"]["auto-clear"] = "no"
						self.mucous.clear_timer.cancel()
						self.mucous.clear_timer = threading.Timer(30.0, self.mucous.ThreadTransfersClear)
						
					else:
						self.mucous.Config["mucous"]["auto-clear"] ="yes"
						self.mucous.clear_timer.cancel()
						self.mucous.clear_timer = threading.Timer(30.0, self.mucous.ThreadTransfersClear)
						self.mucous.clear_timer.start()
					self.Mode()
				elif y in (8, 9, 10, 11) and x >= 39 and x <= 54:
					# Minimum size of rooms displayed in room list
					
					if x >=39 and x <=47:
						
						if y in (8, 9):
							
							self.mucous.Config["tickers"]["cycletime"] = str(float (self.mucous.Config["tickers"]["cycletime"])-0.5) 
						elif y in (10,11):
							
							self.mucous.Config["tickers"]["scrolltime"] = str(float( self.mucous.Config["tickers"]["scrolltime"])-0.1)
					elif x >=48 and x <=54:
						if y in (8, 9):
							
							self.mucous.Config["tickers"]["cycletime"] = str( float( self.mucous.Config["tickers"]["cycletime"])+0.5)
						elif y in (10,11):
							self.mucous.Config["tickers"]["scrolltime"] = str(float( self.mucous.Config["tickers"]["scrolltime"])+0.1)
					if float(self.mucous.Config["tickers"]["scrolltime"]) < 0.1:
						self.mucous.Config["tickers"]["scrolltime"] = str(0.1)
					if float(self.mucous.Config["tickers"]["cycletime"]) < 1.0:
						self.mucous.Config["tickers"]["cycletime"] = str(1.0)
					self.Mode()
				elif y in (8, 9, 10) and x >= 55 and x <= 71:
					# Minimum size of rooms displayed in room list
					if x >=55 and x <=61:
						self.mucous.Config["mucous"]["roomlistminsize"] -= 1
					elif x >=62 and x <=71:
						self.mucous.Config["mucous"]["roomlistminsize"] += 1
					if self.mucous.Config["mucous"]["roomlistminsize"] < 1:
						self.mucous.Config["mucous"]["roomlistminsize"] = 1
					self.Mode()
				
				elif y == 12 and x >=1 and x <=40:
					if str(self.mucous.Config["mucous"]["auto-retry"]) == "yes":
						self.mucous.Config["mucous"]["auto-retry"] = "no"
						self.mucous.retry_timer.cancel()
					else:
						self.mucous.Config["mucous"]["auto-retry"] ="yes"
						self.mucous.retry_timer.cancel()
						self.mucous.retry_timer = threading.Timer(30.0, self.mucous.ThreadTransfersRetry)
						self.mucous.retry_timer.start()
					self.Mode()
					
				elif y ==13  and x >=1 and x <=40:
					# Toggle Autobuddy
					if self.mucous.Config["mucous"]["autobuddy"]  == "yes":
						self.mucous.Config["mucous"]["autobuddy"] = "no"

					elif self.mucous.Config["mucous"]["autobuddy"]  == "no":
						self.mucous.Config["mucous"]["autobuddy"] = "yes"
					self.Mode()
				elif y == 14 and  x >=1 and x <=40:
					self.mucous.ToggleBeep()

					
				elif y in (12, 13, 14) and x >=39 and x <=55:
					# Change charset, encoding, language that text is piped thru
					if "language" in self.mucous.Config["mucous"]:
						if self.mucous.Config["mucous"]["language"] in self.mucous.encodings:
							pos = self.mucous.encodings.index(self.mucous.Config["mucous"]["language"])
							pos +=1
							if pos not in range(len(self.mucous.encodings)):
								pos = 0
							self.mucous.Config["mucous"]["language"]=self.mucous.encodings [pos]
							self.Mode()
						else:
							self.mucous.Config["mucous"]["language"]=self.mucous.encodings [0]
							self.Mode()
			
				elif y in (15, 16, 17):
					if x >=1 and x <=32:
						# Edit custom URL handler process
						self.input="custom-url"
						self.Mode()
					elif x >=35 and x <=49:
						# Cycle thru list of URL handlers
						u = self.mucous.Config["mucous"]["url reader"]
						if u == 'lynx': u = 'links'
						elif u == 'links': u = 'elinks'
						elif u == 'elinks': u = 'firefox'
						elif u == 'firefox': u = 'custom'
						elif u == 'custom': u = 'lynx'
						self.mucous.Config["mucous"]["url reader"] = u
						self.Mode()
					elif x >=50 and x <=65:
						self.mucous.config_manager.update_config()
						self.mucous.Help.Log("status", "Config Saved")
		except Exception, e:
			self.mucous.Help.Log("debug", "Setup.Mouse: " +str(e) )
		
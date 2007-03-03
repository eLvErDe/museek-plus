# This is part of the Mucous Museek Client, and distributed under the GPLv2
# Copyright (c) 2006 daelstorm. 
import curses.wrapper
#import traceback
import time
import sys
## Help logs and windows
#
class Help:
	
	## Constructor
	# @param self Help (Class)
	# @param mucous Mucous (Class)
	def __init__(self, mucous):
		## @var mucous
		# Mucous (Class)
		self.mucous = mucous
		## @var scrolling
		# dict of vertical scroll positions for help, debug
		self.scrolling = {"help":0, "debug":0}
		## @var dimensions
		# Window placement
		self.dimensions = {}
		## @var windows
		# Curses Window instances
		self.windows = {}
		## @var log
		# dict containing help log lists
		self.log = {}
		self.log["private"] = ["Global Private Messaging commands:",\
"To start a Private Message:",\
"1) Type in the username you wish to PM below",\
"2) Press <Enter>",\
"3) Type your messages",
"Use the commands below:",\
"/pm      <user>        (Start PM or switch to user)",\
"/close   <user>        (Close user's PM) ",\
"/close                 (Close current PM)",\
"/ip                    (Get the IP of current user)",\
"/msg <message>         (Send message to previous chosen user)"]
		
		self.log["userinfo"] = ["Global User Information commands:",\
"/userinfo <user>",\
"/stat     <user>",\
"/ip       <user>",\
"Or type the user name that you wish get recieve userinfo from, below.", "--"]
		self.log["chat"] = ["----[Chat Commands]----", \
"/join <room>   /part <room>  /leave <room>",\
"/j <room>      /talk <room>  /say <room> <message>",\
"/users <room>          (lists of users in room)",\
"/autojoin <room>       (Toggle Autojoining room)",\
"/roomlistrefresh       (redownload roomlist from server)",\
"/inrooms               (list of joined rooms)",\
"/clearroom <room>      (clear <room>, or current room)",\
"/pm <user>             (Private Message user)",\
"/msg <message>         (send message to last user)",\
"/url          /urlcustom (command$command)    (Requires X11)",\
"/urlreader (lynx|links|elinks|firefox|custom) (Requires X11)",\
"/np                    (XMMS/BMP Now playing script)",\
"/npcheck               (display Now playing script command)",\
"/npset <command>       (Set Now playing script to command)",\
"/npprefix <text>       (set to np: or /me is now playing:)",\
"/alias <alias> <text>  /unalias <alias>"]
		self.log["tickers"] = ["----<Ticker Commands>---- ",\
"/showtickers           (Hide/Show tickers)",\
"/tickers               (Scroll or Cycle through tickers)",\
"/listtick <room>       (List tickers in the Info mode)",\
"/tickroom <room>       (Choose room for setting tickers)",\
"/settick <message>     (Set ticker to message, and save it)",\
"/defaulttick <message> (Set & save default ticker)",\
"/settemptick <message> (Set ticker only for this session)"]
		self.log["connect"] = ["Connection Configuration",\
"/interface </tmp/museekd.[username]> or <host:port>",\
"/password <interface password>",\
"/connect               (Attempts to connect to Museekd)",\
"/disconnect            (Disconnects from Museekd)",\
"/login                 (Login to Soulseek Server)",\
"/logout                (Logout from Soulseek Server)",\
"/save                  (Writes settings to config)"]

		self.log["setup"] = ["Setup",\
"/autobuddy    (Auto-buddy users you download from)",\
"/autoclear    (auto-clear finished uploads)",\
"/autoretry    (auto-retry failed/errored transfers)",\
"/password     (Mucous' Interface password)",\
"/privbuddy    (Toggle Privileging buddies)",\
"/onlybuddy    (Toggle Only sharing to buddies)",\
"/slots <num>  (Set upload slots)",\
"/unhide       (Toggle showing password)",\
"/rescan       (Rescan Normal Shares)",\
"/rescanbuddy  (Rescan Buddy-only Shares)",\
"/reload       (Reload Shares - make changes take effect)",\
"/extra        (Auto-Send Version info via PM if requested)",\
"/logdir /path (Set Logs Directory)",\
"/logging      (Toggle Logging)"]

		self.log["user"] = ["----[User Commands]---- ",\
"/buddy    <user>    /unbuddy   <user>",\
"/ban      <user>    /unban     <user> (Cannot access your files)",\
"/ignore   <user>    /unignore  <user> (No messages in chat)",\
"/nuke     <user>    /unnuke    <user> (Ban+Ignore)",\
"/trust    <user>    /distrust  <user> (Can Upload to you)",\
"/stat     <user>    /ip        <user>",\
"/userinfo <user>    /giveprivs <user> (Input days next)",\
"/away (Toggle your Online/Away Status)",\
"/autoaway (Go away after 900 seconds of inactivity)"]

		self.log["transfer"] = ["----[Transfer Commands]---- ",\
"/abortu  <num> /abortup    <num> (Abort Upload)",\
"/abortd  <num> /abortdown  <num> (Abort Download)",\
"/removeu <num> /removeup   <num> (Remove Upload)",\
"/removed <num> /removedown <num> (Remove Download)",\
"/retry   <num>                   (Retry Download)",\
"/retryall                        (Retry all Downloads)",\
"/clearup                         (Clear failed/finished Uploads)",\
"/cleardown                       (Clear finished Download)",\
"/percent                         (Toggle Percent/Speed",\
"/transbox                        (Toggle Split/Tabbed Mode))"]
		self.log["modes"] = ["----[Mode Commands]---- ",\
"/chat          (Chat Mode)",\
"/transfer      (Transfer Mode)",\
"/info          (Info Mode)",\
"/browse        (Browse Mode)",\
"/private       (Private Message Mode)",\
"/search        (Search Mode)",\
"/buddylist     (Buddylist Mode)",\
"/banlist       (Banlist Mode)",\
"/ignorelist    (Ignorelist Mode)",\
"/roomlist      (Roomlist Mode)",\
"/setup         (Setup Mode)",\
"/help /debug   (Help & Debug Mode)"]

		self.log["helpcommands"] = ["----<Help Commands>---- ",\
"/help          (This Message)",\
"/help connect  (Connection Commands)",\
"/help setup    (Setup Commands)",\
"/help mode     (Mode Commands)",\
"/help chat     (Chatroom Commands)",\
"/help ticker   (Ticker Commands)",\
"/help user     (User Commands)",\
"/help transfer (Transfer Commands)",\
"/help browse   (Browse Commands)",\
"/help search   (Search Commands)",\
"/help download (Download Commands)",\
"/help keys     (Special Keys)",\
"/quit          (Close Mucous)"]
		
		
		self.log["search"] = [\
"/search                (Switch to Search Mode)",\
"/searchfor   <query>   (Global Search)",\
"/searchbuddy <query>   (Buddy Search)",\
"/searchroom  <query>   (Room Search)",\
"/searchuser  <user>    (Search only one user's shares)",\
"/download <number>     (Download File with number)",\
"/downdir  <number>     (Download directory of File)",\
"/close                 (Close current search)",\
"/clearsearchs          (Removes all searches)",\
"/filter <string>       (limit displayed files)",\
"Mouse: Right-Click     (popup menu)",\
"Press Alt-Left, Alt-Right and Insert to change the current widget's setting.",\
"Press Alt-T to switch between the different widgets"]

		self.log["search"] = ["----<Search Commands>----"] + self.log["search"]
		
		self.log["browse"] = ["----<Browse Commands>----",\
"/browse                (Switch to Browse Mode)",\
"/buser    <user>       (Browse User)",\
"/bsearch  <query>      (Search thru Shares)",\
"/download <number>     (Download file with this number)",\
"/close                 (close current shares)",\
"Press Insert to toggle between shares.",\
"Alt-X                  (Expand/Collapse directory)",\
"/browsewidth <number>  (Resize Directories window)",\
"--File System browsing commands--",\
"cd (change dir) get, getdir (download)",\
"First, type in the user you wish to browse, below.", \
"Right-click on directories or files to display the popup menu."]

		
		self.log["download"] = ["----<Manual Download Commands>----",\
"/downuser <user>    (Set download user)",\
"/downpath <path>    (Download file from user)",\
"/downpathdir <path> (Download dir from user)"]

		self.log["keys"] =["------<Keys>------ ",\
"No guarantees that these HotKeyBar work with your terminal",\
"ESC or Alt + [, ], <-, ->  (Change Room / Transfer display mode)",\
"Insert                     (Same as above)",\
"Tab                        (Completes nicks)",\
"Home/End                   (switches Upload & Download scrolling)",\
"Up, PageUp                 (Scroll Up a line, a page)",\
"Down, PageDown             (Scroll Down a line, a page)",\
"F1->Chat Rooms           F6->Browse Users",\
"F2->Private Messages     F7->User Lists/Interests",\
"F3->Transfers            F8->Rooms List",\
"F4->Search               F9->Setup",\
"F5->Info                 F10->Help"]
		self.log["debug"] = []
		self.log["help"] =[\
"  _____  __ __   ____  ____  __ __  ______",\
" /     \|  |  \_/ ___\/  _ \|  |  \/  ___/",\
"|  Y Y  \  |  /\  \__(  <_> )  |  /\___ \ ",\
"|__|_|  /____/  \___  >____/|____//____  >",\
"      \/            \/                 \/",\
"/help          (This Message)         /quit          (Shut down Mucous)",\
"/help connect  (Connection Commands)  /help setup    (Setup Commands)",\
"/help mode     (Mode Commands)        /help user     (User Commands)",\
"/help chat     (Chatroom Commands)    /help ticker   (Ticker Commands)",\
"/help transfer (Transfer Commands)    /help download (Download Commands)",\
"/help browse   (Browse Commands)      /help search   (Search Commands)",\
"/help keys     (Special Keys)"]


	## Create window, draw title, 
	# call Help.Format
	# call Mucous.SetEditTitle
	# call Mucous.Alert.Check
	# @param self Help (Class)
	def Mode(self):
		try:
			self.mucous.UseAnotherEntryBox()
			# Cleanup stale windows
			if "text" in self.windows:
				del self.windows["text"]
			if "border" in self.windows:
				del self.windows["border"]
				
			if self.mucous.mode not in ("help", "debug"):
				self.mucous.mode = "debug"
			if self.mucous.mode == "help":
				logfile = self.log["help"]
			elif self.mucous.mode == "debug":
				logfile = self.log["debug"]
			self.mucous.PopupMenu.show = False
			
			s = self.dimensions["help"] = {"height": self.mucous.h-7, "width": self.mucous.w-2, "top": 2, "left": 1, "start": 0}
			mw = self.windows["border"] = curses.newwin(s["height"]+2, s["width"]+2, s["top"]-1, s["left"]-1)
			mw.attron(self.mucous.colors["green"])
			mw.border()
			mw.attroff(self.mucous.colors["green"])
			try:
				if self.mucous.mode == "help":
					mw.addstr(0, 3, "< Help Mode >",  self.mucous.colors["green"] | curses.A_BOLD)
					mw.addstr(0, 18, "<            >",  self.mucous.colors["green"])
					mw.addstr(0, 20, "Debug Mode",  curses.A_BOLD)
				elif self.mucous.mode == "debug":
					mw.addstr(0, 3, "<           >",  self.mucous.colors["green"] )
					mw.addstr(0, 5, "Help Mode",  curses.A_BOLD)
					mw.addstr(0, 18, "< Debug Mode >",  self.mucous.colors["green"] | curses.A_BOLD)
			except:
				pass
			mw.refresh()
			tw = self.windows["text"]  = mw.subwin(s["height"], s["width"], s["top"], s["left"])
			tw.scrollok(0)
			tw.idlok(1)
			
			self.scrolling["help"] = -1
			self.scrolling["debug"] = -1
			self.Format()
			
			self.mucous.SetEditTitle("Use /help")
			#if self.Alerts.log in ("New Help", "New Bug", "New Status"):
			#	self.Alerts.setStatus("")
				
			
			self.mucous.Alerts.Check()
			curses.doupdate()
		except Exception, e:
# 			self.Log("debug", ": " + str(e))
			self.mucous.ChatRooms.AppendChat("Status", self.mucous.ChatRooms.current, "Format: ", str(e))
			pass
	## Format lines and then
	# Call Draw for each line
	# @param self Help (Class)
	def Format(self):
		try:
			w = self.dimensions["help"]
			tw = self.windows["text"]
			size = w["height"] * w["width"]
			if self.mucous.mode == "help":
				logfile = self.log["help"]
			elif self.mucous.mode == "debug":
				logfile = self.log["debug"]
			# DEBUGGING
			wrapped_lines = []
			for lines in logfile:
				list_of_strings = self.mucous.FormatData.StringCutWidth(lines, w)
				for string in list_of_strings:
					wrapped_lines.append(string)
			if self.scrolling[self.mucous.mode] == -1:
				self.scrolling[self.mucous.mode] = len(wrapped_lines)
				
			clipped_list, self.scrolling[self.mucous.mode], w["start"] = self.mucous.FormatData.scrollbox(wrapped_lines, self.scrolling[self.mucous.mode], w["height"])
			del wrapped_lines
			count = 0
			
			blanked_lines = []
			for lines in clipped_list:
				s, ls = self.mucous.FormatData.StringAddBlanks(lines, w) 
				blanked_lines.append(s)
				
			clipped_list = blanked_lines
			del blanked_lines
			count = 0
			total_lines = 0
			tw.erase()
			for line in clipped_list:
				try:
					if line is clipped_list[-1]:
						line = line[:-1]
					self.Draw(self.mucous.mode, line, count)
					count += 1
				except Exception, e:
					self.mucous.ChatRooms.AppendChat("Status", self.mucous.ChatRooms.current, 'ERR', str(e) )
			tw.refresh()
	
		except Exception, e:
			self.mucous.ChatRooms.AppendChat("Status", self.mucous.ChatRooms.current, "Format: ", str(e))
			
	## Append anything to the Log (converted to string)
	# @param self Help (Class)
	# @param htype Help type (help, status, debug)
	# @param s data to be appended to the log
	def Log(self, htype, s):
		try:
			s = str(s)
			s = s.replace("\t", "      ")
			if htype == "help":
				if "\n" in s:
					lis = s.split("\n")
					for line in lis:
						self.log["help"].append("%s" %line )
				else:
					self.log["help"].append("%s" %s )
			elif htype in ("status", "debug"):
				timestamp = time.strftime("%H:%M:%S")
				if htype == "status":
					ex = ''
				else: ex = "BUG " 
				
				
				if "\n" in s:
					
					lis = s.split("\n")
					for line in lis:
						newline = ""
						for character in line:
							if curses.ascii.isctrl(character):
								character = curses.ascii.unctrl(character)
							newline += character
						if line is lis[0]:
							self.log["debug"].append("%s %s%s" % (timestamp,ex,newline))
						else:
							self.log["debug"].append("%s%s" % (ex,newline))
				
				else:
					self.log["debug"].append("%s %s%s" %(timestamp, ex,s))
				if htype == "debug":
					
					ex = "BUG "
					tbe = sys.exc_info()
					for line in tbe:
						if line is tbe[0]:
							self.log["debug"].append("%s %s%s" % (timestamp,ex,line))
						#else: self.log["debug"].append("%s%s" % (ex,line))
					tb = self.mucous.traceback.extract_tb(sys.exc_info()[2])
					for line in tb:
						if type(line) is tuple:
							xline = ""
							for item in line:
								xline += str(item) + " "
							line = xline

						newline = ""
						for character in line:
							if curses.ascii.isctrl(character):
								character = curses.ascii.unctrl(character)
							newline += character
						if line is tb[0]:
							self.log["debug"].append("%s %s%s" % (timestamp,ex,newline))
						else: self.log["debug"].append("%s%s" % (ex,newline))
				
			if self.mucous.mode in ( "help", "debug", "status"):
				#self.Draw( htype, s, 0)
				
				self.scrolling[self.mucous.mode] = -1
				self.Format()
				self.mucous.Alerts.Check()
				curses.doupdate()
			else:
	
				if htype not in self.mucous.Alerts.alert["HELP"]:
					if htype == "help":
						self.mucous.Alerts.alert["HELP"].append("help")
					elif htype == "status":
						self.mucous.Alerts.alert["HELP"].append("status")
					elif htype == "debug":
						self.mucous.Alerts.alert["HELP"].append("debug")
				
				self.mucous.Alerts.Check()
		except Exception, e:
			self.mucous.ChatRooms.AppendChat("Status", self.mucous.ChatRooms.current, 'ERR', str(e) )
			pass
	
	## Draw a line in the Help log 
	# @param self Help (Class)
	# @param htype Help type (help, status, debug)
	# @param s data to be drawn
	# @param count scroll position
	def Draw(self, htype, s, count):
		try:
			
			if self.mucous.mode in ( "help", "debug", "status"):
				tw = self.windows["text"]
				w = self.dimensions["help"]
				if count + w["start"] == self.scrolling[self.mucous.mode]:
					attr = curses.A_BOLD
				else:
					attr = curses.A_NORMAL
				lastmessage = ""
				for character in self.mucous.dlang(s):
					if curses.ascii.isctrl(character):
						character = curses.ascii.unctrl(character)
					lastmessage += character
				if self.mucous.mode == "help" and htype == "help":
					tw.addstr(lastmessage, attr)
				elif self.mucous.mode == "debug" and htype in( "status", "debug"):
					tw.addstr(lastmessage, attr)
				else:
					self.mucous.Alerts.setStatus("New Help")
			else:
				self.mucous.Alerts.setStatus("New Help")
		except: # Exception, e:
			
			#self.ChatRooms.AppendChat("Status", self.ChatRooms.current, "Draw: ", str(e))
			pass
		

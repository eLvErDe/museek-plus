# This is part of the Mucous Museek Client, and distributed under the GPLv2
# Copyright (c) 2006 daelstorm. 
import curses.wrapper
## Browse Shares
# Files and Directory viewer
class BrowseShares:
	## Constructor
	# @param self BrowseShares (class)
	# @param mucous Mucous (class)
	def __init__(self, mucous):
		## @var mucous 
		# Mucous (Class)
		self.mucous = mucous
		
		## @var dimensions
		# Window placement
		self.dimensions = {}
		## @var windows
		# Curses Window instances
		self.windows = {}
		## @var requests 
		# Users whose shares we've requested (Discard other requests)
		self.requests = []
		## @var current
		# Currently shown user's shares
		self.current = None
		## @var current_dir
		# Current Directory
		self.current_dir = None
		## @var users 
		# Users whose info we have
		self.users = []
		## @var logs
		# dict containing descriptions and stats of users
		self.logs = {}
		## @var help
		# Instructions
		self.help = self.mucous.Help.log["browse"]
		## @var files 
		# Current user's files in current directory
		self.files = []
		## @var dirs 
		# Current user's directories
		self.dirs = []
		## @var scrolling
		# dict containing vertical scroll position for files, directories
		self.scrolling = {"files": 0, "directories": 0}
		## @var selected
		# Selected window
		self.selected = "directories"
		## @var results
		# dict of users with dict of shares
		self.results = {}
		## @var collapsed 
		# dict of users with list of collapsed directories
		self.collapsed = {}
		## @var browse_num
		# dict of numbers related to the current files in the current directory 
		self.browse_num = {}
		## @var bfilter
		# Filter string for files
		self.bfilter = None
		self.dirswithtree = None
		
	## Get the currently selected user and file
	# @param self BrowseShares (class)
	# @return user, path
	def CurrentFile(self):
		user = self.current
		if self.files != []:
			path = self.current_dir+"\\"+self.files[self.scrolling["files"]]
		else:
			path = self.current_dir
		return user, path
	
	## Get the currently selected user and directory
	# @param self BrowseShares (class)
	# @return user, directory
	def CurrentDir(self):
		if self.selected == "files":
			w = self.dimensions["browse"]
			directory = self.current_dir
			user = self.current
			
		elif self.selected == "directories":
			w = self.dimensions["directories"]
			directory = self.current_dir
			user = self.current
		return user, directory
		
	## Get the user and path from file number
	# @param self BrowseShares (class)
	# @param number number of file
	# @return user, path
	def GetDownloadFromNum(self, number):
		try:

			number = int(number)
			user = self.current
			path = self.current_dir+"\\"+self.files[number]
			return user, path
				
		except Exception, e:
			self.mucous.Help.Log("debug", "download_path_file: " + str(e))
			
	## Create windows and Call draw functions
	# @param self BrowseShares (class)
	def Mode(self):
		try:
			self.dirswithtree = None
			self.mucous.mode = "browse"
			self.mucous.UseAnotherEntryBox()
			self.mucous.PopupMenu.show = False

			# Cleanup stale windows
			if "text" in self.windows:
				del self.windows["text"]
			if "border" in self.windows: 
				del self.windows["border"]
			if "dirwin" in self.windows:
				del self.windows["dirwin"]
			if "dirborder" in self.windows:
				del self.windows["dirborder"]
			if "browsebar" in self.windows:
				del self.windows["browsebar"]
			
			w = self.dimensions["browse"] = {"height": self.mucous.h-11, "width": self.mucous.w-self.mucous.Config["mucous"]["browse_width"], "top": 5, "left": self.mucous.Config["mucous"]["browse_width"]-1, "start": 0}
			# Files Border	
			wbb = self.windows["border"] = curses.newwin(w["height"]+2, w["width"]+2, w["top"]-1, w["left"]-1)
			# Directories Border
			self.windows["dirborder"] = curses.newwin(w["height"]+2, self.mucous.w-w["width"]-2, w["top"]-1, 0)
			self.DrawBrowseWin()
			# Files Text
			tbb = self.windows["text"] = wbb.subwin(w["height"], w["width"], w["top"], w["left"])
			tbb.scrollok(0)
			tbb.idlok(1)
			tbb.noutrefresh()

			d = self.dimensions["directories"] = {"height": w["height"], "width": self.mucous.w-w["width"]-4, "top": w["top"], "left":1}
			# Directories Text
			dw = self.windows["dirwin"] = self.windows["dirborder"].subwin(d["height"], d["width"], d["top"], d["left"])
			dw.erase()
			dw.noutrefresh()
	
# 			self.scrolling["files"] = self.scrolling["directories"] = 0
			# Vars

			self.files = []
			self.dirs = []
			
			self.windows["browsebar"] = curses.newwin(1, self.mucous.w, w["top"]+w["height"]+1, 0)
			self.windows["browsebar"].erase()
			self.windows["browsebar"].noutrefresh()
			
			self.mucous.Alerts.Check()
			self.mucous.HotKeyBar()
			self.FormatBrowse()
			curses.doupdate()
			del w
		except Exception, e:
			self.mucous.Help.Log("debug", "BrowseShares.Mode: " + str(e))
			
	## Recieved shares
	# @param self BrowseShares (class)
	# @param user Username
	# @param shares Shares dict
	def Recieved(self, user, shares):
		try:
			if user not in self.requests:
				return
			self.requests.remove(user)
			if user not in self.users:
				self.users.append(user)
			
			self.current = user
			if self.mucous.mode != "browse":
				self.mucous.Alerts.alert["BROWSE"].append(self.current)
				self.mucous.Alerts.Check()

			self.results[user] = {}
			self.collapsed[user] = []
			#self.num[user] = 0
			self.results[user]["dirs"] = []
			# Debugging
			#self.mucous.Help.Log("debug", shares.keys())
			#########
			if shares != {}:
				sdirs = shares.keys()
				sdirs.sort(key=str.lower)
			
				for item in sdirs:
					#item.rsplit("\\", 1)
					s = item.split("\\")
					path = ''

					parent = s[0]
					for seq in s[1:]:

						parent += "\\"

						path = parent+seq

						if path not in self.results[user]["dirs"]:
							self.results[user]["dirs"].append(path)
						parent =  path
								
				self.results[user]["dirs"].sort(key=str.lower)
			else:
				self.results[user]["dirs"].append("Empty Shares")
				
			self.current_dir = self.results[user]["dirs"][0]
			
			self.results[user]["shares"] = shares
			
			for dirs, files in shares.items():
				self.results[user][dirs] = files
				#result_list = []
				
			if self.mucous.mode == "browse":
				self.mucous.SetEditTitle("Browse "+user+"'s files in " + self.current_dir + " ")
				self.Mode()
		except Exception, e:
			self.mucous.Help.Log("debug", "BrowseShares.Recieved: " + str(e))
			
	## Draw Browse Window Border
	# @param self BrowseShares (class)
	def DrawBrowseWin(self):
		try:
			w = self.dimensions["browse"]
			mw = self.windows["border"]
			if self.mucous.BrowseShares.selected == "files":
				mw.attron(self.mucous.colors["green"])
			else:
				mw.attroff(self.mucous.colors["green"])
			mw.border()
	
			try:
				if self.mucous.BrowseShares.selected == "files":
					attr = self.mucous.colors["green"] | curses.A_BOLD
				else:
					attr = curses.A_BOLD
				if self.current == None:
					mw.addstr(0, 3, "< Browse users >",  attr)
				else:
					mw.addstr(0, 1, "<Num",  attr)
					mw.addch(curses.ACS_VLINE, attr)
					mw.addstr(" Size  ",  attr)
					mw.addch(curses.ACS_VLINE, attr)
					mw.addstr(" Filename >",  attr)
			except:
				pass
			mw.noutrefresh()
			
			self.windows["dirborder"].erase()
			if self.mucous.BrowseShares.selected == "directories":
				self.windows["dirborder"].attron(self.mucous.colors["green"])
				attr = self.mucous.colors["green"] | curses.A_BOLD
			else:
				attr = curses.A_BOLD
			self.windows["dirborder"].border()
			self.windows["dirborder"].attroff(self.mucous.colors["green"] )
			self.windows["dirborder"].addstr(0, 1, "< Directories >",  attr)
			self.windows["dirborder"].addstr(w["height"]+1, 1, "< Width - %s + >"  % self.mucous.Config["mucous"]["browse_width"],  attr)
			self.windows["dirborder"].noutrefresh()

			if self.current == None:
				self.scrolling["directories"] = 0
				self.mucous.SetEditTitle("Choose a user to Browse Shares")
				self.mucous.DrawInstructionsButtons()
			else:
				self.mucous.DrawInstructionsButtons()
				#s = "Browse "+self.current+"'s files in "
				#ls = len(s)
				#self.mucous.SetEditTitle( self.current_dir[:self.mucous.w-8] )
		except Exception, e:
			self.mucous.Help.Log("debug", "BrowseShares.DrawBrowseWin: " + str(e))
			
	## Parse Directories and display extremely formatted list
	# expensive (HIGH CPU Usage) 
	# @param self BrowseShares (class)
	def FormatBrowseDirs(self):
		try: 
			d = self.dimensions["directories"]
			w = self.dimensions["browse"] 
			tw = self.windows["text"]
			Dirwin = self.windows["dirwin"]
			if self.current == None:
                                # Clear windows, display browse help
				Dirwin.erase()
				Dirwin.noutrefresh()
				tw.erase()
				count = 0
				
				for lines in self.help:

					try:
						self.DrawBrowseFileText(lines, count , self.scrolling["files"])
		
					except Exception, e:
						self.mucous.Help.Log("debug", "Browse mode" + str(e))
				tw.noutrefresh()
				return
                        # If the default help isn't displayed
						
			
			#tempdirs.sort(key=str.lower)
			# List, Directory, Scroll position
			collapsed = self.collapsed[self.current]
			
			if self.dirswithtree == None: 
				self.dirswithtree = []
				num = 0
				self.dirs = []
				parents = []
				tempdirs = self.results[self.current]["dirs"]
				for directory in tempdirs:
					parent = "\\".join(directory.split("\\")[:-1])
					if parent in tempdirs and parent not in parents:
						parents.append(parent)
		
				for directory in tempdirs:
					try:
						parent = "\\".join(directory.split("\\")[:-1]) 
						a = 0
						for dirs in collapsed:
							if parent.startswith(dirs+"\\") or parent == dirs:
								a = 1
								break
						if a == 1:
							num += 1
							continue
						if directory in parents:
							if directory in collapsed:
								self.dirswithtree.append([directory, 2])
								self.dirs.append(directory)
							else:
								for dir in collapsed:
									if dir in directory:
										pass
								else:
									self.dirswithtree.append([directory, 1])
									self.dirs.append(directory)
						else:
							for dir in collapsed:
								if dir in directory:
									pass
							else:
								self.dirswithtree.append([directory, 0])
								self.dirs.append(directory)
					except Exception, e:
						for dir in collapsed:
							if dir in directory:
								pass
						else:
							self.dirswithtree.append([directory, 0])
							self.dirs.append(directory)
					num += 1 
			
			if self.mucous.BrowseShares.selected == "directories":
				clipped_list, self.scrolling["directories"], start = self.mucous.FormatData.scrollbox(self.dirswithtree, self.scrolling["directories"], d["height"])
			else:
				clipped_list, self.scrolling["directories"], start = self.mucous.FormatData.scrollbox(self.dirswithtree, self.scrolling["directories"], d["height"])
			#self.mucous.Help.Log("debug", self.scrolling["directories"])
			self.dimensions["directories"]["start"] = start
			count = 0
			Dirwin.erase()
			# Display directory tree
			for s, has_child in clipped_list:
				try:
					dir = s.split("\\")
					pre_spaces = " " * (len(dir)-2)
					Dirwin.addstr(pre_spaces)
					if has_child == 1:
						modifier = "[-]"
						Dirwin.addstr("[")
						Dirwin.addstr("-", self.mucous.colors["red"] | curses.A_BOLD)
						Dirwin.addstr("]")
						size = 3
					elif has_child == 2:
						modifier = "[+]"
						Dirwin.addstr("[")
						Dirwin.addstr("+", self.mucous.colors["green"] | curses.A_BOLD)
						Dirwin.addstr("]")
						size = 3
					else: 
						modifier = "|\\"
						Dirwin.addstr("|\\")
						size = 2
					string = dir[-1][:d["width"]-len(pre_spaces)-size]
					self.mucous.dlang(string)
					string += " " * ( d["width"] -len(string) -len(pre_spaces)-size)
					# Spaces before directory, to make it look like a tree
					
					
					
					#string = (" " * (len(dir)-2)) + modifier
					#string += self.mucous.dlang(dir[-1][:d["width"]-len(string)])
					#string += " " * ( d["width"] -len(string) )
					if count +d["start"] == self.scrolling["directories"]:
						Dirwin.addstr(string, self.mucous.colors["green"])
					else:
						Dirwin.addstr(string)
					count += 1
				except Exception, e:
					pass
					#self.mucous.Help.Log("debug", str(e))
			del clipped_list
			
			
		except Exception, e:
			self.mucous.Help.Log("debug", "BrowseShares.FormatBrowseDirs: " + str(e))
			
	## Format Files and Directories (with calls to functions)
	# @param self BrowseShares (class)
	# @param FormatDirs Format and Draw Dirs (True/False)
	def FormatBrowse(self, FormatDirs=True):
		try:
			if "directories" not in self.dimensions:
				return
			if FormatDirs:
				self.FormatBrowseDirs()
			if self.current != None:
				self.current_dir = self.dirs[self.scrolling["directories"]]
				self.mucous.SetEditTitle(self.current_dir )
			
				self.DrawFiles( self.current, self.current_dir)
			
				self.FileBar( self.current, self.current_dir)
			
			self.windows["dirwin"].noutrefresh()
		
			
			
			self.mucous.DrawTabs(self.users, self.current)

			
		except Exception, e:
			self.mucous.Help.Log("debug", "BrowseShares.FormatBrowse: " + str(e))
			


	## Send a request to museekd to browse this user
	# @param self BrowseShares (class)
	# @param user Username
	def Get(self, user):
		try:
			if user not in self.requests:
				self.requests.append(user)
			self.mucous.D.UserShares(user)
		except Exception, e:
			self.mucous.Help.Log("debug", "BrowseShares.Get" + str(e))

	## Draw file's number, name and size
	# @param self BrowseShares (class)
	# @param line line from list
	# @param count line position in list
	# @param sup scroll position
	def DrawBrowseFileText(self,  line, count, sup):
		try:
			w = self.dimensions["browse"]
			tw = self.windows["text"]
			this_line = self.mucous.dlang( line )
			if len(this_line) > self.dimensions["browse"]["width"]:
				crop = len(this_line) - self.dimensions["browse"]["width"]
				this_line = this_line[:-crop]
				
			if count + w["start"] == sup:
				attr = self.mucous.colors["blackwhite"] |curses.A_REVERSE|curses.A_BOLD	
				nattr = self.mucous.colors["cyan"] |curses.A_REVERSE|curses.A_BOLD
			else:
				attr = curses.A_NORMAL
				nattr = self.mucous.colors["cyan"] 
			if self.current == None:
				tw.addstr(this_line, attr )
			else:
				tw.addstr(this_line[:4], attr )
				tw.addch(curses.ACS_VLINE, attr)
				tw.addstr(this_line[5:12], nattr )
				tw.addch(curses.ACS_VLINE, attr)
				tw.addstr(this_line[13:], attr )
			z = w["width"]-len(this_line)
			space = " " * ( z )
			tw.addstr(space, attr)
			
		except Exception, e:
			#self.mucous.Help.Log("debug", "BrowseShares.BrowseFileText: " + str(e))
			pass
	
	## Draw files in a user's directory
	# @param self BrowseShares (class)
	# @param user Username
	# @param directory Directory of file list
	def DrawFiles(self, user, directory):
		try:
			if self.mucous.mode != "browse":
				self.mucous.Alerts.alert["BROWSE"].append(user)
				self.mucous.Alerts.setStatus("Browse: %s" % user)
				return
			
			tw = self.windows["text"]
			w = self.dimensions["browse"]
			self.browse_num[user] = 0
			browse_list = []
			count =0 
			# file, stats[ size, ftype, [bitrate, length ] ]
			
			if directory not in self.results[user]["shares"] or self.results[user]["dirs"] == {}:
				tw.erase()
				tw.addstr("Empty..")
				tw.refresh()
				self.files =  []
				return
			
			length_list = len(str(len(self.results[user][directory].keys() ) ) )
			list1 = self.results[user][directory].keys()
			list1.sort(key=str.lower)

			for file in list1:
				stats = self.results[user][directory][file]
				count += 1
				
				self.browse_num[user] = self.browse_num[user] +1
				size = str(self.mucous.FormatData.Humanize(stats[0]))
				if len(size) < 6:
					size = ' '* (6-len(size)) + size
				ftype =stats[1]
				
				if ftype.upper() in ('OGG', 'MP3') and stats[2] != []:
					bitrate =str(stats[2][0])
					if bitrate == '':
						bitrate = '0'	
					length =str(stats[2][1])
					if length != '' and length != None:
						minutes = int(length)/60
						seconds = str( int(length) - (60 * minutes))
						if len(seconds) < 2:
							seconds = '0' + seconds
						length = str(minutes)+":"+str(seconds)
					else:
						length = "0:00"
						bitrate = '0'
				else:
					ftype = "None"
					length = "0:00"
					bitrate = '0'
					
				filename = directory + "\\" + file
				#result_list = user, filename
				# Activate Number for Result
				if len(str(count)) < 4:
					s = " " * (4 - len(str(count)))
				else:
					s = ''
				
				size = " " * (7 - len(str(size))) + size[:7]
				line = "%s%s|%s|%s" % ( s, str(count), size, file )
				browse_list.append(line)
				
			self.files = list1
				
			if self.bfilter != None:
				a = []
				for path in browse_list:
					if re.match( self.bfilter, path):
						a.append(path)
				browse_list = a
				del a
					
			clipped_list, self.scrolling["files"], self.dimensions["browse"]["start"] = self.mucous.FormatData.scrollbox(browse_list, self.scrolling["files"], w["height"])
			sup = self.scrolling["files"]
				
			

			count = 0
			tw.erase()
			for line in clipped_list:
				self.DrawBrowseFileText(line, count, sup)
				count += 1
			tw.refresh()
			
		except Exception, e:
			self.mucous.Help.Log("debug", "BrowseShares.DrawFiles: " + str(e))
			
	## Draw File's stats in FileBar
	# @param self BrowseShares (class)
	# @param user Username
	# @param directory Directory of file list
	def FileBar(self, user, directory):
		try:
			self.windows["browsebar"].erase()
			num = self.scrolling["files"]

			if directory not in self.results[user]["shares"]:
				self.windows["browsebar"].refresh()
				return
			if self.results[user]["shares"][directory].keys() != []:
                                list1 = self.results[user]["shares"][directory].keys()
			        list1.sort(key=str.lower)
                                file = list1[num]
                                stats = self.results[user]["shares"][directory][file] 
			else: 
				self.windows["browsebar"].refresh()
				return
	

			size  = self.mucous.FormatData.Humanize(stats[0])
			
			ftype =stats[1]
			if ftype == '':
				ftype = "None"
				length = "0:00"
				bitrate = '0'
			else:
				bitrate =str(stats[2][0])
				if bitrate == '':
					bitrate = '0'	
				length =str(stats[2][1])
				if length != '' and length != None:
					minutes = int(length)/60
					seconds = str( int(length) - (60 * minutes))
					if len(seconds) < 2:
						seconds = '0' + seconds
					length = str(minutes)+":"+str(seconds)
				else:
					length = "0:00"
			
                        #l=len('['+str(num+1)+'] '+" Size: " + str(size)+" Length: " + length + " Bitrate: " + bitrate)
                        atr = self.mucous.colors["cyan"] | curses.A_BOLD

			self.windows["browsebar"].addstr("[")
                        self.windows["browsebar"].addstr(str(num+1), atr )
                        self.windows["browsebar"].addstr("] | Size: ")
                        self.windows["browsebar"].addstr(str(size), atr)
                        self.windows["browsebar"].addstr(" | ")
                        self.windows["browsebar"].addstr(bitrate, atr)
                        self.windows["browsebar"].addstr("Kbps | Length: ")
                        self.windows["browsebar"].addstr(length, atr)
			self.windows["browsebar"].refresh()
		except Exception, e:
			self.mucous.Help.Log("debug", "BrowseShares.FileBar: " + str(e))
			
	## Change Directory to line
	# @param self BrowseShares (class)
	# @param line string that should contain a valid path 
	def ChangeDir(self, line):
		try:
			if self.current_dir == '' or line[0:1] == '\\' or line[1:2] == ":":
				if self.current in self.results.keys():
					if line[0:1] == '\\':
						if line in self.results[self.current].keys():
							self.current_dir = line
						else:
							self.mucous.Help.Log("status", "No such directory: %s user:" % (line[1:],  self.current))
					if line[1:2] == ":":
						if line in self.results[self.current].keys():
							self.current_dir = line
						else:
							self.mucous.Help.Log("status", "No such directory: %s user:" % (line,  self.current))
					else:
						if line in self.results[self.current].keys():
							self.current_dir = line
						else:
							self.mucous.Help.Log("status", "No such directory: %s user:" % (line,  self.current))
				
			elif line =='..':
				z = self.ParentDir()
				if z != 0:
					self.current_dir = z
				else:
					self.mucous.Help.Log("status", "No parent directory, User: " +  self.current)
			else:
				if self.current_dir + '\\'+line in self.results[self.current].keys():
					self.current_dir += '\\'+line
				else:
					self.mucous.Help.Log("status", "No such directory: %sUser: " % (line, self.current) )
					
			if self.current == None:
				self.mucous.SetEditTitle("Choose a user to Browse Shares")
			else:
				s = "Browse "+self.current+"'s files in "
				ls = len(s)
				self.mucous.SetEditTitle(s  + self.current_dir[:self.mucous.w-ls-4] + " ")
		except Exception, e:
			self.mucous.Help.Log("debug", "BrowseShares.ChangeDir: " + str(e))
			
	## Get Parent directory of current_dir
	# @param self BrowseShares (class)
	# @return directory
	def ParentDir(self):
		try:
			splitit = self.current_dir
			splitit = splitit.split("\\")
			s = len(splitit)
			directory=''
			for r in range(s-1):
				if r == 0:
					directory += splitit[r]
				else:
					directory += "\\"+splitit[r]
					
			if directoryz in self.results[self.current]["shares"].keys():
				return directory
			else:
				return None
		except Exception, e:
			self.mucous.Help.Log("debug", "BrowseShares.ParentDir: " + str(e))
	## Close Tab of user
	# @param self BrowseShares (class)
	# @param user Username whose shared are being closed
	def Close(self, user):
		try:
			if user in self.users:
				self.users.remove(user)
				self.scrolling["directories"] = 0
			if user in self.logs:
				del self.logs[user]
			if user in self.results:
				del self.results[user]
			if self.users != []:
				if self.current == user:
					self.current = self.users[0]
			else:
				self.current = None
			if user in self.mucous.Alerts.alert["BROWSE"] and user != "__default":
				self.mucous.Alerts.alert["BROWSE"].remove(user)
			self.Mode()
		except Exception, e:
			self.mucous.Help.Log("debug", "BrowseShares.Close: " + str(e))
	
	## Mouse Coordinates in the Browse Shares Mode
	# @param self is BrowseShares (class)
	# @param x is the horizontal position from the left
	# @param y is the vertical postion from the top
	# @param z is unimportant
	# @param event is the mouse event (button, doubleclick, etc) represented by a number
	def MouseBrowse(self, x,y,z,event):
		try:
			d = self.dimensions["directories"]
			w = self.dimensions["browse"]
			if y in (1, 2, 3):
				if len(self.users) == 1 and self.current == None:
					self.current = self.users[0]
					self.Mode()
				if len(self.users) > 1:	
					if self.current == None:
						self.current = self.users[0]
					else:
						self.current, match = self.mucous.edit.MouseClickTab(x, self.current)
						if match == None:
							s = self.users.index(self.current)
							self.current = self.users[s-1]
						sdirs =  self.results[self.current].keys()
						sdirs.sort(key=str.lower)
						self.current_dir=sdirs[0]
					
					self.Mode()
			elif y == w["top"] + w["height"]:
				if x < self.mucous.Config["mucous"]["browse_width"]:
					if x in (7,8,9,10):
						if self.mucous.Config["mucous"]["browse_width"] > 20: 
							self.mucous.Config["mucous"]["browse_width"] -= 1
							self.Mode()
					elif x in (12,13,14,15,16,17):
						if self.mucous.Config["mucous"]["browse_width"] < self.mucous.w-20:
							self.mucous.Config["mucous"]["browse_width"] += 1
							self.Mode()
					
			elif y == w["top"]-1:
			
				if x >= self.mucous.w-17 and self.current != None:
					self.current = None
					self.Mode()
			elif x in range(d["width"]) and y >= d["top"] and y <= d["top"] + d["height"]:
				if self.current == None:
					return
				if self.mucous.BrowseShares.selected != "directories":
					self.mucous.BrowseShares.selected = "directories"
					self.mucous.BrowseShares.DrawBrowseWin()
					
				self.scrolling["directories"] = y - d["top"] + d["start"]
				self.FormatBrowse()
				
				if event in ( 4096, 16384):
					self.mucous.ChatRooms.DrawBox()
					self.mucous.PopupMenu.Create("browse-dirs", 0, True)
				else:
					self.mucous.ChatRooms.DrawBox()
				curses.doupdate()
				
			elif x >=  w["left"] and y >= w["top"] and y <= w["top"] + w["height"]:
				if self.current == None:
					return
				if self.mucous.BrowseShares.selected != "files":
					self.mucous.BrowseShares.selected = "files"
					self.mucous.BrowseShares.DrawBrowseWin()
				self.scrolling["files"] = y - w["top"]+ w["start"]
				self.FormatBrowse()
				
				if event in ( 4096, 16384):
					#self.mucous.DrawBox()
					self.mucous.PopupMenu.Create("browse-files", 0, True)
				#else:
					#self.ChatRooms.DrawBox()
				curses.doupdate()
				
			elif y in ( self.mucous.h-5, self.mucous.h-6):
				if x>= self.mucous.w-27 and x < self.mucous.w-18:
					self.mucous.PopupMenu.Create("encoding", 0, True)
					return
				elif x >=self.mucous.w-10 and x < self.mucous.w-1 and self.current != None:
					self.Close(self.current)
		except Exception, e:
			self.mucous.Help.Log("debug", "BrowseShares.MouseBrowse: " +str(e) )

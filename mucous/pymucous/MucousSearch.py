# This is part of the Mucous Museek Client, and distributed under the GPLv2
# Copyright (c) 2006 daelstorm. 

import curses.wrapper
import re
## Search for files
#			
class Search:
	## Constructor
	# @param self Search (class)
	# @param mucous Mucous (class)
	def __init__(self, mucous):
		## @var mucous 
		# Mucous (Class)
		self.mucous = mucous
		## @var logs
		# dict of tickets with lists of results
		self.logs = {}
		## @var windows
		# dict containing instances of curses windows
		self.windows = {}
		## @var dimensions
		# dict containing placement data for windows
		self.dimensions = {}
		## @var sorted_search
		# list of sorted results
		self.sorted_search = None
		## @var sfilter
		# Display results that match this case-insensitive string
		self.sfilter = None
		## @var current
		# Current ticket
		self.current = None
		## @var tickets
		# key is the stringified ticket number; value is the string query
		self.tickets = { }
		## @var results
		# results [ ticket ] [ num ] [ str(ticket), user, free, speed, queue, result[0], result[1], ftype, result[3] ]
		self.results = { }
		## @var numresults
		# dict of tickets with number of search results recieved 
		self.numresults = { }
		## @var viewing
		# list of search results
		self.viewing = []
		## @var scrolling
		# vertical scroll position
		self.scrolling = 0
		## @var order
		# Sorting order (default is by number) :: num, user, free, speed, que, path, sizefile, bitrate, time 
		self.order = "num"
		## @var reverse
		# Reverse sorting
		self.reverse = False
		## @var method
		# Searching method (Global by default)
		self.method = "globally"
		## @var username
		# User to search shares of
		self.username = None
		## @var input
		# Input text entry point
		self.input = "default"
		## @var help
		# Search commands
		self.help = ["Search commands:"] + self.mucous.Help.log["search"] + ["Or, type in the query, below."]
		self.switchorder = ["default", "methods", "user", "sort", "reverse", "results", "filter", "tabs"]
	
	
	## Reset input to 'default' and redraw search
	# @param self Search (class)
	def Default(self):
		try:
			self.input = "default"
			self.Mode()
			self.mucous.ModeTopbar()
		except Exception, e:
			self.mucous.Help.Log("debug", "Search.Default: "+ str(e))
			
	## Create Search window
	# @param self Search (class)
	def Mode(self):
		try:
			self.mucous.mode = "search"
			self.mucous.UseAnotherEntryBox()
			self.mucous.PopupMenu.show = False
			
			# Cleanup stale windows
			if "query" in self.windows:
				del self.windows["query"]
			if "border" in self.windows:
				del self.windows["border"]
			if "stats" in self.windows:
				del self.windows["stats"]
			if "options" in self.windows:
				del self.windows["options"]
				
			w = self.dimensions = {"height": self.mucous.h-12, "width": self.mucous.w-2, "top": 5, "left": 1, "start": 0}
			mw = self.windows["border"] = curses.newwin(w["height"]+2, w["width"]+2, w["top"]-1, w["left"]-1)
			
			if self.input == "results":
				color = self.mucous.colors["greenblack"]
				mw.attron(self.mucous.colors["green"])
				mw.border()
				#mw.attroff(self.mucous.colors["green"])
			else:
				mw.border()
			try:
				mw.addstr(0, 3, "< Search >")
			except:
				pass
			
			if self.sfilter != None:
				sfilter = "Filter: " +self.sfilter
				
			else:
				sfilter = "Filter: Disabled"
			lfil = len(sfilter)
			
			if self.input == "filter":
				mw.addstr(0,15, "< ")
				mw.addstr(0,17, "Filter: ", self.mucous.colors["green"] | curses.A_BOLD)
				self.mucous.Setup.EditLine(self.mucous.dlang(self.sfilter),4, 25,1,lfil, edit=(self.input=="filter") )
				mw.addstr(0,25+lfil, " >")
			else:
				mw.addstr(0,15, "< ")
				if self.sfilter != None:
					mw.addstr(0,17, self.mucous.dlang(sfilter), self.mucous.colors["cyan"] | curses.A_BOLD)
				else:
					mw.addstr(0,17, self.mucous.dlang(sfilter), self.mucous.colors["red"] | curses.A_BOLD)
				mw.addstr(0,17+lfil, " >")
			
			
			self.SortBar()
			tw = self.windows["query"] = mw.subwin(w["height"], w["width"], w["top"], w["left"])
			tw.scrollok(0)
			tw.idlok(1)
			tw.attroff(self.mucous.colors["green"])
			self.windows["stats"] = curses.newwin(1, self.mucous.w, self.mucous.h-6, 0)
			self.windows["stats"].erase()
			self.windows["stats"].noutrefresh()
			
			self.windows["options"] = curses.newwin(1, self.mucous.w, self.mucous.h-5, 0)
			self.windows["options"].erase()
			self.windows["options"].noutrefresh()
			
			self.Draw()

			
			self.mucous.Setup.SetupCheck("Globally", (self.method=="globally"), self.mucous.h-5, 0,1,13, True, True, selected=(self.method=="globally"), underlined=(self.input=="methods"))
			self.mucous.Setup.SetupCheck("Buddies", (self.method=="buddies"), self.mucous.h-5, 13,1,13, True, True, selected=(self.method=="buddies"), underlined=(self.input=="methods"))
			self.mucous.Setup.SetupCheck("Rooms", (self.method=="rooms"), self.mucous.h-5, 25,1,10, True, True, selected=(self.method=="rooms"), underlined=(self.input=="methods"))
			self.mucous.Setup.SetupCheck("User:", (self.method=="user"), self.mucous.h-5, 48,1,10, True, True, selected=(self.method=="user"), underlined=(self.input=="methods"))
			self.mucous.Setup.SetupCheck("Wishlist", (self.method=="wishlist"), self.mucous.h-5, 35,1,13, True, True, selected=(self.method=="wishlist"), underlined=(self.input=="methods"))
			#self.mucous.Setup.SetupCheck(self.method.capitalize(), False,self.mucous.h-5, 0,1,15, True, True, selected=(self.method=="globally"))
			self.mucous.Setup.EditLine(self.username, self.mucous.h-5, 58,1,15, edit=(self.input=="user") ) 
			
			if self.method != None:
				if self.method == "user":
					if self.username != None:
						title = "Search "+self.method.capitalize()+" "+self.username+"'s shares"
					else:
						title = "Search "+self.method.capitalize()+" (Pick a user with /searchuser)"
				else:
					title = "Search "+self.method.capitalize()+" for:"
			else:
				title = ""
			self.mucous.SetEditTitle(title, selected=(self.input=="default"))
			
			#self.mucous.DrawTabs(self.results.keys(), self.current)
			self.mucous.DrawTabs(self.tickets.keys(), self.current, selected=(self.input=="tabs"))

			if self.current != None:
				self.DrawInstructionsButtons()
				self.mucous.Alerts.Check()
			else:
				self.mucous.HotKeyBar()
			curses.doupdate()
		except Exception,e:
			self.mucous.Help.Log("debug", "Search.Mode: "+str(e))
			
	## Draw Buttons for switching with the mouse to Instructions view
	# @param self Search (class)
	def DrawInstructionsButtons(self):
		try:
			gi = "Instructions"
			w = self.dimensions
			
			
			pos = w["width"]-3-len(gi)
			mw = self.windows["border"]
			
			mw.addstr(0,pos, "< ")
			mw.addstr(0,pos+2, gi, self.mucous.colors["cyan"] | curses.A_BOLD)
			mw.addstr(0,pos+2+len(gi), " >")
			vertex = w["height"]+1
			mw.addstr(vertex,self.mucous.w-11, "< ")
			mw.addstr(vertex,self.mucous.w-9, "Close ", self.mucous.colors["cyan"] | curses.A_BOLD)
			mw.addstr(vertex,self.mucous.w-3, ">")
			
			mw.noutrefresh()
		except Exception, e:
			self.mucous.Help.Log("debug", "DrawInstructionsButtons: " + str(e))
				
	def MethodSwitch(self, direction=None):
		# HotKeyBar to switch types of searches
		if direction == None:
			direction = "right"
		_list = [ "globally", "buddies", "rooms", "wishlist", "user" ]
		self.method = self.mucous.FormatData.RotateList(direction, _list, self.method, "no")
		#if self.method == "user":
			#self.input="user"
		#else:
			#self.input="default"
		self.Mode()
		
	## Draw Mucous settings
	# @param self is Setup (Class)
	
	def Switch(self, direction=None):
		if direction == None:
			direction = "right"
		self.input = self.mucous.FormatData.RotateList(direction, self.switchorder, self.input, "no")

		self.Mode()
		
	## New Search Ticket recieved
	# @param self Search (class)
	# @param query string searched for
	# @param ticket unique number associated with search
	def NewTicket(self, query, ticket):
		try:
			self.tickets[str(ticket)] = query
			if self.current == None:
				self.current = str(ticket)
			if self.mucous.mode == "search":
				self.Mode()
				self.Stats("status", "Started search for: %s" % query, str(ticket), 0)
			self.results[str(ticket)] = {}	
			self.numresults[str(ticket)] = 0
		except Exception,e:
			self.mucous.Help.Log("debug", "Search.NewTicket: "+str(e))
			
	## New Search Results recieved
	# @param self Search (class)
	# @param ticket unique number (used to organize results)
	# @param user username of user with results
	# @param free is there a free slot open? (True/False)
	# @param speed average speed of user
	# @param queue length of queue
	# @param results list of files [path, size, extension, list of attributes(bitrate, length, unused)]
	def NewResults(self, ticket, user, free, speed, queue, results):
		try:
			if str(ticket) not in self.tickets:
				return 
			for result in results:
				result_list = []
				# Create Result List for future use 
				# clear it next interation
				# Count Search Result
				path, size, extension, attributes = result 
				num = self.numresults[str(ticket)]
				num += 1
				self.numresults[str(ticket)] = num
				# Send Num of Results to Search Window

				ftype = result[2]
				if ftype in ('', None):
					if result[0][-4:-3] == ".":
						ftype = result[0][-3:]
				ftype = ftype.upper()
				result_list = str(ticket), user, free, speed, queue, path, size, ftype, attributes
				self.results[str(ticket)][num] = result_list
			
			if self.mucous.mode != "search" or self.current != str(ticket):
				if str(ticket) not in self.mucous.Alerts.alert["SEARCH"]:
					self.mucous.Alerts.alert["SEARCH"].append( str(ticket) )
					self.mucous.Alerts.Check()
			else:
				if self.current == str(ticket) and  self.mucous.PopupMenu.show != True:
					self.FormatResults(str(ticket))
				else:
					self.Count(num)
					self.mucous.DrawTabs(self.tickets.keys(), self.current)
		except Exception,e:
			self.mucous.Help.Log("debug", "Search.NewResults: "+str(e))
	
	## Get Download from number
	# @param self Search (class)
	# @param num number
	# @return user, path
	def GetDownloadFromNum(self, num):
		try:
			number = int(num)
			if not self.results[self.current].has_key(number):
				self.mucous.Help.Log("debug", "Search.GetDownloadFromNum: No such number")
				return None, None
			user = self.results[self.current][number][1]
			path = self.results[self.current][number][5]
			return user, path
		except Exception,e:
			self.mucous.Help.Log("debug", "Search.GetDownloadFromNum: "+str(e))
	
	## Download Search result 
	# @param self Search (class)
	# @param user Username
	# @param path Path of search result
	# @param getdir If True, Download the directory if False, the file
	def DownloadSearch(self, user, path, getdir=False):
		try:
			if user == None or path == None:
				return
				
			if getdir:
				
				self.mucous.Transfers.FolderDownload(user, path)
			else:
				self.mucous.Transfers.RetryDownload(user, path)
				
			self.mucous.AutobuddyUser(user)
				
				
		except Exception, e:
			self.mucous.Help.Log("debug", "DownloadSearch: " + str(e))
	
	## Draw Sort Bar 
	# @param self Search (class)
	def SortBar(self):
		try:
			w = self.dimensions
			ls = ("Num", "User", "Free", "Speed", "Que", "Path", "Size", "File", "Bitrate", "Time")
			mw = self.windows["border"]
			mw.addstr(w["height"]+1, 1, "<    |    |    |     |   |    |    |    |       |     >")
			pos  = 0
			for i in ls:
				if i == self.order.capitalize():
					if self.input == "sort":
						attrib = self.mucous.colors["green"]| curses.A_BOLD | curses.A_UNDERLINE
					else:
						attrib = self.mucous.colors["green"]| curses.A_BOLD
					mw.addstr(w["height"]+1, 3+pos, self.order.capitalize(), attrib)
				else:
					mw.addstr(w["height"]+1, 3+pos, i, self.mucous.colors["red"] | curses.A_BOLD)
				pos += len(i) + 1
	
			pos = 56
			mw.addstr(w["height"]+1, pos, "<         >")
			pos = 58
			
			
			if self.reverse:
				rcolor = "green"
			else:
				rcolor = "red"
				
			if self.input == "reverse":
				attrib = self.mucous.colors[rcolor] | curses.A_BOLD | curses.A_UNDERLINE
			else:
				attrib = self.mucous.colors[rcolor] | curses.A_BOLD
			mw.addstr(w["height"]+1, pos, "Reverse", attrib)
	
			mw.noutrefresh()
			
		except Exception,e:
			self.mucous.Help.Log("debug", "Search.SortBar: "+str(e))
			
	## Draw Search Instructions/Help or call Search.FormatResults 
	# @param self Search (class)
	def Draw(self):
		try:
			
			if self.current == None:
				tw = self.windows["query"]
				tw.erase()
				w = self.dimensions
				for lines in self.help:
					try:
						lines, ls = self.mucous.FormatData.StringAddBlanks(lines, w)
						tw.addstr(self.mucous.dlang(lines))
					except Exception, e:
						#self.mucous.Help.Log("debug", e)
						pass
				tw.noutrefresh()
				
			else:
				try:
					if self.mucous.PopupMenu.show == True: raise  Exception,  "popup"
					self.FormatResults(self.current)
				except Exception, e:
					#self.mucous.Help.Log("debug", e)
					pass
			
		except Exception,e:
			self.mucous.Help.Log("debug", "Search.Draw: "+str(e))
	
	## Draw the number of Search results in the Search counter
	# @param self Search (class)
	# @param s string
	def Count(self, s):
		try:
			if "searchstatus" in self.mucous.windows["border"]:
				del self.mucous.windows["border"]["searchstatus"]
			ssw =self.mucous.windows["border"]["searchstatus"] = curses.newwin(1, 15, 0, 47)
			ssw.bkgdset(" ", self.mucous.colors["blackwhite"]  | curses.A_REVERSE | curses.A_BOLD)

			if s != None:
				if self.mucous.logs["search_count"] == ["Results: ", s]:
					return
				self.mucous.logs["search_count"] = "Results: ", s

			try:
				ssw.erase()
				ssw.addstr(self.mucous.logs["search_count"][0],  self.mucous.colors["blackwhite"] )
				ssw.addstr(str(self.mucous.logs["search_count"][1]),  self.mucous.colors["blackwhite"] )
				ssw.refresh()
			except Exception, e:
				pass
				#self.mucous.Help.Log( "debug", "Search Status: " + str(e))
		except Exception, e:
			self.mucous.Help.Log("debug", "Search.Count: " + str(e))
	
	## Clear all search data
	# @param self Search (class)
	def Clear(self):
		try:
			self.tickets = {}
			self.logs = {}
			self.numresults = {}
			self.results = {}
			self.current = None
			self.username = None
			self.Count(0)
			self.Mode()
		except Exception, e:
			self.mucous.Help.Log("debug", "Search.Clear: " + str(e))
	
	## Close Search tab
	# @param self Search (class)
	# @param ticket ticket to be closed 
	def Close(self, ticket):
		try:
			ticket = str(ticket)
			if ticket in self.tickets.keys():
				del self.tickets[ticket]
			if ticket in self.results.keys():
				del self.results[ticket]
			if ticket in self.numresults.keys():
				del self.numresults[ticket]
			if ticket in self.logs:
				del self.logs[ticket]
			if self.tickets.keys() != []:
				self.current = self.tickets.keys()[0]
			else:
				self.current = None
				
			if ticket in self.mucous.Alerts.alert["SEARCH"]:
				self.mucous.Alerts.alert["SEARCH"].remove(ticket)
			self.Mode()
		except Exception, e:
			self.mucous.Help.Log("debug", "Search.Close: " + str(e))
	
	## Format Results for this ticket to fit search window and send them to Search.Stats
	# @param self Search (class)
	# @param this_ticket ticket
	def FormatResults(self, this_ticket):
		if self.mucous.mode != "search":
			return
		try:
			tw = self.windows["query"]
			sorting_list = {}
			if str(this_ticket) not in self.results:
				self.Stats("status", 0, "Empty.....", 0 )
				return
			for numbers, results in self.results[str(this_ticket)].items():
				ticket, user, free, speed, queue, path, size, ftype, extended = results
				if this_ticket == ticket and self.current == ticket:
					if ftype.upper() in ('MP3', 'OGG'):
						if extended != []:
							bitrate = extended[0]
							time = extended[1]
						else:
							bitrate = 0
							time = 0
					else: 
						bitrate = 0
						time = 0

					if time in ('', None):
						time = 0
					
					if self.order == "num":
						sorting_list[numbers] = 0
					elif self.order == "user":
						sorting_list[numbers] = user
					elif self.order == "free":
						sorting_list[numbers] =  free
					elif self.order == "speed":
						sorting_list[numbers] =  speed
					elif self.order == "que":
						sorting_list[numbers] =  queue
					elif self.order == "path":
						sorting_list[numbers] =  path
					elif self.order == "size":
						sorting_list[numbers] =  size
					elif self.order == "file":
						sorting_list[numbers] =  ftype
					elif self.order == "bitrate":
						sorting_list[numbers] = bitrate
					elif self.order == "time":
						sorting_list[numbers] = time
					
			#self.mucous.Help.Log("status",sorting_list)
			#self.mucous.Help.Log("status",sorting_list)
			slist = self.mucous.FormatData.sortbyvalue (sorting_list)
			# Filter search while browsing
			if self.sfilter != None:
				s = []
				searchfilter = re.compile('.*' +str(self.sfilter) + '.*', re.DOTALL | re.I)
				
				for x,y  in slist:
					
					if not self.results[str(this_ticket)].has_key(x):
						continue
					z = self.results[str(this_ticket)][x]
					for c in (z[1], z[5]) :
						if re.match(searchfilter, c): 
							s.append(x)
							break
				
				self.sorted_search = s
			else:
				s = []
				for x,y  in slist:
					s.append(x)
				self.sorted_search = s
			if self.reverse == True:
				self.sorted_search.reverse()
			self.logs[str(this_ticket)] = []
			if self.scrolling == -1:
				self.scrolling = len(self.sorted_search)
			clipped_list, self.scrolling, self.dimensions["start"] = self.mucous.FormatData.scrollbox(self.sorted_search, self.scrolling, self.dimensions["height"])
			tw.erase()
			count = 0
			self.viewing = clipped_list
			for number in clipped_list:
				#self.format_this_search(n)
				self.Stats("result", number, str( self.results[this_ticket][number][0] ), count )
				count += 1
				
			
			tw.noutrefresh()
			self.Count(self.numresults[self.current])
			#self.mucous.DrawTabs(self.results.keys(), self.current)
			self.mucous.DrawTabs(self.tickets.keys(), self.current)
			##self.mucous.Help.Log("debug", "Search.Start: " + str(clipped_list))
		except Exception, e:
			self.mucous.Help.Log("debug", "Search.FormatResults: " + str(e))

	## Draw Result and Stats (if it's the current scroll position) 
	# @param self Search (class)
	# @param typer Type of Result (
	# @param result result number
	# @param ticket Ticket
	# @param count Clipped list position
	def Stats(self, typer, result, ticket, count):
		if str(ticket) not in self.logs:
			return
		try:
			if self.mucous.mode != "search":
				return
			if self.current != str(ticket):
				return
			tw = self.windows["query"]
			ss = self.windows["stats"]
			if typer == "status":
				ss.erase()
				ss.addstr(self.mucous.dlang(result), self.mucous.colors["cyan"])
				ss.noutrefresh()
			elif typer == "result":
				number = result
				ticket, user, free, speed, queue, path, size, ftype, extended = self.results[ticket][number]
				size  = self.mucous.FormatData.byte_format(size)

				if ftype.upper() in ('MP3', 'OGG'):
					if extended != []:
						bitrate = extended[0]
						length = int(extended[1])
						minutes = length/60
						seconds = str(length - (60 * minutes))
						
						if len(seconds) < 2:
							seconds = '0' + seconds
					else:
						bitrate = '0'
						minutes = '0'
						seconds = '00'
						length = 0
				else:
					bitrate = '0'
					minutes = '0'
					seconds = '00'
					length = 0
				if free:
					free = 'Y'
				else:
					free = 'N'
		
				
				if count + self.dimensions["start"]== self.scrolling:
					attr =  curses.A_REVERSE | curses.A_BOLD
					attrc = self.mucous.colors["cybg"]| curses.A_BOLD
					ss.erase()

					ss.addstr("F: ")
					atr = self.mucous.colors["cyan"] | curses.A_BOLD
					ss.addstr(free, atr )
					ss.addstr(" | Q:")
					ss.addstr(str(queue), atr )
					ss.addstr(" | ")
					ss.addstr(user[:15], atr )
					ss.addstr(" | ")
					ss.addstr(str(speed/1024), atr )
					ss.addstr("KB/s | Size: ")
					ss.addstr(str(size), atr )
					if bitrate != '0' and length != 0:
						ss.addstr(" | ")
						ss.addstr(str(bitrate), atr )
						ss.addstr("Kbps| Len: ")
						ss.addstr(str(minutes), atr )
						ss.addstr(":")
						ss.addstr(str(seconds), atr )
					ss.noutrefresh()
					
				else:
					attr = curses.A_NORMAL
					attrc = self.mucous.colors["cyan"]
				try:
					extra = ''
					sn = len(str(number))
					sr = len(str(self.numresults[ticket]))
					if sn < sr :
						nub = (" " * (sr - sn)) + str(number)
					else:
						nub = str(number)
						
					f = path.split("\\")
					file = f[-1]
					directory = "\\".join(f[:-1])
					## Shrink directory and file to fit in line
					if len(nub)+2+len(path) >= self.mucous.w-2:
						pos = self.mucous.w-2-len(str(nub)+"| ")-len(directory)-len(file)
						if abs(pos) > len(directory):
							a = abs(pos) - len(directory)
							file = file[:-a]
							directory = ''
						else:
							directory = directory[-pos:]
						

					extra = " " * (self.mucous.w-len(str(nub))- len(directory)-len(file)-4)
					tw.addstr(nub, attrc)
					tw.addch(curses.ACS_VLINE, attr)
					tw.addstr(""+directory+"\\", attr)
					tw.addstr(file, attrc)
					if extra != "":
						tw.addstr(extra, attr)
				except Exception, e:
					pass

		except Exception, e:
			self.mucous.Help.Log("debug", "Search Log: " + str(e))
			
	## Parse input functions
	# @param self is Search (Class)
	# @param key is the character pressed
	def Input(self, key):
		
		if self.mucous.mode != "search":
			return
		elif key in ("popup"):
			if self.mucous.PopupMenu.show == True:
				self.mucous.PopupMenu.Clear()
				return
			if self.mucous.Search.current != "default__":
				self.mucous.PopupMenu.show = True
				self.mucous.PopupMenu.Create("search", 0)
		elif key == "switch":
			self.mucous.Search.Switch(direction="right")
			return
		
		
		elif key in ( "KEY_HOME", "KEY_END"):
			self.mucous.edit.ScrollText(key)
			return
		elif key in( "KEY_LEFT", chr(91), chr(60), "KEY_RIGHT", chr(93), chr(62), "KEY_IC"):
			if key in ("KEY_LEFT", chr(91), chr(60)):
				direction = "left"
			elif key in ("KEY_RIGHT", chr(93), chr(62), "KEY_IC"):
				direction = "right"
			if self.input in ("default", "tabs"):
				if len(self.mucous.Search.tickets.keys()) >= 1:
					place = self.mucous.FormatData.RotateList(direction, self.mucous.Search.tickets.keys(), self.mucous.Search.current, "yes" )
					if self.mucous.Search.current != place:
						self.mucous.Search.current = place
						self.mucous.Search.Mode()
			elif self.input == "sort":
				place = self.mucous.FormatData.RotateList(direction, [ "num", "user", "free", "speed", "que", "path", "size",  "file", "bitrate",  "time"], self.mucous.Search.order, "no" )
				if self.mucous.Search.order  != place:
					self.mucous.Search.order = place
					self.mucous.Search.SortBar()
					if self.mucous.Search.current != None:
						self.mucous.Search.FormatResults(self.mucous.Search.current)
					curses.doupdate()
			elif self.input == "methods":
				self.MethodSwitch(direction)
		
	## Parse input entry line for seaerch
	# @param self is Search (Class)
	# @param line is a text string
	def LineInput(self, line):
		
		if self.mucous.mode != "search":
			return
		
		if self.input == "default" and line != "":
			# Normal Search
			query = line
			
			if self.method == "globally":
				if len(query) > 2 and query != 'mp3':
					self.mucous.D.Search(0, query )
			# Buddies Search
			elif self.method == "buddies":
				self.mucous.D.Search(1, query )
			# Rooms Search	
			elif self.method == "rooms":	
				self.mucous.D.Search(2, query )
			elif self.method == "user":
				if self.username != None:
					self.mucous.D.UserSearch(self.username, query )
			elif self.method == "wishlist":	
				self.mucous.D.WishListSearch(query )
		elif self.input == "user":
			if line == "":
				line = None
			self.username = line
		elif self.input == "results":
			position = self.scrolling-self.dimensions["start"]
			if len(self.viewing)-1 >= position:
				number = self.viewing[position]
				user, path = self.GetDownloadFromNum(number)
				self.DownloadSearch(user, path)
		elif self.input == "reverse":
			if self.reverse == True:
				self.reverse = False
			elif self.reverse == False:
				self.reverse= True
		elif self.input == "filter":
			if line == "":
				line = None
			self.sfilter = line
		if self.input != "default":
			self.SetInput()
				
	def SetInput(self, input=None):
		if input == None:
			self.input = "default"
			self.mucous.UseAnotherEntryBox()
		else:
			self.input = input
		self.Mode()
		
	## Mouse Coordinates in the Search Mode
	# @param self is Search (Class)
	# @param x is the horizontal position from the left
	# @param y is the vertical postion from the top
	# @param z is unimportant
	# @param event is the mouse event (button, doubleclick, etc) represented by a number
	def Mouse(self, x,y,z,event):
		try:
			
			w = self.dimensions
			change = 0
			
			if y >= w["top"] and y < w["top"] + w["height"] and x >= w["left"] and x < w["left"] +w["width"]:
				if self.current != None:
					#self.mucous.Help.Log("debug", "%d:%d::%d" %(x,y,w["top"]) )
					y1 = y- w["top"]
					if self.input != "results":
						self.input = "results"
						self.Mode()
						return
					if y1  + w["start"] in range(self.numresults[self.current]):
						self.scrolling  = y1  + w["start"]
						self.Draw()
						if event in ( 4096, 16384):
							self.mucous.PopupMenu.Create("search", 0, True)
							return

			elif y in (1, 2, 3):
				if len(self.tickets.keys()) >= 1:
					if self.current == None:
						self.current = self.tickets.keys()[0]

					self.current, match = self.mucous.edit.MouseClickTab(x, self.current)	
						
					if match == None:
						s = self.tickets.keys().index(self.current)
						self.current = self.tickets.keys()[s-1]
					self.input = "tabs"
					self.Mode()
					return
#(*) Globally ( ) Buddies ( ) Rooms ( ) Wishlist ( ) User:

			elif y == self.mucous.h-5:
				if x < 59:
					# Toggle type of search
					oldmethod = self.method
					if x < 13:
						self.method = "globally"
					elif x < 25:
						self.method = "buddies"
					elif x < 35:
						self.method = "rooms"
					elif x < 50:
						self.method = "wishlist"
					elif x < 59:
						self.method = "user"
					if self.method != oldmethod:
						self.Mode()
					#self.MethodSwitch()
				elif x >= 59:
					self.input = "user"
					self.Mode()
					return
					
			elif y == self.mucous.h-7:
				#m< Num|User|Free|Speed|Que|Path|Size|File|Bitrate|Time >< Reverse >

				
				if x >= 2 and x <= 6 and self.order != "num":
					self.order = "num"
					change = 1
				elif x >= 7 and x <= 11 and self.order != "user":
					self.order = "user"
					change = 1
				elif x >= 12 and x <= 16  and self.order != "free":
					self.order = "free"
					change = 1
				elif x >= 17 and x <= 22  and self.order != "speed":
					self.order = "speed"
					change = 1
				elif x >= 23 and x <= 26  and self.order != "que":
					self.order = "que"
					change = 1
				elif x >= 27 and x <= 31  and self.order != "path":
					self.order = "path"
					change = 1
				elif x >= 33 and x <= 36  and self.order != "size":
					self.order = "size"
					change = 1
				elif x >= 37 and x <= 41  and self.order != "file":
					self.order = "file"
					change = 1
				elif x >= 42 and x <= 49  and self.order != "bitrate":
					self.order = "bitrate"
					change = 1
				elif x >= 50 and x <= 54  and self.order != "time":
					self.order = "time"
					change = 1
				elif x >= 56 and x <= self.mucous.w-10:
					if self.reverse == True:
						self.reverse = False
					elif self.reverse == False:
						self.reverse= True
					change = 1
				elif x >=self.mucous.w-10 and x < self.mucous.w-1:
					self.Close(self.current)
				
			elif y == 4:
				if x > 15 and x < 36:
					self.input="filter"
					self.Mode()
				elif x >=self.mucous.w-18:
					if self.current != None:
						self.current=None
						change = 1
			else:
				if self.input != "default":
					change = 1
			if change == 1:
				self.input = "default"
				self.Mode()
			
		except Exception, e:
			self.mucous.Help.Log("debug", "Search.Mouse: " +str(e) )
			

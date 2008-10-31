# This is part of the Mucous Museek Client, and distributed under the GPLv2
# Copyright (c) 2006 daelstorm. 
import curses.wrapper
## Create, Display and do things with Popup menus
#
class PopupMenu:
	## Constructor
	# @param self PopupMenu
	# @param mucous mucous (parent)
	def __init__(self, mucous):
		## @var mucous 
		# Mucous (Class)
		self.mucous = mucous
		## @var show
		# menu is redrawn after a refresh_windows
		self.show = False
		## @var current
		# current menu
		self.current = None
		## @var position
		# Vertical position in menu 
		self.position = 0
		## @var menus
		# Menu dict of dicts for each menu with coordinates, 3 windows, and a list of items
		# subdicts' keys are: window, scroll, shadow, name, items, top, width, height, left
		self.menus = {}
		
	
	## Redraw Windows and (maybe) Menu
	# @param self PopupMenu
	def Refresh(self):
		try:
			self.mucous.refresh_windows()
		except Exception, e:
			self.mucous.Help.Log("debug", "PopupMenu.Refresh: " + str(e))
			
	## Redraw Windows and close Menu
	# @param self PopupMenu
	def Clear(self):
		try:
			self.show = False
			self.current = None
			self.Refresh()
		except Exception, e:
			self.mucous.Help.Log("debug", "PopupMenu.Clear: " + str(e))
			
	def Scroll(self, key):
		if self.mucous.mode in ("chat", "lists", "transfer", "search", "browse") and self.show == True:
			# POPUP menu up and down keys
			try:
				if self.current == None:
					return
				if key == "menu_up":
					if self.position >0:
						self.position -= 1
						self.Draw()
						
				elif key == "menu_down":
					if self.position < len(self.menus[self.current]['items'])-1:
						self.position += 1
						self.Draw()
			except Exception, e:
				pass
					
	## Return the correct list for the current menu
	# @param self PopupMenu
	# @return list 
	def List(self):
		try:
			mode = self.mucous.mode 
			if self.current == "encoding":
				return [""]
			if mode == "chat":
				this_list = self.mucous.ChatRooms.logs["roombox"][self.mucous.ChatRooms.current]
			elif mode  == "lists":
				this_list = self.mucous.UsersLists.logs[self.mucous.UsersLists.current]
			elif mode == "transfer":
				this_list = self.mucous.Transfers.get_transfers_list()
			elif mode == "search":
				this_list = self.mucous.Search.viewing
			elif mode  == "browse":
				if self.mucous.BrowseShares.current != None:
					if self.mucous.BrowseShares.selected == "files":
						this_list = self.mucous.BrowseShares.files
					elif self.mucous.BrowseShares.selected == "directories":
						this_list = self.mucous.BrowseShares.dirs
				
			return this_list
		except Exception, e:
			self.mucous.Help.Log("debug", "PopupMenu.List " +str(e))
			
	## Map mouse-clicks to events
	# @param self PopupMenu
	# @param x vertical
	# @param y horizontal
	# @param event is the mouse event (button, doubleclick, etc) represented by a number
	def Mouse(self, x, y, event):
		try:
			menu = self.menus[self.current]
			width = menu["width"]
			top = menu["top"]
			height = menu["height"]
			left = menu["left"]
			
			if x >= left and x < width + left and y >= top and y < top + height:
				if y > top and y < top + height -1:
					y -= top+1
					#self.error_bar("y"+str(y) + " x"+ str(x))
					self.position = y
					self.Draw()
					if event in (2, 8, 16384):
						
						s = self.Execute()
						if s == 0:
							self.mucous.refresh_windows()
							self.show = False
							self.current = None
						elif s == 3:
							self.show = False
							self.current = None
			else:
				self.show = False
				self.current = None
				self.Refresh()
		except Exception, e:
			self.mucous.Help.Log("debug", "PopupMenu.Mouse: " + str(e))
			
	## Run function associated with current menu position
	# @param self PopupMenu
	def Enter(self):
		try:
			s = self.Execute()
			if s == 0:
				self.mucous.refresh_windows()
				self.show = False
				self.current = None
			elif s == 3:
				self.show = False
				self.current = None
			else:
				self.Draw()
			#self.menu()
		except Exception, e:
			self.mucous.Help.Log("debug", "PopupMenu.Enter: " + str(e))

	## Build Current Menu and display it
	# @param self PopupMenu
	def Draw(self):
		try:
			menu = self.current
			self.menus[menu]["scroll"].erase()
			y = 0

			clipped_list, self.position, start = self.mucous.FormatData.scrollbox(self.menus[menu]["items"], self.position, self.menus[menu]["height"]-2)
			#for z in self.menus[menu]["items"]:
			for z in clipped_list:
				try:
					username = self.menus[menu]["name"]
					x = 0
					## Color asterixs in from of user list toggles
					if z[1] in ("banned", "ignored", "buddies", "trusted"):
						
							 
						r = 0
						if self.mucous.config.has_key(z[1]):
							if self.mucous.config[z[1]].has_key(username):
								r = 1

						if r == 1:
							self.menus[menu]["scroll"].addstr(y, x, "* ", self.mucous.colors["green"])
						else:
							self.menus[menu]["scroll"].addstr(y, x, "* ", self.mucous.colors["red"])
						x = 2
						spaces = " " * (self.menus[menu]["width"]-2 - len(z[1]) -2)
						line = z[1].capitalize()+spaces
					## Draw a Horizontal line
					elif z[1] == "line":
						line =" "  * (self.menus[menu]["width"]-2)
						
						self.menus[menu]["window"].addch(y+1, 0, curses.ACS_LTEE, self.mucous.colors["green"])
						self.menus[menu]["window"].addch(y+1, self.menus[menu]["width"]-1, curses.ACS_RTEE, self.mucous.colors["green"])
						
						if self.position == y + start:
							self.menus[menu]["scroll"].hline(y, x,  curses.ACS_HLINE, self.menus[menu]["width"]-2, self.mucous.colors["green"])
						else:
							self.menus[menu]["scroll"].hline(y, x, curses.ACS_HLINE, self.menus[menu]["width"]-2)
						y += 1
						continue
					
					else:
						## Padd line with spaces
						spaces = " " * (self.menus[menu]["width"]-2 - len(z[0]) )
						line = z[0] + spaces
					## remove last character from last item in line
					if z is clipped_list[-1]:
						line = line[:-1]
					if self.position == y + start:
						self.menus[menu]["scroll"].addstr(y, x, line, self.mucous.colors["green"])
					else:
						self.menus[menu]["scroll"].addstr(y, x, line)
				except Exception, e:
					self.mucous.Help.Log("debug", "PopupMenu.Draw " +str(e))
					pass
				y += 1
			self.menus[menu]["window"].noutrefresh()
			self.menus[menu]["scroll"].noutrefresh()
			curses.doupdate()
		except Exception, e:
			self.mucous.Help.Log("debug", "PopupMenu.Draw " +str(e))
	
	## Create Package menu
	# @param self PopupMenu
	# @param menu New Current Menu
	# @param position vertical position of menu item
	# @param show Show this menu
	def Create(self, menu=None, position=None, show=False):
		try:
			if menu:
				self.current = menu
				self.position = 0
			else:
				menu = self.current 
			if position is not None:
				self.position = position
			
			if show:
				self.show = True
			
			
			title = None
			number = None
			
			this_list = self.List()
			if this_list == []:
				return
			
			if menu == "roombox":
				mode = "roombox"
				if self.mucous.ChatRooms.shape == "rightlist":
					left = self.mucous.ChatRooms.dimensions["roombox"]["left"] - 22
				else:
					left = self.mucous.ChatRooms.dimensions["roombox"]["left"]+self.mucous.ChatRooms.dimensions["roombox"]["width"]+1
				top = 2; width = 20
				title = "%s" % (self.mucous.ChatRooms.logs["roombox"][self.mucous.ChatRooms.current][self.mucous.ChatRooms.scrolling[mode]])[:16]
				user = name = self.mucous.ChatRooms.logs["roombox"][self.mucous.ChatRooms.current][self.mucous.ChatRooms.scrolling[mode]]
				if user in self.mucous.user["statistics"].keys():
					number = str(self.mucous.user["statistics"][user][2]) + " Files"
				items = [["Private Message", "all"], ["Userinfo", "all"], ["IP Address", "all"], ["Browse", "all"], ["", "line"], ["Buddy", "buddies"], ["Ban", "banned"], ["Ignore", "ignored"], ["Trusted", "trusted"], ["Give Privileges", "giveprivs"]  ]
				height = len(items) + 2	
			elif menu == "lists":
				mode = self.mucous.UsersLists.current
				left = 25; top = 2; width = 20
				title = "%s" % (this_list[self.mucous.UsersLists.scrolling[mode]])[1][:16]
				name = this_list[self.mucous.UsersLists.scrolling[mode]][1]
				number = self.mucous.UsersLists.scrolling[mode] +1
				
				items = [["Private Message", "all"], ["Userinfo", "all"], ["IP Address", "all"], ["Browse", "all"], ["", "line"], ["Buddy", "buddies"], ["Ban", "banned"], ["Ignore", "ignored"], ["Trusted", "trusted"], ["Give Privileges", "giveprivs"]  ]
				height = len(items) + 2
				
			elif menu == "transfers":
				mode = self.mucous.Transfers.current
				left = 25; top = 1; width = 20
				
				title = "%s" % this_list[self.mucous.Transfers.scrolling[mode]][1][:16]
				name = this_list[self.mucous.Transfers.scrolling[mode]][1]
				
				number = self.mucous.Transfers.get_transfer_number(this_list)
				if self.mucous.Transfers.current == "downloads":
					items = [["Retry", "transfers"], ["Retry All", "transfers"],["Abort", "transfers"], ["Abort User", "transfers"], ["Clear", "transfers"], ["Clear Finished", "transfers"], ["Clear User", "transfers"], ["", "line"], ["Private Message", "all"], ["Userinfo", "all"], ["IP Address", "all"], ["Browse", "all"], ["", "line"], ["Buddy", "buddies"], ["Ban", "banned"], ["Ignore", "ignored"], ["Trusted", "trusted"], ["Give Privileges", "giveprivs"],  ["Get Queue Place", "getplace"] ]
				else:
					items = [["Retry", "transfers"], ["Retry All", "transfers"],["Abort", "transfers"], ["Abort User", "transfers"], ["Clear", "transfers"], ["Clear Finished", "transfers"], ["Clear User", "transfers"], ["", "line"], ["Private Message", "all"], ["Userinfo", "all"], ["IP Address", "all"], ["Browse", "all"], ["", "line"], ["Buddy", "buddies"], ["Ban", "banned"], ["Ignore", "ignored"], ["Trusted", "trusted"], ["Give Privileges", "giveprivs"]  ]
				height = len(items) + 2	
			elif menu == "search":
				mode = "search"
				left = 25; top = 5; width = 20
				number = this_list[self.mucous.Search.scrolling-self.mucous.Search.dimensions["start"]]
				name, path = self.mucous.Search.GetDownloadFromNum(number)
				title = "%s" % name[:14]
				#name = self.mucous.Search.results[self.mucous.Search.current][number][1]
				
				items = [ ["Download", "download"], ["Download Dir", "downdir"], ["Display Full Path", "displaypath"], ["", "line"], ["Private Message", "all"], ["Userinfo", "all"], ["IP Address", "all"], ["Browse", "all"], ["", "line"], ["Buddy", "buddies"], ["Ban", "banned"], ["Ignore", "ignored"], ["Trusted", "trusted"], ["Give Privileges", "giveprivs"]]
				height = len(items) + 2	
			elif menu == "browse-dirs":
				mode = "directories"
				left = 23; top = 5; width = 20
				#number = this_list[self.scrolling[mode]-self.windows["dimensions"]["directories"]["start"]]
				title = "Directory" 
				name = self.mucous.BrowseShares.current
				
				items = [ ["Download Dir", "downdir"], ["Display Full Path", "displaypath"], ["", "line"], ["Private Message", "all"], ["Userinfo", "all"], ["IP Address", "all"], ["Refresh Shares", "all"], ["", "line"], ["Buddy", "buddies"], ["Ban", "banned"], ["Ignore", "ignored"], ["Trusted", "trusted"],  ["Give Privileges", "giveprivs"]]
				height = len(items) + 2	
			elif menu == "browse-files":	
				mode = "files"
				left = 2; top = 5; width = 20
				number = self.mucous.BrowseShares.scrolling[mode] + 1 
				title = " File "
				name = self.mucous.BrowseShares.current
				dir = self.mucous.BrowseShares.current_dir
				#print self.mucous.BrowseShares.files[number-1]
				#title = self.mucous.BrowseShares.files[number -1]
				
				items = [ ["Download", "download"], ["Download Dir", "downdir"], ["Display Full Path", "displaypath"], ["", "line"], ["Private Message", "all"], ["Userinfo", "all"], ["IP Address", "all"], ["Browse", "all"], ["", "line"], ["Buddy", "buddies"], ["Ban", "banned"], ["Ignore", "ignored"], ["Trusted", "trusted"], ["Give Privileges", "giveprivs"]]
				height = len(items) + 2	
			elif menu == "encoding":
				left = self.mucous.w-27; top = 2; width = 20
				number =  None
				title = "Encoding"
				
				if self.mucous.mode == "browse":
					if self.mucous.BrowseShares.current != None:
						name = self.mucous.BrowseShares.current
					else:
						name = "Filesystem"
				elif self.mucous.mode == "chat":
					name = self.mucous.ChatRooms.current
				elif self.mucous.mode == "private":
					name = self.mucous.PrivateChat.current
					
				items = [ ["UTF-8", ""], ["iso-8859-1", ""], ["iso-8859-2", ""], ["iso-8859-3", ""], ["iso-8859-4", ""], ["iso-8859-5", ""], ["iso-8859-6", ""], ["iso-8859-7", ""], ["iso-8859-8", ""], ["iso-8859-9", ""], ["iso-8859-10", ""], ["iso-8859-11", ""], ["iso-8859-13", ""],["iso-8859-14", ""],["iso-8859-15", ""], ["iso-8859-16", ""],["KIO8-R", ""], ["CP1250", ""], ["CP1251", ""],["CP1252", ""],["CP1253", ""],["CP1254", ""],["CP1255", ""],["CP1256", ""],["CP1257", ""],["CP1258", ""],["CP874", ""] ]
				if  len(items)+2 > self.mucous.h-9:
					height = self.mucous.h-9
				else:
					height = len(items)+2
			# Cleanup stale windows
			if menu in self.menus:
				if "scroll" in self.menus[menu]:
					del self.menus[menu]["scroll"]
				if "shadow" in self.menus[menu]:
					del self.menus[menu]["shadow"]
				if "window" in self.menus[menu]:
					del self.menus[menu]["window"]
			shadow = curses.newwin(height+1, width+1, top, left)
			shadow.erase()
			shadow.noutrefresh()
			win = curses.newwin(height, width, top, left)
			win.attron(self.mucous.colors["green"] | curses.A_BOLD)
# 			win.bkgdset(" ", self.colors["green"])
			win.erase()
			win.border()
			if title != None:
				win.addstr(0, 1,  "<%s>" % title)
			if number != None:
				win.addstr(height-1, 1,  "< %s >" % number)
			win.attroff(self.mucous.colors["green"] | curses.A_BOLD)
			win.noutrefresh()
			scroll = win.subwin(height-2, width-2, top+1, left+1)
# 			scroll.erase()
			scroll.scrollok(0)
# 			scroll.noutrefresh()
			self.menus[menu] = {"window": win, "shadow": shadow, "scroll": scroll, "name": name, "items": items, "top": top, "width": width, "height": height, "left": left}
			self.Draw()
			curses.doupdate()
			#sleep(1)
		except Exception, e:
			self.mucous.Help.Log("debug", "PopupMenu.Create " +str(e))
			
	## Select new encoding based on position
	# @param self PopupMenu
	# @return 0
	def ExecuteEncodingsMenu(self):
		if self.position == 0: coding = "UTF-8"
		elif self.position == 1: coding = "iso-8859-1"  
		elif self.position == 2: coding = "iso-8859-2"  
		elif self.position == 3: coding = "iso-8859-3"  
		elif self.position == 4: coding = "iso-8859-4"  
		elif self.position == 5: coding = "iso-8859-5"  
		elif self.position == 6: coding = "iso-8859-6"  
		elif self.position == 7: coding = "iso-8859-7" 
		elif self.position == 8: coding = "iso-8859-8"  
		elif self.position == 9: coding = "iso-8859-9"  
		elif self.position == 10: coding = "iso-8859-10"  
		elif self.position == 11: coding = "iso-8859-11"  
		elif self.position == 12: coding = "iso-8859-13" 
		elif self.position == 13: coding = "iso-8859-14" 
		elif self.position == 14: coding = "iso-8859-15"  
		elif self.position == 15: coding = "iso-8859-16" 
		elif self.position == 16: coding = "KIO8-R" 
		elif self.position == 17: coding = "CP1250" 
		elif self.position == 18: coding = "CP1251" 
		elif self.position == 19: coding = "CP1252" 
		elif self.position == 20: coding = "CP1253" 
		elif self.position == 21: coding = "CP1254" 
		elif self.position == 22: coding = "CP1255" 
		elif self.position == 23: coding = "CP1256" 
		elif self.position == 24: coding = "CP1257" 
		elif self.position == 25: coding = "CP1258"
		elif self.position == 26: coding = "CP874" 
		#self.Help.Log("debug", coding)
		if self.mucous.mode == "chat":
			self.mucous.D.ConfigSet("encoding.rooms", self.mucous.ChatRooms.current, coding)
		elif self.mucous.mode == "private":
			self.mucous.D.ConfigSet("encoding.users", self.mucous.PrivateChat.current, coding)
		elif self.mucous.mode == "browse":
			if self.mucous.BrowseShares.current != None:
				self.mucous.D.ConfigSet("encoding.users", self.mucous.BrowseShares.current, coding)
			else:
				self.mucous.D.ConfigSet("encoding", "filesystem", coding)
		return 0
	
	## Do an action to the selected transfer
	# @param self PopupMenu
	# @return 1 / 0
	def ExecuteTransfers(self):
		try:
			mode = self.mucous.Transfers.current
			the_list = self.List()
			item_num = self.mucous.Transfers.scrolling[mode]
			username = the_list[item_num][1] 
			
			path = the_list[item_num][2]
			userpath = (username, path)
			transfer = self.mucous.Transfers.get_transfer_number(the_list)
			if transfer == None:
				return 1
			if self.position == 0:
				# RETRY DOWNLOAD
				
				if self.mucous.Transfers.current != "downloads":
					return 1
				if transfer in self.mucous.Transfers.downloads.keys():
					for username, path in self.mucous.Transfers.downloads[transfer].items():
						self.mucous.Help.Log("status", "Retrying download: [%s] %s" % (username, path))
						self.mucous.D.DownloadFile(username, path)
				else:
					self.mucous.Help.Log("status", "No such transfer #" + str(transfer))
				return 0
			elif self.position == 1:
				# RETRY ALL DOWNLOADS
				if self.mucous.Transfers.current != "downloads":
					return 1
				for user_path, transfer  in self.mucous.Transfers.transfers["downloads"].items():
					if int(transfer[3]) in (10, 11, 12, 13, 14):
						self.mucous.D.DownloadFile(transfer[1], transfer[2])
				return 0
			elif self.position == 2:
				# ABORT TRANSFER
				if self.mucous.Transfers.current == "downloads":
					if transfer in self.mucous.Transfers.downloads.keys():
						for username, path in self.mucous.Transfers.downloads[transfer].items():
							self.mucous.Help.Log("status", "Aborting download: [%s] %s" % (username, path))
							self.mucous.D.TransferAbort(0, username, path)
					else:
						self.mucous.Help.Log("status", "No such transfer #" + str(transfer))
				elif self.mucous.Transfers.current == "uploads":
					if transfer in self.mucous.Transfers.uploads.keys():
						for username, path in self.mucous.Transfers.uploads[transfer].items():
							self.mucous.Help.Log("status", "Aborting upload: [%s] %s" % (username, path))
							self.mucous.D.TransferAbort(1, username, path)
					else:
						self.mucous.Help.Log("status", "No such transfer #" + str(transfer))
				return 0
			elif self.position == 3:
				# ABORT USER's TRANSFER(s)
				if self.mucous.Transfers.current == "downloads":
					for userpath, values in self.mucous.Transfers.transfers["downloads"].items():
						if userpath[0] == username:
							self.mucous.Help.Log("status", "Aborting download: [%s] %s" % (username, values[2]))
							self.mucous.D.TransferAbort(0, username, values[2])
	
				elif self.mucous.Transfers.current == "uploads":
					for userpath, values in self.mucous.Transfers.transfers["uploads"].items():
						if userpath[0] == username:
							self.mucous.Help.Log("status", "Aborting upload: [%s] %s" % (username, values[2]))
							self.mucous.D.TransferAbort(1, username, values[2])
	
				return 0
			elif self.position == 4:
				# Clear
				if self.mucous.Transfers.current == "downloads":
					if transfer in self.mucous.Transfers.downloads.keys():
						for username, path in self.mucous.Transfers.downloads[transfer].items():
							self.mucous.Help.Log("status", "Removing download: [%s] %s" % (username, path))
							self.mucous.D.TransferRemove(0, username, path)
					else:
						self.mucous.Help.Log("status", "No such transfer #" + str(transfer))
					
				elif self.mucous.Transfers.current == "uploads":
					if transfer in self.mucous.Transfers.uploads.keys():
						for username, path in self.mucous.Transfers.uploads[transfer].items():
							self.mucous.Help.Log("status", "Removing upload: [%s] %s" % (username, path))
							self.mucous.D.TransferRemove(1, username, path)
					else:
						self.mucous.Help.Log("status", "No such transfer #" + str(transfer))
				return 0
			elif self.position == 5:
				# Clear ALL FINISHED/FAILED
				if self.mucous.Transfers.current == "downloads":
					for userpath, values in self.mucous.Transfers.transfers["downloads"].items():
						if values[3] == 0:
							self.mucous.D.TransferRemove(0, values[1], values[2])
				elif self.mucous.Transfers.current == "uploads":
					for userpath, values in self.mucous.Transfers.transfers["uploads"].items():
						if values[3] in (0, 10, 11, 12, 13, 14):
							self.mucous.D.TransferRemove(1, values[1], values[2])
				return 0
			elif self.position == 6:
				# Clear USER's Transfers
				if self.mucous.Transfers.current == "downloads":
					for userpath, values in self.mucous.Transfers.transfers["downloads"].items():
						if userpath[0] == username:
							self.mucous.D.TransferRemove(0, values[1], values[2])
				elif self.mucous.Transfers.current == "uploads":
					for userpath, values in self.mucous.Transfers.transfers["uploads"].items():
						if userpath[0] == username:
							self.mucous.D.TransferRemove(1, values[1], values[2])
				return 0
			elif self.position == 7:
				return 1
			else:
				self.position -=8
				return [username, path]
			
		except Exception, e:
			self.mucous.Help.Log("debug", "PopupMenu.ExecuteTransfers " +str(e))
	
	## Do an action to the selected file or directory
	# @param self PopupMenu
	# @return 1 /0
	def ExecuteBrowse(self):
		try:
			the_list = self.List()
			#username = self.mucous.BrowseShares.current
			user, path = self.mucous.BrowseShares.CurrentFile()
			mode = self.mucous.BrowseShares.selected
			if mode == "directories":
				
				number = self.mucous.BrowseShares.scrolling[mode]-self.mucous.BrowseShares.dimensions["directories"]["start"]
				if self.position == 0:
					# Download
					# CHANGED 12/17/2007 Josh Leder 
					# Transfers.FolderDownload strips the end of the path for some reason 
					# so I am adding a trailing path delimiter (which will be discarded) 
					# to prevent going an extra level up in the folder hierarchy 
					# I'm also changing CurrentFile() to CurrentDir() which seems to make 
					# a lot more sense. :) 
					user, path = self.mucous.BrowseShares.CurrentDir() 
					self.mucous.Transfers.FolderDownload(user, path + "\\")
					return 0
				elif self.position == 1:
					user, path = self.mucous.BrowseShares.CurrentDir()
					self.mucous.Help.Log("status", "[%d] %s" % (number, path))
					return 0
				else:
					self.position -= 2
			elif mode == "files":
				#user, path = self.mucous.BrowseShares.CurrentFile()
				mode = "files"
				number = self.mucous.BrowseShares.scrolling[mode]
				if self.position == 0:
					# Download
					self.mucous.Transfers.RetryDownload(user, path)
					return 0
				elif self.position == 1:
					# Download Dir
					user, path = self.mucous.BrowseShares.CurrentDir() 
					self.mucous.Transfers.FolderDownload(user, path + "\\") 
					return 0
			
				elif self.position == 2:
					self.mucous.Help.Log("status", "[%d] %s" % (number+1, path))
					return 0
				else:
					self.position -= 3

			if self.position == 0:
				return 1
			else:
				self.position -=1
				return user
		except Exception, e:
			self.mucous.Help.Log("debug", "PopupMenu.ExecuteBrowse " +str(e))			
			
	## Call functions associated with the menu item and the list item the menu is connected to
	# @param self PopupMenu
	# @return 1 /0
	def Execute(self):
		try:
			## ENCODINGS
			if self.current == "encoding":
				return self.ExecuteEncodingsMenu()
					
			## The rest of the popup menus
			path = username = None
			if self.current not in ("roombox", "lists", "transfers", "search", "browse-dirs", "browse-files"):
				return 0
			if self.current == "roombox":
				mode = "roombox"
				username = self.mucous.ChatRooms.logs["roombox"][self.mucous.ChatRooms.current][self.mucous.ChatRooms.scrolling[mode]]
			elif self.current == "lists":
				mode = self.mucous.UsersLists.current
				username = self.List()[self.mucous.UsersLists.scrolling[mode]][1]
			elif self.current == "transfers":
				value  = self.ExecuteTransfers()
				if type(value) is int:
					return value
				elif type(value) is list:
					username = value[0]
					path = value[1]
			elif self.current == "search":
				the_list = self.List()
				mode = "search"
				number = the_list[self.mucous.Search.scrolling-self.mucous.Search.dimensions["start"]]
				username, path = self.mucous.Search.GetDownloadFromNum(number)
								
				userpath = (username, path)
				
				if self.position == 0:
					# Download

					self.mucous.Search.DownloadSearch(username, path)
					return 0
				elif self.position == 1:
					# Download Dir
					self.mucous.Search.DownloadSearch(username, path, True)
					return 0
				elif self.position == 2:
					self.mucous.Help.Log("status", "[%d] %s" % (number, path))
					return 0
				elif self.position == 3:
					# Line
					return 1
				else:
					self.position -= 4
					
			elif self.current in ("browse-dirs", "browse-files"):
				a = self.ExecuteBrowse()
				if a in (0, 1):
					return a
				else: username = a
			if username is None:
				return 1
			if self.position == 0:
				self.mucous.PrivateChat.Start(username)
				self.mucous.PrivateChat.Mode()
			elif self.position == 1:
				self.mucous.UserInfo.Get(username)

				self.mucous.UserInfo.Mode()
			elif self.position == 2:
				self.mucous.requests["ip"].append(username)
				self.mucous.D.PeerAddress(username)
				self.mucous.Help.Mode()
			elif self.position == 3:
				self.mucous.BrowseShares.Get(username)
			elif self.position == 4:
				return 1
			elif self.position == 5:
				if not self.mucous.config.has_key("buddies") or username not in self.mucous.config["buddies"].keys():
					self.mucous.ModifyConfig("buddy", username, '')
				else:
					self.mucous.ModifyConfig("unbuddy", username, '')
				return 1
			elif self.position == 6:
				if not self.mucous.config.has_key("banned") or username not in self.mucous.config["banned"].keys():
					self.mucous.ModifyConfig("ban", username, '')
				else:
					self.mucous.ModifyConfig("unban", username, '')
				return 1
			elif self.position == 7:
				if not self.mucous.config.has_key("ignored") or username not in self.mucous.config["ignored"].keys():
					self.mucous.ModifyConfig("ignore", username, '')
				else:
					self.mucous.ModifyConfig("unignore", username, '')
				return 1
			elif self.position == 8:
				if not self.mucous.config.has_key("trusted") or username not in self.mucous.config["trusted"].keys():
					self.mucous.ModifyConfig("trust", username, '')
				else:
					self.mucous.ModifyConfig("distrust", username, '')
				return 1
			elif self.position == 9:
				self.mucous.usernames["privileges"] = username
				self.mucous.SetEditTitle( "% Give Privileges to " + self.mucous.usernames["privileges"])
			elif self.position == 10:
				self.mucous.D.TransferUpdate(username, path) 
			return 0
		except Exception, e:
			self.mucous.Help.Log("debug", "PopupMenu.Execute " +str(e))



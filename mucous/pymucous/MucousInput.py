# This is part of the Mucous Museek Client, and distributed under the GPLv2
# Copyright (c) 2006 daelstorm.
import sys
import os  
import threading
import curses.wrapper
import re
## Input line 
# :: Here we detect which keys have been pressed
#class CharacterParse(threading.Thread):
class CharacterParse:
	## Constructor
	# @param self is CharacterParse
	# @param mucous is the Mucous class
	def __init__(self, mucous):
		
		#threading.Thread.__init__(self)
		## @var mucous
		# Mucous (Class)
		self.mucous = mucous
		## @var win
		# Input line curses instance 
		self.win = None
		
		## @var scroll
		# cursor position
		self.scroll = 0
		self.x = self.scroll
		## @var line
		# string containing the inputed data
		self.line = mucous.line
		## @var length
		# length of line
		self.length = len(self.line)
		## @var escape
		# was an escape key (Alt/Meta/Escape) pressed? (True/False)
		self.escape = False
		## @var word
		# last word in line
		self.word = None
		## @var listline
		# line split up into a list
		self.listline = mucous.listline
		## @var firsttab
		# was the tab pressed before? (True/False)
		self.firsttab = 0
		self.wrap = False
		#self.fixpos()
		
	def SelectEntryBox(self, window, contents=None, wrap=None):
		self.win = window
		## @var h
		# height of input window
		## @var w
		# width of input window
		self.h, self.w = self.win.getmaxyx()
		if contents:
			self.line = contents.replace("\n", "\\n").replace("\t", "\\t")
		else:
			self.line = ""
		self.length = len(self.line)
		self.scroll = self.length
		if wrap != None:
			self.wrap = wrap
		else:
			self.wrap = False
		self.fixpos()
		
	## Add normal characters to the line, or respond to escaped characters or mouse presses
	# @param self CharacterParse
	# @param c the character recieved by getkey
	# @return (True/False)
	def process(self, c):
		if self.win == None:
			return
		try:
			#pos = self.scroll
			#self.length + 
			self.length = len(self.line)
			line = self.line
			# debugging: display keypress
			#try:self.mucous.Help.Log("debug", str(c)+" "+str(chr(c)) )
			#except:pass
			
			# Toggle online ONLY if inactivity timeout was met
			if self.mucous.timedout == True:
				self.mucous.ToggleAwayStatus()
				self.mucous.timedout = False
			if self.mucous.Spl["status"] == 0 and self.mucous.Config["mucous"]["autoaway"] == "yes":
				# Restart inactivity timeout for every key or mousepress if not away, currently
				self.mucous.timers["timeout"].cancel()
				self.mucous.timers["timeout"] = threading.Timer(self.mucous.timeout_time, self.mucous.AwayTimeout)
				self.mucous.timers["timeout"].start()
			else:
				self.mucous.timers["timeout"].cancel()

			if c != chr(9) and c !="KEY_MOUSE":  # Clear self.word if tab wasn't pressed
				self.word = None
				self.firsttab = 0
				self.listline = []
			elif c not in ("KEY_UP", "KEY_DOWN"):
				self.mucous.Spl["history_count"] = -1
				
			if c == "KEY_RESIZE":
				self.mucous.stdscr.keypad(1)
				self.mucous.Build()
				self.line = line
				
			elif c == "r" and self.escape:
				self.mucous.Build()
				self.line = line
				
			elif c == "KEY_MOUSE":
				error = 'mouse'
				if not self.escape:
					line = self.InputFunctions(c, self.line)
					if line != None:
						self.line = line
						self.length = len(self.line)
			elif c == "KEY_LEFT" or c == chr(2):
				error = 'left'
				if self.escape:
					self.InputFunctions(c, self.line)
				else:
					#self.length -= 1
					self.scroll -= 1
			
			elif c == "KEY_RIGHT" or c == chr(6):
				error = 'right'
				if self.escape:
					self.InputFunctions(c, self.line)
				else:
					#self.length += 1
					self.scroll += 1
			elif c in ("KEY_F(1)", "KEY_F(2)", "KEY_F(3)", "KEY_F(4)", "KEY_F(5)", "KEY_F(6)", "KEY_F(7)", "KEY_F(8)", "KEY_F(9)", "KEY_F(10)"):
				if not self.escape:
					self.InputFunctions(c, self.line)
				
			elif c in ("KEY_UP", "KEY_DOWN"):
				# Scrolling
				if not self.escape:
					
					line = self.InputFunctions(c, self.line)
					if line != None:
						self.line = line
						self.length = len(self.line)
					
				elif self.escape:
					# Alt+Up/Down
					line, self.mucous.Spl["history_count"] = self.InputHistory(c, self.line, self.mucous.Spl["history_count"])
					if line is not None:
						self.line = line
						self.length = len(self.line)
			elif c in ("KEY_SELECT", "KEY_FIND", "KEY_PPAGE", "KEY_NPAGE", "KEY_HOME", "KEY_END"): 
				# Scrolling
				if c == "KEY_SELECT":
					c = "KEY_NPAGE"
				elif c == "KEY_FIND":
					c = "KEY_PPAGE"
				if not self.escape:
					line = self.InputFunctions(c, self.line)
					if line != None:
						self.line = line
						self.length = len(self.line)
			elif c == "KEY_IC": # Insert
				self.InputFunctions(c, self.line)
			elif c in ("-", "+"):
				if self.mucous.mode == "setup" and self.mucous.Setup.input in self.mucous.Setup.numberboxes:
					self.InputFunctions(c, self.line)
				elif self.mucous.mode == "browse" and self.mucous.BrowseShares.selected == "directories":
					if c == "-":
						self.InputFunctions("collapse", self.line)
					elif c == "+":
						self.InputFunctions("expand", self.line)
				else:
					self.line = self.line[:self.scroll] + c + self.line[self.scroll:]
					self.scroll += 1
			elif c in ("t", "T", "p", "P", "d", "D", "x", "X", "a", "A"):
				if self.escape:
					if c in ("t", "T"):
						self.InputFunctions("switch", self.line)
					elif c in ("d", "D"):
						self.InputFunctions("delete", self.line)
					elif c in ("p", "P"):
						self.InputFunctions("popup", self.line)
					elif c in ("x", "X"):
						self.InputFunctions("expandcollapse", self.line)
					elif c in ("a", "A"):	
						self.mucous.ToggleAwayStatus()
				else:
					self.line = self.line[:self.scroll] + c + self.line[self.scroll:]
					#self.length += 1
					self.scroll += 1
			elif c == chr(1): 
				# Ctrl-A
				self.length = self.scroll = 0
			elif c == chr(5):
				# Ctrl-E
				#self.length = len(self.line)
				self.scroll = len(self.line)
			# elif c == chr(32):
				#self.mucous.Help.Log("debug", "space")
			# elif c == chr(13):
				#pass
				#self.mucous.Help.Log("debug", "carriage")
			elif c == "KEY_ENTER" or c == chr(10):
				# Keypad Enter, Normal Enter, or Newline pasted from another app
				if self.mucous.PopupMenu.show == True:
					self.mucous.PopupMenu.Enter()
				else:
					self.escape = False
					self.InputFunctions(c, self.line)
					return True
			
			
			elif c == chr(9): 
				# Tab
				if self.word == None:
					#self.length
					#w = self.line.split(' ')
					#self.word = w[-1]
					w = self.line[:self.scroll].split(' ')
					
					self.word = w[-1]
					self.listline = self.line.split(" ")
				w = self.line[:self.scroll].split(' ')
				lw = len(w)
				cpos = lw - 1
				currentword = w[cpos]
				if self.firsttab == None:
					self.firsttab = 0
				# if a space is in currentword, it won't be in the listline, so match it with a multi-space word
				#self.mucous.Help.Log("debug", self.word)
				if currentword not in self.listline:
					for word in self.listline:
						if word.upper().endswith(currentword.upper()):
							currentword = word
							cpos = self.listline.index(currentword)
							
				self.listline, self.firsttab, self.word, xpos = self.InputTabCompletion(self.line, self.word, self.firsttab, self.listline, cpos)
				if self.listline == []:
					return False
				self.line = ''
				# position in listline
				posx = 0
				# segments in listline
				ll = len(self.listline) -1
				# put line back together
				for r in self.listline:
					
					if posx != ll:
						self.line += r +' '
						# place cursor at end of current word
						if posx == xpos:
							self.scroll = len(self.line)-1
					elif posx == ll:
						self.line +=r
						# Place cursor at end of line
						if posx == xpos:
							self.scroll = len(self.line)
					posx += 1
				
				#self.length = len(self.line)
				#return False
			elif c == chr(11): 
				# Ctrl-K
				# Delete everything after cursor position
				self.line = self.line[:self.scroll]
				self.length = len(self.line)
				self.scroll = self.length
			elif c == chr(21): 
				# Ctrl-U
				# Delete line
				self.line = ''
				self.length = 0
				self.scroll = 0 
			elif c == chr(23): 
				# Ctrl-W
				# Delete word before cursor
				z = self.line.split(' ')
	
				if len(z) >1:
					if z[-1] != ' ' and z[-1] != '':
						self.line = ''
						for s in z:
							if s is not z[-1]:
								self.line = self.line + s +" "
							elif s is z[-1]:
								self.line = self.line
								break
					else:
						self.line = ''
						for s in z:
							if s not in (z[-1], z[-2]):
								self.line = self.line + s +" "
							elif s is z[-2]:
								self.line = self.line
								break
				else:
					self.line = ''
				#self.length = len(self.line)
				self.scroll = len(self.line) -1
			elif c == chr(27):
				# Escape ^[
				if self.mucous.PopupMenu.show == True:
					self.mucous.PopupMenu.Clear()
				else:
					self.escape = True
					return False
			elif c == chr(93) or c == chr(91) or c == chr(60) or c == chr(62):
				# ], [, <, >
				if self.escape:
					self.InputFunctions(c, self.line)
				else:
					self.line = self.line[:self.scroll] + c + self.line[self.scroll:]
					#self.length += 1
					self.scroll += 1
			elif c == "KEY_DC"  or c == chr(4):
				# Delete
				# Delete letter after cursor
				self.line = self.line[:self.scroll] + self.line[self.scroll+1:]
			elif c == "KEY_BACKSPACE" or c == chr(8) or c == chr(127):
				# Backspace, Ctrl-H
				# Delete letter before cursor
				if self.scroll > 0:
					self.line = self.line[:self.scroll-1] + self.line[self.scroll:]
					#self.length -= 1
					self.scroll -= 1
	
			elif len(c) == 1 and ord(c[0]) >= 32 and ord(c[0]) < 127:
				# ASCII letters 
				self.line = self.line[:self.scroll] + c + self.line[self.scroll:]
				#self.length += 1
				self.scroll += 1
			elif len(c) == 1 and ord(c[0]) > 127 and ord(c[0]) < 256:
				# ISO8859-* characters
				self.line = self.line[:self.scroll] + c + self.line[self.scroll:]
				#self.length += 1
				self.scroll += 1
			#self.scroll = self.scroll
			self.length = len(self.line)
			self.fixpos()
			
			self.mucous.line = self.line
			self.escape = False
			return False
		except Exception, e:
			self.mucous.Help.Log("debug", "CharacterParse process: \""+str(self.line)+"\" "+ str(e))
			
	## Allow for horizontal scrolling of line
	# @param self CharacterParse
	def fixpos(self):
		try:

			if self.scroll > self.length:
				self.scroll = self.length
			if self.scroll < 0:
				self.scroll = 0
			self.x = self.scroll

			self.win.erase()
			#self.mucous.Help.Log("status", "Editwin: "+ str(self.scroll)+" " + self.line)
			try:
				if self.wrap:
					self.win.addstr(self.line)
				else:

					if self.scroll <= self.w:
						start = 0
						end = self.w

					else:

						start = self.scroll - self.w+1

						end = start + self.w-1

						self.x = self.w -1 

						#self.mucous.Help.Log("status", "x: "+ str(self.x))
						#self.mucous.Help.Log("status", "l: "+ str(len(self.line[start:end])) + " line:"+str(len(self.line)))
					self.win.addstr(self.line[start:end])
			except Exception, e:
				pass
				#self.mucous.Help.Log("debug", "Editwin: "+ str(e))
			self.win.refresh()
		except Exception, e:
			self.mucous.Help.Log("debug", "fixpos: \""+str(self.line)+"\" "+ str(e))
			
	## Delete contents of line
	# @param self CharacterParse (Class)
	def reset(self):
		try:
			self.length = self.scroll = self.x =  0
			self.mucous.line = self.line = ""
	
			self.win.erase()
			self.win.refresh()
		except Exception, e:
			self.mucous.Help.Log("debug", "reset: \""+str(self.line)+"\" "+ str(e))


	## Do various things, based on what input was recieved
	# @param self is CharacterParse (Class)
	# @param key keypress
	# @param line edit bar's contents
	def InputFunctions(self, key, line):
		try:
			
				
			if key == "KEY_MOUSE":
				line = self.MouseXY(line)
					
			

	
			elif key == "KEY_F(1)":
				if self.mucous.mode != "chat": self.mucous.ChatRooms.Mode()
				return
			elif key == "KEY_F(2)":
				if self.mucous.mode != "private": self.mucous.PrivateChat.Mode()
				return
			elif key == "KEY_F(3)":
				if self.mucous.mode != "transfer": self.mucous.Transfers.ModeTransfers()
				return
			elif key == "KEY_F(4)":
				if self.mucous.mode != "search": self.mucous.Search.Default()
				return
			elif key == "KEY_F(5)":
				if self.mucous.mode != "info": self.mucous.UserInfo.Mode()
				return
			elif key == "KEY_F(6)":
				if self.mucous.mode != "browse": self.mucous.BrowseShares.Mode()
				return
			elif key == "KEY_F(7)":
				if self.mucous.mode != "lists": self.mucous.UsersLists.ModeLists()
				return
			elif key == "KEY_F(8)":
				if self.mucous.mode != "roomlist": self.mucous.RoomsList.Mode()
				return
			elif key == "KEY_F(9)":
				if self.mucous.mode != "setup": self.mucous.Setup.Default()
				return
			elif key == "KEY_F(10)":
				if self.mucous.mode not in ("help", "debug", "status") : self.mucous.Help.Mode()
				return
			
			elif key in ("KEY_UP",  "KEY_DOWN", "KEY_PPAGE", "KEY_NPAGE"):
				if self.mucous.PopupMenu.show == True:
					if key =="KEY_UP":
						key = "menu_up"
					elif key =="KEY_DOWN":
						key = "menu_down"
				else:
					return self.ScrollText(key)
				
			elif key == chr(10) or key == "KEY_ENTER":
				# Input Log. Retrieve with <Alt + Up/Down>
				if line not in self.mucous.logs["history"] and line !='':
					self.mucous.logs["history"].append(line)
				else:
					x = self.mucous.logs["history"].pop(self.mucous.logs["history"].index(line))
					self.mucous.logs["history"].append(line)
					x = self.mucous.logs["history"].pop(self.mucous.logs["history"].index(""))
					self.mucous.logs["history"].append("")
			if key in ("menu_up", "menu_down"):
				self.mucous.PopupMenu.Scroll(key)
				return
			if self.mucous.mode == "search":
				self.mucous.Search.Input(key)
				return 
			if key in ("popup"):
				if self.mucous.PopupMenu.show == True:
					self.mucous.PopupMenu.Clear()
				else:
					self.mucous.PopupMenu.show = True
					if self.mucous.mode == "chat":
						if self.mucous.ChatRooms.shape not in ( "noroombox", "chat-only"):
							self.mucous.PopupMenu.Create("roombox", 0)
						else:
							return
					elif self.mucous.mode == "lists":
						self.mucous.PopupMenu.Create("lists", 0)
					elif self.mucous.mode == "transfer":
						self.mucous.PopupMenu.Create("transfers", 0)
					
					elif self.mucous.mode == "browse":
						
						if self.mucous.BrowseShares.current != "default__":
							if self.mucous.BrowseShares.selected == "files":
								self.mucous.PopupMenu.Create("browse-files", 0)
							elif self.mucous.BrowseShares.selected == "directories":
								self.mucous.PopupMenu.Create("browse-dirs", 0)
			elif key in ("delete"):
				if self.mucous.mode == "lists" and self.mucous.UsersLists.current == "interests":
					inp = self.mucous.Recommendations.input
					if inp == "add_likes":
						inp = "del_likes"
					elif inp == "del_likes":
						inp = "add_likes"
					elif inp == "del_hates":
						inp = "add_hates"
					elif inp == "add_hates":
						inp = "del_hates"
					self.mucous.Recommendations.input = inp
					self.mucous.Recommendations.ModeInterests()
			elif key == "expand":
				
				current = self.mucous.BrowseShares.current
				if self.mucous.BrowseShares.dirs == []:
					return
				Dir = self.mucous.BrowseShares.dirs[ self.mucous.BrowseShares.scrolling["directories"] ]

				if Dir in self.mucous.BrowseShares.collapsed[ current ]:
					self.mucous.BrowseShares.dirswithtree = None
					self.mucous.BrowseShares.collapsed[ current ].remove(Dir)
					self.mucous.BrowseShares.FormatBrowse()
			elif key == "expandcollapse":
				if self.mucous.mode == "browse" and self.mucous.BrowseShares.selected == "directories":
					self.mucous.BrowseShares.dirswithtree = None
					current = self.mucous.BrowseShares.current
					if self.mucous.BrowseShares.dirs == []:
						return
					Dir = self.mucous.BrowseShares.dirs[ self.mucous.BrowseShares.scrolling["directories"] ]

					if Dir not in self.mucous.BrowseShares.collapsed[ current ]:
						self.mucous.BrowseShares.collapsed[ current ].append(Dir)
					else:
						self.mucous.BrowseShares.collapsed[ current ].remove(Dir)
					self.mucous.BrowseShares.FormatBrowse()	
			elif key == "collapse":
				if self.mucous.mode == "browse" and self.mucous.BrowseShares.selected == "directories":
					
					current = self.mucous.BrowseShares.current
					if self.mucous.BrowseShares.dirs == []:
						return
					Dir = self.mucous.BrowseShares.dirs[ self.mucous.BrowseShares.scrolling["directories"] ]

					if Dir not in self.mucous.BrowseShares.collapsed[ current ]:
						self.mucous.BrowseShares.dirswithtree = None
						self.mucous.BrowseShares.collapsed[ current ].append(Dir)
						self.mucous.BrowseShares.FormatBrowse()
					
			elif key  == "switch":
				if self.mucous.mode == "chat":
					self.mucous.ChatRooms.WindowCycle()
				
				elif self.mucous.mode == "browse":	
					_list = [ "files", "directories" ]
					self.mucous.BrowseShares.selected = self.mucous.FormatData.RotateList("right", _list, self.mucous.BrowseShares.selected, "no")
					self.mucous.BrowseShares.Mode()
				elif self.mucous.mode == "lists" and self.mucous.UsersLists.current == "interests":
					_list = [ "recommendations", "similar_users", "likes", "hates" ]
					self.mucous.Recommendations.selected = self.mucous.FormatData.RotateList("right", _list, self.mucous.Recommendations.selected, "no")
					self.mucous.Recommendations.ModeInterests()
				elif self.mucous.mode == "transfer":
					# Tab to switch between upload and download scrolling
					if self.mucous.Transfers.current == "uploads":
						self.mucous.Transfers.current="downloads"
					elif self.mucous.Transfers.current=="downloads":
						self.mucous.Transfers.current="uploads"
					self.mucous.Transfers.ModeTransfers()
				elif self.mucous.mode in ( "help", "debug"):
					if self.mucous.mode == "help":
						self.mucous.mode = "debug"
					elif self.mucous.mode == "debug":
						self.mucous.mode = "help"
					self.mucous.Help.Mode()
			elif key in ( "KEY_HOME", "KEY_END"):
				
				
				if self.mucous.mode == "chat":
						
					if key == "KEY_END":
						if self.mucous.ChatRooms.selected == "chatroom":
							self.mucous.ChatRooms.scrolling[self.mucous.ChatRooms.selected] = -1
							self.mucous.ChatRooms.FormatChatText()
						elif self.mucous.ChatRooms.selected == "roombox":
							self.mucous.ChatRooms.scrolling[self.mucous.ChatRooms.selected] = len(self.mucous.ChatRooms.rooms[self.mucous.ChatRooms.current])
							self.mucous.ChatRooms.Mode()
					elif key == "KEY_HOME":
						if self.mucous.ChatRooms.selected == "chatroom":
							self.mucous.ChatRooms.scrolling[self.mucous.ChatRooms.selected] = 0 
							self.mucous.ChatRooms.FormatChatText()
						elif self.mucous.ChatRooms.selected == "roombox":
							self.mucous.ChatRooms.scrolling[self.mucous.ChatRooms.selected] = 0
							self.mucous.ChatRooms.Mode()
	
				
					
			elif key in( "KEY_LEFT", chr(91), chr(60), "KEY_RIGHT", chr(93), chr(62), "KEY_IC"):
				if key == "KEY_LEFT" or key == chr(91) or key == chr(60):
					direction = "left"
				elif key == "KEY_RIGHT" or key == chr(93) or key == chr(62) or key == "KEY_IC":
					direction = "right"
				if self.mucous.mode == "chat":
					place = self.mucous.FormatData.RotateList(direction, self.mucous.ChatRooms.rooms.keys(), self.mucous.ChatRooms.current, "yes" )
					if self.mucous.ChatRooms.current  != place:
						self.mucous.ChatRooms.Change(place)

							
				elif self.mucous.mode == "info":
					self.mucous.UserInfo.Select(direction)

				elif self.mucous.mode == "private":
					if self.mucous.PrivateChat.current != None:
						place = self.mucous.FormatData.RotateList(direction, self.mucous.PrivateChat.logs.keys(), self.mucous.PrivateChat.current, "yes" )
						if self.mucous.PrivateChat.current != place:
							self.mucous.PrivateChat.current = place

							self.mucous.PrivateChat.Start(self.mucous.PrivateChat.current)
							self.mucous.PrivateChat.Mode()
							
				elif self.mucous.mode == "setup":
					
					self.mucous.Setup.current  = self.mucous.FormatData.RotateList(direction, self.mucous.Setup.modes, self.mucous.Setup.current, "no" )
					self.mucous.Setup.input = "default"
					self.mucous.Setup.Mode()
					
				elif self.mucous.mode == "transfer":
					# HotKeyBar to switch listing transfer types
					
					self.mucous.Transfers.sort  = self.mucous.FormatData.RotateList(direction, self.mucous.Transfers.modes, self.mucous.Transfers.sort, "no" )
					
	
					self.mucous.Transfers.ModeTransfers()
		
				
					
				elif self.mucous.mode == "browse":
					if len(self.mucous.BrowseShares.users) >= 1:
						if self.mucous.BrowseShares.current == None:
							self.mucous.BrowseShares.current = self.mucous.BrowseShares.users[0]

							
						place = self.mucous.FormatData.RotateList(direction, self.mucous.BrowseShares.users, self.mucous.BrowseShares.current, "yes" )
						if place != self.mucous.BrowseShares.current:
							self.mucous.BrowseShares.current = place 

							sdirs =  self.mucous.BrowseShares.results[self.mucous.BrowseShares.current].keys()
							sdirs.sort(key=str.lower)
							self.mucous.BrowseShares.current_dir=sdirs[0]
							self.mucous.BrowseShares.scrolling["files"] = self.mucous.BrowseShares.scrolling["directories"] = 0
							self.mucous.BrowseShares.Mode()
					return line	
				elif self.mucous.mode=="lists":
					
					place = self.mucous.FormatData.RotateList(direction, [ "buddied", "banned", "ignored", "trusted", "interests"], self.mucous.UsersLists.current, "no" )
					if self.mucous.UsersLists.current  != place:
						self.mucous.UsersLists.current = place
						self.mucous.UsersLists.ModeLists()

					
				elif self.mucous.mode=="roomlist":
					place = self.mucous.FormatData.RotateList(direction, [ "alpha", "alpha-reversed", "size", "size-reversed"], self.mucous.Config["mucous"]["rooms_sort"], "no" )
					if self.mucous.Config["mucous"]["rooms_sort"]  != place:
						self.mucous.Config["mucous"]["rooms_sort"] = place
						self.mucous.RoomsList.Mode()
			elif key in ("-", "+"):
				if self.mucous.mode == "setup" and self.mucous.Setup.input in self.mucous.Setup.numberboxes:
					self.mucous.Setup.ChangeSize(key)
			
			
			return line
		except Exception, e:
			self.mucous.Help.Log("debug", "InputFunctions: " + str(e))
			
	
	# ---v  KEYS v
	
	## Update Scrolling Numbers
	# @param self CharacterParse (Class)
	# @param num scrolling number
	def UpdateScrollNum(self, num):
		try:
			if self.mucous.mode == "private":
				self.mucous.PrivateChat.scrolling = num
			elif self.mucous.mode == "search":
				self.mucous.Search.scrolling = num
			elif self.mucous.mode == "roomlist":
				self.mucous.RoomsList.scrolling = num
			elif self.mucous.mode == "browse":
				self.mucous.BrowseShares.scrolling[self.mucous.BrowseShares.selected] = num
			elif self.mucous.mode == "info":
				if self.mucous.UserInfo.current == None:
					self.mucous.UserInfo.scrolling_status = num
				else:
					self.mucous.UserInfo.scrolling[self.mucous.UserInfo.current] = num
			elif self.mucous.mode == "lists":
				if self.mucous.UsersLists.current == "interests":
					self.mucous.Recommendations.scrolling[self.mucous.Recommendations.selected] = num
				else:
					self.mucous.UsersLists.scrolling[self.mucous.UsersLists.current] = num
			elif self.mucous.mode == "transfer":
				#if self.mucous.Transfers.current == "interests":
				self.mucous.Transfers.scrolling[self.mucous.Transfers.current] = num
			elif self.mucous.mode == "chat":
				self.mucous.ChatRooms.scrolling[self.mucous.ChatRooms.selected] = num
			elif self.mucous.mode in ("debug", "help"):
				self.mucous.Help.scrolling[self.mucous.mode] = num
		except Exception, e:
			self.mucous.Help.Log("debug", "UpdateScrollNum: " + str(e))

	## Create Tab Completion List
	# @param self CharacterParse (Class)
	def InputCompletionList(self):
		try:
			usercompletionlist = {}
			if self.mucous.config.has_key("buddies"):
				for users in self.mucous.config["buddies"].keys():
					usercompletionlist[self.mucous.dlang(users)] = 0
			if self.mucous.config.has_key("banned"):
				for users in self.mucous.config["banned"].keys():
					usercompletionlist[self.mucous.dlang(users)] = 0
			if self.mucous.config.has_key("ignored"):
				for users in self.mucous.config["ignored"].keys():
					usercompletionlist[self.mucous.dlang(users)] = 0
			
			
			if self.mucous.ChatRooms.rooms.keys() != []:
				for user in self.mucous.ChatRooms.rooms[self.mucous.ChatRooms.current]:
					usercompletionlist[self.mucous.dlang(user)] = 0
		
			pmusers = self.mucous.PrivateChat.logs.keys()
			for user in pmusers:
				usercompletionlist[self.mucous.dlang(user)] = 0
			if self.mucous.mode == "transfers":
				if self.mucous.Transfers.transfers["uploads"].keys() != []:
					for userpath, values in self.mucous.Transfers.transfers["uploads"].items():
						usercompletionlist[self.mucous.dlang(values[1])] = 0
				if self.mucous.Transfers.transfers["downloads"].keys() != []:
					for userpath, values in self.mucous.Transfers.transfers["downloads"].items():
						usercompletionlist[self.mucous.dlang(values[1])] = 0
			elif self.mucous.mode == "browse":
				if self.mucous.BrowseShares.current != None:
					if self.mucous.BrowseShares.current in self.mucous.BrowseShares.results:
						for dirs in self.mucous.BrowseShares.results[self.mucous.BrowseShares.current].keys():
							usercompletionlist[self.mucous.dlang(dirs)] = 0
			for values in self.mucous.commandlist:
				usercompletionlist[self.mucous.dlang(values)] = 0
				
			if usercompletionlist.keys() != []:
				self.mucous.logs["tab_completion"] = usercompletionlist.keys()
				self.mucous.logs["tab_completion"].sort(key=str.lower)
			else:
				self.mucous.logs["tab_completion"] = []
		except Exception, e:
			self.mucous.Help.Log("debug", "InputCompletionList: " + str(e))
			

	## Here's where scrolling works
	# @param self CharacterParse (Class)
	# @param key Key Pressed
	# @return list / Nothing
	def ScrollText(self, key):
		try:
			color_added = None
			scrolltext = None
			scrollnum = None
			position = None
			scrollchange = 0
			if self.mucous.mode  == "setup":
				self.mucous.Setup.Switch(key)
				return None

			elif self.mucous.mode not in ( "transfer", "search", "help", "debug", "info", "private", "lists", "roomlist", "chat", "browse"):
				return 
			if self.mucous.mode == "chat":
				if self.mucous.ChatRooms.selected == "chatroom":
					scrollnum = self.mucous.ChatRooms.scrolling["chatroom"]
					w = self.mucous.ChatRooms.dimensions["chat"]
					if self.mucous.ChatRooms.current != None:
						selected_log = self.mucous.ChatRooms.logs["rooms"][self.mucous.ChatRooms.current]
						 
					else: 
						self.mucous.ChatRooms.DrawChatWin()
						self.mucous.ChatRooms.windows["text"]["chat"].refresh()
						return
				elif self.mucous.ChatRooms.selected == "roombox":
					scrollnum = self.mucous.ChatRooms.scrolling["roombox"]
					w = self.mucous.ChatRooms.dimensions["roombox"]
					selected_log = self.mucous.ChatRooms.logs["roombox"][self.mucous.ChatRooms.current]
				elif self.mucous.ChatRooms.selected == "roomstatus":
					scrollnum = self.mucous.ChatRooms.scrolling["roomstatus"]
					#scrolltext = "roomstatus"
					w = self.mucous.ChatRooms.dimensions["roomstatus"]
					selected_log = self.mucous.ChatRooms.logs["roomstatus"][self.mucous.ChatRooms.current]
				scrolldiff = w["height"]
			elif self.mucous.mode == "browse":
				scrollnum = self.mucous.BrowseShares.scrolling[self.mucous.BrowseShares.selected]
				scrolldiff =self.mucous.BrowseShares.dimensions["browse"]["height"]
			elif self.mucous.mode in ("debug", "help"):
				scrollnum = self.mucous.Help.scrolling[self.mucous.mode]
				scrolldiff = self.mucous.Help.dimensions["help"]["height"]
			elif self.mucous.mode ==  "private":
				scrollnum = self.mucous.PrivateChat.scrolling
				scrolldiff = self.mucous.PrivateChat.dimensions["height"]
				
			elif self.mucous.mode == "info":
				if self.mucous.UserInfo.current == None:
					scrollnum = self.mucous.UserInfo.scrolling_status
				else:
					scrollnum = self.mucous.UserInfo.scrolling[self.mucous.UserInfo.current]
				scrolldiff = self.mucous.UserInfo.dimensions["info"]["height"]
			elif self.mucous.mode == "search":
				if self.mucous.Search.input != "results":
					self.mucous.Search.input="results"
					self.mucous.Search.Mode()
				scrollnum = self.mucous.Search.scrolling
				scrolldiff = self.mucous.Search.dimensions["height"]
				
			elif self.mucous.mode == "lists":
				if self.mucous.UsersLists.current == "interests":
					scrollnum = self.mucous.Recommendations.scrolling[self.mucous.Recommendations.selected]
					scrolldiff = self.mucous.Recommendations.dimensions[self.mucous.Recommendations.selected]["height"]
				else:
					scrollnum = self.mucous.UsersLists.scrolling[self.mucous.UsersLists.current]
					scrolldiff = self.mucous.UsersLists.dimensions[self.mucous.UsersLists.current]["height"]
					
			elif self.mucous.mode == "roomlist":
				scrollnum = self.mucous.RoomsList.scrolling
				scrolldiff = self.mucous.RoomsList.dimensions["height"]
			elif self.mucous.mode == "transfer":
				scrollnum = self.mucous.Transfers.scrolling[self.mucous.Transfers.current]
				scrolldiff = self.mucous.Transfers.dimensions[self.mucous.Transfers.current]["height"]-1
				
			if scrolltext != None:
				position = self.mucous.scrolling[scrolltext]
			if scrollnum != None:
				position = scrollnum
			if key == "KEY_UP":	
				position -= 1
				scrollchange = -1
			elif key == "KEY_DOWN":
				position += 1
				scrollchange = 1
			elif key == "KEY_PPAGE":
				position -= scrolldiff
				scrollchange = -scrolldiff 
				
				# upload or download window height - one line for heading
			elif key == "KEY_NPAGE":
				position += scrolldiff
				#scrollchange = scrolldiff
			if position < 0:
				position = 0
			if key == "KEY_HOME":
				position = 0
				#scrollchange = -scrolldiff 
				
				# upload or download window height - one line for heading
			elif key == "KEY_END":
				position = -1
				#scrollchange = scrolldiff
			
			
			if scrolltext != None:
				self.mucous.scrolling[scrolltext] = position
			if scrollnum != None:
				self.UpdateScrollNum(position)
				
			if self.mucous.mode == "chat":
				if self.mucous.ChatRooms.selected  == "chatroom":
					#start -= 1
					self.mucous.ChatRooms.FormatChatText()
				elif self.mucous.ChatRooms.selected  == "roombox":
					self.mucous.ChatRooms.FormatBox()
				elif self.mucous.ChatRooms.selected  == "roomstatus":
					self.mucous.ChatRooms.DrawStatusText()
					
			elif self.mucous.mode == "browse":

				if self.mucous.BrowseShares.selected == "directories":
					self.mucous.BrowseShares.FormatBrowse(True)
				else:
					self.mucous.BrowseShares.FormatBrowse(False)
				curses.doupdate()
			elif self.mucous.mode == "info":
				self.mucous.UserInfo.DrawText()
			elif self.mucous.mode == "private":
				self.mucous.PrivateChat.Draw()
			elif self.mucous.mode == "search":
				self.mucous.Search.Draw()
			elif self.mucous.mode == "lists":
				if self.mucous.UsersLists.current != "interests":
					self.mucous.UsersLists.SelectLists()
				else:
					self.mucous.Recommendations.DrawInterests()
		
				return
			elif self.mucous.mode == "roomlist":
				self.mucous.RoomsList.Format()
				return self.mucous.RoomsList.sizedrooms[self.mucous.RoomsList.scrolling]
			elif self.mucous.mode in ("help", "debug"):
				self.mucous.Help.Format()
			elif self.mucous.mode == "transfer":
				if self.mucous.Transfers.current == "uploads":
					self.mucous.Transfers.UploadManager()
					curses.doupdate()
				elif self.mucous.Transfers.current == "downloads":
					self.mucous.Transfers.DownloadManager()
					curses.doupdate()
				
		except Exception, e:
			self.mucous.Help.Log("debug", "ScrollText: " + str(e))
			
	## Complete command, word, or user in line
	# @param self CharacterParse (Class)
	# @param line string containing the entire line
	# @param part line segment we are in
	# @param firsttab (0/1) (do not rebuild and match the completion list if 1)
	# @param listline line split into a list
	# @param pos horizontal Postion in line
	# @return listline, firsttab, part, pos
	def InputTabCompletion(self, line, part, firsttab, listline, pos):
		try:
			self.mucous.listline = listline
			
			if firsttab ==0:
				self.InputCompletionList()
				
			if self.mucous.logs["tab_completion"] == []:
				return self.mucous.listline, firsttab, part, pos
				
			if self.mucous.mode not in ("roomlist", "chat", "lists", "info", "private", "browse", "transfer", "setup", "search", "help", "debug", "status"):
				return self.mucous.listline, firsttab, part, pos
			if len(part) <= 0:
				return self.mucous.listline, 0, part, pos
			if firsttab ==0:
				self.mucous.keepit=[]
				if self.mucous.mode == "roomlist":
					for words in self.mucous.RoomsList.rooms.keys():
						if words.upper().startswith(part[0:1].upper()):
							self.mucous.keepit.append(words)
				else:
					for words in self.mucous.logs["tab_completion"]:
						if part[0:1] == '/':
							# Be more picky with /commands
							if len(words) > len(part):
								if words.upper().startswith(part.upper()):
									self.mucous.keepit.append(words)
							
						else:
							if words.upper().startswith(part.upper()):
								self.mucous.keepit.append(words)
				self.mucous.keepit.sort(key=str.lower)
			firsttab +=1

			currentword = self.mucous.listline[pos]	
			lw = len(currentword)

			if currentword.upper().startswith(part.upper()):
				
				for words in self.mucous.keepit:
					self.mucous.listline[pos] = self.mucous.keepit[0]
					z = self.mucous.keepit[0]
					self.mucous.keepit.remove(self.mucous.keepit[0])
					self.mucous.keepit.append(z)


					break
				else:
					
					firsttab = 0
			else:
				firsttab = 0

			return self.mucous.listline, firsttab, part, pos

		except Exception, e:
			self.mucous.Help.Log("debug", "InputTabCompletion: " + str(e))
			
	## Previously inputted messages
	# @param self CharacterParse (Class)
	# @param key KEY_UP or KEY_DOWN
	# @param line current line
	# @param history_count position in the history log
	# @return line, history_count
	def InputHistory(self, key, line, history_count):
		try:
			self.mucous.Spl["history_count"] = history_count
			if line not in self.mucous.logs["history"] and line !='' and not line.isspace():
				self.mucous.logs["history"].append(line)
				
			if len(self.mucous.logs["history"]) == 0:
				# return current line and current position
				return line, self.mucous.Spl["history_count"]
			
			if key == "KEY_UP":
				self.mucous.Spl["history_count"] += 1
			elif key == "KEY_DOWN":
				self.mucous.Spl["history_count"] -= 1
	
			last_line = len(self.mucous.logs["history"]) -1 - self.mucous.Spl["history_count"]
			if last_line < 0:
				last_line = len(self.mucous.logs["history"]) -1
				self.mucous.Spl["history_count"] = 0
			elif last_line > len(self.mucous.logs["history"]) -1:
				last_line = len(self.mucous.logs["history"]) -1
				self.mucous.Spl["history_count"] += 1
			line = self.mucous.logs["history"][ last_line]
			# return old line and line's position
			return line, self.mucous.Spl["history_count"]
		except Exception, e:
			self.mucous.Help.Log("debug", "history: " + str(e))
	

	## Clicked on the tab selector
	# @param self is CharacterParse (Class)
	# @param x Horizontal position
	# @param chosen Mode we are in
	# @return chose, match
	def MouseClickTab(self, x, chosen):
		try:
			choz = self.mucous.activeitems["positions"][chosen]
			match = None
			if x >= choz[0] and x < choz[1]:
				# do nothing if chose search is clicked
				return chosen, 'yes'
			else:
				for key, pos in self.mucous.activeitems["positions"].items():
					if x >= pos[0]   and x < pos[1] :
						match = 'yes'
						break
			if match != None:
				return key, match
			else:
				return chosen, None
		except Exception, e:
			self.mucous.Help.Log("debug", "MouseClickTab: " +str(e) )
		
	## Mouse Coordinates: Switch modes or options in position matches requirements
	# @param self is CharacterParse (Class)
	# @param line is a text string
	# @return line / Nothing
	def MouseXY(self, line):
		
		try:
			(id,x,y,z,event) = curses.getmouse()
			#self.mucous.Help.Log("debug", "%d %d %d %d %d" % (id, x, y, z,event))
			if event in (1, 128, 8192):
				# Ignore PRESSED and RELEASED
				return
			if y == 0:
				if x in range(8):
					if self.mucous.Spl["connected"] == 0:
						#self.mucous.D.connect()
						self.mucous.ManuallyConnect()
					elif self.mucous.Spl["connected"] == 1:
						self.mucous.ToggleAwayStatus()
				elif x > self.mucous.w - 15:
					self.mucous.Help.Mode()
				return
		# 1Chat 2Private 3Transfers 4Search 5Info 6Browse 7Users 8Rooms 9Setup 10Help
			if y >= self.mucous.h-1:
				# clickable mode switching
				if x >= 0 and x < 7:
					self.mucous.ChatRooms.Mode()
				elif x >= 7 and x < 16:
					self.mucous.PrivateChat.Mode()
				elif x >= 16 and x < 27:
					self.mucous.Transfers.ModeTransfers()
				elif x >= 27 and x < 35:
					self.mucous.Search.Default()
				elif x >= 35 and x < 41:
					self.mucous.UserInfo.Mode()
				elif x >= 41 and x < 49:
					self.mucous.BrowseShares.Mode()
				elif x >= 49 and x < 56:
					self.mucous.UsersLists.ModeLists()
				elif x >= 56 and x < 63:
					self.mucous.RoomsList.Mode()
				elif x >= 63 and x < 70:
					self.mucous.Setup.Default()
				elif x >= 70 and x < 76:
					self.mucous.Help.Mode()
				return
				
			if self.mucous.PopupMenu.show == True and self.mucous.PopupMenu.current != None:
				p = self.mucous.PopupMenu.menus[self.mucous.PopupMenu.current]

				if x >= p["left"] and x < p["left"]+p["width"] and y >= p["top"] and y < p["top"]+p["height"]:
					self.mucous.PopupMenu.Mouse(x, y, event)
					return
				else:
					self.mucous.PopupMenu.Clear()
					
			if self.mucous.mode == "chat":
				self.mucous.ChatRooms.MouseChat(x,y,z,event)
			elif self.mucous.mode == "private":
				# Clickable private message switch
				return self.mucous.PrivateChat.Mouse(x,y,z,event)
			elif self.mucous.mode == "browse":
				return self.mucous.BrowseShares.MouseBrowse(x,y,z,event)
			elif self.mucous.mode == "info":
				# Clickable user info tabs
				return self.mucous.UserInfo.Mouse(x,y,z,event)
			elif self.mucous.mode == "search":
				return self.mucous.Search.Mouse(x,y,z,event)
			elif self.mucous.mode == "roomlist":
				# ROOMLIST BUTTONS
				return self.mucous.RoomsList.Mouse(x,y,z,event)
			elif self.mucous.mode == "lists":
				return self.mucous.UsersLists.Mouse(x,y,z,event)
			elif self.mucous.mode == "transfer":
				# TRANSFERS BUTTONS
				# Clickable transfer type switcher
				return self.mucous.Transfers.Mouse(x,y,z,event)
			elif self.mucous.mode == "setup":
				return self.mucous.Setup.Mouse(x,y,z,event)
				

			elif self.mucous.mode in ("help", "debug"):
				if y == self.mucous.Help.dimensions["help"]["top"]-1:
					if x >= 4 and x <= 16 and self.mucous.mode != "help":
						self.mucous.mode = "help"
						self.mucous.Help.Mode()
					elif x >= 18 and x <= 31 and self.mucous.mode != "debug":
						self.mucous.mode = "debug"
						self.mucous.Help.Mode()
			# END OF MOUSE
			return line
		except Exception, e:
			self.mucous.Help.Log("debug", "MouseXY: "+str(e) )

	## Parse Integer set
	# @param self is mucous
	# @param s is a text string
	# @return list of numbers
	def ParseIntegerSet(self, s):
		trim = re.compile('(^\s+|\s+$)')
		sequence = re.compile('\s*,\s*')
		interval = re.compile('(\d+)\s*-\s*(\d+)')

		s = trim.sub('', s)
		nums = set()
	 
		for i in sequence.split(s):
			if i.isdigit():
				nums.add(int(i))
			elif interval.match(i):
				a, b = interval.match(i).groups()
				for j in range(int(a), int(b)+1):
					nums.add(j)
			else:
				return
		return nums

	## Parse Entry box for commands
	# @param self is mucous
	# @param line is a text string
	# @return Nothing / 0 / 1 /2 
	def InputCommands(self, line):
		try:
			if line[:1] != "/" or line[:2] == '//' or line[:4] == '/me ':
				if self.mucous.Spl["title"]:
					self.InputText(line)
				return
			#if line[:1] == "/" and line[:4] != "/me " and line[:2] != '//':
			c = line.split(" ", 1)
			command = c[0]
			if len(c) == 2:
				args = c[1]
			else:
				args = ""

			if command in ("/quit",  "/exit"):
				self.mucous.config_manager.update_config()
				self.mucous.shutdown()
				return 2
			elif command == "/disconnect":
				self.mucous.disconnect()
			elif command == "/debug":
				self.mucous.mode = "debug"
				self.mucous.Help.Mode()
			elif command == "/help":
				self.mucous.mode = "help"
				self.mucous.Help.Mode()
				if args == "chat":
					for line in self.mucous.Help.log["chat"]:
						self.mucous.Help.Log("help", line)
				elif args == "mode":
					for line in self.mucous.Help.log["modes"]:
						self.mucous.Help.Log("help", line)
				elif args == "user":
					for line in self.mucous.Help.log["user"]:
						self.mucous.Help.Log("help", line)
				elif args == "search":
					for line in self.mucous.Help.log["search"]:
						self.mucous.Help.Log("help", line)
				elif args == "browse":
					for line in self.mucous.Help.log["browse"]:
						self.mucous.Help.Log("help", line)
				elif args == "transfer":
					for line in self.mucous.Help.log["transfer"]:
						self.mucous.Help.Log("help", line)
				elif args == "ticker":
					for line in self.mucous.Help.log["tickers"]:
						self.mucous.Help.Log("help", line)
				elif args == "download":
					for line in self.mucous.Help.log["download"]:
						self.mucous.Help.Log("help", line)
				elif args == "":
					for line in self.mucous.Help.log["helpcommands"]:
						self.mucous.Help.Log("help", line)
				elif args == "keys":
					for line in self.mucous.Help.log["keys"]:
						self.mucous.Help.Log("help", line)
				elif args == "connect":
					for line in self.mucous.Help.log["connect"]:
						self.mucous.Help.Log("help", line)
				elif args == "setup":
					for line in self.mucous.Help.log["setup"]:
						self.mucous.Help.Log("help", line)
				try:
					self.mucous.edit.reset()
				except:
					pass
				'''
				Chatrooms
				'''
			elif command == "/talk": 
				self.mucous.ChatRooms.Change(args)
				
			elif command in("/j", "/join"):
				self.mucous.ChatRooms.JoinRoom(args)
				
			elif command in ("/part", "/p", "/l", "/leave") and args == "":
				self.mucous.ChatRooms.Leave()
				
			elif command == "/part" and args != "":
				for room in self.mucous.ChatRooms.rooms.keys():
					if self.mucous.dlang( args ) == room:
						self.mucous.D.LeaveRoom(room)
						
			elif command == "/leave" and args != "":
				for room in self.mucous.ChatRooms.rooms.keys():
					if self.mucous.dlang( args ) == room:
						self.mucous.D.LeaveRoom(room)
				
			elif command == "/autojoin" and args != '':
				if args in self.mucous.ChatRooms.rooms.keys():
					room = args
					self.mucous.ModifyConfig("autojoin", room, '')
				else:
					self.mucous.Help.Log("status", "You aren't in room: %s" %args)
					
			elif command == "/autojoin"  and args == "":
				if self.mucous.ChatRooms.current != None:
					room = self.mucous.ChatRooms.current
					self.mucous.ModifyConfig("autojoin", room, '')
					
			elif command == "/extra":
				
				if self.mucous.Config["mucous"]["extra_requests"] == "Yes":
					self.mucous.Config["mucous"]["extra_requests"] = "No"
				elif self.mucous.Config["mucous"]["extra_requests"] == "No":
					self.mucous.Config["mucous"]["extra_requests"] = "Yes"
				if self.mucous.Config["mucous"]["extra_requests"] not in ("Yes", "No"):
					self.mucous.Config["mucous"]["extra_requests"] = "No"
				self.mucous.Help.Log("status", "Extra CTCP-like version requests are responded to? %s"  % self.mucous.Config["mucous"]["extra_requests"] )
			
			elif command == "/exist":
				user = args
				if user != "" and not user.isspace():
					self.mucous.D.PeerExists(user)
					
			elif command == "/ut":
				try:
					num = int(args)
					for username, path in self.mucous.Transfers.downloads[num].items():
						self.mucous.D.TransferUpdate(username, path)
				except:
					return
					
			elif command == "/beep":
				self.mucous.ToggleBeep()
			elif command =="/ping":
				self.mucous.D.Ping(1)
			elif command == "/close":
				user = None
				if args != '':
					user == args
					
				if self.mucous.mode == 'private':
					if user != None: this_user = user
					else: this_user = self.mucous.PrivateChat.current
					if this_user != None:
						self.mucous.PrivateChat.Close(this_user)
						
				elif self.mucous.mode == 'chat':
					if user != None: room = user
					else: room = self.mucous.ChatRooms.current
					if room in self.mucous.ChatRooms.rooms.keys():
						self.mucous.D.LeaveRoom(room)
						
				elif self.mucous.mode == 'info':
					if user != None: this_user = user
					else: this_user = self.mucous.UserInfo.current
					if this_user != None:
						self.mucous.UserInfo.Close(this_user)
						
				elif self.mucous.mode == 'browse':
					if user != None: this_user = user
					else: this_user = self.mucous.BrowseShares.current
					if this_user != "default__":
						self.mucous.BrowseShares.Close(this_user)
						
				elif self.mucous.mode =='search':
					if self.mucous.Search.current != None:
						self.mucous.Search.Close(self.mucous.Search.current)

			elif command == "/pm" and args != '':
				self.mucous.PrivateChat.current = args
				self.mucous.PrivateChat.Start(self.mucous.PrivateChat.current)
				if self.mucous.mode == 'private':
					self.mucous.PrivateChat.Mode()

					
			elif command == "/msg" and args != '':
				if self.mucous.PrivateChat.current != None:
					message = args
					self.mucous.PrivateChat.Send(self.mucous.PrivateChat.current, message)
				else:
					self.mucous.mode = "debug"
					self.mucous.ModeHelp()
					self.mucous.Help.Log("status", "Set a user to message with /pm!")
					
			elif command == "/autoaway":
				aa = self.mucous.Config["mucous"]["autoaway"]
				if aa == "yes":
					aa = "no"
				elif aa == "no":
					aa = "yes"
				self.mucous.Config["mucous"]["autoaway"] = aa
				self.mucous.Help.Log("status", "Autoaway is On? " + aa )
			elif command == "/away":
				self.mucous.ToggleAwayStatus()
			elif command == "/setaway":
				self.mucous.D.SetStatus(1)
				self.mucous.D.SetStatus(0)
			elif command == "/say" and args !='':
				# /say <room> /me is hungry
				sine = args
				splited = sine.split(' ')
				if len(splited) > 1:
					
					if splited[0] in self.mucous.ChatRooms.rooms.keys():
						room = splited[0]
						if splited[1].isspace():
							pass
						else:
							message = string.join(map(str, splited[1:]))
							self.mucous.ChatRooms.SayInChat(room, message)
					else:
						if len(splited) > 2:
							s = ''
							n = 0
							
							#self.mucous.Help.Log("debug", str(splited))
							for i in range(len(splited)):
								if i == 0:
									s =splited[i]
								else:
									s += ' ' +splited[i]
								n += 1
								if s in self.mucous.ChatRooms.rooms.keys():
									break
		
							if s not in self.mucous.ChatRooms.rooms.keys():
								self.mucous.Help.Log("debug", s)
								pass
							else:
								room = s
								message = string.join(map(str, splited[n:]))
								if message.isspace():
									pass
								else:
									self.mucous.ChatRooms.SayInChat(room, message)
							
		
			elif command == "/url" and args == '':
				self.ReadURL(args)
			elif command == "/urlreader" and args != '':
				self.mucous.Config["mucous"]["url reader"] = args
			elif command == "/urlcustom" and args != '':
				if "$" in args:
					self.mucous.Config["mucous"]["url custom"] = custom
			elif command == "/np" and args == '':
				self.mucous.NowPlaying()
				
			elif command == "/w" and args != '' or command == "/window" and args != "":	
				
				num = args
				if num.isdigit() == 0:
					return
				
				if num == "1":
					self.mucous.ChatRooms.Mode()
				elif num == "2":
					self.mucous.PrivateChat.Mode()
				elif num == "3":
					self.mucous.Transfers.ModeTransfers()
				elif num == "4":
					self.mucous.Search.Mode()
				elif num == "5":
					self.mucous.UserInfo.Mode()
				elif num == "6":
					self.mucous.BrowseShares.Mode()
				elif num == "7":
					self.mucous.UsersLists.ModeLists()
				elif num == "8":
					self.mucous.RoomsList.Mode()
				elif num == "9":
					self.mucous.Setup.Default()
				elif num == "10":
					self.mucous.Help.Mode()
						
			elif command == "/npset" and args != '':
				self.mucous.Config["mucous"]["now-playing"] = args
				
			elif command == "/npcheck" and args == '':
				if "now-playing" in self.mucous.Config["mucous"].keys():
					self.mucous.Help.Log("status", "Now playing command is: "+ str(self.mucous.Config["mucous"]["now-playing"]))
				
			elif command == "/npprefix" and args != '':
				self.mucous.Config["mucous"]["now-playing-prefix"] = args
				
			elif command == "/npprefix":
				if "now-playing-prefix" in self.mucous.Config["mucous"]:
					self.mucous.Config["mucous"]["now-playing-prefix"] = None
			elif command in ("/rescan", "/rescanshares"):
				self.mucous.Muscan.Command(["muscan", "-v"])
				
				self.mucous.Help.Log("status", "Updating normal shares with muscan, don't forget to Reload them.")
			elif command in ("/rescanbuddy"):
				self.mucous.Muscan.Command(["muscan", "-b", "-v"])
				self.mucous.Help.Log("status", "Updating buddy shares with muscan, don't forget to Reload them.")
					
			elif command in ("/reload", "/reloadshares"):
				self.mucous.D.ReloadShares()
						
			elif command == "/redraw":
				self.mucous.line = self.mucous.Build()
				
			elif command == "/logging":
				self.mucous.ToggleLogging()
				
				if self.mucous.mode=="setup":
					self.mucous.Setup.Mode()
			elif command == "/logdir" and args != "":
				path = args
				if not path.isspace() and path != "":
					if os.path.exists(os.path.expanduser(path)):
						self.mucous.Config["mucous"]["log_dir"] = os.path.expanduser(path)
						self.mucous.Help.Log("debug", "Logs directory set to: %s" % path)
					else:
						self.mucous.Help.Log("debug", "Path for logs directory, %s, doesn't exist" % path)
				'''
				User Information
				'''
			elif command == "/userinfo" and args != '':
				user = self.mucous.dlang( args ) 
				self.mucous.UserInfo.Get(user)
				
					
			elif command == "/tc":
				self.InputCompletionList()
			elif command == "/language":
				self.mucous.Config["mucous"]["language"] = args
				
			elif command == "/ip" and args != '':
				try:
					
					user  = self.mucous.dlang( str(args) )
					self.mucous.requests["ip"].append(user)
					self.mucous.D.PeerAddress(user)
				except Exception, e:
					self.mucous.Help.Log("debug", e)
					
			elif command == "/ip" and args == "":
				try:
					if self.mucous.mode == "private" and self.mucous.PrivateChat.current != None:
						user  =  self.mucous.PrivateChat.current
						self.mucous.requests["ip"].append(user)
						self.mucous.D.PeerAddress(user)
				except Exception, e:
					self.mucous.Help.Log("debug", e)
					
			elif command == "/stat" and args != '':
				user = self.mucous.dlang( str(args) )
				self.mucous.requests["statistics"].append(user)
				self.mucous.D.PeerStats(user)
			elif command == "/status" and args != '':
				user = self.mucous.dlang( str(args) )
				self.mucous.requests["statistics"].append(user)
				self.mucous.D.PeerStatus(user)
				'''
				MODE SELECTIONS
				'''
			
			elif command == "/chat" :
				self.mucous.ChatRooms.Mode()
		
			elif command in ("/private", "/privatechat"):
				self.mucous.PrivateChat.Mode()
		
			elif command == "/search"  and args == '':
				self.mucous.Search.Mode()
		
			elif command in ("/transfer", "/t", "/transfers"):
				self.mucous.Transfers.ModeTransfers()
		
			elif command == "/info":
				self.mucous.UserInfo.Mode()
				
			elif command == "/browse":
				self.mucous.BrowseShares.Mode()
					
			elif command == "/buddylist" :
				self.mucous.UsersLists.current = "buddied"
				self.mucous.UsersLists.ModeLists()
			elif command == "/trustlist" :
				self.mucous.UsersLists.current = "trusted"
				self.mucous.UsersLists.ModeLists()	
			elif command == "/banlist" :
				self.mucous.UsersLists.current = "banned"
				self.mucous.UsersLists.ModeLists()
				
			elif command == "/ignorelist" :
				self.mucous.UsersLists.current = "ignored"
				self.mucous.UsersLists.ModeLists()
				
			elif command == "/interests" :
				self.mucous.UsersLists.current = "interests"
				self.mucous.UsersLists.ModeLists()
				
			elif command == "/setup" and args == '':
				self.mucous.Setup.Default()
				
				'''
				CONFIG
				'''
			elif command == "/save":
				self.mucous.config_manager.update_config()
				self.mucous.mode = "debug"
				self.mucous.Help.Mode()
			elif command == "/interface" and args != "":
				self.mucous.Config["connection"]["interface"] = args
				self.mucous.Help.Log("status", "Museekd interface set to: " + args)
		
			elif command == "/password" and args != "":
				self.mucous.Config["connection"]["passw"] = args
				self.mucous.Help.Log("status", "New password set")
			elif command == "/ctcpversion" and args != "":
				user = args
				if user != "" and user.isspace() == False:
					self.mucous.PrivateChat.Send(user, curses.ascii.ctrl("A")+"VERSION"+curses.ascii.ctrl("A"))
			elif command == "/version":
				self.mucous.mode = "debug"
				self.mucous.Help.Mode()
				self.mucous.Help.Log("status", "Mucous version: %s" % self.mucous.Version)
			elif command == "/connect":
				self.mucous.ManuallyConnect()
				'''
				Tickers
				'''
			elif command == "/tickroom":
				if args == '':
					self.mucous.Spl["ticker_room"] =  self.mucous.ChatRooms.current
				elif args != '':
					self.mucous.Spl["ticker_room"] = args
			elif command == "/showtickers":
				self.mucous.ChatRooms.ToggleTickersDisplay()
			elif command == "/tickers" :
				self.mucous.ChatRooms.ToggleTickers()
			elif command == "/defaulttick" and args != '':
				message = args
				self.mucous.D.ConfigSet("default-ticker", "ticker", message)
				
			elif command == "/settemptick" and args != '':
				if self.mucous.Spl["ticker_room"] != None:
					message = args
					self.mucous.D.RoomTickerSet(self.mucous.Spl["ticker_room"], message)
				else:
					self.mucous.Help.Log("status", "Choose a room with /tickroom, first.") 		
			elif command == "/settick" and args != '':
				if self.mucous.Spl["ticker_room"] != None:
					message = args
					self.mucous.D.ConfigSet("tickers", self.mucous.Spl["ticker_room"], message)
					self.mucous.D.RoomTickerSet(self.mucous.Spl["ticker_room"], message)
				else:
					self.mucous.Help.Log("status", "Choose a room with /tickroom, first.") 
				'''
				List tickers in current room or selected rooms
				'''
			
			elif command == "/listtick":
				if args == '':
					woom = self.mucous.ChatRooms.current
				else:
					woom = args
					
				alpha_list  = self.mucous.SortedDict()
				for rooms12 in self.mucous.ChatRooms.tickers:
					alpha_list[rooms12] = self.mucous.ChatRooms.tickers[rooms12]
				#if self.mucous.Config["tickers"]["tickers_enabled"] == 'yes':
				
				for rooms13, ticks in alpha_list.items():
					if rooms13 == woom:
						
						ttickers = ticks.keys()
						if ttickers != []:
							self.mucous.Help.Log("status", "Tickers in room: "+str(rooms13))
						ttickers.sort(key=str.lower)
						for names in ttickers:
							self.mucous.Help.Log("status", " ["+str(names)+'] '+str(ticks[names]))
		
		
				'''
				User Management
				'''
			elif command == "/ban" and args != '':
				username = args
				self.mucous.ModifyConfig("ban", username, '')
				
				
			elif command == "/unban" and args != '':
				username = args
				self.mucous.ModifyConfig("unban", username, '')
				
					
			elif command == "/ignore" and args != '':
				username = args
				self.mucous.ModifyConfig("ignore", username, '')
				
			elif command == "/unignore" and args != '':
				username = args
				self.mucous.ModifyConfig("unignore", username, '')
						
			elif command == "/buddy" and args != '':
				username = args
				self.mucous.ModifyConfig("buddy", username, '')
				
			elif command == "/unbuddy" and args != '':
				username = args
				self.mucous.ModifyConfig("unbuddy", username, '')
				
			elif command == "/nuke" and args != '':
				username = args
				self.mucous.ModifyConfig("ban", username, '')
				self.mucous.ModifyConfig("ignore", username, '')
				
				self.mucous.Help.Log("status", "Nuked: %s" % username)
				
			elif command == "/unnuke" and args != '':
				username = args
				if self.mucous.config.has_key("ignored") and username in self.mucous.config["ignored"].keys():
					self.mucous.ModifyConfig("unignore", username, '')
				if self.mucous.config.has_key("banned") and username in self.mucous.config["banned"].keys():
					self.mucous.ModifyConfig("unban", username, '')
					
				self.mucous.Help.Log("status", "Irradiated: %s" % username)
				
			elif command == "/trust":
				username = args
				self.mucous.ModifyConfig("trust", username, '')
			elif command == "/distrust":	
				username = args
				self.mucous.ModifyConfig("distrust", username, '')
				
			elif command == "/autobuddy":
				if self.mucous.Config["mucous"]["autobuddy"]  == "yes":
					self.mucous.Config["mucous"]["autobuddy"] = "no"
					self.mucous.Help.Log("status", "AutoBuddy Disabled")
				elif self.mucous.Config["mucous"]["autobuddy"]  == "no":
					self.mucous.Config["mucous"]["autobuddy"] = "yes"
					self.mucous.Help.Log("status", "AutoBuddy Enabled")
				if self.mucous.mode == "setup":
					self.mucous.Setup.Mode()
			elif command == "/autoclear":
				if str(self.mucous.Config["mucous"]["auto-clear"]) == "yes":
					self.mucous.Config["mucous"]["auto-clear"] = "no"
				else:
					self.mucous.Config["mucous"]["auto-clear"] = "yes"
				if self.mucous.mode == "setup":
					self.mucous.Setup.Mode()
			elif command == "/autoretry":
				if str(self.mucous.Config["mucous"]["auto-retry"]) == "yes":
					self.mucous.Config["mucous"]["auto-retry"] = "no"
				else:
					self.mucous.Config["mucous"]["auto-retry"] = "yes"
				if self.mucous.mode == "setup":
					self.mucous.Setup.Mode()
				'''
				List Users in room
				'''
			elif command in ( "/list" , "/users"):
				if args != '':
					room = args
				else:
					room = self.mucous.ChatRooms.current
				if room != None:
					self.mucous.show_nick_list(room)
					
			elif command == "/roombox":
				self.mucous.ChatRooms.ChatLayout()
			elif command == "/roomwin":
				self.mucous.ChatRooms.WindowCycle()
			elif command == "/login":
				self.mucous.D.ConnectServer()
				
			elif command == "/logout":
				self.mucous.D.DisconnectServer()
				
			elif command == "/globalrex":
				self.mucous.D.GetGlobalRecommendations()

				
			elif command in ("/rex", "/recommendations"):
				self.mucous.D.GetRecommendations()
				
			elif command == "/uploadto" and args != "":
				user = args
				if user.isspace() == 0 and user != "":
					self.mucous.Transfers.username = user
					
			elif command == "/upload" and args != "":
				path = args
				if self.mucous.Transfers.username == None:
					return
				if path.isspace() != 0 and path == "":
					return
				self.mucous.Transfers.RetryUpload(self.mucous.Transfers.username, path)
					
			elif command in ("/similar", "/similarusers"):
				self.mucous.D.GetSimilarUsers()
				
			elif command == "/itemrex" and args != "":
				if args != "" and args.isspace() == 0:
					item = args
					self.mucous.D.GetItemRecommendations(item)
					
			elif command == "/itemsimilar" and args != "":
				if args != "" and args.isspace() == 0:
					item = args
					self.mucous.D.GetItemSimilarUsers(item)
					
			elif command == "/like" and args != "":
				interest = args
				self.mucous.Recommendations.LikedAdd(interest)
					
			elif command == "/hate" and args != "":
				interest = args
				self.mucous.Recommendations.HatedAdd(interest)
					
			elif command == "/donotlike" and args != "":
				interest = args
				if interest in self.mucous.config["interests.like"]:
					self.mucous.Recommendations.LikedRemove(interest)
					
			elif command == "/donothate" and args != "":
				interest = args
				if interest in self.mucous.config["interests.hate"]:
					self.mucous.Recommendations.HatedRemove(interest)
					
			elif command == "/transbox":
				if self.mucous.Config["mucous"]["transbox"]=="split":
					self.mucous.Config["mucous"]["transbox"]="tabbed"
				elif self.mucous.Config["mucous"]["transbox"]=="tabbed":
					self.mucous.Config["mucous"]["transbox"]="split"
				self.mucous.Transfers.ModeTransfers()
				'''
				List Rooms whose number of users is greater than the number you input
				'''
			elif command == "/roomlist":
				
				if args == '':
					self.mucous.RoomsList.Mode()
				elif args == 'refresh':
					self.mucous.D.RoomList()
					
			elif command in ("/privs", "/privileges"):
				self.mucous.D.CheckPrivileges()
				
			elif command == "/giveprivs" :
				try:
					self.mucous.usernames["privileges"]  = args
					self.mucous.SetEditTitle( "% Give Privileges to " + self.mucous.usernames["privileges"])
				except Exception, e:
					self.mucous.Help.Log("debug", str(e))
				
			elif command == "/inrooms" and args == '':
				w = ''
				for room in self.mucous.ChatRooms.rooms.keys():
					w += room + ', '
					
				self.mucous.Help.Log("status", "You are in: %s" %w[:-2])
				
				'''
				Manual Download
				'''	
			elif command == "/downuser" and  args != '':
				self.mucous.Transfers.ModeTransfers()
				self.mucous.Transfers.downloaduser = args
				self.mucous.SetEditTitle("% % User: "+args + " (input download path) % %")
				
			elif command == "/setdowndir" and  args != '':
				
				self.mucous.Transfers.downloaddirectory = args
				self.mucous.Transfers.ModeTransfers()
				self.mucous.SetEditTitle("Download directory set to: "+self.mucous.Transfers.downloaddirectory )
						
			elif command == "/downpath" and args != '':
				path = args
				user = self.mucous.Transfers.downloaduser
				self.mucous.DownloadFileTo(user, path)
					
			elif command == "/downpathdir" and args != '':
				directory = args
				user = self.mucous.Transfers.downloaduser
				self.mucous.Transfers.FolderDownload(user, directory)
	
				'''
				Search Globally for files & Download them
				'''
			elif command == "/searchfor" and args != '':
				query = args
				if query not in ('mp3', ' ') and len(query) > 2:
					self.mucous.D.Search(0, query)
					
				else:
					self.mucous.Search.Stats("status", "Query \""+ query+"\" was ignored", "default__", 0)
					
			elif command == "/searchuser" and args != '':
				self.mucous.Search.username = args
				if self.mucous.mode=='search':
					self.mucous.Search.Mode()
			elif command == "/searchbuddy" and args != '':
				query = args
				self.mucous.D.Search(1, query)
			elif command == "/searchroom" and args != '':
				query = args
				self.mucous.D.Search(2, query)
			elif command in ("/download", "/downdir") and args != "":
				linput = args
				nums = self.ParseIntegerSet(linput)
				
				if nums == None:
				    self.mucous.Help.Log("status", "Enter an Integer or Range")
				    return
				    
				if command == "/download":
					dtype = "file"
				elif command == "/downdir":
					dtype = "dir"
					
				for i in nums:
					if self.mucous.mode == "search":
						user, path = self.mucous.Search.GetDownloadFromNum(i)
					elif self.mucous.mode == "browse":
						user, path = self.mucous.BrowseShares.GetDownloadFromNum(i)
					else:
						return
					if dtype == "file":
						self.mucous.Transfers.RetryDownload(user, path)
					elif dtype == "dir":
						self.mucous.Transfers.FolderDownload(user, path)

			elif command == "/filter" and args != "":
				self.mucous.Search.sfilter = args
				if self.mucous.mode=='search':
					self.mucous.Search.Mode()
			elif command == "/filter" and args == "":
				self.mucous.Search.sfilter=None
				if self.mucous.mode=='search':
					self.mucous.Search.Mode()
				'''
				Browse Shares & Download from them
				'''
			elif command == "/cd" and args != '':
				self.mucous.ChangeDir(args)
				
			elif command in ("/getdir", "/get") and args != '':
				linput = args
				if command == "/get":
					dtype = "file"
				elif command == "/getdir":
					dtype = "dir"
				if linput == None or not linput.isdigit():
					self.mucous.Help.Log("status", "Enter an Integer")
					
				else:
					if self.mucous.mode == "search":
						user, path = self.mucous.Search.GetDownloadFromNum(linput)
						
					elif self.mucous.mode == "browse":	
						user, path = self.mucous.BrowseShares.GetDownloadFromNum(linput)
					else:
						return
					
					if dtype == "file":
						self.mucous.Transfers.RetryDownload(user, path)
					elif dtype == "dir":
						self.mucous.Transfers.FolderDownload(user, path)
						
			elif command == "/browsewidth" and args != "":
				if args.isdigit():
					width = int(args)
					if width > self.mucous.w-20:
						width = self.mucous.w-20
					if width < 20:
						width = 20 
					self.mucous.Config["mucous"]["browse_width"] = width
					
					if self.mucous.mode == "browse":
						self.mucous.BrowseShares.Mode()
					
			elif command in ("/browseuser",  "/buser") and args != '':
				user = None
				if command == "/browseuser":
					user = args
				elif command == "/buser":
					user = args
				if user != None:
					self.mucous.BrowseShares.Get(user)
					
			elif command in ("/browsesearch", "/bsearch") and args != "":
				l_input = args

				if l_input != None:
					self.mucous.BrowseShares.bfilter = re.compile('.*' +str(l_input) + '.*', re.DOTALL | re.I)
					self.mucous.BrowseShares.FormatBrowse()
					curses.doupdate()
					
			elif command in ("/browsesearch", "/bsearch") and args == '':
				self.mucous.BrowseShares.bfilter = None
				self.mucous.BrowseShares.FormatBrowse()
				curses.doupdate()
				'''
				Manage Transfers
				'''
			elif command in ("/abortd", "/abortdown") and args != '':
				nums = self.ParseIntegerSet(args)

				if nums == None:
					self.mucous.Help.Log("status", "Enter an Integer or Range")
					return

				for transfer in nums:
					user, path = self.mucous.Transfers.GetDownloadFromNum(transfer)
					self.mucous.Transfers.AbortDownload(user, path)
					
			elif command in ("/abortu", "/abortup") and args != '':
				nums = self.ParseIntegerSet(args)

				if nums == None: 
					self.mucous.Help.Log("status", "Enter an Integer or Range")
					return

				for transfer in nums:
					user, path = self.mucous.Transfers.GetUploadFromNum(transfer)
					self.mucous.Transfers.AbortUpload(user, path)
				
			elif command in ("/removeu", "/removeup") and args != '':
				nums = self.ParseIntegerSet(args)

				if nums == None:
					self.mucous.Help.Log("status", "Enter an Integer or Range")
					return

				for transfer in nums:
					user, path = self.mucous.Transfers.GetUploadFromNum(transfer)
					self.mucous.Transfers.ClearUpload(user, path)
				
			elif command in ("/removed", "/removedown") and args != '':
				nums = self.ParseIntegerSet(args)

				if nums == None:
					self.mucous.Help.Log("status", "Enter an Integer or Range")
					return

				for transfer in nums:
					user, path = self.mucous.Transfers.GetDownloadFromNum(transfer)
					self.mucous.Transfers.ClearDownload(user, path)
				

			elif command == "/retry" and args != '':
				nums = self.ParseIntegerSet(args)

				if nums == None:
					self.mucous.Help.Log("status", "Enter an Integer or Range")
					return

				for transfer in nums:
					user, path = self.mucous.Transfers.GetDownloadFromNum(transfer)
					self.mucous.Transfers.RetryDownload(user, path)
						
			elif command == "/retryall":
				self.mucous.Transfers.RetryAllDownloads()
		
			elif command == "/slots" and args != "":
				slots = None
				try:
					slots = int(args)
				except:
					self.mucous.Help.Log("status", "Enter an Integer")
				if slots != None:
					self.mucous.D.ConfigSet("transfers", "upload_slots", str(slots))
		
			elif command == "/privbuddy":
				if self.mucous.config["transfers"]["privilege_buddies"] == "true":
					self.mucous.D.ConfigSet("transfers", "privilege_buddies", "false")
					
				elif self.mucous.config["transfers"]["privilege_buddies"] == "false":
					self.mucous.D.ConfigSet("transfers", "privilege_buddies", "true")
					
					
			elif command == "/onlybuddy":
				if self.mucous.config["transfers"]["only_buddies"] == "true":
					self.mucous.D.ConfigSet("transfers", "only_buddies", "false")
				elif self.mucous.config["transfers"]["only_buddies"] == "false":
					self.mucous.D.ConfigSet("transfers", "only_buddies", "true")
		
			elif command == "/unhide":
				if self.mucous.Setup.password == True:
					self.mucous.Setup.password = False
				elif self.mucous.Setup.password == False:
					self.mucous.Setup.password = True
				if self.mucous.mode == "setup":
					self.mucous.Setup.Mode()
			elif command == "/buddyall":
				self.mucous.Help.Log("status", "Buddying ALL users currently transferring to or from you.")
				currentusersintransferlist = {}
				for userpath, values in self.mucous.Transfers.transfers["uploads"].items():
					currentusersintransferlist[values[1]] = 0
				for userpath, values in self.mucous.Transfers.transfers["downloads"].items():
					currentusersintransferlist[values[1]] = 0
				for username in currentusersintransferlist.keys():
					if not self.mucous.config.has_key("buddies") or username not in self.mucous.config["buddies"].keys():
						self.mucous.D.ConfigSet("buddies", username, "Buddied by mucous")
						
			elif command == "/nick" and args != '':
				if self.mucous.username != None:
					self.mucous.D.ConfigSet("server", "username", args)
			elif command == "/clearsearchs":
				self.mucous.Search.Clear()
				
			elif command == "/clearup":
				self.mucous.Transfers.ClearAllUploads()
				
			elif command == "/percent":
				if self.mucous.Transfers.speed == True:
					self.mucous.Transfers.speed = False
				elif self.mucous.Transfers.speed == False:
					self.mucous.Transfers.speed = True
				self.mucous.Transfers.ModeTransfers()
				
			elif command == "/cleardown":
				self.mucous.Transfers.ClearAllDownloads()
			elif command == "/clear":
				if self.mucous.mode == "chat":
					self.mucous.ChatRooms.ClearLog()
				elif self.mucous.mode == "private":
					self.mucous.PrivateChat.ClearLog()
						
			elif command == "/clearroom":
				if args == '':
					
					self.mucous.ChatRooms.logs["rooms"][self.mucous.ChatRooms.current] = []
					if self.mucous.mode == "chat":
						self.mucous.ChatRooms.Mode()
				elif args != '':
					if args in self.mucous.ChatRooms.logs["rooms"].keys():
						self.mucous.ChatRooms.logs["rooms"][args] = []
						if self.mucous.mode == "chat":
							self.mucous.ChatRooms.Mode()
			elif command == "/aliases":
				self.mucous.Help.Log("status", "Aliases:")
				for alias in self.mucous.Config["aliases"].keys():
					self.mucous.Help.Log("status", "/"+alias+": "+str(self.mucous.Config["aliases"][alias]))
					self.mucous.Help.Log("status", "")
					
			elif command == "/alias" and args != '':
				if args.find(" ") != -1:
					splited = args.split(" ")
					if len(splited) > 1:
						alias = splited[0]
						splited = splited[1:]
						if splited[0] != None:
							message = ''
							for i in splited:
								if i != splited[0]:
									message += ' ' +i
								else:
									message += i
							self.mucous.Config["aliases"][alias] = str(message)
							if alias in self.mucous.Config["aliases"].keys():
								self.mucous.Help.Log("status", "Modified alias: "+alias)
							else:
								self.mucous.Help.Log("status", "Created alias: "+alias)
							if "/"+alias not in self.mucous.commandlist:
								self.mucous.commandlist.append("/"+alias)
				else: 
					return 0
				
			elif command == "/unalias" and args != '':
				alias = args
				if alias in self.mucous.Config["aliases"].keys():
					self.mucous.Help.Log("status", "Deleted alias: "+alias)
					del self.mucous.Config["aliases"][str(alias)]
					if "/"+alias in self.mucous.commandlist:
						self.mucous.commandlist.remove("/"+alias)
						
			elif command[:1] == "/":
				is_alias = 0
				if  line[1:] in self.mucous.Config["aliases"].keys():
					alias = line[1:]
					is_alias = 1
					if self.mucous.mode == "chat" and self.mucous.ChatRooms.current != None:
						self.mucous.ChatRooms.SayInChat( self.mucous.ChatRooms.current, self.mucous.Config["aliases"][alias])
						
					elif self.mucous.mode == "private" and self.mucous.PrivateChat.current != None:
						self.mucous.PrivateChat.Send( self.mucous.PrivateChat.current, self.mucous.Config["aliases"][alias])
						
				if not is_alias:
					return 0
				
			else:
				return 1
			
				
			
		except Exception,e:
			self.mucous.Help.Log("debug", 'commands: ' + str(e))
		pass
	
	def ReadURL(self, args):

		self.mucous.url = None
		logfile = None
		if self.mucous.mode == "chat" and self.mucous.ChatRooms.current != None:
			logfile = []
			for line in self.mucous.ChatRooms.logs["rooms"][self.mucous.ChatRooms.current]:
				logfile.append(line)
		elif self.mucous.mode == "private" and self.mucous.PrivateChat.current != None:
			logfile = []
			for line in self.mucous.PrivateChat.logs[self.mucous.PrivateChat.current]:
				logfile.append(line)
		if logfile != None:
			lene = len(logfile)
			logfile.reverse()
			if self.mucous.mode == "chat":
				for line in logfile:
					if "://" in line[3]:
						urline = line[3].split(" ")
						for x in urline:
							if "://" in x: 
								self.mucous.url = x
								break
						break
			elif self.mucous.mode == "private":
				for timestamp, user, message in logfile:
					if "://" not in message:
						continue
					urline = message.split(" ")
					for x in urline:
						if "://" in x:
								self.mucous.url = x
								break
					break
				

		if self.mucous.url != None:
			urlr = self.mucous.Config["mucous"]["url reader"]
			command = None
			if  urlr == "links":
				if os.path.expandvars("$TERM") != "linux" and os.path.exists("/usr/bin/links"):
					command = "xterm -e 'TERM=xterm-color links "+self.mucous.url +"'"
			elif urlr == "elinks":
				if os.path.expandvars("$TERM") != "linux" and os.path.exists("/usr/bin/elinks"):
					command = "xterm -e 'TERM=xterm-color elinks "+self.mucous.url +"'"
			elif urlr == "lynx":
				if os.path.expandvars("$TERM") != "linux" and os.path.exists("/usr/bin/lynx"):
					command = "xterm -e 'TERM=xterm-color lynx "+self.mucous.url +"'"	
			elif urlr == "firefox":
				command = "firefox -a firefox -remote 'openURL("+self.mucous.url +",new-tab)'"
			elif urlr == "custom":
				if self.mucous.Config["mucous"]["url custom"] != "$":
					command = self.mucous.Config["mucous"]["url custom"].replace("$", self.mucous.url)
				
			if command is not None:
				os.system("%s &> /dev/null &" %command)
				self.mucous.Help.Log("status", "Running Process: %s" % command)
	
	## Special Function that parses the input line, if it's not a command
	# @param self is CharacterParse (Class)
	# @param line is a text string
	def InputText(self, line):
		## Special Input Box for Downloading Manually
		# escape //
		if line[:2] == '//':
			line = line[1:]
		# Manual Download input box
		if self.mucous.Spl["title"][:10] == '% % User: ' and line != '':
			if self.mucous.Transfers.downloaduser != None and self.mucous.Transfers.downloaduser != '':
				path = line
				self.mucous.D.DownloadFile(self.mucous.Transfers.downloaduser, path)
				self.mucous.Help.Log("status", "Trying to Download: " + path+" from "+ self.mucous.Transfers.downloaduser)
		# Ticker set input box
		elif self.mucous.Spl["title"][:12] == '% Set ticker' and line != '':
			self.mucous.D.RoomTickerSet(self.mucous.Spl["ticker_room"], line)
			
		elif self.mucous.Spl["title"]== '% Give Privileges to ' + str(self.mucous.usernames["privileges"]) and line != '':
			try:
				days = int(line)
				self.mucous.D.GivePrivileges(self.mucous.usernames["privileges"], days)
				self.mucous.usernames["privileges"] = None
				if self.mucous.mode == "chat":
					self.mucous.SetEditTitle(self.mucous.ChatRooms.current)
			except:
				self.mucous.Help.Log("debug", "Enter the Number of days of privileges you wish to give " + self.mucous.usernames["privileges"])
	
		else:
			if self.mucous.mode == "setup":
				self.mucous.Setup.InputSetup(line)
				return
			elif self.mucous.mode == "search":
				self.mucous.Search.LineInput(line)
				return
			if line != '':
				if self.mucous.mode == "chat":
					#Normal Chat Room Message
					if self.mucous.ChatRooms.current:
						self.mucous.ChatRooms.SayInChat( self.mucous.ChatRooms.current, line)
				elif self.mucous.mode == "private":
					#Normal Private Messaging
					if self.mucous.PrivateChat.current != None:
						# Private Message
						self.mucous.PrivateChat.Send(self.mucous.PrivateChat.current, line)
					else:
						# Set user to message
						self.mucous.PrivateChat.current = self.mucous.dlang( line)
						self.mucous.SetEditTitle("Send message to: " + self.mucous.PrivateChat.current)
						self.mucous.PrivateChat.Start(self.mucous.PrivateChat.current)
						

				elif self.mucous.mode == "browse":
					# Browse User's shares
					if line[:3] == "cd ":
						self.mucous.ChangeDir(line[3:])
						return 1

					elif line[:3] == "get" and line[3:] != '':
						linput = None
						if line[:4] == "get " and line[4:] != '':
							dtype = "file"
							linput = line[4:]
						elif line[:7] == "getdir "  and line[7:] != '':
							dtype = "dir"
							linput = line[7:]
						if linput == None or not linput.isdigit():
							self.mucous.Help.Log("status", "Enter an Integer")
							
						else:
							user, path = self.mucous.BrowseShares.GetDownloadFromNum(linput)
							if dtype == "file":
								self.mucous.Transfers.RetryDownload(user, path)
							elif dtype == "dir":
								self.mucous.Transfers.FolderDownload(user, path)
					
					
					self.mucous.BrowseShares.Get(line)
					
					
				elif self.mucous.mode == "info":
					# Get User's UserInfo and PeerStats
					user = self.mucous.dlang(line)
					self.mucous.UserInfo.Get(user)
					
					self.mucous.D.PeerStats(user)
					
				elif self.mucous.mode == "lists":
					if self.mucous.UsersLists.current == "buddied":
						self.mucous.ModifyConfig("buddy", line, '')
					elif self.mucous.UsersLists.current == "banned":
						self.mucous.ModifyConfig("ban", line, '')
					elif self.mucous.UsersLists.current == "ignored":
						self.mucous.ModifyConfig("ignore", line, '')
					elif self.mucous.UsersLists.current == "trusted":
						self.mucous.ModifyConfig("trusted", line, '')
					elif self.mucous.UsersLists.current == "interests":
						self.mucous.Recommendations.InputInterests(line)
				elif self.mucous.mode == "roomlist":
					self.mucous.ChatRooms.JoinRoom(line)
				
				
		try:
			self.mucous.edit.reset()
		except:
			pass


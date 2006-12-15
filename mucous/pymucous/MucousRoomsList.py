# This is part of the Mucous Museek Client, and distributed under the GPLv2
# Copyright (c) 2006 daelstorm. 
import curses.wrapper
## RoomsList
# Scrollable and sortable list of rooms on the server
class RoomsList:
	## Constructor (create initial variables)
	# @param self RoomsList (class)
	# @param mucous Mucous (class)
	def __init__(self, mucous):
		## @var mucous 
		# Mucous (Class)
		self.mucous = mucous
		## @var windows
		# dict containing instances of curses windows
		self.windows = {}
		## @var scrolling
		# vertical position in list
		self.scrolling = 0
		## @var dimensions
		# dict containing placement data for windows
		self.dimensions = {}
		## @var rooms
		# dict containing rooms and their sizes; updated by server
		self.rooms = {}
		## @var sizedrooms
		# Sorted rooms; no size data; rebuilt every time viewed
		self.sizedrooms = []
		
	## Build windows and display rooms
	# @param self RoomsList (class)#
	def Mode(self):
		try:
			self.mucous.UseAnotherEntryBox()
			self.mucous.mode = "roomlist"
			self.mucous.PopupMenu.show = False
			
			s = self.dimensions = {"height": self.mucous.h-7, "top": 2, "left": 1, "width": self.mucous.w-2}
			self.DrawWindow()
			
			self.sizedrooms = []
			alpharooms = []
			
			if self.mucous.Config["mucous"]["rooms_sort"]  in ("alpha", "alpha-reversed"):
				for rooms in self.rooms.keys():
					alpharooms.append(rooms)
				alpharooms.sort(key=str.lower)
				if self.mucous.Config["mucous"]["rooms_sort"] =="alpha-reversed":
					alpharooms.reverse()
					
			elif self.mucous.Config["mucous"]["rooms_sort"] in ("size", "size-reversed"):
				bigsizes = []
				bigsizes = self.mucous.FormatData.sortbyvalue (self.rooms)
				if self.mucous.Config["mucous"]["rooms_sort"] == "size":
					bigsizes.reverse()
				for rooms, sizes in bigsizes:
					alpharooms.append(rooms)
			
			for rooms9 in alpharooms:
				if self.rooms[rooms9] >= self.mucous.Config["mucous"]["roomlistminsize"]:
					self.sizedrooms.append(rooms9)
					
			self.Format()
			
			self.mucous.HotKeyBar()
			curses.doupdate()
		except Exception, e:
			self.mucous.Help.Log("debug", "RoomsList.Mode: " + str(e))
		

	## Mouse Coordinates in the RoomsList
	# @param self is RoomsList (Class)
	# @param x is the horizontal position from the left
	# @param y is the vertical postion from the top
	# @param z is unimportant
	# @param event is the mouse event (button, doubleclick, etc) represented by a number
	def Mouse(self, x,y,z,event):
		try:
			if y == self.mucous.h-5:
				if x >= 14 and x <= 22:
					self.mucous.Config["mucous"]["rooms_sort"] = "alpha"
					self.Mode()
				elif x >= 24 and x <= 35:
					self.mucous.Config["mucous"]["rooms_sort"] = "alpha-reversed"
					self.Mode()
				elif x >= 37 and x <= 46:
					self.mucous.Config["mucous"]["rooms_sort"] = "size"
					self.Mode()
				elif x >= 47 and x <= 56:
					self.mucous.Config["mucous"]["rooms_sort"] = "size-reversed"
					self.Mode()
				elif x >= self.mucous.w-16:
					self.mucous.D.RoomList()
					
			w = self.dimensions
			if y >= w["top"] and y < w["top"] + w["height"] and x >= w["left"] and x < w["left"] +w["width"]:
				y -= w["top"]
				if y  + w["start"] in range(len(self.sizedrooms)):
					self.scrolling  = y  + w["start"]
					self.Format()
					return self.sizedrooms[self.scrolling]
		except Exception, e:
			self.mucous.Help.Log("debug", "RoomsList.Mouse: " +str(e) )
	
	## Build and display RoomsList border and sort bar
	# @param self is RoomsList (Class)
	def DrawWindow(self):
		try:
			# Cleanup stale windows
			if "text" in self.windows:
				del self.windows["text"]
			if "border" in self.windows:
				del self.windows["border"]
				
			s = self.dimensions
			mw = self.windows["border"] = curses.newwin(s["height"]+2, s["width"]+2, s["top"]-1, s["left"]-1)
			mw.attron(self.mucous.colors["green"])
			mw.border()
			mw.attroff(self.mucous.colors["green"])
			
			mw.addstr(0, 3, "< Room List >",  self.mucous.colors["green"] | curses.A_BOLD)
			pos = 3
			sorta = "< "
			sort = "Sort by:"
			sortnaz = " Name A-Z"
			sortnza = " Name Z-A"
			sorts90 =" Size 9-0"
			sorts09 =" Size 0-9"
			quick = self.mucous.Config["mucous"]["rooms_sort"]
			sortnaz_color = sortnza_color = sorts90_color = sorts09_color = curses.A_NORMAL
			if quick == "size":
				sorts90_color = self.mucous.colors["green"]
			elif quick == "size-reversed":
				sorts09_color = self.mucous.colors["green"]
			elif quick == "alpha-reversed":
				sortnza_color = self.mucous.colors["green"]
			elif quick == "alpha":
				sortnaz_color = self.mucous.colors["green"]
			mw.addstr(self.mucous.h-6, pos, sorta,  self.mucous.colors["green"] | curses.A_BOLD)
			pos += 2
			mw.addstr(self.mucous.h-6, pos, sort,  curses.A_BOLD)
			pos += len(sort)
			mw.addstr(self.mucous.h-6, pos, sortnaz,  sortnaz_color | curses.A_BOLD)
			pos += len(sortnaz)
			mw.addstr(self.mucous.h-6, pos, " |",  self.mucous.colors["green"] | curses.A_BOLD)
			pos += 2
			mw.addstr(self.mucous.h-6, pos, sortnza,  sortnza_color | curses.A_BOLD)
			pos += len(sortnza)
			mw.addstr(self.mucous.h-6, pos, " |",  self.mucous.colors["green"] | curses.A_BOLD)
			pos += 2
			mw.addstr(self.mucous.h-6, pos, sorts90, sorts90_color | curses.A_BOLD)
			pos += len(sorts90)
			mw.addstr(self.mucous.h-6, pos, " |",  self.mucous.colors["green"] | curses.A_BOLD)
			pos += 2
			mw.addstr(self.mucous.h-6, pos, sorts09,  sorts09_color | curses.A_BOLD)
			pos += len(sorts09)
			mw.addstr(self.mucous.h-6, pos, " >",  self.mucous.colors["green"] | curses.A_BOLD)
			mw.addstr(self.mucous.h-6, self.mucous.w-15, "< Refresh >",  self.mucous.colors["green"] | curses.A_BOLD)
			self.mucous.SetEditTitle("Join a Room")
		
			mw.noutrefresh()
			tw = self.windows["text"] =  mw.subwin(s["height"], s["width"], s["top"], s["left"])
			tw.attron(self.mucous.colors["green"])
			#tw.border()
			tw.attroff(self.mucous.colors["green"])
			#self.scrolling
			self.scrolling = 0
			tw.noutrefresh()
		except Exception, e:
			self.mucous.Help.Log("debug", "RoomsList.DrawWindow: " + str(e))
			
	## Display one room 
	# @param self is RoomsList (Class)
	# @param roomitem a string
	# @param count position in the compacted list
	# @param start padding required for position to be accurate
	def Draw(self, roomitem, count, start):
		try:
			
			if count + start == self.scrolling:
				attrib =  curses.A_BOLD | curses.A_REVERSE
			else:
				attrib = curses.A_NORMAL
			num = str(self.rooms[roomitem])
			while len(num) < 10:
				num += " "
			string = num + self.mucous.dlang(roomitem)
			if len(string) < self.mucous.w-2:
				spaces = " " * (self.mucous.w-2 - len(string))
			else:
				string = string[:self.mucous.w-2]
				spaces = ''
			self.windows["text"].addstr(string+spaces, attrib)
			
		except Exception, e:
			pass
			# Always errors
			#self.mucous.Help.Log("debug", "RoomsList.Draw: " + str(e))
	
	## Display all rooms; Calls Draw 
	# @param self is RoomsList (Class)
	def Format(self):
		try:
			self.windows["text"].erase()
			clipped_list, self.scrolling, self.dimensions["start"] = self.mucous.FormatData.scrollbox(self.sizedrooms, self.scrolling, self.mucous.h-7)
			count =0 
			for rooms10 in clipped_list:
				self.Draw(rooms10, count, self.dimensions["start"])
				count += 1
			self.windows["text"].refresh()
		except Exception, e:
			print e
			self.mucous.Help.Log("debug", "RoomsList.Format: " + str(e))
				

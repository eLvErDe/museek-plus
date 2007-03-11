
import curses.wrapper
## Recommendations
#			
class Recommendations:
	## Constructor
	# @param self Recommendations (class)
	# @param mucous Mucous (class)
	def __init__(self, mucous):
		## @var mucous 
		# Mucous (Class)
		self.mucous = mucous
		## @var dimensions
		# dict containing placement data for windows
		self.dimensions = {}
		## @var input
		# in which interets list to input data
		self.input = "add_likes"
		## @var sort
		# sorting method alphabetical, size
		self.sort = "alpha"
		## @var selected
		# highlighted window
		self.selected = "recommendations"
		## @var logs
		# holds temporary data, until the next scroll update
		self.logs = {"recommendations": [], "likes": [], "hates": [], "similar_users":[] }
		## @var windows
		# dict containing curses window instances
		self.windows = {"text":{}, "border":{}}
		## @var scrolling
		# dict containing scroll position for likes, hates, recommendations and similar users
		self.scrolling = {"likes": 0, "hates": 0, "recommendations": 0, "similar_users": 0}
		## @var data
		# holds temporary data, until the next server update
		self.data = {"recommendations": {}, "similar_users": {}}


	## Create the four windows and call the draw functions
	# @param self Recommendations (class)
	def ModeInterests(self):
		try:
			#self.selected = "recommendations"
			self.mucous.PopupMenu.show = False
# 			self.scrolling[ "recommendations" ] = self.scrolling[ "likes" ] = self.scrolling[ "hates" ] = self.scrolling[ "similar_users" ] = 0
			self.dimensions["recommendations"] = s = {"height": self.mucous.h-8, "top": 3, "left": 21, "width": self.mucous.w-42, "start": 0}
			self.dimensions["likes"] = l = {"height": self.mucous.h/2-5, "top": 3, "left": 1, "width": 18, "start": 0}
			self.dimensions["hates"] = h = {"height": self.mucous.h-8-(l["height"])-2, "top": 5+l["height"], "left": 1, "width": 18, "start": 0}
			self.dimensions["similar_users"] = u = {"height": self.mucous.h-8, "top":3 , "left": self.mucous.w-19, "width": 18, "start": 0}
			
			mw = curses.newwin(3, self.mucous.w,  1, 0)
			mw.attron(self.mucous.colors["green"])
# 			win = curses.newwin(w["height"]+2, w["width"]+2, w["top"]-1, w["left"]-1)
			mw.erase()
			mw.border()
			mw.addstr(0, 3, "< Buddied >",  self.mucous.colors["green"])
			mw.addstr(0, 16, "< Banned >", self.mucous.colors["green"])
			mw.addstr(0, 28, "< Ignored >", self.mucous.colors["green"])
			mw.addstr(0, 40, "< Trusted >", self.mucous.colors["green"] )
			mw.addstr(0, 52, "< Interests >", self.mucous.colors["green"]  | curses.A_BOLD)
			mw.noutrefresh()
			# Cleanup stale windows
			del mw
	

			if self.selected == "likes":
				if self.input not in ("add_likes", "del_likes"):
					self.input = "add_likes"
				if self.input == "add_likes":
					self.mucous.SetEditTitle("Add Liked (Alt-D to Toggle to Delete)")
				elif self.input == "del_likes":
					self.mucous.SetEditTitle("Delete Liked (Alt-D to Toggle to Add)")
			elif self.selected == "hates":
				if self.input not in ("add_hates", "del_hates"):
					self.input = "add_hates"
				if self.input == "add_hates":
					self.mucous.SetEditTitle("Add Hated (Alt-D to Toggle to Delete)")
				elif self.input == "del_hates":
					self.mucous.SetEditTitle("Delete Hated (Alt-D to Toggle to Add)")
			elif self.selected == "recommendations":
				self.mucous.SetEditTitle("Recommendations:")
			elif self.selected == "similar_users":
				self.mucous.SetEditTitle("Similar Users:")
			self.InterestsWindows(s, "recommendations")
			self.InterestsWindows(l, "likes")
			self.InterestsWindows(h, "hates")
			self.InterestsWindows(u, "similar_users")
			self.DrawInterests()

			
			self.mucous.HotKeyBar()
			curses.doupdate()
		except Exception, e:
			self.mucous.Help.Log("debug", "ModeInterests: " + str(e))
			
	## Mouse events in likes, hates, recommendations and similar users
	# @param self Recommendations (class)
	# @param x vertical
	# @param y horizontal
	# @param z doesn't matter
	# @param event is the mouse event (button, doubleclick, etc) represented by a number
	# @return line
	def MouseInterests(self, x,y,z,event):
		try:
			rexw = self.dimensions["recommendations"]
			userw = self.dimensions["similar_users"]
			hatew = self.dimensions["hates"]
			likew = self.dimensions["likes"]
			
			if y >= userw["top"]-1 and y < userw["top"] + userw["height"]+1 and x >= userw["left"]-1 and x < userw["left"]+1 +userw["width"]:
				w = userw
				if self.selected != "similar_users":
					self.selected = "similar_users"
			
				if y == w["top"] + w["height"]:
					if x >= w["left"]+w["width"] -11 and x <=  w["left"]+w["width"]:
						self.mucous.D.GetSimilarUsers()
				if y >= w["top"] and y < w["top"] + w["height"] and x >= w["left"] and x < w["left"] +w["width"]:
					y -= w["top"]
					this_list = self.logs[self.selected]
					if y  + w["start"] in range(len(this_list)):
						self.scrolling[self.selected]  = y  + w["start"]
						self.mucous.line = str(self.logs[self.selected][self.scrolling[self.selected]][0])
				self.ModeInterests()
				return self.mucous.line
			elif y >= likew["top"]-1 and y < likew["top"] + likew["height"]+1 and x >= likew["left"]-1 and x < likew["left"]+1 +likew["width"]:
				w = likew
				if self.selected != "likes":
					self.selected = "likes"
					
				
				if y == w["top"] + w["height"]:
					
					if x >= w["left"]+2 and x <= w["left"]+7:
						
						if self.input == "add_likes":
							self.LikedAdd(self.mucous.line)
						else:
							self.input = "add_likes"
					elif x >= w["left"]+9 and x <= w["left"]+w["width"]:
						if self.input == "del_likes":
							self.LikedRemove(self.mucous.line)
						else:
							self.input = "del_likes"
						
				if y >= w["top"] and y < w["top"] + w["height"] and x >= w["left"] and x < w["left"] +w["width"]:
					y -= w["top"]
					this_list = self.logs[self.selected]
					
					if y  + w["start"] in range(len(this_list)):
						self.scrolling[self.selected]  = y  + w["start"]
						self.mucous.line = self.logs[self.selected][self.scrolling[self.selected]]

					
				self.ModeInterests()
				return self.mucous.line
			elif y >= hatew["top"]-1 and y < hatew["top"] + hatew["height"]+1 and x >= hatew["left"]-1 and x < hatew["left"]+1 +hatew["width"]:
				w = hatew
				if self.selected != "hates":
					self.selected = "hates"
				if y == w["top"] + w["height"]:
					if x >= 2 and x <= 7:
						if self.input == "add_hates":
							self.HatedAdd(self.mucous.line)
						else:
							self.input = "add_hates"
						
					elif x >= w["left"]+9 and x <= w["left"]+w["width"]:
						if self.input == "del_hates":
							self.HatedRemove(self.mucous.line)
						else:
							self.input = "del_hates"
				if y >= w["top"] and y < w["top"] + w["height"] and x >= w["left"] and x < w["left"] +w["width"]:
					y -= w["top"]
					this_list = self.logs[self.selected]
					if y  + w["start"] in range(len(this_list)):
						self.scrolling[self.selected]  = y  + w["start"]
						self.mucous.line = self.logs[self.selected][self.scrolling[self.selected]]
				self.ModeInterests()
				return self.mucous.line
					
			elif y >= rexw["top"]-1 and y < rexw["top"] + rexw["height"]+1 and x >= rexw["left"]-1 and x < rexw["left"]+1 +rexw["width"]:
				w = rexw
				if self.selected != "recommendations":
					self.selected = "recommendations"
				if y == w["top"] -1:
					if x >= w["left"]+w["width"] -len(self.sort) -5 and x <=  w["left"]+w["width"]:
						if self.sort == "size":
							self.sort = "alpha"
						elif self.sort == "alpha":
							self.sort = "size"
				elif y == w["top"] + w["height"]:
					if x >= w["left"]+ 1 and x <=  w["left"]+12:
						self.mucous.D.GetRecommendations()
					elif x >= w["left"]+w["width"] -10 and x <=  w["left"]+w["width"]:
						self.mucous.D.GetGlobalRecommendations()
				elif y >= w["top"] and y < w["top"] + w["height"] and x >= w["left"] and x < w["left"] +w["width"]:
					y -= w["top"]
					this_list = self.logs[self.selected]
					if y  + w["start"] in range(len(this_list)):
						self.scrolling[self.selected]  = y  + w["start"]
						self.mucous.line = self.logs[self.selected][self.scrolling[self.selected]][0]
						
				self.ModeInterests()
				return self.mucous.line
		except Exception, e:
			self.mucous.Help.Log("debug", "MouseInterests: " +str(e) )
			
	
	## Clear recommendations window and read from self.data
	# @param self Recommendations (class)
	def DrawRecommendations(self):
		try:
			mode  = "recommendations"
			self.windows["text"][mode].erase()
			w = self.dimensions[mode]
			self.logs[mode] = []
			sup = self.scrolling[ mode ]
			if self.data[mode] != {}:
				if self.sort == "alpha":
					dict = self.data[mode].keys()
					dict.sort(key=str.lower)
					for item in dict:
						self.logs[mode].append( [item, self.data[mode][item] ])

				elif self.sort == "size":
					dict = self.mucous.FormatData.sortbyvalue(self.data[mode])
					dict.reverse()
					for item, num in dict:
						self.logs[mode].append( [item, num])
				
					
			else:
				self.logs[mode] = [["EMPTY RECOMMENDATIONS", 0]]
			count = 0
			clipped_list, sup, self.dimensions[mode]["start"] = self.mucous.FormatData.scrollbox(self.logs[mode], sup, w["height"])
			self.scrolling[mode] = sup
			start = self.dimensions[mode]["start"]
			for item, num in clipped_list:
				length = len( item+( " " * (w["width"] - len(item)-len(str(num))))+str(num ) )
				if length > w["width"]:
					item = item [:w["width"] - length  ]
				filler = item +( " " * (w["width"] - len(item)-len(str(num))))+str(num )
				if count + start == sup:
					try:
						self.windows["text"][mode].addstr(filler, curses.A_BOLD)
					except: pass
				else:
					try:
						self.windows["text"][mode].addstr(filler)
					except: pass
				count += 1
			self.windows["text"][mode].refresh()
		except Exception, e:
			self.windows["text"][mode].refresh()
 			self.mucous.Help.Log("debug", "DrawRecommendations: " + str(e))
			
	## Clear users window and read from self.data
	# @param self Recommendations (class)
	def DrawSimilarUsers(self):
		try:
			mode  = "similar_users"
			self.windows["text"][mode].erase()
			sup = self.scrolling[ mode ]
			w = self.dimensions[mode]
			if self.data[mode] != {}:
				self.logs[mode] = []
				
				users = self.data[mode].keys()
				users.sort(key=str.lower)
				for user in users:
					self.logs[mode].append( [user, self.data[mode][user] ])
				
				 
				
				clipped_list, sup, start = self.mucous.FormatData.scrollbox(self.logs[mode], sup, w["height"])
				self.dimensions[mode]["start"] = start
				self.scrolling[mode] = sup
			else:
				clipped_list = [["NO SIMILAR USERS", 0]]
				start = 0
			count = 0
			for user, status in clipped_list:
				if count + start == sup:
					attr = curses.A_BOLD
				else: 
					attr = curses.A_NORMAL
				if status == 1:
					self.windows["text"][mode].addstr("* ", self.mucous.colors["yellow"] | attr)
				elif status == 2:
					self.windows["text"][mode].addstr("* ", self.mucous.colors["green"] | attr)
				else:
					self.windows["text"][mode].addstr("* " , attr)
				if len (user )+2 > w["width"]:
					user = user[ :w["width"]-2-len(user) ]
				try:
					self.windows["text"][mode].addstr(user + " " * (w["width"] - len(user)-2),   attr)
				except: pass
				count +=1
				
			
			self.windows["text"][mode].refresh()
		except Exception, e:
			self.mucous.Help.Log( "debug", "DrawSimilarUsers " + str(e) + str(self.scrolling["similar_users" ]))
			self.windows["text"]["similar_users"].refresh()
	
	## Draw everything
	# @param self Recommendations (class)
	def DrawInterests(self):
		self.DrawLiked()
		self.DrawHated()
		self.DrawRecommendations()
		self.DrawSimilarUsers()
		
	## Clear hated window and read from self.data
	# @param self Recommendations (class)
	def DrawHated(self):
		try:
			mode = "hates"

			self.windows["text"][mode].erase()
			w = self.dimensions[mode]
			self.logs[mode] = []
			sup = self.scrolling[ mode ]
			if self.mucous.config.has_key("interests.hate"):
				hates = self.mucous.config["interests.hate"].keys()
				hates.sort(key=str.lower)
				for item in hates:
					self.logs[mode].append( item)
	
			count = 0
			clipped_list, sup, self.dimensions[mode]["start"] = self.mucous.FormatData.scrollbox(self.logs[mode], sup, w["height"])
			self.scrolling[mode] = sup
			start = self.dimensions[mode]["start"]
			for line in clipped_list:
				length = len( line+( " " * (w["width"] - len(line) ) ) )
				if length > w["width"]:
					line = line [:w["width"] - length  ]
				if count + start == sup:
					self.windows["text"][mode].addstr(line + " " * (self.dimensions["hates"]["width"] - len(line)), curses.A_BOLD)
				else:
					self.windows["text"][mode].addstr(line + " " * (self.dimensions["hates"]["width"] - len(line)))
				count += 1
			self.windows["text"][mode].refresh()
		
		except Exception, e:
			#self.mucous.Help.Log( "debug", "DrawHated " + str(e))
			self.windows["text"]["hates"].refresh()
			
	## Clear liked window and read from self.data
	# @param self Recommendations (class)
	def DrawLiked(self):
		try:
			mode = "likes"
			self.windows["text"][mode].erase()
			w = self.dimensions[mode]
			self.logs[mode] = []
			sup = self.scrolling[ mode ]

			if self.mucous.config.has_key("interests.like"):
				likes = self.mucous.config["interests.like"].keys()
				likes.sort(key=str.lower)
				for item in likes:
			
					self.logs[mode].append( item)
	
			count = 0
			clipped_list, sup, self.dimensions[mode]["start"] = self.mucous.FormatData.scrollbox(self.logs[mode], sup, w["height"])
			self.scrolling[mode] = sup
			start = self.dimensions[mode]["start"]
			for line in clipped_list:
				length = len( line+( " " * (w["width"] - len(line) ) ) )
				if length > w["width"]:
					line = line [:w["width"] - length  ]
				if count + start == sup:
					self.windows["text"][mode].addstr(line + " " * (w["width"] - len(line)), curses.A_BOLD)
				else:
					self.windows["text"][mode].addstr(line + " " * (w["width"] - len(line)))
				count += 1
			self.windows["text"][mode].refresh()
		except Exception, e:

			self.windows["text"][mode].refresh()
			
	## Creates all windows, exact stored in it's own self.windows[type][mode] slot
	# @param self Recommendations (class)
	# @param w dimensions dict
	# @param mode window name
	def InterestsWindows(self, w, mode):
		try:
			# Cleanup stale windows
			if mode in self.windows["text"]:
				del self.windows["text"][mode]
			if mode in self.windows["border"]:
				del self.windows["border"][mode]
				 
			self.windows["border"][mode] = win = curses.newwin(w["height"]+2, w["width"]+2, w["top"]-1, w["left"]-1)
			if self.selected == mode:
				win.attron(self.mucous.colors["green"])
			win.border()
			if mode == "recommendations":
				title = "Recommendations"
				rs = len(self.sort)
				win.addstr(0, w["width"]-rs-4, "< %s >" % (" "*rs))
				win.addstr(0, w["width"]-rs-2, self.sort.capitalize(),  curses.A_BOLD | self.mucous.colors["cyan"])
				win.addstr(w["height"]+1, 1, "<          >")
				win.addstr(w["height"]+1, 3, "PERSONAL",  curses.A_BOLD | self.mucous.colors["cyan"])
				win.addstr(w["height"]+1, w["width"]-10, "<        >")
				win.addstr(w["height"]+1, w["width"]-8, "GLOBAL",  curses.A_BOLD | self.mucous.colors["cyan"])
			elif mode == "likes":
				title = "Liked"
				if self.input == "add_likes":
					aattr = curses.A_BOLD | self.mucous.colors["cyan"]
					dattr = curses.A_NORMAL
				elif self.input  == "del_likes":
					aattr = curses.A_NORMAL
					dattr = curses.A_BOLD | self.mucous.colors["cyan"]
				else:
					aattr = curses.A_NORMAL
					dattr = curses.A_NORMAL
				win.addstr(w["height"]+1, 1, "<     >")
				win.addstr(w["height"]+1, 3, "ADD", aattr)
				win.addstr(w["height"]+1, w["width"]-9, "<        >")
				win.addstr(w["height"]+1, w["width"]-7, "DELETE", dattr)
			elif mode == "hates":
				title = "Hated"
				if self.input == "add_hates":
					aattr = curses.A_BOLD | self.mucous.colors["cyan"]
					dattr = curses.A_NORMAL
				elif self.input  == "del_hates":
					aattr = curses.A_NORMAL
					dattr = curses.A_BOLD | self.mucous.colors["cyan"]
				else:
					aattr = curses.A_NORMAL
					dattr = curses.A_NORMAL
				win.addstr(w["height"]+1, 1, "<     >")
				win.addstr(w["height"]+1, 3, "ADD", aattr)
				win.addstr(w["height"]+1, w["width"]-9, "<        >")
				win.addstr(w["height"]+1, w["width"]-7, "DELETE", dattr)
			elif mode == "similar_users":
				title = "Users"
				win.addstr(w["height"]+1, w["width"]-10, "<         >")
				win.addstr(w["height"]+1, w["width"]-8, "REFRESH",  curses.A_BOLD | self.mucous.colors["cyan"])
			
			win.addstr(0, 1, "< %s >" % (" " * len(title)))
			if mode == self.selected:
				 win.addstr(0, 3, title, curses.A_BOLD | self.mucous.colors["green"])
			else:
				 win.addstr(0, 3, title, curses.A_BOLD)
			if self.selected == mode:
				win.attroff(self.mucous.colors["green"])
			win.refresh()
			self.windows["text"][mode] = twin = win.subwin(w["height"], w["width"], w["top"], w["left"])
			
			twin.scrollok(0)
			twin.idlok(1)
	
			

		except Exception, e:
			self.mucous.Help.Log( "debug", "InterestsWindows: " + str(e))
			
	
	## Add an item of interest to your liked list
	# @param self is mucous
	# @param interest is a text string
	def LikedAdd(self, interest):
		try:
			if not interest.isspace() and interest != "":
				self.mucous.D.AddInterest(interest)
		except Exception,e:
			self.mucous.Help.Log("debug", "InterestLikedAdd: " + str(e))	
			
	## Add an item of interest to your hated list
	# @param self is mucous
	# @param interest is a text string
	def HatedAdd(self, interest):
		try:
			if not interest.isspace() and interest != "":
				self.mucous.D.AddHatedInterest(interest)
		except Exception,e:
			self.mucous.Help.Log("debug", "InterestHatedAdd: " + str(e))
	
	## Remove an item of interest from your liked list
	# @param self is mucous
	# @param interest is a text string
	def LikedRemove(self, interest):
		try:
			if not interest.isspace() and interest != "":
				if interest in self.mucous.config["interests.like"]:
					self.mucous.D.RemoveInterest(interest)
		except Exception,e:
			self.mucous.Help.Log("debug", "InterestLikedRemove: " + str(e))
	
	## Remove an item of interest from your hated list
	# @param self is mucous
	# @param interest is a text string
	def HatedRemove(self, interest):
		try:
			if not interest.isspace() and interest != "":
				if interest in self.mucous.config["interests.hate"]:
					self.mucous.D.RemoveHatedInterest(interest)
		except Exception,e:
			self.mucous.Help.Log("debug", "InterestHatedRemove: " + str(e))
			
	## Parse input entry line for interest and match it with Recommendations.selected
	# @param self is mucous
	# @param interest is a text string	
	def InputInterests(self, interest):
		try:
			interest = self.mucous.dlang(interest)
			if self.selected == "likes":
				if self.input == "add_likes":
					self.LikedAdd(interest)
				elif self.input == "del_likes":
					self.LikedRemove(interest)
			elif self.selected == "hates":
				if self.input == "add_hates":
					self.HatedAdd(interest)
				elif self.input == "del_hates":
					self.HatedRemove(interest)
				
				
		except Exception,e:
			self.mucous.Help.Log("debug", "InputInterests: " + str(e))


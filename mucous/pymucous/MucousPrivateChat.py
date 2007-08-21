# This is part of the Mucous Museek Client, and distributed under the GPLv2
# Copyright (c) 2006 daelstorm. 
import os
import curses.wrapper
import time
## Private Chat tabs
#			
class PrivateChat:
	## Constructor
	# @param self PrivateChat (class)
	# @param mucous Mucous (class)
	def __init__(self, mucous):
		## @var mucous 
		# Mucous (Class)
		self.mucous = mucous
		## @var current
		# Currently shown user's private chat
		self.current = None
		## @var users 
		# Users whose info we have
		self.users = []
		## @var logs
		# dict of users with lists of old + new chat messages
		self.logs = {}
		## @var windows
		# dict containing instances of curses windows
		self.windows = {}
		## @var dimensions
		# dict containing placement data for windows
		self.dimensions = {}
		## @var scrolling
		# vertical scroll position
		self.scrolling = -1
		self.switchorder = ["default", "chat", "tabs"]
		self.input = "default"
		
	## Create window & tabs and draw them
	# @param self PrivateChat (class)
	def Mode(self):
		self.mucous.mode = "private"
		self.mucous.UseAnotherEntryBox()
		self.mucous.PopupMenu.show = False
		try:
			# Cleanup stale windows
			if "text" in self.windows:
				del self.windows["text"]
				
			w = self.dimensions = {"height": self.mucous.h-8, "width": self.mucous.w, "top": 4, "left": 0}

			tw =  self.windows["text"] = curses.newwin(w["height"], w["width"], w["top"], w["left"])				
			
			tw.scrollok(0)
			tw.idlok(1)
			ibw = self.mucous.windows["inputborder"]
			self.scrolling = -1 
			self.Draw()
			if self.current != None:
				self.mucous.SetEditTitle("Send message to: " + self.current)
				try:
					blah = None
					if "encoding.users" in self.mucous.config:
						if self.current in self.mucous.config["encoding.users"]:
							blah = self.mucous.config["encoding.users"][self.current]
						else:
							blah = self.mucous.config["encoding"]["network"]
					if blah != None:
						ibw.addstr(0, self.mucous.w-17-len(blah)-4, "<" + (" " *( len(blah) +2) )+  ">")
						ibw.addstr(0, self.mucous.w-17-len(blah)-2, blah, self.mucous.colors["cyan"] | curses.A_BOLD)
					ibw.addstr(0, self.mucous.w-10, "< ")
					ibw.addstr(0, self.mucous.w-8, "Close ", self.mucous.colors["cyan"] | curses.A_BOLD)
					ibw.addstr(0, self.mucous.w-2, ">")
				except:
					pass
				ibw.noutrefresh()
				self.mucous.windows["input"].noutrefresh()
				self.mucous.Alerts.Check()
	
			else:
				self.mucous.SetEditTitle("Set a user to Private Message")
				self.mucous.HotKeyBar()
	
			if self.mucous.Alerts.log == "New PM" or self.mucous.Alerts.log[:5] =="PM: ":
				self.mucous.Alerts.setStatus("")
			pmusers = self.logs.keys()
			pmusers.sort(key=str.lower)
			self.mucous.DrawTabs(pmusers, self.current)
			
			
		except Exception, e:
			self.mucous.Help.Log("debug", "PrivateChat.Mode: " +str(e))
		curses.doupdate()
		
	def ClearLog(self):
		if self.current is None:
			return
		self.logs[self.current] = []
		
	## Close a user's chat
	# @param self PrivateChat (class)
	# @param user username
	def Close(self, user):
		try:
			if user in self.logs.keys():
				del self.logs[user]
			if self.logs.keys() != []:
				for users in self.logs.keys():
					self.current = users
					break
			else:
				self.current = None
			if user in self.mucous.Alerts.alert["PRIVATE"]:
				self.mucous.Alert.alert["PRIVATE"].remove(user)
			if self.mucous.mode == 'private':
				self.Mode()
		except Exception, e:
			self.mucous.Help.Log("debug", "PrivateChat.Close: %s" % str(e))
	
	## Draw chat text
	# @param self PrivateChat (class)
	def Draw(self):
		try:
			scrolltext = "private"
			w = self.dimensions
			tw = self.windows["text"]
			tw.erase()
			if self.current == None:
				# Instructions
				
				for lines in self.mucous.Help.log["private"]:
					try:
						lines, ls = self.mucous.FormatData.StringAddBlanks(lines, w)
						#tw.addstr(self.mucous.dlang(lines))
						tw.addstr(self.mucous.dlang(lines))
					except Exception, e:
						self.mucous.Help.Log("debug", "private display: " + str(e))
				tw.noutrefresh()
				return
					
			# Private chat log
			if self.current not in self.logs:
				tw.noutrefresh()
				return

			listcolors = {}
			wrapped_lines = []
			merged_lines = []
			for timestamp, user, message in self.logs[self.current]:
	
				message = message.replace("\t", "      ")
				message = self.mucous.dlang(message)
				if "\\n" in message:
					message = message.split("\\n")
				else: message = [message]
				
				for line in message:
									
					if line is message[0]:
						# first line
						if line[:4] == "/me ":
							color = self.mucous.colors['green']
							merged_lines = "%s * %s %s"% (timestamp, user, line[4:]) 
						elif user == '':
							color = self.mucous.colors['cyan']
							merged_lines = "%s %s"% (timestamp, line)
						else:
							color = curses.A_NORMAL
							merged_lines = "%s [%s] %s"% (timestamp, user, line)
					else:
						# Second and greater lines
						if message[0][:4] == "/me ":
							color = self.mucous.colors['green']
						elif user == '':
							color = self.mucous.colors['cyan']
						else:
							color = curses.A_NORMAL
						merged_lines = "- %s" % line 
					list_of_strings = self.mucous.FormatData.StringCutWidth(merged_lines, w)
					for line in list_of_strings:
						wrapped_lines.append(line)
						listcolors[line] = color
			if len(self.logs[self.current]):
				del merged_lines
				del list_of_strings
			
			if self.scrolling == -1:
				self.scrolling = len(wrapped_lines)	
			
			clipped_list, self.scrolling, w["start"] = self.mucous.FormatData.scrollbox(wrapped_lines, self.scrolling, w["height"])
			
			
			attrs = curses.A_BOLD #| curses.A_UNDERLINE
			attr = curses.A_NORMAL
			count = 0
			for lines in clipped_list:
				color = listcolors[lines]
				try:
					lines, ls = self.mucous.FormatData.StringAddBlanks(lines, w)
					if count + w["start"] == self.scrolling:
						tw.addstr(self.mucous.dlang(lines), attrs | color )
					else:
						tw.addstr(self.mucous.dlang(lines), attr | color )
					count += 1
				except Exception, e:
					#self.mucous.Help.Log("debug", "private display: " + str(e))
					pass
			tw.noutrefresh()
		except Exception, e:
			self.mucous.Help.Log("debug", "PrivateChat.Draw: " + str(e))
			
	## Recieved a Private Message
	# @param self PrivateChat (class)
	# @param direction 0 == incoming; 1 == outgoing
	# @param timestamp (we use our own)
	# @param user username
	# @param message text
	def Recieved(self,direction, timestamp, user, message):
		try:
			message = message.replace("\n", "\\n")
			ctcpversion = 0
			if message == curses.ascii.ctrl("A")+"VERSION"+curses.ascii.ctrl("A"):
				message = "CTCP VERSION"
				ctcpversion = 1
			
			if user not in self.logs.keys():
				self.logs[user] = []
				if self.mucous.Config["mucous"]["logging"] in ("yes"):
					self.ImportLogs(user)
				
			if self.mucous.Config["mucous"]["logging"] in ("yes"):
				if direction == 0:
					self.mucous.FileLog("private", time.strftime("%d %b %Y %H:%M:%S"), user, "["+user+"]\t"+ message )
				elif direction == 1:
					self.mucous.FileLog("private", time.strftime("%d %b %Y %H:%M:%S"), user, "["+self.mucous.username+"]\t"+ message )
			
			self.Log(direction, user, message)

				
			if ctcpversion == 1 and direction == 0:
				if self.mucous.Config["mucous"]["extra_requests"] == "Yes":
					self.Send(user, "Mucous %s" % self.mucous.Version)
					
			if self.current == None:
				self.current = user
				
			if self.mucous.mode != "private":
				self.mucous.Alerts.Add(user, "PRIVATE")
				#self.Alerts.setStatus("New PM")
				#if user not in self.Alerts.alert["PRIVATE"]:
					#self.Alerts.alert["PRIVATE"].append(user)
				self.mucous.HotKeyBar()
				self.mucous.Beep()
				
			elif self.mucous.mode == "private" and self.current != user:
				self.mucous.Alerts.Add(user, "PRIVATE")
				self.mucous.HotKeyBar()
				self.mucous.Beep()
				
			if self.mucous.mode == "private":
				if self.current == user:
					self.Mode()
				elif self.current != user and self.current != None:
					pmusers = self.logs.keys()
					pmusers.sort(key=str.lower)
					self.mucous.DrawTabs(pmusers, self.current)
					#self.mucous.Alerts.setStatus("PM: "+user)
				
			
		except Exception ,e:
			self.mucous.Help.Log("debug", "PrivateChat.Recieved: " + str(e))
			
	## Send Private Message 
	# @param self PrivateChat (class)
	# @param user username
	# @param message text
	def SendEncoded(self, user, message):
		try:
			self.mucous.D.PrivateMessage(1, user, self.mucous.dencode_language(message))
		except:
			pass
		
	## Split messages with newlines before sending message 
	# @param self PrivateChat (class)
	# @param user username
	# @param message string
	def Send(self, user, message):
		try:
			#Username is already utf-8ified
			
			lang = self.mucous.Config["mucous"]["language"]
			if '\\n' in message:
				
				splited =  message.split('\\n')
				if len(splited) > 7:
					for i in range(8):
						self.SendEncoded(user, splited[i])
				else:
					for i in range(len(splited)):
						self.SendEncoded(user,  splited[i])
				

			elif '\n' in message:
				splited =  message.split('\n')
				
				if len(splited) > 5:
					for i in range(5):
						self.SendEncoded(user,  splited[i])
				else:
					for i in range(len(splited)):
						self.SendEncoded(user,  splited[i])
			else:
				self.SendEncoded(user, message)
		
			if message == curses.ascii.ctrl("A")+"VERSION"+curses.ascii.ctrl("A"):
				message = "CTCP VERSION"
			if self.mucous.Config["mucous"]["logging"] in ("yes"):
				self.mucous.FileLog("private", time.strftime("%d %b %Y %H:%M:%S"), user, "["+self.mucous.username+"]\t" +message )
			#pmtype = "outgoing"
			self.Log(1, user, message)
			
			if self.mucous.Alerts.log == "New PM":
				self.mucous.Alerts.setStatus("")
				
			if self.current == None:
				self.current = user
				if self.mucous.mode == "private":
					self.Mode()
			elif self.current == user:
				if self.mucous.mode == "private":
					self.Mode()
			elif self.current != user and self.current != None:
				pmusers = self.logs.keys()
				pmusers.sort(key=str.lower)
				self.mucous.DrawTabs(pmusers, self.current)
				
		except Exception ,e:
			self.mucous.Help.Log("debug", "PrivateChat.Send: " + str(e))
			
			
	## Split messages with newlines before sending message 
	# @param self PrivateChat (class)
	# @param user username
	def Start(self, user):
		try:
			self.current = user
			if user not in self.logs.keys():
				self.logs[user] = []
				if self.mucous.Config["mucous"]["logging"] in ("yes"):
					self.ImportLogs(user)
				
			if self.mucous.mode == 'private':
				self.Mode()
		except Exception, e:
			self.mucous.Help.Log("debug", "PrivateChat.Start: " + str(e))
			
	## Append Private Message to Chat Log
	# @param self PrivateChat (class)
	# @param direction 0 == incoming; 1 == outgoing
	# @param user username
	# @param message text
	def Log(self, direction, user, message):
		try:
			timestamp = time.strftime("%H:%M:%S")
			if user not in self.logs.keys():
				self.logs[user]=['']
	
			if message[:4] == "/me ":
				if direction:
					self.logs[user].append([timestamp, self.mucous.username,  message])
				else:
					self.logs[user].append([timestamp, user, message])
			else:
				
				if direction:
					self.logs[user].append([timestamp, self.mucous.username, message])
				else:
					self.logs[user].append([timestamp, user, message])
					
			
				
		except Exception, e:
			self.mucous.Help.Log( "debug", "PrivateChat.Log: " + str(e))
			
	## Read old Logfiles and import them to the chat log
	# @param self PrivateChat (class)
	# @param username username
	def ImportLogs(self, username):
		try:
			# Read from Private Chat Logs
			if "\\" in username: username = username.replace("/", "\\")
			if os.path.exists(os.path.expanduser(self.mucous.Config["mucous"]["log_dir"])+"/private"+"/"+username):
				path = os.path.expanduser(self.mucous.Config["mucous"]["log_dir"])+"/private"+"/"+username
				f = open(path, "r")
				a = f.read()
				f.close()
				lines = a.split("\n" )
				numlines = -30
				if len(lines) <= abs(numlines):
					numlines = 0
				for line in lines[numlines:]:
					if line == "":
						continue
					timex = line[12:20]
					user = line[22:]
					if line.find("\t") == -1:
						# old format
						user = user[:user.find("]")]
						message = line[21+len(user)+3:]
					else:
						# new format with Tab
						user = user[:user.find("\t")-1]
						message = line[line.find("\t")+1:]
						
					self.logs[username].append([timex, user, message])
				
				self.logs[username].append([time.strftime("%H:%M:%S"), "", "------ Old Chat Above ------"])
		except Exception,e:
			self.mucous.Help.Log("debug", "PrivateChat.ImportLogs: " +str( e) )
	## Mouse Coordinates in the Private Chat Mode
	# @param self is PrivateChat (class)
	# @param x is the horizontal position from the left
	# @param y is the vertical postion from the top
	# @param z is unimportant
	# @param event is the mouse event (button, doubleclick, etc) represented by a number
	def Mouse(self, x,y,z,event):
		try:
			if self.current == None:
				return
			if y in (2, 3, 4):
				if len(self.logs.keys()) > 1:
					pmusers =  self.logs.keys()
					pmusers.sort(key=str.lower)
					self.current, match = self.mucous.edit.MouseClickTab(x, self.current)
					if match == None:
						s = pmusers.index(self.current)
						self.current = pmusers[s-1]
					self.Start(self.current)
					self.Mode()
					
			if y == self.mucous.h-3 or y == self.mucous.h-4:
				if x>= self.mucous.w-27 and x < self.mucous.w-18:
					self.mucous.PopupMenu.Create("encoding", 0, True)
				elif x >=self.mucous.w-10 and x < self.mucous.w-1:
					self.Close(self.current)
		except Exception, e:
			self.mucous.Help.Log("debug", "PrivateChat.Mouse: " +str(e) )

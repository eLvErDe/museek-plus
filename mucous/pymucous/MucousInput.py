# This is part of the Mucous Museek Client, and distributed under the GPLv2
# Copyright (c) 2006 daelstorm.
import sys
import os  
import threading
import curses.wrapper
## Input line 
# :: Here we detect which keys have been pressed
class CharacterParse(threading.Thread):
	## Constructor
	# @param self is CharacterParse
	# @param mucous is the Mucous class
	def __init__(self, mucous):
		
		threading.Thread.__init__(self)
		## @var mucous
		# Mucous (Class)
		self.mucous = mucous
		## @var win
		# Input line curses instance 
		self.win = mucous.windows["input"]
		## @var h
		# height of input window
		## @var w
		# width of input window
		self.h, self.w = self.win.getmaxyx()
		## @var scroll
		# cursor position
		self.scroll = 0
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
		
		self.fixpos()
		
	## Add normal characters to the line, or respond to escaped characters or mouse presses
	# @param self CharacterParse
	# @param c the character recieved by getkey
	# @return (True/False)
	def process(self, c):
		try:
			pos = self.length + self.scroll
			
			# debugging: display keypress
			#self.mucous.Help.Log("debug", c )
			
			# Toggle online ONLY if inactivity timeout was met
			if self.mucous.timedout == True:
				self.mucous.ToggleAwayStatus()
				self.mucous.timedout = False
			if self.mucous.Spl["status"] == 0 and self.mucous.Config["mucous"]["autoaway"] == "yes":
				# Restart inactivity timeout for every key or mousepress if not away, currently
				self.mucous.timeout_timer.cancel()
				self.mucous.timeout_timer = threading.Timer(self.mucous.timeout_time, self.mucous.AwayTimeout)
				self.mucous.timeout_timer.start()
			else:
				self.mucous.timeout_timer.cancel()

	
			if c != chr(9) and c !="KEY_MOUSE":  # Clear self.word if tab wasn't pressed
				self.word = None
				self.firsttab = 0
				self.listline = []
			elif c not in ("KEY_UP", "KEY_DOWN"):
				self.mucous.Spl["history_count"] = -1

			if c == "KEY_MOUSE":
				error = 'mouse'
				if not self.escape:
					line = self.mucous.InputFunctions(c, self.line)
					if line != None:
						self.line = line
						self.length = len(self.line)
			elif c == "KEY_LEFT" or c == chr(2):
				error = 'left'
				if self.escape:
					self.mucous.InputFunctions(c, self.line)
				else:
					self.length -= 1
			
			elif c == "KEY_RIGHT" or c == chr(6):
				error = 'right'
				if self.escape:
					self.mucous.InputFunctions(c, self.line)
				else:
					self.length += 1
			elif c in ("KEY_F(1)", "KEY_F(2)", "KEY_F(3)", "KEY_F(4)", "KEY_F(5)", "KEY_F(6)", "KEY_F(7)", "KEY_F(8)", "KEY_F(9)", "KEY_F(10)"):
				if not self.escape:
					self.mucous.InputFunctions(c, self.line)
				
			elif c in ("KEY_UP", "KEY_DOWN"):
				# Scrolling
				if not self.escape:
					line = self.mucous.InputFunctions(c, self.line)
					if line != None:
						self.line = line
						self.length = len(self.line)
					
				elif self.escape:
					# Alt+Up/Down
					line, self.mucous.Spl["history_count"] = self.mucous.InputHistory(c, self.line, self.mucous.Spl["history_count"])
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
					line = self.mucous.InputFunctions(c, self.line)
					if line != None:
						self.line = line
						self.length = len(self.line)
			elif c == "KEY_IC": # Insert
				self.mucous.InputFunctions(c, self.line)
			
			elif c in ("t", "T", "p", "P", "d", "D", "x", "X"):
				if self.escape:
					if c in ("t", "T"):
						self.mucous.InputFunctions("switch", self.line)
					elif c in ("d", "D"):
						self.mucous.InputFunctions("delete", self.line)
					elif c in ("p", "P"):
						self.mucous.InputFunctions("popup", self.line)
					elif c in ("x", "X"):
						self.mucous.InputFunctions("collapse", self.line)
				else:
					self.line = self.line[:pos] + c + self.line[pos:]
					self.length += 1
			elif c == chr(1): 
				# Ctrl-A
				self.length = self.scroll = 0
			elif c == chr(5):
				# Ctrl-E
				self.length = len(self.line)
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
					self.mucous.InputFunctions(c, self.line)
					return True
			
			
			elif c == chr(9): 
				# Tab
				if self.word == None:
					#self.length
					#w = self.line.split(' ')
					#self.word = w[-1]
					w = self.line[:self.length].split(' ')
					
					self.word = w[-1]
					self.listline = self.line.split(" ")
				w = self.line[:self.length].split(' ')
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
							
				self.listline, self.firsttab, self.word, xpos = self.mucous.InputTabCompletion(self.line, self.word, self.firsttab, self.listline, cpos)
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
							self.length = len(self.line)-1
					elif posx == ll:
						self.line +=r
						# Place cursor at end of line
						if posx == xpos:
							self.length = len(self.line)
					posx += 1
				
				#self.length = len(self.line)
				#return False
			elif c == chr(11): 
				# Ctrl-K
				# Delete everything after cursor position
				self.line = self.line[:pos]
				self.length = len(self.line)
				self.scroll = 0
			elif c == chr(21): 
				# Ctrl-U
				# Delete line
				self.line = ''
				self.length = 0
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
				self.length = len(self.line)
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
					self.mucous.InputFunctions(c, self.line)
				else:
					self.line = self.line[:pos] + c + self.line[pos:]
					self.length += 1
			elif c == "KEY_DC"  or c == chr(4):
				# Delete
				# Delete letter after cursor
				self.line = self.line[:pos] + self.line[pos+1:]
			elif c == "KEY_BACKSPACE" or c == chr(8) or c == chr(127):
				# Backspace, Ctrl-H
				# Delete letter before cursor
				if pos > 0:
					self.line = self.line[:pos-1] + self.line[pos:]
					self.length -= 1
	
			elif len(c) == 1 and ord(c[0]) >= 32 and ord(c[0]) < 127:
				# ASCII letters 
				self.line = self.line[:pos] + c + self.line[pos:]
				self.length += 1
			elif len(c) == 1 and ord(c[0]) > 127 and ord(c[0]) < 256:
				# ISO8859-* characters
				self.line = self.line[:pos] + c + self.line[pos:]
				self.length += 1
	
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
			while self.scroll + self.length > len(self.line):
				self.length -= 1
				
			while self.length >= self.w:
				self.scroll += 1
				
				self.length -= 1
			if self.length < 0:
				self.scroll += self.length
				
				self.length = 0
	
			if self.scroll < 0:
				self.scroll = 0
	
			self.win.erase()
	
			try:
				self.win.addstr(self.line[self.scroll:self.scroll+self.w-1])
			except Exception, e:
				self.mucous.Help.Log("debug", "Editwin: "+ str(e))
			self.win.refresh()
		except Exception, e:
			self.mucous.Help.Log("debug", "fixpos: \""+str(self.line)+"\" "+ str(e))
			
	## Delete contents of line
	# @param self CharacterParse
	def reset(self):
		try:
			self.length = self.scroll = 0
			self.mucous.line = self.line = ""
	
			self.win.erase()
			self.win.refresh()
		except Exception, e:
			self.mucous.Help.Log("debug", "reset: \""+str(self.line)+"\" "+ str(e))

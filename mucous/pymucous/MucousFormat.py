# This is part of the Mucous Museek Client, and distributed under the GPLv2
# Copyright (c) 2006 daelstorm. 
from UserDict import UserDict
import curses.wrapper, curses.ascii

		


## Dictionary that's sorted alphabetically
# @param UserDict dictionary to be alphabetized	
class SortedDict(UserDict):
	## Constructor
	# @param self SortedDict
	def __init__(self):
		self.__keys__ = []
		self.__sorted__ = True
		UserDict.__init__(self)
		
	## Set key
	# @param self SortedDict
	# @param key dict key
	# @param value dict value
	def __setitem__(self, key, value):
		if not self.__dict__.has_key(key):
			self.__keys__.append(key) 
			self.__sorted__ = False   
		UserDict.__setitem__(self, key, value)
	## Delete key
	# @param self SortedDict
	# @param key dict key
	def __delitem__(self, key):
		self.__keys__.remove(key)
		UserDict.__delitem__(self, key)
	## Get keys
	# @param self SortedDict
	# @return __keys__ 
	def keys(self):
		if not self.__sorted__:
			self.__keys__.sort()
			self.__sorted__ = True
		return self.__keys__
	## Get items
	# @param self SortedDict
	# @return list of keys and items
	def items(self):
		if not self.__sorted__:
			self.__keys__.sort()
			self.__sorted__ = True
		for key in self.__keys__:
			yield key, self[key]

## Parse data and reformat it to fit inside the constraints of its window
#
class FormatData:
	## Constructor
	# @param self FormatData (class)
	# @param mucous Mucous (Class)
	def __init__(self, mucous):
		## @var mucous 
		# Mucous (Class)
		self.mucous = mucous

	## Determine Line number and list size to height
	# We assume that this list has already been wrapped
	# @param self FormatData (class)
	# @param current_list list to be shrunk
	# @param hightlight_position where our vertical scroll position is
	# @param height how many lines are in the window
	# @return list, scrolling position, start position
	def scrollbox(self, current_list, hightlight_position, height):
		# Limit text to what fits inside the window
		try:
			length = len(current_list)
			if hightlight_position < 0:
				hightlight_position = 0
			elif hightlight_position > length -1:
				hightlight_position = length-1
			
			start = hightlight_position-height/2
			
			if start  < 0:
				to = hightlight_position + (height-height/2)-start
				start  = 0
				
			else:
				to = hightlight_position + (height-height/2)
				
			if to >= length:
				start = abs(to - length - start)
				to =length
				
			if length < height:
				start =0
			#self.Help.Log("debug", "s%se%su%s"%(start,to,hightlight_position) )
			if current_list[start:to] != [None]:
				clipped_list = current_list[start:to]
			else:
				clipped_list = []
			return clipped_list, hightlight_position, start
		except Exception, e:
			self.mucous.Help.Log("debug", "scrollbox: " + str(e))
			
	## Convert a number into a human-readable string
	# Formats are: 0GB, 0MB, 0KB, 0B
	# @param self FormatData
	# @param filesize can be integer in a string or an integer
	# @return formatted size (string)
	def byte_format(self, filesize):
		try:
			filesize =  int(filesize)
			if filesize >= 1073741824:
				filefrmt = str(filesize/1024/1024/1024) +"GB"
			elif filesize >= 1048576 and filesize <= 1073741824:
				filefrmt = str(filesize/1024/1024) +"MB"
			elif filesize < 1048576 and filesize >= 1024:
				filefrmt = str(filesize/1024) +"KB"
			elif filesize < 1024 and filesize > 0:
				filefrmt = str(filesize) +" B"	
			elif filesize == 0:
				filefrmt = '0'
			return filefrmt
		except Exception, e:
			self.mucous.Help.Log("debug", "byte_format: " + str(e))
        
	## Another int-to-string Conversion
	# Formats are: 0.0GB, 0.0MB, 0.0KB, 0 
	# @param self FormatData
	# @param size filesize can be integer in a string or an integer
	# @return formatted size (string)
	def Humanize(self, size):
		if size is None:
			return None
		try:
			s = int(size)
			if s >= 1000*1024*1024:
				r = "%.1fGB" % ((float(s) / (1024.0*1024.0*1024.0)))
			elif s >= 1000*1024:
				r = "%.1fMB" % ((float(s) / (1024.0*1024.0)))
			elif s >= 1000:
				r = "%.1fKB" % ((float(s) / 1024.0))
			else:
				r = str(size)
			return r
		except Exception, e:
			return size
			
	## Sort a dict by values
	# Return the sorted dict
	# @param self FormatData (class)
	# @param dict the dict
	def sortbyvalue(self, dict):
		try:
			""" Return a list of (key, value) pairs, sorted by value. """
			_swap2 = lambda (x,y): (y,x)
			mdict = map(_swap2, dict.items())
			mdict.sort()
			mdict = map(_swap2, mdict)
			return mdict
		except Exception, e:
			self.mucous.Help.Log("debug", "sortbyvalue: " + str(e))
			
	## Pad string to fill width of window
	# @param self FormatData (class) 
	# @param s string
	# @param w window_dimensions dict
	# @return padded string, length of string
	def StringAddBlanks(self, s, w):
		try:
			#, total_lines
			
			s = str(s); ls = len(s)
			if ls > w["width"]:
				# Add spaces if longer than a single line
				div = (ls/w["width"]) + 1
				length = (w["width"] * div) -  ls 
				if length != 0:
					s += (length * " ")
				#total_lines += div
	
			else:
				# Add spaces till end of first and only line
				s += " " * (w["width"] - ls)
			return s, ls #, total_lines
		except Exception, e:
			self.mucous.Help.Log("debug", "StringAddBlanks: " + str(e))
			
	## Select an item in the list one place away
	# @param self FormatData (class)
	# @param direction (left or right)
	# @param _list the list
	# @param place current position in list
	# @param sort do we alphabetically sort this list? (true/false)
	# @return new or old place 
	def RotateList(self, direction, _list, place, sort):
		try:
			if not _list:
				return place
			if sort == "yes":
				_list.sort(key=str.lower)
			if not place in _list:
				if direction == "left":
					ix = -1
				elif direction == "right":
					ix = 0
				

			else:
				ix = _list.index(place)
				if direction == "left":
					ix -= 1
				elif direction == "right":
					ix += 1
				
				if ix < 0:
					ix = -1
				elif ix >= len(_list):
					ix = 0
			if ix != None:
				place = _list[ix]
			return place
		except Exception, e:
			self.mucous.Help.Log("debug", "RotateList: " +str(e) )
					
	## Break a long string into a list of strings that each fit inside the required width
	# @param self FormatData (class)
	# @param string our long string
	# @param w the window dimensions dict contains "width"
	# @return list
	def StringCutWidth(self, string, w):
		try:
			s = str(string) 
			ls = len(s)
			list_of_strings = []
			if ls > w["width"]:
				div = (ls/w["width"]) + 1
				# Cut long lines into multiple lines
				for seq in range(div):
					list_of_strings.append(s[:w["width"]])
					s = s[w["width"]:]
			else:
				# Short line added to list
				list_of_strings.append(s)
			return list_of_strings
		except Exception, e:
			self.mucous.Help.Log("debug", ": " + str(e))

	## Break a list apart and wrap each line in it to the required width and clip it so only the lines that fit are 
	# @param self FormatData (class)
	# @param the_list list of strings
	# @param scroll scrolling position
	# @param w the window dimensions dict contains "width" and "start"
	# @return clipped_list, numlines, start
	# @return list, scroll position, start position
	def wrap_n_clip(self, the_list, scroll, w):
		try:
			wrapped_lines = []
			for lines in the_list:
				#lines = str(lines)
				lines1 = ""
				for a in lines:
					if curses.ascii.isctrl(a):
						a = curses.ascii.unctrl(a)
					lines1 += a
				list_of_strings = self.StringCutWidth(lines1, w)
				for string in list_of_strings:
					wrapped_lines.append(string)
			if scroll == -1 or scroll > len(wrapped_lines):
				scroll = len(wrapped_lines)
			clipped_list, numlines, w["start"] = self.scrollbox(wrapped_lines, scroll, w["height"])
			return clipped_list, scroll, w["start"]
		except Exception, e:
			self.mucous.Help.Log("debug", "wrap_n_clip " +str(e))
			
	## Add a number of characters to the current cursor position of a window
	# @param self FormatData (class)
	# @param window the curses window
	# @param character the special character
	# @param number how many times to display the character
	def Hline(self, window, character, number):
		while number:
			window.addch(character)
			number -= 1

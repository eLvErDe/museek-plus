# This is part of the Mucous Museek Client, and distributed under the GPLv2
# Copyright (c) 2006 daelstorm. 
import os

import curses.wrapper
## Transfers (Uploads and Downloads)
#
class Transfers:
	## Constructor
	# @param self Transfers (class)
	# @param mucous Mucous (class)
	def __init__(self, mucous):
		## @var mucous 
		# Mucous (Class)
		self.mucous = mucous
		## @var logs
		# dict containing upload and downloads counters
		self.logs = {"uploads": "Up: 0", "downloads": "Down: 0", }
		## @var windows
		# dict containing instances of curses windows
		self.windows = {"text": {}, "border": {}}
		## @var dimensions
		# dict containing placement data for windows
		self.dimensions = {}
		## @var scrolling
		# dict of uploads and downloads contains the vertical scroll position
		self.scrolling = {"uploads": 0, "downloads": 0}
		## @var sort
		# Sorting method
		self.sort = "all"
		## @var sorted_transfer_list
		# Alphabetically Sorted transfer list, for setting / getting transfer numbers 
		self.sorted_transfer_list = {}
		## @var current
		# Current transfer window
		self.current = "downloads"
		## @var speed
		# Show speed if True, Percent if False
		self.speed = True
		## @var transfers
		# dict of downloads and uploads (ALL transfers) 
		self.transfers = {"downloads": {}, "uploads": {} }
		## @var uploads
		# Status-Limited Uploads
		self.uploads = {}
		## @var downloads
		# Status-Limited Downloads
		self.downloads = {}
		## @var modes
		# the sorting modes for Transfers.sort
		self.modes = ["all","active",  "queued", "finished", "failed"]
		## @var states
		# String associated with transfer states
		self.states = {0: "Finished", 1: "Xferring", 2: "Negotiating", 3:"Waiting", 4: "Establishing", 5: "Initiating", 6: "Connecting",  7: "Queued", 8:"Address", 9:  "Status", 10: "Offline",11: "Closed",12: "Can't Connect", 13: "Aborted",14: "Not Shared"}
		## @var username
		# Upload to this user
		self.username = None
		## @var downloaddirectory
		# directory DownloadTo transfers get downloaded into
		self.downloaddirectory = os.path.expanduser("~/")

	## Create Windows, draw borders :: Call: Transfers.TransferBar and Transfers.UploadManager and Transfers.DownloadManager
	# @param self Transfers (class)
	def ModeTransfers(self):
		self.mucous.mode = "transfer"
		self.mucous.UseAnotherEntryBox()
		self.mucous.PopupMenu.show = False

		try:
			# Cleanup stale windows
			if "uploads" in self.windows["text"]:
				del self.windows["text"]["uploads"]
			if "uploads" in self.windows["border"]:
				del self.windows["border"]["uploads"]
			if "downloads" in self.windows["text"]:
				del self.windows["text"]["downloads"]
			if "downloads" in self.windows["border"]:
				del self.windows["border"]["downloads"]
				
			if self.mucous.Config["mucous"]["transbox"] == "split":
				
					
				u = self.dimensions["uploads"] = {"height": self.mucous.h/2-4, "width": self.mucous.w-2, "top": 2, "left": 1}
				d = self.dimensions["downloads"] = {"height": self.mucous.h-5-u["height"]-4, "width": self.mucous.w-2, "top": self.mucous.h/2, "left": 1}
				self.DrawTransfersLists("uploads")
				self.DrawTransfersLists("downloads")
				
				# Draw download and upload windows
				uw = self.windows["border"]["uploads"]
				uw.noutrefresh()
				dw = self.windows["border"]["downloads"]
				dw.noutrefresh()
				
				utw = self.windows["text"]["uploads"] = uw.subwin( u["height"], u["width"], u["top"], u["left"])
				utw.scrollok(0)
				utw.idlok(1)
				utw.noutrefresh()
				
				dtw = self.windows["text"]["downloads"] = dw.subwin(d["height"],d["width"], d["top"],d["left"])
				dtw.scrollok(0)
				dtw.idlok(1)
				dtw.noutrefresh()
				
			elif self.mucous.Config["mucous"]["transbox"] == "tabbed":
				if self.current == "uploads":
					# Draw upload window
					u = self.dimensions["uploads"] = {"height": self.mucous.h-7, "width": self.mucous.w-2, "top": 2, "left": 1}
					self.DrawTransfersLists("uploads")
					
					uw = self.windows["border"]["uploads"]
					uw.refresh()
					utw = self.windows["text"]["uploads"] = uw.subwin( u["height"], u["width"], u["top"], u["left"])
					utw.scrollok(0)
					utw.idlok(1)
					utw.noutrefresh()
				else:
					# Draw download window
					d = self.dimensions["downloads"] = {"height": self.mucous.h-7, "width": self.mucous.w-2, "top": 2, "left": 1}
					self.DrawTransfersLists("downloads")

					dw = self.windows["border"]["downloads"]
					dw.refresh()
					# Draw download window
					dtw = self.windows["text"]["downloads"] = dw.subwin(d["height"],d["width"], d["top"],d["left"])
					dtw.scrollok(0)
					dtw.idlok(1)
					dtw.noutrefresh()
			
		except Exception, e:
			self.mucous.Help.Log("debug", "transfer mode: " + str(e))
			
		try:
			self.TransferBar()
						
			self.mucous.SetEditTitle("Modify Transfers")
			if self.mucous.Config["mucous"]["transbox"] == "split":
				self.UploadManager()
				self.DownloadManager()
				#curses.doupdate()
			else:
				if self.current == "uploads":
					self.UploadManager()
					#curses.doupdate()
				if self.current == "downloads":
					self.DownloadManager()
					#curses.doupdate()
			self.Status()
		except Exception, e:
			self.mucous.Help.Log("debug", "transfer panel: " + str(e))


		self.mucous.HotKeyBar()
		curses.doupdate()

		
	## Update Transfer Counters
	# @param self Transfers (class)
	def Status(self):
		if "uploadstatus" in self.windows["border"]:
			del self.windows["border"]["uploadstatus"]
		if "downloadstatus" in self.windows["border"]:
			del self.windows["border"]["downloadstatus"]
		## Upload status window
		usw = 0
		usw = self.windows["border"]["uploadstatus"]  = curses.newwin(1, 10, 0, 25)
		usw.bkgdset(" ", self.mucous.colors["blackwhite"]  | curses.A_REVERSE | curses.A_BOLD)
		usw.idlok(1)
		try:
			usw.erase()
			usw.addstr(self.logs["uploads"],  self.mucous.colors["blackwhite"] )
		except:
			pass
		usw.noutrefresh()
		
		dsw = self.windows["border"]["downloadstatus"] = curses.newwin(1, 12, 0, 35)
		dsw.bkgdset(" ", self.mucous.colors["blackwhite"]  | curses.A_REVERSE | curses.A_BOLD)
		dsw.idlok(1)
		try:
			dsw.erase()
			dsw.addstr(self.logs["downloads"],  self.mucous.colors["blackwhite"] )
		except:
			pass
		dsw.noutrefresh()
	## Get Upload user and path from number
	# @param self Transfers
	# @param num number
	# @return username, path 
	def GetUploadFromNum(self, num):
		if num == None:
			return None, None
		if num in self.uploads.keys():
			for username,path in self.uploads[num].items():
				return username, path
		else:
			self.mucous.Help.Log("status", "No such upload #" + str(num))
			return None, None
	## Get Download user and path from number
	# @param self Transfers
	# @param num number
	# @return username, path
	def GetDownloadFromNum(self, num):
		if num == None:
			return None, None
		if num in self.downloads.keys():
			for username,path in self.downloads[num].items():
				return username, path
		else:
			self.mucous.Help.Log("status", "No such download #" + str(num))
			return None, None
		
	## Download a file to the Transfers.downloaddirectory
	# @param self Transfers
	# @param username Username
	# @param path File path 
	def DownloadFileTo(self, username, path):
		if username == None or path == None:
			return
		p = path.split("\\")
		self.mucous.D.DownloadFileTo(user, path, self.downloaddirectory+"/"+p[-1])
		self.mucous.Help.Log("status", "Trying to Download: " + path+" from "+ user+"to "+self.downloaddirectory+"/"+p[-1])
		
	## Download a folder
	# @param self Transfers
	# @param username Username
	# @param path Folder path (filename is removed)
	def FolderDownload(self,username, path):
		if username == None or path == None:
			return
		path = path.replace("/", "\\")
		r = path.split('\\')
		r.remove(r[-1])
		directory = ''
		for s in r:
			if s is not r[-1]:
				directory += s + "\\"
			else:
				directory += s
		directory = directory.replace("/", "\\")
		self.mucous.D.GetFolderContents(username, directory)
		self.mucous.Help.Log("status", "Try to Download directory: %s from %s" % (directory, username))
		
	## Abort a Download
	# @param self Transfers
	# @param username Username
	# @param path File path
	def AbortDownload(self, username, path):
		if username == None or path == None:
			return
		self.mucous.D.TransferAbort(0, username, path)
		self.mucous.Help.Log("status", "Aborting download: [%s] %s" % (username, path))
		
	## Abort an Upload
	# @param self Transfers
	# @param username Username
	# @param path File path
	def AbortUpload(self, username, path):
		if username == None or path == None:
			return
		self.mucous.D.TransferAbort(1, username, path)
		self.mucous.Help.Log("status", "Aborting upload: [%s] %s" % (username, path))
		
	## Clear all failed or finished uploads from the transfer manager
	# @param self Transfers
	def ClearAllUploads(self):
		for userpath, values in self.transfers["uploads"].items():
			if values[3] in (0, 10, 11, 12, 13, 14):
				self.mucous.D.TransferRemove(1, values[1], values[2])
				
	## Clear all of a user's finished downloads from the transfer manager
	# @param self Transfers
	# @param user Username
	def ClearUserDownloads(self, user):
		for userpath, values in self.transfers["downloads"].items():
			if values[3] == 0:
				self.mucous.D.TransferRemove(0, values[1], values[2])
	## Clear all of a user's uploads
	# @param self Transfers
	# @param user Username
	def ClearUserUploads(self, user):
		for userpath, values in self.transfers["uploads"].items():
			if values[3] == 0:
				self.mucous.D.TransferRemove(0, values[1], values[2])
	## Remove all finished downloads
	# @param self Transfers
	def ClearAllDownloads(self):
		for userpath, values in self.transfers["downloads"].items():
			if values[3] == 0:
				self.mucous.D.TransferRemove(0, values[1], values[2])
				
	## Retry all failed downloads
	# @param self Transfers (Class)
	def RetryAllDownloads(self):
		for user_path, transfer in self.transfers["downloads"].items():
			if int(transfer[3]) in (10, 11, 12, 13, 14):
				self.mucous.D.DownloadFile(transfer[1], transfer[2])
	## Retry a Download
	# @param self Transfers
	# @param username Username
	# @param path File path
	def RetryDownload(self, username, path):
		if username == None or path == None:
			return
		path = path.replace("/", "\\")
		self.mucous.D.DownloadFile(username, path)
		self.mucous.Help.Log("status", "Downloading: [%s] %s" % (username, path))
	## Retry a Upload
	# @param self Transfers
	# @param username Username
	# @param path File path
	def RetryUpload(self, username, path):
		if username == None or path == None:
			return
		path = path.replace("/", "\\")
		self.mucous.D.UploadFile(username, path)
		self.mucous.Help.Log("status", "Uploading: [%s] %s" % (username, path))
	## Clear a Download
	# @param self Transfers
	# @param username Username
	# @param path File path
	def ClearDownload(self, username, path):
		if username == None or path == None:
			return
		self.mucous.D.TransferRemove(0, username, path)
		self.mucous.Help.Log("status", "Removing download: [%s] %s" % (username, path))
	## Retry an Upload
	# @param self Transfers
	# @param username Username
	# @param path File path
	def ClearUpload(self, username, path):
		if username == None or path == None:
			return
		self.mucous.D.TransferRemove(1, username, path)
		self.mucous.Help.Log("status", "Removing upload: [%s] %s" % (username, path))
		
	## Draw the Upload counter with s as the value
	# @param self Transfers
	# @param s string
	def DrawUploadCount(self, s):
		try:
			self.logs["uploads"] = "Up: %s" %str(s)
			usw = self.windows["border"]["uploadstatus"]
			try:
				usw.erase()
				usw.addstr(self.logs["uploads"], self.mucous.colors["blackwhite"] )
				usw.refresh()
			except Exception, e:
				self.mucous.Help.Log( "debug", "Upload Status: " + str(e))
		except Exception, e:
			self.mucous.Help.Log("debug", "DrawUploadCount: " + str(e))
	## Draw the Download counter with s as the value
	# @param self Transfers
	# @param s string
	def DrawDownloadCount(self, s):
		try:
			self.logs["downloads"] = "Down: %s" %str(s)
			dsw = self.windows["border"]["downloadstatus"]
			try:
				dsw.erase()
				dsw.addstr(self.logs["downloads"], self.mucous.colors["blackwhite"] )
				dsw.refresh()
			except Exception, e:
				self.mucous.Help.Log( "debug", "Download Status: " + str(e))
		except Exception, e:
			self.mucous.Help.Log("debug", "DrawDownloadCount: " + str(e))
	
	## Wrap a green string with < and >
	# @param self Transfers (Class)
	# @param window Curses window instance 
	# @param height vertical position
	# @param pos horizontal position
	# @param string string to be wrapped
	def TArrows(self, window, height, pos, string):
		try:
			window.addstr(height, pos, "< ")
			window.addstr(height, pos+2, string.capitalize(), self.mucous.colors["green"] | curses.A_BOLD)
			window.addstr(height, pos+2+len(string), " >")
		except Exception, e:
			self.mucous.Help.Log("debug", "TArrows: " + str(e))
			
	## Limit list to Transfers.sort method and window height :: Call Transfers.TransferItem
	# @param self Transfers (Class)
	# @param list the transfers list
	# @return list
	def ListCompact(self, list):
		try:
			if self.sort == 'active':
				status_list = (1, 2, 3, 4, 5, 6, 8, 9,)
			elif self.sort == 'all':
				status_list = (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14)
			elif self.sort == 'finished':
				status_list = (0,)
			elif self.sort == 'queued':
				status_list = (7,)
			elif self.sort == 'failed':
				status_list = ( 10, 11, 12, 13)
			if list == "downloads":
				self.downloads = {}
			elif list == "uploads":
				self.uploads = {}
			statuslist = []
			statuslist = self.transfers[list].keys()
			statuslist.sort()
			
			self.sorted_transfer_list[list] = statuslist
			num = 0
			filterlist = []
			for user_path in statuslist:
				valid = 1
				num += 1
				transfer, user, path, status, error, filepos, filesize, rate, place = self.transfers[list][user_path]
				username = {}
				username[user] = path
				if list == "downloads":
					self.downloads[num] = username
				else:
					self.uploads[num] = username
				if status not in status_list:
					valid = 0
				if valid:
					filterlist.append([user_path, num])
				
			del statuslist

			clipped_list, self.scrolling[list], self.dimensions[list]["start"] = self.mucous.FormatData.scrollbox(filterlist, self.scrolling[list], self.dimensions[list]["height"]-1)
			del filterlist
			finallist = []
			for user_path, number in clipped_list:
				item = self.TransferItem(list, user_path, number)
				finallist.append(item)
			del clipped_list
			return finallist
		except Exception, e:
			self.mucous.Help.Log("debug", "ListCompact: " + str(e))
			
	## Draw Transfer bar on selected window
	# @param self Transfers (Class)
	def TransferBar(self):
		try:
			s = self.dimensions[self.current ]
			window = self.windows["border"][ self.current ]
			pos = 2
			 
			self.TArrows(window, s["height"]+1, pos, self.mucous.Config["mucous"]["transbox"])
			
			if self.sort == 'all':
				
				pos = 20
				self.TArrows(window, s["height"]+1, pos, self.sort)
			else:
				window.addstr(s["height"]+1, 20, "< All >")

			if self.sort == 'active':
				pos = 28
				self.TArrows(window, s["height"]+1, pos, self.sort)
			else:
				window.addstr(s["height"]+1, 28, "< "+'Active'+" >")

			if self.sort == 'queued':
				pos = 39
				self.TArrows(window, s["height"]+1, pos, self.sort)
			else:
				window.addstr(s["height"]+1, 39, "< "+'Queued'+" >")
			if self.sort == 'finished':
				pos = 50
				self.TArrows(window, s["height"]+1, pos, self.sort)
			else:
				window.addstr(s["height"]+1, 50, "< "+'Finished'+" >")
			if self.sort == 'failed':
				pos = 63
				self.TArrows(window, s["height"]+1, pos, self.sort)
			else:
				window.addstr(s["height"]+1, 63, "< "+'Failed'+" >")
			window.refresh()
		except Exception, e:
			self.mucous.Help.Log("debug", "TransferBar: " + str(e))
			
	## Draw Transfer List Window Border 
	# @param self Transfers (Class)
	# @param window window name (uploads, downloads)
	def DrawTransfersLists(self, window):
		try:
			if self.mucous.mode != "transfer":
				return
			# Cleanup stale windows
			if window in self.windows["border"]:
				del self.windows["border"][window]
			s = self.dimensions[window]
			win = self.windows["border"][window] = curses.newwin(s["height"]+2, s["width"]+2, s["top"]-1, s["left"]-1)
			
			if self.mucous.Config["mucous"]["transbox"] == "tabbed":
				win.attron(self.mucous.colors["green"])
				win.border()
				if window == "uploads":
					uattr = self.mucous.colors["green"] | curses.A_BOLD
					uuattr = self.mucous.colors["green"] | curses.A_BOLD | curses.A_UNDERLINE
					duattr = self.mucous.colors["green"] | curses.A_UNDERLINE
					dattr = self.mucous.colors["green"]
						
					
				elif window == "downloads":
					uattr = self.mucous.colors["green"] 
					uuattr = self.mucous.colors["green"] | curses.A_UNDERLINE
					duattr = self.mucous.colors["green"] | curses.A_BOLD  | curses.A_UNDERLINE
					dattr = self.mucous.colors["green"] | curses.A_BOLD
					
				pos = 3
				win.addstr(0, pos, "<", dattr)
				pos += 1
				u = " Uploading Transfers "
				win.addstr(0, pos, u,  uuattr)
				pos += len(u)
				win.addstr(0, pos, ">", dattr)
				pos += 2
				win.addstr(0, pos, "<", dattr)
				pos += 1
				d = " Downloading Transfers "
				win.addstr(0, pos, d,  duattr)
				pos += len(d)
				win.addstr(0, pos, ">", dattr)
				
			else:
				if self.current == "uploads" and window == "uploads":
					win.attron(self.mucous.colors["green"])
					win.border()
					win.addstr(0, 3, "< Uploading Transfers >", curses.A_BOLD|self.mucous.colors["green"])
				elif self.current == "uploads" and window == "downloads":
					win.border()
					win.addstr(0, 3, "< Downloading Transfers >", curses.A_BOLD)
					
				elif self.current == "downloads" and window == "downloads":
					win.attron(self.mucous.colors["green"])
					win.border()
					win.addstr(0, 3, "< Downloading Transfers >", curses.A_BOLD|self.mucous.colors["green"])
				elif self.current == "downloads" and window == "uploads":
					win.border()
					win.addstr(0, 3, "< Uploading Transfers >", curses.A_BOLD)
				win.addch(0, 29, curses.ACS_TTEE )
				win.addch(0, 29, curses.ACS_TTEE )
				win.addch(0, 40, curses.ACS_TTEE )
				if window == "downloads":
					win.addch(0, 45, curses.ACS_TTEE )
				
	
	
			self.scrolling[self.current] = 0
		except Exception, e:
			self.mucous.Help.Log("debug", "DrawTransfersLists: " + str(e))
			
	## Draw Download List Contents 
	# @param self Transfers (Class)
	def DownloadManager(self):
		try:
			if self.mucous.mode != "transfer":
				return
			if self.current != "downloads" and self.mucous.Config["mucous"]["transbox"] != "split":
				return
			
			dtw = self.windows["text"]["downloads"]
			w = self.dimensions["downloads"]

			finallist = self.ListCompact("downloads")
			s = self.scrolling["downloads"]
			start = self.dimensions["downloads"]["start"]

			try:
				if self.speed == True:
					swting = "Numb","Speed"," Size "," Username ","  Status  "," Que","  Path"
				else:
					swting = "Numb","Perc "," Size "," Username ","  Status  "," Que","  Path"
				
				dtw.erase()
				length = 0
				if self.current == "downloads":
					hattr = self.mucous.colors["green"] | curses.A_REVERSE
					hattr1 = self.mucous.colors["green"]
				else:
					hattr = self.mucous.colors["blackwhite"]
					hattr1 = curses.A_NORMAL  
				for i in swting:
					length += len(i)
					dtw.addstr(i, hattr)
					if i is not swting[-1]:
						dtw.addch(curses.ACS_VLINE, hattr1)
						length += 1
				filler = ((self.mucous.w-2 - length) * " ")
				dtw.addstr(filler, hattr)

				
			except:
				pass
			try:
				
				if finallist == []:
					dtw.addstr("\nNo transfers in this category, hit INSERT to switch to another.")
					dtw.noutrefresh()
					return
				count = 0
				for transfers in finallist:
					if transfers == None:
						continue
					
					if transfers[1] == 1:
						sattr = self.mucous.colors["green"] | curses.A_BOLD
					elif transfers[1] == 0:
						sattr = self.mucous.colors["magenta"] | curses.A_BOLD
					elif transfers[1] in (2, 3, 4, 5, 6, 8, 9):
						sattr = self.mucous.colors["yellow"]| curses.A_BOLD
					elif transfers[1] in (10, 11, 12, 13, 14):
						sattr = self.mucous.colors["red"] | curses.A_BOLD
					else:
						sattr = curses.color_pair(0) | curses.A_BOLD
					attr = curses.A_NORMAL
					attr1 = curses.A_NORMAL
					if count + start == s:
						attr = curses.A_REVERSE | curses.A_BOLD
						attr1 = curses.A_NORMAL | curses.A_BOLD
						sattr = curses.A_REVERSE | curses.A_BOLD | sattr

					try:
						dtw.addstr(self.mucous.encode_language(transfers[0])[:4], attr)
						dtw.addch(curses.ACS_VLINE, attr1)
						dtw.addstr(self.mucous.encode_language(transfers[0])[5:10], attr)
						dtw.addch(curses.ACS_VLINE, attr1)
						dtw.addstr(self.mucous.encode_language(transfers[0])[11:17], attr)
						dtw.addch(curses.ACS_VLINE, attr1)
						dtw.addstr(self.mucous.encode_language(transfers[0])[18:28], attr)
						dtw.addch(curses.ACS_VLINE, attr1)
						dtw.addstr(self.mucous.encode_language(transfers[0])[29:39], sattr)
						dtw.addch(curses.ACS_VLINE, attr1)
						dtw.addstr(self.mucous.encode_language(transfers[0])[40:44], attr)
						dtw.addch(curses.ACS_VLINE, attr1)
						dtw.addstr(self.mucous.encode_language(transfers[0])[45:w["width"]], attr)
						
					except :
						pass
					count += 1
			except Exception, e:
				self.mucous.Help.Log("debug", "Download log: " + str(e))
			dtw.noutrefresh()

		except Exception, e:
			self.mucous.Help.Log("debug", "Download Manager: " + str(e))
	## Draw Upload List Contents 
	# @param self Transfers (Class)
	def UploadManager(self):
		try: 
			if self.mucous.mode != "transfer":
				return
			if self.current != "uploads" and self.mucous.Config["mucous"]["transbox"] != "split":
				return

			utw = self.windows["text"]["uploads"]
			w = self.dimensions["uploads"]
			
			finallist = self.ListCompact("uploads")
			s = self.scrolling["uploads"]
			start = self.dimensions["uploads"]["start"]
			try:
				if self.speed == True:
					swting = "Numb","Speed"," Size "," Username ","  Status  ","  Path"
				else:
					swting = "Numb","Perc "," Size "," Username ","  Status  ","  Path"
				utw.erase()
				length = 0
				if self.current == "uploads":
					hattr = self.mucous.colors["green"] | curses.A_REVERSE
					hattr1 = self.mucous.colors["green"]
				else:
					hattr = self.mucous.colors["blackwhite"]
					hattr1 = curses.A_NORMAL  
				for i in swting:
					length += len(i)
					utw.addstr(i, hattr)
					if i is not swting[-1]:
						utw.addch(curses.ACS_VLINE, hattr1)
						length += 1
				filler = ((self.mucous.w-2 - length) * " ")
				utw.addstr(filler, hattr)
			except:
				pass
				
			try:
				if finallist == []:
					utw.addstr("\nNo transfers in this category, hit INSERT to switch to another.")
					utw.noutrefresh()
					return
				count = 0
				for transfers in finallist:
					if transfers == None:
						continue
					
					if transfers[1] == 1:
						sattr = self.mucous.colors["green"] | curses.A_BOLD
					elif transfers[1] == 0:
						sattr = self.mucous.colors["magenta"] | curses.A_BOLD
					elif transfers[1] in (10, 11, 12, 13, 14):
						sattr = self.mucous.colors["red"] | curses.A_BOLD
					else:
						sattr = curses.color_pair(0) | curses.A_BOLD
					
					if count + start == s:
						attr = curses.A_REVERSE | curses.A_BOLD
						attr1 = curses.A_NORMAL | curses.A_BOLD
						sattr = curses.A_REVERSE | curses.A_BOLD | sattr
					else:
						attr = curses.A_NORMAL 
						attr1 = curses.A_NORMAL
					try:
						utw.addstr(self.mucous.encode_language(transfers[0])[:4], attr)
						utw.addch(curses.ACS_VLINE, attr1)
						utw.addstr(self.mucous.encode_language(transfers[0])[5:10], attr)
						utw.addch(curses.ACS_VLINE, attr1)
						utw.addstr(self.mucous.encode_language(transfers[0])[11:17], attr)
						utw.addch(curses.ACS_VLINE, attr1)
						utw.addstr(self.mucous.encode_language(transfers[0])[18:28], attr)
						utw.addch(curses.ACS_VLINE, attr1)
						utw.addstr(self.mucous.encode_language(transfers[0])[29:39], sattr)
						utw.addch(curses.ACS_VLINE, attr1)
						utw.addstr(self.mucous.encode_language(transfers[0])[40:w["width"]], attr)

					except:
						pass
					count += 1
					
			except:
				pass
			utw.noutrefresh()
		except Exception, e:
			self.mucous.Help.Log("debug", "Upload Manager: " + str(e))
			
	## Merge transfer stats into a single string
	# @param self Transfers (Class)
	# @param tran transfer string (uploads, downloads)
	# @param user_path tuple (user, path )
	# @param num transfer number
	# @return line, status
	def TransferItem(self, tran, user_path, num):
		try:
			vals = self.transfers[tran][user_path]
			transfer, user, path, status, error, filepos, filesize, rate, place = vals
			path = self.mucous.dlang(self.mucous.dlang(path))

			if filesize:
				filefrmt  = self.mucous.FormatData.byte_format(filesize)
				if len(filefrmt) < 6:
					filefrmt = ' ' * (6-len(filefrmt)) + filefrmt

			else:
				percent = '  0'
				filefrmt = '0 Bits'
			n =  ((4 - len(str(num))) * " ") + str(num)
			u =  user[:10] + ((10 - len(user[:10])) * " ")
			if status == 14 :
				cut_status = self.mucous.dlang(error)[:10]
			else:	
				cut_status = self.states[status][:10]
			if len(cut_status) < 10:
				cut_status += (10-len(cut_status)) *" "
			
			if tran == "downloads":
				if place == 4294967295:
					place = ""
				else:
					place = str(place)
				if len(place) < 4:
					cut_place = (" " * (4 -(len(place) )) )+ place
				else:
					cut_place = place[:3] + "~"

			if self.speed == True:
				#Speed
				ratefrmt  = self.mucous.FormatData.byte_format(rate)
				if len(ratefrmt) < 5:
					ratefrmt += ' '* (5-len(ratefrmt))
				
				if tran == "downloads":
					line = "%s|%s|%s|%s|%s|%s|%s" % (str(n), ratefrmt[:5], filefrmt , u, cut_status, cut_place, path[-self.mucous.w+45+2:])
				else:
					line = "%s|%s|%s|%s|%s|%s" % (str(n), ratefrmt[:5], filefrmt , u, cut_status,  path[-self.mucous.w+40+2:])
			else:
				# Percent
				if filesize != 0:
					percent = str(100 * filepos / filesize)
					percent =  ((3 - len(percent)) * " ") + percent
				else: percent = '  0'
				
				if tran == "downloads":
					line = "%s| %s%%|%s|%s|%s|%s|%s" % (str(n), percent[:5] , filefrmt , u, cut_status, cut_place, path[-self.mucous.w+45+2:])
				else:
					line = "%s| %s%%|%s|%s|%s|%s" % (str(n), percent[:5] , filefrmt , u, cut_status,  path[-self.mucous.w+40+2:])
						
			#line = self.mucous.dlang(line)
			line += ((self.mucous.w-2 - len(line)) * " ")
			return line, status
		except Exception, e:
			self.mucous.Help.Log("debug", "Transfer Item: "+tran +": " + str(e))

	
	## Mouse Coordinates in the Transfers Modes (Outside windows, on the borders)
	# @param self is Transfers (Class)
	# @param x is the horizontal position from the left
	# @param y is the vertical postion from the top
	# @param z is unimportant
	# @param event is the mouse event (button, doubleclick, etc) represented by a number
	def Mouse(self, x,y,z,event):
		try:
			up = self.scrolling[self.current]
			if self.mucous.Config["mucous"]["transbox"]=="split":
				u = self.dimensions["uploads"]
				d = self.dimensions["downloads"]
				
				w = None
				
				if y >= u["top"]+1 and y <= u["top"] + u["height"] and x >= u["left"] and x < u["left"] +u["width"]:
					if self.current != "uploads":
						self.current = "uploads"
						self.ModeTransfers()
					w = u
					#y -= w["top"]+1
				elif y >= d["top"]+1 and y <= d["top"] + d["height"] and x >= d["left"] and x < d["left"] +d["width"]:
					if self.current != "downloads":
						self.current = "downloads"
						self.ModeTransfers()
					w = d

				if w != None:
					if self.MouseTransferWindow(x, y, z, event, w) == 1:
						return

			else:
				if y == 1:
					if x >=2 and x < 26:
						self.current="uploads"
					elif x >=27 and x < 40:	
						self.current="downloads"
					self.ModeTransfers()
					return
					
				if self.current == "uploads":
					w = self.dimensions["uploads"]
				else:
					w = self.dimensions["downloads"]
				r = self.MouseTransferWindow(x, y, z, event, w)
				if r == 1:
					return	
		except Exception, e:
			self.mucous.Help.Log("debug", "Transfers.Mouse: " +str(e) )
	
	## Mouse Coordinates in a Transfers Window
	# @param self is Transfers (Class)
	# @param x is the horizontal position from the left
	# @param y is the vertical postion from the top
	# @param z is unimportant
	# @param event is the mouse event (button, doubleclick, etc) represented by a number
	# @param w is the curses window
	# @return 0 or 1
	def MouseTransferWindow(self, x, y, z, event, w):
		try:
			if y >= w["top"]+1 and y < w["top"] + w["height"] and x >= w["left"] and x < w["left"] +w["width"]:
							
				y -= w["top"] +1
				if y  + w["start"] in range(len( self.get_transfers_list() )):
					if self.mucous.mode in ("chat", "private", "info", "search", "browse", "transfer"):
						mode = self.current
						self.scrolling[mode] = y  + w["start"]
	
				
				if self.current == "downloads":
					self.DownloadManager()
					curses.doupdate()
				else:
					self.UploadManager()
					curses.doupdate()
				if event in ( 4096, 16384):
					if self.get_transfers_list() != []:
						self.mucous.PopupMenu.Create("transfers", 0, True)
				return 1
			elif y == w["top"] + w["height"]:
				if x >=2 and x < 18:
					if self.mucous.Config["mucous"]["transbox"]=="split":
						self.mucous.Config["mucous"]["transbox"]="tabbed"
					elif self.mucous.Config["mucous"]["transbox"]=="tabbed":
						self.mucous.Config["mucous"]["transbox"]="split"
					self.ModeTransfers()
					return
				elif x >= 20:
					if x >=20 and x < 28:
						self.sort = 'all'
					elif x >=28 and x < 39:
						self.sort = 'active'
					elif x >=39 and x < 50:
						self.sort = 'queued'	
					elif x >=50 and x < 58:
						self.sort = 'finished'
					elif x >=63 and x < 75:
						self.sort = 'failed'
					self.ModeTransfers()
					return 1
			return 0
		except Exception, e:
			self.mucous.Help.Log("debug", "MouseTransferWindow: " + str(e))
			
	## Get the transfer number of the currently selected transfer 
	# @param self is Transfers (Class)
	# @param this_list This lists
	# @return number
	def get_transfer_number(self, this_list):
		try:
			if self.current != "downloads":
				mode = "uploads"
			else:
				mode = "downloads"
			number = None
			username = this_list[self.scrolling[mode]][1] 
			path = this_list[self.scrolling[mode]][2]
			
			userpath = (username, path)
			
			
			count = 1
			transfer = None
			for user, path  in self.sorted_transfer_list[mode]:
				if (user, path) == userpath:
					#transfer = count
					number = count
					break
				count += 1
			return number
		except Exception, e:
			self.mucous.Help.Log("debug", "get_transfer_number: " + str(e))

	## Get the transfer list (status limited to Transfers.sort)  
	#:: format of each item in the list is [upload, username, path, status, error, filepos, filesize, rate] 
	# @param self is Transfers (Class)
	# @return this_list
	def get_transfers_list(self):
		try:
			this_list = []
			
			if self.sort == 'active':
				status_list = (1, 2, 3, 4, 5, 6, 8, 9,)
			elif self.sort == 'all':
				status_list = (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,  14)
			elif self.sort == 'finished':
				status_list = (0, 99999)
			elif self.sort == 'queued':
				status_list = (7, 99999)
			elif self.sort == 'failed':
				status_list = (  10, 11, 12, 13)
			if self.current == "uploads":
				
				for username, path in self.sorted_transfer_list["uploads"]:
					vals = self.transfers["uploads"][(username, path)]
					upload, username, path, status, error, filepos, filesize, rate = vals[0], vals[1], vals[2], vals[3], vals[4], vals[5], vals[6], vals[7]
					if status in status_list:
						this_list.append(vals)
						
			elif self.current == "downloads":
				#self.mucous.Help.Log("debug", self.sorted_transfer_list["downloads"])
				for username, path in self.sorted_transfer_list["downloads"]:
					vals = self.transfers["downloads"][(username, path)]
					upload, username, path, status, error, filepos, filesize, rate = vals[0], vals[1], vals[2], vals[3], vals[4], vals[5], vals[6], vals[7]
					if status in status_list:
						this_list.append(vals)
			#this_list.sort(key=str.lower)
			return this_list
		except Exception, e:
			self.mucous.Help.Log("debug", "get_transfers_list: " + str(e))
			

#!/usr/bin/python
import sys, os, ConfigParser, time
from PyQt4 import QtCore, QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *

from mainwindow import Ui_MainWindow
from chatroom import Ui_Room
import imagedata

from networking import Networking

config_dir = str(os.path.expanduser("~/.muqt/"))
log_dir = str(os.path.expanduser("~/.muqt/logs/"))
config_file = config_dir+"config"
DEBUG = 1
import gettext
tr_cache = {}

def _(s):
	global tr_cache
	if not tr_cache.has_key(s):
		tr_cache[s] = gettext.gettext(s)
	return tr_cache[s]
## Modify and read the Murmur Config
#
	
def recode(s):
	try:
		return s.decode(locale.nl_langinfo(locale.CODESET), "replace").encode("utf-8", "replace")
	except:
		return s
		
class ConfigManager:
	## Constructor
	# @param self ConfigManager
	# @param murmur Murmur (Class)
	def __init__(self, app):
		## @var parser
		# ConfigParser instance
		self.parser = ConfigParser.ConfigParser()
		## @var murmur
		# Murmur (class)
		self.app = app

		self.app.Config = {\
"connection":{"interface": 'localhost:2240', "passw":None}, \
"museekd": {"configfile": os.path.expanduser("~/.museekd/config.xml")}, \
\
"muqt":{"trayapp": True, "tooltips": False, "autobuddy": False, "rooms_sort": "size",
"now-playing": "default", "browse_display_mode": "filesystem"}, \
\
"ui":{"icontheme": "", "chatme":"FOREST GREEN", "chatremote":"","chatlocal":"BLUE", \
"chathilite":"red", "useronline":"BLACK", "useraway":"ORANGE","useroffline":"#aa0000",\
"usernamehotspots":1, "usernamestyle": "bold", "textbg": "", "search":"","searchq":"GREY", \
"inputcolor":"", "exitdialog": 1, "notexists": 1,\
"decimalsep":",", "chatfont": "", "roomlistcollapsed": 0, "tabclosers": 1,\
"buddylistinchatrooms": 0, "trayicon": 1, "soundenabled": 1, "soundtheme": "",\
"soundcommand": "play -q", "filemanager": "rox $", "speechenabled": 0, "enabletrans":0,\
"width": 800, "height": 600},\
\
"columns":{"userlist":[1,1,1,1,1,1,1,1,1],  "chatrooms":{}, "downloads":[1,1,1,1,1,1,1,1,1], "uploads":[1,1,1,1,1,1,1,1,1], "search":[1,1,1,1,1,1,1,1,1,1] },\
\
"logging": { "logcollapsed": False, "log_dir":  os.path.expanduser("~/.muqt/logs/"),
"logrooms": True, "rooms":[], "chatrooms": True, "private": [], "logprivate": True}, \
\
"urls":{"urlcatching":True ,"protocols": {"http":"firefox %s", "https":"firefox %s"}, "humanizeurls": True},\
\
"players": { "default": "xmms -e $", "npothercommand": "", "npplayer": "infopipe", "npformatlist": [], "npformat": "" }
}
		
	## Create config dict from config file
	# @param self ConfigManager
	def create_config(self):
	
		self.parser.read([config_file])
	
		app_config_file = file(config_file, 'w')
		
		for i in self.app.Config.keys():
			if not self.parser.has_section(i):
				self.parser.add_section(i)
			for j in self.app.Config[i].keys():
				if j not in ["nonexisting", "hated", "options"]:
					self.parser.set(i,j, self.app.Config[i][j])
				else:
					self.parser.remove_option(i,j)
		self.parser.write(app_config_file)
		app_config_file.close()
	
	## Create config file and parse options
	# @param self ConfigManager
	def read_config(self):
		
		self.parser.read([config_file])
		for i in self.parser.sections():
			for j in self.parser.options(i):
				val = self.parser.get(i,j, raw = 1)
	
				if j in ['login','passw','interface', 'trayapp',  'tickers_enabled', "ticker_cycle",  "autobuddy",  "now-playing", "log_dir", "configfile",  "urlcatching",  "humanizeurls", "autobuddy" ] or i == "ui" and j not in [ "usernamehotspots", "width", "height"] and val != "None":
					self.app.Config[i][j] = val
				elif i == 'aliases' and val != "None":
					self.app.Config[i][j] = val
				else:
					try:
						self.app.Config[i][j] = eval(val, {})
					except:
						try:
							self.app.Config[i][j] = None
						except:
							pass
	
	## Write config file to disk
	# @param self ConfigManager
	def update_config(self):
		app_config_file = file(config_file, 'w')
		for i in self.app.Config.keys():
			if not self.parser.has_section(i):
				self.parser.add_section(i)
			for j in self.app.Config[i].keys():
				if j not in ["somethingwrong"]:
					
					self.parser.set(i,j, self.app.Config[i][j])
				else:
					self.parser.remove_option(i,j)
		self.parser.write(app_config_file)
		app_config_file.close()
	
	## Check the filesystem for the existance of the config file
	# Create it if it doesn't exist
	# Read it if it does exist 
	# @param self ConfigManager
	def check_path(self):
		if os.path.exists(config_dir):
			if os.path.exists(config_file) and os.stat(config_file)[6] > 0:
				self.read_config()
			else:
				self.create_config()
				
		else:
			os.mkdir(config_dir, 0700)
			self.create_config()
		
		
class MuQT(QtGui.QMainWindow):
	def __init__(self, parent=None):
		QtGui.QWidget.__init__(self, parent)
		self.Config = {}
		self.pid = os.getpid()
		## @var config_manager
		# ConfigManager (Class)
		self.config_manager = ConfigManager(self)
		self.config_manager.check_path()
		self.ui = Ui_MainWindow()
		self.ui.setupUi(self)
		self.timer = None
		self.status = None
		

		self.statusbar = None
		self.user_stats = {}
		self.user_exists = {}
		self.ip_requested = []
		self.username = None
		self.images = {}
		
		for i in "away",  "online",  "offline", "noexist", "logo", "close", "green", "yellow", "red", "icon", "away_trusted", "away_banned", "away_buddied", "away_ignored", "online_trusted", "online_banned", "online_ignored", "online_buddied", "offline_trusted", "offline_banned", "offline_ignored", "offline_buddied", "hilite", "empty":
			#loader = gtk.gdk.PixbufLoader("png")
			
			data = getattr(imagedata, i)
			loader = QPixmap()
			if loader.loadFromData(data):
				self.images[i] = QIcon(loader)
			else:
				print i, "failed to load"
			#loader.write(data, len(data))
			#loader.close()
			#self.images[i] = loader.get_pixbuf()
			del loader, data
		#gc.collect()
		#self.MurmurWindow.set_icon(self.images["icon"])
		
		self.ui.ChatRooms = QtGui.QTabWidget(self.ui.ChatRoomsLabel)
		self.ui.ChatRooms.setObjectName("ChatRooms")
		self.ui.verticalLayout_3.addWidget(self.ui.ChatRooms)
		
		self.ChatRooms = ChatRooms(self)
		self.Networking = Networking(self)
		
		# Networking signals
		self.connect(self.Networking, SIGNAL("JoinRoom(PyQt_PyObject, PyQt_PyObject)"), self.ChatRooms.JoinRoom)
		self.connect(self.Networking, SIGNAL("LeaveRoom(PyQt_PyObject)"), self.ChatRooms.LeaveRoom)
		self.connect(self.Networking, SIGNAL("UserStatus(PyQt_PyObject, PyQt_PyObject)"), self.ChatRooms.GetUserStatus)
		self.connect(self.Networking, SIGNAL("SayChatRoom(PyQt_PyObject,PyQt_PyObject, PyQt_PyObject)"), self.ChatRooms.SayChatRoom)
		#self.connect(self.Networking, SIGNAL("SayChatRoom(PyQt_PyObject, PyQt_PyObject, PyQt_PyObject)"), self.PrivateChat.SayPrivate)
		self.connect(self.Networking, SIGNAL("Log(PyQt_PyObject)"), self.AppendToLogWindow)
		self.connect(self.Networking, SIGNAL("PrivateMessage(PyQt_PyObject,PyQt_PyObject, PyQt_PyObject)"), self.Networking.PrivateMessage)
		self.connect(self.Networking, SIGNAL("UserJoinedRoom(PyQt_PyObject, PyQt_PyObject)"), self.ChatRooms.UserJoinedRoom)
		self.connect(self.Networking, SIGNAL("UserLeftRoom(PyQt_PyObject, PyQt_PyObject)"), self.ChatRooms.UserLeftRoom)
		# Main Menu actions
		self.connect(self.ui.actionToggle_Away, SIGNAL("activated()"), self.away_toggle)
		self.connect(self.ui.actionAbout_Qt, SIGNAL("activated()"), self.OnAbout)
		
		self.Networking.start()
		
	def OnAbout(self):
		QMessageBox.aboutQt(self)
		
	def AppendToLogWindow(self, message):
		message = "%s %s" % (recode(time.strftime("%H:%M:%S")), message)
		self.ui.logWindow.append(message)
		
	def away_toggle(self):
		try:
			if self.username != None:
				if self.status == 0:
					self.Networking.SetStatus(1)
				elif self.status == 1:
					self.Networking.SetStatus(0)
			else:
				self.AppendToLogWindow(_("You are disconnected from Museek"))

		except Exception,e:
			if DEBUG: Output("away_toggle ERROR", e)
			
	def GetStatusImage(self, user, num):
		bud = self.Networking.config.has_key("buddies") and  self.Networking.config["buddies"].has_key(user)
		ban = self.Networking.config.has_key("banned") and  self.Networking.config["banned"].has_key(user)
		ign = self.Networking.config.has_key("ignored") and  self.Networking.config["ignored"].has_key(user)
		tru = self.Networking.config.has_key("trusted") and  self.Networking.config["trusted"].has_key(user)

		if num == 0:
			image = "offline"
		elif num == 1:
			image = "away"
		elif num == 2:
			image = "online"

		if num in (0, 1, 2):
			if ban:
				image += "_banned"
			elif ign:
				image += "_ignored"
			elif tru:
				image += "_trusted"
			elif bud:
				image += "_buddied"
		if num == 0:
			if self.user_exists.has_key(user) and not self.user_exists[user]:
				#if not ban and not ign and not tru and not bud:
				image = "noexist"
		
		return  self.images[image]
	def numfmt(self, value):
		v = str(float(value)) + '0000'
		i = v.index('.')
		if i < 4:
			return v[:5]
		else:
			return v[:i+2]
		
	def HSpeed(self, speed):
		return _("%s KB/s") % self.numfmt(float(speed) / 1024.0)
	
	def Humanize(self, size):
		if size is None:
			return None

		try:
			s = int(size)
			if s >= 1000*1024*1024:
				r = _("%s GB") % self.numfmt(float(s) / 1073741824.0 )
			elif s >= 1000*1024:
				r = _("%s MB") % self.numfmt(float(s) / 1048576.0)
			elif s >= 1000:
				r = _("%s KB") % self.numfmt(float(s) / 1024.0)
			else:
				r = _("%s  B") % self.numfmt(float(s) )
			return r
		except Exception, e:
			Output(e)
			return size 
		
		
class ChatRooms:
	def __init__(self, parent=None):
		self.frame=parent
		self.joinedrooms = {}

		
	def JoinRoom(self, room, users):
		#print room, users
		#print self.frame.ui.ChatRooms
		#print Chatroom
		if room not in self.joinedrooms:
			self.joinedrooms[room] = Chatroom(self, room, users)
			self.frame.ui.ChatRooms.addTab(self.joinedrooms[room], room)
		else:
			self.joinedrooms[room].Rejoined(users)
		
	def LeaveRoom(self, room):
		roomwidget = self.joinedrooms[room]
		self.frame.ui.ChatRooms.removeTab(self.frame.ui.ChatRooms.indexOf(roomwidget))
		#if room.logfile is not None:
			#room.logfile.close()
			#room.logfile = None
		roomwidget.destroy()
		del self.joinedrooms[room]

			
	def UserJoinedRoom(self, room, username):
		if self.joinedrooms.has_key(room):
			self.joinedrooms[room].UserJoinedRoom(username)
	
	def UserLeftRoom(self, room, username):
		self.joinedrooms[room].UserLeftRoom(username)
	
	
	def SayChatRoom(self, room, user, text):
		if self.frame.Networking.config.has_key("ignored") and user in self.frame.Networking.config["ignored"].keys():
			return
		if room in self.joinedrooms:
			self.joinedrooms[room].SayInChatRoom(user, text)
			
	def GetUserStatus(self, user, status):
		for room in self.joinedrooms.values():
			if user in room.users.keys():
				room.GetUserStatus(user, status)
	def ConnClose(self):
		for room in self.joinedrooms.values():
			room.ConnClose()
		
class Chatroom(QtGui.QWidget):
	def __init__(self, chatrooms=None, room=None, users=None):
		self.lines = []
		self.chatrooms = chatrooms
		self.frame = chatrooms.frame
		self.room = room
		QtGui.QWidget.__init__(self, None)
		self.ui = Ui_Room()
		self.ui.setupUi(self)
		#self.connect(self, SIGNAL("PrivateMessage(PyQt_PyObject,PyQt_PyObject, PyQt_PyObject)"), self.frame.Networking.PrivateMessage)
		self.ui.UserList.setHeaderLabels(("Status", "User", "Speed", "Files"))
		self.ui.UserList.setIndentation(0)
		self.ui.UserList.setColumnWidth(0, 20)
		self.ui.UserList.setColumnWidth(1, 150)
		self.ui.UserList.setColumnWidth(2, 80)
		self.ui.UserList.setColumnWidth(3, 50)
		self.ui.UserList.setColumnCount(4);
		self.ui.UserList.sortItems(1, Qt.AscendingOrder);
		self.tag_remote = self.makecolour(self.ui.UserList, "chatremote")
		self.tag_local = self.makecolour(self.ui.UserList, "chatlocal")
		self.tag_me = self.makecolour(self.ui.UserList, "chatme")
		self.tag_hilite = self.makecolour(self.ui.UserList, "chathilite")
		
		self.users = {}
		for username,stats in users.items():
			self.users[username] = QTreeWidgetItem(self.ui.UserList.invisibleRootItem())
			
			status, speed, downloads, files, dirs = self.frame.user_stats[username] = [stats[0], stats[1], stats[2], stats[3], stats[4]]
			hspeed = self.frame.HSpeed(speed)
			hfiles = self.frame.Humanize(files)
			#print self.frame.user_stats[user]
			self.users[username].setIcon(0, self.frame.GetStatusImage(username, status))
			self.users[username].setText(1, username)
			self.users[username].setText(2, hspeed)
			self.users[username].setText(3, hfiles)
			self.users[username].setText(4, str(status))
			self.users[username].setData(5, 0, QVariant(speed))
			self.users[username].setData(6, 0, QVariant(files))
			#newuser.setText(0, status)
		self.connect(self.ui.ChatEntry, SIGNAL("returnPressed()" ), self.SayRoom)
		self.connect(self.ui.Close, SIGNAL("clicked()"), self.LeaveRoom)
		self.tag_log = None
		
	def ConnClose(self):
		self.ui.UserList.clear()
		self.users.clear()
		
	def Rejoined(self, users):
		self.ui.UserList.clear()
		self.users.clear()
		for username,stats in users.items():
			self.users[username] = QTreeWidgetItem(self.ui.UserList.invisibleRootItem())
			status, speed, downloads, files, dirs = self.frame.user_stats[username] = [stats[0], stats[1], stats[2], stats[3], stats[4]]
			hspeed = self.frame.HSpeed(speed)
			hfiles = self.frame.Humanize(files)
			#print self.frame.user_stats[user]
			self.users[username].setIcon(0, self.frame.GetStatusImage(username, status))
			self.users[username].setText(1, username)
			self.users[username].setText(2, hspeed)
			self.users[username].setText(3, hfiles)
			self.users[username].setText(4, str(status))
			self.users[username].setData(5, 0, QVariant(speed))
			self.users[username].setData(6, 0, QVariant(files))
			
		
	def LeaveRoom(self):
		self.frame.Networking.LeaveRoom(self.room)
		
	def SayRoom(self):
		self.frame.Networking.SayRoom(self.room, str(self.ui.ChatEntry.text()))
		self.ui.ChatEntry.clear()
		
	def GetUserStatus(self, user, status):
		if not self.users.has_key(user):
			return
		#print self.users[user].text(0)
		if status != int(self.users[user].text(4)):
			self.users[user].setText(4,  str(status))
			img = self.frame.GetStatusImage(user, status)
			self.users[user].setIcon(0, QIcon(img))
		else:
			return
		if status == 1:
			action = ("%s has gone away")
		else:
			action = ("%s has returned")
		AppendLine(self.ui.StatusLog, action % user) #, self.tag_log)
		#if user in self.tag_users.keys():
			#color = self.getUserStatusColor(status)
			#self.changecolour(self.tag_users[user], color)
		#self.usersmodel.set(self.users[user], 0, img, 4, status)
			
	def SayInChatRoom(self, user, line):
		#line = line.replace("<", "<pre><")
		#if len(self.lines) >= 400:
			#buffer = self.chatview.get_buffer()
			#start = buffer.get_start_iter()
			#end = buffer.get_iter_at_line(self.lines[200])
			#self.chatview.get_buffer().delete(start, end)
			#del self.lines[0:200]
		
		# Display /me messages as "* username message"
		if line[:4] == "/me ":
			message = "* %s %s"  % (user,line[4:])
			tag = Qt.green
		else:
			message = "[%s] %s" % (user, line.replace("<", "\\<"))
			if user == self.frame.username:
				tag = self.tag_local
			elif line.upper().find(self.frame.username.upper()) > -1:
				tag = self.tag_hilite
			else:
				tag = self.tag_remote
		message = "\n-- ".join(message.split("\n"))
		#message = "%s %s" % (recode(time.strftime("%H:%M:%S")), message)
		#if user in self.tag_users.keys():
			#usertag = self.tag_users[user]
		#else:
			#usertag = self.tag_users[user] = self.makecolour(self.chatview.get_buffer(), color, username=user)
		#self.lines.append(AppendLine(self.chatview, message, tag=tag, username=user, usertag=usertag))
		AppendLine(self.ui.ChatLog, message, tag)
		#if self.frame.username is not None:
			#if user != self.frame.username and self.frame.username in message:
				#self.chatrooms.request_hilite(self.Main)
			#else:
				#self.chatrooms.request_changed(self.Main)
			#self.frame.RequestIcon(self.frame.ChatRoomLabel)
			
	def makecolour(self, buffer, colour, username=None):
		color = self.frame.Config["ui"][colour]
		if color:
			tag = QtGui.QBrush( QColor(color) )
		else:
			tag = QtGui.QBrush(  )
			
	def changecolour(self, tag, colour):
		if self.frame.Config["ui"].has_key(colour):
			color = self.frame.Config["ui"][colour]
		else:
			color = ""
		#font = self.frame.Config["ui"]["chatfont"]
		if color:
			tag = QtGui.QBrush(  QColor(color)  )
			
			
	def UserJoinedRoom(self, username):
		if self.users.has_key(username):
			return

		status, speed, downloads, files, dirs = self.frame.user_stats[username]
		AppendLine(self.ui.StatusLog, _("%s joined the room") % username) #, self.tag_log)
		img = self.frame.GetStatusImage(username, status)
		hspeed = self.frame.HSpeed(speed)
		hfiles = self.frame.Humanize(files)
		self.users[username] = QTreeWidgetItem(self.ui.UserList.invisibleRootItem())
		self.users[username].setText(1, username)
		#status, speed, downloads, files, dirs = self.frame.user_stats[username] = [stats[0], stats[1], stats[2], stats[3], stats[4]]
		#print self.frame.user_stats[user]
		self.users[username].setIcon(0,  self.frame.GetStatusImage(username, status))
		self.users[username].setText(4, str(status))
		self.users[username].setData(2, 0, QVariant(speed))
		self.users[username].setData(3, 0, QVariant(files))
		#iter = username.ui.append([img, username, hfiles, hspeed, status])
		#self.users[username] = iter
		#color = self.getUserStatusColor(status)
		#if username in self.tag_users.keys():
			#self.changecolour(self.tag_users[username], color)
		#else:
			#self.tag_users[username] = self.makecolour(self.chatview.get_buffer(), color, username=username)
	
	def UserLeftRoom(self, username):
		if not self.users.has_key(username):
			return
		AppendLine(self.ui.StatusLog, _("%s left the room") % username, self.tag_log)
		#self.usersmodel.remove(self.users[username])
		#self.users[username].close()
		self.ui.UserList.invisibleRootItem().removeChild(self.users[username])
		del self.users[username]
		#if username in self.tag_users.keys():
			#color = self.getUserStatusColor(-1)
			#self.changecolour(self.tag_users[username], color)
			
def AppendLine(logwindow, message, tag=None):
	if tag is None:
		tag = QtGui.QBrush()
	
	message = "\n-- ".join(message.split("\n"))
	cursor = QtGui.QTextCursor( logwindow.document() )
	cursor.movePosition(QTextCursor.End, QTextCursor.MoveAnchor)
	charformat = QtGui.QTextCharFormat()
	charformat.setForeground( QtGui.QBrush())
	cursor.insertText("%s " % (recode(time.strftime("%H:%M:%S")) ), charformat)

	charformat.setForeground(tag)
	cursor.insertText(message+"\n", charformat)
	
if __name__ == "__main__":
	app = QtGui.QApplication(sys.argv)
	muqt = MuQT()
	muqt.show()
	sys.exit(app.exec_())

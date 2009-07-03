#!/usr/bin/python
import sys, os, ConfigParser, time
from PyQt4 import QtCore, QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *

from mainwindow import Ui_MainWindow
from chatroom import Ui_Room
from privatechat import Ui_Private
from settings import Ui_SettingsWindow
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
		
		#self.ui.ChatRooms = QtGui.QTabWidget(self.ui.ChatRoomsLabel)
		#self.ui.ChatRooms.setObjectName("ChatRooms")
		
		
		self.ChatRooms = ChatRooms(self)
		self.ui.ChatRoomsLayout.addWidget(self.ChatRooms)
		self.PrivateChats = PrivateChats(self)
		self.ui.PrivateChatsLayout.addWidget(self.PrivateChats)
		self.Networking = Networking(self)
		self.Downloads = Downloads(self)
		self.Uploads = Uploads(self)
		self.Settings = Settings(self)
		
		# Networking signals
		self.connect(self.Networking, SIGNAL("JoinRoom(PyQt_PyObject, PyQt_PyObject)"), self.ChatRooms.JoinRoom)
		self.connect(self.Networking, SIGNAL("LeaveRoom(PyQt_PyObject)"), self.ChatRooms.LeaveRoom)
		self.connect(self.Networking, SIGNAL("UserStatus(PyQt_PyObject, PyQt_PyObject)"), self.ChatRooms.GetUserStatus)
		self.connect(self.Networking, SIGNAL("SayChatRoom(PyQt_PyObject,PyQt_PyObject, PyQt_PyObject)"), self.ChatRooms.SayChatRoom)
		#self.connect(self.Networking, SIGNAL("SayChatRoom(PyQt_PyObject, PyQt_PyObject, PyQt_PyObject)"), self.PrivateChat.SayPrivate)
		self.connect(self.Networking, SIGNAL("Log(PyQt_PyObject)"), self.AppendToLogWindow)
		self.connect(self.Networking, SIGNAL("ShowMessage(PyQt_PyObject,PyQt_PyObject, PyQt_PyObject)"), self.PrivateChats.ShowMessage)
		self.connect(self.Networking, SIGNAL("UserJoinedRoom(PyQt_PyObject, PyQt_PyObject)"), self.ChatRooms.UserJoinedRoom)
		self.connect(self.Networking, SIGNAL("UserLeftRoom(PyQt_PyObject, PyQt_PyObject)"), self.ChatRooms.UserLeftRoom)
		self.connect(self.Networking, SIGNAL("ConnClose()"), self.ChatRooms.ConnClose)
		# Main Menu actions
		self.connect(self.ui.actionToggle_Away, SIGNAL("activated()"), self.away_toggle)
		self.connect(self.ui.actionAbout_Qt, SIGNAL("activated()"), self.OnAbout)
		self.connect(self.ui.actionConnect_to_daemon, SIGNAL("activated()"), self.OnConnect)
		self.connect(self.ui.actionDisconnect_from_daemon, SIGNAL("activated()"), self.OnDisconnect)
		self.connect(self.ui.actionConfigure, SIGNAL("activated()"), self.OnConfigure)
		
		self.Networking.start()
	
	def OnConfigure(self):
		self.Settings.show()
		
	def OnDisconnect(self):
		self.Networking.disconnect("")
	def OnConnect(self):
		self.Networking.connect_to_museekd("") 
		
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

	def InitialiseColumns(self, treeview, *args):
		i = 0
		
		cols = []
		labels = [label[0] for label in args]
		treeview.setHeaderLabels(labels )
		treeview.setColumnCount(len(labels))
		for c in args:
			if c[2] == "text":
				pass
			treeview.setColumnWidth(i, c[1])
			i += 1

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
		
	def HumanSize(self, number):
		try:
			s = float(int(number))
			if s >= 1000*1024*1024:
				r = _("%.2f GiB") % (s / (1024.0*1024.0*1024.0))
			elif s >= 1000*1024:
				r = _("%.2f MiB") % (s / (1024.0*1024.0))
			elif s >= 1000:
				r = _("%.2f KiB") % (s / 1024.0)
			else:
				r = _("%d Bytes") % s
			return r
		except Exception, e:
			return number
			
	def HumanSpeed(self, number):
		try:
			s = float(int(number))
			if s >= 1000*1024*1024:
				r = _("%.2f GiB/s") % (s / (1024.0*1024.0*1024.0))
			elif s >= 1000*1024:
				r = _("%.2f MiB/s") % (s / (1024.0*1024.0))
			elif s >= 1000:
				r = _("%.2f KiB/s") % (s / 1024.0)
			else:
				r = _("%d B/s") % (number)
			return r
		except Exception, e:
			return number
			
	def Humanize(self, number):
		fashion = self.Config["ui"]["decimalsep"]
		if fashion == "" or fashion == "<None>":
			return str(number)
		elif fashion == "<space>":
			fashion = " "
		number = str(number)
		if number[0] == "-":
			neg = "-"
			number = number[1:]
		else:
			neg = ""
		ret = ""
		while number[-3:]:
			part, number = number[-3:], number[:-3]
			ret = "%s%s%s" % (part, fashion, ret)
		return neg + ret[:-1]
		
	def makecolour(self, buffer, colour, username=None):
		color = self.Config["ui"][colour]
		if color:
			tag = QtGui.QBrush( QColor(color) )
		else:
			tag = QtGui.QBrush(  )
			
	def changecolour(self, tag, colour):
		if self.Config["ui"].has_key(colour):
			color = self.Config["ui"][colour]
		else:
			color = ""
		#font = self.frame.Config["ui"]["chatfont"]
		if color:
			tag = QtGui.QBrush(  QColor(color)  )

class Settings(QtGui.QDialog):
	def __init__(self, parent):
		self.frame = parent
		QtGui.QDialog.__init__(self, self.frame)
		self.ui = Ui_SettingsWindow()
		self.ui.setupUi(self)
		
		#self.ui.SettingsList.
		self.frame.InitialiseColumns(self.ui.SettingsList, [_("Settings"), 170, "text"])
		
		self.pages = {}
		
		for page in ['GUI', 'Museek Daemon']:
			self.pages[page] = QTreeWidgetItem(self.ui.SettingsList.invisibleRootItem())
			self.pages[page].setText(0, page)
			self.pages[page].setExpanded(True)
		for page in ["Login", "Appearance"]:
			self.pages[page] = QTreeWidgetItem()
			self.pages[page].setText(0, page)
			self.pages["GUI"].addChild(self.pages[page])
		for page in ["Server", "Client Interfaces", "Transfers", "Chat Rooms", "Users", "Userinfo", "Shares"]:
			#self.pages[page] = QTreeWidgetItem(self.pages["Museek Daemon"])
			self.pages[page] = QTreeWidgetItem()
			self.pages[page].setText(0, page)
			self.pages["Museek Daemon"].addChild(self.pages[page])
		
		
		self.ui.stackedWidget.setCurrentWidget(self.ui.PageShares)
		self.connect(self.ui.SettingsList, SIGNAL("itemSelectionChanged()"), self.OnSelection)
		
	def OnSelection(self):
		widget = self.ui.SettingsList.selectedItems()
		if widget[0] is self.pages["Login"]:
			newpage = self.ui.PageLogin
		elif widget[0] is self.pages["Appearance"]:
			newpage = self.ui.PageAppearance
		elif widget[0] is self.pages["Server"]:
			newpage = self.ui.PageServer
		elif widget[0] is self.pages["Client Interfaces"]:
			newpage = self.ui.PageClients
		elif widget[0] is self.pages["Transfers"]:
			newpage = self.ui.PageTransfers
		elif widget[0] is self.pages["Chat Rooms"]:
			newpage = self.ui.PageChatRooms
		elif widget[0] is self.pages["Users"]:
			newpage = self.ui.PageUsers
		elif widget[0] is self.pages["Userinfo"]:
			newpage = self.ui.PageUserinfo
		elif widget[0] is self.pages["Shares"]:
			newpage = self.ui.PageShares
		else:
			newpage = self.ui.PageBlank
		self.ui.stackedWidget.setCurrentWidget(newpage)
		
class Downloads:
	def __init__(self, parent=None):
		self.frame = parent
		cols = self.frame.InitialiseColumns(self.frame.ui.Downloads,
		[_("Username"), 100, "text"], #0
		[_("Filename"), 250, "text"], #1
		[_("Speed"), 80, "text"], #2
		[_("Status"), 80, "text"], #3
		[_("Pos"), 50, "text"], #4
		[_("Size"), 80, "text"], #5
		[_("Path"), 350, "text"], #6
		)
		self.frame.ui.Downloads.setIndentation(0)
		#print cols

		
class Uploads:
	def __init__(self, parent=None):
		self.frame = parent
		cols = self.frame.InitialiseColumns(self.frame.ui.Uploads,
		[_("Username"), 100, "text"], #0
		[_("Filename"), 250, "text"], #1
		[_("Speed"), 80, "text"], #2
		[_("Status"), 80, "text"], #3
		[_("Pos"), 50, "text"], #4
		[_("Size"), 80, "text"], #5
		[_("Path"), 350, "text"], #6
		)
		self.frame.ui.Uploads.setIndentation(0)
class PrivateChats(QtGui.QTabWidget):
	def __init__(self, parent=None):
		self.frame=parent
		self.users = {}
		QtGui.QTabWidget.__init__(self, self.frame.ui.PrivateChatsLabel)
		user = "daelstorm"
		
		
	def SendMessage(self, user, text = None, direction = None):
		if not self.users.has_key(user):
			self.users[user] = PrivateChat(self, user)
			self.addTab(self.users[user], user)

		#if direction:
			#if self.get_current_page() != self.page_num(self.users[user].Main):
				#self.set_current_page(self.page_num(self.users[user].Main))
		if text is not None:
			self.users[user].SendMessage(text)
	
	def ShowMessage(self, direction, user, text):
		if self.frame.Networking.config.has_key("ignored") and self.frame.Networking.config["ignored"].has_key(user):
			return
		ctcpversion = 0
		if text == "\x01VERSION\x01":
			ctcpversion = 1
			text = "CTCP VERSION"
		self.SendMessage(user, None)
		tab = self.users[user]
		tab.ShowMessage(direction, text)
		#if ctcpversion and self.frame.np.config.sections["server"]["ctcpmsgs"] == 0:
			#self.SendMessage(msg.user, "Nicotine %s" % version)
		#self.request_changed(tab.Main)
		#self.frame.RequestIcon(self.frame.PrivateChatTabLabel)
		#if self.get_current_page() != self.page_num(self.users[msg.user].Main) or self.frame.notebook1.get_current_page() != 1:
			#if msg.user not in self.frame.tray_status["hilites"]["private"]:
				#self.frame.tray_status["hilites"]["private"].append(msg.user)
				#self.frame.SetImage(None)
		#self.request_changed(self.users[user].Main)
		
	def OnClose(self, user):
		tab = self.users[user]
		self.removeTab(self.indexOf(tab))
		#if room.logfile is not None:
			#room.logfile.close()
			#room.logfile = None
		tab.destroy()
		del self.users[user]
		
class PrivateChat(QtGui.QWidget):
	def __init__(self, chats, user):
		self.chats  = chats
		self.frame = chats.frame
		self.user = user
		self.Status = 0
		QtGui.QWidget.__init__(self, None)
		self.ui = Ui_Private()
		self.ui.setupUi(self)
		self.tag_remote = self.frame.makecolour(self.ui.ChatLog, "chatremote")
		self.tag_local = self.frame.makecolour(self.ui.ChatLog, "chatlocal")
		self.tag_me = self.frame.makecolour(self.ui.ChatLog, "chatme")
		self.tag_hilite = self.frame.makecolour(self.ui.ChatLog, "chathilite")
		if self.frame.user_stats.has_key(self.user):
			status = self.frame.user_stats[self.user][0]
			if status == 1:
				color = "useraway"
			elif status == 2:
				color = "useronline"
			else:
				color = "useroffline"
		else:
			color = "useroffline"
		self.tag_username = self.frame.makecolour(self.ui.ChatLog, color)
		self.connect(self.ui.ChatEntry, SIGNAL("returnPressed()" ), self.OnEnter)
		self.connect(self.ui.Close, SIGNAL("clicked()"), self.OnClose)
		
	def ShowMessage(self, direction, text):
		if self.user == self.frame.username:
			tag = self.tag_local
		elif text.upper().find(self.frame.username.upper()) > -1:
			tag = self.tag_hilite
		else:
			tag = self.tag_remote
		
		if direction == 0:
			username = self.user
		else:
			username = self.frame.username
				
		if text[:4] == "/me ":
			line = "* %s %s" % (username, text[4:])
			tag = self.tag_me
		else:
			line = "[%s] %s" % (username, text)
		#self.frame.RequestIcon(self.frame.PrivateChatLabel)
		AppendLine(self.ui.ChatLog, line, tag, "%c", username=self.user, usertag=self.tag_username)
		#if self.z:
			#self.z = 0
			#self.tag_username.set_property("foreground", "#00FF00")
		#else:
			#self.z = 1
			#self.tag_username.set_property("foreground", "#FF0000")
		#if self.Log.get_active():
			#self.logfile = WriteLog(self.logfile, self.frame.Config["logging"]["logsdir"], self.user, line)
			#self.frame.Logging.PrivateChatLog( direction, self.user, text)
		
		#autoreply = self.frame.np.config.sections["server"]["autoreply"]
		#if self.frame.away and not self.autoreplied and autoreply:
			#self.SendMessage("[Auto-Message] %s" % autoreply)
			#self.autoreplied = 1
	def OnClose(self):
		self.chats.OnClose(self.user)

		
	def OnEnter(self):
		text = str(self.ui.ChatEntry.text())
		
		if text[:2] == "//":
			text = text[1:]
		self.SendMessage(text)
		self.frame.Networking.PrivateMessage(0, self.user, text)
		self.ui.ChatEntry.clear()
		
	def SendMessage(self, text):
		
		if text[:4] == "/me ":
			line = "* %s %s" % (self.frame.username, text[4:])
			tag = self.tag_me
		else:
			
			if text == "\x01VERSION\x01":
				line = "CTCP VERSION"
			else:
				line = "[%s] %s" % (self.frame.username, text)
			tag = self.tag_local
			
		AppendLine(self.ui.ChatLog, line, tag, "%c", username=self.user, usertag= self.tag_username)
		#if self.Log.get_active():
			#self.frame.Logging.PrivateChatLog( 1, self.user, text)
			
class ChatRooms(QtGui.QTabWidget):
	def __init__(self, parent=None):
		self.frame=parent
		self.joinedrooms = {}
		QtGui.QTabWidget.__init__(self, self.frame.ui.ChatRoomsLabel)
		
	def JoinRoom(self, room, users):
		#print room, users
		#print self.frame.ui.ChatRooms
		#print Chatroom
		if room not in self.joinedrooms:
			self.joinedrooms[room] = Chatroom(self, room, users)
			self.addTab(self.joinedrooms[room], room)
		else:
			self.joinedrooms[room].Rejoined(users)
		
	def LeaveRoom(self, room):
		roomwidget = self.joinedrooms[room]
		self.removeTab(self.frame.ui.ChatRooms.indexOf(roomwidget))
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
		self.ui.UserList.setHeaderLabels(("", "User", "Speed", "Files"))
		self.ui.UserList.setIndentation(0)
		self.ui.UserList.setColumnWidth(0, 20)
		self.ui.UserList.setColumnWidth(1, 100)
		self.ui.UserList.setColumnWidth(2, 100)
		self.ui.UserList.setColumnWidth(3, 50)
		self.ui.UserList.setColumnCount(4);
		
		self.ui.UserList.sortItems(1, Qt.AscendingOrder);
		self.tag_remote = self.frame.makecolour(self.ui.UserList, "chatremote")
		self.tag_local = self.frame.makecolour(self.ui.UserList, "chatlocal")
		self.tag_me = self.frame.makecolour(self.ui.UserList, "chatme")
		self.tag_hilite = self.frame.makecolour(self.ui.UserList, "chathilite")
		
		self.users = {}
		for username,stats in users.items():
			self.users[username] = QTreeWidgetItem(self.ui.UserList.invisibleRootItem())
			
			status, speed, downloads, files, dirs = self.frame.user_stats[username] = [stats[0], stats[1], stats[2], stats[3], stats[4]]
			hspeed = self.frame.HumanSpeed(speed)
			hfiles = self.frame.Humanize(files)
			#print self.frame.user_stats[user]
			self.users[username].setIcon(0, self.frame.GetStatusImage(username, status))
			self.users[username].setText(1, username)
			self.users[username].setText(2, hspeed)
			self.users[username].setTextAlignment(2, Qt.AlignRight)
			self.users[username].setText(3, hfiles)
			self.users[username].setTextAlignment(3, Qt.AlignRight)
			self.users[username].setText(4, str(status))
			self.users[username].setData(5, 0, QVariant(speed))
			self.users[username].setData(6, 0, QVariant(files))
			#newuser.setText(0, status)
		self.connect(self.ui.ChatEntry, SIGNAL("returnPressed()" ), self.SayRoom)
		self.connect(self.ui.Close, SIGNAL("clicked()"), self.LeaveRoom)
		self.tag_log = None
		self.ui.UserList.resizeColumnToContents(0)
		#self.ui.UserList.resizeColumnToContents(1)
		self.ui.UserList.resizeColumnToContents(2)
		#self.ui.Close.setIcon(self.frame.images["close"])
	def ConnClose(self):
		self.ui.UserList.clear()
		self.users.clear()
		
	def Rejoined(self, users):
		self.ui.UserList.clear()
		self.users.clear()
		for username,stats in users.items():
			self.users[username] = QTreeWidgetItem(self.ui.UserList.invisibleRootItem())
			status, speed, downloads, files, dirs = self.frame.user_stats[username] = [stats[0], stats[1], stats[2], stats[3], stats[4]]
			hspeed = self.frame.HumanSpeed(speed)
			hfiles = self.frame.Humanize(files)
			#print self.frame.user_stats[user]
			self.users[username].setIcon(0, self.frame.GetStatusImage(username, status))
			self.users[username].setText(1, username)
			self.users[username].setText(2, hspeed)
			self.users[username].setTextAlignment(2, Qt.AlignRight)
			self.users[username].setText(3, hfiles)
			self.users[username].setTextAlignment(3, Qt.AlignRight)
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
			
	
			
			
	def UserJoinedRoom(self, username):
		if self.users.has_key(username):
			return

		status, speed, downloads, files, dirs = self.frame.user_stats[username]
		AppendLine(self.ui.StatusLog, _("%s joined the room") % username) #, self.tag_log)
		img = self.frame.GetStatusImage(username, status)
		hspeed = self.frame.HumanSpeed(speed)
		hfiles = self.frame.Humanize(files)
		self.users[username] = QTreeWidgetItem(self.ui.UserList.invisibleRootItem())
		hspeed = self.frame.HumanSpeed(speed)
		hfiles = self.frame.Humanize(files)
		self.users[username].setIcon(0, self.frame.GetStatusImage(username, status))
		self.users[username].setText(1, username)
		self.users[username].setText(2, hspeed)
		self.users[username].setTextAlignment(2, Qt.AlignRight)
		self.users[username].setText(3, hfiles)
		self.users[username].setTextAlignment(3, Qt.AlignRight)
		self.users[username].setText(4, str(status))
		self.users[username].setData(5, 0, QVariant(speed))
		self.users[username].setData(6, 0, QVariant(files))
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
			
def AppendLine(logwindow, message, tag=None, timestamp = "%H:%M:%S", username=None, usertag=None, scroll=True):
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

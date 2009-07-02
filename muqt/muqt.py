#!/usr/bin/python
import sys, os, ConfigParser, time
from PyQt4 import QtCore, QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *

from mainwindow import Ui_MainWindow
from chatroom import Ui_Room

from networking import Networking

config_dir = str(os.path.expanduser("~/.muqt/"))
log_dir = str(os.path.expanduser("~/.muqt/logs/"))
config_file = config_dir+"config"
DEBUG = 1
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
		self.ChatRooms = ChatRooms(self)
		self.Networking = Networking(self)
		
		self.connect(self.Networking, SIGNAL("JoinRoom(PyQt_PyObject, PyQt_PyObject)"), self.ChatRooms.JoinRoom)
		self.connect(self.Networking, SIGNAL("SayChatRoom(PyQt_PyObject,PyQt_PyObject, PyQt_PyObject)"), self.ChatRooms.SayChatRoom)
		#self.connect(self.Networking, SIGNAL("SayChatRoom(PyQt_PyObject, PyQt_PyObject, PyQt_PyObject)"), self.PrivateChat.SayPrivate)
		self.connect(self.Networking,  SIGNAL("Log(PyQt_PyObject)"), self.AppendToLogWindow)
		self.connect(self.Networking, SIGNAL("PrivateMessage(PyQt_PyObject,PyQt_PyObject, PyQt_PyObject)"), self.Networking.PrivateMessage)
		self.connect(self.ui.actionAbout_Qt, SIGNAL("activated()"), self.OnAbout)
		self.Networking.start()
	def OnAbout(self):
		QMessageBox.aboutQt(self)
	def AppendToLogWindow(self, message):
		print "M", message
		message = "%s %s" % (recode(time.strftime("%H:%M:%S")), message)
		self.ui.logWindow.append(message)

class ChatRooms:
	def __init__(self, parent=None):
		self.frame=parent
		self.joinedrooms = {}

		
	def JoinRoom(self, room, users):
		#print room, users
		#print self.frame.ui.ChatRooms
		#print Chatroom
		self.joinedrooms[room] = Chatroom(self, room, users)
		
		self.frame.ui.ChatRooms.addTab(self.joinedrooms[room], room)
		
	def SayChatRoom(self, room, user, text):
		if self.frame.Networking.config.has_key("ignored") and user in self.frame.Networking.config["ignored"].keys():
			return
		if room in self.joinedrooms:
			self.joinedrooms[room].SayInChatRoom(user, text)
	
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
		self.ui.UserList.setColumnCount(4);
		self.users = {}
		for user,stats in users.items():
			self.users[user] = QTreeWidgetItem(self.ui.UserList)
			self.users[user].setText(1, user)
			status, speed, downloads, files, dirs = self.frame.user_stats[user] = [stats[0], stats[1], stats[2], stats[3], stats[4]]
			#print self.frame.user_stats[user]
			self.users[user].setText(0,  str(status))
			self.users[user].setData(2, 0, QVariant(speed))
			self.users[user].setData(3, 0, QVariant(files))
			#newuser.setText(0, status)
		self.connect(self.ui.ChatEntry, SIGNAL("returnPressed()" ), self.SayRoom)
		
	def SayRoom(self):
		self.frame.Networking.SayRoom(self.room, str(self.ui.ChatEntry.text()))
		self.ui.ChatEntry.clear()
		
	def GetUserStatus(self, user, status):
		if not self.users.has_key(user):
			return

                if status != self.usersmodel.get_value(self.users[user], 4):
                        img = self.frame.GetStatusImage(user, status)
			if status == 1:
				action = _("%s has gone away")
			else:
				action = _("%s has returned")
			AppendLine(self.RoomLog, action % user, self.tag_log)
			if user in self.tag_users.keys():
				color = self.getUserStatusColor(status)
				self.changecolour(self.tag_users[user], color)
			self.usersmodel.set(self.users[user], 0, img, 4, status)
			
	def SayInChatRoom(self, user, line):

		#if len(self.lines) >= 400:
			#buffer = self.chatview.get_buffer()
			#start = buffer.get_start_iter()
			#end = buffer.get_iter_at_line(self.lines[200])
			#self.chatview.get_buffer().delete(start, end)
			#del self.lines[0:200]
		
		# Display /me messages as "* username message"
		if line[:4] == "/me ":
			message = "* %s %s"  % (user, line[4:])
			#tag = self.tag_me
		else:
			message = "[%s] %s" % (user, line)
			#if user == self.frame.username:
				#tag = self.tag_local
			#elif line.upper().find(self.frame.username.upper()) > -1:
				#tag = self.tag_hilite
			#else:
				#tag = self.tag_remote

		message = "\n-- ".join(message.split("\n"))
		message = "%s %s" % (recode(time.strftime("%H:%M:%S")), message)
		#if user in self.tag_users.keys():
			#usertag = self.tag_users[user]
		#else:
			#usertag = self.tag_users[user] = self.makecolour(self.chatview.get_buffer(), color, username=user)
		#self.lines.append(AppendLine(self.chatview, message, tag=tag, username=user, usertag=usertag))
		AppendLine(self.ui.ChatLog, message)
		#if self.frame.username is not None:
			#if user != self.frame.username and self.frame.username in message:
				#self.chatrooms.request_hilite(self.Main)
			#else:
				#self.chatrooms.request_changed(self.Main)
			#self.frame.RequestIcon(self.frame.ChatRoomLabel)
			
def AppendLine(logwindow, message):
	message = "\n-- ".join(message.split("\n"))
	message = "%s %s" % (recode(time.strftime("%H:%M:%S")), message)
	logwindow.append(message)
	
if __name__ == "__main__":
	app = QtGui.QApplication(sys.argv)
	muqt = MuQT()
	muqt.show()
	sys.exit(app.exec_())

#! /usr/bin/env python
#-*- coding: utf-8 -*-
# Murmur - a PyGTK2 client for museek 
# Code fragments taken from mucous, musetup-gtk, and nicotine
#
# daelstorm (C) 2005-2007
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
import sys

def Output(*args):

	message = ""
	for arg in args:
		message += str(arg) + " "
	print message
	
win32 = sys.platform.startswith("win")
if win32:
     # Fetchs gtk2 path from registry
     try: import dbhash
     except: Output( _("Warning: the Berkeley DB module, dbhash, could not be loaded."))
     import _winreg
     import msvcrt
     try:
       k = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, "Software\\GTK\\2.0")
     except EnvironmentError:
       Output( _("You must install the Gtk+ 2.2 Runtime Environment to run this program"))
       while not msvcrt.kbhit():
        pass
       sys.exit(1)
     else:    
      gtkdir = _winreg.QueryValueEx(k, "Path")
      import os
      os.environ['PATH'] += ";%s/lib;%s/bin" % (gtkdir[0], gtkdir[0])
#import gtk,
if win32:
	try:
		import gtk
	except:
		import pygtk
else:
	import pygtk
	pygtk.require("2.0")
import gtk
import gobject
import sys, os
import threading
import traceback
from threading import Thread
ENCHANT=True
try:
	import enchant
except ImportError:
	ENCHANT=False
SEXY=True
try:
	import sexy
except ImportError:
	SEXY=False
	Output( "Note: Python Bindings for libsexy were not found. To enable SpellChecking, install them from http://www.chipx86.com/wiki/Libsexy")
import gc
#gc.set_debug(gc.DEBUG_LEAK)
try:
	from Crypto.Hash import SHA256
	
except ImportError:	
	
	try:
		import mucipher
	except ImportError:
		Output( "WARNING: The Mucipher Module for Python wasn't found and neither was PyCrypto. One of these encryption modules is necessary to allow Murmur to connect to the Museek Daemon.\nDownload mucipher here: http://thegraveyard.org/files/pymucipher-0.0.1.tar.gz\nExtract the tarball, and as Root or sudo, run:\npython setup.py install\nYou'll need GCC, Python and SWIG.\nOr download PyCrypto from here: http://www.amk.ca/python/code/crypto.html")
		sys.exit()
try:
	import messages, driver
except ImportError, error:
	
	try:
		from museek import messages, driver
	except ImportError, error:
		Output("WARNING: The Museek Message-Parsing modules, messages.py and/or driver.py  were not found. Please install them into your '/usr/lib/python2.X/site-packages/museek' directory, or place them in a 'museek' subdirectory of the directory that contains the murmur python scipt.", error)
		sys.exit()

trayicon_load = 1

if gtk.pygtk_version[0] >= 2 and gtk.pygtk_version[1] >= 10:
	pass
else:
	try:
		from pymurmur import trayicon
	except:
		Output("Optional TrayIcon module not found. Please build it if you want the TrayIcon to work.")
		trayicon_load = 0
try:
	from pymurmur import imagedata
	from pymurmur.settings import Settings
except ImportError, e:
	Output("Failed loading imagedata", e)
	os._exit(1)
try:
	from pymurmur.utils import *
	from pymurmur.utils import _
	from pymurmur import utils
except ImportError, e:
	Output("Failed loading pymurmur utils", e)
	os._exit(1)
from pymurmur.muscan import Muscan
	
from pymurmur.entrydialog import *

from time import sleep

import select, string, re, ConfigParser, getopt, shutil, commands

import threading, time
import signal
import tempfile
import imghdr

	
pid = os.getpid()

geoip_fail=0
try:
	import GeoIP
	gi = GeoIP.new(GeoIP.GEOIP_MEMORY_CACHE)
except ImportError:
	Output("Python module geoip could not be loaded.")
	geoip_fail=1

config_dir = str(os.path.expanduser("~/.murmur/"))
log_dir = str(os.path.expanduser("~/.murmur/logs/"))
config_file = config_dir+"config"

DEBUG = False

def usage():
	Output("""Murmur is a PyGTK2 client for Museek, the P2P Soulseek Daemon
Dir: Daelstorm
Credit: Hyriand 
Version: %s
	Default options: none
	-c,	--config <file>	Use a different config file
	-l,	--log <dir>	Use a different logging directory
	-v,	--version	Display version and quit
	-d				Debug mode

	-h,	--help		Display this help and exit
	""" %version)
	sys.exit(2)
	
try:
	opts, args = getopt.getopt(sys.argv[1:], "hc:vl:d", ["help", "config=", "version", "log="])
except getopt.GetoptError:
	usage()
	sys.exit(2)
for opts, args in opts:
	if opts in ("-h", "--help"):
		usage()
		sys.exit()
	if opts in ("-c", "--config"):
		config_file=str(os.path.expanduser(args))
	if opts in ("-l", "--log"):
		log_dir=str(os.path.expanduser(args))
	if opts in ("-d"):
		DEBUG = True
	if opts in ("-v", "--version"):
		Output("Murmur version: %s" % version)
		sys.exit(2)

#parser = ConfigParser.ConfigParser()


## Modify and read the Murmur Config
#
class ConfigManager:
	## Constructor
	# @param self ConfigManager
	# @param murmur Murmur (Class)
	def __init__(self, murmur):
		## @var parser
		# ConfigParser instance
		
		self.parser = ConfigParser.ConfigParser()
		## @var murmur
		# Murmur (class)
		self.murmur = murmur

		self.murmur.Config = {\
"connection":{"interface": 'localhost:2240', "passw":None}, \
"museekd": {"configfile": os.path.expanduser("~/.museekd/config.xml")}, \
\
"murmur":{"trayapp": True, "tooltips": False, "autobuddy": False, "rooms_sort": "size",
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
"logging": { "logcollapsed": False, "log_dir":  os.path.expanduser("~/.murmur/logs/"),
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
	
		murmur_config_file = file(config_file, 'w')
		
		for i in self.murmur.Config.keys():
			if not self.parser.has_section(i):
				self.parser.add_section(i)
			for j in self.murmur.Config[i].keys():
				if j not in ["nonexisting", "hated", "options"]:
					self.parser.set(i,j, self.murmur.Config[i][j])
				else:
					self.parser.remove_option(i,j)
		self.parser.write(murmur_config_file)
		murmur_config_file.close()	
	
	## Create config file and parse options
	# @param self ConfigManager
	def read_config(self):
		
		self.parser.read([config_file])
		for i in self.parser.sections():
			for j in self.parser.options(i):
				val = self.parser.get(i,j, raw = 1)
	
				if j in ['login','passw','interface', 'trayapp',  'tickers_enabled', "ticker_cycle",  "autobuddy",  "now-playing", "log_dir", "configfile",  "urlcatching",  "humanizeurls", "autobuddy" ] or i == "ui" and j not in [ "usernamehotspots", "width", "height"] and val != "None":
					self.murmur.Config[i][j] = val
				elif i == 'aliases' and val != "None":
					self.murmur.Config[i][j] = val
				else:
					try:
						self.murmur.Config[i][j] = eval(val, {})
					except:
						try:
							self.murmur.Config[i][j] = None
						except:
							pass
	
	## Write config file to disk
	# @param self ConfigManager
	def update_config(self):
		murmur_config_file = file(config_file, 'w')
		for i in self.murmur.Config.keys():
			if not self.parser.has_section(i):
				self.parser.add_section(i)
			for j in self.murmur.Config[i].keys():
				if j not in ["somethingwrong"]:
					
					self.parser.set(i,j, self.murmur.Config[i][j])
				else:
					self.parser.remove_option(i,j)
		self.parser.write(murmur_config_file)
		murmur_config_file.close()
	
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
			
		
class BuddiesComboBoxEntry(gtk.ComboBoxEntry):
	def __init__(self, frame):
		self.frame = frame
		gtk.ComboBoxEntry.__init__(self)
		self.items = {}
		self.store = gtk.ListStore(gobject.TYPE_STRING)
		self.set_model(self.store)
		self.set_text_column(0)
		self.store.set_default_sort_func(lambda *args: -1) 
  		self.store.set_sort_column_id(-1, gtk.SORT_ASCENDING)
        	self.show()
		
	def Fill(self):
		self.items.clear()
		self.store.clear()
		self.items[""] = self.store.append([""])
		if not self.frame.Networking.config.has_key("buddies"):
			return
		for user in self.frame.Networking.config["buddies"].keys():
			self.items[user] = self.store.append([user])
		self.store.set_sort_column_id(0, gtk.SORT_ASCENDING)	
	def Append(self, item):
		if self.items.has_key(item):
			return
        	self.items[item] = self.get_model().append([item])
		
	def Remove(self, item):
		if self.items.has_key(item):
			self.get_model().remove(self.items[item] )
			del self.items[item]
		
	
class ImageDialog( gtk.Dialog):
	def __init__(self, Mapp, title="Image", message="", picture=None, modal= True):
		gtk.Dialog.__init__(self)
		self.connect("destroy", self.quit)
		self.connect("delete_event", self.quit)
		if modal:
			self.set_modal(True)
 	
		box = gtk.VBox(spacing=10)
		box.set_border_width(10)
		self.vbox.pack_start(box)
		box.show()
		if message:
			label = gtk.Label()
			label.set_markup(message)
			label.set_line_wrap(True)
			box.pack_start(label)
			label.show()
		if picture is not None and picture != "":
			#pixbufanim2 = gtk.gdk.PixbufAnimation("%s"% picture)
			image1 = gtk.Image()
			image1.set_from_pixbuf(picture)
			image1.show()
			box.pack_start(image1)
		button = gtk.Button("Okay", stock="gtk-ok")
		button.connect("clicked", self.click)
		button.set_flags(gtk.CAN_DEFAULT)
		self.action_area.pack_start(button)
		button.show()
		button.grab_default()
		self.ret = None

	def quit(self, w=None, event=None):
		self.hide()
		self.destroy()
		gtk.main_quit()

	def click(self, button):
		self.ret = None
		self.quit()



class Networking(driver.Driver, Thread):
	def __init__(self, xapp):
		driver.Driver.__init__(self)
		Thread.__init__(self)
		self.frame = self.app=xapp
		self.config = {}
		self.socket = None
		self.timer = None
		self.connected = False
		
	def connect_to_museekd(self, string):

	
		try:
			if self.socket is not None:
				return
			if self.frame.Config["connection"]["passw"] != None:
				if self.timer is not None: self.timer.cancel()
				self.timer = threading.Timer(10.0, self.CheckNick)
				self.timer.start()

				self.connect(self.frame.Config["connection"]["interface"],  self.frame.Config["connection"]["passw"], messages.EM_CHAT |  messages.EM_USERINFO| messages.EM_PRIVATE | messages.EM_TRANSFERS  | messages.EM_USERSHARES | messages.EM_CONFIG |  messages.EM_INTERESTS|  messages.EM_DEBUG)
					
				if DEBUG:
					Output("connect attempt")

			else:
				raise Exception,  "NOPASS"


		except Exception, e:
			self.timer.cancel()

			
			#if DEBUG:
				#if "INVPASS" in e:
					#message = "Incorrect Password for the museek daemon. Please try another."
					#self.SetToolTip( "Couldn't connect to Museek: %s" % "Invalid Password")
			if  "NOPASS" in e:
				message = "Set a Password in Murmur Settings."
				#self.SetToolTip( "Couldn't connect to Museek: %s" % "Set a Password")
			elif  "Connection refused" in e:
				message = e[1] +", make sure the daemon is running, or change the interface."
				#self.SetToolTip( "Make sure Museek is running, or try a different Interface.")
			Output("Connect to museekd Error: ", e)
			gobject.idle_add(self.frame.AppendToLogWindow, message)
			
			self.connected = False
			
	def cb_login_error(self, reason):
		#gtk.gdk.threads_enter()
		try:
			if reason == "INVPASS":
				message = "Couldn't connect to Museek: %s" % "Invalid Password"

			else:
				message = "Couldn't Login to Museek: %s" % reason
				
			gobject.idle_add(self.frame.TrayApp.SetToolTip, message)
			gobject.idle_add(self.frame.AppendToLogWindow, message)	
			gobject.idle_add(self.frame.TrayApp.SetImage,"red")
			if self.timer is not None: self.timer.cancel()
		except Exception,e:
			if DEBUG: Output("RECIEVED: cb_login_error ERROR", e)
		self.connected = False
		#gtk.gdk.threads_leave()

	def cb_login_ok(self):
		
		try:
			message = "Logging in to Museek..."
			gobject.idle_add(self.frame.AppendToLogWindow, message)	
		except Exception,e:
			if DEBUG: Output("RECIEVED: cb_login_ok ERROR", e)
		self.connected = True
		
	def disconnect(self, string=""):
		
		try:
			if self.socket != None:
				#self.timer.cancel()
				#driver.Driver.close(self)
				self.close()
				self.connected = False
				self.config = {}
				self.frame.downloads.Clear()
				self.frame.uploads.Clear()
			if self.timer is not None: self.timer.cancel()
			
		except Exception,e:
			if DEBUG: Output("disconnect ERROR", e)
		
	def cb_disconnected(self):
		gtk.gdk.threads_enter()
		if DEBUG: Output("RECIEVED: cb_disconnected")
		try:
			
			gobject.idle_add(self.frame.TrayApp.SetImage,"red")

			gobject.idle_add(self.frame.TrayApp.SetToolTip, "Disconnected from Museek")
			
			gobject.idle_add(self.frame.update_statusbar, "Disconnected from Museek")
			
			self.frame.username = None
			self.frame.status = 2
			self.config = {}

			self.frame.ChatRooms.ConnClose()

			self.connected = False
			if self.timer is not None: self.timer.cancel()
		except Exception, e:
			if DEBUG: Output("RECIEVED: cb_disconnected ERROR", e)
		gtk.gdk.threads_leave()
		
	def CheckNick(self):
		try:

			if self.frame.username != None:
				# We have a username, so we probably logged in successfully
				return
			message = "Connection is taking a while to start, maybe you are trying to connect to something besides a museek daemon, or you museek daemon is not started? Closing socket to %s.." % self.frame.Config["connection"]["interface"]
			if DEBUG:
				
				Output(message)
			gobject.idle_add(self.frame.AppendToLogWindow, message)
			if self.socket is not None:
				self.close()
			self.timer.cancel()
			
		except Exception,e:
			if DEBUG: Output("CheckNick ERROR", e)

			
	## Ping
	def cb_ping(self):
		if DEBUG: Output("RECIEVED: cb_ping")
		pass

	def cb_server_privileges(self, time_left):
		if DEBUG: Output("RECIEVED: cb_server_privileges: %i" % time_left)
		
	def cb_config_set(self, domain, key, value):
		gtk.gdk.threads_enter()
		try:
			if self.config.has_key(domain) and key in self.config[domain].keys():
				if DEBUG: Output("Modified <"+key+"> in <" +domain+"> to <"+value + ">")
				self.config[domain][key] = value
				if domain == "banned":
					self.frame.userlists.banned.update(key, value)
				elif domain == "buddies":
					self.frame.userlists.buddies.update(key, value)
				elif domain == "ignored":
					self.frame.userlists.ignored.update(key, value)
				elif domain == "trusted":
					self.frame.userlists.trusted.update(key, value)
			
				
					
			else:
				if domain not in self.config:
					if DEBUG: Output("Created domain <" +domain+">")
					self.config[domain] = {}
				if value == '':
					if DEBUG: Output("Added <"+key+"> to <" +domain+">")
				else:
					if DEBUG: Output("Added <"+key+"> to <" +domain+"> and set to <"+value+">")
				
				self.config[domain][key] = value
				if domain == "banned":
					self.frame.userlists.banned.append(key, value)
				elif domain in "buddies":
					self.frame.userlists.buddies.append(key, value)
				elif domain in "ignored":
					self.frame.userlists.ignored.append(key, value)
				elif domain in "trusted":
					self.frame.userlists.trusted.append(key, value)
			if domain in ("banned", "buddies", "ignored", "trusted"):
				# Update lists
				self.frame.ChatRooms.GetListStatus(key)
				self.frame.userlists.updateListIcon(key)
				if domain == "buddies":
					for widget in self.frame.BuddiesComboEntries:
						gobject.idle_add(widget.Append, key)
					
			elif domain == "interests.hate":
				self.frame.Recommendations.AddedHatedInterest(key)
			elif domain == "interests.like":
				self.frame.Recommendations.AddedLikedInterest(key)
			elif domain == "autojoin":
				self.frame.ChatRooms.AutoJoin(key, True)
		except Exception,e:
			if DEBUG: Output("RECIEVED: cb_config_set ERROR", e)

		gtk.gdk.threads_leave()

	## Delete keys from self.config
	def cb_config_remove(self, domain, key):
		gtk.gdk.threads_enter()
		
		if key in self.config[domain].keys():
			if DEBUG: Output("Removed <"+key+"> from <" +domain+">")
			if domain == "banned":
				self.frame.userlists.banned.remove(key)
			elif domain in "buddies":
				self.frame.userlists.buddies.remove(key)
			elif domain in "ignored":
				self.frame.userlists.ignored.remove(key)
			elif domain in "trusted":
				self.frame.userlists.trusted.remove(key)
			elif domain == "interests.hate":
				self.frame.Recommendations.RemovedHatedInterest(key)
			elif domain == "interests.like":
				self.frame.Recommendations.RemovedLikedInterest(key)
			elif domain == "autojoin":
				self.frame.ChatRooms.AutoJoin(key, False)
			if domain in ("banned", "buddies", "ignored", "trusted"):
				gobject.idle_add(self.frame.ChatRooms.GetListStatus, key)
				gobject.idle_add(self.frame.userlists.updateListIcon, key)
				if domain == "buddies":
					for widget in self.frame.BuddiesComboEntries:
						gobject.idle_add(widget.Remove, key)
			del self.config[domain][key]
		gtk.gdk.threads_leave()
		#self.display_config_update(domain)
	
	## Copy config to self.config at connection
	def cb_config_state(self, config):
		gtk.gdk.threads_enter()
		
		self.config = config.copy()
		if self.config.keys() != []:
			if self.config.has_key("buddies" ):
				gobject.idle_add(self.frame.userlists.buddies.Fill, self.config["buddies"])

				for widget in self.frame.BuddiesComboEntries:
					gobject.idle_add(widget.Fill)
						
			
			if self.config.has_key("banned"):

				gobject.idle_add(self.frame.userlists.banned.Fill, self.config["banned"])
				
				for user in self.config["banned"].keys():
				
					if not self.frame.user_stats.has_key(user):
						self.PeerStats(user)
						self.PeerStatus(user)
						
			if self.config.has_key("ignored"):
				gobject.idle_add(self.frame.userlists.ignored.Fill, self.config["ignored"])
				for user in self.config["ignored"].keys():
				
					if not self.frame.user_stats.has_key(user):
						self.PeerStats(user)
						self.PeerStatus(user)
					
	
			if self.config.has_key("trusted"):
				gobject.idle_add(self.frame.userlists.trusted.Fill, self.config["trusted"])

			if self.config.has_key("interests.like"):
				for interest in self.config["interests.like"].keys():
					self.frame.Recommendations.AddedLikedInterest(interest)
			if self.config.has_key("interests.hate"):
				for interest in self.config["interests.hate"].keys():
					self.frame.Recommendations.AddedHatedInterest(interest)
		self.frame.Muscan.GetConfig()
		gtk.gdk.threads_leave()
				
	## Add new/replace old keys to self.config
	def mod_config(self, changetype, username, value=''):
		#Output(changetype, username, value
		if changetype == "buddy":
			if not self.config.has_key("banned") or username not in self.config["buddies"].keys():
				self.ConfigSet("buddies", username, value)
				if not self.frame.user_stats.has_key(username):
					self.PeerExists(username)
		elif changetype == "unbuddy":
			if self.config.has_key("buddies") and username in self.config["buddies"].keys():
				self.ConfigRemove("buddies", username)
		
		elif changetype == "trust":
			if not self.config.has_key("trusted") or username not in self.config["trusted"].keys():
				self.ConfigSet("trusted", username, value)
				if not self.frame.user_stats.has_key(username):
					self.PeerExists(username)
		elif changetype == "distrust":
			if self.config.has_key("trusted") and username in self.config["trusted"].keys():
				self.ConfigRemove("trusted", username)
	
		elif changetype == "ban":
			if not self.config.has_key("banned") or username not in self.config["banned"].keys():
				self.ConfigSet("banned", username, value)
				if not self.frame.user_stats.has_key(username):
					self.PeerExists(username)
		elif changetype == "unban":
			if self.config.has_key("buddies") and username in self.config["buddies"].keys():
				self.ConfigRemove("banned", username)
		
		elif changetype == "ignore":
			if not self.config.has_key("ignored") or username not in self.config["ignored"].keys():
				self.ConfigSet("ignored", username, value)
	
				if not self.frame.user_stats.has_key(username):
					self.PeerExists(username)
		elif changetype == "unignore":
			if self.config.has_key("ignored") and username in self.config["ignored"].keys():
				self.ConfigRemove("ignored", username)
			
		elif changetype == "autojoin":
			room = username
			if not self.config.has_key("autojoin") or room not in self.config["autojoin"].keys():
				self.ConfigSet("autojoin", room, "")
		
		elif changetype == "unautojoin":
			room = username
			if self.config.has_key("autojoin") and room in self.config["autojoin"].keys():
				self.ConfigRemove("autojoin", room)
		
	def Send(self, message):
		if self.frame.Networking.connected == False:
			return
		try:
			self.send( message ) 
		except Exception, e:
			if DEBUG: Output("Sending message failed", message, e)
			
	## Got Global Recommendations
	# @param self Networking (Driver Class)
	# @param recommendations list of recommendations [item, number of recommends] 
	def cb_get_global_recommendations(self, recommendations):
		try:
		
			gobject.idle_add(self.frame.Recommendations.UpdateRecommendations, recommendations)
	
		except Exception, e:
			Output("CB Get Global Recommendations" + str(e))
	
	## Got Similar Users list
	# @param self Networking (Driver Class)
	# @param users List of format [username, status=(0,1,2)]
	def cb_get_similar_users(self, users):
		try:
			
			
			gobject.idle_add(self.frame.Recommendations.UpdateSimilarUsers, users)
		
		except Exception, e:
			Output("CB Similar Users" + str(e))
			self.frame.PrintTraceBack()
	
	## Got Personal Recommendations
	# @param self Networking (Driver Class)
	# @param recommendations list of recommendations [item, number of recommends]
	def cb_get_recommendations(self, recommendations):
		try:
			
			gobject.idle_add(self.frame.Recommendations.UpdateRecommendations, recommendations)
		
		except Exception, e:
			Output("CB Get  Recommendations" + str(e))
	
	## Got Similar Users list for an Item
	# @param self Networking (Driver Class)
	# @param item string
	# @param users List of format [username, status=(0,1,2)]
	def cb_get_item_similar_users(self, item, users):
		try:
			
			gobject.idle_add(self.frame.Recommendations.UpdateSimilarUsers, users)
			
		except Exception, e:
			Output("CB Item Similar Users" + str(e))
	
	## Got Recommendations for an Item
	# @param self Networking (Driver Class)
	# @param item string
	# @param recommendations list of recommendations [item, number of recommends]
	def cb_get_item_recommendations(self, item, recommendations):
		try:
			gobject.idle_add(self.frame.Recommendations.UpdateRecommendations, recommendations)

		except Exception, e:
			Output("CB Get Item Recommendations" + str(e))

        def cb_room_list(self, roomlist):
		#gtk.gdk.threads_enter()

		if DEBUG: Output("RECIEVED: cb_room_list", len(roomlist))
	
		gobject.idle_add(self.frame.ChatRooms.UpdateRoomList, roomlist)
		#gtk.gdk.threads_leave()

	def cb_room_tickers(self, room, tickers):
		if DEBUG: Output("RECIEVED: cb_room_ticker")
		gtk.gdk.threads_enter()
		gtk.gdk.threads_leave()

	def cb_room_ticker_set(self, room, user, message):
		gtk.gdk.threads_enter()
		if DEBUG: Output("RECIEVED: cb_room_ticker_set")
		gtk.gdk.threads_leave()

	def cb_search_ticket(self, query, ticket):
		gtk.gdk.threads_enter()
		
		if DEBUG: Output("RECIEVED: cb_search_ticket", query, ticket)
		gobject.idle_add(self.frame.Searches.NewSearch, query, ticket)
		
		gtk.gdk.threads_leave()


	def cb_search_results(self, ticket, user, free, speed, queue, results):
		gtk.gdk.threads_enter()
		if DEBUG: Output("RECIEVED: cb_search_results")

		gobject.idle_add(self.frame.Searches.NewResults,ticket, user, free, speed, queue, results)
		gtk.gdk.threads_leave()



	def cb_transfer_state(self, downloads, uploads):
		#gtk.gdk.threads_enter()
		if DEBUG: Output("RECIEVED: cb_transfer_state")
		try:
			for transfer in uploads:
				gobject.idle_add(self.frame.uploads.append, transfer.user, None, transfer.rate, transfer.state, transfer.filepos, transfer.filesize, transfer.path, transfer.error, transfer.place)
			for transfer in downloads:
				gobject.idle_add(self.frame.downloads.append, transfer.user, None, transfer.rate, transfer.state, transfer.filepos, transfer.filesize, transfer.path, transfer.error, transfer.place)
		except Exception, e:
			Output(e)
		#gtk.gdk.threads_leave()


	def cb_transfer_update(self, transfer):
		#gtk.gdk.threads_enter()
		try:
			if DEBUG: Output("RECIEVED: cb_transfer_update", transfer.user,  transfer.path)
			if transfer.is_upload:
				gobject.idle_add(self.frame.uploads.update , transfer.user, None, transfer.rate, transfer.state, transfer.filepos, transfer.filesize, transfer.path, transfer.error, transfer.place)
			else:
				gobject.idle_add(self.frame.downloads.update, transfer.user, None, transfer.rate, transfer.state, transfer.filepos, transfer.filesize, transfer.path, transfer.error, transfer.place)
		except Exception,e:
			if DEBUG: Output("RECIEVED: cb_transfer_update ERROR", e)
		#gtk.gdk.threads_leave()
			
	def cb_transfer_remove(self, transfer):
		gtk.gdk.threads_enter()
		
		if DEBUG: Output("RECIEVED: cb_transfer_remove")
		#user_path = transfer[1], transfer[2]
		if transfer[0]:
			gobject.idle_add(self.frame.uploads.remove, transfer)
		else:
			gobject.idle_add(self.frame.downloads.remove, transfer)
		gtk.gdk.threads_leave()
		
	def cb_user_shares(self, user, shares):
		gtk.gdk.threads_enter()
		if DEBUG: Output("RECIEVED: cb_user_shares")
		#self.frame.get_shares(user, shares)
		try:
			gobject.idle_add(self.frame.userbrowses.GotUserBrowse, user, shares)
		except Exception,e:
			if DEBUG: Output("RECIEVED: cb_user_shares ERROR", e)
		gtk.gdk.threads_leave()
			
	def cb_user_info(self, user, info, picture, uploads, queue, slotsfree):
		gtk.gdk.threads_enter()
		try:
			if DEBUG:
				Output("got %s's userinfo" % user)

			gobject.idle_add(self.frame.userinfos.GotUserInfo, user, info, picture, uploads, queue, slotsfree)
	
		except Exception,e:
			if DEBUG: Output("RECIEVED: cb_user_info ERROR", e)
		gtk.gdk.threads_leave()
		
	def cb_room_said(self, room, user, text):
		gtk.gdk.threads_enter()
		try:
			gobject.idle_add(self.frame.ChatRooms.SayChatRoom, room, user, text)
			gobject.idle_add(self.frame.Logging.ChatRoomLog, room, user, text)
		except Exception,e:
			if DEBUG: Output("RECIEVED: cb_room_said ERROR", e)
		gtk.gdk.threads_leave()
			
	def cb_room_state(self, roomlist, joined, tickers):
		gtk.gdk.threads_enter()
		if DEBUG: Output("Logged in, getting Rooms")
		try:
		
			gobject.idle_add(self.frame.ChatRooms.UpdateRoomList, roomlist)
			joined_rooms = list(joined.keys())
			joined_rooms.sort(key=str.lower)
			for room in joined_rooms:
#				for users, stats in joined[room].items():
					#Output(stats)
#					self.frame.user_stats[users] = [stats[0], stats[1], stats[2], stats[3], stats[4]]
				gobject.idle_add(self.frame.ChatRooms.JoinRoom, str(room), joined[room])

		except Exception, e:
			if DEBUG: Output("Room State bug", e)
			self.frame.PrintTraceBack()

		gtk.gdk.threads_leave()
		
	def cb_room_joined(self, room, list_of_users, private, owner, operators):
		gtk.gdk.threads_enter()
		if DEBUG:
			Output("Joined room: %s" % room)
		try:
			
	
			gobject.idle_add(self.frame.ChatRooms.JoinRoom, room, list_of_users)
				
				
		except Exception, e:
			if DEBUG: Output("Join room bug", e)
		gtk.gdk.threads_leave()
		
	def cb_room_left(self, room):
		if DEBUG: Output("Leave room", room)
		gtk.gdk.threads_enter()
		if DEBUG: Output("RECIEVED: cb_room_left %s"% room)
		try:
			gobject.idle_add(self.frame.ChatRooms.LeaveRoom, room)
		except Exception, e:
			if DEBUG: Output("Leave room bug", e)
		gtk.gdk.threads_leave()
			
	def cb_room_user_joined(self, room, user, stats):
		#if DEBUG: Output("User Joined room",user)
		gtk.gdk.threads_enter()
		try:

			#self.frame.rooms[room][user] = stats
			self.frame.user_stats[user] = [stats[0], stats[1],stats[2],stats[3],stats[4]]
			self.frame.user_exists[user] = stats[5]
			
			gobject.idle_add(self.frame.ChatRooms.UserJoinedRoom, room, user)
			
		except Exception, e:
			if DEBUG: Output("User Joined room bug", e)
		gtk.gdk.threads_leave()

	
	def cb_room_user_left(self, room, user):
		#if DEBUG: Output("User Left room",user)
		gtk.gdk.threads_enter()
		try:

			gobject.idle_add(self.frame.ChatRooms.UserLeftRoom, room, user)

		except Exception, e:
			if DEBUG: Output("User Left room bug", e)
		gtk.gdk.threads_leave()

	def cb_private_message(self, direction, timestamp, user, message):
		
		if DEBUG: Output("RECIEVED: cb_private_message", user)
		#gtk.gdk.threads_enter()
		if DEBUG: Output(direction, timestamp, user, message)
		try:
			
			

			gobject.idle_add(self.frame.PrivateChats.ShowMessage, direction, user, message)
			
			#gobject.idle_add(self.frame.Logging.PrivateChatLog, direction, user, message)
			
			if DEBUG:
				if direction:
					user = self.frame.username
					Output("You PM'd to %s: %s" % (user, message))
				else:
					Output("%s PM'd to you: %s" % (user, message))
		except Exception,e:
			if DEBUG: Output("RECIEVED: cb_private_message ERROR", e)
		#gtk.gdk.threads_leave()
			
	def cb_peer_address(self, user, ip, port):
		
		if DEBUG: Output("RECIEVED: cb_peer_address")
		#gtk.gdk.threads_enter()
		try:
			if user in self.frame.ip_requested:
				self.frame.ip_requested.remove(user)
				if geoip_fail==0:
					country =  gi.country_name_by_addr( str(ip) )
					message="%s's IP: %s Port: %s Country: %s"  % (user, str(ip), str(port), country)
					#s= self.frame.display_box(title="IP Address", message = message  )
					
					if DEBUG: Output(message)
				else:
					if DEBUG: Output("No Geoip")
					message="%s's IP: %s Port: %s" % (user, str(ip), str(port) )
				gobject.idle_add(self.frame.AppendToLogWindow, message)
		except Exception,e:
			if DEBUG: Output("RECIEVED: cb_peer_address ERROR", e)
		#gtk.gdk.threads_leave()
			

		
	def cb_server_state(self, state, username):
		gtk.gdk.threads_enter()
		try:
			
			if state != self.frame.status:
				if DEBUG:
					Output( str(state), username)
			
				self.frame.status = state
				if self.frame.status:
					message = "Connected to Museek: %s Status: Away" % username
					
					gobject.idle_add(self.frame.TrayApp.SetToolTip, message)
					gobject.idle_add(self.frame.update_statusbar, message)
					gobject.idle_add(self.frame.TrayApp.SetImage, "yellow")
					
				elif self.frame.status  == 2:
					message = "Connected to Museek: %s Status: Online" % username
					
					gobject.idle_add(self.frame.TrayApp.SetToolTip, message )
					gobject.idle_add(self.frame.update_statusbar, message )
					gobject.idle_add(self.frame.TrayApp.SetImage, "green")
					
				else:
					gobject.idle_add(self.frame.TrayApp.SetImage, "red")

			self.frame.username = username
					
		except Exception,e:
			if DEBUG: Output("RECIEVED: cb_server_state ERROR", e)
		gtk.gdk.threads_leave()
		
	def cb_server_status_set(self, status):
		gtk.gdk.threads_enter()
		if DEBUG: Output("RECIEVED: cb_server_status_set ", status)
		try:
			
			if status != self.frame.status:
				self.frame.status = status
				if self.frame.status:
					message = "Connected to Museek: %s Status: Away" % self.frame.username
					
					gobject.idle_add(self.frame.TrayApp.SetImage,"yellow")
				else:
					message = "Connected to Museek: %s Status: Online" % self.frame.username
					
					gobject.idle_add(self.frame.TrayApp.SetImage, "green")
				gobject.idle_add(self.frame.TrayApp.SetToolTip, message ) 
				gobject.idle_add(self.frame.update_statusbar, message )
			
		except Exception,e:
			if DEBUG: Output("RECIEVED: cb_server_status_set ERROR", e)
		gtk.gdk.threads_leave()
                
	def cb_peer_exists(self, user, exists):
		#gtk.gdk.threads_enter()
		#if DEBUG: Output("%s exists? %i" % (user, exists))
		try:
			
			self.frame.user_exists[user] = exists
			if self.frame.user_stats.has_key(user):
				status = self.frame.user_stats[user][0]
				gobject.idle_add(self.frame.ChatRooms.GetUserStatus, user, status)
				gobject.idle_add(self.frame.userlists.updateListIcon, user)
				gobject.idle_add(self.frame.PrivateChats.updateStatus, user, status)
				
		except Exception,e:
			if DEBUG: Output("RECIEVED: cb_peer_exists ERROR", e)
		#gtk.gdk.threads_leave()
                
	def cb_peer_stats(self, user, speed, downloads, files, dirs, slotsfull, country):
		#if DEBUG: Output("RECIEVED: cb_peer_stats", user, speed, downloads,) files, dirs

		try:
			if user in self.frame.user_stats:
				stats = self.frame.user_stats[user]
				#if stats[1] == speed and stats[2] == downloads and stats[3] == files and stats[4] == dirs:
					#Output("same")
					#return
				self.frame.user_stats[user] = [stats[0], speed, downloads, files, dirs]
				
			else:
				self.frame.user_stats[user] = [0, speed, downloads, files, dirs]
		except Exception,e:
			if DEBUG: Output("RECIEVED: cb_peer_stats ERROR", e)
		#gtk.gdk.threads_enter()
		
		gobject.idle_add(self.frame.ChatRooms.GetUserStats, user, speed, files)
		gobject.idle_add(self.frame.userlists.updateStats, user, self.frame.user_stats[user])

		#gtk.gdk.threads_leave()
			
	def cb_peer_status(self, user, status):
		#if DEBUG: Output("RECIEVED: cb_peer_status", user, status)
		try:
			if user in self.frame.user_stats:

				self.frame.user_stats[user][0] = status
				
	
			else:
				self.frame.user_stats[user] = [status, 0, 0, 0, 0]
			
			## Update Lists with new status information
			gobject.idle_add(self.frame.ChatRooms.GetUserStatus, user, status)
			gobject.idle_add(self.frame.userlists.updateStatus, user, status)
			gobject.idle_add(self.frame.PrivateChats.updateStatus, user, status)
			
		except Exception,e:
			if DEBUG: Output("RECIEVED: cb_peer_status ERROR1", e)
			
	## Museekd sent us a special message
	#
	# @param self is the Networking class
	# @param type is a bool; False if the message relates to the Server / True for Peer
	# @param message is the message string
	def cb_status_message(self, type, message):
		#gtk.gdk.threads_enter()
		try:
			if type == 1:
				stype = "Peer"
			elif type == 0:
				stype = "Server"
			
			gobject.idle_add(self.frame.AppendToLogWindow, "%s Message: %s" % (stype, message))
			
		except Exception,e:
			Output("cb_status_message: " +str( e))
		#gtk.gdk.threads_leave()
	## Museekd sent us a debug message
	# @param self Networking (Driver Class)
	# @param domain is a string the value of which is a debug type
	# @param message is the message string
	def cb_debug_message(self, domain, message):

		gtk.gdk.threads_enter()
		try:
			if domain in ["museek.note", "museek.warn"] :
				gobject.idle_add(self.frame.AppendToLogWindow, "%s Message: %s" % (domain, message))
		except Exception,e:
			Output("cb_debug_message: " +str( e))
		gtk.gdk.threads_leave()


	## Process Socket Data
	def run(self):
		if DEBUG:  Output(self.frame.pid)
		self.connected = True
		while 1:
		#if 1:
			try:
				d = 0
				#while self.frame.Config["connection"]["passw"]  == None or self.frame.Config["connection"]["passw"] == "None":
					#sleep(0.1)
				if self.socket != None:

					r, w, x = select.select([self.socket,], [], [self.socket], d)
					
					if self.socket in r:
						#try:
							#Output(r[0])
						#except:
							#pass
						driver.Driver.process(self)
				else:
					
					if self.connected == True:
						self.connect_to_museekd("")
					
				sleep(0.001)
				
				#self.frame.MurmurWindow.show_all() 

			except Exception, e:
				if DEBUG: Output("Process Exception", Exception, e)
			except e:
				if DEBUG: Output(e)
		
	## Failsafe Sending of messages to Museekd
	# @param self Networking (class)
	# @param message messages instance
	def SendMessage(self, message):
		if self.frame.Networking.connected == False:
			return
		try:
			self.send(message)
		except Exception, e:
			if  e.args[0] == 10054:
				self.cb_disconnected()
			Output("SendMessage: " + str(e) + " " + str(message))
			
	## Abort transfer (remains in transfer list)
	# @param self Networking (class)
	# @param direction (1: upload, 0:download)
	# @param user username
	# @param path file to be transfered
	def TransferAbort(self, direction, user, path):
		## Transfer messages
		message = messages.TransferAbort(direction, user, path)
		self.SendMessage(message)
	
	## Remove transfer from transfer list
	# @param self Networking (class)
	# @param direction (1: upload, 0:download)
	# @param user username
	# @param path file to be transfered
	def TransferRemove(self, direction, user, path):
		message = messages.TransferRemove(direction, user, path)
		self.SendMessage(message)
	
	## Check place in queue
	# @param self Networking (class)
	# @param user username
	# @param path file in queue
	def TransferUpdate(self, user, path):
		message = messages.TransferUpdate(user, path)	
		self.SendMessage(message)
	
	## Download a file
	# @param self Networking (class)
	# @param user username
	# @param path file to be transfered
	def DownloadFile(self, user, path):
		message = messages.DownloadFile(user, path)
		self.SendMessage(message)
	## Download a file to a local directory
	# @param self Networking (class)
	# @param user username
	# @param path file to be transfered
	# @param directory local save directory
	def DownloadFileTo(self, user, path, directory):
		message = messages.DownloadFileTo(user, path, directory)
		self.SendMessage(message)
	## Ask user to send the contents of a directory
	# @param self Networking (class)
	# @param user username
	# @param directory Directory to recieve
	def GetFolderContents(self, user, directory):
		message = messages.GetFolderContents(user, directory)
		self.SendMessage(message)
	## Upload a file to user from path
	# @param self Networking (class)
	# @param user username
	# @param path file to be transfered	
	def UploadFile(self, user, path):
		message = messages.UploadFile(user, path) 
		self.SendMessage(message)
	## Say message in room
	# @param self Networking (class)
	# @param room Room
	# @param message text
	def SayRoom(self, room, message):
		messages.SayRoom(room, message) 
		self.SendMessage(message)
	## Leave a Room
	# @param self Networking (class)
	# @param room leave this room
	def LeaveRoom(self, room):
		message = messages.LeaveRoom(room) 
		self.SendMessage(message)
	## Get Room List
	# @param self Networking (class)
	def RoomList(self):
		message = messages.RoomList()
		self.SendMessage(message)
	## Say line in room
	# @param self Networking (class)
	# @param room A Room you are in
	# @param line message
	def SayRoom(self, room, line):
		message = messages.SayRoom(room, line) 
		self.SendMessage(message)
	## Set your ticker in room
	# @param self Networking (class)
	# @param room Room name
	# @param ticker message
	def RoomTickerSet(self, room, ticker):
		message = messages.RoomTickerSet(room, ticker)
		self.SendMessage(message)
	## Join a room
	# @param self Networking (class)
	# @param room Room name
	def JoinRoom(self, room):
		message = messages.JoinRoom(room)
		self.SendMessage(message)
	## Get a user's IP address and listen port
	# @param self Networking (class)
	# @param user Username
	def PeerAddress(self, user):
		message = messages.PeerAddress(user)
		self.SendMessage(message)
	
	## Private Chat messages
	# @param self Networking (class)
	# @param direction 1 if outgoing 0 if incoming
	# @param user username of user message is coming from or going to
	# @param line message
	def PrivateMessage(self, direction, user, line):
		message = messages.PrivateMessage(direction, user, line) 
		self.SendMessage(message)
		
	## Get a user's away status 
	# @param self Networking (class)
	# @param user username 
	def PeerStatus(self, user):
		message = messages.PeerStatus(user)
		self.SendMessage(message)
	## Does a user exist in the server's database? 
	# @param self Networking (class)
	# @param user username 
	def PeerExists(self, user):
		self.SendMessage(messages.PeerExists(user))
	## Get a user's statistics
	# @param self Networking (class)
	# @param user username
	def PeerStats(self, user):
		message = messages.PeerStats(user)
		self.SendMessage(message)
	## Get a user's userinfo
	# @param self Networking (class)
	# @param user username
	def UserInfo(self, user):
		message = messages.UserInfo(user)
		self.SendMessage(message)
	## Get a user's shares
	# @param self Networking (class)
	# @param user username
	def UserShares(self, user):
		message = messages.UserShares(user)
		self.SendMessage(message)
	## Search for a string in one of three methods
	# @param self Networking (class)
	# @param searchtype (0:Global, 1:Buddy, 2:Rooms)
	# @param query Search string
	def Search(self, searchtype, query, user=None):

		if searchtype in (0, 1, 2):
			message = messages.Search(searchtype, query )
			self.SendMessage(message)
		elif searchtype == 3 and user is not None:
			self.UserSearch(user, query)
			
	## Search a user's shares for a string
	# @param self Networking (class)
	# @param user Username
	# @param query Search string
	def UserSearch(self, user, query):
		message = messages.UserSearch(user, query )
		self.SendMessage(message)
	## Search via the Wishlist
	# @param self Networking (class)
	# @param query Search Query
	def WishListSearch(self, query):
		message = messages.WishListSearch(query )
		self.SendMessage(message)
	## Get a list of users with your interests
	# @param self Networking (class)
	def GetSimilarUsers(self):
		message = messages.GetSimilarUsers()
		self.SendMessage(message)
	## Get a list of recommendations related to your interests
	# @param self Networking (class)
	def GetRecommendations(self):
		message = messages.GetRecommendations()
		self.SendMessage(message)
	## Get a list of recommendations related based on popularity
	# @param self Networking (class)
	def GetGlobalRecommendations(self):
		message = messages.GetGlobalRecommendations()
		self.SendMessage(message)
	## Add A liked interest
	# @param self Networking (class)
	# @param interest string
	def AddInterest(self, interest):
		message = messages.AddInterest(interest)
		self.SendMessage(message)
	## Add a hated interest
	# @param self Networking (class)
	# @param interest string
	def AddHatedInterest(self, interest):
		message = messages.AddHatedInterest(interest)
		self.SendMessage(message)
	## Remove a liked interest
	# @param self Networking (class)
	# @param interest string
	def RemoveInterest(self, interest):
		message = messages.RemoveInterest(interest)
		self.SendMessage(message)
	## Remove a hated interest
	# @param self Networking (class)
	# @param interest string
	def RemoveHatedInterest(self, interest):
		message = messages.RemoveHatedInterest(interest)
		self.SendMessage(message)
	## Check the amount of time of server privileges we have left
	# @param self Networking (class)
	def CheckPrivileges(self):
		message = messages.CheckPrivileges()
		self.SendMessage(message)
	## Give a number of days of privileges ot a user (Must have privileges to give them)
	# @param self Networking (class)
	# @param user Username
	# @param days days of privileges
	def GivePrivileges(self, user, days):
		message = messages.GivePrivileges(user, days)
		self.SendMessage(message)
		
	## Set your away status
	# @param self Networking (class)
	# @param status (1:away, 0:online)
	def SetStatus(self, status):
		message = messages.SetStatus(status)
		self.SendMessage(message)
		
	## Museekd connect to server (Reconnect if connected)
	# @param self Networking (class)
	def ConnectServer(self):
		message = messages.ConnectServer()
		self.SendMessage(message)
	## Museekd disconnect from server
	# @param self Networking (class)
	def DisconnectServer(self):
		message = messages.DisconnectServer()
		self.SendMessage(message)
	## Museekd reload the shares db from disk
	# @param self Networking (class)	
	def ReloadShares(self):
		message = messages.ReloadShares()
		self.SendMessage(message)
	## Set an option in Museekd's config
	# @param self Networking (class)
	# @param domain parent of key
	# @param key key being changed
	# @param value value of key
	def ConfigSet(self, domain, key, value):
		message = messages.ConfigSet(domain, key, value)
		self.SendMessage(message)
	## Remove an option from Museekd's config
	# @param self Networking (class)
	# @param domain parent of key
	# @param key key being changed
	def ConfigRemove(self, domain, key):
		message = messages.ConfigRemove(domain, key)
		self.SendMessage(message)
	## Ping Museekd
	# @param self Networking (class)
	# @param num number that will be echoed back to us
	def Ping(self, num):
		message = messages.Ping(num)
		self.SendMessage(message)



class PopupMenu(gtk.Menu):
	def __init__(self, frame, type):
		gtk.Menu.__init__(self)
		self.frame = frame
		self.type = type
		self.user = None
		self.directory = None
		self.path = None
		self.file = None
		self.ready = False
		
	def setup(self, *items):
		for item in items:
			if item[0] == "":
				menuitem = gtk.MenuItem()
			elif item[0] == 1:
				menuitem = gtk.MenuItem(item[1])
				menuitem.set_submenu(item[2])
				if item[3] is not None:
					menuitem.connect("activate", item[3])
			else:
				if item[0][0] == "$":
					menuitem = gtk.CheckMenuItem(item[0][1:])
				elif item[0][0] == "#":
					menuitem = gtk.ImageMenuItem(item[0][1:])
					img = gtk.image_new_from_stock(item[2], gtk.ICON_SIZE_MENU)
        				menuitem.set_image(img)
				elif item[0][0] == "%":
					menuitem = gtk.ImageMenuItem(item[0][1:])	
					img = gtk.Image()
					img.set_from_pixbuf(item[2])
					menuitem.set_image(img)
				else:
					menuitem = gtk.MenuItem(item[0])
				if item[1] is not None:
					menuitem.connect("activate", item[1])
			self.append(menuitem)
			menuitem.show()
		return self

	def set_user(self, user):
		self.user = user
		
	def set_path(self, path):
		self.path = path
		
	def set_directory(self, directory):
		self.directory = directory
		
	def set_file(self, file):
		self.file = file
			
	def get_user(self):
		return self.user
		
	def OnSendMessage(self, widget):
		if DEBUG: Output("OnSendMessage")
		self.frame.GetPrivateChat(self.user) 
	
	def OnShowIPaddress(self, widget):
		if DEBUG: Output("OnShowIPaddress")
		self.frame.GetIPAddress(self.user)
	
	def OnDownloadFile(self, widget):
		if DEBUG: Output("OnDownloadFile")
		if self.path != None:
			path = self.path
		else:
			
			if self.directory[-1] != "\\":
				path = self.directory+"\\"+self.file
			else:
				path = self.directory+self.file
		self.frame.Networking.DownloadFile(self.user, path)
		
		
	def OnDownloadDirectory(self, widget):
		if DEBUG: Output("OnDownloadDirectory")
		if self.path != None:
			dire = ''	
			for path in self.path.split("\\")[:-1]:
				dire += path+"\\"
			
			self.frame.Networking.GetFolderContents(self.user, dire)
		else:
			self.frame.Networking.GetFolderContents(self.user, self.directory)

	
	def OnGetUserInfo(self, widget):
		if DEBUG: Output("OnGetUserInfo", self.user)
		self.frame.GetUserInfo(self.user)
		
	def OnBrowseUser(self, widget):
		self.frame.GetShares(self.user)
	
	def OnBuddyUser(self, widget):
		if self.ready == False:
			return
		if widget.get_active():
			self.frame.Networking.mod_config("buddy", self.user)
		else:
			self.frame.Networking.mod_config("unbuddy", self.user)
	
	def OnBanUser(self, widget):
		if self.ready == False:
			return
		if widget.get_active():
			self.frame.Networking.mod_config("ban", self.user)
		else:
			self.frame.Networking.mod_config("unban", self.user)
			
	def OnTrustUser(self, widget):
		if self.ready == False:
			return
		if widget.get_active():
			self.frame.Networking.mod_config("trust", self.user)
		else:
			self.frame.Networking.mod_config("distrust", self.user)
			
	def OnIgnoreUser(self, widget):
		if self.ready == False:
			return
		if widget.get_active():

			self.frame.Networking.mod_config("ignore", self.user)
		else:
			self.frame.Networking.mod_config("unignore", self.user)
			
	def OnLeaveRoom(self, widget):
		room = self.user
		self.frame.RoomLeave(room)
		
	def OnJoinRoom(self, widget):
		room = self.user
		self.frame.JoinARoom(room)

	
	def OnAbortDownTransfer(self, widget):
		self.frame.Networking.TransferAbort(0, self.user, self.path)
		
	def OnAbortUpTransfer(self, widget):
		self.frame.Networking.TransferAbort(1, self.user, self.path)
		
	def OnClearDownTransfer(self, widget):
		self.frame.Networking.TransferRemove(0, self.user, self.path)
		
	def OnClearUpTransfer(self, widget):
		self.frame.Networking.TransferRemove(1, self.user, self.path)
		
	def OnRetryTransfer(self, widget):	
		self.frame.Networking.DownloadFile(self.user, self.path)




class UserBrowse:
	def __init__(self, browses, user):
	
		self.frame = browses.frame
		self.browses = browses
		self.user = user 
		self.shares = {}
		self.selected_files = []
		self.files = {}
		self.directories = {}
		
		self.Main = gtk.VBox(False, 0)
		self.Main.show()
		self.Main.set_spacing(0)
		### Search Shares
		self.search_position = 0
		self.search_list = []
		self.query = None
		###
		hbox1 = gtk.HBox(False, 5)
		hbox1.show()
		hbox1.set_spacing(5)
		
		self.label1 = gtk.Label(("Search Shares For:"))
		self.label1.set_padding(5, 0)
		self.label1.show()
		hbox1.pack_start(self.label1, False, False, 5)
		
		self.entry = entry= gtk.Entry()
		entry.set_text("")
		#entry.set_activates_default(True)
		entry.set_editable(True)
		entry.connect_object("activate", self.SearchShares, user)
		entry.show()
		hbox1.pack_start(entry, True, True, 5)
		
		self.SearchButton = self.frame.CreateIconButton(gtk.STOCK_FIND, "stock", self.SearchShares, "Search")

		
		hbox1.pack_start(self.SearchButton, False, False, 5)
		
		self.RefreshButton = self.frame.CreateIconButton(gtk.STOCK_REFRESH, "stock", self.OnRefreshShares, "Refresh")

		
		hbox1.pack_start(self.RefreshButton, False, False, 5)
		
		self.Main.pack_start(hbox1, False, True, 0)
		
		hbox2 = gtk.HBox(False, 0)
		hbox2.show()
		hbox2.set_spacing(0)
		
		hpaned1 = gtk.HPaned()
		hpaned1.show()
		
		self.DirSW = DirSW = gtk.ScrolledWindow()
		self.DirSW.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.DirSW.set_size_request(250, -1)
		self.DirSW.show()
		self.DirSW.set_shadow_type(gtk.SHADOW_IN)
		
		
		self.DirStore = gtk.TreeStore(  str, str )
		
		self.DirTreeView = DirTreeView = gtk.TreeView(self.DirStore)
		#self.DirTreeView.set_property("rules-hint", True)
		self.DirTreeView.set_enable_tree_lines(True)
		#DirTreeView.get_selection().set_mode(gtk.SELECTION_MULTIPLE)
		DirTreeView.set_headers_visible(True)
		cols = InitialiseColumns(self.DirTreeView,
			[_("Directories"), -1, "text"], #0
		)
		cols[0].set_sort_column_id(0)
		#self.DirTreeView.set_search_column(0)
		DirTreeView.show()
		
		#self.DirStore.set_sort_column_id(0, gtk.SORT_ASCENDING)
		
		self.dir_popup_menu = popup = PopupMenu(self.frame, "browse-dirs")
		popup.setup(
			("#" + _("_Download Directory"), self.OnDownloadDirectory, gtk.STOCK_GO_DOWN),
			("#" + _("_Download Recursively"), self.OnDownloadDirectoryRecursive, gtk.STOCK_GO_DOWN),
			("#" + _("_Upload Directory"), self.OnUploadDirectoryTo, gtk.STOCK_GO_UP),
			("#" + _("_Upload Recursively"), self.OnUploadDirectoryRecursiveTo, gtk.STOCK_GO_UP),
			("", None),
			("#" + _("_Private Message"), popup.OnSendMessage, gtk.STOCK_EDIT),
			("#" + _("_User Info"), popup.OnGetUserInfo, gtk.STOCK_INFO),
			("#" + _("IP A_ddress"), popup.OnShowIPaddress, gtk.STOCK_NETWORK),
			("#" + _("Reload _Shares"), popup.OnBrowseUser, gtk.STOCK_HARDDISK),
			("$" + _("_Buddy this user"), popup.OnBuddyUser),
			("$" + _("Ba_n this user"), popup.OnBanUser),
			("$" + _("_Ignore this user"), popup.OnIgnoreUser),
			("$" + _("_Trust this user"), popup.OnTrustUser),
		)
		self.dir_popup_menu.set_user(user)
		DirTreeView.connect("button_press_event", self.OnDirClicked)
		DirTreeView.get_selection().connect("changed", self.OnDirSelected)
		DirSW.add(DirTreeView)

		
		hpaned1.pack1(DirSW, False, False)
		self.FilesStore = self.FileStore = gtk.ListStore(  str, str, str, int, str, int )
		
		self.FilesTreeView = gtk.TreeView(self.FileStore)
		self.FilesTreeView.set_property("rules-hint", True)
		self.FilesTreeView.set_reorderable(True)
		self.FilesTreeView.get_selection().set_mode(gtk.SELECTION_MULTIPLE)
		self.FilesTreeView.set_headers_visible(True)
		
		cols = InitialiseColumns(self.FilesTreeView,
		[_("Filename"), 250, "text"], #0
		[_("Size"), 100, "text"], #1
		[_("Length"), 70, "text"], #2
		[_("Bitrate"), 70, "text"], #3
		)
		#for ix in range(len(cols)):
		cols[0].set_sort_column_id(0)
		cols[1].set_sort_column_id(5)
		cols[2].set_sort_column_id(2)
		cols[3].set_sort_column_id(3)
		
		self.files_popup_menu = popup = PopupMenu(self.frame, "browse-files")
		popup.setup(
			("#" + _("Download _File(s)"), self.OnDownloadFiles, gtk.STOCK_GO_DOWN),
			("#" + _("_Download Directory"), popup.OnDownloadDirectory, gtk.STOCK_GO_DOWN),
			("#" + _("Upload _File(s)"), self.OnUploadFiles, gtk.STOCK_GO_UP),
			("", None),
			("#" + _("_Private Message"), popup.OnSendMessage, gtk.STOCK_EDIT),
			("#" + _("_User Info"), popup.OnGetUserInfo, gtk.STOCK_INFO),
			("#" + _("_IP Address"), popup.OnShowIPaddress, gtk.STOCK_NETWORK),
			("#" + _("Reload _Shares"), popup.OnBrowseUser, gtk.STOCK_HARDDISK),
			("$" + _("_Buddy this user"), popup.OnBuddyUser),
			("$" + _("Ba_n this user"), popup.OnBanUser),
			("$" + _("_Ignore this user"), popup.OnIgnoreUser),
			("$" + _("_Trust this user"), popup.OnTrustUser),
		)
		self.files_popup_menu.set_user(user)
		self.FilesTreeView.connect("button_press_event", self.OnFileClicked)
		self.FilesTreeView.show()
		
		self.FilesSW = gtk.ScrolledWindow()
		self.FilesSW.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.FilesSW.set_shadow_type(gtk.SHADOW_IN)
		self.FilesSW.add(self.FilesTreeView)
		self.FilesSW.show()
		directory = self.BrowseGetDirs(user)
		self.BrowseChangeDir(user, directory)
		
		hpaned1.pack2(self.FilesSW, True, True)
		
		hbox2.pack_start(hpaned1, True, True, 0)
		
		self.Main.pack_start(hbox2, True, True, 0)
			
		
	def OnDirSelected(self, selection):
		model, iter = selection.get_selected()
		if iter is None:
			self.selected_folder = None
			self.BrowseChangeDir(self.user, None)
			return
		path = model.get_path(iter)
		directory = self.DirStore.get_value(self.DirStore.get_iter(path), 1)
		self.selected_folder = directory
		self.BrowseChangeDir(self.user, directory)
		
	def OnDownloadDirectory(self, widget):
		self.DownloadDirectory(self.selected_folder)
	
	def DownloadDirectory(self, dir, prefix = "", recurse = 0):
		if dir == None:
			return
		ldir = prefix + dir[:-1].split("\\")[-1]
		for file in self.shares[dir]:
			self.frame.Networking.DownloadFileTo(self.user, "\\".join([dir, file]), ldir)

		if not recurse:
			return
		for directory in self.shares.keys():
			if dir in directory and dir != directory:
	
				self.DownloadDirectory(directory, os.path.join(ldir, ""), recurse)
				
	def DownloadDirectoryRecursive(self, dir, prefix = ""):
		# Find all files and add them to list
		if dir == None:
			return
		localdir = prefix + dir[:-1].split("\\")[-1]
		files = []
		
		if dir in self.shares.keys():
			for file in self.shares[dir]:
				files.append(["\\".join([dir, file]), localdir])
		
		for directory in self.shares.keys():
			if dir in directory and dir != directory:
				files += self.DownloadDirectoryRecursive(directory, os.path.join(localdir, ""))
	
		return files

	def OnDownloadDirectoryRecursive(self, widget):

		prefix = ""
		dir = self.selected_folder 
		if dir == None:
			return
		localdir = prefix + dir[:-1].split("\\")[-1]

		files = []
		files += self.DownloadDirectoryRecursive(dir, os.path.join(localdir, ""))
		
		# Check the number of files to be downloaded, just to make sure we aren't accidently downloading hundreds or thousands
		numfiles = len(files)
		go_ahead=0
		if len(files) > 100:
			go_ahead = Option_Box(self.frame, title=_('Murmur: Download %i files?' %numfiles), message=_("Are you sure you wish to download %i files from %s's directory %s?" %( numfiles, self.user, dir ) ), option1=_("Ok"), option3=_("Cancel"), option2=None, status="warning" )
			
		else:
			go_ahead = 1
			
		if go_ahead == 1:
			# Good to go, we download these
			for item in files:
				file, localpath = item
				#self.frame.np.transfers.getFile(self.user, file, localpath)
				self.frame.Networking.DownloadFileTo(self.user, file, localpath)

			
	def OnUploadDirectoryRecursiveTo(self, widget):
		self.OnUploadDirectoryTo(widget, recurse=1)
	
	def UploadDirectoryTo(self, user, dir, recurse = 0):
		if dir == None:
			return

		ldir = dir[:-1].split("\\")[-1]
		
		if user is None or user == "":
			return
		else:
			if dir in self.shares.keys():

				for file in self.shares[dir]:
					self.frame.Networking.UploadFile(user, "%s\\%s" %(dir, file) )
		
		if not recurse:
			return

		for directory in self.shares.keys():
			if dir in directory and dir != directory:
				self.UploadDirectoryTo(user, directory, recurse)
				
	def OnUploadDirectoryTo(self, widget, recurse = 0):
		dir = self.selected_folder
		if dir == None:
			return
	
		users = []
		if self.frame.Networking.config.has_key("buddies"):
			for user in self.frame.Networking.config["buddies"]:
				users.append(user)
		users.sort()
		user = input_box(self.frame, title=_("Nicotine: Upload Directory's Contents"),
		message=_('Enter the User you wish to upload to:'),
		default_text='', droplist=users)
		self.UploadDirectoryTo(user, dir, recurse)
		
	def OnDownloadFiles(self, widget, prefix = ""):
		dir = self.selected_folder
		for fn in self.selected_files:
			self.frame.Networking.DownloadFileTo(self.user, "\\".join([dir, fn]), prefix)
			
	def OnDownloadFilesTo(self, widget):
		ldir = ChooseDir(self.frame.MainWindow, self.frame.Config["transfers"]["downloaddir"])
		if ldir is None:
			return

		for directory in ldir: # iterate over selected files
			try:
 				self.OnDownloadFiles(widget, directory)
				
			except IOError: # failed to open
				self.message('failed to open %r for reading', directory) # notify user
	
	def OnUploadFiles(self, widget, prefix = ""):
		dir = self.selected_folder
		users = []
		if self.frame.Networking.config.has_key("buddies"):
			for user in self.frame.Networking.config["buddies"]:
				users.append(user)
		users.sort()
		username = input_box(self.frame, title=_('Murmur: Upload File(s)'),
		message=_('Enter the User you wish to upload to:'),
		default_text='', droplist=users)
		if username is None or username == "":
			pass
		else:
			for path in self.selected_files:
				self.frame.Networking.UploadFile(username, "%s\\%s" %(self.directory, path) )
				
	def OnDirClicked(self, widget, event):
		if event.button != 3:
			self.MouseAction(widget, event)
			return
		self.OnDirPopupMenu(widget, event)
			
	def OnDirPopupMenu(self, widget, event):
		user = self.user
		menu = self.dir_popup_menu
		items = menu.get_children()
		
		d = self.DirTreeView.get_path_at_pos(int(event.x), int(event.y))
		if not d:
			return
		path, column, x, y = d
		directory = self.DirStore.get_value(self.DirStore.get_iter(path), 1)
		menu.set_directory(directory)
		num = 9
		
		
		if self.frame.Networking.config != {}:
			items[num].set_active(self.frame.Networking.config.has_key("buddies") and self.frame.Networking.config["buddies"].has_key(user))
			items[num+1].set_active(self.frame.Networking.config.has_key("banned") and self.frame.Networking.config["banned"].has_key(user))
			items[num+2].set_active(self.frame.Networking.config.has_key("ignored") and self.frame.Networking.config["ignored"].has_key(user))
			items[num+3].set_active(self.frame.Networking.config.has_key("trusted") and self.frame.Networking.config["trusted"].has_key(user))
		menu.popup(None, None, None, event.button, event.time)
			
	def SelectedFilesCallback(self, model, path, iter):
		self.selected_files.append(self.FileStore.get_value(iter, 0))
		
	def OnFileClicked(self, widget, event):
		if event.button == 1 and event.type == gtk.gdk._2BUTTON_PRESS:
			# Double-Left-Click
			self.selected_files = []
			self.FilesTreeView.get_selection().selected_foreach(self.SelectedFilesCallback)
			self.OnDownloadFiles(widget)
			self.FilesTreeView.get_selection().unselect_all()
			return True
		elif event.button == 3:
			# Right-click
			self.selected_files = []
			self.FilesTreeView.get_selection().selected_foreach(self.SelectedFilesCallback)
			return self.OnFilePopupMenu(widget, event)
		# Left-click / etc
		return False
		
	def OnFilePopupMenu(self, widget, event):
		user = self.files_popup_menu.user
		items = self.files_popup_menu.get_children()
		
		if DEBUG:  Output("-- Popup -- :")
		
		d = self.FilesTreeView.get_path_at_pos(int(event.x), int(event.y))
		if not d:
			return
		path, column, x, y = d
		
		file = self.FileStore.get_value(self.FileStore.get_iter(path), 0)
		directory = self.FileStore.get_value(self.FileStore.get_iter(path), 4)
		self.files_popup_menu.set_file(file)
		self.files_popup_menu.set_directory(directory)
		num = 8

		if self.frame.Networking.config != {}:
			items[num].set_active(self.frame.Networking.config.has_key("buddies") and self.frame.Networking.config["buddies"].has_key(user))
			items[num+1].set_active(self.frame.Networking.config.has_key("banned") and self.frame.Networking.config["banned"].has_key(user))
			items[num+2].set_active(self.frame.Networking.config.has_key("ignored") and self.frame.Networking.config["ignored"].has_key(user))
			items[num+3].set_active(self.frame.Networking.config.has_key("trusted") and self.frame.Networking.config["trusted"].has_key(user))

		
		self.FilesTreeView.emit_stop_by_name("button_press_event")
		self.files_popup_menu.popup(None, None, None, event.button, event.time)
		return True
		
	def MouseAction(self, widget, event):
		d = self.DirTreeView.get_path_at_pos(int(event.x), int(event.y))
		if not d:
			return
		path, column, x, y = d
		directory = self.DirStore.get_value(self.DirStore.get_iter(path), 1)
		self.BrowseChangeDir(self.user, directory)
		
	def SetSensitive(self, bool):
		self.DirTreeView.set_sensitive(bool)
		self.FilesTreeView.set_sensitive(bool)
		self.SearchButton.set_sensitive(bool)
		self.RefreshButton.set_sensitive(bool)
		self.entry.set_sensitive(bool)
		self.label1.set_sensitive(bool)
		
	def ShowShares(self, user, shares):
		try:
			
			if user != self.user:
				return
			
			self.shares = shares
			directory = self.BrowseGetDirs(user)
			self.BrowseChangeDir(user, directory)
			self.DirTreeView.expand_all()
			self.SetSensitive(True)
			
		except Exception,e:
			if DEBUG: Output("get_shares", e)
			
	def OnClose(self, user):
		self.browses.RemoveTab(self)
		
	def OnRefreshShares(self, widget):
		self.SetSensitive(False)
		self.frame.GetShares(self.user)
		
	def SearchShares(self, widget):
		query = self.entry.get_text()
		if self.query == query:
			self.search_position += 1
		else: 
			self.search_position = 0
		self.query = query
		if self.query == "":
			return
		if DEBUG: Output("Query:", query)
		dir = self.directory
		self.search_list = []
		
		searchfilter = re.compile('.*' +str(query) + '.*', re.DOTALL | re.I)

		for directory, files in self.shares.items():
			if directory == dir:
				continue
			if re.match(searchfilter, directory):
				if directory not in self.search_list:
					self.search_list.append(directory)
			for file in files.keys():
				if re.match(searchfilter, file):
					if directory not in self.search_list:
						self.search_list.append(directory)
		if self.search_list != []:
			if self.search_position not in range(len(self.search_list)):
				self.search_position = 0
			self.search_list.sort()
			directory = self.search_list[self.search_position]
			#self.BrowseChangeDir(user, directory)
			## Set directory position to be the searched directory
			self.DirTreeView.set_cursor(self.DirStore.get_path(self.directories[directory]))
			
			# Get matching files in the current directory
			resultfiles = []
			for file in self.shares[directory]:
				if re.match(searchfilter, file):
					resultfiles.append(file)
					
			sel = self.FilesTreeView.get_selection()
			sel.unselect_all()
			l = 1
			resultfiles.sort()
			for fn in resultfiles:
				path = self.FilesStore.get_path(self.files[fn])
				# Select each matching file in directory
				sel.select_path(path)
				if l:
					# Position cursor at first match
					#self.FilesTreeView.set_cursor(path)
					self.FilesTreeView.scroll_to_cell(path, None, True, 0.5, 0.5)
					l = 0


		else:
			self.search_position = 0

		
	def BrowseGetDirs( self, user):
		self.DirStore.clear()
		self.directories.clear()
		
			
		z = list(self.shares.keys())
		if z == []:
			return
		z.sort()
		zz = []
		directory = ""
		
		try:
			for item in z:
				s = item.split("\\")
				path = ''

				parent = s[0]
				if parent == '':
					parent += "\\"
					if parent not in self.directories.keys():
						self.directories[parent] =  self.DirStore.append(None, [parent, parent])
				parent = s[0]
				for seq in s[1:]:
					if parent == "":
						parent += "\\"
						path = parent+seq
					else:
						path = parent+"\\"+seq


					if parent not in self.directories.keys():
						self.directories[parent] =  self.DirStore.append(None, [parent, parent])


					if path not in zz:
						zz.append(path)
						self.directories[path] = self.DirStore.append(self.directories[parent], [path.split("\\")[-1], path ] )
					parent = path

			directory = zz[0]
			return directory
		except Exception,e:
			if DEBUG: Output("BrowseGetDirs", e)
			
	def BrowseChangeDir(self, user, directory):
		self.directory = directory
		self.FileStore.clear()
		self.files = {}
		if not self.shares.has_key(directory):
			return
		path = self.DirStore.get_path( self.directories[directory] )
		self.DirTreeView.expand_to_path(path)
		#file, stats[ size, ftype, [bitrate, length ] ]
		if directory in self.shares:
			for files,stats in self.shares[directory].items():
				#Output(files, stats)
				size, ftype, attr  = stats
				#[bitrate, length, something ]
				if ftype == "" or attr == []:
					length = 0
					bitrate = 0
				else:
					bitrate = attr[0]
					length = self.frame.Length2Time(attr[1])

				iter = self.FileStore.append([files, self.frame.Humanize(size), length, bitrate, directory, int(size)])
				self.files[files] = iter
			
		# Finish
		self.FileStore.set_sort_column_id(0, gtk.SORT_ASCENDING)
			
class UserBrowses(IconNotebook):
	def __init__(self, frame):
		IconNotebook.__init__(self, frame.images)
		self.set_scrollable(True)
		self.show()
		self.users = {}
		self.frame = frame
		self.set_border_width(4)
		self.set_tab_pos(gtk.POS_TOP)
		self.connect("switch-page", self.OnSwitchPage)
		
	def GotUserBrowse(self, user, shares):
		if self.users.has_key(user):
			self.users[user].ShowShares(user, shares)
		else:
			self.users[user] = UserBrowse(self, user)
		
			self.append_page(self.users[user].Main, user[:15], self.users[user].OnClose)

			self.users[user].ShowShares(user, shares)
		self.request_changed(self.users[user].Main)
		self.frame.RequestIcon(self.frame.UserBrowseLabel)	
	def OnSwitchPage(self, notebook, page, page_num, force = 0):
		
		if self.frame.notebook_outside.get_current_page() != 4 and not force:
			return
		page = notebook.get_nth_page(page_num)	

	def RemoveTab(self, tab):
		self.remove_page(self.page_num(tab.Main))
		del self.users[tab.user]
		tab.Main.destroy()
		del tab
		gc.collect()
		
class UserInfo:
	def __init__(self, infos, user):
		self.frame = infos.frame
		self.userinfos = infos
		self.user = user

		userinfoVbox = gtk.VBox(False, 0)
		userinfoVbox.show()
		userinfoVbox.set_spacing(0)

		scrolledwindow1 = gtk.ScrolledWindow()
		scrolledwindow1.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		scrolledwindow1.show()
		scrolledwindow1.set_shadow_type(gtk.SHADOW_OUT)
	
		self.UserInfoText = UserInfoText = gtk.TextView()
		UserInfoText.set_wrap_mode(gtk.WRAP_WORD)
		UserInfoText.set_cursor_visible(True)
		UserInfoText.set_editable(False)
		self.textbuffer = textbuffer = UserInfoText.get_buffer()
		#textbuffer.set_text(info)
		UserInfoText.show()
		scrolledwindow1.add(UserInfoText)
	
		userinfoVbox.pack_start(scrolledwindow1, True, True, 5)
	
		vboxstats = gtk.VBox(False, 0)
		vboxstats.show()
		vboxstats.set_spacing(5)
		vboxstats.set_border_width(4)
		#  
		hbox2 = gtk.HBox(False, 0)
		hbox2.show()
		hbox2.set_spacing(0)
	
		label4 = gtk.Label(("Slots:"))
		label4.set_padding(0, 0)
		label4.show()
		hbox2.pack_start(label4, False, False, 0)
	
		self.SlotsNum = SlotsNum = gtk.Label() # str(slotsfree)
		SlotsNum.set_padding(0, 0)
		SlotsNum.show()
		hbox2.pack_start(SlotsNum, False, False, 5)
		label6 = gtk.Label(("Uploads allowed:"))
		label6.set_padding(0, 0)
		label6.show()
		
		hbox2.pack_start(label6, False, False, 5)
	
		self.UploadsAllowed = UploadsAllowed = gtk.Label() #str(uploads) 
		UploadsAllowed.set_padding(0, 0)
		UploadsAllowed.show()
		hbox2.pack_start(UploadsAllowed, False, False, 5)
		
	
		vboxstats.pack_start(hbox2, False, False, 0)
	
		hbox5 = gtk.HBox(False, 0)
		hbox5.show()
		hbox5.set_spacing(0)
	
		label7 = gtk.Label(("Queue:"))
		label7.set_padding(0, 0)
		label7.show()
		hbox5.pack_start(label7, False, False, 0)
	
		self.QueueNum = QueueNum = gtk.Label() # str(queue)
		QueueNum.set_padding(0, 0)
		QueueNum.show()
		hbox5.pack_start(QueueNum, False, False, 5)
	
		label8 = gtk.Label(("Files:"))
		label8.set_padding(0, 0)
		label8.show()
		hbox5.pack_start(label8, False, False, 0)

		self.FilesNum = FilesNum = gtk.Label() # str(files)
		FilesNum.set_padding(0, 0)
		FilesNum.show()
		hbox5.pack_start(FilesNum, False, False, 5)
		
		label82 = gtk.Label(("Speed:"))
		label82.set_padding(0, 0)
		label82.show()
		hbox5.pack_start(label82, False, False, 0)
		
		self.SpeedNum = SpeedNum = gtk.Label() # str(speed)
		SpeedNum.set_padding(0, 0)
		SpeedNum.show()
		hbox5.pack_start(SpeedNum, False, False, 5)
	
		vboxstats.pack_start(hbox5, False, False, 0)
	
		hboxbuttons = gtk.HBox(False, 0)
		hboxbuttons.show()
		hboxbuttons.set_spacing(5)

		close_button = gtk.Button("Close", stock="gtk-close")
		close_button.connect_object("clicked", self.OnClose, user)
		close_button.show()
		
		hboxbuttons.pack_start(close_button, False, False, 0)
		
		refresh_button = gtk.Button("Refresh", stock="gtk-refresh")
		refresh_button.connect_object("clicked", self.refresh_userinfo, user)
		refresh_button.show()	
		hboxbuttons.pack_start(refresh_button, False, False, 0)

		vboxstats.pack_start(hboxbuttons, False, False, 5)
		userinfoVbox.pack_start(vboxstats, False, False, 0)
		
		
		self.ScrollPicture = gtk.ScrolledWindow()
		self.ScrollPicture.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.ScrollPicture.set_shadow_type(gtk.SHADOW_OUT)
		self.UserImage = gtk.Image()
		self.UserImage.show()
		
		self.ScrollPicture.add_with_viewport(self.UserImage)
		self.ScrollPicture.show()

		self.Main = gtk.HPaned()
		self.Main.set_border_width(4)
		
		self.Main.pack1(userinfoVbox, False, True)
		self.Main.pack2(self.ScrollPicture, True, True)
		self.Main.show()

		
	def OnClose(self, user):
		self.userinfos.remove_page(self.Main)
		del self.userinfos.users[self.user]
		self.Main.destroy()

	def refresh_userinfo(self, user):
		self.frame.Networking.UserInfo(user)

	def ShowInfo(self, user, info, picture, uploads, queue, slotsfree):
		if DEBUG:
			Output("%s info received"%  user)
		textbuffer  = self.textbuffer
		UserInfoText = self.UserInfoText
		textbuffer = UserInfoText.get_buffer()
		textbuffer.set_text(info)

		self.SlotsNum.set_label( str(slotsfree) )
		self.QueueNum.set_label( str(queue) )
		self.UploadsAllowed.set_label( str(uploads) )
		
		if user in self.frame.user_stats:
			self.files = abs( self.frame.user_stats[user][3] )
			self.speed =  abs(self.frame.user_stats[user][1])
		else:
			self.speed = self.files = 0
			
		self.SpeedNum.set_label(str(self.speed))
		self.FilesNum.set_label(str(self.files))
		
		if picture != None and picture != "":
			s = gtk.gdk.PixbufLoader()
			try:
				s.write(picture)
			except Exception, error:
				if DEBUG:
					Output(error)
				pass
			s.close()
			self.UserImage.set_from_pixbuf(s.get_pixbuf())
			self.UserImage.show()
			del s, picture
			gc.collect()
		else:
			self.UserImage.set_from_pixbuf(None)
		

class UserInfos(IconNotebook):
	def __init__(self, frame):
		IconNotebook.__init__(self, frame.images)
		self.set_scrollable(True)
		self.show()
		self.users = {}
		self.frame = frame
		self.set_border_width(4)
		self.set_tab_pos(gtk.POS_TOP)
		self.connect("switch-page", self.OnSwitchPage)
		
	def GotUserInfo(self, user, info, picture, uploads, queue, slotsfree):
		#self.frame.UserInfoLabel.set_image(self.images["online"])
		self.frame.RequestIcon(self.frame.UserInfoLabel)
		if self.users.has_key(user):
			self.users[user].ShowInfo(user, info, picture, uploads, queue, slotsfree)
		else:
			self.users[user] = UserInfo(self, user)

			self.append_page(self.users[user].Main, user[:15], self.users[user].OnClose)
			

			self.users[user].ShowInfo(user, info, picture, uploads, queue, slotsfree)
		self.request_changed(self.users[user].Main)
	def OnSwitchPage(self, notebook, page, page_num, force = 0):
		
		if self.frame.notebook_outside.get_current_page() != 4 and not force:
			return
		page = notebook.get_nth_page(page_num)	
		
	def RemoveTab(self, tab):
		self.remove_page(self.page_num(tab.Main))
		del self.users[tab.user]
		
class PrivateChat:
	def __init__(self, chats, user):
		self.chats  = chats
		self.frame = chats.frame
		self.user = user
		self.Status = 0
		self.Main = gtk.VBox(False, 0)
		self.Main.show()
		self.Main.set_spacing(0)
	
		self.scrolledwindow16 = gtk.ScrolledWindow()
		self.scrolledwindow16.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.scrolledwindow16.show()
		self.scrolledwindow16.set_shadow_type(gtk.SHADOW_IN)
	
		self.ChatScroll = gtk.TextView()
		self.ChatScroll.set_wrap_mode(gtk.WRAP_WORD)
		self.ChatScroll.set_cursor_visible(False)
		self.ChatScroll.set_editable(False)
		self.ChatScroll.show()
		self.scrolledwindow16.add(self.ChatScroll)
	
		self.Main.pack_start(self.scrolledwindow16, True, True, 0)
	
		self.hbox5 = gtk.HBox(False, 5)
		self.hbox5.show()
		#self.hbox5.set_spacing(0)
		if self.frame.SEXY:
			self.ChatLine = sexy.SpellEntry()
		else:
			self.ChatLine = gtk.Entry()

		self.ChatLine.set_text("")
		self.ChatLine.set_editable(True)
		self.ChatLine.show()
		self.ChatLine.set_visibility(True)
		self.ChatLine.connect("activate", self.OnEnter)
		self.ChatLine.connect("key_press_event", self.OnKeyPress)
		
		self.hbox5.pack_start(self.ChatLine, True, True, 5)
		self.hbox5.set_focus_child(self.ChatLine)
		
		self.Encoding_List = gtk.ListStore(gobject.TYPE_STRING)
		self.Encoding = gtk.ComboBox()
		self.Encoding.show()
		self.Encoding.connect("changed", self.OnEncodingChanged)
	
		self.Encoding.set_model(self.Encoding_List)
		cell = gtk.CellRendererText()
		self.Encoding.pack_start(cell, True)
		self.Encoding.add_attribute(cell, 'text', 0)
		self.hbox5.pack_start(self.Encoding, False, False, 0)
	
		self.Log = gtk.CheckButton()

		self.Log.set_active(self.frame.Config["logging"]["logprivate"])
		self.Log.set_label(_("Log"))
		self.Log.show()
		self.Log.connect("toggled", self.OnLogToggled)
		self.hbox5.pack_start(self.Log, False, False, 5)
		self.button1 = self.frame.CreateIconButton(gtk.STOCK_CLOSE, "stock", self.OnClose, _("Close"))

	
		self.hbox5.pack_start(self.button1, False, False, 5)
	
		self.Main.pack_start(self.hbox5, False, True, 0)
		self.Main.set_focus_child(self.hbox5)
	
	
		
		self.popup_menu = popup = PopupMenu(self.frame, "something")
		popup.setup(
			("#" + _("Close"), self.OnClose, gtk.STOCK_CLOSE),
			("", None),
			("#" + _("Show IP address"), popup.OnShowIPaddress, gtk.STOCK_NETWORK),
			("#" + _("Get user info"), popup.OnGetUserInfo, gtk.STOCK_INFO),
			("#" + _("Browse files"), popup.OnBrowseUser, gtk.STOCK_HARDDISK),
			#(_("Give privileges"), popup.OnGivePrivileges),
			("$" + _("Add user to list"), popup.OnBuddyUser),
			("$" + _("Ban this user"), popup.OnBanUser),
			("$" + _("Ignore this user"), popup.OnIgnoreUser),
			("$" + _("_Trust this user"), popup.OnTrustUser),
			#(_("Client Version"), popup.OnVersion ),
		)
		popup.set_user(user)
		self.ChatScroll.connect("button_press_event", self.OnPopupMenuPrivate)
		self.ChatLine.grab_focus()
		self.UpdateColours()
		
	def UpdateColours(self):
		def makecolour(buffer, colour):
			if self.frame.Config["ui"].has_key(colour):
				colour = self.frame.Config["ui"][colour]
			else:
				colour = ""
			font = self.frame.Config["ui"]["chatfont"]

			if colour:
				return buffer.create_tag(foreground = colour, font=font)
			else:
				return buffer.create_tag( font=font)

				
		buffer = self.ChatScroll.get_buffer()
		self.tag_remote = makecolour(buffer, "chatremote")
		self.tag_local = makecolour(buffer, "chatlocal")
		self.tag_me = makecolour(buffer, "chatme")
		self.tag_hilite = makecolour(buffer, "chathilite")
		
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
		self.tag_username = makecolour(buffer, color)

		
	def changecolour(self, tag, colour):
		if self.frame.Config["ui"].has_key(colour):
			colour = self.frame.Config["ui"][colour]
		else:
			colour = ""
		font = self.frame.Config["ui"]["chatfont"]
		
		if colour:
			tag.set_property("foreground", colour)
			tag.set_property("font", font)
		else:
			tag.set_property("font", font)
			
	def ChangeColours(self):
		
				
		self.changecolour(self.tag_remote, "chatremote")
		self.changecolour(self.tag_local, "chatlocal")
		self.changecolour(self.tag_me, "chatme")
		self.changecolour(self.tag_hilite, "chathilite")
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
		self.changecolour(self.tag_username, color)
		self.setStatus()
		
	def setStatus(self):
		
		if self.frame.user_stats.has_key(self.user):

			if self.Status == self.frame.user_stats[self.user][0]:
				return
			self.Status = self.frame.user_stats[self.user][0]
			if self.Status == 1:
				color = "useraway"
			elif self.Status == 2:
				color = "useronline"
			else:
				color = "useroffline"
		else:
			color = "useroffline"
			self.Status = 0

		self.changecolour(self.tag_username, color)
		
	def OnClose(self, widget):

		self.chats.remove_page(self.Main)
		del self.chats.users[self.user]
		self.Main.destroy()
		
	def OnLogToggled(self, widget):
		
		act = self.Log.get_active()
		if act:
			if self.user not in self.frame.Config["logging"]["private"]:
				self.frame.Config["logging"]["private"].append(self.user)
			
		else:
			if self.user in self.frame.Config["logging"]["private"]:
				self.frame.Config["logging"]["private"].remove(self.user)

	def OnEncodingChanged(self, widget):
		pass
	
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
		self.frame.RequestIcon(self.frame.PrivateChatLabel)
		AppendLine(self.ChatScroll, line, tag, "%c", username=self.user, usertag=self.tag_username)
		#if self.z:
			#self.z = 0
			#self.tag_username.set_property("foreground", "#00FF00")
		#else:
			#self.z = 1
			#self.tag_username.set_property("foreground", "#FF0000")
		if self.Log.get_active():
			#self.logfile = WriteLog(self.logfile, self.frame.Config["logging"]["logsdir"], self.user, line)
			self.frame.Logging.PrivateChatLog( direction, self.user, text)
		
		#autoreply = self.frame.np.config.sections["server"]["autoreply"]
		#if self.frame.away and not self.autoreplied and autoreply:
			#self.SendMessage("[Auto-Message] %s" % autoreply)
			#self.autoreplied = 1

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
			
		AppendLine(self.ChatScroll, line, tag, "%c", username=self.user, usertag= self.tag_username)
		if self.Log.get_active():
			self.frame.Logging.PrivateChatLog( 1, self.user, text)
		
		

		
	COMMANDS = ["/alias ", "/unalias ", "/whois ", "/browse ", "/ip ", "/pm ", "/msg ", "/search ", "/usearch ", "/rsearch ", "/bsearch ", "/join ", "/leave", "/add ", "/buddy ", "/unbuddy ", "/ban ", "/ignore ", "/unban ", "/unignore ", "/clear", "/part ", "/quit", "/rescan", "/tick", "/nsa", "/info", "/ctcpversion", "/rem ", "/cl", "/t", "/a", "/q", "/l", "/p", "/bs", "/rs", "/us", "/s", "/m", "/w", "/al", "/un", "/w", "/updatenormal", "/updatebuddy", "/rescannormal", "/rescanbuddy", "/addnormaldir", "/addbuddydir", "/removenormaldir", "/removebuddydir", "/listshares", "/listnormal", "/listbuddy" ]
				
	def destroy(self):
		#if self.frame.translux:
			#self.frame.translux.unsubscribe(self.tlux_chat)
		self.Main.destroy()

	def OnPopupMenuPrivate(self, widget, event):
		if event.button != 3:
			return
		items = self.popup_menu.get_children()
		user = self.user
		num = 5
		config = self.frame.Networking.config
		self.popup_menu.ready = False
		if "buddies" in config:
			items[num].set_active(user in config["buddies"])
		if "banned" in config:
			items[num+1].set_active(user in config["banned"])
		if "ignored" in config:
			items[num+2].set_active(user in config["ignored"])
		if "trusted" in config:
			items[num+3].set_active(user in config["trusted"])
		self.popup_menu.popup(None, None, None, event.button, event.time)
		self.popup_menu.ready = True
		self.ChatScroll.emit_stop_by_name("button_press_event")
		return True
		
	def OnEnter(self, widget):
		text = widget.get_text()
		
		result = None
		#result = expand_alias(self.frame.np.config.aliases, text)
		if result is not None:
			text = result
		if not text:
			widget.set_text("")
			return
			
		s = text.split(" ", 1)
		cmd = s[0]
		if len(s) == 2 and s[1]:
			realargs = args = s[1]
		else:
			args = self.user
			realargs = ""

		#if cmd in ("/alias", "/al"):
			#AppendLine(self.ChatScroll, self.frame.np.config.AddAlias(realargs), None, "")
		#elif cmd in ("/unalias", "/un"):
			#AppendLine(self.ChatScroll, self.frame.np.config.Unalias(realargs), None, "")
		if cmd in ["/w", "/whois", "/info"]:
			if args:
				self.frame.GetUserInfo(args)
				self.frame.OnUserInfo(None)
		elif cmd in ["/b", "/browse"]:
			if args:
				self.frame.GetShares(args)
				self.frame.OnUserShares(None)
		elif cmd == "/ip":
			if args:
				if args not in self.frame.ip_requested:
					self.frame.ip_requested.append(args)
					self.frame.Networking.PeerAddress(args)
			else:
				if self.user not in self.frame.ip_requested:
					self.frame.ip_requested.append(self.user)
					self.frame.Networking.PeerAddress(self.user)
		elif cmd == "/nsa":
			if args:
				self.frame.GetUserInfo(args)
				self.frame.GetShares(args)
				self.frame.OnUserInfo(None)
				
		elif cmd == "/reload":
			self.Networking.ReloadShares()
			
		elif cmd == "/pm":
			if realargs:
				self.frame.PrivateChats.SendMessage(realargs, None, 1)
		elif cmd in ["/m", "/msg"]:
			if realargs:
				s = realargs.split(" ", 1)
				user = s[0]
				if len(s) == 2:
					msg = s[1]
				else:
					msg = None
				self.frame.PrivateChats.SendMessage(user, msg)
		elif cmd in ["/s", "/search"]:
			if realargs:
				self.frame.Networking.Search(0, realargs)
				self.frame.OnSearch(None)
		elif cmd in ["/us", "/usearch"]:
			s = args.split(" ", 1)
			if len(s) == 2:
				self.frame.Networking.Search(3, s[1], s[0])
				self.frame.OnSearch(None)
			else:
				return
		elif cmd in ["/rs", "/rsearch"]:
			if realargs:
				self.frame.Networking.Search(1, realargs)
				self.frame.OnSearch(None)
		elif cmd in ["/bs", "/bsearch"]:
			if realargs:
				self.frame.Networking.Search(2, realargs)
				self.frame.OnSearch(None)
		elif cmd in ["/ad", "/add", "/buddy"]:
			if args:
				self.frame.Networking.mod_config("buddy", args)
		elif cmd in ["/rem", "/unbuddy"]:
			if args:
				self.frame.Networking.mod_config("unbuddy", args)
		elif cmd == "/ban":
			if args:
				self.frameNetworking.mod_config("ban", args)
		elif cmd == "/unban":
			if args:
				self.frame.Networking.mod_config("unban", args)
		elif cmd == "/ignore":
			if args:
				self.frame.Networking.mod_config("ignore", args)
		elif cmd == "/unignore":
			if args:
				self.frame.Networking.mod_config("unignore", args)
		elif cmd == "/trust":
			if args:
				self.frame.Networking.mod_config("trust", args)
		elif cmd == "/distrust":
			if args:
				self.frame.Networking.mod_config("distrust", args)
		elif cmd == "/ctcpversion":
			if args:
				self.frame.Networking.PrivateMessage(0, args, "\x01VERSION\x01")
			else:
				self.frame.Networking.PrivateMessage(0, self.user, "\x01VERSION\x01")
		elif cmd in ["/clear", "/cl"]:
			self.chatview.get_buffer().set_text("")
		elif cmd in ["/a", "/away"]:
			self.frame.OnAway(None)
		elif cmd in ["/q", "/quit"]:
			self.frame.OnExit(None)
		elif cmd in ["/c", "/close", "/l", "/leave", "/part", "/p"]:
			self.OnClose(None)
			
		elif cmd == "/updatenormal":
			self.frame.Muscan.UpdateNormal()
		elif cmd == "/updatebuddy":
			self.frame.Muscan.UpdateBuddy()
		elif cmd == "/rescan":
			self.frame.Muscan.RescanNormal()
			self.frame.Muscan.RescanBuddy()
		elif cmd == "/rescannormal":
			self.frame.Muscan.RescanNormal()
		elif cmd == "/rescanbuddy":
			self.frame.Muscan.RescanBuddy()
		
		elif cmd == "/addnormaldir":
			if args:
				self.frame.Muscan.AddNormalDirectory(args)
			else:
				return
		elif cmd == "/removenormaldir":
			if args:
				self.frame.Muscan.RemoveNormalDirectory(args)
			else:
				return
		elif cmd == "/addbuddydir":
			if args:
				self.frame.Muscan.AddBuddyDirectory(args)
			else:
				return
		elif cmd == "/removebuddydir":
			if args:
				self.frame.Muscan.RemoveBuddyDirectory(args)
			else:
				return
		elif cmd == "/listshares":
			self.frame.Muscan.ListNormal()
			self.frame.Muscan.ListBuddy()
		elif cmd == "/listnormal":
			self.frame.Muscan.ListNormal()
		elif cmd == "/listbuddy":
			self.frame.Muscan.ListBuddy()
		elif cmd in("/join", "/j"):
			if not args:
				return
			self.frame.JoinARoom(args)
		elif cmd in  ("/c",  "/close", "/leave", "/l"):
			if not args:
				return
			self.frame.RoomLeave(args)
			
		elif cmd and cmd[:1] == "/" and cmd != "/me" and cmd[:2] != "//":
			self.frame.AppendToLogWindow(_("Command %s is not recognized") % text)
			return
		else:
			if text[:2] == "//":
				text = text[1:]
			self.SendMessage(text)
			self.frame.Networking.PrivateMessage(0, self.user, text)
		widget.set_text("")
		
	def OnKeyPress(self, widget, event):
		
		if event.keyval == gtk.gdk.keyval_from_name("Prior"):
			scrolled = self.ChatScroll.get_parent()
			adj = scrolled.get_vadjustment()
			adj.set_value(adj.value - adj.page_increment)
		elif event.keyval == gtk.gdk.keyval_from_name("Next"):
			scrolled = self.ChatScroll.get_parent()
			adj = scrolled.get_vadjustment()
			max = adj.upper - adj.page_size
			new = adj.value + adj.page_increment
			if new > max:
				new = max
			adj.set_value(new)
		if event.keyval != gtk.gdk.keyval_from_name("Tab"):
			return False
		ix = widget.get_position()
		text = widget.get_text()[:ix].split(" ")[-1]
		
		list = [self.user] + ["museek"]
		if self.frame.Networking.config.has_key("buddies"):
			list += self.frame.Networking.config["buddies"].keys()
		if ix == len(text) and text[:1] == "/":
			list +=  self.COMMANDS 
			#self.frame.np.config.aliases.keys()] +

		completion, single = self.frame.GetCompletion(text, list)
		if completion:
			widget.insert_text(completion, ix)
			widget.set_position(ix + len(completion))
		widget.emit_stop_by_name("key_press_event")
		return True
		
class PrivateChats(IconNotebook):
	def __init__(self, frame):
		IconNotebook.__init__(self, frame.images)
		self.set_scrollable(True)
		self.show()
		self.users = {}
		self.frame = frame
		self.set_border_width(4)
		self.set_tab_pos(gtk.POS_TOP)
		self.connect("switch-page", self.OnSwitchPage)
		
	def SendMessage(self, user, text = None, direction = None):
		if not self.users.has_key(user):
			tab = PrivateChat(self, user)
			self.users[user] = tab
			

			self.append_page(tab.Main, user[:15], tab.OnClose)

		if direction:
			if self.get_current_page() != self.page_num(self.users[user].Main):
				self.set_current_page(self.page_num(self.users[user].Main))
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
		self.request_changed(self.users[user].Main)
		
	def OnSwitchPage(self, notebook, page, page_num, force = 0):
		if self.frame.notebook_outside.get_current_page() != 1 and not force:
			return
		page = notebook.get_nth_page(page_num)	
		for name, tab in self.users.items():
			if tab.Main is page:
				gobject.idle_add(tab.ChatLine.grab_focus)
				break
	def UpdateColours(self):
		for user in self.users.values():
			user.ChangeColours()
			
	def updateStatus(self, user, status):
		if self.users.has_key(user):
			gobject.idle_add(self.users[user].setStatus)
			
	def RemoveTab(self, tab):
		self.remove_page(self.page_num(tab.Main) )
		del self.users[tab.user]
		tab.Main.destroy()
		
class Searches(IconNotebook):
	def __init__(self, frame):
		IconNotebook.__init__(self, frame.images)
		self.set_scrollable(True)
		self.show()
		
		self.frame = frame
		self.set_border_width(4)
		self.set_tab_pos(gtk.POS_TOP)
		self.connect("switch-page", self.OnSwitchPage)
		
		self.tickets = {}

	def RemoveTab(self, tab):
		self.remove_page(self.page_num(tab.Main) )
		del self.tickets[tab.ticket]
		
	def DoSearch(self, searchtype, query, user=None):
		self.frame.Networking.Search(searchtype, query, user=None)
			
	def OnSwitchPage(self, notebook, page, page_num, force = 0):
		if self.frame.notebook_outside.get_current_page() != 1 and not force:
			return
		page = notebook.get_nth_page(page_num)	
		for name, tab in self.users.items():
			if tab.Main is page:
				gobject.idle_add(tab.Entry.grab_focus)
				break
			
	def NewResults(self, ticket, user, free, speed, queue, results):
		if self.tickets.has_key(ticket):
			self.tickets[ticket].NewResults(user, free, speed, queue, results)
			self.request_changed(self.tickets[ticket].Main)
			
	def NewSearch(self, query, ticket):
		#if str(ticket) not in self.frame.widgets["search"]:
			#self.DoSearch(str(ticket))
		
		if not self.tickets.has_key(ticket):
			tab = Search(self, query, ticket)
			self.tickets[ticket] = tab
			
			self.append_page(self.tickets[ticket].Main, query[:15], tab.OnClose)


	#def SearchFrame(self):
		
		
class Search:
	def __init__(self, searches, query, ticket):
		self.searches = searches
		self.frame = searches.frame
		self.query = str(query)
		self.ticket = ticket
		self.num = 0
		self.selected_results = []
		self.selected_users = []
		
		self.ScrolledWindow =  gtk.ScrolledWindow()
		self.ScrolledWindow.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.ScrolledWindow.show()
		self.ScrolledWindow.set_shadow_type(gtk.SHADOW_IN)
		self.store = gtk.ListStore(int, str, str, str, int, int, str, int, str, str, str, int, int)
		self.treeview = treeview = gtk.TreeView(self.store)
		self.treeview.set_property("rules-hint", True)
		treeview.show()
		treeview.set_headers_visible(True)
		self.treeview.get_selection().set_mode(gtk.SELECTION_MULTIPLE)
		treeview.set_reorderable(True)
		
		cols = InitialiseColumns(treeview,
		["", 30, "text"], # 0
		[_("Filename"), 250, "text"], #1
		[_("User"), 100, "text"], #2
		[_("Size"), 100, "text"], #3
		[_("Speed"), 80, "text"], #4
		[_("Queue"), 50, "text"], #5
		[_("Instant Download"), 20, "text"], #6
		[_("Bitrate"), 50, "text"], #7
		[_("Length"), 50, "text"], #8
		[_("Directory"), 500, "text"], #9
		# Full path 10 
		# size (number) 11
		# length (number) 12
		)
		
		cols[0].set_sort_column_id(0)
		cols[1].set_sort_column_id(1)
		cols[2].set_sort_column_id(2)
		cols[3].set_sort_column_id(11)
		cols[4].set_sort_column_id(4)
		cols[5].set_sort_column_id(5)
		cols[6].set_sort_column_id(6)
		cols[7].set_sort_column_id(7)
		cols[8].set_sort_column_id(12)
		cols[9].set_sort_column_id(9)

		

		self.popup_menu = popup = PopupMenu(self.frame, "search")
		popup.setup(
			("#" + _("Download _File"), self.OnDownloadSelected, gtk.STOCK_GO_DOWN),
			("#" + _("_Download Directory"), popup.OnDownloadDirectory, gtk.STOCK_GO_DOWN),
			("", None),
			("#" + _("_Private Message"), popup.OnSendMessage, gtk.STOCK_EDIT),
			("", None),
			("#" + _("_User Info"), popup.OnGetUserInfo, gtk.STOCK_INFO),
			("#" + _("IP A_ddress"), popup.OnShowIPaddress, gtk.STOCK_NETWORK),
			("#" + _("Reload _Shares"), popup.OnBrowseUser, gtk.STOCK_HARDDISK),
			("$" + _("_Buddy this user"), popup.OnBuddyUser),
			("$" + _("Ba_n this user"), popup.OnBanUser),
			("$" + _("_Ignore this user"), popup.OnIgnoreUser),
			("$" + _("_Trust this user"), popup.OnTrustUser),
		)
		treeview.connect("button_press_event", self.OnMouseClicked)
					
		self.ScrolledWindow.add(treeview)
		
		self.Main = self.ScrolledWindow

	def OnClose(self, widget):

		self.searches.RemoveTab(self)
		self.Main.destroy()
		
	def NewResults(self, user, free, speed, queue, results):
		try:
			
			
			Store = self.store
			treeview = self.treeview
			num = self.num
			if free == 1:
				free = "Y"
			else:
				free = "N"
			for files in results:
				num += 1
				dire = ''
				size = self.frame.Humanize( files[1])
				for path in files[0].split("\\")[:-1]:
					dire += path+"\\"
				if files[3] == []:
					bitrate = 0
					time = "0:00"
					length = 0
				else:
					bitrate = files[3][0]
					time = self.frame.Length2Time(files[3][1])
					length = files[3][1]
				
				self.store.append([ num, files[0].split("\\")[-1], user, size, speed, queue,free,  bitrate, time, dire , files[0], files[1], length ])

 			self.num = num

				
		except Exception, e:
			Output("ERROR: DoSearchResults: ", e)

	def SelectedResultsCallback(self, model, path, iter):
		user = model.get_value(iter, 2)
		fn = model.get_value(iter, 10)
		
		self.selected_results.append((user, fn))
		
		if not user in self.selected_users:
			self.selected_users.append(user)
			
	def OnDownloadSelected(self, widget):

		for file in self.selected_results:
			self.frame.Networking.DownloadFile(file[0], file[1])
			
	def OnMouseClicked(self, widget, event):
		if event.button == 1 and event.type == gtk.gdk._2BUTTON_PRESS:
			self.selected_results = []
			self.selected_users = []
			self.treeview.get_selection().selected_foreach(self.SelectedResultsCallback)
			self.OnDownloadSelected(widget)
			self.treeview.get_selection().unselect_all();
			return True
		
		elif event.button == 3:
			return self.OnPopupMenu(widget, event)
		return False
	
	def OnPopupMenu(self, widget, event):
		
		self.selected_results = []
		self.selected_users = []
		self.treeview.get_selection().selected_foreach(self.SelectedResultsCallback)
		
		items = self.popup_menu.get_children()
		d = self.treeview.get_path_at_pos(int(event.x), int(event.y))
		
		if not d:
			return
		path, column, x, y = d
		user =  self.store.get_value(self.store.get_iter(path), 2)
		file = self.store.get_value(self.store.get_iter(path), 10)
		self.popup_menu.set_user(user)
		self.popup_menu.set_path(file)
		num = 8
		
		self.popup_menu.set_user(user)
		#items[5].set_active(user in self.Networking.config["buddies"].keys())
		#if string is not "search":
		self.popup_menu.ready = False
		if self.frame.Networking.config != {}:
			items[num].set_active(self.frame.Networking.config.has_key("buddies") and self.frame.Networking.config["buddies"].has_key(user))
			items[num+1].set_active(self.frame.Networking.config.has_key("banned") and self.frame.Networking.config["banned"].has_key(user))
			items[num+2].set_active(self.frame.Networking.config.has_key("ignored") and self.frame.Networking.config["ignored"].has_key(user))
			items[num+3].set_active(self.frame.Networking.config["trusted"].has_key(user))
		self.popup_menu.popup(None, None, None, event.button, event.time)
		self.popup_menu.ready = True
		return True # Don't select files
		

class TrayApp:
	def __init__(self, frame):
		self.t = None
		self.frame = frame
		self.is_mapped = 1
		self.current_image = None
		self.pygtkicon = (gtk.pygtk_version[0] >= 2 and gtk.pygtk_version[1] >= 10)

	def Toggle(self, widget):
		if trayicon_load == 0:
			return
		
		self.is_mapped = 1
		if self.t == None:
			self.TrayMenu()
			if self.pygtkicon:
				self.t = gtk.StatusIcon()
				self.t.set_visible(True)
				self.t.connect("popup-menu", self.OnStatusIconPopup)
				self.t.connect("activate", self.OnStatusIconClicked)
			else:
				self.t = trayicon.TrayIcon("Murmur")
				self.eventbox = gtk.EventBox()
				self.t.add(self.eventbox)
				self.eventbox.connect_object("button_press_event", self.OnTrayiconClicked, popup)
				self.eventbox_closed = False
			
			self.TrayIconStatus()

			if not self.pygtkicon:
				self.t.show_all()

		else:
			
			if not self.pygtkicon:
				self.t.hide()
				self.eventbox.hide()
				self.eventbox = None
				self.eventbox_closed = True
				self.tooltips = gtk.Tooltips()
			#self.t = None
			self.Close()
		#self.tooltips.set_delay(1500)
		
	def Close(self):
		if self.t is None:
			return
		if self.pygtkicon:
			self.t.set_visible(False)
		self.t = None
			
	def OnStatusIconClicked(self, status_icon):
		self.HideUnhideWindow(None)
		
	def OnStatusIconPopup(self, status_icon, button, activate_time):
		if button == 3:
			self.tray_popup_menu.popup(None, None, None, button, activate_time)
			
	def TrayIconStatus(self):
		if DEBUG: Output("Status:", self.frame.status)
		if self.frame.status:
			message = _("Connected to Museek: %s Status: Away" % self.frame.username)
			self.SetToolTip( message)
			self.frame.update_statusbar( message)
			
			self.SetImage("yellow")
		elif self.frame.status == 0:
			message = _("Connected to Museek: %s Status: Online" % self.frame.username)
			self.SetToolTip( message )
			self.frame.update_statusbar( message )
			self.SetImage("green")
		else:
			self.SetImage("red")
			
	def OnTrayiconServer(self, widget):
		items = self.tray_popup_menu_server.get_children()
		
		
		if self.current_image_name == "red":
			items[0].set_sensitive(True)
			items[1].set_sensitive(False)
		else:
			items[0].set_sensitive(False)
			items[1].set_sensitive(True)
		return
			
	def HideUnhideWindow(self, widget):
		if self.is_mapped:
			self.frame.MurmurWindow.unmap()
			self.is_mapped = 0
		else:
			self.frame.MurmurWindow.map()
			self.frame.MurmurWindow.grab_focus()
			self.is_mapped = 1
			
	def TrayMenu(self):
		try:
			self.tray_popup_menu_server= popup0 = PopupMenu(self.frame, "")
			popup0.setup(
				("#" + _("_Connect"), self.frame.connect_process, gtk.STOCK_CONNECT),
				("#" + _("_Disconnect"), self.frame.disconnect, gtk.STOCK_DISCONNECT),
			)
			self.tray_popup_menu = popup = PopupMenu(self.frame, "")
			popup.setup(
				("#" + _("Hide / Unhide Murmur"), self.HideUnhideWindow, gtk.STOCK_GOTO_BOTTOM),
				(1, _("Server"), popup0, self.OnTrayiconServer),
				("#" + _("Settings"), self.frame.SettingsWindow, gtk.STOCK_PREFERENCES),
				("#" + _("Lookup a User's IP"), self.frame.LookupIP, gtk.STOCK_NETWORK),
				("#" + _("Lookup a User's Info"), self.frame.LookupUserInfo, gtk.STOCK_DIALOG_INFO),
				("#" + _("Lookup a User's Shares"), self.frame.LookupUserShares, gtk.STOCK_HARDDISK),
				("%" + _("Toggle Away"), self.frame.away_toggle, self.frame.images["away"] ),
				("#" + _("Quit"), self.frame.quit, gtk.STOCK_QUIT),
			)

			

			
		except Exception, e:
			if DEBUG: Output("menu error", e)
		

		
	def OnTrayiconClicked(self, widget, event):
		try:
			if event.type == gtk.gdk.BUTTON_PRESS:
				if event.button == 1:
					self.HideUnhideWindow(None)
				else:
					items = self.tray_popup_menu.get_children()
					if self.current_image_name == "red":
						items[3].set_sensitive(False)
						items[4].set_sensitive(False)
						items[5].set_sensitive(False)
						items[6].set_sensitive(False)
					else:
						
						items[3].set_sensitive(True)
						items[4].set_sensitive(True)
						items[5].set_sensitive(True)
						items[6].set_sensitive(True)
					widget.popup(None, None, None, event.button, event.time)
					# Tell calling code that we have handled this event the buck
					# stops here.
					return True
				# Tell calling code that we have not handled this event pass it on.
			return False
		except Exception,e:
			if DEBUG: Output("button_press error", e)
	
	def SetImage(self, image):
		try:
			if self.t == None:
				return
			self.current_image_name = image
			if not self.pygtkicon:
				self.eventbox.hide()
			
				if self.current_image != None and self.eventbox_closed == False:
					self.eventbox.remove(self.current_image)
				image1 = gtk.Image()
				image1.set_from_pixbuf(self.frame.images[image])
				image1.show()
				self.current_image = image1
				self.eventbox.add(self.current_image)
				self.eventbox.show()

			else:
				self.t.set_from_pixbuf(self.frame.images[image])
	

		
		except Exception,e:
			if DEBUG: Output("Error in SetImage", e)
			
	def SetToolTip(self, message):
		try:
			#gtk.gdk.threads_enter()
			if self.t is None:
				return
			if self.frame.Config["murmur"]["tooltips"]:
				if not self.pygtkicon:
					self.tooltips.set_tip(self.eventbox, message)
				else:
					self.t.set_tooltip(message)
			#gtk.gdk.threads_leave()
		except Exception,e:
			if DEBUG: Output("SetToolTip ERROR", e)
			

class ChatRoom:
	
	def __init__(self, chatrooms, room, users):

		self.lines = []
		self.chatrooms = chatrooms
		self.frame = chatrooms.frame
		self.room = room
		self.leaving = 0
		
		self.textbuffer = gtk.TextBuffer()
		self.chatview = gtk.TextView(buffer=self.textbuffer)
		self.chatview.set_editable(False)
		self.chatview.set_wrap_mode( gtk.WRAP_WORD)
		self.chatview.set_cursor_visible(False)
		self.chatview.show()
		
		self.chatScroll = gtk.ScrolledWindow()
		self.chatScroll.set_border_width(1)
		self.chatScroll.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.chatScroll.set_shadow_type(gtk.SHADOW_IN)
		self.chatScroll.add(self.chatview)
		self.chatScroll.show()
		if self.frame.SEXY:
			self.chatEntry = sexy.SpellEntry()
		else:
			self.chatEntry = gtk.Entry()

		self.chatEntry.set_text("")
		
		self.chatEntry.connect("activate", self.ChatEnter, self.chatEntry, "chat")
		self.chatEntry.connect("key_press_event", self.OnKeyPress)
		self.chatEntry.show()
		# Containing box with scrolled window and text input box
		self.vbox = gtk.VBox(spacing=3)
		self.vbox.set_border_width(3)
		self.vbox.pack_start(self.chatScroll, True, True, 0)
		self.vbox.pack_end(self.chatEntry, False, False, 0)
		self.vbox.show()
		
		
		
		self.statusbuffer = gtk.TextBuffer()
		
		self.RoomLog = gtk.TextView(buffer=self.statusbuffer)
		
		self.RoomLog.set_editable(False)
		self.RoomLog.set_wrap_mode( gtk.WRAP_NONE)
		self.RoomLog.set_cursor_visible(False)
		self.RoomLog.show()
		
		self.status_scroll = gtk.ScrolledWindow()
		self.status_scroll.set_border_width(1)
		self.status_scroll.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.status_scroll.set_shadow_type(gtk.SHADOW_IN)
		self.status_scroll.add(self.RoomLog)
		self.status_scroll.show()
		

		
		self.usersmodel = gtk.ListStore(gtk.gdk.Pixbuf, str, str, str, int )
		self.UserList = gtk.TreeView(self.usersmodel)
		self.UserList.set_property("rules-hint", True)
		# Users-in-room Listbox with users and files
		
		cols = InitialiseColumns(self.UserList, 
			[_("Status"), 0, "pixbuf"], #0
			[_("Username"), 100, "text"], #1
			[_("Files"), 0, "text"], #2
			[_("Speed"), 0, "text"], #3
		)
		
		cols[0].set_sort_column_id(4)
		cols[1].set_sort_column_id(1)
		cols[2].set_sort_column_id(2)
		cols[3].set_sort_column_id(3)
		cols[0].get_widget().hide()
		config = self.frame.Config
		if not config["columns"]["chatrooms"].has_key(room):
			config["columns"]["chatrooms"][room] = [1, 1, 1, 1]
		for i in range (4):
			parent = cols[i].get_widget().get_ancestor(gtk.Button)
			if parent:
				parent.connect('button_press_event', PressHeader)
			# Read Show / Hide column settings from last session
			cols[i].set_visible(config["columns"]["chatrooms"][room][i])
			
		self.users = {}
		for user in users.keys():
			img = self.frame.GetStatusImage(user, users[user][0]) 
			hspeed = self.frame.HSpeed(users[user][1])
			hfiles = Humanize(users[user][3])
			iter = self.usersmodel.append([img, str(user), hfiles, hspeed, users[user][0] ] )
			self.users[user] = iter
		self.usersmodel.set_sort_column_id(1, gtk.SORT_ASCENDING)
		
		self.popup_menu = popup = PopupMenu(self.frame, "chat")
		popup.setup(
			("#" + _("Private _Message"), popup.OnSendMessage, gtk.STOCK_EDIT),
			("", None),
			("#" + _("Show IP address"), popup.OnShowIPaddress, gtk.STOCK_NETWORK),
			("#" + _("Get user info"), popup.OnGetUserInfo, gtk.STOCK_INFO),
			("#" + _("Browse files"), popup.OnBrowseUser, gtk.STOCK_HARDDISK),
			("$" + _("_Buddy this user"), popup.OnBuddyUser),
			("$" + _("Ba_n this user"), popup.OnBanUser),
			("$" + _("_Ignore this user"), popup.OnIgnoreUser),
			("$" + _("_Trust this user"), popup.OnTrustUser),
		)
		self.UserList.connect("button_press_event", self.OnPopupMenuRoom, "chat", "")
		self.UserList.show()
		
		# Scrollwindow containing Users-in-room Listbox
		self.UserList_scroll = gtk.ScrolledWindow()
		self.UserList_scroll.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.UserList_scroll.add(self.UserList)
		self.UserList_scroll.set_shadow_type(gtk.SHADOW_ETCHED_IN)
		self.UserList_scroll.show()
		self.remoteSet = False
		self.autojoinCheck = gtk.CheckButton(label="Autojoin")
		if "autojoin" in self.frame.Networking.config.keys():
			if room in self.frame.Networking.config["autojoin"].keys():
				self.autojoinCheck.set_active(True)
			else:
				self.autojoinCheck.set_active(False)
		else:
			self.autojoinCheck.set_active(False)
		self.autojoinCheck.connect("toggled", self.autojoinToggled)
		self.autojoinCheck.show()
		
		self.Leave = self.frame.CreateIconButton(gtk.STOCK_CLOSE, "stock", self.OnLeave, _("Leave Room"))

		over_button = gtk.HBox(spacing=3)
		over_button.set_border_width(3)
		over_button.pack_start(self.autojoinCheck, True, True, 0)
		over_button.pack_end(self.Leave, False, False, 1)
		over_button.show()

		userlist_over_button = gtk.VBox(spacing=0)
		userlist_over_button.set_border_width(0)
		userlist_over_button.pack_start(self.UserList_scroll, True, True, 0)
		userlist_over_button.pack_end(over_button, False, False, 1)
		userlist_over_button.show()
		
		# Pane that contains the Statusbox and Chatroom
		self.vpane = gtk.VPaned()
		self.vpane.pack1(self.status_scroll, False, True)
		self.vpane.pack2(self.vbox, True, False)
		self.vpane.show()
		
		# Pane that contains the Statusbox+Chatroom and Listbox
		self.Main = gtk.HPaned()
		self.Main.pack1(self.vpane, True, True)
		self.Main.pack2(userlist_over_button, False, True)
		self.Main.show()

		
		self.chatEntry.grab_focus()
		self.vbox.set_focus_child(self.chatEntry)
		
		self.UpdateColours()

	def saveColumns(self):
		columns = []
		for column in self.UserList.get_columns():
			columns.append(column.get_visible())
		self.frame.Config["columns"]["chatrooms"][self.room] = columns
		
	def autojoinToggled(self, widget):
		
		if not self.frame.Networking.config.has_key("autojoin") or self.remoteSet:
			return
		if self.room in self.frame.Networking.config["autojoin"].keys():
			self.frame.Networking.mod_config("unautojoin", self.room)
		else:
			self.frame.Networking.mod_config("autojoin", self.room)
			
	def Rejoined(self, users):
		for user in users.keys():
			if self.users.has_key(user):
				self.usersmodel.remove(self.users[user])
			img = self.frame.GetStatusImage(user, users[user][0])
			hspeed = self.frame.HSpeed(users[user][1])
			hfiles = Humanize(users[user][3])
			iter = self.usersmodel.append([img, str(user), hfiles, hspeed, users[user][0] ] ) 
			self.users[user] = iter

		AppendLine(self.chatview, _("--- reconnected ---"), None)

	def destroy(self):
		self.Main.destroy()
		
	def ChatEnter(self, something, entry, mtype):

		text = entry.get_text()
		result = None
		#result = expand_alias(self.frame.np.config.aliases, text)
		if result is not None:
			text = result
		if not text:
			widget.set_text("")
			return
			
		s = text.split(" ", 1)
		cmd = s[0]
		if len(s) == 2 and s[1]:
			realargs = args = s[1]
		else:
			args = "" #self.user
			realargs = ""
			
		if text == "":
			return
		if cmd in ["/w", "/whois", "/info"]:
			if args:
				self.frame.GetUserInfo(args)
				self.frame.OnUserInfo(None)
		elif cmd in ["/b", "/browse"]:
			if args:
				self.frame.GetShares(args)
				self.frame.OnUserShares(None)
		elif cmd == "/ip":
			if args:
				if args not in self.frame.ip_requested:
					self.frame.ip_requested.append(args)
					self.frame.Networking.PeerAddress(args)
		elif cmd == "/nsa":
			if args:
				self.frame.GetUserInfo(args)
				self.frame.GetShares(args)
				self.frame.OnUserInfo(None)
				
		elif cmd == "/reload":
			self.frame.Networking.ReloadShares()
			
		elif cmd == "/pm":
			if realargs:
				self.frame.PrivateChats.SendMessage(realargs, None, 1)
		elif cmd in ["/m", "/msg"]:
			if realargs:
				s = realargs.split(" ", 1)
				user = s[0]
				if len(s) == 2:
					msg = s[1]
				else:
					msg = None
				self.frame.PrivateChats.SendMessage(user, msg)
		elif cmd in ["/s", "/search"]:
			if realargs:
				self.frame.Networking.Search(0, realargs)
				self.frame.OnSearch(None)
		elif cmd in ["/us", "/usearch"]:
			s = args.split(" ", 1)
			if len(s) == 2:
				self.frame.Networking.Search(3, s[1], s[0])
				self.frame.OnSearch(None)
			else:
				return
			#if realargs:
				##self.frame.Searches.user = self.user
				#self.frame.Searches.DoSearch(3, realargs)
				#self.frame.OnSearch(None)
		elif cmd in ["/rs", "/rsearch"]:
			if realargs:
				self.frame.Networking.Search(2, realargs)
				self.frame.OnSearch(None)
		elif cmd in ["/bs", "/bsearch"]:
			if realargs:
				self.frame.Networking.Search(1, realargs)
				self.frame.OnSearch(None)
		elif cmd in ["/ad", "/add", "/buddy"]:
			if args:
				self.frame.Networking.mod_config("buddy", args)
		elif cmd in ["/rem", "/unbuddy"]:
			if args:
				self.frame.Networking.mod_config("unbuddy", args)
		elif cmd == "/ban":
			if args:
				self.frame.Networking.mod_config("ban", args)
		elif cmd == "/unban":
			if args:
				self.frame.Networking.mod_config("unban", args)
		elif cmd == "/ignore":
			if args:
				self.frame.Networking.mod_config("ignore", args)
		elif cmd == "/unignore":
			if args:
				self.frame.Networking.mod_config("unignore", args)
		elif cmd == "/trust":
			if args:
				self.frame.Networking.mod_config("trust", args)
		elif cmd == "/distrust":
			if args:
				self.frame.Networking.mod_config("distrust", args)
		elif cmd == "/ctcpversion":
			if args:
				self.frame.Networking.PrivateMessage(0, args, "\x01VERSION\x01")
			
		elif cmd in ["/clear", "/cl"]:
			self.chatview.get_buffer().set_text("")
		elif cmd in ["/a", "/away"]:
			
			self.frame.away_toggle(None)
		elif cmd in ["/q", "/quit"]:
			self.frame.window_quit(None)
		elif cmd in ["/c", "/close", "/l", "/leave", "/part", "/p"]:
			self.OnLeave(None)
		elif cmd == "/updatenormal":
			self.frame.Muscan.UpdateNormal()
		elif cmd == "/updatebuddy":
			self.frame.Muscan.UpdateBuddy()
		elif cmd == "/rescan":
			self.frame.Muscan.RescanNormal()
			self.frame.Muscan.RescanBuddy()
		elif cmd == "/rescannormal":
			self.frame.Muscan.RescanNormal()
		elif cmd == "/rescanbuddy":
			self.frame.Muscan.RescanBuddy()
		
		elif cmd == "/addnormaldir":
			if not args:
				return
			self.frame.Muscan.AddNormalDirectory(args)

		elif cmd == "/removenormaldir":
			if not args:
				return
			self.frame.Muscan.RemoveNormalDirectory(args)

		elif cmd == "/addbuddydir":
			if not args:
				return
			self.frame.Muscan.AddBuddyDirectory(args)

		elif cmd == "/removebuddydir":
			if not args:
				return
			self.frame.Muscan.RemoveBuddyDirectory(args)
		elif cmd in ( "/tick", "/t"):
			# TODO
			pass
		
		elif cmd == "/listshares":
			self.frame.Muscan.ListNormal()
			self.frame.Muscan.ListBuddy()
		elif cmd == "/listnormal":
			self.frame.Muscan.ListNormal()
		elif cmd == "/listbuddy":
			self.frame.Muscan.ListBuddy()
		elif cmd in("/join", "/j"):
			if not args:
				return
			self.frame.JoinARoom(args)
		elif cmd in  ("/c",  "/close", "/leave", "/l"):
			if not args:
				self.OnLeave("")
			self.frame.RoomLeave(args)
		elif cmd and cmd[:1] == "/" and cmd != "/me" and cmd[:2] != "//":
			self.frame.AppendToLogWindow(_("Command %s is not recognized") % text)
			return
		else:
			if text[:2] == "//":
				text = text[1:]

			self.frame.Networking.SayRoom(self.room, text)
		entry.set_text("")
		

				
	COMMANDS = ["/alias ", "/unalias ", "/whois ", "/browse ", "/ip ", "/pm ", "/msg ", "/search ", "/usearch ", "/rsearch ", "/bsearch ", "/join ", "/leave", "/add ", "/buddy ", "/unbuddy ", "/ban ", "/ignore ", "/unban ", "/unignore ", "/clear", "/part ", "/quit", "/rescan", "/tick", "/nsa", "/info", "/ctcpversion", "/rem ", "/cl", "/t", "/a", "/q", "/l", "/p", "/bs", "/rs", "/us", "/s", "/m", "/w", "/al", "/un", "/w", "/updatenormal", "/updatebuddy", "/rescannormal", "/rescanbuddy", "/addnormaldir", "/addbuddydir", "/removenormaldir", "/removebuddydir", "/listshares", "/listnormal", "/listbuddy",  "/close", "/leave", "/join" ]
		
	def SayInChatRoom(self, user, line):

		if len(self.lines) >= 400:
			buffer = self.chatview.get_buffer()
			start = buffer.get_start_iter()
			end = buffer.get_iter_at_line(self.lines[200])
			self.chatview.get_buffer().delete(start, end)
			del self.lines[0:200]
		
		# Display /me messages as "* username message"
		if line[:4] == "/me ":
			message = "* %s %s"  % (user, line[4:])
			tag = self.tag_me
		else:
			message = "[%s] %s" % (user, line)
			if user == self.frame.username:
				tag = self.tag_local
			elif line.upper().find(self.frame.username.upper()) > -1:
				tag = self.tag_hilite
			else:
				tag = self.tag_remote

		message = "\n-- ".join(message.split("\n"))

		if user in self.tag_users.keys():
			usertag = self.tag_users[user]
		else:
			usertag = self.tag_users[user] = self.makecolour(self.chatview.get_buffer(), color, username=user)
		self.lines.append(AppendLine(self.chatview, message, tag=tag, username=user, usertag=usertag))
			
		if self.frame.username is not None:
			if user != self.frame.username and self.frame.username in message:
				self.chatrooms.request_hilite(self.Main)
			else:
				self.chatrooms.request_changed(self.Main)
			self.frame.RequestIcon(self.frame.ChatRoomLabel)
		#if self.Log.get_active():
			#self.logfile = WriteLog(self.logfile, self.frame.np.config.sections["logging"]["logsdir"], self.room, line)

	def OnKeyPress(self, widget, event):
		if event.keyval == gtk.gdk.keyval_from_name("Prior"):
			scrolled = self.chatview.get_parent()
			adj = scrolled.get_vadjustment()
			adj.set_value(adj.value - adj.page_increment)
		elif event.keyval == gtk.gdk.keyval_from_name("Next"):
			scrolled = self.chatview.get_parent()
			adj = scrolled.get_vadjustment()
			max = adj.upper - adj.page_size
			new = adj.value + adj.page_increment
			if new > max:
				new = max
			adj.set_value(new)
			
		## Tab Completion
		if event.keyval != gtk.gdk.keyval_from_name("Tab"):
			return False
		ix = widget.get_position()
		text = widget.get_text()[:ix].split(" ")[-1]
		list = self.users.keys() + [i for i in self.frame.Networking.config["buddies"] ] + ["murmur"]
		if ix == len(text) and text[:1] == "/":
			list += self.COMMANDS
			#["/"+k for k in self.frame.np.config.aliases.keys()] +
		completion, single = self.frame.GetCompletion(text, list)
		if completion:
			if single:
				if ix == len(text) and text[:1] != "/":
					completion += ": "
			widget.insert_text(completion, ix)
			widget.set_position(ix + len(completion))
		widget.emit_stop_by_name("key_press_event")
		return True
				
	def UserJoinedRoom(self, username):
		if self.users.has_key(username):
			return

		status, speed, downloads, files, dirs = self.frame.user_stats[username]
		AppendLine(self.RoomLog, _("%s joined the room") % username, self.tag_log)
		img = self.frame.GetStatusImage(username, status)
		hspeed = self.frame.HSpeed(speed)
		hfiles = Humanize(files)
		iter = self.usersmodel.append([img, username, hfiles, hspeed, status])
		self.users[username] = iter
		color = self.getUserStatusColor(status)
		if username in self.tag_users.keys():
			self.changecolour(self.tag_users[username], color)
		else:
			self.tag_users[username] = self.makecolour(self.chatview.get_buffer(), color, username=username)
	
	def UserLeftRoom(self, username):
		if not self.users.has_key(username):
			return
		AppendLine(self.RoomLog, _("%s left the room") % username, self.tag_log)
		self.usersmodel.remove(self.users[username])
		del self.users[username]
		if username in self.tag_users.keys():
			color = self.getUserStatusColor(-1)
			self.changecolour(self.tag_users[username], color)
		
	def GetListStatus(self, user):
		status = self.usersmodel.get_value(self.users[user], 4)
		img = self.frame.GetStatusImage(user, status)
		self.usersmodel.set(self.users[user], 0, img, 4, status)
		
	def GetUserStats(self, user, speed, files):
		if not self.users.has_key(user):
			return
		hspeed = self.frame.HSpeed(speed)
		hfiles = Humanize(files)
		self.usersmodel.set(self.users[user], 2, hfiles, 3, hspeed)

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

	def makecolour(self, buffer, colour, username=None):
		colour = self.frame.Config["ui"][colour]
		font =  self.frame.Config["ui"]["chatfont"]
		
		if colour:
			tag = buffer.create_tag(foreground = colour, font=font)
		else:
			tag = buffer.create_tag( font=font)
		if username is not None:
			usernamestyle = self.frame.Config["ui"]["usernamestyle"]
			

			if usernamestyle == "bold":
				tag.set_property("weight",  pango.WEIGHT_BOLD)
			else:
				tag.set_property("weight",  pango.WEIGHT_NORMAL)
			if usernamestyle == "italic":
				tag.set_property("style",  pango.STYLE_ITALIC)
			else:
				tag.set_property("style",  pango.STYLE_NORMAL)
			if usernamestyle == "underline":
				tag.set_property("underline", pango.UNDERLINE_SINGLE)
			else:
				tag.set_property("underline", pango.UNDERLINE_NONE)
				
			tag.connect("event", self.UserNameEvent, username)
			tag.last_event_type = -1
		return tag
			
	def UserNameEvent(self, tag, widget, event, iter, user):
		if tag.last_event_type == gtk.gdk.BUTTON_PRESS and event.type == gtk.gdk.BUTTON_RELEASE and event.button == 1:
			if user in self.users.keys():
				self.popup_menu.set_user(user)
				items = self.popup_menu.get_children()
				# Chat, Userlists use the normal popup system
				num = 5
				self.popup_menu.ready = True
				items[num].set_active(self.frame.Networking.config.has_key("buddies") and self.frame.Networking.config["buddies"].has_key(user))
				items[num+1].set_active(self.frame.Networking.config.has_key("banned") and self.frame.Networking.config["banned"].has_key(user))
				items[num+2].set_active(self.frame.Networking.config.has_key("ignored") and self.frame.Networking.config["ignored"].has_key(user))
				items[num+3].set_active(self.frame.Networking.config["trusted"].has_key(user))
				self.popup_menu.popup(None, None, None, event.button, event.time)
				self.popup_menu.ready = False
		tag.last_event_type = event.type
		
	def UpdateColours(self):
		map = self.chatScroll.get_style().copy()
		self.backupcolor = map.text[gtk.STATE_NORMAL]
		buffer = self.chatview.get_buffer()
		self.tag_remote = self.makecolour(buffer, "chatremote")
		self.tag_local = self.makecolour(buffer, "chatlocal")
		self.tag_me = self.makecolour(buffer, "chatme")
		self.tag_hilite = self.makecolour(buffer, "chathilite")
		self.tag_users = {}
		for user in self.users:
			status = self.usersmodel.get_value(self.users[user], 4)
			color = self.getUserStatusColor(status)
			self.tag_users[user] = self.makecolour(buffer, color, user)

		buffer = self.RoomLog.get_buffer()
		self.tag_log = self.makecolour(buffer, "chatremote")
		
	def getUserStatusColor(self, status):
		if status == 1:
			color = "useraway"
		elif status == 2:
			color = "useronline"
		else:
			color = "useroffline"
		return color
	
	def changecolour(self, tag, colour):
		if self.frame.Config["ui"].has_key(colour):
			color = self.frame.Config["ui"][colour]
		else:
			color = ""
		font = self.frame.Config["ui"]["chatfont"]
		
		if color:
			tag.set_property("foreground", color)
			tag.set_property("font", font)
			if colour in ["useraway", "useronline", "useroffline"]:
				tag.set_property("weight",  pango.WEIGHT_BOLD)
		else:
			tag.set_property("font", font)

	def changecolour(self, tag, colour):
		if self.frame.Config["ui"].has_key(colour):
			color = self.frame.Config["ui"][colour]
		else:
			color = ""
		font = self.frame.Config["ui"]["chatfont"]
		
		if color == "":
			color = self.backupcolor
		else:
			color = gtk.gdk.color_parse(color)
		tag.set_property("foreground-gdk", color)
		tag.set_property("font", font)
		# Hotspots
		if colour in ["useraway", "useronline", "useroffline"]:
			usernamestyle = self.frame.Config["ui"]["usernamestyle"]
			if usernamestyle == "bold":
				tag.set_property("weight",  pango.WEIGHT_BOLD)
			else:
				tag.set_property("weight",  pango.WEIGHT_NORMAL)
			if usernamestyle == "italic":
				tag.set_property("style",  pango.STYLE_ITALIC)
			else:
				tag.set_property("style",  pango.STYLE_NORMAL)
			if usernamestyle == "underline":
				tag.set_property("underline", pango.UNDERLINE_SINGLE)
			else:
				tag.set_property("underline", pango.UNDERLINE_NONE)
				
	def ChangeColours(self):
		map = self.chatScroll.get_style().copy()
		self.backupcolor = map.text[gtk.STATE_NORMAL]
		
		self.changecolour(self.tag_log, "chatremote")
		self.changecolour(self.tag_remote, "chatremote")
		self.changecolour(self.tag_local, "chatlocal")
		self.changecolour(self.tag_me, "chatme")
		self.changecolour(self.tag_hilite, "chathilite")

		for user in self.users.keys():
			color = self.getUserStatusColor(self.usersmodel.get_value(self.users[user], 4))
			if user in self.tag_users.keys():
				self.changecolour(self.tag_users[user], color)
			else:
				self.tag_users[user] = self.makecolour(buffer, color, user)

		
	def OnPopupMenuRoom(self, widget, event, string, string2):
		if event.button != 3:
			return
		items = self.popup_menu.get_children()
		#if DEBUG: Output(string)
		# Chat, Userlists use the normal popup system
		num = 5
		if string == "chat":
			
			room = self.room
			if DEBUG:  Output("-- Popup -- Room:", string)
		
			d = self.UserList.get_path_at_pos(int(event.x), int(event.y))
			if not d:
				return
			path, column, x, y = d
			user =  self.usersmodel.get_value(self.usersmodel.get_iter(path), 1)
		self.popup_menu.ready = False
		self.popup_menu.set_user(user)
		items[num].set_active(self.frame.Networking.config.has_key("buddies") and self.frame.Networking.config["buddies"].has_key(user))
		items[num+1].set_active(self.frame.Networking.config.has_key("banned") and self.frame.Networking.config["banned"].has_key(user))
		items[num+2].set_active(self.frame.Networking.config.has_key("ignored") and self.frame.Networking.config["ignored"].has_key(user))
		items[num+3].set_active(self.frame.Networking.config.has_key("trusted") and self.frame.Networking.config["trusted"].has_key(user))
		self.popup_menu.popup(None, None, None, event.button, event.time)
		self.popup_menu.ready = True

	def OnLeave(self, widget = None):
		if self.leaving:
			return
		if self.room in self.chatrooms.joinedrooms:
			self.frame.Networking.LeaveRoom(self.room)
		self.Leave.set_sensitive(False)
		self.leaving = 1
		if self.room in self.frame.Config["logging"]["rooms"]:
			self.frame.Config["logging"]["rooms"].remove(self.room)
		
	def ConnClose(self):
		self.usersmodel.clear()
		self.users = {}
		AppendLine(self.chatview, _("--- disconnected from museek daemon ---"), None)
		



class ChatRooms(IconNotebook):
	
	def __init__(self, frame):
		
		IconNotebook.__init__(self, frame.images)
		self.popup_enable()
		self.frame = frame
		
		self.joinedrooms = {}
		self.rooms = {}
		self.autojoin = 1
		
		self.CreateRoomList()
		
		self.set_tab_pos(gtk.POS_TOP)
		self.set_scrollable(True)
		self.show()
		self.connect("switch-page", self.OnSwitchPage)
	
		
	#def ConnClose(self):
		#self.roomsmaster.ConnClose()
		
	def OnSwitchPage(self, notebook, page, page_num, force = 0):
		if self.frame.notebook_outside.get_current_page() != 0 and not force:
			return
		page = notebook.get_nth_page(page_num)	
		for name, tab in self.joinedrooms.items():
			if tab.Main is page:
				gobject.idle_add(tab.chatEntry.grab_focus)
				break

		#self.frame = frame
		
	def CreateRoomList(self):
		label = gtk.Label(_("Join room:") )
		label.show()
		
		self.entry = gtk.Entry()
		self.entry.set_text("")
		self.entry.grab_focus()
		self.entry.show()
		self.entry.connect("activate", self.EnterRoom, self.entry)
		
		#self.refresh_button = gtk.Button(_("Refresh List") )
		#self.refresh_button.connect_object("clicked", self.OnPopupRefresh, "")
		#self.refresh_button.show()
		self.refresh_button = self.frame.CreateIconButton(gtk.STOCK_REFRESH, "stock", self.OnPopupRefresh, _("Refresh List"))
		
		addhbox = gtk.HBox(spacing=3)
		addhbox.set_border_width(3)
		addhbox.pack_start(label, False, False, 0)
		addhbox.pack_start(self.entry, True, True,  0)
		addhbox.pack_end(self.refresh_button, False, False,  0)
		addhbox.show()
		
		self.store = gtk.ListStore( int, str )

		# Users-in-room Listbox with users and files
		self.treeview  = gtk.TreeView(self.store)
		self.treeview.set_property("rules-hint", True)
		RoomSize = gtk.TreeViewColumn('Size')
		cell = gtk.CellRendererText()
		RoomSize.pack_start(cell, True)
		RoomSize.add_attribute(cell, 'text', 0)
		RoomSize.set_sort_column_id(0)
		RoomSize.set_resizable(True)

		RoomName = gtk.TreeViewColumn(_('Room Name'))
		cell = gtk.CellRendererText()
		RoomName.pack_start(cell, True)
		RoomName.add_attribute(cell, 'text', 1)
		RoomName.set_sort_column_id(1)
		RoomName.set_resizable(True)
		
		#self.treeview.set_search_column(0)
		self.treeview.set_search_column(1)
		
		
		self.treeview.append_column(RoomSize)
		self.treeview.append_column(RoomName)

		
		#self.store.set_sort_column_id(0, gtk.SORT_DESCENDING)
		
		self.popup_menu = popup = PopupMenu(self.frame, "rooms")
		popup.setup(
			("#" + _("_Join Room"), popup.OnJoinRoom, gtk.STOCK_JUMP_TO),
			("#" + _("_Leave Room"), popup.OnLeaveRoom, gtk.STOCK_CLOSE),
					)
		self.treeview.connect("button_press_event", self.OnPopupMenuRoomList)
		self.treeview.show()
		
		# Scrollwindow containing Users-in-room Listbox
		widget = gtk.ScrolledWindow()
		widget.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)
		widget.add(self.treeview)
		widget.set_shadow_type(gtk.SHADOW_ETCHED_IN)
		
		widget.show()
		
		# Containing box with scrolled window and text input box
		self.Main = gtk.VBox(spacing=3)
		self.Main.set_border_width(3)
		self.Main.pack_start(widget, True, True, 0)
		self.Main.pack_end(addhbox, False, False, 0)
		self.Main.show()

		self.frame.notebook_roomlist.add(self.Main)
		
	def saveColumns(self):
		for room in self.frame.Config["columns"]["chatrooms"].keys()[:]:
			if room not in self.joinedrooms.keys():
				del self.frame.Config["columns"]["chatrooms"][room]
		for room in self.joinedrooms.values():
			room.saveColumns()
				
	def EnterRoom(self, widget, event):
		room = self.entry.get_text()
		if room == "":
			return
		self.JoinARoom(room)
		self.entry.set_text("")
	
	def JoinARoom(self, room):
		if room != None and room != "":
			self.frame.Networking.JoinRoom(room)
		
	def UpdateRoomList(self, rooms):
		if DEBUG: Output("UpdateRoomList")
		self.store.clear()
		self.rooms.clear()
		# Vastly speed up/Do not sort
		self.store.set_default_sort_func(lambda *args: -1) 
  		self.store.set_sort_column_id(-1, gtk.SORT_ASCENDING)
		self.treeview.freeze_child_notify()
		
		for room, sizes in rooms.items():
			self.rooms[room] = self.store.append([ sizes, room] )
		# Back to normal/Sort Descending
		self.treeview.thaw_child_notify()
		self.store.set_sort_column_id(0, gtk.SORT_DESCENDING)
		
	def OnResort(self, column, column_id):
		if self.roomsmodel.sort_col == column_id:
			order = self.roomsmodel.sort_order
			if order == gtk.SORT_ASCENDING:
				order = gtk.SORT_DESCENDING
			else:
				order = gtk.SORT_ASCENDING
			column.set_sort_order(order)
			self.roomsmodel.sort_order = order
			self.frame.roomlist.RoomsList.set_model(None)
			self.roomsmodel.sort()
			self.frame.roomlist.RoomsList.set_model(self.roomsmodel)
			return
		cols = self.frame.roomlist.RoomsList.get_columns()
		cols[column_id].set_sort_indicator(True)
		cols[self.roomsmodel.sort_col].set_sort_indicator(False)
		self.roomsmodel.sort_col = column_id
		self.OnResort(column, column_id)
		
	def OnListClicked(self, widget, event):
		if self.roomsmodel is None:
			return False
		if event.button == 1 and event.type == gtk.gdk._2BUTTON_PRESS:
			d = self.frame.roomlist.RoomsList.get_path_at_pos(int(event.x), int(event.y))
			if d:
				path, column, x, y = d
				room = self.roomsmodel.get_value(self.roomsmodel.get_iter(path), 0)

			return True
		elif event.button == 3:
			return self.OnPopupMenuRoomList(widget, event)
		return False
		
	def OnPopupMenuRoomList(self, widget, event):
		if event.button != 3 or self.store is None:
			return
		items = self.popup_menu.get_children()
		d = self.treeview.get_path_at_pos(int(event.x), int(event.y))
		if d:
			path, column, x, y = d
			room = self.store.get_value(self.store.get_iter(path), 1)

			if room in self.joinedrooms.keys():
				act = (False, True)
			else:
				act = (True, False)
		else:
			room = None
			act = (False, False)
		self.popup_menu.set_user(room)
		self.popup_menu.ready = False
		items[0].set_sensitive(act[0])
		items[1].set_sensitive(act[1])
		self.popup_menu.popup(None, None, None, event.button, event.time)
		self.popup_menu.ready = True

	def OnPopupRefresh(self, widget):
		self.frame.Networking.RoomList()
		
	def SayChatRoom(self, room, user, text):
		if self.frame.Networking.config.has_key("ignored") and user in self.frame.Networking.config["ignored"].keys():
			return
		self.joinedrooms[room].SayInChatRoom(user, text)

	
	def JoinRoom(self, room, users):
		if self.joinedrooms.has_key(room):
			self.joinedrooms[room].Rejoined(users)
			return
		
		for user, stats in users.items():
			#Output(stats)
			self.frame.user_stats[user] = [stats[0], stats[1], stats[2], stats[3], stats[4]]
			# online status, avgspeed, numdownloads, numfiles, numdirs
		if self.frame.Config["logging"]["logrooms"] and room not in self.frame.Config["logging"]["rooms"]:
			self.frame.Config["logging"]["rooms"].append(room)
		tab = ChatRoom(self, room, users)
		self.joinedrooms[room] = tab
		#chatlabel = gtk.Label(room) 
		#chatlabel.show()
		self.frame.ChatRooms.append_page(tab.Main, room, tab.OnLeave)
		#self.frame.ChatRooms.append_page(tab.Main, chatlabel)

				
	def GetUserStats(self, user, avgspeed, files):
		for room in self.joinedrooms.values():
                        if user in room.users.keys():
			     room.GetUserStats(user, avgspeed, files)
	
	def GetUserStatus(self, user, status):
		for room in self.joinedrooms.values():
                        if user in room.users.keys():
			     gobject.idle_add(room.GetUserStatus, user, status)
			     
	def GetListStatus(self, user):
		for room in self.joinedrooms.values():
                        if room.users.has_key(user):
			     gobject.idle_add(room.GetListStatus, user)
			
	def UserJoinedRoom(self, room, username):
		if self.joinedrooms.has_key(room):
			self.joinedrooms[room].UserJoinedRoom(username)
	
	def UserLeftRoom(self, room, username):
		self.joinedrooms[room].UserLeftRoom(username)
	
	def TickerSet(self, msg):
		self.joinedrooms[msg.room].TickerSet(msg)

	def TickerAdd(self, msg):
		self.joinedrooms[msg.room].TickerAdd(msg)

	def TickerRemove(self, msg):
		self.joinedrooms[msg.room].TickerRemove(msg)

	def UpdateColours(self):
		for room in self.joinedrooms.values():
			room.ChangeColours()

	def LeaveRoom(self, room):
		roomwidget = self.joinedrooms[room]
		#if room.logfile is not None:
			#room.logfile.close()
			#room.logfile = None
		page_num = self.frame.ChatRooms.page_num( roomwidget.Main)

		self.frame.ChatRooms.remove_page(page_num)
		roomwidget.destroy()
		del self.joinedrooms[room]
		
	def AutoJoin(self, room, on):
		if self.joinedrooms.has_key(room):
			self.joinedrooms[room].remoteSet = True
			self.joinedrooms[room].autojoinCheck.set_active(on)
			self.joinedrooms[room].remoteSet = False
	def ConnClose(self):
		self.roomsmodel = None
		#self.frame.roomlist.RoomsList.set_model(None)
		for room in self.joinedrooms.values():
			room.ConnClose()
		self.autojoin = 1
		
class Logging:
	def __init__(self, frame):
		self.frame = frame
	
	def ChatRoomLog(self, room, user, message):
		
		if room not in self.frame.Config["logging"]["rooms"]:
			return
		text = "[%s]\t%s" % (user, message)
		self.FileLog("rooms", time.strftime("%d %b %Y %H:%M:%S"), room, text )

	def PrivateChatLog(self, direction, user, message):
		
		if not self.frame.Config["logging"]["logprivate"] and user not in self.frame.Config["logging"]["private"]:
			return
		if direction:
			username = self.frame.username
		else:
			username = user
		text = "[%s]\t%s" % (username, message)
		self.FileLog("private", time.strftime("%d %b %Y %H:%M:%S"), user, text )
		
	def FileLog(self, messagetype, timestamp, place, message):
		try:
			if '/' in place:
				place = place.replace("/", "\\")
			path = os.path.join(os.path.expanduser(self.frame.Config["logging"]["log_dir"]), messagetype, place)
			dir = os.path.split(path)[0]
			try:
				if not os.path.isdir(dir):
					os.makedirs(dir)
				f = open(path, "a")
				## replace inline newlines to preserve formatting
				message.replace("\n","\\n")
				f.write("%s %s\n" % (timestamp, message))
				f.close()
			except:
				message = "Cannot write to file %s, check permissions" % path
				Output(message    )
				gobject.idle_add(self.frame.AppendToLogWindow, message)
		except Exception, e:
			Output("FileLog: " + str(e))

class Downloads:
	def __init__(self, frame):
		self.frame = frame
		self.downloads = {}
		self.states = {0: "Finished", 1: "Transferring", 2: "Negotiating", 3:"Waiting", 4: "Establishing", 5: "Initiating", 6: "Connecting",  7: "Queued", 8:"Address", 9:  "Status", 10: "Offline",11: "Closed",12: "Can't Connect", 13: "Aborted", 14: "Remote Error", 15: "Local Error", 16: "Queued"}
		self.Main = gtk.VBox(False, 5)
		self.Main.set_border_width(3)
		self.Main.show()
		self.hbox1 = gtk.HBox(False, spacing=5)
		self.hbox1.set_border_width(3)
		self.hbox1.show()
		self.Main.pack_start(self.hbox1, False, False)
		
		label1 = gtk.Label(("Downloads"))
		label1.set_padding(0, 0)
		label1.show()
		
		self.hbox1.pack_start(label1, False, False)
		
		
		self.ToggleTree = gtk.CheckButton("Group by Users")
		self.ToggleTree.connect("toggled", self.OnToggleTree)
		self.ToggleTree.show()
		#self.frame.CreateIconButton(gtk.STOCK_INDENT, "stock", self.OnToggleTree, "Group by Users")
		self.hbox1.pack_end(self.ToggleTree, False, False)
		
		
		
		
		
		scrolledwindow1 = gtk.ScrolledWindow()
		scrolledwindow1.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		scrolledwindow1.show()
		scrolledwindow1.set_shadow_type(gtk.SHADOW_IN)
		
		self.store = gtk.TreeStore(str, str, str, str, str, str, str, int, int, int )
		
		self.treeview = gtk.TreeView(self.store)
		self.treeview.set_property("rules-hint", True)
		self.treeview.get_selection().set_mode(gtk.SELECTION_MULTIPLE)
		self.create_transfer_columns("downloads")
		self.treeview.set_headers_visible(True)
		
		self.popup_menu = popup = PopupMenu(self.frame, "downloads")
		popup.setup(
			("#" + _("_Retry Transfer(s)"), self.OnRetryDownTransfers, gtk.STOCK_REDO),
			("#" + _("_Abort Transfer(s)"), self.OnAbortDownTransfers, gtk.STOCK_CANCEL),
			("#" + _("_Clear Transfer(s)"), self.OnClearDownTransfers, gtk.STOCK_CLEAR),
			("", None),
			("#" + _("_Private Message"), popup.OnSendMessage, gtk.STOCK_EDIT),
			("#" + _("_User Info"), popup.OnGetUserInfo, gtk.STOCK_INFO),
			("#" + _("IP A_ddress"), popup.OnShowIPaddress, gtk.STOCK_NETWORK),
			("#" + _("Browse _Shares"), popup.OnBrowseUser, gtk.STOCK_HARDDISK),
			("$" + _("_Buddy this user"), popup.OnBuddyUser),
			("$" + _("Ba_n this user"), popup.OnBanUser),
			("$" + _("_Ignore this user"), popup.OnIgnoreUser),
			("$" + _("_Trust this user"), popup.OnTrustUser),
		)
		self.treeview.connect("button_press_event", self.OnPopupMenuDownloads)
		self.treeview.show()
		
		scrolledwindow1.add(self.treeview)
	
		self.Main.pack_start(scrolledwindow1, True, True, 0)
		self.selected_transfers = []
		self.selected_users = []
		self.users = {}
		
		
		self.TreeUsers = self.ToggleTree.get_active()
		
	def OnToggleTree(self, widget):
		self.TreeUsers = self.ToggleTree.get_active()
			
		self.RebuildTransfers()
		
	def RebuildTransfers(self):
		newiters = []
		for iter in self.downloads.values():
			if iter in self.users.values():
				pass
			else:
				newiters.append(iter)
		transfers = []
		for i in newiters:
			user = self.store.get_value(i, 0)
			filename = self.store.get_value(i, 1)
			rate = self.store.get_value(i, 2)
			state = self.store.get_value(i, 3)
			filepos = self.store.get_value(i, 4)
			filesize = self.store.get_value(i, 5)
			path = self.store.get_value(i, 6)
			realRate = self.store.get_value(i, 7)
			realPos = self.store.get_value(i, 8)
			realSize = self.store.get_value(i, 9)
			transfers.append([user, filename, rate, state, filepos, filesize, path, realRate, realPos, realSize])
		self.Clear()
		for transfer in transfers:
			user, filename, rate, state, filepos, filesize, path, realRate, realPos, realSize = transfer
			self.append(user, filename, realRate, state, realPos, realSize, path)
		
	def Clear(self):
		self.users.clear()
		self.selected_transfers = []
		self.selected_users = []
		self.downloads.clear()
		self.store.clear()
		
	def append(self, user, filename=None, rate=None, state=None, filepos=None, filesize=None, path=None, error=None, place=None):
		if state == 14:
			state = error
		elif type(state) in (int, long):
			state = self.states[state]
			
		else:
			if state.isdigit():
				state = self.states[int(state)]
			
		if self.TreeUsers:
			if not self.users.has_key(user):
				self.users[user] = self.store.append(None, [user, "", "", "", "", "", "", 0, 0, 0])
	
		parent = None
		if self.TreeUsers:
			parent = self.users[user]
		iters =  self.store.append(parent, [user, path.split("\\")[-1], self.frame.Humanize(rate), state, self.frame.Humanize(filepos), self.frame.Humanize( filesize),   path, rate, filepos, filesize])
		
		self.downloads[(user, path)] =  iters

		
	def update(self, user, filename=None, rate=None, state=None, filepos=None, filesize=None, path=None, error=None, place=None):
		user_path = (user, path)
		if user_path in self.downloads:
			if state == 14:
				state = error
			else:
				state = self.states[state]
			self.store.set(self.downloads[user_path], 0, user, 1, path.split("\\")[-1], 2,  self.frame.Humanize(rate), 3 ,state, 4, self.frame.Humanize(filepos),  5, self.frame.Humanize(filesize),  6, path)
		else:
			self.append(user, filename, rate, state, filepos, filesize, path)
			
	def SelectedTransfersCallback(self, model, path, iter):
		user = model.get_value(iter, 0)
		file = model.get_value(iter, 6)
		self.selected_transfers.append(iter)
		if user not in self.selected_users:
			self.selected_users.append(user)

				
	def select_transfers(self):
		self.selected_transfers = []
		self.selected_users = []
		self.treeview.get_selection().selected_foreach(self.SelectedTransfersCallback)
		
	def OnRetryDownTransfers(self, widget):
		model = self.store
		self.select_transfers()
		for iter in self.selected_transfers:
			user = model.get_value(iter, 0)
			path = model.get_value(iter, 6)
			if path == "": continue
			self.frame.Networking.DownloadFile(user, path)

		
	
	def OnClearDownTransfers(self, widget):
		model = self.store
		self.select_transfers()
		for iter in self.selected_transfers:
			user = model.get_value(iter, 0)
			path = model.get_value(iter, 6)
			if path == "": continue
			self.frame.Networking.TransferRemove(0, user, path)
			
	def OnAbortDownTransfers(self, widget):
		model = self.store
		self.select_transfers()
		for iter in self.selected_transfers:
			user = model.get_value(iter, 0)
			path = model.get_value(iter, 6)
			if path == "": continue
			self.frame.Networking.TransferAbort(0, user, path)
		
	def remove(self, transfer):
		user_path =  transfer[1], transfer[2]
		if user_path in self.downloads:
			self.store.remove(self.downloads[ user_path ])
			del self.downloads[ user_path ]

			
	
	def OnPopupMenuDownloads(self, widget, event):
		if event.button != 3:
			return
		d = self.treeview.get_path_at_pos(int(event.x), int(event.y))
		if not d:
			return
		path, column, x, y = d
		user = self.store.get_value(self.store.get_iter(path), 0)
		file = self.store.get_value(self.store.get_iter(path), 6)
		if file == "" or path == "":
			return True
		self.popup_menu.set_user(user)
		self.popup_menu.set_path(file)
		
		items = self.popup_menu.get_children()
		num = 8
		self.popup_menu.ready = False
		items[num].set_active(self.frame.Networking.config.has_key("buddies") and self.frame.Networking.config["buddies"].has_key(user))
		items[num+1].set_active(self.frame.Networking.config.has_key("banned") and self.frame.Networking.config["banned"].has_key(user))
		items[num+2].set_active(self.frame.Networking.config.has_key("ignored") and self.frame.Networking.config["ignored"].has_key(user))
		items[num+3].set_active(self.frame.Networking.config.has_key("trusted") and self.frame.Networking.config["trusted"].has_key(user))
		self.popup_menu.popup(None, None, None, event.button, event.time)
		self.popup_menu.ready = True
		self.treeview.emit_stop_by_name("button_press_event")
		return True

	def create_transfer_columns(self, direction):
		try:
			cols = InitialiseColumns(self.treeview,
				[_("Username"), 100, "text"], #0
				[_("Filename"), 250, "text"], #1
				[_("Speed"), 80, "text"], #2
				[_("Status"), 80, "text"], #3
				[_("Pos"), 50, "text"], #4
				[_("Size"), 80, "text"], #5
				[_("Path"), 350, "text"], #6
			)
			cols[0].set_sort_column_id(0)
			cols[1].set_sort_column_id(1)
			cols[2].set_sort_column_id(7)
			cols[3].set_sort_column_id(3)
			cols[4].set_sort_column_id(8)
			cols[5].set_sort_column_id(9)
			cols[6].set_sort_column_id(6)
			self.treeview.set_search_column(1)

			#self.store.set_sort_column_id(0, gtk.SORT_ASCENDING)
		except Exception, e:
			Output(e)

	
class Uploads:
	def __init__(self, frame):
		self.frame = frame
		self.uploads = {}
		self.states = {0: "Finished", 1: "Transferring", 2: "Negotiating", 3:"Waiting", 4: "Establishing", 5: "Initiating", 6: "Connecting",  7: "Queued", 8:"Address", 9:  "Status", 10: "Offline",11: "Closed",12: "Can't Connect", 13: "Aborted", 14: "Remote Error", 15: "Local Error", 16: "Queued"}
		
		
		self.Main = gtk.VBox(False, 5)
		self.Main.set_border_width(3)
		self.Main.show()
		self.hbox1 = gtk.HBox(False, spacing=5)
		self.hbox1.set_border_width(3)
		self.hbox1.show()
		self.Main.pack_start(self.hbox1, False, False)
		
		label1 = gtk.Label(("Uploads"))
		label1.set_padding(0, 0)
		label1.show()
		
		self.hbox1.pack_start(label1, False, False)
		
		#self.ToggleTree = self.frame.CreateIconButton(gtk.STOCK_INDENT, "stock", self.OnToggleTree, "Group by Users")
		self.ToggleTree = gtk.CheckButton("Group by Users")
		self.ToggleTree.connect("toggled", self.OnToggleTree)
		self.ToggleTree.show()
		self.hbox1.pack_end(self.ToggleTree, False, False)

	
		scrolledwindow2 = gtk.ScrolledWindow()
		scrolledwindow2.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		scrolledwindow2.show()
		scrolledwindow2.set_shadow_type(gtk.SHADOW_IN)
		
		self.store = gtk.TreeStore(str, str, str, str, str, str, str, int, int, int )
		self.treeview = gtk.TreeView(self.store)
		self.treeview.get_selection().set_mode(gtk.SELECTION_MULTIPLE)
		self.create_transfer_columns("uploads")
		self.treeview.set_property("rules-hint", True)
		self.treeview.set_headers_visible(True)
		self.popup_menu = popup = PopupMenu(self.frame, "uploads")
		popup.setup(
			("#" + _("_Abort Transfer"), self.OnAbortUpTransfers, gtk.STOCK_CANCEL),
			("#" + _("_Clear Transfer"), self.OnClearUpTransfers, gtk.STOCK_CLEAR),
			("", None),
			("#" + _("_Retry Transfer"), self.OnRetryUpTransfers, gtk.STOCK_CLEAR),
			("", None),
			("#" + _("_Private Message"), popup.OnSendMessage, gtk.STOCK_EDIT),
			("#" + _("_User Info"), popup.OnGetUserInfo, gtk.STOCK_INFO),
			("#" + _("IP A_ddress"), popup.OnShowIPaddress, gtk.STOCK_NETWORK),
			("#" + _("Browse _Shares"), popup.OnBrowseUser, gtk.STOCK_HARDDISK),
			("$" + _("_Buddy this user"), popup.OnBuddyUser),
			("$" + _("Ba_n this user"), popup.OnBanUser),
			("$" + _("_Ignore this user"), popup.OnIgnoreUser),
			("$" + _("_Trust this user"), popup.OnTrustUser),
					)
		self.treeview.connect("button_press_event", self.OnPopupMenuUploads)
		self.treeview.show()
		scrolledwindow2.add(self.treeview)
	
		self.Main.pack_start(scrolledwindow2, True, True, 0)
		self.selected_transfers = []
		self.selected_users = []
		self.TreeUsers = self.ToggleTree.get_active()
		self.users = {}
		
	def Clear(self):
		self.users.clear()
		self.selected_transfers = []
		self.selected_users = []
		self.uploads.clear()
		self.store.clear()
		
	
		
	def OnToggleTree(self, widget):
		self.TreeUsers = self.ToggleTree.get_active()
			
		self.RebuildTransfers()
			
		self.RebuildTransfers()
		
	def RebuildTransfers(self):
		newiters = []
		for iter in self.uploads.values():
			if iter in self.users.values():
				pass
			else:
				newiters.append(iter)
		transfers = []
		for i in newiters:
			user = self.store.get_value(i, 0)
			filename = self.store.get_value(i, 1)
			rate = self.store.get_value(i, 2)
			state = self.store.get_value(i, 3)
			filepos = self.store.get_value(i, 4)
			filesize = self.store.get_value(i, 5)
			path = self.store.get_value(i, 6)
			realRate = self.store.get_value(i, 7)
			realPos = self.store.get_value(i, 8)
			realSize = self.store.get_value(i, 9)
			transfers.append([user, filename, rate, state, filepos, filesize, path, realRate, realPos, realSize])
		self.Clear()
		for transfer in transfers:
			user, filename, rate, state, filepos, filesize, path, realRate, realPos, realSize = transfer
			self.append(user, filename, realRate, state, realPos, realSize, path)
			
	def SelectedTransfersCallback(self, model, path, iter):
		user = model.get_value(iter, 0)
		file = model.get_value(iter, 6)
		self.selected_transfers.append(iter)
		if user not in self.selected_users:
			self.selected_users.append(user)
			
	def select_transfers(self):
		self.selected_transfers = []
		self.selected_users = []
		self.treeview.get_selection().selected_foreach(self.SelectedTransfersCallback)
	## Retry a Upload
	# @param self Uploads
	# @param username Username
	# @param path File path
	def RetryUpload(self, username, path):
		if username == None or path == None:
			return
		path = path.replace("/", "\\")
		self.frame.Networking.UploadFile(username, path)
		#self.mucous.Help.Log("status", "Uploading: [%s] %s" % (username, path))	
		
	def OnRetryUpTransfers(self, widget):
		model = self.store
		self.select_transfers()
		for iter in self.selected_transfers:
			user = model.get_value(iter, 0)
			path = model.get_value(iter, 6)
			if path == "":
				continue
			self.frame.Networking.TransferRemove(1, user, path)
			self.RetryUpload(user, path)

		
	
	def OnClearUpTransfers(self, widget):
		model = self.store
		self.select_transfers()
		for iter in self.selected_transfers:
			user = model.get_value(iter, 0)
			path = model.get_value(iter, 6)
			if path == "":
				continue
			self.frame.Networking.TransferRemove(1, user, path)
			
	def OnAbortUpTransfers(self, widget):
		model = self.store
		self.select_transfers()
		for iter in self.selected_transfers:
			user = model.get_value(iter, 0)
			path = model.get_value(iter, 6)
			if path == "":
				continue
			self.frame.Networking.TransferAbort(1, user, path)
		
	def append(self, user, filename=None, rate=None, state=None, filepos=None, filesize=None, path=None, error=None, place=None):
		user_path = (user, path)
		if state == 14:
			state = error
		elif type(state) in (int, long):
			state = self.states[state]
			
		else:
			if state.isdigit():
				state = self.states[int(state)]
		if self.TreeUsers:
			if not self.users.has_key(user):
				self.users[user] = self.store.append(None, [user, "", "", "", "", "", "", 0, 0, 0])
		parent = None
		if self.TreeUsers:
			parent = self.users[user]
		iters =  self.store.append(parent, [user, path.split("\\")[-1], self.frame.Humanize(rate), state, self.frame.Humanize(filepos), self.frame.Humanize( filesize),   path, rate, filepos, filesize])
		self.uploads[(user, path)] =  iters

		
	def update(self, user, filename=None, rate=None, state=None, filepos=None, filesize=None, path=None, error=None, place=None):
		user_path = (user, path)
		if user_path in self.uploads:
			self.store.set(self.uploads[user_path], 0, user, 1, path.split("\\")[-1], 2, self.frame.Humanize(rate), 3 ,self.states[state], 4, self.frame.Humanize(filepos),  5, self.frame.Humanize(filesize),  6, path, 7, rate, 8, filepos, 9, filesize)
		else:
			self.append(user, filename, rate, state, filepos, filesize, path, error, place)
			
	def remove(self, transfer):
		user_path =  transfer[1], transfer[2]
		if user_path in self.uploads:
			self.store.remove( self.uploads[ user_path ] )
			del self.uploads[ user_path ]

			
			
	def OnPopupMenuUploads(self, widget, event):
		if event.button != 3:
			return
		items = self.popup_menu.get_children()
		
		d = self.treeview.get_path_at_pos(int(event.x), int(event.y))
		if not d:
			return
		path, column, x, y = d
		user = self.store.get_value(self.store.get_iter(path), 0)
		file = self.store.get_value(self.store.get_iter(path), 6)
		if file == "" or path == "":
			return True
		self.popup_menu.set_user(user)
		self.popup_menu.set_path(file)
		self.popup_menu.ready = False
		num = 9
		items[num].set_active(self.frame.Networking.config.has_key("buddies") and self.frame.Networking.config["buddies"].has_key(user))
		items[num+1].set_active(self.frame.Networking.config.has_key("banned") and self.frame.Networking.config["banned"].has_key(user))
		items[num+2].set_active(self.frame.Networking.config.has_key("ignored") and self.frame.Networking.config["ignored"].has_key(user))
		items[num+3].set_active(self.frame.Networking.config.has_key("trusted") and self.frame.Networking.config["trusted"].has_key(user))
		
		
		self.popup_menu.popup(None, None, None, event.button, event.time)
		self.popup_menu.ready = True
		self.treeview.emit_stop_by_name("button_press_event")
		return True
	
	def create_transfer_columns(self, direction):
		try:
			cols = InitialiseColumns(self.treeview,
				[_("Username"), 100, "text"], #0
				[_("Filename"), 250, "text"], #1
				[_("Speed"), 80, "text"], #2
				[_("Status"), 80, "text"], #3
				[_("Pos"), 50, "text"], #4
				[_("Size"), 80, "text"], #5
				[_("Path"), 350, "text"], #6
			)
			
			self.treeview.set_search_column(1)
			cols[0].set_sort_column_id(0)
			cols[1].set_sort_column_id(1)
			cols[2].set_sort_column_id(7)
			cols[3].set_sort_column_id(3)
			cols[4].set_sort_column_id(8)
			cols[5].set_sort_column_id(9)
			cols[6].set_sort_column_id(6)
			#self.store.set_sort_column_id(0, gtk.SORT_ASCENDING)
		except Exception, e:
			Output(e)

class ListsOfUsers(IconNotebook):
	def __init__(self, frame):
		self.frame = frame
		IconNotebook.__init__(self, frame.images)
		
		self.set_border_width(4)
		self.set_tab_pos(gtk.POS_TOP)
		self.set_scrollable(True)
		self.show()

		self.buddies= ListBuddies(self.frame)
		self.append_page(self.buddies.Main, _("Buddies"))
		self.trusted = ListTrusted(self.frame)
		self.append_page(self.trusted.Main, _("Trusted"))
		self.banned = ListBanned(self.frame)
		self.append_page(self.banned.Main, _("Banned"))
		self.ignored = ListIgnored(self.frame)
		self.append_page(self.ignored.Main, _("Ignored"))
		

		
	def updateStats(self, user, stats):
		for w in [ self.buddies, self.trusted, self.banned, self.ignored ]:
			gobject.idle_add(w.updateStats, user, stats)
				
	def updateStatus(self, user, status):
		for w in [ self.buddies, self.trusted, self.banned, self.ignored ]:
			gobject.idle_add(w.updateStatus, user, status)
			
	def updateListIcon(self, user):
		for w in [ self.buddies, self.trusted, self.banned, self.ignored ]:
			gobject.idle_add(w.updateListIcon, user)
class ListBanned:
	def __init__(self, frame):
		# Text input box
		self.frame = frame
		self.entry = entry = gtk.Entry()
		self.users = {}
		entry.set_text("")
		entry.grab_focus()
		entry.show()
		entry.connect("activate", self.EnterBanned, entry)
		
		label = gtk.Label(_("Add User:"))
		label.show()
		
		
		addhbox = gtk.HBox(spacing=3)
		addhbox.set_border_width(3)
		addhbox.pack_start(label, False, False, 0)
		addhbox.pack_end(entry, True, True,  0)
		addhbox.show()
		
		
		self.store = gtk.ListStore(gtk.gdk.Pixbuf, str, str, str, str, int )
		
		self.treeview = UserList = gtk.TreeView(self.store)
		self.treeview.set_property("rules-hint", True)
		cols = InitialiseColumns(self.treeview,
				[_("Status"), -1, "pixbuf"], #0
				[_("Username"), 100, "text"], #1
				[_("Files"), 50, "text"], #2
				[_("Speed"), 80, "text"], #3
				[_("Comments"), 100, "text"], #4
			)
		cols[0].set_sort_column_id(5)
		cols[1].set_sort_column_id(1)
		cols[2].set_sort_column_id(2)
		cols[3].set_sort_column_id(3)
		cols[4].set_sort_column_id(4)
		#self.store.set_sort_column_id(1, gtk.SORT_ASCENDING)
		
		self.popup_menu = popup = PopupMenu(self.frame, "userlists")
		popup.setup(
			("#" + _("_Private Message"), popup.OnSendMessage, gtk.STOCK_EDIT),
			("", None),
			("#" + _("_User Info"), popup.OnGetUserInfo, gtk.STOCK_INFO),
			("#" + _("IP A_ddress"), popup.OnShowIPaddress, gtk.STOCK_NETWORK),
			("#" + _("Browse _Shares"), popup.OnBrowseUser, gtk.STOCK_HARDDISK),
			("$" + _("_Buddy this user"), popup.OnBuddyUser),
			("$" + _("Ba_n this user"), popup.OnBanUser),
			("$" + _("_Ignore this user"), popup.OnIgnoreUser),
			("$" + _("_Trust this user"), popup.OnTrustUser),
		)
		UserList.connect("button_press_event", self.OnPopupMenuBanned)
		UserList.show()
		
		# Scrollwindow containing Users-in-room Listbox
		widget = gtk.ScrolledWindow()
		widget.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)
		widget.add(UserList)
		widget.set_shadow_type(gtk.SHADOW_ETCHED_IN)
		
		widget.show()
		
		# Containing box with scrolled window and text input box
		self.Main= gtk.VBox(spacing=3)
		self.Main.set_border_width(3)
		self.Main.pack_start(widget, True, True, 0)
		self.Main.pack_end(addhbox, False, False, 0)
		self.Main.show()

		# Naming the tab
		self.label = gtk.Label(_("Banned") )
		self.label.show()
		
	def OnPopupMenuBanned(self, widget, event):
		if event.button != 3:
			return
		d = self.treeview.get_path_at_pos(int(event.x), int(event.y))
		if not d:
			return
		path, column, x, y = d
		user =  self.store.get_value(self.store.get_iter(path), 1)
		self.popup_menu.set_user(user)
		items = self.popup_menu.get_children()
		num = 5
		self.popup_menu.ready = False
		items[num].set_active(self.frame.Networking.config.has_key("buddies") and self.frame.Networking.config["buddies"].has_key(user))
		items[num+1].set_active(self.frame.Networking.config.has_key("banned") and self.frame.Networking.config["banned"].has_key(user))
		items[num+2].set_active(self.frame.Networking.config.has_key("ignored") and self.frame.Networking.config["ignored"].has_key(user))
		items[num+3].set_active(self.frame.Networking.config.has_key("trusted") and self.frame.Networking.config["trusted"].has_key(user))
		self.popup_menu.popup(None, None, None, event.button, event.time)
		self.popup_menu.ready = True
		self.treeview.emit_stop_by_name("button_press_event")
		return True

	def Fill(self, dict):
		self.store.set_default_sort_func(lambda *args: -1) 
		self.store.set_sort_column_id(-1, gtk.SORT_ASCENDING)
		for user, comment in dict.items():
			self.append(user, comment)
		self.store.set_sort_column_id(1, gtk.SORT_ASCENDING)
		
	def EnterBanned(self, widget, whatever):
		user = self.entry.get_text()
		if user != "":
			self.frame.Networking.mod_config("ban", user)
		self.entry.set_text("")
		if user not in self.user_stats:
			self.frame.Networking.PeerExists(user)
				
	def updateStatus(self, user, status):
                if user not in self.users.keys():
                    return
                if status == self.store.get_value(self.users[user], 5):
                  return
                img = self.frame.GetStatusImage(user, status)
		self.store.set(self.users[user], 0, img, 5, status)
		
	def updateListIcon(self, user):
		if user not in self.users.keys():
			return
		status = self.store.get_value(self.users[user], 5)
		img = self.frame.GetStatusImage(user, status)
		self.store.set(self.users[user], 0, img, 5, status)
		
	def updateStats(self, user, stats):
                if user not in self.users.keys():
                    return
		status, speed, downloads, files, dirs = stats 
		self.store.set(self.users[user], 1, user, 2, Humanize(files), 3, self.frame.HSpeed(speed))
		
	def update(self, user, comments):
		if user not in self.users:
			return
		self.store.set(self.users[user], 1, user,  4, comments )
		
	def append(self, user, comments):
		if user in self.users:
			return
		if user in self.frame.user_stats:
			status, speed, downloads, files, dirs = self.frame.user_stats[user]
		else:
			status = speed = downloads = files = dirs = 0
		self.users[user] = self.store.append([ self.frame.GetStatusImage(user, status), user,  Humanize(files), self.frame.HSpeed(speed), comments, status ])

	def remove(self, user):
		self.store.remove(self.users[user])
		del self.users[user]
	def clear(self, user):
		self.store.clear()
	
class ListBuddies:
	def __init__(self, frame):
		# Text input box
		self.frame = frame
		self.entry = entry = gtk.Entry()
		self.users = {}
		entry.set_text("")
		entry.grab_focus()
		entry.show()
		entry.connect("activate", self.EnterBuddy)
		
		label = gtk.Label(_("Add User:"))
		label.show()
		
		
		addhbox = gtk.HBox(spacing=3)
		addhbox.set_border_width(3)
		addhbox.pack_start(label, False, False, 0)
		addhbox.pack_end(entry, True, True,  0)
		addhbox.show()
		
		
		self.store = gtk.ListStore(gtk.gdk.Pixbuf, str,str,str, str, int )
		
		self.treeview = UserList = gtk.TreeView(self.store)
		self.treeview.set_property("rules-hint", True)
		cols = InitialiseColumns(self.treeview,
				[_("Status"), -1, "pixbuf"], #0
				[_("Username"), 100, "text"], #1
				[_("Files"), 50, "text"], #2
				[_("Speed"), 80, "text"], #3
				[_("Comments"), 100, "text"], #4
			)
		cols[0].set_sort_column_id(5)
		cols[1].set_sort_column_id(1)
		cols[2].set_sort_column_id(2)
		cols[3].set_sort_column_id(3)
		cols[4].set_sort_column_id(4)

		
		#self.store.set_sort_column_id(1, gtk.SORT_ASCENDING)
		
		self.popup_menu = popup = PopupMenu(self.frame, "userlists")
		popup.setup(
			("#" + _("_Private Message"), popup.OnSendMessage, gtk.STOCK_EDIT),
			("", None),
			("#" + _("_User Info"), popup.OnGetUserInfo, gtk.STOCK_INFO),
			("#" + _("IP A_ddress"), popup.OnShowIPaddress, gtk.STOCK_NETWORK),
			("#" + _("Browse _Shares"), popup.OnBrowseUser, gtk.STOCK_HARDDISK),
			("$" + _("_Buddy this user"), popup.OnBuddyUser),
			("$" + _("Ba_n this user"), popup.OnBanUser),
			("$" + _("_Ignore this user"), popup.OnIgnoreUser),
			("$" + _("_Trust this user"), popup.OnTrustUser),
		)
		UserList.connect("button_press_event", self.OnPopupMenuBuddies)
		UserList.show()
		
		# Scrollwindow containing Users-in-room Listbox
		widget = gtk.ScrolledWindow()
		widget.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)
		widget.add(UserList)
		widget.set_shadow_type(gtk.SHADOW_ETCHED_IN)
		
		widget.show()
		
		# Containing box with scrolled window and text input box
		self.Main= gtk.VBox(spacing=3)
		self.Main.set_border_width(3)
		self.Main.pack_start(widget, True, True, 0)
		self.Main.pack_end(addhbox, False, False, 0)
		self.Main.show()
	
		# Naming the tab
		self.label = gtk.Label("Buddies" ) 
		self.label.show()
		
	def OnPopupMenuBuddies(self, widget, event):
		if event.button != 3:
			return
		d = self.treeview.get_path_at_pos(int(event.x), int(event.y))
		if not d:
			return
		path, column, x, y = d
		user =  self.store.get_value(self.store.get_iter(path), 1)
		self.popup_menu.set_user(user)
		items = self.popup_menu.get_children()
		num = 5
		self.popup_menu.ready = False
		items[num].set_active(self.frame.Networking.config.has_key("buddies") and self.frame.Networking.config["buddies"].has_key(user))
		items[num+1].set_active(self.frame.Networking.config.has_key("banned") and self.frame.Networking.config["banned"].has_key(user))
		items[num+2].set_active(self.frame.Networking.config.has_key("ignored") and self.frame.Networking.config["ignored"].has_key(user))
		items[num+3].set_active(self.frame.Networking.config.has_key("trusted") and self.frame.Networking.config["trusted"].has_key(user))
		self.popup_menu.popup(None, None, None, event.button, event.time)
		self.popup_menu.ready = True
		#self.treeview.emit_stop_by_name("button_press_event")
		return True
	
	def Fill(self, dict):
		self.store.set_default_sort_func(lambda *args: -1) 
		self.store.set_sort_column_id(-1, gtk.SORT_ASCENDING)
		for user, comment in dict.items():
			self.append(user, comment)
		self.store.set_sort_column_id(1, gtk.SORT_ASCENDING)
		
	def EnterBuddy(self, widget):
		user = self.entry.get_text()
		if user != "":
			self.frame.Networking.mod_config("buddy", user)
		self.entry.set_text("")
	
	def updateStatus(self, user, status):
                if user not in self.users.keys():
                    return
		if status == self.store.get_value(self.users[user], 5):
                  return
                img = self.frame.GetStatusImage(user, status)
		self.store.set(self.users[user], 0, img, 5, status)
		
	def updateListIcon(self, user):
		if user not in self.users.keys():
			return
		status = self.store.get_value(self.users[user], 5)
		img = self.frame.GetStatusImage(user, status)
		self.store.set(self.users[user], 0, img, 5, status)
		
	def updateStats(self, user, stats):
                if user not in self.users.keys():
                    return
		status, speed, downloads, files, dirs = stats 
		self.store.set(self.users[user], 1, user, 2, Humanize(files), 3, self.frame.HSpeed(speed))
		
	def update(self, user, comments):
		if user not in self.users:
			return
		self.store.set(self.users[user], 1, user,  4, comments )
		
	def append(self, user, comments):
		if user in self.users:
			return
		if user in self.frame.user_stats:
			status, speed, downloads, files, dirs = self.frame.user_stats[user]
		else:
			status = speed = downloads = files = dirs = 0

		self.users[user] = self.store.append([ self.frame.GetStatusImage(user, status), user,  Humanize(files), self.frame.HSpeed(speed), comments, status ])
		
	def remove(self, user):
		self.store.remove(self.users[user])
		del self.users[user]
		
	def clear(self, user):
		self.store.clear()
		
class ListTrusted:
	def __init__(self, frame):
		# Text input box
		self.users = {}
		self.frame = frame
		self.entry = entry = gtk.Entry()
		entry.set_text("")
		entry.grab_focus()
		entry.show()
		entry.connect("activate", self.EnterTrusted, entry)
		
		label = gtk.Label(_("Add User:"))
		label.show()
		
		
		addhbox = gtk.HBox(spacing=3)
		addhbox.set_border_width(3)
		addhbox.pack_start(label, False, False, 0)
		addhbox.pack_end(entry, True, True,  0)
		addhbox.show()
		
		
		self.store = gtk.ListStore(gtk.gdk.Pixbuf, str, str, str, str, int )
		
		self.treeview = UserList = gtk.TreeView(self.store)
		self.treeview.set_property("rules-hint", True)
		cols = InitialiseColumns(self.treeview,
				[_("Status"), -1, "pixbuf"], #0
				[_("Username"), 100, "text"], #1
				[_("Files"), 50, "text"], #2
				[_("Speed"), 80, "text"], #3
				[_("Comments"), 100, "text"], #4
			)
		cols[0].set_sort_column_id(5)
		cols[1].set_sort_column_id(1)
		cols[2].set_sort_column_id(2)
		cols[3].set_sort_column_id(3)
		cols[4].set_sort_column_id(4)

		
		
		
		self.popup_menu = popup = PopupMenu(self.frame, "userlists")
		popup.setup(
			("#" + _("_Private Message"), popup.OnSendMessage, gtk.STOCK_EDIT),
			("", None),
			("#" + _("_User Info"), popup.OnGetUserInfo, gtk.STOCK_INFO),
			("#" + _("IP A_ddress"), popup.OnShowIPaddress, gtk.STOCK_NETWORK),
			("#" + _("Browse _Shares"), popup.OnBrowseUser, gtk.STOCK_HARDDISK),
			("$" + _("_Buddy this user"), popup.OnBuddyUser),
			("$" + _("Ba_n this user"), popup.OnBanUser),
			("$" + _("_Ignore this user"), popup.OnIgnoreUser),
			("$" + _("_Trust this user"), popup.OnTrustUser),
		)
		UserList.connect("button_press_event", self.OnPopupMenuTrusted)
		UserList.show()
		
		# Scrollwindow containing Users-in-room Listbox
		widget = gtk.ScrolledWindow()
		widget.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)
		widget.add(UserList)
		widget.set_shadow_type(gtk.SHADOW_ETCHED_IN)
		
		widget.show()
		
		# Containing box with scrolled window and text input box
		self.Main= gtk.VBox(spacing=3)
		self.Main.set_border_width(3)
		self.Main.pack_start(widget, True, True, 0)
		self.Main.pack_end(addhbox, False, False, 0)
		self.Main.show()
		
		#vseparator = gtk.VSeparator()
		#vseparator.show()
		
		# Naming the tab
		self.label = gtk.Label(_("Trusted") )
		self.label.show()
		
	def OnPopupMenuTrusted(self, widget, event):
		if event.button != 3:
			return
		d = self.treeview.get_path_at_pos(int(event.x), int(event.y))
		if not d:
			return
		path, column, x, y = d
		user =  self.store.get_value(self.store.get_iter(path), 1)
		self.popup_menu.set_user(user)
		items = self.popup_menu.get_children()
		num = 5
		self.popup_menu.ready = False
		items[num].set_active(self.frame.Networking.config.has_key("buddies") and self.frame.Networking.config["buddies"].has_key(user))
		items[num+1].set_active(self.frame.Networking.config.has_key("banned") and self.frame.Networking.config["banned"].has_key(user))
		items[num+2].set_active(self.frame.Networking.config.has_key("ignored") and self.frame.Networking.config["ignored"].has_key(user))
		items[num+3].set_active(self.frame.Networking.config.has_key("trusted") and self.frame.Networking.config["trusted"].has_key(user))
		self.popup_menu.popup(None, None, None, event.button, event.time)
		self.popup_menu.ready = True
		self.treeview.emit_stop_by_name("button_press_event")
		return True
	
	def Fill(self, dict):
		self.store.set_default_sort_func(lambda *args: -1) 
		self.store.set_sort_column_id(-1, gtk.SORT_ASCENDING)
		for user, comment in dict.items():
			self.append(user, comment)
		self.store.set_sort_column_id(1, gtk.SORT_ASCENDING)
		
	def EnterTrusted(self, widget, whatever):
		user = self.entry.get_text()
		if user != "":
			self.frame.Networking.mod_config("trust", user)
		self.entry.set_text("")
	
	def updateStatus(self, user, status):
                if user not in self.users.keys():
                    return
		if status == self.store.get_value(self.users[user], 5):
                  return
                img = self.frame.GetStatusImage(user, status)
		self.store.set(self.users[user], 0, img, 5, status)
		
	def updateListIcon(self, user):
		if user not in self.users.keys():
			return
		status = self.store.get_value(self.users[user], 5)
		img = self.frame.GetStatusImage(user, status)
		self.store.set(self.users[user], 0, img, 5, status)
		
	def updateStats(self, user, stats):
                if user not in self.users.keys():
                    return
		status, speed, downloads, files, dirs = stats 
		self.store.set(self.users[user], 1, user, 2, Humanize(files), 3, self.frame.HSpeed(speed))
		
	def update(self, user, comments):
		if user not in self.users:
			return
		self.store.set(self.users[user], 1, user,  4, comments )
		
	def append(self, user, comments):
		if user in self.users:
			return
		if user in self.frame.user_stats:
			status, speed, downloads, files, dirs = self.frame.user_stats[user]
		else:
			status = speed = downloads = files = dirs = 0

		self.users[user] = self.store.append([ self.frame.GetStatusImage(user, status), user,  Humanize(files), self.frame.HSpeed(speed), comments, status ])
		
	def remove(self, user):
		self.store.remove(self.users[user])
		del self.users[user]
		
	def clear(self, user):
		self.store.clear()
		
class ListIgnored:
	def __init__(self, frame):
		self.users = {}
		self.frame = frame
		self.entry = entry = gtk.Entry()
		entry.set_text("")
		entry.grab_focus()
		entry.show()
		entry.connect("activate", self.EnterIgnored, entry)
		
		label = gtk.Label(_("Add User:"))
		label.show()
		
		
		addhbox = gtk.HBox(spacing=3)
		addhbox.set_border_width(3)
		addhbox.pack_start(label, False, False, 0)
		addhbox.pack_end(entry, True, True,  0)
		addhbox.show()
		
		
		self.store = gtk.ListStore(gtk.gdk.Pixbuf, str, str, str, str, int )
		
		self.treeview = UserList = gtk.TreeView(self.store)
		self.treeview.set_property("rules-hint", True)
		cols = InitialiseColumns(self.treeview,
				[_("Status"), -1, "pixbuf"], #0
				[_("Username"), 100, "text"], #1
				[_("Files"), 50, "text"], #2
				[_("Speed"), 80, "text"], #3
				[_("Comments"), 100, "text"], #4
			)
		cols[0].set_sort_column_id(5)
		cols[1].set_sort_column_id(1)
		cols[2].set_sort_column_id(2)
		cols[3].set_sort_column_id(3)
		cols[4].set_sort_column_id(4)

		
		#self.store.set_sort_column_id(1, gtk.SORT_ASCENDING)
		
		self.popup_menu = popup = PopupMenu(self.frame, "userlists")
		popup.setup(
			("#" + _("_Private Message"), popup.OnSendMessage, gtk.STOCK_EDIT),
			("", None),
			("#" + _("_User Info"), popup.OnGetUserInfo, gtk.STOCK_INFO),
			("#" + _("IP A_ddress"), popup.OnShowIPaddress, gtk.STOCK_NETWORK),
			("#" + _("Browse _Shares"), popup.OnBrowseUser, gtk.STOCK_HARDDISK),
			("$" + _("_Buddy this user"), popup.OnBuddyUser),
			("$" + _("Ba_n this user"), popup.OnBanUser),
			("$" + _("_Ignore this user"), popup.OnIgnoreUser),
			("$" + _("_Trust this user"), popup.OnTrustUser),
		)
		UserList.connect("button_press_event", self.OnPopupMenuIgnored)
		UserList.show()
		
		# Scrollwindow containing Users-in-room Listbox
		widget = gtk.ScrolledWindow()
		widget.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)
		widget.add(UserList)
		widget.set_shadow_type(gtk.SHADOW_ETCHED_IN)
		
		widget.show()
		
		# Containing box with scrolled window and text input box
		self.Main= gtk.VBox(spacing=3)
		self.Main.set_border_width(3)
		self.Main.pack_start(widget, True, True, 0)
		self.Main.pack_end(addhbox, False, False, 0)
		self.Main.show()
		
		#vseparator = gtk.VSeparator()
		#vseparator.show()
		
		# Naming the tab
		self.label = gtk.Label(_("Ignored") )
		self.label.show()

	def OnPopupMenuIgnored(self, widget, event):
		if event.button != 3:
			return
		d = self.treeview.get_path_at_pos(int(event.x), int(event.y))
		if not d:
			return
		path, column, x, y = d
		user =  self.store.get_value(self.store.get_iter(path), 1)
		self.popup_menu.set_user(user)
		items = self.popup_menu.get_children()
		num = 5 
		self.popup_menu.ready = False
		items[num].set_active(self.frame.Networking.config.has_key("buddies") and self.frame.Networking.config["buddies"].has_key(user))
		items[num+1].set_active(self.frame.Networking.config.has_key("banned") and self.frame.Networking.config["banned"].has_key(user))
		items[num+2].set_active(self.frame.Networking.config.has_key("ignored") and self.frame.Networking.config["ignored"].has_key(user))
		items[num+3].set_active(self.frame.Networking.config.has_key("trusted") and self.frame.Networking.config["trusted"].has_key(user))
		self.popup_menu.popup(None, None, None, event.button, event.time)
		self.popup_menu.ready = True
		self.treeview.emit_stop_by_name("button_press_event")
		return True
	
	def Fill(self, dict):
		self.store.set_default_sort_func(lambda *args: -1) 
		self.store.set_sort_column_id(-1, gtk.SORT_ASCENDING)
		for user, comment in dict.items():
			self.append(user, comment)
		self.store.set_sort_column_id(1, gtk.SORT_ASCENDING)
		
	def EnterIgnored(self, widget, whatever):
		user = self.entry.get_text()
		if user != "":
			self.frame.Networking.mod_config("ignore", user)
		self.entry.set_text("")
	
	def updateStatus(self, user, status):
                if user not in self.users.keys():
                    return
		if status == self.store.get_value(self.users[user], 5):
                  return
                img = self.frame.GetStatusImage(user, status)
		self.store.set(self.users[user], 0, img, 5, status)
		
	def updateListIcon(self, user):
		if user not in self.users.keys():
			return
		status = self.store.get_value(self.users[user], 5)
		img = self.frame.GetStatusImage(user, status)
		self.store.set(self.users[user], 0, img, 5, status)
		
	def updateStats(self, user, stats):
                if user not in self.users.keys():
                    return
		status, speed, downloads, files, dirs = stats 
		self.store.set(self.users[user], 1, user, 2, Humanize(files), 3, self.frame.HSpeed(speed))
		
	def update(self, user, comments):
		if user not in self.users:
			return
		self.store.set(self.users[user], 1, user,  4, comments )
		
	def append(self, user, comments):
		if user in self.users:
			return
		if user in self.frame.user_stats:
			status, speed, downloads, files, dirs = self.frame.user_stats[user]
		else:
			status = speed = downloads = files = dirs = 0

		self.users[user] = self.store.append([ self.frame.GetStatusImage(user, status), user,  Humanize(files), self.frame.HSpeed(speed), comments, status ])
			
	def remove(self, user):
		self.store.remove(self.users[user])
		del self.users[user]
		
	def clear(self, user):
		self.store.clear()
		
class Recommendations:
	def __init__(self, frame):
		self.frame = frame
		self.users = {}
		self.recommendations = {}
		self.liked = {}
		self.hated = {}
		
		
		self.vbox1 = gtk.VBox(homogeneous=False, spacing=0)
		self.vbox1.show()
		
		self.hbox1 = gtk.HBox(homogeneous=False, spacing=5)
		self.hbox1.show()
		self.hbox1.set_spacing(5)
		self.hbox1.set_border_width(5)
		# Global Recommendations Button
		self.GlobalRecommendationsButton = gtk.Button()
		self.GlobalRecommendationsButton.show()
		
		self.hbox2 = gtk.HBox(homogeneous=False, spacing=3)
		self.hbox2.show()
		
		self.image1 = gtk.Image()
		self.image1.set_from_stock(gtk.STOCK_REFRESH, 4)
		self.image1.show()
		self.hbox2.pack_start(self.image1)
		
		self.label1 = gtk.Label(_("Global Recommendations"))
		self.label1.show()
		self.hbox2.pack_start(self.label1)
		
		self.GlobalRecommendationsButton.add(self.hbox2)
		# Recommendations Button
		self.RecommendationsButton = gtk.Button()
		self.RecommendationsButton.show()
		
		self.hbox4 = gtk.HBox(homogeneous=False, spacing=3)
		self.hbox4.show()
		
		self.image3 = gtk.Image()
		self.image3.set_from_stock(gtk.STOCK_REFRESH, 4)
		self.image3.show()
		self.hbox4.pack_start(self.image3)
		
		self.label3 = gtk.Label(_("Recommendations"))
		self.label3.show()
		self.hbox4.pack_start(self.label3)
		
		self.RecommendationsButton.add(self.hbox4)
		# Similar Users Button
		self.SimilarUsersButton = gtk.Button()
		self.SimilarUsersButton.show()
		
		self.hbox3 = gtk.HBox(homogeneous=False, spacing=3)
		self.hbox3.show()
		
		self.image2 = gtk.Image()
		self.image2.set_from_stock(gtk.STOCK_REFRESH, 4)
		self.image2.show()
		self.hbox3.pack_start(self.image2)
		
		self.label2 = gtk.Label(_("Similar Users"))
		self.label2.show()
		self.hbox3.pack_start(self.label2)
		
		self.SimilarUsersButton.add(self.hbox3)
		
		# Pack
		self.SimilarUsersButton.connect("clicked", self.GetSimilarUsers)
		self.RecommendationsButton.connect("clicked", self.GetRecommendations)
		self.GlobalRecommendationsButton.connect("clicked", self.GetGlobalRecommendations)
		
		self.hbox1.pack_end(self.SimilarUsersButton, False, False, 0)
		self.hbox1.pack_end(self.RecommendationsButton, False, False, 0)
		self.hbox1.pack_end(self.GlobalRecommendationsButton, False, False, 0)
		
		self.vbox1.pack_start(self.hbox1, False, False, 0)
		
		# Interests, Recommendations, and Similar Users Lists
		self.hpaned1 = gtk.HPaned()
		self.hpaned1.show()
		self.hpaned1.set_border_width(5)
		
		self.vbox2 = gtk.VBox(homogeneous=False, spacing=5)
		self.vbox2.show()
		# Liked Items
		self.LikesStore = gtk.ListStore( str )
		self.LikesTreeView = gtk.TreeView(self.LikesStore)
		cols0 = InitialiseColumns(self.LikesTreeView, 
			[_("I Like..."), 0, "text"], #1
		)
		cols0[0].set_sort_column_id(0)
		self.likes_popup_menu = popup = PopupMenu(self.frame, "likes")
		popup.setup(
			("#" + _("_Remove this item"), self.OnRemoveLikedInterest, gtk.STOCK_CANCEL),
			("#" + _("Re_commendations for this item"), self.OnInterestRecommendations, gtk.STOCK_INDEX),
		)
		self.LikesTreeView.connect("button_press_event", self.OnPopupLikes)
		self.LikesSW = gtk.ScrolledWindow()
		self.LikesSW.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		
		self.LikesSW.add(self.LikesTreeView)
		self.LikesSW.show()
		self.vbox2.pack_start(self.LikesSW)
		
		self.AddLikedButton = self.frame.CreateIconButton(gtk.STOCK_ADD, "stock", self.OnAddLikedInterest, "Add")
	
		self.vbox2.pack_start(self.AddLikedButton, False, False)
		# Hated Items
		self.HatesStore = gtk.ListStore( str )
	
		self.HatesTreeView = gtk.TreeView(self.HatesStore)
		cols1 = InitialiseColumns(self.HatesTreeView, 
			[_("I Hate..."), 0, "text"], #1
		)
		cols1[0].set_sort_column_id(0)
		self.hated_popup_menu = popup = PopupMenu(self.frame, "hated")
		popup.setup(("#" + _("Remove this item"), self.OnRemoveHatedInterest, gtk.STOCK_CANCEL))
		self.HatesTreeView.connect("button_press_event", self.OnPopupHated)
		
		self.HatesSW = gtk.ScrolledWindow()
		self.HatesSW.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.HatesSW.add(self.HatesTreeView)
		self.HatesSW.show()
		
		self.vbox2.pack_start(self.HatesSW)
		
		self.AddHatedButton = self.frame.CreateIconButton(gtk.STOCK_ADD, "stock", self.OnAddHatedInterest, "Add")

		self.vbox2.pack_start(self.AddHatedButton, False, False)
		
		self.hpaned1.pack1(self.vbox2, False, False)
		
		self.hpaned2 = gtk.HPaned()
		self.hpaned2.show()
		# Recommendations
		self.RecommendationsStore = gtk.ListStore( str, int )
		self.RecommendationsTreeView = gtk.TreeView(self.RecommendationsStore)
		
		cols2 = InitialiseColumns(self.RecommendationsTreeView, 
			[_("Recommendations"), 150, "text"], #1
			[_("Rating"), 0, "text"], #2
		)

		cols2[0].set_sort_column_id(0)
		cols2[1].set_sort_column_id(1)
		
		self.RecommendationsSW = gtk.ScrolledWindow()
		self.RecommendationsSW.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.RecommendationsSW.add(self.RecommendationsTreeView)
		self.RecommendationsSW.show()
		
		
		self.rec_popup_menu = popup = PopupMenu(self.frame, "recommendations")
		popup.setup(
			("$" + _("I _like this"), self.OnAddLikeRecommendation),
			("$" + _("I _don't like this"), self.OnAddHatedRecommendation),
			("#" + _("_Recommendations for this item"), self.OnItemRecommendations, gtk.STOCK_INDEX),
			("", None),
			("#" + _("_Search for this item"), self.OnSearchRecommendations, gtk.STOCK_FIND),
		)
		self.RecommendationsTreeView.connect("button_press_event", self.OnPopupRecommendation)	
		self.hpaned2.pack1(self.RecommendationsSW, False, False)
		# Similar Users
		
		self.SimilarUsersStore = gtk.ListStore(gtk.gdk.Pixbuf, str, str, str, int )
		self.SimilarUsersTreeView = gtk.TreeView(self.SimilarUsersStore)
		self.SimilarUsersTreeView.set_property("rules-hint", True)
		# Users-in-room Listbox with users and files
		
		cols = InitialiseColumns(self.SimilarUsersTreeView, 
			["", -1, "pixbuf"], #0
			[_("Username"), 100, "text"], #1
			[_("Files"), 0, "text"], #2
			[_("Speed"), 0, "text"], #3
		)
		
		cols[0].set_sort_column_id(4)
		cols[1].set_sort_column_id(1)
		cols[2].set_sort_column_id(2)
		cols[3].set_sort_column_id(3)
		
		self.users_popup_menu = popup = PopupMenu(self.frame, "similar_users")
		popup.setup(
			("#" + _("Private _Message"), popup.OnSendMessage, gtk.STOCK_EDIT),
			("", None),
			("#" + _("Show IP address"), popup.OnShowIPaddress, gtk.STOCK_NETWORK),
			("#" + _("Get user info"), popup.OnGetUserInfo, gtk.STOCK_INFO),
			("#" + _("Browse files"), popup.OnBrowseUser, gtk.STOCK_HARDDISK),
			("$" + _("_Buddy this user"), popup.OnBuddyUser),
			("$" + _("Ba_n this user"), popup.OnBanUser),
			("$" + _("_Ignore this user"), popup.OnIgnoreUser),
			("$" + _("_Trust this user"), popup.OnTrustUser),
		)
		self.SimilarUsersTreeView.connect("button_press_event", self.OnPopupSimilarUsers)
		
		self.SimilarUsersSW = gtk.ScrolledWindow()
		self.SimilarUsersSW.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.SimilarUsersSW.add(self.SimilarUsersTreeView)
		self.SimilarUsersSW.show()
		
		self.hpaned2.pack2(self.SimilarUsersSW, False, False)
		
		self.hpaned1.pack2(self.hpaned2, False, False)
		
		self.vbox1.pack_start(self.hpaned1)
		
		self.Main = self.vbox1

	def OnPopupHated(self, widget, event):
		if event.button != 3:
			return
		items = self.hated_popup_menu.get_children()
		d = self.HatesTreeView.get_path_at_pos(int(event.x), int(event.y))
		if not d:
			return
		path, column, x, y = d
		interest = self.HatesStore.get_value(self.HatesStore.get_iter(path), 0)
		self.hated_popup_menu.set_user(interest)
		self.hated_popup_menu.popup(None, None, None, event.button, event.time)
		self.HatesTreeView.emit_stop_by_name("button_press_event")
		return True
	
	def OnPopupLikes(self, widget, event):
		if event.button != 3:
			return
		items = self.likes_popup_menu.get_children()
		d = self.LikesTreeView.get_path_at_pos(int(event.x), int(event.y))
		if not d:
			return
		path, column, x, y = d
		interest = self.LikesStore.get_value(self.LikesStore.get_iter(path), 0)
		self.likes_popup_menu.set_user(interest)
		self.likes_popup_menu.popup(None, None, None, event.button, event.time)
		self.LikesTreeView.emit_stop_by_name("button_press_event")
		return True
	
	def OnPopupRecommendation(self, widget, event):
		if event.button != 3:
			return
		items = self.rec_popup_menu.get_children()
		d = self.RecommendationsTreeView.get_path_at_pos(int(event.x), int(event.y))
		if not d:
			return
		path, column, x, y = d
		interest = self.RecommendationsStore.get_value(self.RecommendationsStore.get_iter(path), 0)
		self.rec_popup_menu.set_user(interest)
		self.rec_popup_menu.popup(None, None, None, event.button, event.time)
		self.RecommendationsTreeView.emit_stop_by_name("button_press_event")
		return True
	
	
	
	def OnPopupSimilarUsers(self, widget, event):
		if event.button != 3:
			return
		
		d = self.SimilarUsersTreeView.get_path_at_pos(int(event.x), int(event.y))
		if not d:
			return
		path, column, x, y = d
		user = self.SimilarUsersStore.get_value(self.SimilarUsersStore.get_iter(path), 1)
		self.users_popup_menu.set_user(user)
		num = 5
		items = self.users_popup_menu.get_children()
		num = 5
		items[num].set_active(self.frame.Networking.config.has_key("buddies") and self.frame.Networking.config["buddies"].has_key(user))
		items[num+1].set_active(self.frame.Networking.config.has_key("banned") and self.frame.Networking.config["banned"].has_key(user))
		items[num+2].set_active(self.frame.Networking.config.has_key("ignored") and self.frame.Networking.config["ignored"].has_key(user))
		items[num+3].set_active(self.frame.Networking.config.has_key("trusted") and self.frame.Networking.config["trusted"].has_key(user))
		self.users_popup_menu.popup(None, None, None, event.button, event.time)
		self.SimilarUsersTreeView.emit_stop_by_name("button_press_event")
		return True
	
	def OnAddLikeRecommendation(self, widget):
		pass
	def OnAddHatedRecommendation(self, widget):
		pass

	def OnItemRecommendations(self, widget):
		pass
	def OnSearchRecommendations(self, widget):
		pass	
	
	def OnAddLikedInterest(self, widget):
		interest = self.frame.PopupEntry(title='Add Liked Interest',
		message='Enter something you like:',
		default_text='')
		self.frame.Networking.AddInterest(interest)

	def OnAddHatedInterest(self, interest):
		interest = self.frame.PopupEntry(title='Add Hated Interest',
		message='Enter something you do not like:',
		default_text='')
		self.frame.Networking.AddHatedInterest(interest)
		
	def OnRemoveLikedInterest(self, widget):
		interest = self.likes_popup_menu.user
		if interest != "" and interest != None and self.frame.Networking.config["interests.like"].has_key(interest):
			
			self.frame.Networking.RemoveInterest(interest)
	
	def OnInterestRecommendations(self, widget):
		interest = self.likes_popup_menu.user
		if interest != "" and interest != None and self.frame.Networking.config["interests.like"].has_key(interest):
			self.frame.Networking.GetItemRecommendations(interest)
			self.frame.Networking.GetItemSimilarUsers(interest)
			
			
	def OnRemoveHatedInterest(self, widget):
		interest = self.hated_popup_menu.user
		if interest != "" and interest != None and self.frame.Networking.config["interests.hate"].has_key(interest):
			self.frame.Networking.RemoveHatedInterest(interest)
	
	def AddedLikedInterest(self, interest):
		if interest in self.liked:
			return
		iter = self.LikesStore.append([interest])
		self.liked[interest] = iter
	def AddedHatedInterest(self, interest):
		if interest in self.hated:
			return
		iter = self.HatesStore.append([interest])
		self.hated[interest] = iter
	def RemovedLikedInterest(self, interest):
		if interest not in self.liked:
			return
		self.LikesStore.remove(self.liked[interest])
		del self.liked[interest]
	def RemovedHatedInterest(self, interest):
		if interest not in self.hated:
			return
		self.HatesStore.remove(self.hated[interest])
		del self.hated[interest]
	def GetGlobalRecommendations(self, widget):
		self.frame.Networking.GetGlobalRecommendations()
	def GetRecommendations(self, widget):
		self.frame.Networking.GetRecommendations()
	def GetSimilarUsers(self, widget):
		self.frame.Networking.GetSimilarUsers()

	def UpdateRecommendations(self, ListOfRecommendations):
		self.RecommendationsStore.clear()
		self.recommendations = {}
		for rec, num in ListOfRecommendations.items():
			#Output(rec, num)
			iter = self.RecommendationsStore.append([self.frame.decode(rec), num])
			self.recommendations[rec] = iter
			
	def UpdateSimilarUsers(self, ListOfSimilarUsers):
		self.SimilarUsersStore.clear()
		self.users = {}
		if DEBUG: Output(ListOfSimilarUsers)
		for username, status in ListOfSimilarUsers.items():
			if self.frame.user_stats.has_key(username):
				#if DEBUG: Output(username)
				status2, speed, downloads, files, dirs = self.frame.user_stats[username]
			else:
				speed = downloads = files = dirs = 0
				self.frame.user_stats[username] = status, speed, downloads, files, dirs
			img = self.frame.GetStatusImage(username, status)
			hspeed = self.frame.HSpeed(speed)
			hfiles = Humanize(files)
			iter = self.SimilarUsersStore.append([img, username, hfiles, hspeed, status])
			self.users[username] = iter
	
class Murmur( Networking):

	def __init__(self):

		gtk.gdk.threads_init()
		signal.signal(signal.SIGINT, signal.SIG_DFL)
		self.DEBUG = DEBUG
		self.SEXY = SEXY


		# default config
		self.Config = {}

		## @var config_manager
		# ConfigManager (Class)
		self.config_manager = ConfigManager(self)
		self.config_manager.check_path()
		utils.PROTOCOL_HANDLERS = self.Config["urls"]["protocols"].copy()
		utils.MURMUR = self
		self.username = None
		self.pid = pid
		self.timer = None
		self.status = None
		

		self.statusbar = None
		self.BuddiesComboEntries = []
		
		self.statusbarmessage = "Offline"
		
		
		self.chatlog = []
		#self.room_lists = {} # the actual rooms
		self.lists_rooms = None # pygtk ListView index
		self.user_stats = {}
		self.user_exists = {}
		self.ip_requested = []
		self.TrayApp = TrayApp(self)

		self.CreateMurmurWindow()
		self.MurmurWindow.set_title(_("Murmur") + " " + version)
		
		self.subprocess_fail=0	
		try:
			import subprocess
			self.subprocess = subprocess
		except ImportError:
			self.subprocess_fail=1
			
		self.Networking = Networking(self)
		#self.murmur_config = murmur_config
		self.Settings = Settings(self)
		self.EntryDialog = EntryDialog(self)
		self.ImageDialog = ImageDialog(self)
		self.Logging = Logging(self)
		self.Muscan = Muscan(self)
		self.Config["murmur"]["trayapp"]
		if self.Config["murmur"]["trayapp"]:
			self.TrayApp.Toggle(None)
		self.quit_w = "No"
		self.Networking.start()

		self.MurmurWindow.show_all()
		utils.USERNAMEHOTSPOTS = 1
		
		
	def disconnect(self, string):
		self.Networking.disconnect("")
		
	def connect_process(self, string):
		
		
		self.MurmurWindow.show_all()
		
		self.Networking.connect_to_museekd("") 

	def SaveColumns(self):
		for i in [self.ChatRooms]:
			#, self.chatrooms.roomsctrl, self.downloads, self.uploads, self.searches
			i.saveColumns()
		self.config_manager.update_config()
		
	
	def AppendToLogWindow(self, message, bug=0):
		AppendLine(self.LogWindow, message, None)
		if self.Config["logging"]["logcollapsed"]:
			self.update_statusbar(message)
			
	def update_statusbar(self, message):
		try:
			self.statusbarmessage = message
			if self.statusbar != None and self.context_id != None:
				self.statusbar.push(self.context_id, self.statusbarmessage)
		except Exception, e:
			if DEBUG: Output("update_statusbar", Exception, e)
	def PrintTraceBack(self):
		tb = traceback.extract_tb(sys.exc_info()[2])
		for line in tb:
			Output(line)
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
	
	def OpenImage(self, widget, title, filters):
		dialog = gtk.FileChooserDialog(title=title, parent=None, action=gtk.FILE_CHOOSER_ACTION_OPEN, buttons=(gtk.STOCK_OK, gtk.RESPONSE_ACCEPT, gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT))
		dialog.set_select_multiple(False)
		dialog.set_current_folder_uri("file://"+gtk.getcwd().getpwuid(os.getuid())[5])
		ff = gtk.FileFilter()
		for ffilter in filters:
			ff.add_pattern(ffilter)
			

		dialog.set_property("filter", ff)
		response = dialog.run()
		
		if response == gtk.RESPONSE_ACCEPT:
			
			res = dialog.get_filenames()
			for files in res:
				file = files
		else:
			file = res = None
		
		dialog.destroy()
		return file
		
	def OnOpenImage(self, widget):
		newimage = self.OpenImage(widget, _("Select an Userinfo Image"), ["*.jpg", "*.jpeg", "*.png","*.bmp", "*.xpm", "*.ico","*.gif"])
		if newimage != None:
			try:
				shutil.copy(newimage, self.Networking.config["userinfo"]["image"] )
				pass

			except Exception, e:
				if self.DEBUG: Output("Creating image", str(e))
		
	
			
	def byte_format(self, filesize):
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
		

			
	def Length2Time(self, length):
		if length != '' and length != None:
			minutes = int(length)/60
			seconds = str( int(length) - (60 * minutes))
			if len(seconds) < 2:
				seconds = '0' + seconds
			length = str(minutes)+":"+str(seconds)
		else:
			length = "0:00"
		return length
		
	

 	def window_quit(self, w=None, event=None):
		self.config_manager.update_config()
 		self.MurmurWindow.hide()
 		self.MurmurWindow.destroy()
		self.MurmurWindow = None
 		#gtk.main_quit()
		
		quit_on_close = True
		if quit_on_close == True:
			
			os._exit(0)
		
	def click(self, button):
		self.MurmurWindow.ret = None
		self.MurmurWindow.quit()

	def quit(self, string):
		os._exit(0)
		
	def userstatus(self, string):
		try:
			if self.status:
				stat = "Away"
			else:
				stat = "Online"
			if self.username != None:
				message="%s is %s" % (self.username, stat)
			else:
				message="You are Offline"
	
			self.AppendToLogWindow(message)
		except Exception,e:
			if DEBUG: Output("userstatus ERROR", e)

	def about(self, string):
		try:
			title=_("About Murmur %s" % version)
			win = ImageDialog(self, title=title,
			message="<b>Murmur</b> is a PyGTK <b>Museek-Plus</b> client."\
"\nCreator: daelstorm"\
"\nVersion: %s"\
"\n\nMuseek-Plus Website: http://www.museek-plus.source-forge.net/"\
"\nMuseek-Plus Trac: http://www.museek-plus.org/"\

"\nWebpage: http://thegraveyard.org/daelstorm/murmur.php"\
"\nTrac: http://www.museek-plus.org/wiki/murmur"\
 % version, picture = self.images["logo"])
			win.set_title(title)
			win.show()
			gtk.main()
			win.destroy()

			return win.ret
		except Exception,e:
			if DEBUG: Output("about ERROR", e)
			
	def ChatRoomCommands(self, widget):
		helpdict = [
		[00, "", _("Usernames with spaces not supported in two-part commands")],
		[01, "/join <i>room</i>", _("Join a Room")], \
		[02, "/close /c /leave /l /part /p", _("Close tab / Leave Room")],\
		[03, "/pm <i>username</i>", _("Open a Private Chat with a user")], 
		[04, "/msg /m <i>username</i> <i>message</i>", _("Send a user a private message")],\
		[05, "/browse /b <i>username</i>", _("Browse a user")],\
		[06, "/w /whois /info <i>username</i>", _("Get Userinfo")],\
		[07, "/ip <i>username</i>", _("Get a user's IP address")],\
		[8, "\n/add /a /buddy <i>username</i>", _("\nAdd a user your buddy list")], \
		[9, "/unbuddy /un /rem <i>username</i>", _("Remove user from your buddy list")], \
		[10, "/trust <i>username</i>", _("Add a user your trusted list")], \
		[11, "/distrust <i>username</i>", _("Remove user from your trusted list")], \
		[12, "/ban <i>username</i>", _("Add a user your banned list")], \
		[13, "/unban <i>username</i>", _("Remove user from your banned list")], \
		[14, "/ignore <i>username</i>", _("Add a user your ignored list")], \
		[15, "/unignore <i>username</i>", _("Remove user from your ignored list")], \
		#"16\n/tick /t <i>message</i>", _("\nSet ticker"), \
		[17, "/nsa <i>username</i>", _("Get user's Info and Shares")], \
		[18, "/ctcpversion <i>username</i>", _("Request user's Client version (supported by Nicotine-Plus / Museek-Plus)")],\
		
		[19, "\n/search /s <i>query</i>", _("\nSearch for Query")], \
		[20, "/bsearch /bs <i>query</i>", _("Search all Buddies for Query")], \
		[21, "/rsearch /rs <i>query</i>", _("Search all Rooms for Query")], \
		[22, "/usearch /us <i>username</i> <i>query</i>", _("Search a user")], \
		
		[23, "\n/clear /cl", _("Clear Log")], \
		[24, "/quit /q", _("Quit Murmur")] ]
		OpenHelpDialog(self, title="Help", message=_("<b>Chat Commands</b>"), modal= True, helpdict=helpdict, sort=True)
		
	def SharesCommands(self, widget):
		helpdict = [
		[00, "/reload", _("Reload Shares Database from disk (after a update or rescan)")],\
		[01, "\n/updatenormal", _("\nUpdate Normal Shares")], \
		[02, "/updatebuddy", _("Update Buddy Shares")], \
		[03, "/rescan", _("Rescan Normal and Buddy Shares")], \
		[04, "/rescannormal", _("Rescan Normal Shares")], \
		[05, "/rescanbuddy", _("Rescan Buddy Shares")], \
		[06, "/addnormaldir <i>/path</i>", _("Add a Directory to Normal Shares")], \
		[07, "/addbuddydir <i>/path</i>", _("Add a Directory to Buddy Shares")], \
		[8, "/removenormaldir <i>/path</i>", _("Remove a Directory from Normal Shares")], \
		[9, "/removebuddydir <i>/path</i>", _("Remove a Directory from Buddy Shares")], \
		[10, "\n/listshares", _("\nList All Shares")], \
		[11, "/listnormal", _("List Normal Shares")], \
		[12, "/listbuddy", _("List Buddy Shares") ] ]
		OpenHelpDialog(self, title="Help", message=_("<b>Shares Commands</b>"), modal= True, helpdict=helpdict, sort=True)
		
	def PopupEntry(self, title="Input Box", message="", default_text='', modal= True, List=[]):
		try:
			win = EntryDialog(self, message, default_text, modal=modal, droplist=List)
			win.set_title(title)
			win.show()
			gtk.main()
			
			return win.ret
		
		except Exception,e:
			if DEBUG: Output("PopupEntry ERROR", e)

	def PopupEntryPrivate(self, string):
		user = self.PopupEntry(title='Open Private Message',
		message='Enter the user you wish to Send Private Messages to:',
		default_text='', List=self.Networking.config["buddies"].keys())
		if user != None and user != "":
			self.PrivateChats.SendMessage(user)
		
	def PopupEntryJoinRoom(self, string, buf):
		room = self.PopupEntry(title='Join a room',
		message='Enter the room you wish to Join:',
		default_text='', List=self.ChatRooms.rooms.keys())
		if room != None and room != "":
			self.JoinARoom(room)

	def PopupEntryLeaveRoom(self, string, buf):
		room = self.PopupEntry(title='Leave a room',
		message='Enter the room you wish to Leave:',
		default_text='', List=self.ChatRooms.rooms.keys())
		if room != None and room != "":
			self.RoomLeave(room)
			
	def settings(self, modal=True):
		win = self.Settings
		win.set_title("Murmur Settings")

		win.read_config()
		win.show()


	
	def SettingsWindow(self, string):
	
		result = self.settings()
		if result is None:
			pass
		else:
			if DEBUG:
				Output("settings: "+str(result))

		
	def LookupUserShares(self, string):
		try:
			user = self.PopupEntry(title='Browse User',
			message='Enter the username of the user who\'s Shares you wish to Browse:',
			default_text='', List=self.Networking.config["buddies"].keys())
			if user is not None:
				self.GetShares(user)
		except Exception,e:
			if DEBUG: Output("LookupUserShares ERROR", e)
	
	def LookupUserInfo(self, string):
		try:
			user = self.PopupEntry(title='Lookup Userinfo',
			message='Enter the username of the user who\'s UserInfo you wish to see:',
			default_text='', List=self.Networking.config["buddies"].keys())
			if user is not None:
				self.GetUserInfo(user)

		except Exception,e:
			if DEBUG: Output("LookupUserInfo ERROR", e)
			
	def LookupIP(self, string):
		try:
			user = self.PopupEntry(title='Lookup IP',
			message='Enter the username of the user who\'s IP you wish to know:',
			default_text='', List=self.Networking.config["buddies"].keys())
			if user is not None:
				self.GetIPAddress(user)

		except Exception,e:
			if DEBUG: Output("LookupIP ERROR", e)
				
	def away_toggle(self, string):
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
			
			

	def ToggleLogWindow(self, widget, buf):
		active = widget.get_active()

		if active:
			if self.LogScrolledWindow in self.vpaned1.get_children():
				self.vpaned1.remove(self.LogScrolledWindow)
				self.Config["logging"]["logcollapsed"] = True

		else:
			if not self.LogScrolledWindow in self.vpaned1.get_children():
				self.vpaned1.pack2(self.LogScrolledWindow, False, True)
				ScrollBottom(self.LogScrolledWindow)
				self.Config["logging"]["logcollapsed"] = False
			
	def CreateIconButton(self, icon, icontype, callback, label=None):
		button = gtk.Button()
		button.connect_object("clicked", callback, "")
		button.show()
		
		Alignment = gtk.Alignment(0.0, 0.5, 0, 0)
		Alignment.show()
	
		Hbox = gtk.HBox(False, 2)
		Hbox.show()
		Hbox.set_spacing(2)
	
		image = gtk.Image()
		image.set_padding(0, 0)
		if icontype == "stock":
			image.set_from_stock(icon, 4)
		else:
			image = gtk.Image()
			image.set_from_pixbuf(icon)
		image.show()
		Hbox.pack_start(image, False, False, 0)
		Alignment.add(Hbox)
		if label:
			Label = gtk.Label(label)
			Label.set_padding(0, 0)
			Label.show()
			Hbox.pack_start(Label, False, False, 0)
		button.add(Alignment)
		return button
		
		
	def CreateMurmurWindow(self):

		self.accel_group = gtk.AccelGroup()

		self.MurmurWindow = gtk.Window(gtk.WINDOW_TOPLEVEL)
		self.MurmurWindow.set_default_size(700, 500)
		self.MurmurWindow.set_position(gtk.WIN_POS_CENTER)
		self.MurmurWindow.add_accel_group(self.accel_group)
		self.MurmurWindow.show()
		
		self.MurmurWindow.connect("destroy", self.window_quit)
		self.MurmurWindow.connect("delete_event", self.window_quit)
		
		self.clip_data = ""
		self.current_tab = 0
		self.MurmurWindow.connect("selection_get", self.OnSelectionGet)
		self.MurmurWindow.connect("focus_in_event", self.OnFocusIn)
		self.MurmurWindow.connect("focus_out_event", self.OnFocusOut)
		#self.f = None
		
		self.imagedirectory = "images"
		self.images = {}
		
		for i in "away",  "online",  "offline", "noexist", "logo", "close", "green", "yellow", "red", "icon", "away_trusted", "away_banned", "away_buddied", "away_ignored", "online_trusted", "online_banned", "online_ignored", "online_buddied", "offline_trusted", "offline_banned", "offline_ignored", "offline_buddied", "hilite", "empty":
			loader = gtk.gdk.PixbufLoader("png")
			data = getattr(imagedata, i)
			loader.write(data, len(data))
			loader.close()
			self.images[i] = loader.get_pixbuf()
			del loader, data
		gc.collect()
		self.MurmurWindow.set_icon(self.images["icon"])

		#self.transfers= {"uploads": {}, "downloads": {} }

		self.roomtab = None
		
		#self.close_image = os.path.join(self.imagedirectory, "x.png")
		#self.close_image = self.images["close"]
		vbox_murmurwindow = gtk.VBox()
		vbox_murmurwindow.set_spacing(2)
	
		########################
		# Menu bar
		########################
		self.MainMenu = self.CreateMainMenu()
		########################
		vbox_murmurwindow.pack_start(self.MainMenu, False, False, 0)
		self.vpaned1 = gtk.VPaned()
		self.vpaned1.set_border_width(4)
		self.notebook_outside = gtk.Notebook()
		self.notebook_outside.set_tab_pos(gtk.POS_TOP)
		self.notebook_outside.set_scrollable(False)
		
		self.LogWindow = gtk.TextView()
		self.LogWindow.set_wrap_mode(gtk.WRAP_WORD)
		self.LogWindow.set_cursor_visible(False)
		self.LogWindow.set_editable(False)
		self.LogWindow.show()
		
		self.LogScrolledWindow = gtk.ScrolledWindow()
		self.LogScrolledWindow.set_border_width(1)
		self.LogScrolledWindow.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.LogScrolledWindow.set_shadow_type(gtk.SHADOW_IN)
		self.LogScrolledWindow.add_with_viewport(self.LogWindow)
		self.LogScrolledWindow.show()
		# Vertically arrange tabs and log window
		self.vpaned1.pack1(self.notebook_outside, True, True)

		if not self.Config["logging"]["logcollapsed"]:
			self.vpaned1.pack2(self.LogScrolledWindow)
		else:
			self.ItemLogWindow.set_active(1)
        	self.vpaned1.show()
		vbox_murmurwindow.pack_start(self.vpaned1, True, True, 0)				
		self.statusbar = gtk.Statusbar()
		self.context_id = self.statusbar.get_context_id("")
		self.statusbar.push(self.context_id, self.statusbarmessage)
		self.statusbar.show()
		vbox_murmurwindow.pack_end(self.statusbar, False, False, 0)
		
 		#self.MurmurWindow.action_area.hide()

		vbox_murmurwindow.show()
		#self.MurmurWindow.set_default(vbox_murmurwindow)
		#if create:
            	self.MurmurWindow.add(vbox_murmurwindow)
		
				
		self.notebook_roomlist = gtk.Frame()
		self.notebook_roomlist.show()	

		# Chat Room Construction
		
		
		self.ChatRooms = ChatRooms(self)
		self.ChatRoomsVbox = gtk.VBox()
		self.ChatRoomsVbox.set_border_width(5)
		self.ChatRoomsVbox.pack_start(self.ChatRooms)
		self.ChatRoomsVbox.show()
		
		# Private Chat Construction
		self.privatevbox = gtk.VBox(False, 0)
		self.privatevbox.show()
		self.privatevbox.set_spacing(0)
	
		self.PrivateChats = PrivateChats(self)
		
	
		self.hbox20 = gtk.HBox(False, 5)
		self.hbox20.show()
		self.hbox20.set_spacing(5)
		self.hbox20.set_border_width(5)
		
		# Entry
		self.PrivateChatEntry = BuddiesComboBoxEntry(self)
		self.BuddiesComboEntries.append(self.PrivateChatEntry)
		self.hbox20.pack_start(self.PrivateChatEntry, False, True, 0)
		self.PrivateChatEntry.child.connect("activate", self.OnGetPrivateChat)
		
		# Button
		self.sPrivateChatButton = self.CreateIconButton(gtk.STOCK_JUMP_TO, "stock", self.OnGetPrivateChat, "Start Message")
		
		
		
		self.hbox20.pack_start(self.sPrivateChatButton, False, False, 0)
		# Private Alignment
		self.privatevbox.pack_start(self.hbox20, False, True, 0)
		self.privatevbox.pack_start(self.PrivateChats, True, True, 0)
		# Transfers Construction
		self.notebook_transfers = gtk.Frame()
		self.notebook_transfers.show()

		vpaned1 = gtk.VPaned()
		self.downloads = Downloads(self)
		vpaned1.pack1(self.downloads.Main, False, True)
		self.uploads = Uploads(self)
		vpaned1.pack2(self.uploads.Main, False, True)
		vpaned1.show()
		#vpaned1.pack2(self.Main, False, True)

		self.notebook_transfers.add(vpaned1)

		
		
		# Searches Construction
		self.Searches = Searches(self)
		self.SearchVbox = gtk.VBox(False, 0)
		self.SearchVbox.show()
		self.SearchVbox.set_spacing(0)
	
		hbox1 = gtk.HBox(False, 5)
		hbox1.show()
		hbox1.set_spacing(5)
		hbox1.set_border_width(4)
	
		self.SearchEntry = gtk.Entry()
		self.SearchEntry.set_text("")
		self.SearchEntry.set_editable(True)
		self.SearchEntry.connect("activate", self.DoSearch)
		self.SearchEntry.show()
		hbox1.pack_start(self.SearchEntry, True, True, 0)
	
		self.GlobalSearch = gtk.RadioButton()
		self.GlobalSearch.set_active(True)
		self.GlobalSearch.set_label(("Global"))
		self.GlobalSearch.show()
	
		hbox1.pack_start(self.GlobalSearch, False, False, 0)
	
		self.BuddySearch = gtk.RadioButton(self.GlobalSearch)
		self.BuddySearch.set_active(False)
		self.BuddySearch.set_label(("Buddies"))
		self.BuddySearch.show()
	
		hbox1.pack_start(self.BuddySearch, False, False, 0)
	
		self.RoomSearch = gtk.RadioButton(self.GlobalSearch)
		self.RoomSearch.set_active(False)
		self.RoomSearch.set_label(("Rooms"))
		self.RoomSearch.show()
	
		hbox1.pack_start(self.RoomSearch, False, False, 0)
	
		self.SearchButton = self.CreateIconButton(gtk.STOCK_FIND, "stock", self.DoSearch, "Search")
		
	
		hbox1.pack_start(self.SearchButton, False, False, 0)
	
		self.SearchVbox.pack_start(hbox1, False, False, 0)
		self.SearchVbox.pack_start(self.Searches)
		
		# User lists Construction
		self.userlists = ListsOfUsers(self)
		
		# Userinfo Construction
		
		self.userinfovbox = gtk.VBox(False, 0)
		self.userinfovbox.show()
		self.userinfovbox.set_spacing(0)

		self.userinfos = UserInfos(self)
	
		self.hbox21 = gtk.HBox(False, 5)
		self.hbox21.show()
		self.hbox21.set_spacing(5)
		self.hbox21.set_border_width(5)
		
		self.UserinfoEntry = gtk.Entry()
		self.UserinfoEntry.set_text("")
		self.UserinfoEntry.set_editable(True)
		self.UserinfoEntry.show()
		self.UserinfoEntry.set_visibility(True)
		self.hbox21.pack_start(self.UserinfoEntry, False, True, 0)
		
		self.sUserinfoButton = self.CreateIconButton(gtk.STOCK_JUMP_TO, "stock", self.OnGetUserInfo, _("Get UserInfo"))
		
	
		self.hbox21.pack_start(self.sUserinfoButton, False, False, 0)
	
		self.userinfovbox.pack_start(self.hbox21, False, True, 0)
		
		self.UserinfoEntry.connect("activate", self.OnGetUserInfo)
		self.userinfovbox.pack_start(self.userinfos, True, True, 0)
		# User Browse Construction
		self.userbrowsevbox = gtk.VBox(False, 0)
		self.userbrowsevbox.show()
		self.userbrowsevbox.set_spacing(0)
		
		self.userbrowses = UserBrowses(self)
		
		
	
		self.hbox22 = gtk.HBox(False, 5)
		self.hbox22.show()
		self.hbox22.set_spacing(5)
		self.hbox22.set_border_width(5)
	
		
	
		self.SharesEntry = gtk.Entry()
		self.SharesEntry.set_text("")
		self.SharesEntry.set_editable(True)
		self.SharesEntry.show()
		self.SharesEntry.set_visibility(True)
		self.hbox22.pack_start(self.SharesEntry, False, True, 0)
		
		self.sSharesButton = self.CreateIconButton(gtk.STOCK_JUMP_TO, "stock", self.OnGetShares, _("Browse Shares"))
	
		self.hbox22.pack_start(self.sSharesButton, False, False, 0)
		self.SharesEntry.connect("activate", self.OnGetShares)

		self.userbrowsevbox.pack_start(self.hbox22, False, True, 0)
		self.userbrowsevbox.pack_start(self.userbrowses, True, True, 0)
		# Room List Construction
		## Recommendations and Interests
		
		self.Recommendations = Recommendations(self)
		
		#
		#
		# Add tabs to Notebook
		self.ChatRoomLabel = ImageLabel(_("Chat Rooms"), self.images["empty"])
		self.notebook_outside.append_page(self.ChatRoomsVbox, self.ChatRoomLabel)
		
		self.PrivateChatLabel = ImageLabel(_("Private Chat"), self.images["empty"])
		self.notebook_outside.append_page(self.privatevbox, self.PrivateChatLabel)
		
		self.TransfersLabel = ImageLabel(_("Transfers"), self.images["empty"])
		self.notebook_outside.append_page(self.notebook_transfers, self.TransfersLabel)
		
		self.SearchLabel = ImageLabel(_("Search"), self.images["empty"])
		self.notebook_outside.append_page(self.SearchVbox, self.SearchLabel)
		
		self.UserInfoLabel = ImageLabel(_("User Info"), self.images["empty"])
		self.notebook_outside.append_page(self.userinfovbox, self.UserInfoLabel)
		
		self.UserBrowseLabel = ImageLabel(_("User Browse"), self.images["empty"])
		self.notebook_outside.append_page(self.userbrowsevbox, self.UserBrowseLabel )
		
		self.UserListsLabel = ImageLabel(_("Users & Interests"), self.images["empty"])
		self.notebook_outside.append_page(self.userlists, self.UserListsLabel)
		
		self.RoomListLabel = ImageLabel(_("Room List"), self.images["empty"])
		self.notebook_outside.append_page(self.notebook_roomlist, self.RoomListLabel)
		
		#self.InterestsLabel = ImageLabel(_("Interests"), self.images["empty"])
		#self.notebook_outside.append_page(self.Recommendations.Main, self.InterestsLabel)
		self.userlists.append_page(self.Recommendations.Main, _("Interests"))
		self.notebook_outside.show()
		self.notebook_outside.connect("switch_page", self.OnSwitchPage)

	def OnSwitchPage(self, notebook, page, page_nr):
		l = [self.ChatRoomLabel, self.PrivateChatLabel, self.TransfersLabel, self.SearchLabel, self.UserInfoLabel, self.UserBrowseLabel, self.UserListsLabel, self.RoomListLabel, ][page_nr]
		n = [self.ChatRooms, self.PrivateChats, None, self.Searches, self.userinfos, self.userbrowses, self.userlists, None][page_nr]
		self.current_tab = l
		if l is not None:
			l.set_image(self.images["empty"])
		if n is not None:
			#Output(n)
			n.popup_disable()
			n.popup_enable()
			if n.get_current_page() != -1:
				n.dismiss_icon(n, None, n.get_current_page())

		if page_nr == 0 and self.ChatRooms:
			p = n.get_current_page()
			self.ChatRooms.OnSwitchPage(n, None, p, 1)
		elif page_nr == 1:
			p = n.get_current_page()
			self.PrivateChats.OnSwitchPage(n, None, p, 1)
			
	def RequestIcon(self, tablabel):
		if tablabel == self.PrivateChatLabel and not self.got_focus:
			self.MurmurWindow.set_icon(self.images["online"])
		if self.current_tab != tablabel:
			tablabel.set_image(self.images["online"])
			
	def OnFocusIn(self, widget, event):
		self.MurmurWindow.set_icon(self.images["icon"])
		self.got_focus = True
	
	def OnFocusOut(self, widget, event):
		self.got_focus = False	
		
	def OnSelectionGet(self, widget, data, info, timestamp):
		data.set_text(self.clip_data, -1)
	
	def GetCompletion(self, part, list):
		matches = []
		for match in list:
			if match in matches:
				continue
			if match[:len(part)] == part and len(match) > len(part):
				matches.append(match)
		
		if len(matches) == 0:
			return "", 0
		elif len(matches) == 1:
			return matches[0][len(part):], 1
		else:
			prefix = matches[0]
			for item in matches:
				for i in range(len(prefix)):
					if prefix[:i+1] != item[:i+1]:
						prefix = prefix[:i]
						break
			return prefix[len(part):], 0
				
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
		
	def encode(self, str, networkenc = None):
		if networkenc is None:
			networkenc = self.config.sections["server"]["enc"]
		if type(str) is types.UnicodeType:
			return str.encode(networkenc,'replace')
		else:
			return str.decode("utf-8",'replace').encode(networkenc,'replace')

	def decode(self, string, networkenc = None):
		if networkenc is None:
			if self.Networking.config.has_key("encoding") and self.Networking.config["encoding"].has_key("default"):
				networkenc = self.Networking.config["encoding"]["default"]
			#else:
		networkenc = "iso8859-15"
		return str(string).decode(networkenc,'replace').encode("utf-8", "replace")

	def getencodings(self):
		return [["Latin", 'ascii'], ["US-Canada", 'cp037'],  ['Hebrew', 'cp424'], ['US English', 'cp437'], ['International', 'cp500'], ['Greek', 'cp737'], ['Estonian', 'cp775'], ['Western European', 'cp850'], ['Central European', 'cp852'], ['Cyrillic', 'cp855'], ['Cyrillic', 'cp856'], ['Turkish', 'cp857'], ['Portuguese', 'cp860'], ['Icelandic', 'cp861'], ['Hebrew', 'cp862'], ['French Canadian', 'cp863'], ['Arabic', 'cp864'], ['Nordic', 'cp865'], ['Cyrillic', 'cp866'], ['Latin-9', 'cp869'], ['Thai', 'cp874'], ['Greek', 'cp875'], ['Japanese', 'cp932'], ['Chinese Simple', 'cp936'], ['Korean', 'cp949'], ['Chinese Traditional', 'cp950'], ['Urdu',  'cp1006'], ['Turkish',  'cp1026'], ['Latin', 'cp1140'], ['Central European', 'cp1250'], ['Cyrillic', 'cp1251'], ['Latin', 'cp1252'], ['Greek', 'cp1253'], ['Turkish', 'cp1254'], ['Hebrew', 'cp1255'], ['Arabic', 'cp1256'], ['Baltic', 'cp1257'], ['Vietnamese', 'cp1258'],  ['Latin', 'iso8859-1'], ['Latin 2', 'iso8859-2'], ['South European', 'iso8859-3'], ['North European', 'iso8859-4'], ['Cyrillic', 'iso8859-5'], ['Arabic', 'iso8859-6'], ['Greek', 'iso8859-7'], ['Hebrew', 'iso8859-8'], ['Turkish', 'iso8859-9'], ['Nordic', 'iso8859-10'], ['Thai', 'iso8859-11'], ['Baltic', 'iso8859-13'], ['Celtic', 'iso8859-14'], ['Western European', 'iso8859-15'], ['South-Eastern European', 'iso8859-16'], ['Cyrillic', 'koi8-r'], ['Latin', 'latin-1'], ['Cyrillic', 'mac-cyrillic'], ['Greek', 'mac-greek'], ['Icelandic', 'mac-iceland'], ['Latin 2', 'mac-latin2'], ['Latin', 'mac-roman'], ['Turkish', 'mac-turkish'], ['International', 'utf-16'], ['International', 'utf-7'], ['International', 'utf-8']]
	
	def DoSearch(self, widget):
		
		query = self.SearchEntry.get_text()
		if query == "":
			return
		elif query.isspace():
			return
		elif len(query) < 3:
			return
		if self.GlobalSearch.get_active():
			self.Networking.Search(0, query)
		elif self.BuddySearch.get_active():
			self.Networking.Search(1, query)
		elif self.RoomSearch.get_active():
			self.Networking.Search(2, query)
		self.SearchEntry.set_text("")
		
	def OnSearch(self, widget):
		self.notebook_outside.set_current_page(3)
		
	def OnGetShares(self, widget):
		self.GetShares(self.SharesEntry.get_text())
		self.SharesEntry.set_text("")
		
	def OnGetUserInfo(self, widget):
		self.GetUserInfo(self.UserinfoEntry.get_text())
		self.UserinfoEntry.set_text("")
		
	def OnGetPrivateChat(self, widget):
		self.GetPrivateChat(self.PrivateChatEntry.child.get_text())
		self.PrivateChatEntry.child.set_text("")
		
	def GetMyUserInfo(self, widget):
		if self.username is not None:
			self.Networking.UserInfo(self.username)
			if self.username not in self.user_stats:
				self.Networking.PeerExists(self.username)
				
	def GetUserInfo(self, user):
		if user != "" and not user.isspace():
			self.Networking.UserInfo(user)
			if user not in self.user_stats:
				self.Networking.PeerExists(user)
				
	def OnUserInfo(self, widget):
		self.notebook_outside.set_current_page(4)
		
	def GetIPAddress(self, user):
		if user not in self.ip_requested and user != None and user != "" and not user.isspace():
			self.ip_requested.append(user)
			self.Networking.PeerAddress(user)
			
	def GetPrivateChat(self, user):
		if user != "" and user != None and not user.isspace():
			self.PrivateChats.SendMessage(user)
			if user not in self.user_stats:
				self.Networking.PeerExists(user)
				
	def GetMyShares(self, widget):
		if self.username is not None:
			self.Networking.UserShares(self.username)
			if self.username not in self.user_stats:
				self.Networking.PeerExists(self.username)
				
	def GetShares(self, user):
		if user != "" and user != None and not user.isspace():
			self.Networking.UserShares(user)
			if user not in self.user_stats:
				self.Networking.PeerExists(user)
				
	def OnUserShares(self, widget):
		self.notebook_outside.set_current_page(5)
		
	def JoinARoom(self, room):

		if room != None and room != "":
			self.Networking.JoinRoom(room)
			
	def RoomLeave(self, room):

		if room != None and room != "":
			if room in self.ChatRooms.joinedrooms:
				self.Networking.LeaveRoom(room)
				
	def CreateMainMenu(self):
		
		MainMenu = gtk.MenuBar()
		
		
		# File Menu
		self.MenuFile = gtk.Menu()
		ItemFile = gtk.MenuItem(_("_File"))
		ItemFile.show()
		ItemFile.set_submenu(self.MenuFile)
		
		

		
		ItemConnect = gtk.ImageMenuItem(_("_Connect"))
		ItemConnect.connect_object("activate", self.connect_process, None)
		ItemConnect.add_accelerator("activate", self.accel_group, gtk.gdk.keyval_from_name("C"), gtk.gdk.MOD1_MASK, gtk.ACCEL_VISIBLE)
        	ItemConnect.set_image(gtk.image_new_from_stock(gtk.STOCK_CONNECT, gtk.ICON_SIZE_MENU))
		ItemConnect.show()
		
		ItemDisconnect = gtk.ImageMenuItem(_("_Disconnect"))
		ItemDisconnect.connect_object("activate", self.disconnect, "disconnect")
		ItemDisconnect.add_accelerator("activate", self.accel_group, gtk.gdk.keyval_from_name("D"), gtk.gdk.MOD1_MASK, gtk.ACCEL_VISIBLE)
		ItemDisconnect.set_image(gtk.image_new_from_stock(gtk.STOCK_DISCONNECT, gtk.ICON_SIZE_MENU))
		ItemDisconnect.show()
		
		

		
		
		# Continuation of File Menu
		
		
		self.ItemAway = gtk.ImageMenuItem(_("Toggle Away"))
		self.ItemAway.add_accelerator("activate", self.accel_group, gtk.gdk.keyval_from_name("A"), gtk.gdk.MOD1_MASK , gtk.ACCEL_VISIBLE)
		self.ItemAway.connect("activate", self.away_toggle)
		img = gtk.Image()
		img.set_from_pixbuf(self.images["away"])
		self.ItemAway.set_image(img)
		self.ItemAway.show()
		
		ItemQuit = gtk.ImageMenuItem(_("_Quit Murmur"))
		ItemQuit.connect("activate", self.window_quit, "")
		ItemQuit.add_accelerator("activate", self.accel_group, gtk.gdk.keyval_from_name("Q"), gtk.gdk.MOD1_MASK, gtk.ACCEL_VISIBLE)
		ItemQuit.set_image(gtk.image_new_from_stock(gtk.STOCK_QUIT, gtk.ICON_SIZE_MENU))
		ItemQuit.show()
		
		self.MenuFile.append(ItemConnect)
		self.MenuFile.append(ItemDisconnect)
		
		self.seperator1 = gtk.MenuItem()
        	self.seperator1.show()
        	self.MenuFile.append(self.seperator1)
		
		self.MenuFile.append(self.ItemAway)
		
		self.seperator2 = gtk.MenuItem()
        	self.seperator2.show()
        	self.MenuFile.append(self.seperator2)
		
		self.MenuFile.append(ItemQuit)
		
		
		# Settings Menu
		MenuSettings = gtk.Menu()
		ItemSettings = gtk.MenuItem(_("S_ettings"))
		ItemSettings.show()
		ItemSettings.set_submenu(MenuSettings)
		ItemSettings.show()
				
		ItemMuseekSettings = gtk.ImageMenuItem(_("Museek _Settings"))
		ItemMuseekSettings.connect_object("activate", self.SettingsWindow, "settings")
		ItemMuseekSettings.add_accelerator("activate", self.accel_group, gtk.gdk.keyval_from_name("S"), gtk.gdk.MOD1_MASK, gtk.ACCEL_VISIBLE)
		ItemMuseekSettings.set_image(gtk.image_new_from_stock(gtk.STOCK_PREFERENCES, gtk.ICON_SIZE_MENU))
		ItemMuseekSettings.show()

		MenuSettings.append(ItemMuseekSettings)
		
		self.ItemLogWindow = ItemLogWindow = gtk.CheckMenuItem(_("Hide log _window"))
		ItemLogWindow.set_active(False)
		ItemLogWindow.connect("activate", self.ToggleLogWindow, "")
		ItemLogWindow.add_accelerator("activate", self.accel_group, gtk.gdk.keyval_from_name("W"), gtk.gdk.MOD1_MASK, gtk.ACCEL_VISIBLE)
		ItemLogWindow.show()
		MenuSettings.append(ItemLogWindow)
		
		self.ItemTray = ItemTray = gtk.CheckMenuItem("Hide _Trayapp")
		
		if self.Config["murmur"]["trayapp"]:
			ItemTray.set_active(True)
			
		else:
			ItemTray.set_active(False)
		ItemTray.connect_object("activate", self.TrayApp.Toggle, ItemTray)
		ItemTray.add_accelerator("activate", self.accel_group, gtk.gdk.keyval_from_name("T"), gtk.gdk.MOD1_MASK, gtk.ACCEL_VISIBLE)
		ItemTray.show()
		
		MenuSettings.append(ItemTray)
		
		# Room Menu
		MenuRooms = gtk.Menu()
		ItemRooms = gtk.MenuItem(_("_Rooms"))
		ItemRooms.set_submenu(MenuRooms)
		ItemRooms.show()
		
		ItemJoinRoom = gtk.ImageMenuItem(_("_Join Room"))
		ItemJoinRoom.connect("activate", self.PopupEntryJoinRoom, "")
		ItemJoinRoom.add_accelerator("activate", self.accel_group, gtk.gdk.keyval_from_name("J"), gtk.gdk.MOD1_MASK, gtk.ACCEL_VISIBLE)
		ItemJoinRoom.set_image(gtk.image_new_from_stock(gtk.STOCK_JUMP_TO, gtk.ICON_SIZE_MENU))
		ItemJoinRoom.show()
		
		ItemLeaveRoom = gtk.ImageMenuItem(_("_Leave Room"))
		ItemLeaveRoom.connect("activate", self.PopupEntryLeaveRoom, "")
		ItemLeaveRoom.add_accelerator("activate", self.accel_group, gtk.gdk.keyval_from_name("L"), gtk.gdk.MOD1_MASK, gtk.ACCEL_VISIBLE)
		ItemLeaveRoom.set_image(gtk.image_new_from_stock(gtk.STOCK_CLOSE, gtk.ICON_SIZE_MENU))
		ItemLeaveRoom.show()
		
		MenuRooms.append(ItemJoinRoom)
		MenuRooms.append(ItemLeaveRoom)
		
		# Users Menu
		ItemUsers = gtk.MenuItem(("_Users"))
		ItemUsers.show()
		
		MenuUsers = gtk.Menu()
		ItemUsers.set_submenu(MenuUsers)
		
		ItemUserInfoU = self.CreateImageMenuItem(_("Get my User Info"),  self.GetMyUserInfo, gtk.STOCK_INFO)
		MenuUsers.append(ItemUserInfoU)
		
		ItemBrowseU = self.CreateImageMenuItem(_("Browse my Shares"),  self.GetMyShares, gtk.STOCK_HARDDISK)
		MenuUsers.append(ItemBrowseU)
		
		seperator3 = gtk.MenuItem()
		seperator3.show()
		MenuUsers.append(seperator3)
		
		ItemPrivate = self.CreateImageMenuItem(_("Send _Private Message to a User"),  self.PopupEntryPrivate, gtk.STOCK_EDIT, "P")
		MenuUsers.append(ItemPrivate)
		
		ItemIP = self.CreateImageMenuItem(_("Lookup a User's IP Address"),  self.LookupIP, gtk.STOCK_NETWORK, "I")
		MenuUsers.append(ItemIP)
		
		ItemUserInfo = self.CreateImageMenuItem(_("Lookup a User's Info"),  self.LookupUserInfo, gtk.STOCK_INFO, "E")
		MenuUsers.append(ItemUserInfo)
		
		ItemBrowse = self.CreateImageMenuItem(_("Browse a User's Shares"),  self.LookupUserShares, gtk.STOCK_HARDDISK, "B")
		MenuUsers.append(ItemBrowse)

		# Help Menu
		ItemHelp = gtk.MenuItem(_("_Help"))
		ItemHelp.show()
	
		MenuHelp = gtk.Menu()
		MenuHelp.show()
		ItemHelp.set_submenu(MenuHelp)
		
		ChatCommandsItem = self.CreateImageMenuItem(_("_Chat Commands"),  self.ChatRoomCommands, gtk.STOCK_HELP, "F1")
	
		MenuHelp.append(ChatCommandsItem)
		

		SharesCommandsItem = self.CreateImageMenuItem(_("_Shares Commands"),  self.SharesCommands, gtk.STOCK_HELP)
		MenuHelp.append(SharesCommandsItem)
		
		seperator4 = gtk.MenuItem()
		seperator4.show()
		MenuHelp.append(seperator4)
		
		ItemAbout = self.CreateImageMenuItem(_("_About"),  self.about, gtk.STOCK_HELP, "F1")
		MenuHelp.append(ItemAbout)
		
		

		
		
		# Arrange Main Menu
		MainMenu.append(ItemFile)
		MainMenu.append(ItemSettings)
		MainMenu.append(ItemRooms)
		MainMenu.append(ItemUsers)
		MainMenu.append(ItemHelp)
		MainMenu.show()
		return MainMenu

	def CreateImageMenuItem(self, title, callback, stockicon, hotkey=None):
		Item = gtk.ImageMenuItem(title)
		Item.connect("activate", callback)
		Item.set_image(gtk.image_new_from_stock(stockicon, gtk.ICON_SIZE_MENU))
		if hotkey:
			Item.add_accelerator("activate", self.accel_group, gtk.gdk.keyval_from_name(hotkey), gtk.gdk.MOD1_MASK, gtk.ACCEL_VISIBLE)
		Item.show()
		return Item
Murmur()

mainloop = gobject.MainLoop()
mainloop.run()


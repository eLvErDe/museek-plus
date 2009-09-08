# This is part of the Murmur Museek Client, and distributed under the GPLv2
# Copyright (c) 2007 daelstorm.
import commands, os, sys, getopt #, subprocess
import locale, gettext

import signal
import pygtk, gtk
import gobject
import time, stat, string, shutil

from time import sleep
from utils import _
REQUIRE_PYMUSEEK = '0.3.0'

def versionCheck(version):
	build = 255
	def _required():
		global REQUIRE_PYMUSEEK
		r = REQUIRE_PYMUSEEK.split(".")
		r_major, r_minor, r_micro = [int(i) for i in r[:3]]
		return (r_major << 24) + (r_minor << 16) + (r_micro << 8) + build
	
	s = version.split(".")
	major, minor, micro = [int(i) for i in s[:3]]

	if (major << 24) + (minor << 16) + (micro << 8) + build >= _required():
		return True
	return False
try:
	import messages, driver
except:
	try:
		import museek
		from museek import messages, driver
		if not hasattr(museek, 'VERSION'):
			raise Exception, "Cannot use unknown museek python bindings version."
		elif hasattr(museek, 'VERSION') and not versionCheck(museek.VERSION): 
			raise Exception, "Cannot use museek python bindings version: %s" % (museek.VERSION,)
	except Exception, error:
		print "WARNING: The Museek Message-Parsing modules, messages.py and/or driver.py were not found, or are an old version. Please install them into your '/usr/lib/python2.X/site-packages/museek' directory, or place them in a 'museek' subdirectory of the directory that contains the mucous python script."
		print error
		
		sys.exit()
		
class EntryDialog( gtk.Dialog):
    def __init__(self, Mapp, message="", message2=None, key='', value=None, modal= True, List=[], v_list=[], second=True):
        gtk.Dialog.__init__(self)
        self.connect("destroy", self.quit)
        self.connect("delete_event", self.quit)
	self.ret = key
	self.ret2 = value
        if modal:
            self.set_modal(True)
        box = gtk.VBox(spacing=10)
        box.set_border_width(10)
        self.vbox.pack_start(box)
        box.show()
        if message:
            label = gtk.Label(message)
            box.pack_start(label)
            label.show()
	    
	self.combo = gtk.combo_box_entry_new_text()

	alist = List
	alist.sort()
	for i in alist:
		self.combo.append_text( i)
	if key != None:
		self.combo.child.set_text(key)
	self.combo.grab_focus()
	
	self.combo.show()
	
	box.pack_start(self.combo)
	if message2 != None and message2 != "":
		label2 = gtk.Label(message2)
		box.pack_start(label2)
		if second == True:
	  		label2.show()
	self.combov = gtk.combo_box_entry_new_text()
	
	vlist = v_list
	vlist.sort()
	for i in vlist:
		self.combo.append_text( i)
	if value is not None:
		self.combov.child.set_text(value)
	self.combov.grab_focus()
	if message2 == _("Encoding"):
		for encoding in Mapp.encodings:
			self.combov.append_text(encoding)
	if second == True and message2 != None and message2 != "":
		self.combov.show()
	
	box.pack_start(self.combov)
	
        button = gtk.Button(_("OK"))
        button.connect("clicked", self.click)
        button.set_flags(gtk.CAN_DEFAULT)
        self.action_area.pack_start(button)
        button.show()
	
        button.grab_default()
        button = gtk.Button(_("Cancel"))
        button.connect("clicked", self.close)
        button.set_flags(gtk.CAN_DEFAULT)
	
        self.action_area.pack_start(button)
        button.show()
        
    def close(self, w=None, event=None):
	self.ret = self.ret2 = None
        self.quit()
	
    def quit(self, w=None, event=None):

        self.hide()
        self.destroy()

	
	
    def click(self, button):
        self.ret = self.combo.child.get_text()
	self.ret2 = self.combov.child.get_text()
	
        self.quit()
	
class Settings( gtk.Dialog):
	def __init__(self, xapp, message="", modal= True):
		gtk.Dialog.__init__(self)
		self.app=xapp
		self.murmur_config = self.app.Config
		self.set_transient_for(self.app.MurmurWindow)
		self.set_default_size(600, 550)
		self.connect("destroy", self.quit)
		self.connect("delete_event", self.quit)
		self.parents= {}
		self.SharedDirs = {"normal":[], "buddy":[] }
		if modal:
			self.set_modal(True)
		
		self.vbox.set_spacing(5)
		
		
		# DIALOGS
		self.dialogs()
		self.vbox.pack_start(self.MainNotebook)
		
		OkButton = self.app.CreateIconButton(gtk.STOCK_OK, "stock", self.click, _("OK"))
		self.action_area.pack_start(OkButton)
		OkButton.set_flags(gtk.CAN_DEFAULT)

		
		SaveButton = self.app.CreateIconButton(gtk.STOCK_SAVE, "stock", self.save, _("Save"))
		self.action_area.pack_start(SaveButton)
		SaveButton.set_flags(gtk.CAN_DEFAULT)


		CancelButton = self.app.CreateIconButton(gtk.STOCK_CANCEL, "stock", self.quit, _("Cancel"))
		self.action_area.pack_start(CancelButton)
		CancelButton.set_flags(gtk.CAN_DEFAULT)
		CancelButton.grab_default()
		

		self.ret = None
	
	def input_box(self, title=_("Input Box"), message="", message2= "", key='', value=None, modal= True, List=[], vlist=[], second=True):
		try:
			win = EntryDialog(self, message,  message2, key, value, modal, List, vlist, second)
			win.set_title(title)
			win.show()
			win.run()
			win.destroy()
			return win.ret, win.ret2
		
		except Exception,e:
			print e
			
	def ServerHostChanged(self, widget):
		if self.config == {}: return
		if "server" in self.config:
			#self.app.Networking.Send(messages.ConfigSet("server", "host", self.EntryServerHost.get_text()))
			self.config["server"]["host"] = self.EntryServerHost.get_text()
			
	def ServerUsernameChanged(self, widget):
		if self.config == {}: return
		if "server" in self.config:
			self.config["server"]["username"] = self.EntryServerUsername.get_text()
			
	def ServerPortChanged(self, widget):
		if self.config == {}: return
		if "server" in self.config:
			self.config["server"]["port"] = int( self.ServerPort.get_value() )
			
	def ServerPasswordChanged(self, widget):
		if self.config == {}: return
		if "server" in self.config:
			self.config["server"]["password"] = self.EntryServerPassword.get_text()
			
	def EncodingFSChanged(self, widget):
		if self.config == {}: return
		if "encoding" in self.config:
			self.config["encoding"]["filesystem"] = self.filesystemEncoding.child.get_text()
			
	def EncodingNWChanged(self, widget):
		if self.config == {}: return
		if "encoding" in self.config:
			self.config["encoding"]["network"] = self.networkEncoding.child.get_text()
			
	def LastPortChanged(self, widget):
		if self.config == {}:
			return
		if "clients.bind" in self.config:
			self.config["clients.bind"]["last"] = str(int( self.LastPort.get_value() ))
			
	def FirstPortChanged(self, widget):
		if self.config == {}:
			return
		if "clients.bind" in self.config:
			self.config["clients.bind"]["first"] =str( int( self.FirstPort.get_value() ))
			
	def InterfacePasswordChanged(self, widget):
		if self.config == {}:
			return
		if "interfaces" in self.config:
			self.config["interfaces"]["password"] = self.interfacePassword.get_text()
			
	def ConnectModeChanged(self, widget):
		if self.config == {}:
			return
		if "clients" in self.config:
			self.config["clients"]["connectmode"] = self.connectMode.child.get_text()
			
		
	def PrivilegeBuddiesChanged(self, widget):
		if self.config == {}:
			return
		if self.privilege_buddies_Check.get_active() == True:
			self.config["transfers"]["privilege_buddies"] = "true"
		else:
			self.config["transfers"]["privilege_buddies"] = "false"
		
	def EnableBuddySharesChanged(self, widget):
		if self.config == {}:
			return
		if self.have_buddy_shares_Check.get_active() == True:
			self.config["transfers"]["have_buddy_shares"] = "true"
		else:
			self.config["transfers"]["have_buddy_shares"] = "false"
		
	def TrustUploadsChanged(self, widget):
		if self.config == {}:
			return
		if self.trusting_uploads_Check.get_active() == True:
			self.config["transfers"]["trusting_uploads"] = "true"
			
		else:
			self.config["transfers"]["trusting_uploads"] = "false"
		
	def OnlyBuddiesChanged(self, widget):
		if self.config == {}:
			return
		if self.only_buddies_Check.get_active() == True:
			self.config["transfers"]["only_buddies"] = "true"
			
		else:
			self.config["transfers"]["only_buddies"] = "false"
		
	def UserWarningsChanged(self, widget):
		if self.config == {}:
			return
		if self.user_warnings_Check.get_active() == True:
			self.config["transfers"]["user_warnings"] = "true"
			
		else:
			self.config["transfers"]["user_warnings"] = "false"
		
	def UploadSlotsChanged(self, widget):
		if self.config != {}:
			self.config["transfers"]["upload_slots"] = str(int( self.uploadSlots.get_value() ))
			
	def DownloadSlotsChanged(self, widget):
		if self.config != {}:
			self.config["transfers"]["download_slots"] = str(int( self.downloadSlots.get_value() ))
		
	def UploadRateChanged(self, widget):
		if self.config != {}:
			self.config["transfers"]["upload_rate"] = str(int( self.uploadRate.get_value() ))
		
	def DownloadRateChanged(self, widget):
		if self.config != {}:
			self.config["transfers"]["download_rate"] = str(int( self.downloadRate.get_value() ))
		
	def LoggingChanged(self, widget):
		if self.config != {}:
			self.config["logging"]["output"] = str(int( self.LogMethod.get_active() ))
		
		
	def DownloadDirChanged(self, widget):
		if self.config != {}:
			self.config["transfers"]["download-dir"] = self.EntryDownloadDIr.get_text()
		
	def IncompleteDirChanged(self, widget):
		if self.config != {}:
			self.config["transfers"]["incomplete-dir"] = self.EntryIncompleteDir.get_text()
		
	def DownloadDBChanged(self, widget):
		if self.config != {}:
			self.config["transfers"]["downloads"] = self.EntryDownloadsDBase.get_text()
		
	def NormalSharesDBChanged(self, widget):
		if self.config != {}:
			self.config["shares"]["database"] = self.EntryNormalShares.get_text()
		
	def BuddySharesDBChanged(self, widget):
		if self.config != {}:
			self.config["buddy.shares"]["database"]= self.EntryBuddyOnlyShares.get_text()
		
	def DefaultTickerChanged(self, widget):
		if self.config != {}:
			self.config["default-ticker"]["ticker"] = self.defaultTicker.get_text()
		
	def UserinfoTextChanged(self, widget):
		if self.importinguserinfo == 1:
			return
		string = None
 		try:
			string = self.userinfoBuffer.get_text(self.userinfoBuffer.get_start_iter(), self.userinfoBuffer.get_end_iter())
		except Exception, e:
			self.Bug(_("Loading userinfo into the Config failed.."), str(e))
			
		if string is not None:
			if self.config != {}:
				self.config["userinfo"]["text"] = string

	def AddItemToList(self, node, message, message2, treeview):
		
		key, value = self.input_box(title=_("Add Config setting"), message=message, message2=message2, key="", value="", modal= True, List=[], vlist=[], second=True)
	
		if node == None or key == None:
			return
		if key == "" or key.isspace():
			return
		
		if value == None:
			value = ""

		if node in self.config:
			if key in self.config[node]:
				self.Bug(_("Item %s already in the %s list. Please edit instead.") %(key, node), "")
				return
			# a genuine new key
			self.config[node][key] = value
	
			self.parents[node][key] = treeview.get_model().append( None, [ key, value ] )
			
	def OnAddTrusted(self, widget):
		treeview = self.TrustedTreeview
		self.AddItemToList("trusted", _("Trusted User"), _("Comment"), treeview)
	
	def OnAddBuddy(self, widget):
		treeview = self.BuddiesTreeview
		self.AddItemToList("buddies", _("Buddied User"), _("Comment"), treeview)
	
	def OnAddBanned(self, widget):
		treeview = self.BannedTreeview
		self.AddItemToList("banned", _("Banned User"), _("Comment"), treeview)

	def OnAddInterface(self, widget):
		treeview = self.interfacesTreeview
		self.AddItemToList("interfaces.bind", _("Socket / Port"), None, treeview)
	
	def OnAddIgnored(self, widget):
		treeview = self.IgnoredTreeview
		self.AddItemToList("ignored", _("Ignored User"), _("Comment"), treeview)
	
	def OnAddAlert(self, widget):
		treeview = self.AlertsTreeview
		self.AddItemToList("alerts", _("User Status Alert"), None, treeview)
	
	def OnAddTicker(self, widget):
		treeview = self.tickersTreeview
		self.AddItemToList("tickers", _("Ticker's Room"), _("Ticker"), treeview)
		
	def OnAddEncoding(self, widget):
		treeview = self.encodingsTreeview
		self.AddItemToList("encoding.rooms", _("Room"), _("Encoding"), treeview)
		
	def OnAddAutojoin(self, widget):
		treeview = self.autojoinTreeview
		self.AddItemToList("autojoin", _("AutoJoin Room"), None, treeview)

	
	def TreeViewSelection3(self, model, path, iter):
		key = model.get_value(iter, 0)
		value = model.get_value(iter, 1)
		self.selected_items = [key, value]
		
	def EditItemToList(self, node, message, message2, key, value, num, treeview):
		self.key = key
		self.value = value
		key, value = self.input_box(title="Edit Config setting", message=message, message2=message2, key=key, value=value, modal= True, List=[], vlist=[], second=True)
		
		if node == None or key == None:
			return
		if key == "" or key.isspace():
			return
		
		if value == None:
			value = ""

		if node in self.config:
			# a genuine new key
			
			self.config[node][key] = value
			
			if self.key == key:
				treeview.get_model().set(self.parents[node][key], 0, key, 1, value)
				# Remove old key
			else:
				treeview.get_model().remove(self.parents[node][self.key])
				del self.config[node][self.key]
				
				treeview.get_model().append(None, [key, value])
	
	def OnEditEncoding(self, widget):
		treeview = self.encodingsTreeview
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection3)
		if self.selected_items != []:
			key, value = self.selected_items
			self.EditItemToList("encoding.rooms", _("Room"), _("Encoding"), key, value, None, treeview)
		

	def OnEditAlert(self, widget):
		# 2 column treeview
		treeview = self.AlertsTreeview
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection3)
		if self.selected_items != []:
			key, value = self.selected_items
	 		
			self.EditItemToList("alerts", _("User Status Alert"), None, key, value, None, treeview)
			
	def OnEditInterface(self, widget):
		# 2 column treeview
		treeview = self.interfacesTreeview
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection3)
		if self.selected_items != []:
			key, value = self.selected_items
	 		
			self.EditItemToList("interfaces.bind", _("Socket / Port"), None, key, value, None, treeview)
			
	def OnEditAutojoin(self, widget):
		# 2 column treeview
		treeview = self.autojoinTreeview
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection3)
		if self.selected_items != []:
			key, value = self.selected_items
	 		
			self.EditItemToList("autojoin", _("AutoJoin Room"), None, key, value, None, treeview)
						
	def OnEditTicker(self, widget):
		treeview = self.tickersTreeview
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection3)
		if self.selected_items != []:
			key, value = self.selected_items
			self.EditItemToList("tickers", _("Ticker's Room"), _("Ticker"), key, value, None, treeview)
			
	def OnEditTrusted(self, widget):
		treeview = self.TrustedTreeview
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection3)
		if self.selected_items != []:
			key, value = self.selected_items
			self.EditItemToList("trusted", _("Trusted User"), _("Comment"), key, value, None, treeview)
			
	def OnEditBuddy(self, widget):
		treeview = self.BuddiesTreeview
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection3)
		if self.selected_items != []:
			key, value = self.selected_items
			self.EditItemToList("buddies", _("Buddied User"), _("Comment"), key, value, None, treeview)
			
	def OnEditBanned(self, widget):
		treeview = self.encodingsTreeview
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection3)
		if self.selected_items != []:
			key, value = self.selected_items
			self.EditItemToList("banned", _("Banned User"), _("Comment"), key, value, None, treeview)
	
	def OnEditIgnored(self, widget):
		treeview = self.encodingsTreeview
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection3)
		if self.selected_items != []:
			key, value = self.selected_items
			self.EditItemToList("ignored", _("Ignored User"), _("Comment"), key, value, num, treeview)

	def RemoveItemFromList(self, node, key, value, treeview):
		if node == None or key == None:
			return
		if key == "" or key.isspace():
			return
		
		if node in self.config:
			del self.config[node][key]
			treeview.get_model().remove(self.parents[node][key])

	def OnRemoveAlert(self, widget):
		# 2 column treeview
		treeview = self.AlertsTreeview
		node = "alerts"
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection3)
		if self.selected_items != []:
			print self.selected_items
			key, value = self.selected_items
	 		
			self.RemoveItemFromList(node, key, None, treeview)
		
	def OnRemoveTicker(self, widget):
		treeview = self.tickersTreeview
		node="tickers"
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection3)
		if self.selected_items != []:
			key, value = self.selected_items
			self.RemoveItemFromList(node, key, value, treeview)
		
	def OnRemoveTrusted(self, widget):
		treeview = self.TrustedTreeview
		node = "trusted"
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection3)
		if self.selected_items != []:
			key, value = self.selected_items
			self.RemoveItemFromList(node, key, value, treeview)
		
	def OnRemoveBuddy(self, widget):
		treeview = self.BuddiesTreeview
		node="buddies"
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection3)
		if self.selected_items != []:
			key, value = self.selected_items
			self.RemoveItemFromList(node, key, None, treeview)
		
	def OnRemoveBanned(self, widget):
		treeview = self.BannedTreeview
		node="banned"
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection3)
		if self.selected_items != []:
			key, value = self.selected_items
			
			self.RemoveItemFromList(node, key, None, treeview)
		
	def OnRemoveIgnored(self, widget):
		treeview = self.IgnoredTreeview
		node="ignored"
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection3)
		if self.selected_items != []:
			key, value = self.selected_items
			
			self.RemoveItemFromList(node, key, value, treeview)
		
	def OnRemoveInterface(self, widget):
		# 2 column treeview
		treeview = self.interfacesTreeview
		node="interface"
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection3)
		if self.selected_items != []:
			key = self.selected_items
	 		
			self.RemoveItemFromList(node, key, None, treeview)

	def OnRemoveEncoding(self, widget):
		treeview = self.encodingsTreeview
		node="encoding.rooms"
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection3)
		if self.selected_items != []:
			key, value = self.selected_items
			
			self.RemoveItemFromList(node, key, value, treeview)

	def OnRemoveAutojoin(self, widget):
		# 2 column treeview
		node ="autojoin"
		treeview = self.autojoinTreeview
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection3)
		if self.selected_items != []:

			for key in self.selected_items:
				self.RemoveItemFromList(node, key, None, treeview)

		
	def OpenFile(self, widget, title, filters):
		dialog = gtk.FileChooserDialog(title=title, parent=None, action=gtk.FILE_CHOOSER_ACTION_OPEN, buttons=(gtk.STOCK_OK, gtk.RESPONSE_ACCEPT, gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT))
		dialog.set_select_multiple(False)
		if os.environ.has_key(["HOME"]) and os.path.exists(os.environ["HOME"]+"/.museekd"):
			dialog.set_current_folder_uri("file://"+os.environ["HOME"]+"/.museekd")
		
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

	def OnClearUserinfo(self, widget):
		self.config["userinfo"]["text"] = ""
		self.refreshConfigDisplay()
		
	def OnImportUserinfo(self, widget):
		self.importinguserinfo = 1
		text = self.OpenFile(widget, _("Import Userinfo from a text file"), ["*"])
		if text != None:
			if os.path.getsize(text) > 100000:
				self.Bug(_("Userinfo file is greater than 100KBytes, not using."), "")
				return
			textfile = open(text)

			self.config["userinfo"]["text"] = textfile.read()
			self.refreshConfigDisplay()
		self.importinguserinfo = 0
			
	def OnDownloadDir(self, widget):
		directory = self.OpenDirectory(widget, _("Choose a Download Directory"))
		if directory != None:
			self.config["transfers"]["download-dir"] = directory
			self.refreshConfigDisplay()
	
	def OnIncompleteDir(self, widget):
		directory = self.OpenDirectory(widget, _("Choose a Incomplete Downloads Directory"))
		if directory != None:
			self.config["transfers"]["incomplete-dir"] = directory
			self.refreshConfigDisplay()
	
	def OnDownloadDBase(self, widget):
		file = self.OpenFile(widget, _("Choose a Download Database"), ["*.downloads"])
		if file != None:
			self.config["transfers"]["downloads"] = file
			self.refreshConfigDisplay()

	def OnNormalDBase(self, widget):
		file = self.OpenFile(widget, _("Choose a Normal Shares Database"), ["*.shares"])
		if file != None:
			if "shares" not in self.config:
				self.config["shares"] = {}
			self.config["shares"]["database"] = file
			self.refreshConfigDisplay()
			
	def OnBuddyDBase(self, widget):
		file = self.OpenFile(widget, _("Choose a Buddy Shares Database"), ["*.buddyshares"])
		if file != None:
			if "buddy.shares" not in self.config:
				self.config["buddy.shares"] = {}
			self.config["buddy.shares"]["database"] = file
			self.refreshConfigDisplay()
			
	def OnOpenImage(self, widget):
		newimage = self.OpenImage(widget, _("Select an Userinfo Image"), ["*.jpg", "*.jpeg", "*.png","*.bmp", "*.xpm", "*.ico","*.gif"])
		if newimage != None:
			try:
				image = open(newimage, 'rb').read()
				self.app.Networking.Send(messages.ConfigSetUserImage(image))
				
			except Exception, e:
				self.Bug("Creating image", str(e))
			self.refreshConfigDisplay()
			
	def OpenImage(self, widget, title, filters):
		dialog = gtk.FileChooserDialog(title=title, parent=None, action=gtk.FILE_CHOOSER_ACTION_OPEN, buttons=(gtk.STOCK_OK, gtk.RESPONSE_ACCEPT, gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT))
		dialog.set_select_multiple(False)
		dialog.set_current_folder_uri("file://"+os.getcwd())
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
		
	def OnClearImage(self, widget):
		
		os.remove(self.config["userinfo"]["image"])
		image = file(self.config["userinfo"]["image"], "w")
		image.close()

		self.refreshConfigDisplay()
		
	def OnAddNormalDirs(self, widget):
		directory = self.OpenDirectory(widget, _("Add a buddy-only Shared Directory"))
		if directory != None:
			
			self.muscan_execute("muscan -c " + self.CONFIG_PATH + " -v -s "+ directory)
			self.OnRefreshNormalDirs(None)
	def OnAddBuddyDirs(self, widget):
		directory = self.OpenDirectory(widget, _("Add a buddy-only Shared Directory"))
		if directory != None:
			self.muscan_execute("muscan -c " + self.CONFIG_PATH + " -v -b -s "+ directory)
			self.OnRefreshBuddyDirs(None)
			
	def OnRemoveBuddyDirs(self, widget):
		treeview = self.BuddyDirTreeview
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection3)
		if self.selected_items != []:
			key, num = self.selected_items
			if key != "" and key is not None:
				self.muscan_execute("muscan -c " + self.CONFIG_PATH + " -b -v -u "+ key)
				self.OnRefreshBuddyDirs(None)
			
	def OnRemoveNormalDirs(self, widget):
		treeview = self.NormalDirTreeview

		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection3)
		if self.selected_items != []:
			key, num = self.selected_items
			if key != "" and key is not None:
				self.muscan_execute("muscan -c " + self.CONFIG_PATH + " -v -u "+ key)
				self.OnRefreshNormalDirs(None)
				
	def muscan_execute(self, command):
		commands.getoutput(command)
		#z= subprocess.Popen( command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
		#stdout_text, stderr_text = z.communicate()
		#z.wait()
		#stdout_text = stdout_text.split('\n')
		#for line in stdout_text:
			#if line.isspace() or line == '':
				#pass
			#else:
				#print line

	def OnRescanBuddyDirs(self, widget):
		
		self.muscan_execute("muscan -c", self.CONFIG_PATH + " -b -v -r")
		self.Statusbar.pop(self.status_context_id)
		self.Statusbar.push(self.status_context_id, "Rescanned Buddy shares")
		
	def OnRescanNormalDirs(self, widget):
		self.muscan_execute("muscan -c "+ self.CONFIG_PATH +" -v  -r")
		self.Statusbar.pop(self.status_context_id)
		self.Statusbar.push(self.status_context_id, "Rescanned Normal shares")
		
	def OnRefreshBuddyDirs(self, widget):
		p = "/usr/bin/muscan"
		if os.path.exists(p):
			output = commands.getoutput("muscan -c "+ self.CONFIG_PATH + " -b  -l")
			stdout_text = output.split('\n')

			self.SharedDirs["buddy"] = []
			for line in stdout_text:
				if line.isspace() or line == '':
					pass
				else:
					self.SharedDirs["buddy"].append(line)
			self.createTreeFor( self.BuddyDirTreestore, self.BuddyDirTreeview, "buddydirs")

	def OnRefreshNormalDirs(self, widget):
		p = "/usr/bin/muscan"
		if os.path.exists(p):
		
			output = commands.getoutput("muscan -c "+ self.CONFIG_PATH + "  -l")
			stdout_text = output.split('\n')
			
			self.SharedDirs["normal"] = []
			for line in stdout_text:
				if line.isspace() or line == '':
					pass
				else:
					self.SharedDirs["normal"].append(line)
			self.createTreeFor( self.NormalDirTreestore, self.NormalDirTreeview, "normaldirs")
					
	def Bug(self, where, message):
		try:
			#print where, message
			return
			win = BugDialog(self, where, message)
			if message != "":
				win.set_title(_("Exception Detected"))
			else:
				win.set_title(_("Problem Detected"))
			win.show()
			win.run()
			win.destroy()
		
		except Exception,e:
			print e
			
	def OpenDirectory(self, widget, title):
		dialog = gtk.FileChooserDialog(title=title, parent=None, action=gtk.FILE_CHOOSER_ACTION_SELECT_FOLDER, buttons=(gtk.STOCK_OK, gtk.RESPONSE_ACCEPT, gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT))
		dialog.set_select_multiple(False)
		dialog.set_current_folder_uri("file://"+os.getcwd())

		response = dialog.run()
		
		if response == gtk.RESPONSE_ACCEPT:
			
			res = dialog.get_filenames()
			for dir in res:
				directory = dir
		else:
			directory = res = None
		
		dialog.destroy()
		return directory
		
	def OpenConfig(self, widget):
		dialog = gtk.FileChooserDialog(title=_("Select the Museek Daemon Config"), parent=None, action=gtk.FILE_CHOOSER_ACTION_OPEN, buttons=(gtk.STOCK_OK, gtk.RESPONSE_ACCEPT, gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT))
		dialog.set_select_multiple(False)
		if os.environ.has_key(["HOME"]) and os.path.exists(os.environ["HOME"]+"/.museekd"):
			dialog.set_current_folder_uri("file://"+os.environ["HOME"]+"/.museekd")
		ff = gtk.FileFilter()
		ff.add_pattern("*.xml")
		dialog.set_property("filter", ff)
		response = dialog.run()
		
		if response == gtk.RESPONSE_ACCEPT:
			
			res = dialog.get_filenames()
			for file in res:
				self.CONFIG_PATH = file
				self.tryReadConfig()
		else:
			res = None
		
		dialog.destroy()
		
	def TabServer(self):
		self.label1 = gtk.Label(_("Server"))
		self.label1.show()
		
		self.ServerSW = gtk.ScrolledWindow()
		self.ServerSW.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.ServerSW.show()
		self.ServerSW.set_shadow_type(gtk.SHADOW_NONE)
	
	
		self.vbox2 = gtk.VBox(False, 5)
		self.vbox2.show()
		self.vbox2.set_spacing(5)
		self.vbox2.set_border_width(5)
	
		self.hbox2 = gtk.HBox(False, 5)
		self.hbox2.show()
		self.hbox2.set_spacing(5)
	
		self.Server_Label = gtk.Label(_("Server Host:"))
		self.Server_Label.show()
		self.hbox2.pack_start(self.Server_Label, False, False, 0)
	
		self.EntryServerHost = gtk.Entry()
		self.EntryServerHost.set_text("")
		self.EntryServerHost.set_editable(True)
		self.EntryServerHost.show()
		self.EntryServerHost.set_visibility(True)
		self.EntryServerHost.connect("changed", self.ServerHostChanged)

		self.hbox2.pack_end(self.EntryServerHost, False, True, 0)
	
		self.vbox2.pack_start(self.hbox2, False, False, 0)
	
		self.hbox5 = gtk.HBox(False, 0)
		self.hbox5.show()
		self.hbox5.set_spacing(0)
	
		self.s_port_Label = gtk.Label(_("Server Port:"))
		self.s_port_Label.show()
		self.hbox5.pack_start(self.s_port_Label, False, False, 0)
		aj = gtk.Adjustment(value=2240, lower=0, upper=65535, step_incr=1, page_incr=0, page_size=0)
		self.ServerPort = gtk.SpinButton(aj)
		self.ServerPort.set_numeric(True)
		self.ServerPort.show()
		self.ServerPort.connect("changed", self.ServerPortChanged)
		
		self.hbox5.pack_end(self.ServerPort, False, True, 0)
	
		self.vbox2.pack_start(self.hbox5, False, False, 0)
	
		self.hbox4 = gtk.HBox(False, 5)
		self.hbox4.show()
		self.hbox4.set_spacing(5)
	
		self.s_username_Label = gtk.Label(_("Server Username:"))
		self.s_username_Label.show()
		self.hbox4.pack_start(self.s_username_Label, False, False, 0)
	
		self.EntryServerUsername = gtk.Entry()
		self.EntryServerUsername.set_text("")
		self.EntryServerUsername.set_editable(True)
		self.EntryServerUsername.show()
		self.EntryServerUsername.set_visibility(True)
		self.EntryServerUsername.connect("changed", self.ServerUsernameChanged)
		self.hbox4.pack_end(self.EntryServerUsername, False, True, 0)
	
		self.vbox2.pack_start(self.hbox4, False, False, 0)
	
		self.hbox3 = gtk.HBox(False, 5)
		self.hbox3.show()
		self.hbox3.set_spacing(5)
	
		self.s_password_Label = gtk.Label(_("Server Password:"))
		self.s_password_Label.show()
		self.hbox3.pack_start(self.s_password_Label, False, False, 0)
	
		self.EntryServerPassword = gtk.Entry()
		self.EntryServerPassword.set_text("")
		self.EntryServerPassword.set_editable(True)
		self.EntryServerPassword.show()
		self.EntryServerPassword.set_visibility(False)
		
		self.EntryServerPassword.connect("changed", self.ServerPasswordChanged)
		self.hbox3.pack_end(self.EntryServerPassword, False, True, 0)
	
		self.vbox2.pack_start(self.hbox3, False, False, 0)
	
		self.frame1 = gtk.Frame()
		self.frame1.show()
		self.frame1.set_shadow_type(gtk.SHADOW_IN)
	
		self.alignment1 = gtk.Alignment(0.5, 0.5, 1, 1)
		self.alignment1.show()
	
		self.vbox3 = gtk.VBox(False, 5)
		self.vbox3.show()
		self.vbox3.set_spacing(5)
		self.vbox3.set_border_width(5)

		self.hbox7 = gtk.HBox(False, 5)
		self.hbox7.show()
		self.hbox7.set_spacing(5)
	
		self.filesystem_Label = gtk.Label(_("Filesystem: "))
		self.filesystem_Label.set_padding(5, 0)
		self.filesystem_Label.show()
		self.hbox7.pack_start(self.filesystem_Label, False, False, 0)
	
		self.filesystemEncoding_List = gtk.ListStore(gobject.TYPE_STRING)
		self.filesystemEncoding = gtk.ComboBoxEntry()
		self.filesystemEncoding.show()
		for encoding in self.encodings:
			self.filesystemEncoding_List.append([encoding])
		self.filesystemEncoding.connect("changed", self.EncodingFSChanged)
		
		self.filesystemEncoding.set_model(self.filesystemEncoding_List)
		self.filesystemEncoding.set_text_column(0)
		self.hbox7.pack_end(self.filesystemEncoding, False, True, 0)
	
		self.vbox3.pack_start(self.hbox7, False, False, 0)
	
		self.hbox8 = gtk.HBox(False, 5)
		self.hbox8.show()
		self.hbox8.set_spacing(5)
	
		self.network_Label = gtk.Label(_("Network:"))
		self.network_Label.set_padding(5, 0)
		self.network_Label.show()
		self.hbox8.pack_start(self.network_Label, False, False, 0)
	
		self.networkEncoding_List = gtk.ListStore(gobject.TYPE_STRING)
		self.networkEncoding = gtk.ComboBoxEntry()
		self.networkEncoding.show()
		for encoding in self.encodings:
			self.networkEncoding_List.append([encoding])
		self.networkEncoding.connect("changed", self.EncodingNWChanged)
		
		self.networkEncoding.set_model(self.networkEncoding_List)
		self.networkEncoding.set_text_column(0)
		self.hbox8.pack_end(self.networkEncoding, False, True, 0)
	
		self.vbox3.pack_start(self.hbox8, False, False, 0)
	
		self.alignment1.add(self.vbox3)
	
		self.frame1.add(self.alignment1)
	
		self.encodings__Label = gtk.Label()
		self.encodings__Label.set_markup(_("<b>Encodings</b>") )
		self.encodings__Label.show()
		self.frame1.set_label_widget(self.encodings__Label)
	
		self.vbox2.pack_start(self.frame1, False, False, 0)
	
		self.hbox33 = gtk.HBox(False, 5)
		self.hbox33.show()
		self.hbox33.set_spacing(5)
	
		self.fp_Label = gtk.Label(_("First Soulseek Port: "))
		self.fp_Label.show()
		self.hbox33.pack_start(self.fp_Label, False, False, 0)
		
		fp = gtk.Adjustment(value=2239, lower=0, upper=65535, step_incr=1, page_incr=0, page_size=0)
		self.FirstPort = gtk.SpinButton(fp)
		self.FirstPort.show()
		self.FirstPort.connect("changed", self.FirstPortChanged)
		
		self.hbox33.pack_start(self.FirstPort, True, True, 0)
	
		self.lp_Label = gtk.Label(_("Last Soulseek Port:"))
		self.lp_Label.show()
		self.hbox33.pack_start(self.lp_Label, False, False, 0)
		lp = gtk.Adjustment(value=2234, lower=0, upper=65535, step_incr=1, page_incr=0, page_size=0)
		self.LastPort = gtk.SpinButton(lp)
		self.LastPort.show()
		self.LastPort.connect("changed", self.LastPortChanged)
		
		self.hbox33.pack_start(self.LastPort, True, True, 0)
	
		self.vbox2.pack_start(self.hbox33, False, False, 0)
		self.LoggingHbox = gtk.HBox(False, 5)
		self.LoggingHbox.show()
		self.LoggingHbox.set_spacing(5)
		
		
		self.LogMethod = gtk.CheckButton()
		self.LogMethod.set_active(False)
		self.LogMethod.set_label(_("Output to Syslog instead of standard output"))
		self.LogMethod.show()
		self.LogMethod.connect("clicked", self.LoggingChanged)

		self.LoggingHbox.pack_end(self.LogMethod, False, True, 0)
	
		self.vbox2.pack_start(self.LoggingHbox, False, False, 0)
		self.ServerSW.add_with_viewport(self.vbox2)
	
		
		
	def TabMuseekClients(self):
		
		self.MuseekClientsSW = gtk.ScrolledWindow()
		self.MuseekClientsSW.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.MuseekClientsSW.show()
		self.MuseekClientsSW.set_shadow_type(gtk.SHADOW_NONE)
	
		self.viewport3 = gtk.Viewport()
		self.viewport3.show()
		self.viewport3.set_shadow_type(gtk.SHADOW_IN)
	
		self.vbox4 = gtk.VBox(False, 5)
		self.vbox4.show()
		self.vbox4.set_spacing(5)
		self.vbox4.set_border_width(5)
		
		self.hbox11 = gtk.HBox(False, 5)
		self.hbox11.show()
		self.hbox11.set_spacing(5)
		
	
		self.label33 = gtk.Label(_("Client Interfaces Password:"))
		self.label33.show()
		self.hbox11.pack_start(self.label33, False, False, 0)
	
		self.interfacePassword = gtk.Entry()
		self.interfacePassword.set_text("")
		self.interfacePassword.set_editable(True)
		self.interfacePassword.show()
		self.interfacePassword.set_visibility(False)
		self.interfacePassword.connect("changed", self.InterfacePasswordChanged)
		
		self.hbox11.pack_end(self.interfacePassword, True, True)
	
		self.vbox4.pack_start(self.hbox11, False, True)
	
		self.frame2 = gtk.Frame()
		self.frame2.show()
		self.frame2.set_shadow_type(gtk.SHADOW_IN)
	
		self.alignment2 = gtk.Alignment(0.5, 0.5, 1, 1)
		self.alignment2.show()
	
		self.hbox10 = gtk.HBox(False, 5)
		self.hbox10.show()
		self.hbox10.set_spacing(5)
		self.hbox10.set_border_width(5)
	
		self.scrolledwindow16 = gtk.ScrolledWindow()
		self.scrolledwindow16.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.scrolledwindow16.show()
		self.scrolledwindow16.set_shadow_type(gtk.SHADOW_IN)
		
		self.interfacesTreestore = gtk.TreeStore( str, str )
		self.interfacesTreeview = gtk.TreeView(self.interfacesTreestore)
		self.interfacesTreeview.set_property("rules-hint", True)
		self.interfacesTreestore.set_sort_column_id(0, gtk.SORT_ASCENDING)
		self.interfacesTreeview.show()
		self.interfacesTreeview.set_headers_visible(True)
		column = gtk.TreeViewColumn('Interfaces')
		cell = gtk.CellRendererText()
		cell.set_property('mode', gtk.CELL_RENDERER_MODE_EDITABLE)
		column.pack_start(cell, True)
		column.add_attribute(cell, 'text', 0)
		column.set_sort_column_id(0)
		self.interfacesTreeview.append_column(column)
		self.scrolledwindow16.add(self.interfacesTreeview)
	
		self.hbox10.pack_start(self.scrolledwindow16, True, True)
	
		self.vbox5 = gtk.VBox(False, 5)
		self.vbox5.show()
		self.vbox5.set_spacing(5)
		self.vbox5.set_border_width(3)

		
		self.AddInterface = self.app.CreateIconButton(gtk.STOCK_ADD, "stock", self.OnAddInterface, _("Add"))
	
		self.vbox5.pack_start(self.AddInterface, False, False)
		
		self.EditInterface = self.app.CreateIconButton(gtk.STOCK_EDIT, "stock", self.OnEditInterface, _("Edit"))
	
		self.vbox5.pack_start(self.EditInterface, False, False)
		
		self.RemoveInterface = self.app.CreateIconButton(gtk.STOCK_REMOVE, "stock", self.OnRemoveInterface, _("Remove"))

		
		self.vbox5.pack_start(self.RemoveInterface, False, False)
	
		self.hbox10.pack_end(self.vbox5, False, True)
	
		self.alignment2.add(self.hbox10)
	
		self.frame2.add(self.alignment2)
	
		self.label32 = gtk.Label()
		self.label32.set_markup(_("<b>Client Interface Ports &amp; Sockets</b>"))
		self.label32.show()
		self.frame2.set_label_widget(self.label32)
	
		self.vbox4.pack_start(self.frame2, True, True)
	
		self.viewport3.add(self.vbox4)
	
		self.MuseekClientsSW.add(self.viewport3)
	
		self.label3 = gtk.Label(_("Museek Clients"))
		self.label3.show()
		
	def TabTransfers(self):
		self.TransfersScrollWindow = gtk.ScrolledWindow()
		self.TransfersScrollWindow.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.TransfersScrollWindow.show()
		self.TransfersScrollWindow.set_shadow_type(gtk.SHADOW_NONE)
	
		self.viewport4 = gtk.Viewport()
		self.viewport4.show()
		self.viewport4.set_shadow_type(gtk.SHADOW_IN)
	
		self.vbox6 = gtk.VBox(False, 5)
		self.vbox6.show()
		self.vbox6.set_spacing(5)
		self.vbox6.set_border_width(5)
	
		self.hbox12 = gtk.HBox(False, 5)
		self.hbox12.show()
		self.hbox12.set_spacing(5)
		self.hbox12.set_border_width(3)
	
		self.privilege_buddies_Check = gtk.CheckButton()
		self.privilege_buddies_Check.set_active(False)
		self.privilege_buddies_Check.set_label(_("Privilege Buddies"))
		self.privilege_buddies_Check.show()
		self.privilege_buddies_Check.connect("clicked", self.PrivilegeBuddiesChanged)
		
		self.hbox12.pack_start(self.privilege_buddies_Check, False, False, 0)
	
		self.have_buddy_shares_Check = gtk.CheckButton()
		self.have_buddy_shares_Check.set_active(False)
		self.have_buddy_shares_Check.set_label(_("Enable Buddy Shares"))
		self.have_buddy_shares_Check.show()
		self.have_buddy_shares_Check.connect("clicked", self.EnableBuddySharesChanged)
		self.hbox12.pack_start(self.have_buddy_shares_Check, False, False, 0)
	
		self.only_buddies_Check = gtk.CheckButton()
		self.only_buddies_Check.set_active(False)
		self.only_buddies_Check.set_label(_("Only Share to Buddies"))
		self.only_buddies_Check.show()
		self.only_buddies_Check.connect("clicked", self.OnlyBuddiesChanged)
		self.hbox12.pack_start(self.only_buddies_Check, False, False, 0)
	
		self.vbox6.pack_start(self.hbox12, False, False, 0)
	
		self.hbox13 = gtk.HBox(False, 0)
		self.hbox13.show()
		self.hbox13.set_spacing(0)
		self.hbox13.set_border_width(3)
	
		self.trusting_uploads_Check = gtk.CheckButton()
		self.trusting_uploads_Check.set_active(False)
		self.trusting_uploads_Check.set_label(_("Allow Buddies to send you files"))
		self.trusting_uploads_Check.show()
		self.trusting_uploads_Check.connect("clicked", self.TrustUploadsChanged)
		
		self.hbox13.pack_start(self.trusting_uploads_Check, False, False, 0)
	
		self.vbox6.pack_start(self.hbox13, False, False, 0)
	
		self.hbox14 = gtk.HBox(False, 0)
		self.hbox14.show()
		self.hbox14.set_spacing(0)
		self.hbox14.set_border_width(3)
	
		self.user_warnings_Check = gtk.CheckButton()
		self.user_warnings_Check.set_active(False)
		self.user_warnings_Check.set_label(_("Send Automatic Warnings via Private Chat"))
		self.user_warnings_Check.show()
		self.user_warnings_Check.connect("clicked", self.UserWarningsChanged)
		
		self.hbox14.pack_start(self.user_warnings_Check, False, False, 0)
	
		self.vbox6.pack_start(self.hbox14, False, False, 0)
	
		self.hbox15 = gtk.HBox(False, 5)
		self.hbox15.show()
		self.hbox15.set_spacing(5)
		self.hbox15.set_border_width(3)
	
		self.label34 = gtk.Label(_("Upload Slots:"))
		self.label34.show()
		self.hbox15.pack_start(self.label34, False, False, 0)
		us = gtk.Adjustment(value=0, lower=0, upper=1000, step_incr=1, page_incr=0, page_size=0)
		
		self.uploadSlots = gtk.SpinButton(us)
		self.uploadSlots.show()
		self.uploadSlots.connect("value-changed", self.UploadSlotsChanged)
		
		self.hbox15.pack_start(self.uploadSlots, False, True, 0)
	
		self.label37 = gtk.Label(_("Upload rate:"))
		self.label37.show()
		self.hbox15.pack_start(self.label37, False, False, 0)
		us = gtk.Adjustment(value=0, lower=0, upper=1000, step_incr=1, page_incr=0, page_size=0)
		
		self.uploadRate = gtk.SpinButton(us)
		self.uploadRate.show()
		self.uploadRate.connect("value-changed", self.UploadRateChanged)
		
		self.hbox15.pack_start(self.uploadRate, False, True, 0)
		
		self.vbox6.pack_start(self.hbox15, False, True, 0)
		
		
		self.hbox18 = gtk.HBox(False, 5)
		self.hbox18.show()
		self.hbox18.set_spacing(5)
		self.hbox18.set_border_width(3)
	
		self.label36 = gtk.Label(_("Download Slots:"))
		self.label36.show()
		self.hbox18.pack_start(self.label36, False, False, 0)
		us = gtk.Adjustment(value=0, lower=0, upper=1000, step_incr=1, page_incr=0, page_size=0)
		
		self.downloadSlots = gtk.SpinButton(us)
		self.downloadSlots.show()
		self.downloadSlots.connect("value-changed", self.DownloadSlotsChanged)
		
		self.hbox18.pack_start(self.downloadSlots, False, True, 0)
	
		self.label38 = gtk.Label(_("Download rate:"))
		self.label38.show()
		self.hbox18.pack_start(self.label38, False, False, 0)
		us = gtk.Adjustment(value=0, lower=0, upper=1000, step_incr=1, page_incr=0, page_size=0)
		
		self.downloadRate = gtk.SpinButton(us)
		self.downloadRate.show()
		self.downloadRate.connect("value-changed", self.DownloadRateChanged)
		
		self.hbox18.pack_start(self.downloadRate, False, True, 0)
		
		self.vbox6.pack_start(self.hbox18, False, True, 0)		
		
		
		self.hbox19 = gtk.HBox(False, 5)
		self.hbox19.show()
		self.hbox19.set_spacing(5)
		self.hbox19.set_border_width(3)
		
		self.pabel= gtk.Label(_("Connection Mode:"))
		self.pabel.show()
		self.hbox19.pack_start(self.pabel, False, False, 0)
		
		self.connectMode_List = gtk.ListStore(gobject.TYPE_STRING)
		self.connectMode = gtk.ComboBoxEntry()
		self.connectMode.show()
		
		
		for item in ["passive", "active"]:
			self.connectMode_List.append([item])
	
		self.connectMode.set_model(self.connectMode_List)
		self.connectMode.set_text_column(0)
		self.connectMode.connect("changed", self.ConnectModeChanged)
		self.hbox19.pack_start(self.connectMode, False, False, 0)
		
		self.vbox6.pack_start(self.hbox19, False, True, 0)
		
		
		self.hbox16 = gtk.HBox(False, 5)
		self.hbox16.show()
		self.hbox16.set_spacing(5)
		self.hbox16.set_border_width(3)
	
		self.label35 = gtk.Label(_("Download Directory:"))
		self.label35.show()
		self.hbox16.pack_start(self.label35, False, False, 0)
	
		self.EntryDownloadDIr = gtk.Entry()
		self.EntryDownloadDIr.set_text("")
		self.EntryDownloadDIr.set_editable(True)
		self.EntryDownloadDIr.show()
		self.EntryDownloadDIr.set_visibility(True)
		self.EntryDownloadDIr.connect("changed", self.DownloadDirChanged)
		
		self.hbox16.pack_start(self.EntryDownloadDIr, True, True, 0)
		
		self.downloadDirButton = self.app.CreateIconButton(gtk.STOCK_OPEN, "stock", self.OnDownloadDir, "Select Directory")

	
		self.hbox16.pack_end(self.downloadDirButton, False, False, 0)
	
		self.vbox6.pack_start(self.hbox16,  False, False, 0)
	
		self.hbox17 = gtk.HBox(False, 5)
		self.hbox17.show()
		self.hbox17.set_spacing(5)
		self.hbox17.set_border_width(3)
	
		self.id_Label = gtk.Label(_("Incomplete Directory:"))

		self.id_Label.show()
		self.hbox17.pack_start(self.id_Label, False, False, 0)
	
		self.EntryIncompleteDir = gtk.Entry()
		self.EntryIncompleteDir.set_text("")
		self.EntryIncompleteDir.set_editable(True)
		self.EntryIncompleteDir.show()
		self.EntryIncompleteDir.set_visibility(True)
		self.EntryIncompleteDir.connect("changed", self.IncompleteDirChanged)
		
		self.hbox17.pack_start(self.EntryIncompleteDir, True, True, 0)
	
		self.incompleteDirButton = self.app.CreateIconButton(gtk.STOCK_OPEN, "stock", self.OnIncompleteDir, "Select Directory")

		self.hbox17.pack_end(self.incompleteDirButton, False, False, 0)
	
		self.vbox6.pack_start(self.hbox17,  False, False, 0)
	
		self.dButtons_hbox = gtk.HBox(False, 5)
		self.dButtons_hbox.show()
		self.dButtons_hbox.set_spacing(5)
		self.dButtons_hbox.set_border_width(3)
	
		self.ddb_Label = gtk.Label(_("Downloads Database:"))
		self.ddb_Label.show()
		self.dButtons_hbox.pack_start(self.ddb_Label, False, False, 0)
	
		self.EntryDownloadsDBase = gtk.Entry()
		self.EntryDownloadsDBase.set_text("")
		self.EntryDownloadsDBase.set_editable(True)
		self.EntryDownloadsDBase.show()
		self.EntryDownloadsDBase.set_visibility(True)
		self.EntryDownloadsDBase.connect("changed", self.DownloadDBChanged)
		
		self.dButtons_hbox.pack_start(self.EntryDownloadsDBase, True, True, 0)
		
		self.downloadsDBaseButton = self.app.CreateIconButton(gtk.STOCK_OPEN, "stock", self.OnDownloadDBase, "Select Database")
		
		self.downloadsDBaseButton.connect("clicked", self.OnDownloadDBase)
		
		self.dButtons_hbox.pack_end(self.downloadsDBaseButton, False, False, 0)
	
		self.vbox6.pack_start(self.dButtons_hbox,  False, False, 0)
	
		self.viewport4.add(self.vbox6)
	
		self.TransfersScrollWindow.add(self.viewport4)
	
		self.TransfersLabel = gtk.Label(_("Transfers"))
		self.TransfersLabel.show()
		
		
	def TabChatRoom(self):
		self.hboxchat = gtk.HBox(True, 5)
		self.hboxchat.set_border_width(5)
		self.hboxchat.show()
		
		self.ChatRoomNoteBook = gtk.Notebook()
		self.ChatRoomNoteBook.set_tab_pos(gtk.POS_TOP)
		self.ChatRoomNoteBook.set_scrollable(False)
		self.ChatRoomNoteBook.show()
		self.hboxchat.pack_start(self.ChatRoomNoteBook, True, True)
		
		self.autojoin_hbox = gtk.HBox(False, 5)
		
		self.autojoin_hbox.set_border_width(5)
		self.autojoin_hbox.show()
	
		self.autojoinScrollWindow2 = gtk.ScrolledWindow()
		self.autojoinScrollWindow2.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.autojoinScrollWindow2.show()
		self.autojoinScrollWindow2.set_shadow_type(gtk.SHADOW_IN)
	
		self.autojoinTreestore = gtk.TreeStore( str, str )
		self.autojoinTreeview = gtk.TreeView(self.autojoinTreestore)
		self.autojoinTreeview.set_property("rules-hint", True)
		self.autojoinTreestore.set_sort_column_id(0, gtk.SORT_ASCENDING)

		self.autojoinTreeview.show()
		self.autojoinTreeview.set_headers_visible(True)
		column = gtk.TreeViewColumn('Rooms')
		cell = gtk.CellRendererText()
		cell.set_property('mode', gtk.CELL_RENDERER_MODE_EDITABLE)
		column.pack_start(cell, True)
		column.add_attribute(cell, 'text', 0)
		column.set_sort_column_id(0)
		self.autojoinTreeview.append_column(column)
		
		self.autojoinScrollWindow2.add(self.autojoinTreeview)
	
		self.autojoin_hbox.pack_start(self.autojoinScrollWindow2, True, True, 0)
	
		self.vbox17 = gtk.VBox(False, 5)
		self.vbox17.show()
		self.vbox17.set_spacing(5)
		self.vbox17.set_border_width(3)
		
		
		self.AddAutojoin = self.app.CreateIconButton(gtk.STOCK_ADD, "stock", self.OnAddAutojoin, _("Add"))
		self.vbox17.pack_start(self.AddAutojoin, False, False, 0)
		
		self.EditAutojoin = self.app.CreateIconButton(gtk.STOCK_EDIT, "stock", self.OnEditAutojoin, _("Edit"))
		self.vbox17.pack_start(self.EditAutojoin, False, False, 0)
		
		self.RemoveAutojoin = self.app.CreateIconButton(gtk.STOCK_REMOVE, "stock", self.OnRemoveAutojoin, _("Remove"))
		self.vbox17.pack_start(self.RemoveAutojoin, False, False, 0)
	
		self.autojoin_hbox.pack_start(self.vbox17, False, True, 0)
		self.autojoin_hbox.show()
	
		self.autojoinLabel = gtk.Label(_("AutoJoined"))
		self.autojoinLabel.show()
		
			
		self.encodings_hbox = gtk.HBox(False, 5)
		self.encodings_hbox.set_border_width(5)
		self.encodings_hbox.show()
	
		self.encodingsScrollWindow2 = gtk.ScrolledWindow()
		self.encodingsScrollWindow2.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.encodingsScrollWindow2.show()
		self.encodingsScrollWindow2.set_shadow_type(gtk.SHADOW_IN)
	
		self.encodingsTreestore = gtk.TreeStore(  str, str )
		self.encodingsTreeview = gtk.TreeView(self.encodingsTreestore)
		self.encodingsTreeview.set_property("rules-hint", True)
		self.encodingsTreestore.set_sort_column_id(0, gtk.SORT_ASCENDING)
		self.encodingsTreeview.show()
		self.encodingsTreeview.set_headers_visible(True)
		column = gtk.TreeViewColumn('Rooms')
		cell = gtk.CellRendererText()
		cell.set_property('mode', gtk.CELL_RENDERER_MODE_EDITABLE)
		column.pack_start(cell, True)
		column.add_attribute(cell, 'text', 0)
		column.set_sort_column_id(0)
		self.encodingsTreeview.append_column(column)
		column = gtk.TreeViewColumn('Encodings')
		cell = gtk.CellRendererText()
		cell.set_property('mode', gtk.CELL_RENDERER_MODE_EDITABLE)
		column.pack_start(cell, True)
		column.add_attribute(cell, 'text', 1)
		column.set_sort_column_id(1)
		self.encodingsTreeview.append_column(column)
		self.encodingsScrollWindow2.add(self.encodingsTreeview)
		self.encodings_hbox.pack_start(self.encodingsScrollWindow2, True, True, 0)
	
		self.encodings_vbox = gtk.VBox(False, 5)
		self.encodings_vbox.show()
		self.encodings_vbox.set_spacing(5)
		self.encodings_vbox.set_border_width(3)
		
		self.AddEncoding = self.app.CreateIconButton(gtk.STOCK_ADD, "stock", self.OnAddEncoding, _("Add"))
		self.encodings_vbox.pack_start(self.AddEncoding, False, False, 0)
		
		self.EditEncoding = self.app.CreateIconButton(gtk.STOCK_EDIT, "stock", self.OnEditEncoding, _("Edit"))
		self.encodings_vbox.pack_start(self.EditEncoding, False, False, 0)
		
		self.RemoveEncoding = self.app.CreateIconButton(gtk.STOCK_REMOVE, "stock", self.OnRemoveEncoding, _("Remove"))
		self.encodings_vbox.pack_start(self.RemoveEncoding, False, False, 0)
		
	
		self.encodings_hbox.pack_start(self.encodings_vbox, False, True, 0)
	
		self.encodingsLabel = gtk.Label(_("Encodings"))
		self.encodingsLabel.show()
		
		self.vbox1 = gtk.VBox(False, 5)
		self.vbox1.set_border_width(5)
		self.vbox1.show()
	
		self.hbox1 = gtk.HBox(False, 5)
		self.hbox1.show()
		self.hbox1.set_spacing(5)
	
		self.defaultTickerLabel = gtk.Label(_("Default Ticker:"))
		self.defaultTickerLabel.show()
		self.hbox1.pack_start(self.defaultTickerLabel, False, False, 0)
	
		self.defaultTicker = gtk.Entry()
		self.defaultTicker.set_text("")
		self.defaultTicker.set_editable(True)
		self.defaultTicker.show()
		self.defaultTicker.set_visibility(True)
		self.defaultTicker.connect("changed", self.DefaultTickerChanged)
		
		self.hbox1.pack_start(self.defaultTicker, True, True, 0)
	
		self.vbox1.pack_start(self.hbox1, False, True, 0)
	
		self.hbox34 = gtk.HBox(False, 0)
		self.hbox34.show()
		self.hbox34.set_spacing(0)
	
		self.tickersScrollWindow = gtk.ScrolledWindow()
		self.tickersScrollWindow.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.tickersScrollWindow.show()
		self.tickersScrollWindow.set_shadow_type(gtk.SHADOW_IN)
	
		self.tickersTreestore = gtk.TreeStore(  str,str )
		self.tickersTreeview = gtk.TreeView(self.tickersTreestore)
		self.tickersTreeview.set_property("rules-hint", True)
		self.tickersTreestore.set_sort_column_id(0, gtk.SORT_ASCENDING)
		self.tickersTreeview.show()
		self.tickersTreeview.set_headers_visible(True)
		column = gtk.TreeViewColumn('Rooms')
		cell = gtk.CellRendererText()
		cell.set_property('mode', gtk.CELL_RENDERER_MODE_EDITABLE)
		column.pack_start(cell, True)
		column.add_attribute(cell, 'text', 0)
		column.set_sort_column_id(0)
		self.tickersTreeview.append_column(column)
		column = gtk.TreeViewColumn('Tickers')
		cell = gtk.CellRendererText()
		cell.set_property('mode', gtk.CELL_RENDERER_MODE_EDITABLE)
		column.pack_start(cell, True)
		column.add_attribute(cell, 'text', 1)
		column.set_sort_column_id(1)
		self.tickersTreeview.append_column(column)
		
		self.tickersScrollWindow.add(self.tickersTreeview)
	
		self.hbox34.pack_start(self.tickersScrollWindow, True, True, 0)
	
		self.vbox15 = gtk.VBox(False, 5)
		self.vbox15.show()
		self.vbox15.set_spacing(5)
		self.vbox15.set_border_width(3)
		
		
		self.AddTicker = self.app.CreateIconButton(gtk.STOCK_ADD, "stock", self.OnAddTicker, _("Add"))
		self.vbox15.pack_start(self.AddTicker, False, False, 0)
		
		self.EditTicker = self.app.CreateIconButton(gtk.STOCK_EDIT, "stock", self.OnEditTicker, _("Edit"))
		self.vbox15.pack_start(self.EditTicker, False, False, 0)
		
		self.RemoveTicker = self.app.CreateIconButton(gtk.STOCK_REMOVE, "stock", self.OnRemoveTicker, _("Remove"))
		self.vbox15.pack_start(self.RemoveTicker, False, False, 0)
		
	
		self.hbox34.pack_start(self.vbox15, False, True, 0)
	
		self.vbox1.pack_start(self.hbox34, True, True, 0)
	
		self.TickersLabel = gtk.Label(_("Tickers"))
		self.TickersLabel.show()
		self.ChatRoomNoteBook.append_page(self.autojoin_hbox, self.autojoinLabel)
	
		self.ChatRoomNoteBook.append_page(self.encodings_hbox, self.encodingsLabel)
	
		self.ChatRoomNoteBook.append_page(self.vbox1, self.TickersLabel)
	
	
		self.ChatRooms_Label = gtk.Label(_("Chat Rooms"))
		self.ChatRooms_Label.show()
		
	def TabUsers(self):
		self.hboxusers = gtk.HBox(True, 5)
		self.hboxusers.set_border_width(5)
		self.hboxusers.show()
		
		self.users_Label = gtk.Label(_("Users"))
		self.users_Label.show()
		
		self.UsersNoteBook = gtk.Notebook()
		self.UsersNoteBook.set_tab_pos(gtk.POS_TOP)
		self.UsersNoteBook.set_scrollable(False)
		self.UsersNoteBook.show()
		self.hboxusers.pack_start(self.UsersNoteBook, True, True)
		
		self.TabUsersBuddies()
		self.TabUsersTrusted()
		self.TabUsersBanned()
		self.TabUsersIgnored()
		self.TabUsersAlerts()
		
		
	def TabUsersBuddies(self):
		self.hbox22 = gtk.HBox(False, 5)
		self.hbox22.show()
		self.hbox22.set_spacing(5)
		self.hbox22.set_border_width(5)
		
		self.scrolledwindow19 = gtk.ScrolledWindow()
		self.scrolledwindow19.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.scrolledwindow19.show()
		self.scrolledwindow19.set_shadow_type(gtk.SHADOW_NONE)
	
		self.BuddiesTreestore = gtk.TreeStore(  str,str )
		self.BuddiesTreeview = gtk.TreeView(self.BuddiesTreestore)
		self.BuddiesTreeview.set_property("rules-hint", True)
		self.BuddiesTreestore.set_sort_column_id(0, gtk.SORT_ASCENDING)
		self.BuddiesTreeview.show()
		self.BuddiesTreeview.set_headers_visible(True)
		column = gtk.TreeViewColumn(_('Users'))
		cell = gtk.CellRendererText()
		cell.set_property('mode', gtk.CELL_RENDERER_MODE_EDITABLE)
		column.pack_start(cell, True)
		column.add_attribute(cell, 'text', 0)
		column.set_sort_column_id(0)
		self.BuddiesTreeview.append_column(column)
		column = gtk.TreeViewColumn(_('Comments'))
		cell = gtk.CellRendererText()
		cell.set_property('mode', gtk.CELL_RENDERER_MODE_EDITABLE)
		column.pack_start(cell, True)
		column.add_attribute(cell, 'text', 1)
		column.set_sort_column_id(1)
		self.BuddiesTreeview.append_column(column)
		
		self.scrolledwindow19.add(self.BuddiesTreeview)
	
		self.hbox22.pack_start(self.scrolledwindow19, True, True, 0)
	
		self.vbox7 = gtk.VBox(False, 5)
		self.vbox7.show()
		self.vbox7.set_spacing(5)
		self.vbox7.set_border_width(3)
	
		self.AddBuddy = self.app.CreateIconButton(gtk.STOCK_ADD, "stock", self.OnAddBuddy, _("Add"))
	
		self.vbox7.pack_start(self.AddBuddy, False, False, 0)
		
		self.EditBuddy = self.app.CreateIconButton(gtk.STOCK_EDIT, "stock", self.OnEditBuddy, _("Edit"))
	
		self.vbox7.pack_start(self.EditBuddy, False, False, 0)
		
		self.RemoveBuddy = self.app.CreateIconButton(gtk.STOCK_REMOVE, "stock", self.OnRemoveBuddy, _("Remove"))
		
		self.vbox7.pack_start(self.RemoveBuddy, False, False, 0)
	
		self.hbox22.pack_start(self.vbox7, False, True, 0)
	
		self.BuddiesTab = gtk.Label(_("Buddies"))
		self.BuddiesTab.show()
		self.UsersNoteBook.append_page(self.hbox22, self.BuddiesTab)
		
	def TabUsersBanned(self):
		
		self.hbox23 = gtk.HBox(False, 5)
		self.hbox23.show()
		self.hbox23.set_spacing(5)
		self.hbox23.set_border_width(5)
		
		self.BannedScrollWindow = gtk.ScrolledWindow()
		self.BannedScrollWindow.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.BannedScrollWindow.show()
		self.BannedScrollWindow.set_shadow_type(gtk.SHADOW_NONE)
	
	
		self.BannedTreestore = gtk.TreeStore(  str,str )
		self.BannedTreeview = gtk.TreeView(self.BannedTreestore)
		self.BannedTreeview.set_property("rules-hint", True)
		self.BannedTreestore.set_sort_column_id(0, gtk.SORT_ASCENDING)
		self.BannedTreeview.show()
		self.BannedTreeview.set_headers_visible(True)
		column = gtk.TreeViewColumn(_('Users'))
		cell = gtk.CellRendererText()
		cell.set_property('mode', gtk.CELL_RENDERER_MODE_EDITABLE)
		column.pack_start(cell, True)
		column.add_attribute(cell, 'text', 0)
		column.set_sort_column_id(0)
		self.BannedTreeview.append_column(column)
		column = gtk.TreeViewColumn(_('Comments'))
		cell = gtk.CellRendererText()
		cell.set_property('mode', gtk.CELL_RENDERER_MODE_EDITABLE)
		column.pack_start(cell, True)
		column.add_attribute(cell, 'text', 1)
		column.set_sort_column_id(1)
		self.BannedTreeview.append_column(column)

		self.BannedScrollWindow.add(self.BannedTreeview)
	
		self.hbox23.pack_start(self.BannedScrollWindow, True, True, 0)
	
		self.vbox8 = gtk.VBox(False, 5)
		self.vbox8.show()
		self.vbox8.set_spacing(5)
		self.vbox8.set_border_width(3)
		
		self.AddBanned = self.app.CreateIconButton(gtk.STOCK_ADD, "stock", self.OnAddBanned, _("Add"))
		self.vbox8.pack_start(self.AddBanned, False, False, 0)
		
		self.EditBanned = self.app.CreateIconButton(gtk.STOCK_EDIT, "stock", self.OnEditBanned, _("Edit"))
		self.vbox8.pack_start(self.EditBanned, False, False, 0)
		
		self.RemoveBanned = self.app.CreateIconButton(gtk.STOCK_REMOVE, "stock", self.OnRemoveBanned, _("Remove"))
		
		self.vbox8.pack_start(self.RemoveBanned, False, False, 0)
	
		self.hbox23.pack_start(self.vbox8, False, True, 0)
	
		self.BannedTab = gtk.Label(_("Banned"))
		self.BannedTab.show()
	
		
		self.UsersNoteBook.append_page(self.hbox23, self.BannedTab)
	
	def TabUsersIgnored(self):
		
		self.hbox24 = gtk.HBox(False, 5)
		self.hbox24.show()
		self.hbox24.set_spacing(5)
		self.hbox24.set_border_width(5)
		
		self.IgnoredScrollWindow = gtk.ScrolledWindow()
		self.IgnoredScrollWindow.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.IgnoredScrollWindow.show()
		self.IgnoredScrollWindow.set_shadow_type(gtk.SHADOW_NONE)
	
		self.IgnoredTreestore = gtk.TreeStore( str, str )
		self.IgnoredTreeview = gtk.TreeView(self.IgnoredTreestore)
		self.IgnoredTreeview.set_property("rules-hint", True)
		self.IgnoredTreestore.set_sort_column_id(0, gtk.SORT_ASCENDING)
		self.IgnoredTreeview.show()
		self.IgnoredTreeview.set_headers_visible(True)
		column = gtk.TreeViewColumn(_('Users'))
		cell = gtk.CellRendererText()
		cell.set_property('mode', gtk.CELL_RENDERER_MODE_EDITABLE)
		column.pack_start(cell, True)
		column.add_attribute(cell, 'text', 0)
		column.set_sort_column_id(0)
		self.IgnoredTreeview.append_column(column)
		column = gtk.TreeViewColumn(_('Comments'))
		cell = gtk.CellRendererText()
		cell.set_property('mode', gtk.CELL_RENDERER_MODE_EDITABLE)
		column.pack_start(cell, True)
		column.add_attribute(cell, 'text', 1)
		column.set_sort_column_id(1)
		self.IgnoredTreeview.append_column(column)
	
		self.IgnoredScrollWindow.add(self.IgnoredTreeview)
	
		self.hbox24.pack_start(self.IgnoredScrollWindow, True, True, 0)
	
		self.vbox9 = gtk.VBox(False, 5)
		self.vbox9.show()
		self.vbox9.set_spacing(5)
		self.vbox9.set_border_width(3)
	
		self.AddIgnored = self.app.CreateIconButton(gtk.STOCK_ADD, "stock", self.OnAddIgnored, _("Add"))
		self.vbox9.pack_start(self.AddIgnored, False, False, 0)
		
		self.EditIgnored = self.app.CreateIconButton(gtk.STOCK_EDIT, "stock", self.OnEditIgnored, _("Edit"))
	
		self.vbox9.pack_start(self.EditIgnored, False, False, 0)
			
		self.RemoveIgnored = self.app.CreateIconButton(gtk.STOCK_REMOVE, "stock", self.OnRemoveIgnored, _("Remove"))	
		
		self.vbox9.pack_start(self.RemoveIgnored, False, False, 0)
	
		self.hbox24.pack_start(self.vbox9, False, True, 0)
	
		self.IgnoredTab = gtk.Label(_("Ignored"))
		self.IgnoredTab.show()
		
		self.UsersNoteBook.append_page(self.hbox24, self.IgnoredTab)
		
		
	def TabUsersTrusted(self):
		
		self.hbox25 = gtk.HBox(False, 5)
		self.hbox25.show()
		self.hbox25.set_spacing(5)
		self.hbox25.set_border_width(5)
		
		self.TrustedScrollWindow = gtk.ScrolledWindow()
		self.TrustedScrollWindow.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.TrustedScrollWindow.show()
		self.TrustedScrollWindow.set_shadow_type(gtk.SHADOW_NONE)

		self.TrustedTreestore = gtk.TreeStore( str, str )
		self.TrustedTreeview = gtk.TreeView(self.TrustedTreestore)
		self.TrustedTreeview.set_property("rules-hint", True)
		self.TrustedTreestore.set_sort_column_id(0, gtk.SORT_ASCENDING)
		self.TrustedTreeview.show()
		self.TrustedTreeview.set_headers_visible(True)
		column = gtk.TreeViewColumn(_('Users'))
		cell = gtk.CellRendererText()
		cell.set_property('mode', gtk.CELL_RENDERER_MODE_EDITABLE)
		column.pack_start(cell, True)
		column.add_attribute(cell, 'text', 0)
		column.set_sort_column_id(0)
		self.TrustedTreeview.append_column(column)
		column = gtk.TreeViewColumn(_('Comments'))
		cell = gtk.CellRendererText()
		cell.set_property('mode', gtk.CELL_RENDERER_MODE_EDITABLE)
		column.pack_start(cell, True)
		column.add_attribute(cell, 'text', 1)
		column.set_sort_column_id(1)
		self.TrustedTreeview.append_column(column)

		self.TrustedScrollWindow.add(self.TrustedTreeview)
	
		self.hbox25.pack_start(self.TrustedScrollWindow, True, True, 0)
	
		self.trusted_vbox = gtk.VBox(False, 5)
		self.trusted_vbox.show()
		self.trusted_vbox.set_spacing(5)
		self.trusted_vbox.set_border_width(3)
	
		self.AddTrusted = self.app.CreateIconButton(gtk.STOCK_ADD, "stock", self.OnAddTrusted, _("Add"))
		
		self.trusted_vbox.pack_start(self.AddTrusted, False, False, 0)
	
		self.EditTrusted = self.app.CreateIconButton(gtk.STOCK_EDIT, "stock", self.OnEditTrusted, _("Edit"))
		
		self.trusted_vbox.pack_start(self.EditTrusted, False, False, 0)
		
		self.RemoveTrusted = self.app.CreateIconButton(gtk.STOCK_REMOVE, "stock", self.OnRemoveTrusted, _("Remove"))	
		
		self.trusted_vbox.pack_start(self.RemoveTrusted, False, False, 0)
	
		self.hbox25.pack_start(self.trusted_vbox, False, True, 0)
	
		self.TrustedTab = gtk.Label(_("Trusted"))
		self.TrustedTab.show()
		
		self.UsersNoteBook.append_page(self.hbox25, self.TrustedTab)
		
	def TabUsersAlerts(self):
		
		self.alert_hbox = gtk.HBox(False, 5)
		self.alert_hbox.set_border_width(5)
		self.alert_hbox.set_spacing(5)
		self.alert_hbox.show()
		
		self.alertScrollWindow = gtk.ScrolledWindow()
		self.alertScrollWindow.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.alertScrollWindow.show()
		self.alertScrollWindow.set_shadow_type(gtk.SHADOW_NONE)
	
	
		self.AlertsTreestore = gtk.TreeStore( str, str )
		self.AlertsTreeview = gtk.TreeView(self.AlertsTreestore)
		self.AlertsTreeview.set_property("rules-hint", True)
		self.AlertsTreestore.set_sort_column_id(0, gtk.SORT_ASCENDING)
		self.AlertsTreeview.show()
		self.AlertsTreeview.set_headers_visible(True)
		column = gtk.TreeViewColumn(_('Users'))
		cell = gtk.CellRendererText()
		cell.set_property('mode', gtk.CELL_RENDERER_MODE_EDITABLE)
		column.pack_start(cell, True)
		column.add_attribute(cell, 'text', 0)
		column.set_sort_column_id(0)
		self.AlertsTreeview.append_column(column)
		
		self.alertScrollWindow.add(self.AlertsTreeview)
	
		self.alert_hbox.pack_start(self.alertScrollWindow, True, True)
	
		self.vbox14 = gtk.VBox(False, 5)
		self.vbox14.set_border_width(3)
		self.vbox14.set_spacing(5)
		self.vbox14.show()
		
		self.AddAlert = self.app.CreateIconButton(gtk.STOCK_ADD, "stock", self.OnAddAlert, _("Add"))
	
		self.vbox14.pack_start(self.AddAlert, False, False)
		
		self.EditAlert = self.app.CreateIconButton(gtk.STOCK_EDIT, "stock", self.OnEditAlert, _("Edit"))	
		self.vbox14.pack_start(self.EditAlert, False, False)
		
		self.RemoveAlert = self.app.CreateIconButton(gtk.STOCK_REMOVE, "stock", self.OnRemoveAlert, _("Remove"))
		
		self.vbox14.pack_start(self.RemoveAlert, False, False)
	
		self.alert_hbox.pack_start(self.vbox14, False, True)

		self.alertsLabel = gtk.Label(_("Alerts"))
		self.alertsLabel.show()

		self.UsersNoteBook.append_page(self.alert_hbox, self.alertsLabel)
		
	
	def TabUserInfo(self):
		self.UserinfoLabel = gtk.Label(_("My Userinfo"))
		self.UserinfoLabel.show()
		
		self.UserInfoSW = gtk.ScrolledWindow()
		self.UserInfoSW.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.UserInfoSW.show()
		self.UserInfoSW.set_shadow_type(gtk.SHADOW_NONE)
	
		self.viewport5 = gtk.Viewport()
		self.viewport5.show()
		self.viewport5.set_shadow_type(gtk.SHADOW_IN)
	
		self.hpaned1 = gtk.HPaned()
		self.hpaned1.show()
		
		# Left Side
		self.vbox12 = gtk.VBox(False, 5)
		self.vbox12.show()
		self.vbox12.set_spacing(5)
		self.vbox12.set_border_width(5)
	
		self.UserinfoTextScrollWindow = gtk.ScrolledWindow()
		self.UserinfoTextScrollWindow.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.UserinfoTextScrollWindow.show()
		self.UserinfoTextScrollWindow.set_shadow_type(gtk.SHADOW_IN)
		
		self.userinfoBuffer = gtk.TextBuffer()
		self.userinfoTextview = gtk.TextView(self.userinfoBuffer)
		self.userinfoTextview.set_cursor_visible(True)
		self.userinfoTextview.set_editable(True)
		self.userinfoTextview.set_wrap_mode(gtk.WRAP_WORD)
		self.userinfoTextview.show()
		self.userinfoBuffer.connect("changed", self.UserinfoTextChanged)
		
		self.UserinfoTextScrollWindow.add(self.userinfoTextview)
	
		self.vbox12.pack_start(self.UserinfoTextScrollWindow, True, True, 0)
	
		self.hbox26 = gtk.HBox(False, 5)
		self.hbox26.show()
		self.hbox26.set_spacing(5)
		self.removeUserinfoText = self.app.CreateIconButton(gtk.STOCK_CLEAR, "stock", self.OnClearUserinfo, _("Clear"))
		self.hbox26.pack_start(self.removeUserinfoText, False, False, 5)
		
		self.selectUserinfoText = self.app.CreateIconButton(gtk.STOCK_CLEAR, "stock", self.OnImportUserinfo, _("Import Text"))
		
		self.hbox26.pack_end(self.selectUserinfoText, False, False, 5)
	
		self.vbox12.pack_start(self.hbox26, False, True, 5)
	
		self.hpaned1.pack1(self.vbox12, False, True)
		
		# Right Side
		self.vbox11 = gtk.VBox(False, 5)
		self.vbox11.show()
		self.vbox11.set_spacing(5)
		self.vbox11.set_border_width(5)
		
		self.scrolledwindow24 = gtk.ScrolledWindow()
		self.scrolledwindow24.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.scrolledwindow24.show()
		self.scrolledwindow24.set_shadow_type(gtk.SHADOW_IN)
	
		self.viewport6 = gtk.Viewport()
		self.viewport6.show()
		self.viewport6.set_shadow_type(gtk.SHADOW_IN)
	
		self.userinfoImage = gtk.Image()
		self.userinfoImage.show()
		self.viewport6.add(self.userinfoImage)
	
		self.scrolledwindow24.add(self.viewport6)
	
		self.vbox11.pack_start(self.scrolledwindow24, True, True, 5)
		
		self.EntryImage = gtk.Entry()
		self.EntryImage.set_text("")
		self.EntryImage.set_editable(True)
		self.EntryImage.show()
		self.EntryImage.set_visibility(True)

		self.vbox11.pack_start(self.EntryImage, False, True, 5)
		
		self.hboxcl = gtk.HBox(False, 5)
		self.hboxcl.show()
		self.hboxcl.set_spacing(5)
		
		self.removeUserinfoImage = self.app.CreateIconButton(gtk.STOCK_CLEAR, "stock", self.OnClearImage, _("Clear Image"))
		self.hboxcl.pack_start(self.removeUserinfoImage, False, False, 0)
				
		self.selectUserinfoImage = self.app.CreateIconButton(gtk.STOCK_OPEN, "stock", self.OnOpenImage, _("Select Image"))
		self.hboxcl.pack_end(self.selectUserinfoImage, False, False, 0)
	
		self.vbox11.pack_start(self.hboxcl, False, True, 5)
	
		self.hpaned1.pack2(self.vbox11, True, True)
	
		self.viewport5.add(self.hpaned1)
	
		self.UserInfoSW.add(self.viewport5)
	
		
		
	def TabShares(self):
		
		self.SharesSW = gtk.ScrolledWindow()
		self.SharesSW.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.SharesSW.show()
		self.SharesSW.set_shadow_type(gtk.SHADOW_NONE)
	
		self.viewport7 = gtk.Viewport()
		self.viewport7.show()
		self.viewport7.set_shadow_type(gtk.SHADOW_NONE)
	
		self.vbox13 = gtk.VBox(False, 0)
		self.vbox13.show()
		self.vbox13.set_spacing(5)
		self.vbox13.set_border_width(5)
		
		self.frame3 = gtk.Frame()
		self.frame3.show()
		self.frame3.set_border_width(3)
		self.frame3.set_shadow_type(gtk.SHADOW_IN)
		self.vbox13.pack_start(self.frame3, False, False, 0)
		
		self.vbox324 = gtk.VBox(False, 5)
		self.vbox324.set_border_width(5)
		self.vbox324.show()
		
		self.alignment3 = gtk.Alignment(0.5, 0.5, 1, 1)
		self.alignment3.show()
		self.alignment3.add(self.vbox324)
	
		self.frame3.add(self.alignment3)
	
		self.normalSharesLabel = gtk.Label()
		self.normalSharesLabel.set_markup(_("<b>Normal Shares</b>"))
		self.normalSharesLabel.show()
		self.frame3.set_label_widget(self.normalSharesLabel)
	
		

		self.hbox29 = gtk.HBox(False, 5)
		self.hbox29.set_border_width(5)
		self.hbox29.show()
		
		self.vbox324.pack_start(self.hbox29, False, False, 0)
		
		self.EntryNormalShares = gtk.Entry()
		self.EntryNormalShares.set_text("")
		self.EntryNormalShares.set_editable(True)
		self.EntryNormalShares.show()
		self.EntryNormalShares.set_visibility(True)
		
		self.EntryNormalShares.connect("changed", self.NormalSharesDBChanged)
		
		self.hbox29.pack_start(self.EntryNormalShares, True, True, 0)
		
		self.selectNormalDBase = self.app.CreateIconButton(gtk.STOCK_OPEN, "stock", self.OnNormalDBase, _("Select DB"))
		self.hbox29.pack_end(self.selectNormalDBase, False, False, 0)
	

		self.sharesdirshbox = gtk.HBox(False, 5)
		self.sharesdirshbox.show()
		self.sharesdirshbox.set_spacing(5)
		self.sharesdirshbox.set_border_width(5)
	
		self.sharesdirsScrollWindow = gtk.ScrolledWindow()
		self.sharesdirsScrollWindow.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.sharesdirsScrollWindow.show()
		self.sharesdirsScrollWindow.set_shadow_type(gtk.SHADOW_IN)
		self.sharesdirshbox.pack_start(self.sharesdirsScrollWindow, True, True)
		
		self.NormalDirTreestore = gtk.TreeStore( str, str )
		self.NormalDirTreeview = gtk.TreeView(self.NormalDirTreestore)
		self.NormalDirTreeview.show()
		self.NormalDirTreeview.set_headers_visible(True)
		
		self.NormalDirTreeview.set_property("rules-hint", True)
		self.NormalDirTreestore.set_sort_column_id(0, gtk.SORT_ASCENDING)
		column = gtk.TreeViewColumn(_('Directories'))
		cell = gtk.CellRendererText()
		cell.set_property('mode', gtk.CELL_RENDERER_MODE_EDITABLE)
		column.pack_start(cell, True)
		column.add_attribute(cell, 'text', 0)
		column.set_sort_column_id(0)
		self.NormalDirTreeview.append_column(column)
		
		self.sharesdirsScrollWindow.add(self.NormalDirTreeview)
	
		
	
		self.nSharesButtonsVbox = gtk.VBox(False, 5)
		self.nSharesButtonsVbox.show()
		self.nSharesButtonsVbox.set_spacing(5)
		self.nSharesButtonsVbox.set_border_width(0)
		
		self.refreshNormalDir = self.app.CreateIconButton(gtk.STOCK_REFRESH, "stock", self.OnRefreshNormalDirs, _("Refresh list"))
		self.nSharesButtonsVbox.pack_start(self.refreshNormalDir, False, False)
		
		self.rescanNormalDir = self.app.CreateIconButton(gtk.STOCK_EXECUTE, "stock", self.OnRescanNormalDirs, _("Rescan shares"))
		self.nSharesButtonsVbox.pack_start(self.rescanNormalDir, False, False)
		
		self.addNormalDir = self.app.CreateIconButton(gtk.STOCK_ADD, "stock", self.OnAddNormalDirs, _("Add directory"))
		self.nSharesButtonsVbox.pack_start(self.addNormalDir, False, False)
		
		self.removeNormalDir = self.app.CreateIconButton(gtk.STOCK_REMOVE, "stock", self.OnRemoveNormalDirs, _("Remove directory"))
		self.nSharesButtonsVbox.pack_end(self.removeNormalDir, False, False)
	
		self.sharesdirshbox.pack_start(self.nSharesButtonsVbox, False, True)
	
		self.vbox324.pack_start(self.sharesdirshbox, False, False, 0)
		
		# Buddy Shares
		# -----------------------------------
		self.frame4 = gtk.Frame()
		self.frame4.show()
		self.frame4.set_border_width(3)
		self.frame4.set_shadow_type(gtk.SHADOW_IN)
		self.label39 = gtk.Label()
		self.label39.set_markup(_("<b>Buddy-Only Shares</b>"))
		self.label39.show()
		self.frame4.set_label_widget(self.label39)
		
		self.alignment4 = gtk.Alignment(0.5, 0.5, 1, 1)
		self.alignment4.show()
		self.frame4.add(self.alignment4)
		
		self.vbox28 = gtk.VBox(False, 5)
		self.vbox28.show()
		self.vbox28.set_spacing(5)
		self.vbox28.set_border_width(5)
		
		self.alignment4.add(self.vbox28)
		
		self.hbox28 = gtk.HBox(False, 5)
		self.hbox28.show()
		self.hbox28.set_spacing(5)
		self.hbox28.set_border_width(5)
		self.vbox28.pack_start(self.hbox28, True, True, 0)
		
		self.EntryBuddyOnlyShares = gtk.Entry()
		self.EntryBuddyOnlyShares.set_text("")
		self.EntryBuddyOnlyShares.set_editable(True)
		self.EntryBuddyOnlyShares.show()
		self.EntryBuddyOnlyShares.set_visibility(True)
		self.EntryBuddyOnlyShares.connect("changed", self.BuddySharesDBChanged)
		
		self.hbox28.pack_start(self.EntryBuddyOnlyShares, True, True, 0)
		
		self.selectBuddyOnlyDBase = self.app.CreateIconButton(gtk.STOCK_OPEN, "stock", self.OnBuddyDBase, _("Select DB"))
		self.hbox28.pack_end(self.selectBuddyOnlyDBase, False, False, 0)
		
		self.bSharedDirsVbox = gtk.HBox(False, 5)
		self.bSharedDirsVbox.show()
		self.bSharedDirsVbox.set_spacing(5)
		self.bSharedDirsVbox.set_border_width(5)
	
		self.bsharesdirsScrollWindow = gtk.ScrolledWindow()
		self.bsharesdirsScrollWindow.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.bsharesdirsScrollWindow.show()
		self.bsharesdirsScrollWindow.set_shadow_type(gtk.SHADOW_IN)
	
		self.BuddyDirTreestore = gtk.TreeStore( str, str )
		self.BuddyDirTreeview = gtk.TreeView(self.BuddyDirTreestore)
		self.BuddyDirTreeview.show()
		self.BuddyDirTreeview.set_headers_visible(True)
		self.BuddyDirTreeview.set_property("rules-hint", True)
		self.BuddyDirTreestore.set_sort_column_id(0, gtk.SORT_ASCENDING)
		column = gtk.TreeViewColumn(_('Directories'))
		cell = gtk.CellRendererText()
		cell.set_property('mode', gtk.CELL_RENDERER_MODE_EDITABLE)
		column.pack_start(cell, True)
		column.add_attribute(cell, 'text', 0)
		column.set_sort_column_id(0)
		self.BuddyDirTreeview.append_column(column)
		self.bsharesdirsScrollWindow.add(self.BuddyDirTreeview)
	
		self.bSharedDirsVbox.pack_start(self.bsharesdirsScrollWindow, True, True, 0)
	
		self.bSharesButtonsVbox = gtk.VBox(False, 5)
		self.bSharesButtonsVbox.show()
		self.bSharesButtonsVbox.set_spacing(5)
		self.bSharesButtonsVbox.set_border_width(0)
		
		
		self.refreshBuddyDir = self.app.CreateIconButton(gtk.STOCK_REFRESH, "stock", self.OnRefreshBuddyDirs, _("Refresh list"))
		self.bSharesButtonsVbox.pack_start(self.refreshBuddyDir, False, False, 0)
		
		self.rescanBuddyDir = self.app.CreateIconButton(gtk.STOCK_EXECUTE, "stock", self.OnRescanBuddyDirs, _("Rescan shares"))
		self.bSharesButtonsVbox.pack_start(self.rescanBuddyDir, False, False, 0)
		
		self.addBuddyDir = self.app.CreateIconButton(gtk.STOCK_ADD, "stock", self.OnAddBuddyDirs, _("Add directory"))
		self.bSharesButtonsVbox.pack_start(self.addBuddyDir, False, False, 0)
		
		self.removeBuddyDir = self.app.CreateIconButton(gtk.STOCK_REMOVE, "stock", self.OnRemoveBuddyDirs, _("Remove directory"))
		self.bSharesButtonsVbox.pack_end(self.removeBuddyDir, False, False, 0)
	
		self.bSharedDirsVbox.pack_start(self.bSharesButtonsVbox, False, True, 0)
	
		self.vbox28.pack_start(self.bSharedDirsVbox, True, True, 0)
		
		self.vbox13.pack_start(self.frame4, False, True, 0)
		
		
		# -----------------------------------
		self.viewport7.add(self.vbox13)
	
		self.SharesSW.add(self.viewport7)
	
		self.sharesDBLabel = gtk.Label(_("Shares"))
		self.sharesDBLabel.show()
		# -----------------------------------

	def TabMurmurAppearance(self):
		self.labelMurmurAppearance = gtk.Label(_("Appearance"))
		self.labelMurmurAppearance.show()
		vboxAppearance = gtk.VBox(False, spacing=5)
		vboxAppearance.set_border_width(5)
		vboxAppearance.show()
		self.FontsSW = gtk.ScrolledWindow()
		self.FontsSW.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.FontsSW.show()
		self.FontsSW.set_shadow_type(gtk.SHADOW_NONE)
		self.FontsSW.add_with_viewport(vboxAppearance)

		self.hbox172 = gtk.HBox(False, 0)
		self.hbox172.show()
		self.hbox172.set_spacing(0)

		self.chatfontlabel = gtk.Label(_("Chat Font:"))
		self.chatfontlabel.set_alignment(0, 0.5)
		self.chatfontlabel.set_line_wrap(False)
		self.chatfontlabel.show()
		self.hbox172.pack_start(self.chatfontlabel, False, False, 0)

		self.SelectChatFont = gtk.FontButton()
		self.SelectChatFont.show()
		self.hbox172.pack_start(self.SelectChatFont, False, False, 5)

		vboxAppearance.pack_start(self.hbox172, False, True, 0)

		self.hbox182 = gtk.HBox(False, 0)
		self.hbox182.show()
		self.hbox182.set_spacing(5)

		self.label213 = gtk.Label(_("Decimal seperator:"))
		self.label213.set_alignment(0, 0.5)
		self.label213.set_line_wrap(False)
		self.label213.show()
		self.hbox182.pack_start(self.label213, False, False, 0)

		self.DecimalSep_List = gtk.ListStore(gobject.TYPE_STRING)
		self.DecimalSep = gtk.ComboBoxEntry()
		self.DecimalSep.set_size_request(99, -1)
		self.DecimalSep.show()

		self.entry89 = self.DecimalSep.child
		self.entry89.set_text("")
		self.entry89.set_editable(False)
		self.entry89.show()
		self.entry89.set_visibility(True)

		self.DecimalSep.set_model(self.DecimalSep_List)
		self.DecimalSep.set_text_column(0)
		self.hbox182.pack_start(self.DecimalSep, False, False, 0)

		vboxAppearance.pack_start(self.hbox182, False, False, 0)

		self.expander2 = gtk.Expander()
		self.expander2.set_expanded(False)
		self.expander2.show()
		self.expander2.set_spacing(0)

		self.table2 = gtk.Table()
		self.table2.show()
		self.table2.set_row_spacings(3)
		self.table2.set_col_spacings(15)

		self.PickRemote = gtk.Button()
		self.PickRemote.show()

		self.alignment35 = gtk.Alignment(0, 0, 0, 0)
		self.alignment35.show()

		self.hbox124 = gtk.HBox(False, 0)
		self.hbox124.show()
		self.hbox124.set_spacing(2)

		self.image32 = gtk.Image()
		self.image32.set_from_stock(gtk.STOCK_SELECT_COLOR, 4)
		self.image32.show()
		self.hbox124.pack_start(self.image32, False, False, 0)

		self.label197 = gtk.Label(_("Remote text"))
		self.label197.set_line_wrap(False)
		self.label197.show()
		self.hbox124.pack_start(self.label197, False, False, 0)

		self.alignment35.add(self.hbox124)

		self.PickRemote.add(self.alignment35)

		self.table2.attach(self.PickRemote, 0, 1, 0, 1, gtk.FILL, 0, 0, 0)

		self.PickLocal = gtk.Button()
		self.PickLocal.show()

		self.alignment43 = gtk.Alignment(0, 0, 0, 0)
		self.alignment43.show()

		self.hbox132 = gtk.HBox(False, 0)
		self.hbox132.show()
		self.hbox132.set_spacing(2)

		self.image40 = gtk.Image()
		self.image40.set_from_stock(gtk.STOCK_SELECT_COLOR, 4)
		self.image40.show()
		self.hbox132.pack_start(self.image40, False, False, 0)

		self.label205 = gtk.Label(_("Local text"))
		self.label205.set_line_wrap(False)
		self.label205.show()
		self.hbox132.pack_start(self.label205, False, False, 0)

		self.alignment43.add(self.hbox132)

		self.PickLocal.add(self.alignment43)

		self.table2.attach(self.PickLocal, 0, 1, 1, 2, gtk.FILL, 0, 0, 0)

		self.PickMe = gtk.Button()
		self.PickMe.show()

		self.alignment44 = gtk.Alignment(0, 0, 0, 0)
		self.alignment44.show()

		self.hbox133 = gtk.HBox(False, 0)
		self.hbox133.show()
		self.hbox133.set_spacing(2)

		self.image41 = gtk.Image()
		self.image41.set_from_stock(gtk.STOCK_SELECT_COLOR, 4)
		self.image41.show()
		self.hbox133.pack_start(self.image41, False, False, 0)

		self.label206 = gtk.Label(_("/me text"))
		self.label206.set_line_wrap(False)
		self.label206.show()
		self.hbox133.pack_start(self.label206, False, False, 0)

		self.alignment44.add(self.hbox133)

		self.PickMe.add(self.alignment44)

		self.table2.attach(self.PickMe, 0, 1, 2, 3, gtk.FILL, 0, 0, 0)

		self.PickHighlight = gtk.Button()
		self.PickHighlight.show()

		self.alignment45 = gtk.Alignment(0, 0, 0, 0)
		self.alignment45.show()

		self.hbox134 = gtk.HBox(False, 0)
		self.hbox134.show()
		self.hbox134.set_spacing(2)

		self.image42 = gtk.Image()
		self.image42.set_from_stock(gtk.STOCK_SELECT_COLOR, 4)
		self.image42.show()
		self.hbox134.pack_start(self.image42, False, False, 0)

		self.label207 = gtk.Label(_("Highlight text"))
		self.label207.set_line_wrap(False)
		self.label207.show()
		self.hbox134.pack_start(self.label207, False, False, 0)

		self.alignment45.add(self.hbox134)

		self.PickHighlight.add(self.alignment45)

		self.table2.attach(self.PickHighlight, 0, 1, 3, 4, gtk.FILL, 0, 0, 0)

		self.Remote = gtk.Entry()
		self.Remote.set_text("")
		self.Remote.set_editable(False)
		self.Remote.show()
		self.Remote.set_visibility(True)
		self.table2.attach(self.Remote, 1, 2, 0, 1, gtk.EXPAND|gtk.FILL, 0, 0, 0)

		self.Local = gtk.Entry()
		self.Local.set_text("")
		self.Local.set_editable(False)
		self.Local.show()
		self.Local.set_visibility(True)
		self.table2.attach(self.Local, 1, 2, 1, 2, gtk.EXPAND|gtk.FILL, 0, 0, 0)

		self.Me = gtk.Entry()
		self.Me.set_text("")
		self.Me.set_editable(False)
		self.Me.show()
		self.Me.set_visibility(True)
		self.table2.attach(self.Me, 1, 2, 2, 3, gtk.EXPAND|gtk.FILL, 0, 0, 0)

		self.Highlight = gtk.Entry()
		self.Highlight.set_text("")
		self.Highlight.set_editable(False)
		self.Highlight.show()
		self.Highlight.set_visibility(True)
		self.table2.attach(self.Highlight, 1, 2, 3, 4, gtk.EXPAND|gtk.FILL, 0, 0, 0)

		self.DefaultRemote = gtk.Button()
		self.DefaultRemote.show()

		self.alignment36 = gtk.Alignment(0, 0, 0, 0)
		self.alignment36.show()

		self.hbox125 = gtk.HBox(False, 0)
		self.hbox125.show()
		self.hbox125.set_spacing(2)

		self.image33 = gtk.Image()
		self.image33.set_from_stock(gtk.STOCK_CANCEL, 4)
		self.image33.show()
		self.hbox125.pack_start(self.image33, False, False, 0)

		self.label198 = gtk.Label(_("Default"))
		self.label198.set_line_wrap(False)
		self.label198.show()
		self.hbox125.pack_start(self.label198, False, False, 0)

		self.alignment36.add(self.hbox125)

		self.DefaultRemote.add(self.alignment36)

		self.table2.attach(self.DefaultRemote, 2, 3, 0, 1, gtk.FILL, 0, 0, 0)

		self.DefaultLocal = gtk.Button()
		self.DefaultLocal.show()

		self.alignment40 = gtk.Alignment(0, 0, 0, 0)
		self.alignment40.show()

		self.hbox129 = gtk.HBox(False, 0)
		self.hbox129.show()
		self.hbox129.set_spacing(2)

		self.image37 = gtk.Image()
		self.image37.set_from_stock(gtk.STOCK_CANCEL, 4)
		self.image37.show()
		self.hbox129.pack_start(self.image37, False, False, 0)

		self.label202 = gtk.Label(_("Default"))
		self.label202.set_line_wrap(False)
		self.label202.show()
		self.hbox129.pack_start(self.label202, False, False, 0)

		self.alignment40.add(self.hbox129)

		self.DefaultLocal.add(self.alignment40)

		self.table2.attach(self.DefaultLocal, 2, 3, 1, 2, gtk.FILL, 0, 0, 0)

		self.DefaultMe = gtk.Button()
		self.DefaultMe.show()

		self.alignment41 = gtk.Alignment(0, 0, 0, 0)
		self.alignment41.show()

		self.hbox130 = gtk.HBox(False, 0)
		self.hbox130.show()
		self.hbox130.set_spacing(2)

		self.image38 = gtk.Image()
		self.image38.set_from_stock(gtk.STOCK_CANCEL, 4)
		self.image38.show()
		self.hbox130.pack_start(self.image38, False, False, 0)

		self.label203 = gtk.Label(_("Default"))
		self.label203.set_line_wrap(False)
		self.label203.show()
		self.hbox130.pack_start(self.label203, False, False, 0)

		self.alignment41.add(self.hbox130)

		self.DefaultMe.add(self.alignment41)

		self.table2.attach(self.DefaultMe, 2, 3, 2, 3, gtk.FILL, 0, 0, 0)

		self.DefaultHighlight = gtk.Button()
		self.DefaultHighlight.show()

		self.alignment42 = gtk.Alignment(0, 0, 0, 0)
		self.alignment42.show()

		self.hbox131 = gtk.HBox(False, 0)
		self.hbox131.show()
		self.hbox131.set_spacing(2)

		self.image39 = gtk.Image()
		self.image39.set_from_stock(gtk.STOCK_CANCEL, 4)
		self.image39.show()
		self.hbox131.pack_start(self.image39, False, False, 0)

		self.label204 = gtk.Label(_("Default"))
		self.label204.set_line_wrap(False)
		self.label204.show()
		self.hbox131.pack_start(self.label204, False, False, 0)

		self.alignment42.add(self.hbox131)

		self.DefaultHighlight.add(self.alignment42)

		self.table2.attach(self.DefaultHighlight, 2, 3, 3, 4, gtk.FILL, 0, 0, 0)

		self.label208 = gtk.Label("")
		self.label208.set_alignment(0, 0.5)
		self.label208.set_padding(0, 5)
		self.label208.set_line_wrap(False)
		self.label208.set_markup(_("<b>List and search colours</b>"))
		self.label208.show()
		self.table2.attach(self.label208, 0, 3, 4, 5, gtk.FILL, 0, 0, 0)

		self.PickImmediate = gtk.Button()
		self.PickImmediate.show()

		self.alignment46 = gtk.Alignment(0, 0, 0, 0)
		self.alignment46.show()

		self.hbox135 = gtk.HBox(False, 0)
		self.hbox135.show()
		self.hbox135.set_spacing(2)

		self.image43 = gtk.Image()
		self.image43.set_from_stock(gtk.STOCK_SELECT_COLOR, 4)
		self.image43.show()
		self.hbox135.pack_start(self.image43, False, False, 0)

		self.label209 = gtk.Label(_("List Text"))
		self.label209.set_line_wrap(False)
		self.label209.show()
		self.hbox135.pack_start(self.label209, False, False, 0)

		self.alignment46.add(self.hbox135)

		self.PickImmediate.add(self.alignment46)

		self.table2.attach(self.PickImmediate, 0, 1, 5, 6, gtk.FILL, 0, 0, 0)

		self.Immediate = gtk.Entry()
		self.Immediate.set_text("")
		self.Immediate.set_editable(False)
		self.Immediate.show()
		self.Immediate.set_visibility(True)
		self.table2.attach(self.Immediate, 1, 2, 5, 6, gtk.EXPAND|gtk.FILL, 0, 0, 0)

		self.DefaultImmediate = gtk.Button()
		self.DefaultImmediate.show()

		self.alignment48 = gtk.Alignment(0, 0, 0, 0)
		self.alignment48.show()

		self.hbox137 = gtk.HBox(False, 0)
		self.hbox137.show()
		self.hbox137.set_spacing(2)

		self.image45 = gtk.Image()
		self.image45.set_from_stock(gtk.STOCK_CANCEL, 4)
		self.image45.show()
		self.hbox137.pack_start(self.image45, False, False, 0)

		self.label211 = gtk.Label(_("Default"))
		self.label211.set_line_wrap(False)
		self.label211.show()
		self.hbox137.pack_start(self.label211, False, False, 0)

		self.alignment48.add(self.hbox137)

		self.DefaultImmediate.add(self.alignment48)

		self.table2.attach(self.DefaultImmediate, 2, 3, 5, 6, gtk.FILL, 0, 0, 0)

		self.DefaultQueue = gtk.Button()
		self.DefaultQueue.show()

		self.alignment49 = gtk.Alignment(0, 0, 0, 0)
		self.alignment49.show()

		self.hbox138 = gtk.HBox(False, 0)
		self.hbox138.show()
		self.hbox138.set_spacing(2)

		self.image46 = gtk.Image()
		self.image46.set_from_stock(gtk.STOCK_CANCEL, 4)
		self.image46.show()
		self.hbox138.pack_start(self.image46, False, False, 0)

		self.label212 = gtk.Label(_("Default"))
		self.label212.set_line_wrap(False)
		self.label212.show()
		self.hbox138.pack_start(self.label212, False, False, 0)

		self.alignment49.add(self.hbox138)

		self.DefaultQueue.add(self.alignment49)

		self.table2.attach(self.DefaultQueue, 2, 3, 6, 7, gtk.FILL, 0, 0, 0)

		self.Queue = gtk.Entry()
		self.Queue.set_text("")
		self.Queue.set_editable(False)
		self.Queue.show()
		self.Queue.set_visibility(True)
		self.table2.attach(self.Queue, 1, 2, 6, 7, gtk.EXPAND|gtk.FILL, 0, 0, 0)

		self.PickQueue = gtk.Button()
		self.PickQueue.show()

		self.alignment47 = gtk.Alignment(0, 0, 0, 0)
		self.alignment47.show()

		self.hbox136 = gtk.HBox(False, 0)
		self.hbox136.show()
		self.hbox136.set_spacing(2)

		self.image44 = gtk.Image()
		self.image44.set_from_stock(gtk.STOCK_SELECT_COLOR, 4)
		self.image44.show()
		self.hbox136.pack_start(self.image44, False, False, 0)

		self.label210 = gtk.Label(_("With queue"))
		self.label210.set_line_wrap(False)
		self.label210.show()
		self.hbox136.pack_start(self.label210, False, False, 0)

		self.alignment47.add(self.hbox136)

		self.PickQueue.add(self.alignment47)

		self.table2.attach(self.PickQueue, 0, 1, 6, 7, gtk.FILL, 0, 0, 0)

		self.OnlineColor = gtk.Entry()
		self.OnlineColor.set_text("")
		self.OnlineColor.set_editable(False)
		self.OnlineColor.show()
		self.OnlineColor.set_visibility(True)
		self.table2.attach(self.OnlineColor, 1, 2, 8, 9, gtk.EXPAND|gtk.FILL, 0, 0, 0)

		self.OfflineColor = gtk.Entry()
		self.OfflineColor.set_text("")
		self.OfflineColor.set_editable(False)
		self.OfflineColor.show()
		self.OfflineColor.set_visibility(True)
		self.table2.attach(self.OfflineColor, 1, 2, 9, 10, gtk.EXPAND|gtk.FILL, 0, 0, 0)

		self.DefaultOnline = gtk.Button()
		self.DefaultOnline.show()

		self.alignment81 = gtk.Alignment(0, 0, 0, 0)
		self.alignment81.show()

		self.hbox186 = gtk.HBox(False, 0)
		self.hbox186.show()
		self.hbox186.set_spacing(2)

		self.image75 = gtk.Image()
		self.image75.set_from_stock(gtk.STOCK_CANCEL, 4)
		self.image75.show()
		self.hbox186.pack_start(self.image75, False, False, 0)

		self.label311 = gtk.Label(_("Default"))
		self.label311.set_line_wrap(False)
		self.label311.show()
		self.hbox186.pack_start(self.label311, False, False, 0)

		self.alignment81.add(self.hbox186)

		self.DefaultOnline.add(self.alignment81)

		self.table2.attach(self.DefaultOnline, 2, 3, 8, 9, gtk.FILL, 0, 0, 0)

		self.DefaultOffline = gtk.Button()
		self.DefaultOffline.show()

		self.alignment82 = gtk.Alignment(0, 0, 0, 0)
		self.alignment82.show()

		self.hbox187 = gtk.HBox(False, 0)
		self.hbox187.show()
		self.hbox187.set_spacing(2)

		self.image76 = gtk.Image()
		self.image76.set_from_stock(gtk.STOCK_CANCEL, 4)
		self.image76.show()
		self.hbox187.pack_start(self.image76, False, False, 0)

		self.label312 = gtk.Label(_("Default"))
		self.label312.set_line_wrap(False)
		self.label312.show()
		self.hbox187.pack_start(self.label312, False, False, 0)

		self.alignment82.add(self.hbox187)

		self.DefaultOffline.add(self.alignment82)

		self.table2.attach(self.DefaultOffline, 2, 3, 9, 10, gtk.FILL, 0, 0, 0)

		self.UsernameHotspots = gtk.CheckButton()
		self.UsernameHotspots.set_active(False)
		self.UsernameHotspots.set_label(_("Username Colours and Hotspots"))
		self.UsernameHotspots.show()
		self.UsernameHotspots.connect("toggled", self.OnUsernameHotspotsToggled)
		self.table2.attach(self.UsernameHotspots, 0, 3, 7, 8, gtk.FILL, 0, 0, 0)

		self.AwayColor = gtk.Entry()
		self.AwayColor.set_text("")
		self.AwayColor.set_editable(False)
		self.AwayColor.show()
		self.AwayColor.set_visibility(True)
		self.table2.attach(self.AwayColor, 1, 2, 10, 11, gtk.EXPAND|gtk.FILL, 0, 0, 0)

		self.PickAway = gtk.Button()
		self.PickAway.show()

		self.alignment80 = gtk.Alignment(0, 0, 0, 0)
		self.alignment80.show()

		self.hbox185 = gtk.HBox(False, 0)
		self.hbox185.show()
		self.hbox185.set_spacing(2)

		self.image74 = gtk.Image()
		self.image74.set_from_stock(gtk.STOCK_SELECT_COLOR, 4)
		self.image74.show()
		self.hbox185.pack_start(self.image74, False, False, 0)

		self.label310 = gtk.Label(_("Away"))
		self.label310.set_line_wrap(False)
		self.label310.show()
		self.hbox185.pack_start(self.label310, False, False, 0)

		self.alignment80.add(self.hbox185)

		self.PickAway.add(self.alignment80)

		self.table2.attach(self.PickAway, 0, 1, 10, 11, gtk.FILL, 0, 0, 0)

		self.DefaultAway = gtk.Button()
		self.DefaultAway.show()

		self.alignment83 = gtk.Alignment(0, 0, 0, 0)
		self.alignment83.show()

		self.hbox188 = gtk.HBox(False, 0)
		self.hbox188.show()
		self.hbox188.set_spacing(2)

		self.image77 = gtk.Image()
		self.image77.set_from_stock(gtk.STOCK_CANCEL, 4)
		self.image77.show()
		self.hbox188.pack_start(self.image77, False, False, 0)

		self.label313 = gtk.Label(_("Default"))
		self.label313.set_line_wrap(False)
		self.label313.show()
		self.hbox188.pack_start(self.label313, False, False, 0)

		self.alignment83.add(self.hbox188)

		self.DefaultAway.add(self.alignment83)

		self.table2.attach(self.DefaultAway, 2, 3, 10, 11, gtk.FILL, 0, 0, 0)

		self.PickOnline = gtk.Button()
		self.PickOnline.show()

		self.alignment78 = gtk.Alignment(0, 0, 0, 0)
		self.alignment78.show()

		self.hbox183 = gtk.HBox(False, 0)
		self.hbox183.show()
		self.hbox183.set_spacing(2)

		self.image72 = gtk.Image()
		self.image72.set_from_stock(gtk.STOCK_SELECT_COLOR, 4)
		self.image72.show()
		self.hbox183.pack_start(self.image72, False, False, 0)

		self.label308 = gtk.Label(_("Online"))
		self.label308.set_line_wrap(False)
		self.label308.show()
		self.hbox183.pack_start(self.label308, False, False, 0)

		self.alignment78.add(self.hbox183)

		self.PickOnline.add(self.alignment78)

		self.table2.attach(self.PickOnline, 0, 1, 8, 9, gtk.FILL, 0, 0, 0)

		self.PickOffline = gtk.Button()
		self.PickOffline.show()

		self.alignment79 = gtk.Alignment(0, 0, 0, 0)
		self.alignment79.show()

		self.hbox184 = gtk.HBox(False, 0)
		self.hbox184.show()
		self.hbox184.set_spacing(2)

		self.image73 = gtk.Image()
		self.image73.set_from_stock(gtk.STOCK_SELECT_COLOR, 4)
		self.image73.show()
		self.hbox184.pack_start(self.image73, False, False, 0)

		self.label309 = gtk.Label(_("Offline"))
		self.label309.set_line_wrap(False)
		self.label309.show()
		self.hbox184.pack_start(self.label309, False, False, 0)

		self.alignment79.add(self.hbox184)

		self.PickOffline.add(self.alignment79)

		self.table2.attach(self.PickOffline, 0, 1, 9, 10, gtk.FILL, 0, 0, 0)

		self.DefaultBackground = gtk.Button()
		self.DefaultBackground.show()

		self.alignment94 = gtk.Alignment(0, 0, 0, 0)
		self.alignment94.show()

		self.hbox210 = gtk.HBox(False, 0)
		self.hbox210.show()
		self.hbox210.set_spacing(2)

		self.image88 = gtk.Image()
		self.image88.set_from_stock(gtk.STOCK_CANCEL, 4)
		self.image88.show()
		self.hbox210.pack_start(self.image88, False, False, 0)

		self.label360 = gtk.Label(_("Default"))
		self.label360.set_line_wrap(False)
		self.label360.show()
		self.hbox210.pack_start(self.label360, False, False, 0)

		self.alignment94.add(self.hbox210)

		self.DefaultBackground.add(self.alignment94)

		self.table2.attach(self.DefaultBackground, 2, 3, 12, 13, gtk.FILL, 0, 0, 0)

		self.BackgroundColor = gtk.Entry()
		self.BackgroundColor.set_text("")
		self.BackgroundColor.set_editable(False)
		self.BackgroundColor.show()
		self.BackgroundColor.set_visibility(True)
		self.table2.attach(self.BackgroundColor, 1, 2, 12, 13, gtk.EXPAND|gtk.FILL, 0, 0, 0)

		self.PickBackground = gtk.Button()
		self.PickBackground.show()

		self.alignment93 = gtk.Alignment(0, 0, 0, 0)
		self.alignment93.show()

		self.hbox209 = gtk.HBox(False, 0)
		self.hbox209.show()
		self.hbox209.set_spacing(2)

		self.image87 = gtk.Image()
		self.image87.set_from_stock(gtk.STOCK_SELECT_COLOR, 4)
		self.image87.show()
		self.hbox209.pack_start(self.image87, False, False, 0)

		self.label359 = gtk.Label(_("Background"))
		self.label359.set_line_wrap(False)
		self.label359.show()
		self.hbox209.pack_start(self.label359, False, False, 0)

		self.alignment93.add(self.hbox209)

		self.PickBackground.add(self.alignment93)

		self.table2.attach(self.PickBackground, 0, 1, 12, 13, gtk.FILL, 0, 0, 0)

		self.hbox197 = gtk.HBox(False, 0)
		self.hbox197.show()
		self.hbox197.set_spacing(3)

		self.label321 = gtk.Label(_("Username Font Style:"))
		self.label321.set_line_wrap(False)
		self.label321.show()
		self.hbox197.pack_start(self.label321, False, False, 0)

		self.UsernameStyle_List = gtk.ListStore(gobject.TYPE_STRING)
		self.UsernameStyle = gtk.ComboBoxEntry()
		self.UsernameStyle.show()

		self.UsernameStyle.set_model(self.UsernameStyle_List)
		self.UsernameStyle.set_text_column(0)
		self.hbox197.pack_start(self.UsernameStyle, False, True, 0)

		self.table2.attach(self.hbox197, 0, 3, 11, 12, gtk.FILL, gtk.EXPAND|gtk.FILL, 0, 0)

		self.PickInput = gtk.Button()
		self.PickInput.show()

		self.alignment100 = gtk.Alignment(0, 0, 0, 0)
		self.alignment100.show()

		self.hbox220 = gtk.HBox(False, 0)
		self.hbox220.show()
		self.hbox220.set_spacing(2)

		self.image94 = gtk.Image()
		self.image94.set_from_stock(gtk.STOCK_SELECT_COLOR, 4)
		self.image94.show()
		self.hbox220.pack_start(self.image94, False, False, 0)

		self.label374 = gtk.Label(_("Input Text"))
		self.label374.set_line_wrap(False)
		self.label374.show()
		self.hbox220.pack_start(self.label374, False, False, 0)

		self.alignment100.add(self.hbox220)

		self.PickInput.add(self.alignment100)

		self.table2.attach(self.PickInput, 0, 1, 13, 14, gtk.FILL, 0, 0, 0)

		self.InputColor = gtk.Entry()
		self.InputColor.set_text("")
		self.InputColor.set_editable(False)
		self.InputColor.show()
		self.InputColor.set_visibility(True)
		self.table2.attach(self.InputColor, 1, 2, 13, 14, gtk.EXPAND|gtk.FILL, 0, 0, 0)

		self.DefaultInput = gtk.Button()
		self.DefaultInput.show()

		self.alignment101 = gtk.Alignment(0, 0, 0, 0)
		self.alignment101.show()

		self.hbox221 = gtk.HBox(False, 0)
		self.hbox221.show()
		self.hbox221.set_spacing(2)

		self.image95 = gtk.Image()
		self.image95.set_from_stock(gtk.STOCK_CANCEL, 4)
		self.image95.show()
		self.hbox221.pack_start(self.image95, False, False, 0)

		self.label375 = gtk.Label(_("Default"))
		self.label375.set_line_wrap(False)
		self.label375.show()
		self.hbox221.pack_start(self.label375, False, False, 0)

		self.alignment101.add(self.hbox221)

		self.DefaultInput.add(self.alignment101)

		self.table2.attach(self.DefaultInput, 2, 3, 13, 14, gtk.FILL, 0, 0, 0)

		self.expander2.add(self.table2)

		self.label306 = gtk.Label("")
		self.label306.set_line_wrap(False)
		self.label306.set_markup(_("<b>Colours</b>"))
		self.label306.show()
		self.expander2.set_label_widget(self.label306)

		vboxAppearance.pack_start(self.expander2, False, False, 0)

		for item in ["<None>", ",", ".", "<space>"]:
			self.DecimalSep.append_text(item)
		
		for item in ["bold", "italic", "underline", "normal"]:
			self.UsernameStyle.append_text(item)
		self.UsernameStyle.child.set_editable(False)
		
		
		
		self.PickRemote.connect("clicked", self.PickColour, self.Remote)
		self.PickLocal.connect("clicked", self.PickColour, self.Local)
		self.PickMe.connect("clicked", self.PickColour, self.Me)
		self.PickHighlight.connect("clicked", self.PickColour, self.Highlight)
		self.PickImmediate.connect("clicked", self.PickColour, self.Immediate)
		self.PickQueue.connect("clicked", self.PickColour, self.Queue)
		
		self.PickAway.connect("clicked", self.PickColour, self.AwayColor)
		self.PickOnline.connect("clicked", self.PickColour, self.OnlineColor)
		self.PickOffline.connect("clicked", self.PickColour, self.OfflineColor)
		
		self.DefaultAway.connect("clicked", self.DefaultColour, self.AwayColor)
		self.DefaultOnline.connect("clicked", self.DefaultColour, self.OnlineColor)
		self.DefaultOffline.connect("clicked", self.DefaultColour, self.OfflineColor)
		
		self.PickBackground.connect("clicked", self.PickColour, self.BackgroundColor)
		self.DefaultBackground.connect("clicked", self.DefaultColour, self.BackgroundColor)
		
		self.PickInput.connect("clicked", self.PickColour, self.InputColor)
		self.DefaultInput.connect("clicked", self.DefaultColour, self.InputColor)
		
		self.DefaultRemote.connect("clicked", self.DefaultColour, self.Remote)
		self.DefaultLocal.connect("clicked", self.DefaultColour, self.Local)
		self.DefaultMe.connect("clicked", self.DefaultColour, self.Me)
		self.DefaultHighlight.connect("clicked", self.DefaultColour, self.Highlight)
		self.DefaultImmediate.connect("clicked", self.DefaultColour, self.Immediate)
		self.DefaultQueue.connect("clicked", self.DefaultColour, self.Queue)
		self.DefaultQueue.connect("clicked", self.DefaultColour, self.Queue)
		

		
		# To set needcolors flag
		self.SelectChatFont.connect("font-set", self.FontsColorsChanged)
		self.Local.connect("changed", self.FontsColorsChanged)
		self.Remote.connect("changed", self.FontsColorsChanged)
		self.Me.connect("changed", self.FontsColorsChanged)
		self.Highlight.connect("changed", self.FontsColorsChanged)
		self.BackgroundColor.connect("changed", self.FontsColorsChanged)
		self.Immediate.connect("changed", self.FontsColorsChanged)
		self.Queue.connect("changed", self.FontsColorsChanged)
		self.AwayColor.connect("changed", self.FontsColorsChanged)
		self.OnlineColor.connect("changed", self.FontsColorsChanged)
		self.OfflineColor.connect("changed", self.FontsColorsChanged)
		self.UsernameStyle.child.connect("changed", self.FontsColorsChanged)
		self.InputColor.connect("changed", self.FontsColorsChanged)
			
	def FontsColorsChanged(self, widget):
		self.needcolors = 1
		
	def OnEnableTransparentToggled(self, widget):
		sensitive = widget.get_active()
		self.PickTint.set_sensitive(sensitive)
		
		self.TintAlpha.set_sensitive(sensitive)
		self.DefaultTint.set_sensitive(sensitive)
		self.TintColor.set_sensitive(sensitive)
		self.Blue.set_sensitive(sensitive)
		self.Red.set_sensitive(sensitive)
		self.Green.set_sensitive(sensitive)
		self.label346.set_sensitive(sensitive)
		self.label348.set_sensitive(sensitive)
		self.label349.set_sensitive(sensitive)
		self.label347.set_sensitive(sensitive)
		
		
	def OnUsernameHotspotsToggled(self, widget):
		sensitive = widget.get_active()
		self.AwayColor.set_sensitive(sensitive)
		self.OnlineColor.set_sensitive(sensitive)
		self.OfflineColor.set_sensitive(sensitive)
		
		self.DefaultAway.set_sensitive(sensitive)
		self.DefaultOnline.set_sensitive(sensitive)
		self.DefaultOffline.set_sensitive(sensitive)
		
		self.PickAway.set_sensitive(sensitive)
		self.PickOnline.set_sensitive(sensitive)
		self.PickOffline.set_sensitive(sensitive)

		
	def PickColour(self, widget, entry):
		dlg = gtk.ColorSelectionDialog(_("Pick a colour, any colour"))
		colour = entry.get_text()

		if colour != None and colour !='':
			colour = gtk.gdk.color_parse(colour)
			dlg.colorsel.set_current_color(colour)
			
		if dlg.run() == gtk.RESPONSE_OK:
			colour = dlg.colorsel.get_current_color()

			colour = "#%02X%02X%02X" % (colour.red / 256, colour.green / 256, colour.blue / 256)
			entry.set_text(colour)

		dlg.destroy()
		
	#def ColourScale(self, widget):
		#tint = self.TintColor.get_text()
		#if tint != "":
			#if tint[0] == "#" and len(tint) == 7:
				#try:
					#red   = int(tint[1:3], 16)
					#green = int(tint[3:5], 16)
					#blue  = int(tint[5:], 16)
	
					#self.Red.set_value(red)
					#self.Blue.set_value(blue)
					#self.Green.set_value(green)
				#except Exception, e:
					#print e
	#def ScaleColour(self, widget):
		#if self.settingup:
			#return
		#red = int(self.Red.get_value() )
		#green = int(self.Green.get_value())
		#blue = int(self.Blue.get_value())

		#colour = "#%02X%02X%02X" % (red, green, blue)

		#self.TintColor.set_text(colour)
		
	def DefaultColour(self, widget, entry):
		entry.set_text("")
		
		
	def TabMurmurLogin(self):
		self.labelMurmur = gtk.Label(_("Murmur"))
		self.labelMurmur.show()

		self.labelMurmurLogin = gtk.Label(_("Login"))
		self.labelMurmurLogin.show()
		
		hboxMurmur = gtk.VBox(False, spacing=5)
		hboxMurmur.set_border_width(5)
		hboxMurmur.show()
		self.MurmurSW = gtk.ScrolledWindow()
		self.MurmurSW.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.MurmurSW.show()
		self.MurmurSW.set_shadow_type(gtk.SHADOW_NONE)
		self.MurmurSW.add_with_viewport(hboxMurmur)
		
		self.frame1 = gtk.Frame()
		self.frame1.show()
		self.frame1.set_shadow_type(gtk.SHADOW_IN)
		
		self.connectLabel = gtk.Label()
		self.connectLabel.set_markup(_("<b>Connect to Museek Daemon</b>") )

		self.connectLabel.show()
		self.frame1.set_label_widget(self.connectLabel)
		self.InterFaceTable = gtk.Table( homogeneous=False)
		self.InterFaceTable.set_border_width(5)
		self.InterFaceTable.show()
		self.InterFaceTable.set_col_spacings(5)
		self.InterFaceTable.set_row_spacings(5)
		
		count = 0
		# Museek Interface

		label3 = gtk.Label("Museek Interface:")
		label3.set_alignment(0, 0)
		label3.show()
		self.Interface = gtk.Entry()
		if self.murmur_config["connection"]["interface"] != None:
			self.Interface.set_text(self.murmur_config["connection"]["interface"])
		else:
			self.Interface.set_text("")
		self.Interface.show()
		
		self.InterFaceTable.attach(label3, 0, 1, count, count+1, gtk.FILL,  gtk.SHRINK, 0, 0)
		self.InterFaceTable.attach(self.Interface, 1, 2, count, count+1, gtk.FILL, gtk.FILL, 0, 0)
		count += 1
		
		# Password
		label2 = gtk.Label("Password:")
		label2.set_alignment(0, 0)
		label2.show()
		self.Password = gtk.Entry()
		if self.murmur_config["connection"]["passw"] != None:
			self.Password.set_text(self.murmur_config["connection"]["passw"])
		else:
			self.Password.set_text("")
		self.Password.set_visibility(False)
		self.Password.show()
		
		self.InterFaceTable.attach(label2, 0, 1, count, count+1, gtk.FILL,  gtk.SHRINK, 0, 0)
		self.InterFaceTable.attach(self.Password, 1, 2, count, count+1, gtk.FILL, gtk.SHRINK, 0, 0)
		
		self.frame1.add(self.InterFaceTable)
		
		
		# Tooltips
		self.TooltipCheck = gtk.CheckButton("Tooltips")
		
		
		if self.murmur_config["murmur"]["tooltips"]:
			self.TooltipCheck.set_active(True)
		else:
			self.TooltipCheck.set_active(False)
		self.TooltipCheck.connect("toggled", self.tooltip_check)	
		self.TooltipCheck.show()

		self.TrayIcon = gtk.CheckButton(_("Load Tray Icon at start"))
		self.TrayIcon.connect("toggled", self.trayapp_check)
		if self.murmur_config["murmur"]["trayapp"]:
			self.TrayIcon.set_active(True)
		else:
			self.TrayIcon.set_active(False)
		
		self.TrayIcon.show()
	
		label6 = gtk.Label("Museekd's Config File:")
		label6.set_alignment(0, 0)
		
		label6.show()
		
		self.hbox142 = gtk.HBox(False, spacing=5)
		self.hbox142.set_border_width(5)
		self.hbox142.show()
		
		self.Configfile = gtk.Entry()
		if str(self.murmur_config["museekd"]["configfile"]) != "None":
			self.Configfile.set_text(self.murmur_config["museekd"]["configfile"])
		else:
			self.Configfile.set_text("Input the path to the Museekd Config")
		self.Configfile.show()
		
		self.SelectConfig = self.app.CreateIconButton(gtk.STOCK_OPEN, "stock", self.OnSelectConfig, _("Select"))
		
		self.hbox142.pack_start(self.Configfile, True, True)
		self.hbox142.pack_start(self.SelectConfig, False, False)
		
		# Insert horizontal boxes into vertical box
	
		hboxMurmur.pack_start(self.frame1, False, False)
		
		self.vbox2 = gtk.VBox(False, spacing=5)
		self.vbox2.show()
		self.vbox2.pack_start(self.TooltipCheck, False, False)	#checkbox (tooltip)
		self.vbox2.pack_start(self.TrayIcon, False, False)	#checkbox (trayapp)
		self.vbox2.pack_start(label6, False, False)
		self.vbox2.pack_start(self.hbox142, False, False)
		hboxMurmur.pack_start(self.vbox2, False, False)
		
	def OnSelectConfig(self, widget):
		text = self.OpenFile(widget, _("Select Museekd config file"), ["*.xml"])
		
		if text != None:
			self.murmur_config["museekd"]["configfile"] = text
			self.Configfile.set_text(text)
	
	def dialogs(self):
		self.encodings= ["UTF-8", "UTF-7", "UTF-16", "UTF-32", "KOI8-R", "ISO8859-1", "ISO8859-2", "ISO8859-3", "ISO8859-4", "ISO8859-5", "ISO8859-6", "ISO8859-7", "ISO8859-8", "ISO8859-9", "ISO8859-10", "ISO8859-11", "ISO8859-13", "ISO8859-14", "ISO8859-15", "ISO8859-16", "CP1250", "CP1251", "CP1252", "CP1253", "CP1254", "CP1255", "CP1256", "CP1257", "CP1258", "CP874"]
		
		self.MainNotebook = gtk.Notebook()
		self.MainNotebook.set_tab_pos(gtk.POS_TOP)
		self.MainNotebook.set_scrollable(False)
		self.MainNotebook.show()
		
		self.MurmurNotebook = gtk.Notebook()
		self.MurmurNotebook.set_tab_pos(gtk.POS_LEFT)
		self.MurmurNotebook.set_scrollable(True)
		self.MurmurNotebook.show()
		self.TabMurmurLogin()
		self.MurmurNotebook.append_page(self.MurmurSW, self.labelMurmurLogin)

		self.TabMurmurAppearance()
		self.MurmurNotebook.append_page(self.FontsSW, self.labelMurmurAppearance)
		
		self.MainNotebook.append_page(self.MurmurNotebook, self.labelMurmur)

		self.MuseekdNotebook = gtk.Notebook()
		self.MuseekdNotebook.set_tab_pos(gtk.POS_LEFT)
		self.MuseekdNotebook.set_scrollable(True)
		self.MuseekdNotebook.show()
		
		self.labelMuseekd = gtk.Label(_("Museek Daemon"))
		self.labelMuseekd.show()
		
		self.MainNotebook.append_page(self.MuseekdNotebook, self.labelMuseekd)
		self.TabServer()
		self.MuseekdNotebook.append_page(self.ServerSW, self.label1)

		self.TabMuseekClients()
		self.MuseekdNotebook.append_page(self.MuseekClientsSW, self.label3)
		
		self.TabTransfers()
		self.MuseekdNotebook.append_page(self.TransfersScrollWindow, self.TransfersLabel)
		
		self.TabChatRoom()
		self.MuseekdNotebook.append_page(self.hboxchat, self.ChatRooms_Label)
			
		self.TabUsers()
		self.MuseekdNotebook.append_page(self.hboxusers, self.users_Label)
			
		self.TabUserInfo()
		self.MuseekdNotebook.append_page(self.UserInfoSW, self.UserinfoLabel)
		
		self.TabShares()
		self.MuseekdNotebook.append_page(self.SharesSW, self.sharesDBLabel)
	

	def createTreeFor(self, store, treeview, name):
		store.clear()
		if name in ("normaldirs", "buddydirs"):
			if name == "normaldirs":
				dict = self.SharedDirs["normal"]
			elif name == "buddydirs":
				dict = self.SharedDirs["buddy"]
			parents = self.parents[name] = {}
			num = 0
			for key in dict:
				parents[key] =  store.append(None, [key, num])
				num += 1
			self.parents[name] = parents
			
		elif name in self.config:
			dict = self.config[name]
			
			parents = self.parents[name] = {}
			num = 0
			for key, value in dict.items():

				if name in ("interfaces.bind", "autojoin", "alerts"):
					parents[key] =  store.append(None, [key, None])
				else:
					parents[key] =  store.append(None, [key, value])
				num += 1
			self.parents[name] = parents
		else:
			self.config[name] = {}
			
	def refreshConfigDisplay(self):
		self.populate_dialogs()
		
	def MuseekSettingsSensitive(self, sensitive):

		self.ServerSW.set_sensitive( sensitive)
		self.MuseekClientsSW.set_sensitive( sensitive)
		self.TransfersScrollWindow.set_sensitive( sensitive)
		self.hboxchat.set_sensitive( sensitive)
		self.hboxusers.set_sensitive( sensitive)
		self.UserInfoSW.set_sensitive( sensitive)
		self.SharesSW.set_sensitive( sensitive)

	def ClearSettings(self):
		self.MuseekSettingsSensitive(False)
		self.EntryServerHost.set_text("")
		self.ServerPort.set_value(0)
		self.EntryServerPassword.set_text("")
		self.EntryServerUsername.set_text("")
		self.filesystemEncoding.child.set_text("")

		self.networkEncoding.child.set_text("")
		self.LastPort.set_value(0)
		self.FirstPort.set_value(0)
		self.LogMethod.set_active(0)
		# Museek Clients
		self.interfacePassword.set_text("")
		self.interfacesTreestore.clear()

		# Transfers
		self.connectMode.child.set_text("")

		self.privilege_buddies_Check.set_active(False)
		self.have_buddy_shares_Check.set_active(False)
		self.trusting_uploads_Check.set_active(False)
		self.only_buddies_Check.set_active(False)
		self.user_warnings_Check.set_active(False)
		self.uploadSlots.set_value(0)
		self.uploadRate.set_value(0)
		self.downloadSlots.set_value(0)
		self.downloadRate.set_value(0)
		self.EntryDownloadDIr.set_text("")
		self.EntryIncompleteDir.set_text("")
		self.EntryDownloadsDBase.set_text("")
		# Chat rooms
		self.defaultTicker.set_text("")
		self.autojoinTreestore.clear()
		self.encodingsTreestore.clear()
		self.tickersTreestore.clear()

		# Users
		self.BuddiesTreestore.clear()
		self.BannedTreestore.clear()
		self.IgnoredTreestore.clear()

		self.TrustedTreestore.clear()

		# My userinfo
		self.importinguserinfo =1

		s = self.userinfoBuffer.get_start_iter()
		e = self.userinfoBuffer.get_end_iter()
		self.userinfoBuffer.delete(s, e)

		self.userinfoImage.set_from_file("")
		self.EntryImage.set_text("")

		s= self.userinfoBuffer.get_start_iter()
		self.userinfoBuffer.insert(s, "")
		self.importinguserinfo =0
		# Shares DBs


		self.EntryNormalShares.set_text("")
		self.EntryBuddyOnlyShares.set_text("")
		# Alerts
		self.AlertsTreestore.clear()
		self.NormalDirTreestore.clear()
		self.BuddyDirTreestore.clear()
		#self.OnRefreshBuddyDirs(None)

			
	def populate_dialogs(self):
		self.parents= {}
		# Server
		if self.config == {}:
			self.ClearSettings()
			return
		#try:
		if 1:
			self.MuseekSettingsSensitive(True)
			self.app.Muscan.GetConfig()
			
			self.CONFIG_PATH = self.app.Muscan.configfile
			config = self.config
			if "server" in self.config:
				if "host" in self.config["server"]:
					self.EntryServerHost.set_text(self.config["server"]["host"])
				if "port" in self.config["server"]:
					self.ServerPort.set_value(int(self.config["server"]["port"]))
				if "password" in self.config["server"]:
					self.EntryServerPassword.set_text(self.config["server"]["password"])
				if "username" in self.config["server"]:
					self.EntryServerUsername.set_text(self.config["server"]["username"])
			if "encoding" in self.config:
				if "filesystem" in self.config["encoding"]:
					self.filesystemEncoding.child.set_text(self.config["encoding"]["filesystem"])
				if "network" in self.config["encoding"]:
					self.networkEncoding.child.set_text(self.config["encoding"]["network"])
			if "clients.bind" in self.config:
				if "last" in self.config["clients.bind"]:
					self.LastPort.set_value(int(self.config["clients.bind"]["last"]))
				if "first" in self.config["clients.bind"]:
					self.FirstPort.set_value(int(self.config["clients.bind"]["first"]))
			if "logging" in self.config:
				if "output" in self.config["logging"]:
					self.LogMethod.set_active(int(self.config["logging"]["output"]))
			else:
				self.config["logging"] = {"output":0}
			# Museek Clients
			if "password" in self.config["interfaces"]:
				self.interfacePassword.set_text(str(self.config["interfaces"]["password"]))
			self.createTreeFor( self.interfacesTreestore, self.interfacesTreeview, "interfaces.bind")	

			# Transfers
			if "connectmode" in self.config["clients"]:
				self.connectMode.child.set_text(str(self.config["clients"]["connectmode"]))

			if str(self.config["transfers"]["privilege_buddies"]) == "true":
				self.privilege_buddies_Check.set_active(True)
			else:
				self.privilege_buddies_Check.set_active(False)
			if "have_buddy_shares" in self.config["transfers"]:
				if str(self.config["transfers"]["have_buddy_shares"]) == "true":
					self.have_buddy_shares_Check.set_active(True)
				else:
					self.have_buddy_shares_Check.set_active(False)
			else:
				self.have_buddy_shares_Check.set_active(False)
			if "trusting_uploads" in self.config["transfers"]:
				if str(self.config["transfers"]["trusting_uploads"]) == "true":
					self.trusting_uploads_Check.set_active(True)
				else:
					self.trusting_uploads_Check.set_active(False)
			else:
				self.trusting_uploads_Check.set_active(False)
			if str(self.config["transfers"]["only_buddies"]) == "true":
				self.only_buddies_Check.set_active(True)
			else:
				self.only_buddies_Check.set_active(False)
			if "user_warnings" in self.config["transfers"]:
				if str(self.config["transfers"]["user_warnings"]) == "true":
					self.user_warnings_Check.set_active(True)
				else:
					self.user_warnings_Check.set_active(False)
			else:
				self.user_warnings_Check.set_active(False)
			if "upload_slots" in self.config["transfers"]:
				self.uploadSlots.set_value(int(self.config["transfers"]["upload_slots"]))
			if "upload_rate" in self.config["transfers"]:
				self.uploadRate.set_value(int(self.config["transfers"]["upload_rate"]))
			if "download_slots" in config["transfers"].keys():
				self.downloadSlots.set_value(int(self.config["transfers"]["download_slots"]))
			if "download_rate" in config["transfers"].keys():
				self.downloadRate.set_value(int(self.config["transfers"]["download_rate"]))
			if "download-dir" in config["transfers"].keys():
				self.EntryDownloadDIr.set_text(str(self.config["transfers"]["download-dir"]))
			if "incomplete-dir" in self.config["transfers"]:
				self.EntryIncompleteDir.set_text(str(self.config["transfers"]["incomplete-dir"]))
			if "downloads" in self.config["transfers"]:
				self.EntryDownloadsDBase.set_text(str(self.config["transfers"]["downloads"]))
			# Chat rooms
			if "default-ticker" in self.config:
				if "ticker" in self.config["default-ticker"]:
					self.defaultTicker.set_text(str(self.config["default-ticker"]["ticker"]))
			self.createTreeFor( self.autojoinTreestore, self.autojoinTreeview, "autojoin")
			self.createTreeFor( self.encodingsTreestore, self.encodingsTreeview, "encoding.rooms")
			self.createTreeFor( self.tickersTreestore, self.tickersTreeview, "tickers")

			# Users
			self.createTreeFor( self.BuddiesTreestore, self.BuddiesTreeview, "buddies")
			self.createTreeFor( self.BannedTreestore, self.BannedTreeview, "banned")
			self.createTreeFor( self.IgnoredTreestore, self.IgnoredTreeview, "ignored")
			
			self.createTreeFor( self.TrustedTreestore, self.TrustedTreeview, "trusted")
			
			# My userinfo
			self.importinguserinfo =1
			
			s = self.userinfoBuffer.get_start_iter()
			e = self.userinfoBuffer.get_end_iter()
			self.userinfoBuffer.delete(s, e)

			if "image" in config["userinfo"].keys():
				self.userinfoImage.set_from_file(str(self.config["userinfo"]["image"]))
				self.EntryImage.set_text(str(self.config["userinfo"]["image"]))

			s= self.userinfoBuffer.get_start_iter()
			if "text" in config["userinfo"].keys():
				self.userinfoBuffer.insert(s, str(self.config["userinfo"]["text"]))
			self.importinguserinfo =0
			# Shares DBs
			
	
			if "database" in self.config["shares"]:
				self.EntryNormalShares.set_text(str(self.config["shares"]["database"]))
			if "buddy.shares" not in self.config:
				self.config["buddy.shares"] = {"database": config_path+".buddyshares"}
			if "database" in self.config["buddy.shares"]:
				self.EntryBuddyOnlyShares.set_text(str(self.config["buddy.shares"]["database"]))
			# Alerts
			self.createTreeFor( self.AlertsTreestore, self.AlertsTreeview, "alerts")
			self.OnRefreshNormalDirs(None)
			self.OnRefreshBuddyDirs(None)

			ui = self.murmur_config["ui"]
			#private = config["privatechat"]
			#transfers = self.murmur_config["transfers"]
			
			if ui["chatfont"] is not None:
				self.SelectChatFont.set_font_name(ui["chatfont"])
				
			if ui["chatlocal"] is not None:
				self.Local.set_text(ui["chatlocal"])
			if ui["chatremote"] is not None:
				self.Remote.set_text(ui["chatremote"])
			if ui["chatme"] is not None:
				self.Me.set_text(ui["chatme"])
			if ui["chathilite"] is not None:
				self.Highlight.set_text(ui["chathilite"])
				
			if ui["useraway"] is not None:
				self.AwayColor.set_text(ui["useraway"])
			if ui["useronline"] is not None:
				self.OnlineColor.set_text(ui["useronline"])
			if ui["useroffline"] is not None:
				self.OfflineColor.set_text(ui["useroffline"])
			if ui["usernamehotspots"] is not None:
				self.UsernameHotspots.set_active(ui["usernamehotspots"])
			if ui["textbg"] is not None:
				self.BackgroundColor.set_text(ui["textbg"])
			if ui["inputcolor"] is not None:
				self.InputColor.set_text(ui["inputcolor"])
			self.OnUsernameHotspotsToggled(self.UsernameHotspots)
			if ui["search"] is not None:
				self.Immediate.set_text(ui["search"])
			if ui["searchq"] is not None:
				self.Queue.set_text(ui["searchq"])
			if ui["decimalsep"] is not None:
				self.DecimalSep.child.set_text(ui["decimalsep"])
			#if ui["exitdialog"] is not None:
				#self.ExitDialog.set_active(ui["exitdialog"])
			
			#if private["store"] is not None:
				#self.ReopenPrivateChats.set_active(private["store"])

			if ui["usernamestyle"] is not None:
				self.UsernameStyle.child.set_text(ui["usernamestyle"])
			#if transfers["enabletransferbuttons"] is not None:
				#self.ShowTransferButtons.set_active(transfers["enabletransferbuttons"])

			self.needcolors = 0
		#except Exception, e:
			#self.Bug(_("Populating the Dialogs with config data failed:"), str(e))
			#print e

	def read_config(self):
		self.SharedDirs = {"normal":[], "buddy":[] }
		self.config = {}
		for section in self.app.Networking.config.keys():
			self.config[section] = {}
			for key, value in self.app.Networking.config[section].items():
				self.config[section][key] = value
		if self.murmur_config["connection"]["interface"] != None:
			self.Interface.set_text(self.murmur_config["connection"]["interface"])
		else:
			self.Interface.set_text("Input a Museek Interface")
		if self.murmur_config["connection"]["passw"] != None:
			self.Password.set_text(self.murmur_config["connection"]["passw"])
		else:
			self.Password.set_text("Input a password")
		

		if self.murmur_config["murmur"]["tooltips"]:
			self.TooltipCheck.set_active(True)
		else:
			self.TooltipCheck.set_active(False)
		self.populate_dialogs()
		
		
	def trayapp_check(self, string):
		self.murmur_config["murmur"]["trayapp"] = self.TrayIcon.get_active()


	def tooltip_check(self, string):
		self.murmur_config["murmur"]["tooltips"] = self.TooltipCheck.get_active()
	
			
			
	def quit(self, w=None, event=None):
		self.hide()
		
	def click(self, button):
		self.save(button)
		self.quit()
		
	def save(self, button):
		self.murmur_config["connection"]["interface"] = self.Interface.get_text()
		self.murmur_config["connection"]["passw"] = self.Password.get_text()
		self.murmur_config["murmur"]["tooltips"] = self.TooltipCheck.get_active()
		self.murmur_config["murmur"]["trayapp"] = self.TrayIcon.get_active()
		settings = {
			"ui": {
				
				"chatfont": self.SelectChatFont.get_font_name(),
				"chatlocal": self.Local.get_text(),
				"chatremote": self.Remote.get_text(),
				"chatme": self.Me.get_text(),
				"chathilite": self.Highlight.get_text(),
				"textbg": self.BackgroundColor.get_text(),
				"inputcolor": self.InputColor.get_text(),
				"search": self.Immediate.get_text(),
				"searchq": self.Queue.get_text(),
				"decimalsep": self.DecimalSep.child.get_text(),
				"useraway": self.AwayColor.get_text(),
				"useronline": self.OnlineColor.get_text(),
				"useroffline": self.OfflineColor.get_text(),
				"usernamehotspots": int(self.UsernameHotspots.get_active()),
				"usernamestyle": self.UsernameStyle.child.get_text(),
				}
			}
		for domain in settings.keys():
			for key, value in settings[domain].items():
				self.murmur_config[domain][key] = value
		self.WriteConfig()
		self.app.ChatRooms.UpdateColours()
		self.app.PrivateChats.UpdateColours()
			
	def WriteConfig(self):
		oldconfig = self.app.Networking.config
		for section in oldconfig:
			for key, value in oldconfig[section].items():
				if key not in self.config[section].keys():
					self.app.Networking.Send(messages.ConfigRemove(section, key))
					#print "Remove", section, key, value
					
		for section in self.config:
			for key, value in self.config[section].items():
				if key not in oldconfig[section].keys():
					self.app.Networking.Send(messages.ConfigSet(section, key, str(value)))
					#print "Added", section, key, value
				else:
					if str(self.config[section][key]) != str(oldconfig[section][key]):
						#print "new", self.config[section][key] 
						#print "old", oldconfig[section][key]
						self.app.Networking.Send(messages.ConfigSet(section, key, str(value)))
						#print "Changed", section, key, value
		self.app.config_manager.update_config()

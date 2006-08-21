import commands, os, sys, getopt #, subprocess
import locale, gettext
from xml.dom import minidom
import signal
import gtk, gobject
import time, stat, string, pwd, shutil
from time import sleep
from utils import _
from museek import messages
class Settings( gtk.Dialog):
	def __init__(self, xapp, message="", modal= True):
		gtk.Dialog.__init__(self)
		self.app=xapp
		self.murmur_config = self.app.murmur_config
		self.set_transient_for(self.app.MurmurWindow)
		self.set_default_size(600, 400)
		self.connect("destroy", self.quit)
		self.connect("delete_event", self.quit)
		self.parents= {}
		self.SharedDirs = {"normal":[], "buddy":[] }
		if modal:
			self.set_modal(True)
		
		self.vbox.set_spacing(5)
		hbox = gtk.HBox(True, spacing=5)
		
		
		self.frame1 = gtk.Frame()
		self.frame1.show()
		self.frame1.set_shadow_type(gtk.SHADOW_IN)
		
		self.encodings__Label = gtk.Label()
		self.encodings__Label.set_markup(_("<b>Connect to Museek Daemon</b>") )
		self.encodings__Label.set_padding(0, 0)
		
		self.encodings__Label.show()
		self.frame1.set_label_widget(self.encodings__Label)
		
		# Museek Interface
		sbox3 = gtk.HBox(True, spacing=5)
		sbox3.show()
		label3 = gtk.Label("Museek Interface:")
		sbox3.pack_start(label3, True, False)
		label3.show()
		self.Interface = gtk.Entry()
		if self.murmur_config["connection"]["interface"] != None:
			self.Interface.set_text(self.murmur_config["connection"]["interface"])
		else:
			self.Interface.set_text("Input a Museek Interface")
		sbox3.pack_start(self.Interface, True, False)
		self.Interface.show()
		
		
		# Password
		sbox2 = gtk.HBox(True, spacing=5)
		sbox2.show()
		label2 = gtk.Label("Password:")
		sbox2.pack_start(label2, True, False)
		label2.show()
		self.Password = gtk.Entry()
		if self.murmur_config["connection"]["passw"] != None:
			self.Password.set_text(self.murmur_config["connection"]["passw"])
		else:
			self.Password.set_text("Input a password")
		sbox2.pack_start(self.Password, True, False)
		self.Password.show()
		
		
		
		self.vbox1 = gtk.VBox()
		self.vbox1.pack_start(sbox3) #entry3  (interface)
		self.vbox1.pack_start(sbox2) #entry2  (password)
		self.vbox1.show()
		self.frame1.add(self.vbox1)
		
		
		# Tooltips
		sbox4 = gtk.HBox(True, spacing=5)
		sbox4.show()
		label1 = gtk.Label("Tooltips:")
		sbox4.pack_start(label1, False, False)
		
		label1.show()
		self.TooltipCheck = gtk.CheckButton(label=None)
		self.TooltipCheck.connect("toggled", self.tooltip_check, "check button 1")
		
		if self.murmur_config["murmur"]["tooltips"] == "yes":
			self.TooltipCheck.set_active(True)
		else:
			self.TooltipCheck.set_active(False)
			
		self.TooltipCheck.show()
		sbox4.pack_start(self.TooltipCheck, False, False)
	
		# Tooltips
		sbox5 = gtk.HBox(True, spacing=5)
		sbox5.show()
		label1 = gtk.Label("Load Tray App at start:")
		sbox5.pack_start(label1, False, False)
		
		label1.show()
		self.check_button2 = gtk.CheckButton(label=None)
		
		if self.murmur_config["murmur"]["trayapp"] == "yes":
			self.check_button2.set_active(True)
		else:
			self.check_button2.set_active(False)
		self.check_button2.connect("toggled", self.trayapp_check, "check button 1")	
		self.check_button2.show()
		sbox5.pack_start(self.check_button2, False, False)
		
		# Museek Interface
		sbox6 = gtk.HBox(True, spacing=5)
		sbox6.show()
		label6 = gtk.Label("Museekd's Config File:")
		sbox6.pack_start(label6, True, False)
		label6.show()
		self.Configfile = gtk.Entry()
		if str(self.murmur_config["museekd"]["configfile"]) != "None":
			self.Configfile.set_text(self.murmur_config["museekd"]["configfile"])
		else:
			self.Configfile.set_text("Input the path to the Museekd Config")
		self.Configfile.show()
		sbox6.pack_start(self.Configfile, True, True)
		
		
		
		
		
		
		# Insert horizontal boxes into vertical box
		#self.vbox.pack_start(self.frame1, False, False)
		hbox.pack_start(self.frame1, False, False)
		hbox.show()
		self.vbox2 = gtk.VBox()
		self.vbox2.show()
		self.vbox2.pack_start(sbox4, False, False)	#checkbox (tooltip)
		self.vbox2.pack_start(sbox5, False, False)	#checkbox (trayapp)
		self.vbox2.pack_start(sbox6, True, True)
		hbox.pack_start(self.vbox2, True, True)
		self.vbox.pack_start(hbox)
		
		# DIALOGS
		self.dialogs()
		self.vbox.pack_start(self.notebook1)
		
		button = gtk.Button("Save")
		button.connect("clicked", self.click)
		button.set_flags(gtk.CAN_DEFAULT)
		self.action_area.pack_start(button)
		button.show()
		button.grab_default()
		button = gtk.Button("Cancel")
		button.connect("clicked", self.quit)
		button.set_flags(gtk.CAN_DEFAULT)
		self.action_area.pack_start(button)
		button.show()
		self.ret = None
		
	def ServerHostChanged(self, widget):
		if self.config == {}: return
		if "server" in self.config:
			#self.app.ProcessMessages.Send(messages.ConfigSet("server", "host", self.EntryServerHost.get_text()))
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
			
	def EncodingDefaultChanged(self, widget):
		if self.config == {}: return
		if "encoding" in self.config:
			self.config["encoding"]["default"] = self.defaultEncoding.child.get_text()
			
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
		#print self.config.items()
		if self.config != {}:
			self.config["transfers"]["upload_slots"] = str(int( self.uploadSlots.get_value() ))
		
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
			#	

			
	def OnAddTrusted(self, widget):
		self.AddItemToList("trusted", _("Trusted User"), _("Comment"))
	
	def OnAddBuddy(self, widget):
		self.AddItemToList("buddies", _("Buddied User"), _("Comment"))
	
	def OnAddBanned(self, widget):
		self.AddItemToList("banned", _("Banned User"), _("Comment"))

	def OnAddInterface(self, widget):
		self.AddItemToList("interfaces.bind", _("Socket / Port"), None)
	
	def OnAddIgnored(self, widget):
		self.AddItemToList("ignored", _("Ignored User"), _("Comment"))
	
	def OnAddAlert(self, widget):
		self.AddItemToList("alerts", _("User Status Alert"), None)
	
	def OnAddTicker(self, widget):
		self.AddItemToList("tickers", _("Ticker's Room"), _("Ticker"))
		
	def OnAddEncoding(self, widget):
		self.AddItemToList("encoding.rooms", _("Room"), _("Encoding"))
		
	def OnAddAutojoin(self, widget):
		self.AddItemToList("autojoin", _("AutoJoin Room"), None)

	
	def TreeViewSelection3(self, model, path, iter):
		key = model.get_value(iter, 0)
		value = model.get_value(iter, 1)
		num = model.get_value(iter, 2)
		self.selected_items = [key, value, num]
		
	def TreeViewSelection2(self, model, path, iter):
		key = model.get_value(iter, 0)
		num = model.get_value(iter, 1)
		self.selected_items = [key, num]
		
	def EditItemToList(self, node, message, message2, key, value, num):
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

			if self.key != key:
				# Remove old key
				del self.config[node][self.key]
			self.populate_dialogs()
			#self.populate_store()
	def OnEditEncoding(self, widget):
		# 3 column treeview
		treeview = self.encodingsTreeview
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection3)
		if self.selected_items != []:
			key, value, num = self.selected_items
			self.EditItemToList("encoding.rooms", _("Room"), _("Encoding"), key, value, num)
		

	def OnEditAlert(self, widget):
		# 2 column treeview
		treeview = self.AlertsTreeview
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection2)
		if self.selected_items != []:
			key, num = self.selected_items
	 		
			self.EditItemToList("alerts", _("User Status Alert"), None, key, None, num)
			
	def OnEditInterface(self, widget):
		# 2 column treeview
		treeview = self.interfacesTreeview
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection2)
		if self.selected_items != []:
			key, num = self.selected_items
	 		
			self.EditItemToList("interfaces.bind", _("Socket / Port"), None, key, None, num)
			
	def OnEditAutojoin(self, widget):
		# 2 column treeview
		treeview = self.autojoinTreeview
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection2)
		if self.selected_items != []:
			key, num = self.selected_items
	 		
			self.EditItemToList("autojoin", _("AutoJoin Room"), None, key, None, num)
						
	def OnEditTicker(self, widget):
		# 3 column treeview
		treeview = self.tickersTreeview
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection3)
		if self.selected_items != []:
			key, value, num = self.selected_items
			self.EditItemToList("tickers", _("Ticker's Room"), _("Ticker"), key, value, num)
			
	def OnEditTrusted(self, widget):
		# 3 column treeview
		treeview = self.TrustedTreeview
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection3)
		if self.selected_items != []:
			key, value, num = self.selected_items
			self.EditItemToList("trusted", _("Trusted User"), _("Comment"), key, value, num)
			
	def OnEditBuddy(self, widget):
		# 3 column treeview
		treeview = self.BuddiesTreeview
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection3)
		if self.selected_items != []:
			key, value, num = self.selected_items
			self.EditItemToList("buddies", _("Buddied User"), _("Comment"), key, value, num)
	def OnEditBanned(self, widget):
		# 3 column treeview
		treeview = self.encodingsTreeview
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection3)
		if self.selected_items != []:
			key, value, num = self.selected_items
			self.EditItemToList("banned", _("Banned User"), _("Comment"), key, value, num)
	
	

				
	def OnEditIgnored(self, widget):
		# 3 column treeview
		treeview = self.encodingsTreeview
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection3)
		if self.selected_items != []:
			key, value, num = self.selected_items
			self.EditItemToList("ignored", _("Ignored User"), _("Comment"), key, value, num)

	def RemoveItemFromList(self, node, key, value, num):
		if node == None or key == None:
			return
		if key == "" or key.isspace():
			return
		
		if node in self.config:
			del self.config[node][key]
			self.populate_dialogs()
			#self.populate_store()
			
	def OnRemoveAlert(self, widget):
		# 2 column treeview
		treeview = self.AlertsTreeview
		node = "alerts"
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection2)
		if self.selected_items != []:
			key, num = self.selected_items
	 		
			self.RemoveItemFromList(node, key, None, num)
		
	def OnRemoveTicker(self, widget):
		# 3 column treeview
		treeview = self.tickersTreeview
		node="tickers"
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection3)
		if self.selected_items != []:
			key, value, num = self.selected_items
			self.RemoveItemFromList(node, key, value, num)
		
	def OnRemoveTrusted(self, widget):
		# 3 column treeview
		treeview = self.TrustedTreeview
		node = "trusted"
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection3)
		if self.selected_items != []:
			key, value, num = self.selected_items
			self.RemoveItemFromList(node, key, value, num)
		
	def OnRemoveBuddy(self, widget):
		# 3 column treeview
		treeview = self.BuddiesTreeview
		node="buddies"
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection3)
		if self.selected_items != []:
			key, value, num = self.selected_items
			self.RemoveItemFromList(node, key, None, num)
		
	def OnRemoveBanned(self, widget):
		# 3 column treeview
		treeview = self.BannedTreeview
		node="banned"
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection3)
		if self.selected_items != []:
			key, value, num = self.selected_items
			
			self.RemoveItemFromList(node, key, None, num)
		
	def OnRemoveIgnored(self, widget):
		treeview = self.IgnoredTreeview
		node="ignored"
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection3)
		if self.selected_items != []:
			key, value, num = self.selected_items
			
			self.RemoveItemFromList(node, key, value, num)
		
	def OnRemoveInterface(self, widget):
		# 2 column treeview
		treeview = self.interfacesTreeview
		node="interface"
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection2)
		if self.selected_items != []:
			key, num = self.selected_items
	 		
			self.RemoveItemFromList(node, key, None, num)

	def OnRemoveEncoding(self, widget):
		# 3 column treeview
		treeview = self.encodingsTreeview
		node="encoding.rooms"
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection3)
		if self.selected_items != []:
			key, value, num = self.selected_items
			
			self.RemoveItemFromList(node, key, value, num)

	def OnRemoveAutojoin(self, widget):
		# 2 column treeview
		node ="autojoin"
		treeview = self.autojoinTreeview
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection2)
		if self.selected_items != []:
			key, num = self.selected_items

			self.RemoveItemFromList(node, key, None, num)

		


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
				shutil.copy(newimage, self.config["userinfo"]["image"] )

			except Exception, e:
				self.Bug("Creating image", str(e))
			self.refreshConfigDisplay()
	def OpenImage(self, widget, title, filters):
		dialog = gtk.FileChooserDialog(title=title, parent=None, action=gtk.FILE_CHOOSER_ACTION_OPEN, buttons=(gtk.STOCK_OK, gtk.RESPONSE_ACCEPT, gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT))
		dialog.set_select_multiple(False)
		dialog.set_current_folder_uri("file://"+pwd.getpwuid(os.getuid())[5])
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
			#self.muscan_execute(["muscan", "-c", self.CONFIG_PATH, "-v", "-s", directory])
			self.muscan_execute("muscan -c " + self.CONFIG_PATH + " -v -s "+ directory)
			self.OnRefreshNormalDirs(None)
	def OnAddBuddyDirs(self, widget):
		directory = self.OpenDirectory(widget, _("Add a buddy-only Shared Directory"))
		if directory != None:
			#self.muscan_execute(["muscan", "-c", self.CONFIG_PATH, "-v", "-b", "-s", directory])
			self.muscan_execute("muscan -c " + self.CONFIG_PATH + " -v -b -s "+ directory)
			self.OnRefreshBuddyDirs(None)
			
	def OnRemoveBuddyDirs(self, widget):
		treeview = self.BuddyDirTreeview
		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection2)
		if self.selected_items != []:
			key, num = self.selected_items
			if key != "" and key is not None:
				#self.muscan_execute(["muscan", "-c", self.CONFIG_PATH, "-b", "-v",  "-u", key])
				self.muscan_execute("muscan -c " + self.CONFIG_PATH + " -b -v -u "+ key)
				self.OnRefreshBuddyDirs(None)
			
	def OnRemoveNormalDirs(self, widget):
		treeview = self.NormalDirTreeview

		self.selected_items = []
		treeview.get_selection().selected_foreach(self.TreeViewSelection2)
		if self.selected_items != []:
			key, num = self.selected_items
			if key != "" and key is not None:
				#self.muscan_execute(["muscan", "-c", self.CONFIG_PATH, "-v",  "-u", key])
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
		
		#self.muscan_execute(["muscan", "-c", self.CONFIG_PATH,  "-b", "-v", "-r"])
		self.muscan_execute("muscan -c", self.CONFIG_PATH + " -b -v -r")
		self.Statusbar.pop(self.status_context_id)
		self.Statusbar.push(self.status_context_id, "Rescanned Buddy shares")
		
	def OnRescanNormalDirs(self, widget):
		#self.muscan_execute("muscan ", "-c", self.CONFIG_PATH,"-v", "-r"])
		self.muscan_execute("muscan -c "+ self.CONFIG_PATH +" -v  -r")
		self.Statusbar.pop(self.status_context_id)
		self.Statusbar.push(self.status_context_id, "Rescanned Normal shares")
		
	def OnRefreshBuddyDirs(self, widget):
		p = "/usr/bin/muscan"
		if os.path.exists(p):
			output = commands.getoutput("muscan -c "+ self.CONFIG_PATH + " -b  -l")
			stdout_text = output.split('\n')
			#z = subprocess.Popen( ["muscan", "-c", self.CONFIG_PATH, "-b", "-l"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
			#stdout_text, stderr_text = z.communicate()
			#z.wait()
			#print stdout_text
			#stdout_text = stdout_text.split('\n')
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
		
			#z = subprocess.Popen( ["muscan", "-c", self.CONFIG_PATH, "-l"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
			#stdout_text, stderr_text = z.communicate()
			#z.wait()
			#stdout_text = stdout_text.split('\n')
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
		dialog.set_current_folder_uri("file://"+pwd.getpwuid(os.getuid())[5])

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
		dialog.set_current_folder_uri("file://"+pwd.getpwuid(os.getuid())[5]+"/.museekd")
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
		
		
	def dialogs(self):
		self.encodings= ["UTF-8", "UTF-7", "UTF-16", "UTF-32", "KOI8-R", "ISO8859-1", "ISO8859-2", "ISO8859-3", "ISO8859-4", "ISO8859-5", "ISO8859-6", "ISO8859-7", "ISO8859-8", "ISO8859-9", "ISO8859-10", "ISO8859-11", "ISO8859-13", "ISO8859-14", "ISO8859-15", "ISO8859-16", "CP1250", "CP1251", "CP1252", "CP1253", "CP1254", "CP1255", "CP1256", "CP1257", "CP1258", "CP874"]
		
		self.notebook1 = gtk.Notebook()
		self.notebook1.set_tab_pos(gtk.POS_TOP)
		self.notebook1.set_scrollable(False)
		self.notebook1.show()
	
		self.scrolledwindow1 = gtk.ScrolledWindow()
		self.scrolledwindow1.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.scrolledwindow1.show()
		self.scrolledwindow1.set_shadow_type(gtk.SHADOW_NONE)
	
		self.viewport2 = gtk.Viewport()
		self.viewport2.show()
		self.viewport2.set_shadow_type(gtk.SHADOW_IN)
	
		self.vbox2 = gtk.VBox(False, 5)
		self.vbox2.show()
		self.vbox2.set_spacing(5)
		self.vbox2.set_border_width(3)
	
		self.hbox2 = gtk.HBox(False, 5)
		self.hbox2.show()
		self.hbox2.set_spacing(5)
	
		self.Server_Label = gtk.Label(_("Server Host:"))
		self.Server_Label.set_padding(0, 0)
		self.Server_Label.show()
		self.hbox2.pack_start(self.Server_Label, False, False, 0)
	
		self.EntryServerHost = gtk.Entry()
		self.EntryServerHost.set_text("")
		self.EntryServerHost.set_editable(True)
		self.EntryServerHost.show()
		self.EntryServerHost.set_visibility(True)
		self.EntryServerHost.connect("changed", self.ServerHostChanged)

		self.hbox2.pack_end(self.EntryServerHost, False, True, 0)
	
		self.vbox2.pack_start(self.hbox2, True, True, 0)
	
		self.hbox5 = gtk.HBox(False, 0)
		self.hbox5.show()
		self.hbox5.set_spacing(0)
	
		self.s_port_Label = gtk.Label(_("Server Port:"))
		self.s_port_Label.set_padding(0, 0)
		self.s_port_Label.show()
		self.hbox5.pack_start(self.s_port_Label, False, False, 0)
		aj = gtk.Adjustment(value=2240, lower=0, upper=65535, step_incr=1, page_incr=0, page_size=0)
		self.ServerPort = gtk.SpinButton(aj)
		self.ServerPort.set_numeric(True)
		self.ServerPort.show()
		self.ServerPort.connect("changed", self.ServerPortChanged)
		
		self.hbox5.pack_end(self.ServerPort, False, True, 0)
	
		self.vbox2.pack_start(self.hbox5, True, True, 0)
	
		self.hbox4 = gtk.HBox(False, 5)
		self.hbox4.show()
		self.hbox4.set_spacing(5)
	
		self.s_username_Label = gtk.Label(_("Server Username:"))
		self.s_username_Label.set_padding(0, 0)
		self.s_username_Label.show()
		self.hbox4.pack_start(self.s_username_Label, False, False, 0)
	
		self.EntryServerUsername = gtk.Entry()
		self.EntryServerUsername.set_text("")
		self.EntryServerUsername.set_editable(True)
		self.EntryServerUsername.show()
		self.EntryServerUsername.set_visibility(True)
		self.EntryServerUsername.connect("changed", self.ServerUsernameChanged)
		self.hbox4.pack_end(self.EntryServerUsername, False, True, 0)
	
		self.vbox2.pack_start(self.hbox4, True, True, 0)
	
		self.hbox3 = gtk.HBox(False, 5)
		self.hbox3.show()
		self.hbox3.set_spacing(5)
	
		self.s_password_Label = gtk.Label(_("Server Password:"))
		self.s_password_Label.set_padding(0, 0)
		self.s_password_Label.show()
		self.hbox3.pack_start(self.s_password_Label, False, False, 0)
	
		self.EntryServerPassword = gtk.Entry()
		self.EntryServerPassword.set_text("")
		self.EntryServerPassword.set_editable(True)
		self.EntryServerPassword.show()
		self.EntryServerPassword.set_visibility(False)
		
		self.EntryServerPassword.connect("changed", self.ServerPasswordChanged)
		self.hbox3.pack_end(self.EntryServerPassword, False, True, 0)
	
		self.vbox2.pack_start(self.hbox3, True, True, 0)
	
		self.frame1 = gtk.Frame()
		self.frame1.show()
		self.frame1.set_shadow_type(gtk.SHADOW_IN)
	
		self.alignment1 = gtk.Alignment(0.5, 0.5, 1, 1)
		self.alignment1.show()
	
		self.vbox3 = gtk.VBox(False, 5)
		self.vbox3.show()
		self.vbox3.set_spacing(5)
	
		self.hbox6 = gtk.HBox(False, 5)
		self.hbox6.show()
		self.hbox6.set_spacing(5)
	
		self.default___Label = gtk.Label(_("Default: "))
		self.default___Label.set_padding(5, 0)
		self.default___Label.show()
		self.hbox6.pack_start(self.default___Label, False, False, 0)
	
		self.defaultEncoding_List = gtk.ListStore(gobject.TYPE_STRING)
		self.defaultEncoding = gtk.ComboBoxEntry()
		self.defaultEncoding.show()
		for encoding in self.encodings:
			self.defaultEncoding_List.append([encoding])
	
		self.defaultEncoding.set_model(self.defaultEncoding_List)
		self.defaultEncoding.set_text_column(0)
		self.defaultEncoding.connect("changed", self.EncodingDefaultChanged)
		
		self.hbox6.pack_end(self.defaultEncoding, False, True, 0)
	
		self.vbox3.pack_start(self.hbox6, True, True, 0)

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
	
		self.vbox3.pack_start(self.hbox7, True, True, 0)
	
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
	
		self.vbox3.pack_start(self.hbox8, True, True, 0)
	
		self.alignment1.add(self.vbox3)
	
		self.frame1.add(self.alignment1)
	
		self.encodings__Label = gtk.Label()
		self.encodings__Label.set_markup(_("<b>Encodings</b>") )
		
		self.encodings__Label.set_padding(0, 0)
		self.encodings__Label.show()
		self.frame1.set_label_widget(self.encodings__Label)
	
		self.vbox2.pack_start(self.frame1, True, True, 0)
	
		self.hbox33 = gtk.HBox(False, 5)
		self.hbox33.show()
		self.hbox33.set_spacing(5)
		self.hbox33.set_border_width(3)
	
		self.fp_Label = gtk.Label(_("First Soulseek Port: "))
		self.fp_Label.set_padding(0, 0)
		self.fp_Label.show()
		self.hbox33.pack_start(self.fp_Label, False, False, 0)
		
		fp = gtk.Adjustment(value=2239, lower=0, upper=65535, step_incr=1, page_incr=0, page_size=0)
		self.FirstPort = gtk.SpinButton(fp)
		self.FirstPort.show()
		self.FirstPort.connect("changed", self.FirstPortChanged)
		
		self.hbox33.pack_start(self.FirstPort, True, True, 0)
	
		self.lp_Label = gtk.Label(_("Last Soulseek Port:"))
		self.lp_Label.set_padding(0, 0)
		self.lp_Label.show()
		self.hbox33.pack_start(self.lp_Label, False, False, 0)
		lp = gtk.Adjustment(value=2234, lower=0, upper=65535, step_incr=1, page_incr=0, page_size=0)
		self.LastPort = gtk.SpinButton(lp)
		self.LastPort.show()
		self.LastPort.connect("changed", self.LastPortChanged)
		
		self.hbox33.pack_start(self.LastPort, True, True, 0)
	
		self.vbox2.pack_start(self.hbox33, True, True, 0)
	
		self.viewport2.add(self.vbox2)
	
		self.scrolledwindow1.add(self.viewport2)
	
		self.label1 = gtk.Label(_("Server"))
		self.label1.set_padding(0, 0)
		self.label1.show()
		self.scrolledwindow3 = gtk.ScrolledWindow()
		self.scrolledwindow3.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.scrolledwindow3.show()
		self.scrolledwindow3.set_shadow_type(gtk.SHADOW_NONE)
	
		self.viewport3 = gtk.Viewport()
		self.viewport3.show()
		self.viewport3.set_shadow_type(gtk.SHADOW_IN)
	
		self.vbox4 = gtk.VBox(False, 0)
		self.vbox4.show()
		self.vbox4.set_spacing(0)
	
		self.hbox11 = gtk.HBox(False, 5)
		self.hbox11.show()
		self.hbox11.set_spacing(5)
		self.hbox11.set_border_width(3)
	
		self.label33 = gtk.Label(_("Client Interfaces Password:"))
		self.label33.set_padding(0, 0)
		self.label33.show()
		self.hbox11.pack_start(self.label33, False, False, 0)
	
		self.interfacePassword = gtk.Entry()
		self.interfacePassword.set_text("")
		self.interfacePassword.set_editable(True)
		self.interfacePassword.show()
		self.interfacePassword.set_visibility(False)
		self.interfacePassword.connect("changed", self.InterfacePasswordChanged)
		
		self.hbox11.pack_end(self.interfacePassword, False, True, 0)
	
		self.vbox4.pack_start(self.hbox11, False, True, 0)
	
		self.frame2 = gtk.Frame()
		self.frame2.show()
		self.frame2.set_shadow_type(gtk.SHADOW_IN)
	
		self.alignment2 = gtk.Alignment(0.5, 0.5, 1, 1)
		self.alignment2.show()
	
		self.hbox10 = gtk.HBox(False, 0)
		self.hbox10.show()
		self.hbox10.set_spacing(0)
	
		self.scrolledwindow16 = gtk.ScrolledWindow()
		self.scrolledwindow16.set_policy(gtk.POLICY_ALWAYS, gtk.POLICY_ALWAYS)
		self.scrolledwindow16.show()
		self.scrolledwindow16.set_shadow_type(gtk.SHADOW_NONE)
		
		self.interfacesTreestore = gtk.TreeStore(  str, int )
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
	
		self.hbox10.pack_start(self.scrolledwindow16, True, True, 0)
	
		self.vbox5 = gtk.VBox(False, 5)
		self.vbox5.show()
		self.vbox5.set_spacing(5)
		self.vbox5.set_border_width(3)
	
		self.AddInterface = gtk.Button()
		self.AddInterface.set_label(_("Add Interface"))
		self.AddInterface.show()
	
		self.vbox5.pack_start(self.AddInterface, False, False, 0)
		
		self.EditInterface = gtk.Button()
		self.EditInterface.set_label(_("Edit Interface"))
		self.EditInterface.show()
	
		self.vbox5.pack_start(self.EditInterface, False, False, 0)
		
		self.RemoveInterface = gtk.Button()
		self.RemoveInterface.set_label(_("Remove Interface"))
		self.RemoveInterface.show()
		
		self.AddInterface.connect("clicked", self.OnAddInterface)
		self.EditInterface.connect("clicked", self.OnEditInterface)
		self.RemoveInterface.connect("clicked", self.OnRemoveInterface)
		
		self.vbox5.pack_start(self.RemoveInterface, False, False, 0)
	
		self.hbox10.pack_end(self.vbox5, False, True, 0)
	
		self.alignment2.add(self.hbox10)
	
		self.frame2.add(self.alignment2)
	
		self.label32 = gtk.Label()
		self.label32.set_markup(_("<b>Client Interface Ports &amp; Sockets</b>"))
		
		self.label32.set_padding(0, 0)
		self.label32.show()
		self.frame2.set_label_widget(self.label32)
	
		self.vbox4.pack_start(self.frame2, True, True, 0)
	
		self.viewport3.add(self.vbox4)
	
		self.scrolledwindow3.add(self.viewport3)
	
		self.label3 = gtk.Label(_("Museek Clients"))
		self.label3.set_padding(0, 0)
		self.label3.show()
		self.TransfersScrollWindow = gtk.ScrolledWindow()
		self.TransfersScrollWindow.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.TransfersScrollWindow.show()
		self.TransfersScrollWindow.set_shadow_type(gtk.SHADOW_NONE)
	
		self.viewport4 = gtk.Viewport()
		self.viewport4.show()
		self.viewport4.set_shadow_type(gtk.SHADOW_IN)
	
		self.vbox6 = gtk.VBox(False, 0)
		self.vbox6.show()
		self.vbox6.set_spacing(0)
	
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
	
		self.vbox6.pack_start(self.hbox12, True, True, 0)
	
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
	
		self.vbox6.pack_start(self.hbox13, True, True, 0)
	
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
	
		self.vbox6.pack_start(self.hbox14, True, True, 0)
	
		self.hbox15 = gtk.HBox(False, 5)
		self.hbox15.show()
		self.hbox15.set_spacing(5)
		self.hbox15.set_border_width(3)
	
		self.label34 = gtk.Label(_("Upload Slots:"))
		self.label34.set_padding(0, 0)
		self.label34.show()
		self.hbox15.pack_start(self.label34, False, False, 0)
		us = gtk.Adjustment(value=1, lower=0, upper=1000, step_incr=1, page_incr=0, page_size=0)
		
		self.uploadSlots = gtk.SpinButton(us)
		self.uploadSlots.show()
		self.uploadSlots.connect("changed", self.UploadSlotsChanged)
		
		self.hbox15.pack_start(self.uploadSlots, False, True, 0)
		
		self.pabel= gtk.Label(_("Connection Mode:"))
		self.pabel.set_padding(0, 0)
		self.pabel.show()
		self.hbox15.pack_start(self.pabel, False, False, 0)
		
		self.connectMode_List = gtk.ListStore(gobject.TYPE_STRING)
		self.connectMode = gtk.ComboBoxEntry()
		self.connectMode.show()
		
		
		for item in ["passive", "active"]:
			self.connectMode_List.append([item])
	
		self.connectMode.set_model(self.connectMode_List)
		self.connectMode.set_text_column(0)
		self.connectMode.connect("changed", self.ConnectModeChanged)
		self.hbox15.pack_start(self.connectMode, False, False, 0)
		
		self.vbox6.pack_start(self.hbox15, False, True, 0)
		
		self.hbox16 = gtk.HBox(False, 5)
		self.hbox16.show()
		self.hbox16.set_spacing(5)
		self.hbox16.set_border_width(3)
	
		self.label35 = gtk.Label(_("Download Directory:"))
		self.label35.set_padding(0, 0)
		self.label35.show()
		self.hbox16.pack_start(self.label35, False, False, 0)
	
		self.EntryDownloadDIr = gtk.Entry()
		self.EntryDownloadDIr.set_text("")
		self.EntryDownloadDIr.set_editable(True)
		self.EntryDownloadDIr.show()
		self.EntryDownloadDIr.set_visibility(True)
		self.EntryDownloadDIr.connect("changed", self.DownloadDirChanged)
		
		self.hbox16.pack_start(self.EntryDownloadDIr, True, True, 0)
	
		self.downloadDirButton = gtk.Button()
		self.downloadDirButton.set_label(_("Select Directory"))
		self.downloadDirButton.show()
		
		self.downloadDirButton.connect("clicked", self.OnDownloadDir)
		
	
		self.hbox16.pack_end(self.downloadDirButton, False, False, 0)
	
		self.vbox6.pack_start(self.hbox16, True, True, 0)
	
		self.hbox17 = gtk.HBox(False, 5)
		self.hbox17.show()
		self.hbox17.set_spacing(5)
		self.hbox17.set_border_width(3)
	
		self.id_Label = gtk.Label(_("Incomplete Directory:"))
		self.id_Label.set_padding(0, 0)
		self.id_Label.show()
		self.hbox17.pack_start(self.id_Label, False, False, 0)
	
		self.EntryIncompleteDir = gtk.Entry()
		self.EntryIncompleteDir.set_text("")
		self.EntryIncompleteDir.set_editable(True)
		self.EntryIncompleteDir.show()
		self.EntryIncompleteDir.set_visibility(True)
		self.EntryIncompleteDir.connect("changed", self.IncompleteDirChanged)
		
		self.hbox17.pack_start(self.EntryIncompleteDir, True, True, 0)
	
		self.incompleteDirButton = gtk.Button()
		self.incompleteDirButton.set_label(_("Select Directory"))
		self.incompleteDirButton.show()
		
		self.incompleteDirButton.connect("clicked", self.OnIncompleteDir)
	
		self.hbox17.pack_end(self.incompleteDirButton, False, False, 0)
	
		self.vbox6.pack_start(self.hbox17, True, True, 0)
	
		self.dButtons_hbox = gtk.HBox(False, 5)
		self.dButtons_hbox.show()
		self.dButtons_hbox.set_spacing(5)
		self.dButtons_hbox.set_border_width(3)
	
		self.ddb_Label = gtk.Label(_("Downloads Database:"))
		self.ddb_Label.set_padding(0, 0)
		self.ddb_Label.show()
		self.dButtons_hbox.pack_start(self.ddb_Label, False, False, 0)
	
		self.EntryDownloadsDBase = gtk.Entry()
		self.EntryDownloadsDBase.set_text("")
		self.EntryDownloadsDBase.set_editable(True)
		self.EntryDownloadsDBase.show()
		self.EntryDownloadsDBase.set_visibility(True)
		self.EntryDownloadsDBase.connect("changed", self.DownloadDBChanged)
		
		self.dButtons_hbox.pack_start(self.EntryDownloadsDBase, True, True, 0)
		
		self.downloadsDBaseButton = gtk.Button()
		self.downloadsDBaseButton.set_label(_("Select Database"))
		self.downloadsDBaseButton.show()
		
		self.downloadsDBaseButton.connect("clicked", self.OnDownloadDBase)
		
		self.dButtons_hbox.pack_end(self.downloadsDBaseButton, False, False, 0)
	
		self.vbox6.pack_start(self.dButtons_hbox, True, True, 0)
	
		self.viewport4.add(self.vbox6)
	
		self.TransfersScrollWindow.add(self.viewport4)
	
		self.TransfersLabel = gtk.Label(_("Transfers"))
		self.TransfersLabel.set_padding(0, 0)
		self.TransfersLabel.show()
	
		self.notebook3 = gtk.Notebook()
		self.notebook3.set_tab_pos(gtk.POS_TOP)
		self.notebook3.set_scrollable(False)
		self.notebook3.show()
	
		self.autojoin_hbox = gtk.HBox(False, 0)
		
		self.autojoin_hbox.set_spacing(0)
	
		self.autojoinScrollWindow2 = gtk.ScrolledWindow()
		self.autojoinScrollWindow2.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.autojoinScrollWindow2.show()
		self.autojoinScrollWindow2.set_shadow_type(gtk.SHADOW_NONE)
	
		self.autojoinTreestore = gtk.TreeStore(  str, int )
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
	
		self.AddAutojoin = gtk.Button()
		self.AddAutojoin.set_label(_("Add"))
		self.AddAutojoin.show()
	
		self.vbox17.pack_start(self.AddAutojoin, False, False, 0)
		
		self.EditAutojoin = gtk.Button()
		self.EditAutojoin.set_label(_("Edit"))
		self.EditAutojoin.show()
	
		self.vbox17.pack_start(self.EditAutojoin, False, False, 0)	
		self.RemoveAutojoin = gtk.Button()
		self.RemoveAutojoin.set_label(_("Remove"))
		self.RemoveAutojoin.show()
		
		self.AddAutojoin.connect("clicked", self.OnAddAutojoin)
		self.EditAutojoin.connect("clicked", self.OnEditAutojoin)
		self.RemoveAutojoin.connect("clicked", self.OnRemoveAutojoin)
		
		self.vbox17.pack_start(self.RemoveAutojoin, False, False, 0)
	
		self.autojoin_hbox.pack_start(self.vbox17, False, True, 0)
		self.autojoin_hbox.show()
	
		self.autojoinLabel = gtk.Label(_("AutoJoined"))
		self.autojoinLabel.set_padding(0, 0)
		self.autojoinLabel.show()
		
			
		self.encodings_hbox = gtk.HBox(False, 0)
		self.encodings_hbox.show()
		self.encodings_hbox.set_spacing(0)
	
		self.encodingsScrollWindow2 = gtk.ScrolledWindow()
		self.encodingsScrollWindow2.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.encodingsScrollWindow2.show()
		self.encodingsScrollWindow2.set_shadow_type(gtk.SHADOW_NONE)
	
		self.encodingsTreestore = gtk.TreeStore(  str,str, int )
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
	
		self.AddEncoding = gtk.Button()
		self.AddEncoding.set_label(_("Add"))
		self.AddEncoding.show()
	
		self.encodings_vbox.pack_start(self.AddEncoding, False, False, 0)
		
		self.EditEncoding = gtk.Button()
		self.EditEncoding.set_label(_("Edit"))
		self.EditEncoding.show()
	
		self.encodings_vbox.pack_start(self.EditEncoding, False, False, 0)
		
		self.RemoveEncoding = gtk.Button()
		self.RemoveEncoding.set_label(_("Remove"))
		self.RemoveEncoding.show()
		
		self.AddEncoding.connect("clicked", self.OnAddEncoding)
		self.EditEncoding.connect("clicked", self.OnEditEncoding)
		self.RemoveEncoding.connect("clicked", self.OnRemoveEncoding)
		
		self.encodings_vbox.pack_start(self.RemoveEncoding, False, False, 0)
	
		self.encodings_hbox.pack_start(self.encodings_vbox, False, True, 0)
	
		self.encodingsLabel = gtk.Label(_("Encodings"))
		self.encodingsLabel.set_padding(0, 0)
		self.encodingsLabel.show()
		
		self.vbox1 = gtk.VBox(False, 0)
		self.vbox1.show()
		self.vbox1.set_spacing(0)
	
		self.hbox1 = gtk.HBox(False, 5)
		self.hbox1.show()
		self.hbox1.set_spacing(5)
	
		self.defaultTickerLabel = gtk.Label(_("Default Ticker:"))
		self.defaultTickerLabel.set_padding(0, 0)
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
		self.tickersScrollWindow.set_shadow_type(gtk.SHADOW_NONE)
	
		self.tickersTreestore = gtk.TreeStore(  str,str, int )
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
	
		self.AddTicker = gtk.Button()
		self.AddTicker.set_label(_("Add"))
		self.AddTicker.show()
	
		self.vbox15.pack_start(self.AddTicker, False, False, 0)
		
		self.EditTicker = gtk.Button()
		self.EditTicker.set_label(_("Edit"))
		self.EditTicker.show()
	
		self.vbox15.pack_start(self.EditTicker, False, False, 0)
			
		self.RemoveTicker = gtk.Button()
		self.RemoveTicker.set_label(_("Remove"))
		self.RemoveTicker.show()
		
		self.AddTicker.connect("clicked", self.OnAddTicker)
		self.EditTicker.connect("clicked", self.OnEditTicker)
		self.RemoveTicker.connect("clicked", self.OnRemoveTicker)
		
		self.vbox15.pack_start(self.RemoveTicker, False, False, 0)
	
		self.hbox34.pack_start(self.vbox15, False, True, 0)
	
		self.vbox1.pack_start(self.hbox34, True, True, 0)
	
		self.TickersLabel = gtk.Label(_("Tickers"))
		self.TickersLabel.set_padding(0, 0)
		self.TickersLabel.show()
		self.notebook3.append_page(self.autojoin_hbox, self.autojoinLabel)
	
		self.notebook3.append_page(self.encodings_hbox, self.encodingsLabel)
	
		self.notebook3.append_page(self.vbox1, self.TickersLabel)
	
	
		self.ChatRooms_Label = gtk.Label(_("Chat Rooms"))
		self.ChatRooms_Label.set_padding(0, 0)
		self.ChatRooms_Label.show()
		self.notebook_users = gtk.Notebook()
		self.notebook_users.set_tab_pos(gtk.POS_TOP)
		self.notebook_users.set_scrollable(False)
		self.notebook_users.show()
	
		self.hbox22 = gtk.HBox(False, 0)
		self.hbox22.show()
		self.hbox22.set_spacing(0)
	
		self.scrolledwindow19 = gtk.ScrolledWindow()
		self.scrolledwindow19.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.scrolledwindow19.show()
		self.scrolledwindow19.set_shadow_type(gtk.SHADOW_NONE)
	
		self.BuddiesTreestore = gtk.TreeStore(  str,str, int )
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
	
		self.AddBuddy = gtk.Button()
		self.AddBuddy.set_label(_("Add"))
		self.AddBuddy.show()
	
		self.vbox7.pack_start(self.AddBuddy, False, False, 0)
		
		self.EditBuddy = gtk.Button()
		self.EditBuddy.set_label(_("Edit"))
		self.EditBuddy.show()
	
		self.vbox7.pack_start(self.EditBuddy, False, False, 0)
		
		self.RemoveBuddy = gtk.Button()
		self.RemoveBuddy.set_label(_("Remove"))
		self.RemoveBuddy.show()
		
		self.AddBuddy.connect("clicked", self.OnAddBuddy)
		self.EditBuddy.connect("clicked", self.OnEditBuddy)
		self.RemoveBuddy.connect("clicked", self.OnRemoveBuddy)
		
		self.vbox7.pack_start(self.RemoveBuddy, False, False, 0)
	
		self.hbox22.pack_start(self.vbox7, False, True, 0)
	
		self.BuddiesTab = gtk.Label(_("Buddies"))
		self.BuddiesTab.set_padding(0, 0)
		self.BuddiesTab.show()
		self.hbox23 = gtk.HBox(False, 0)
		self.hbox23.show()
		self.hbox23.set_spacing(0)
	
		self.BannedScrollWindow = gtk.ScrolledWindow()
		self.BannedScrollWindow.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.BannedScrollWindow.show()
		self.BannedScrollWindow.set_shadow_type(gtk.SHADOW_NONE)
	
	
		self.BannedTreestore = gtk.TreeStore(  str,str, int )
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
	
		self.AddBanned = gtk.Button()
		self.AddBanned.set_label(_("Add"))
		self.AddBanned.show()
	
		self.vbox8.pack_start(self.AddBanned, False, False, 0)
		
		self.EditBanned = gtk.Button()
		self.EditBanned.set_label(_("Edit"))
		self.EditBanned.show()
	
		self.vbox8.pack_start(self.EditBanned, False, False, 0)	
		self.RemoveBanned = gtk.Button()
		self.RemoveBanned.set_label(_("Remove"))
		self.RemoveBanned.show()
		
		self.EditBanned.connect("clicked", self.OnEditBanned)
		self.AddBanned.connect("clicked", self.OnAddBanned)
		self.RemoveBanned.connect("clicked", self.OnRemoveBanned)
		
		self.vbox8.pack_start(self.RemoveBanned, False, False, 0)
	
		self.hbox23.pack_start(self.vbox8, False, True, 0)
	
		self.BannedTab = gtk.Label(_("Banned"))
		self.BannedTab.set_padding(0, 0)
		self.BannedTab.show()
		self.hbox24 = gtk.HBox(False, 0)
		self.hbox24.show()
		self.hbox24.set_spacing(0)
	
		self.IgnoredScrollWindow = gtk.ScrolledWindow()
		self.IgnoredScrollWindow.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.IgnoredScrollWindow.show()
		self.IgnoredScrollWindow.set_shadow_type(gtk.SHADOW_NONE)
	
		self.IgnoredTreestore = gtk.TreeStore(  str,str, int )
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
	
		self.AddIgnored = gtk.Button()
		self.AddIgnored.set_label(_("Add"))
		self.AddIgnored.show()
	
		self.vbox9.pack_start(self.AddIgnored, False, False, 0)
		
		self.EditIgnored = gtk.Button()
		self.EditIgnored.set_label(_("Edit"))
		self.EditIgnored.show()
	
		self.vbox9.pack_start(self.EditIgnored, False, False, 0)
			
		self.RemoveIgnored = gtk.Button()
		self.RemoveIgnored.set_label(_("Remove"))
		self.RemoveIgnored.show()
		
		self.EditIgnored.connect("clicked", self.OnEditIgnored)
		self.AddIgnored.connect("clicked", self.OnAddIgnored)
		self.RemoveIgnored.connect("clicked", self.OnRemoveIgnored)	
		
		self.vbox9.pack_start(self.RemoveIgnored, False, False, 0)
	
		self.hbox24.pack_start(self.vbox9, False, True, 0)
	
		self.IgnoredTab = gtk.Label(_("Ignored"))
		self.IgnoredTab.set_padding(0, 0)
		self.IgnoredTab.show()
		self.hbox25 = gtk.HBox(False, 0)
		self.hbox25.show()
		self.hbox25.set_spacing(0)
	
		self.TrustedScrollWindow = gtk.ScrolledWindow()
		self.TrustedScrollWindow.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.TrustedScrollWindow.show()
		self.TrustedScrollWindow.set_shadow_type(gtk.SHADOW_NONE)

		self.TrustedTreestore = gtk.TreeStore(  str,str, int )
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
	
		self.AddTrusted = gtk.Button()
		self.AddTrusted.set_label(_("Add"))
		self.AddTrusted.show()
		
		self.trusted_vbox.pack_start(self.AddTrusted, False, False, 0)
	
		self.EditTrusted = gtk.Button()
		self.EditTrusted.set_label(_("Edit"))
		self.EditTrusted.show()
		
		self.trusted_vbox.pack_start(self.EditTrusted, False, False, 0)
		
		self.RemoveTrusted = gtk.Button()
		self.RemoveTrusted.set_label(_("Remove"))
		self.RemoveTrusted.show()
		
		self.AddTrusted.connect("clicked", self.OnAddTrusted)
		self.EditTrusted.connect("clicked", self.OnEditTrusted)
		self.RemoveTrusted.connect("clicked", self.OnRemoveTrusted)	
		
		self.trusted_vbox.pack_start(self.RemoveTrusted, False, False, 0)
	
		self.hbox25.pack_start(self.trusted_vbox, False, True, 0)
	
		self.TrustedTab = gtk.Label(_("Trusted"))
		self.TrustedTab.set_padding(0, 0)
		self.TrustedTab.show()
		self.notebook_users.append_page(self.hbox22, self.BuddiesTab)
	
		self.notebook_users.append_page(self.hbox23, self.BannedTab)
	
		self.notebook_users.append_page(self.hbox24, self.IgnoredTab)
	
		self.notebook_users.append_page(self.hbox25, self.TrustedTab)
	
		self.users_Label = gtk.Label(_("Users"))
		self.users_Label.set_padding(0, 0)
		self.users_Label.show()
		self.userinfoScrollWindow = gtk.ScrolledWindow()
		self.userinfoScrollWindow.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.userinfoScrollWindow.show()
		self.userinfoScrollWindow.set_shadow_type(gtk.SHADOW_NONE)
	
		self.viewport5 = gtk.Viewport()
		self.viewport5.show()
		self.viewport5.set_shadow_type(gtk.SHADOW_IN)
	
		self.hpaned1 = gtk.HPaned()
		self.hpaned1.show()
	
		self.vbox12 = gtk.VBox(False, 0)
		self.vbox12.show()
		self.vbox12.set_spacing(0)
	
		self.UserinfoTextScrollWindow = gtk.ScrolledWindow()
		self.UserinfoTextScrollWindow.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.UserinfoTextScrollWindow.show()
		self.UserinfoTextScrollWindow.set_shadow_type(gtk.SHADOW_NONE)
		
		self.userinfoBuffer = gtk.TextBuffer()
		self.userinfoTextview = gtk.TextView(self.userinfoBuffer)
		self.userinfoTextview.set_cursor_visible(True)
		self.userinfoTextview.set_editable(True)
		self.userinfoTextview.set_wrap_mode(gtk.WRAP_WORD)
		self.userinfoTextview.show()
		self.userinfoBuffer.connect("changed", self.UserinfoTextChanged)
		
		self.UserinfoTextScrollWindow.add(self.userinfoTextview)
	
		self.vbox12.pack_start(self.UserinfoTextScrollWindow, True, True, 0)
	
		self.hbox26 = gtk.HBox(False, 0)
		self.hbox26.show()
		self.hbox26.set_spacing(0)
	
		self.removeUserinfoText = gtk.Button()
		self.removeUserinfoText.set_label(_("Clear"))
		self.removeUserinfoText.show()
		
		self.removeUserinfoText.connect("clicked", self.OnClearUserinfo)
		
		self.hbox26.pack_start(self.removeUserinfoText, False, False, 0)
	
		self.selectUserinfoText = gtk.Button()
		self.selectUserinfoText.set_label(_("Import Text"))
		self.selectUserinfoText.show()
		self.selectUserinfoText.connect("clicked", self.OnImportUserinfo)
		
		self.hbox26.pack_end(self.selectUserinfoText, False, False, 0)
	
		self.vbox12.pack_start(self.hbox26, False, True, 0)
	
		self.hpaned1.pack1(self.vbox12, False, True)
	
		self.vbox11 = gtk.VBox(False, 0)
		self.vbox11.show()
		self.vbox11.set_spacing(0)
	
		self.scrolledwindow24 = gtk.ScrolledWindow()
		self.scrolledwindow24.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.scrolledwindow24.show()
		self.scrolledwindow24.set_shadow_type(gtk.SHADOW_NONE)
	
		self.viewport6 = gtk.Viewport()
		self.viewport6.show()
		self.viewport6.set_shadow_type(gtk.SHADOW_IN)
	
		self.userinfoImage = gtk.Image()
		self.userinfoImage.set_padding(0, 0)
		self.userinfoImage.show()
		self.viewport6.add(self.userinfoImage)
	
		self.scrolledwindow24.add(self.viewport6)
	
		self.vbox11.pack_start(self.scrolledwindow24, True, True, 0)
	
		self.hbox27 = gtk.HBox(False, 0)
		self.hbox27.show()
		self.hbox27.set_spacing(0)
	
		self.removeUserinfoImage = gtk.Button()
		self.removeUserinfoImage.set_label(_("Clear image"))
		self.removeUserinfoImage.show()
		self.removeUserinfoImage.connect("clicked", self.OnClearImage)
		
		self.hbox27.pack_start(self.removeUserinfoImage, False, False, 0)
		self.EntryImage = gtk.Entry()
		self.EntryImage.set_text("")
		self.EntryImage.set_editable(True)
		self.EntryImage.show()
		self.EntryImage.set_visibility(True)
		self.hbox27.pack_start(self.EntryImage, True, True, 0)
		
		self.selectUserinfoImage = gtk.Button()
		self.selectUserinfoImage.set_label(_("Select Image"))
		self.selectUserinfoImage.show()
		self.selectUserinfoImage.connect("clicked", self.OnOpenImage)
	
		self.hbox27.pack_end(self.selectUserinfoImage, False, False, 0)
	
		self.vbox11.pack_start(self.hbox27, False, True, 0)
	
		self.hpaned1.pack2(self.vbox11, True, True)
	
		self.viewport5.add(self.hpaned1)
	
		self.userinfoScrollWindow.add(self.viewport5)
	
		self.userinfo__Label = gtk.Label(_("My Userinfo"))
		self.userinfo__Label.set_padding(0, 0)
		self.userinfo__Label.show()
		self.databasesScrollWindow = gtk.ScrolledWindow()
		self.databasesScrollWindow.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.databasesScrollWindow.show()
		self.databasesScrollWindow.set_shadow_type(gtk.SHADOW_NONE)
	
		self.viewport7 = gtk.Viewport()
		self.viewport7.show()
		self.viewport7.set_shadow_type(gtk.SHADOW_IN)
	
		self.vbox13 = gtk.VBox(False, 0)
		self.vbox13.show()
		self.vbox13.set_spacing(0)
	
		self.frame3 = gtk.Frame()
		self.frame3.show()
		self.frame3.set_border_width(3)
		self.frame3.set_shadow_type(gtk.SHADOW_NONE)
	
		self.alignment3 = gtk.Alignment(0.5, 0.5, 1, 1)
		self.alignment3.show()
	
		self.hbox29 = gtk.HBox(False, 5)
		self.hbox29.show()
		self.hbox29.set_spacing(5)
		self.hbox29.set_border_width(3)
	
		self.EntryNormalShares = gtk.Entry()
		self.EntryNormalShares.set_text("")
		self.EntryNormalShares.set_editable(True)
		self.EntryNormalShares.show()
		self.EntryNormalShares.set_visibility(True)
		
		self.EntryNormalShares.connect("changed", self.NormalSharesDBChanged)
		
		self.hbox29.pack_start(self.EntryNormalShares, True, True, 0)
	
		self.selectNormalDBase = gtk.Button()
		self.selectNormalDBase.set_label(_("Select DB"))
		self.selectNormalDBase.show()
		self.selectNormalDBase.connect("clicked", self.OnNormalDBase)
		self.hbox29.pack_end(self.selectNormalDBase, False, False, 0)
	
		self.alignment3.add(self.hbox29)
	
		self.frame3.add(self.alignment3)
	
		self.normalSharesLabel = gtk.Label()
		self.normalSharesLabel.set_markup(_("<b>Normal Shares</b>"))
		
		self.normalSharesLabel.set_padding(0, 0)
		self.normalSharesLabel.show()
		self.frame3.set_label_widget(self.normalSharesLabel)
	
		self.vbox13.pack_start(self.frame3, False, True, 0)
	
		# -----------------------------------

		self.sharesdirshbox = gtk.HBox(False, 0)
		self.sharesdirshbox.show()
		self.sharesdirshbox.set_spacing(0)
		self.sharesdirshbox.set_border_width(5)
	
		self.sharesdirsScrollWindow = gtk.ScrolledWindow()
		self.sharesdirsScrollWindow.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.sharesdirsScrollWindow.show()
		self.sharesdirsScrollWindow.set_shadow_type(gtk.SHADOW_NONE)
		
		self.NormalDirTreestore = gtk.TreeStore(  str,int )
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
	
		self.sharesdirshbox.pack_start(self.sharesdirsScrollWindow, True, True, 0)
	
		self.nSharesButtonsVbox = gtk.VBox(False, 1)
		self.nSharesButtonsVbox.show()
		self.nSharesButtonsVbox.set_spacing(1)
		self.nSharesButtonsVbox.set_border_width(5)
	
		self.refreshNormalDir = gtk.Button()
		self.refreshNormalDir.set_label(_("Refresh list"))
		self.refreshNormalDir.show()
		self.refreshNormalDir.connect("clicked", self.OnRefreshNormalDirs)
		self.nSharesButtonsVbox.pack_start(self.refreshNormalDir, False, False, 0)
		
		self.rescanNormalDir = gtk.Button()
		self.rescanNormalDir.set_label(_("Rescan shares"))
		self.rescanNormalDir.show()
		self.rescanNormalDir.connect("clicked", self.OnRescanNormalDirs)
		self.nSharesButtonsVbox.pack_start(self.rescanNormalDir, False, False, 0)
	
		self.addNormalDir = gtk.Button()
		self.addNormalDir.set_label(_("Add dir"))
		self.addNormalDir.show()
		self.addNormalDir.connect("clicked", self.OnAddNormalDirs)
		
		self.nSharesButtonsVbox.pack_start(self.addNormalDir, False, False, 0)
	
		self.removeNormalDir = gtk.Button()
		self.removeNormalDir.set_label(_("Remove dir"))
		self.removeNormalDir.show()
		self.removeNormalDir.connect("clicked", self.OnRemoveNormalDirs)
	
		self.nSharesButtonsVbox.pack_end(self.removeNormalDir, False, False, 0)
	
		self.sharesdirshbox.pack_start(self.nSharesButtonsVbox, False, True, 0)
	
		self.vbox13.pack_start(self.sharesdirshbox, True, True, 0)
	
		self.bSharedDirsVbox = gtk.HBox(False, 0)
		self.bSharedDirsVbox.show()
		self.bSharedDirsVbox.set_spacing(0)
		self.bSharedDirsVbox.set_border_width(5)
	
		self.bsharesdirsScrollWindow = gtk.ScrolledWindow()
		self.bsharesdirsScrollWindow.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.bsharesdirsScrollWindow.show()
		self.bsharesdirsScrollWindow.set_shadow_type(gtk.SHADOW_NONE)
	
		self.BuddyDirTreestore = gtk.TreeStore(  str, int )
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
	
		self.bSharesButtonsVbox = gtk.VBox(False, 1)
		self.bSharesButtonsVbox.show()
		self.bSharesButtonsVbox.set_spacing(1)
		self.bSharesButtonsVbox.set_border_width(5)
	
		self.refreshBuddyDir = gtk.Button()
		self.refreshBuddyDir.set_label(_("Refresh list"))
		self.refreshBuddyDir.show()
		self.refreshBuddyDir.connect("clicked", self.OnRefreshBuddyDirs)
	
		self.bSharesButtonsVbox.pack_start(self.refreshBuddyDir, False, False, 0)
		
		self.rescanBuddyDir = gtk.Button()
		self.rescanBuddyDir.set_label(_("Rescan shares"))
		self.rescanBuddyDir.show()
		self.rescanBuddyDir.connect("clicked", self.OnRescanBuddyDirs)
		self.bSharesButtonsVbox.pack_start(self.rescanBuddyDir, False, False, 0)
		
		self.addBuddyDir = gtk.Button()
		self.addBuddyDir.set_label(_("Add dir"))
		self.addBuddyDir.show()
		self.addBuddyDir.connect("clicked", self.OnAddBuddyDirs)
		self.bSharesButtonsVbox.pack_start(self.addBuddyDir, False, False, 0)
	
		self.removeBuddyDir = gtk.Button()
		self.removeBuddyDir.set_label(_("Remove dir"))
		self.removeBuddyDir.show()
		self.removeBuddyDir.connect("clicked", self.OnRemoveBuddyDirs)
		self.bSharesButtonsVbox.pack_end(self.removeBuddyDir, False, False, 0)
	
		self.bSharedDirsVbox.pack_start(self.bSharesButtonsVbox, False, True, 0)
	
		# ------------------------
		self.frame4 = gtk.Frame()
		self.frame4.show()
		self.frame4.set_border_width(3)
		self.frame4.set_shadow_type(gtk.SHADOW_NONE)
	
		self.alignment4 = gtk.Alignment(0.5, 0.5, 1, 1)
		self.alignment4.show()
	
		self.hbox28 = gtk.HBox(False, 5)
		self.hbox28.show()
		self.hbox28.set_spacing(5)
		self.hbox28.set_border_width(3)
	
		self.EntryBuddyOnlyShares = gtk.Entry()
		self.EntryBuddyOnlyShares.set_text("")
		self.EntryBuddyOnlyShares.set_editable(True)
		self.EntryBuddyOnlyShares.show()
		self.EntryBuddyOnlyShares.set_visibility(True)
		self.EntryBuddyOnlyShares.connect("changed", self.BuddySharesDBChanged)
		
		self.hbox28.pack_start(self.EntryBuddyOnlyShares, True, True, 0)
	
		self.selectBuddyOnlyDBase = gtk.Button()
		self.selectBuddyOnlyDBase.set_label(_("Select DB"))
		self.selectBuddyOnlyDBase.show()
		self.selectBuddyOnlyDBase.connect("clicked", self.OnBuddyDBase)
		self.hbox28.pack_end(self.selectBuddyOnlyDBase, False, False, 0)
	
		self.alignment4.add(self.hbox28)
	
		self.frame4.add(self.alignment4)
	
		self.label39 = gtk.Label()
		self.label39.set_markup(_("<b>Buddy-Only Shares</b>"))
		self.label39.set_padding(0, 0)
		self.label39.show()
		self.frame4.set_label_widget(self.label39)
	
		self.vbox13.pack_start(self.frame4, False, True, 0)
		
		self.vbox13.pack_start(self.bSharedDirsVbox, True, True, 0)
		# -----------------------------------
		self.viewport7.add(self.vbox13)
	
		self.databasesScrollWindow.add(self.viewport7)
	
		self.sharesDBLabel = gtk.Label(_("Shares"))
		self.sharesDBLabel.set_padding(0, 0)
		self.sharesDBLabel.show()
		# -----------------------------------
		self.alert_hbox = gtk.HBox(False, 0)
		self.alert_hbox.show()
		self.alert_hbox.set_spacing(0)
	
		self.alertScrollWindow = gtk.ScrolledWindow()
		self.alertScrollWindow.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.alertScrollWindow.show()
		self.alertScrollWindow.set_shadow_type(gtk.SHADOW_NONE)
	
	
		self.AlertsTreestore = gtk.TreeStore(  str, int )
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
	
		self.alert_hbox.pack_start(self.alertScrollWindow, True, True, 0)
	
		self.vbox14 = gtk.VBox(False, 5)
		self.vbox14.show()
		self.vbox14.set_spacing(5)
		self.vbox14.set_border_width(3)
	
		self.AddAlert = gtk.Button()
		self.AddAlert.set_label(_("Add"))
		self.AddAlert.show()
	
		self.vbox14.pack_start(self.AddAlert, False, False, 0)
		
		self.EditAlert = gtk.Button()
		self.EditAlert.set_label(_("Edit"))
		self.EditAlert.show()
	
		self.vbox14.pack_start(self.EditAlert, False, False, 0)
		self.RemoveAlert = gtk.Button()
		self.RemoveAlert.set_label(_("Remove"))
		self.RemoveAlert.show()
		
		self.EditAlert.connect("clicked", self.OnEditAlert)
		self.AddAlert.connect("clicked", self.OnAddAlert)
		self.RemoveAlert.connect("clicked", self.OnRemoveAlert)
		
		self.vbox14.pack_start(self.RemoveAlert, False, False, 0)
	
		self.alert_hbox.pack_start(self.vbox14, False, True, 0)

		self.alertsLabel = gtk.Label(_("Alerts"))
		self.alertsLabel.set_padding(0, 0)
		self.alertsLabel.show()
		self.notebook_users.append_page(self.alert_hbox, self.alertsLabel)
		self.notebook1.append_page(self.scrolledwindow1, self.label1)
	
		self.notebook1.append_page(self.scrolledwindow3, self.label3)
	
		self.notebook1.append_page(self.TransfersScrollWindow, self.TransfersLabel)
	
		self.notebook1.append_page(self.notebook3, self.ChatRooms_Label)
	
		self.notebook1.append_page(self.notebook_users, self.users_Label)
	
		self.notebook1.append_page(self.userinfoScrollWindow, self.userinfo__Label)
	
		self.notebook1.append_page(self.databasesScrollWindow, self.sharesDBLabel)
	
		

		#self.label_dialog = gtk.Label(_("General Settings"))
		#self.label_dialog.set_padding(0, 0)
		#self.label_dialog.show()
		
		#self.notebook_parent.append_page(self.notebook1, self.label_dialog)

	def createTreeFor(self, store, treeview, name):
		store.clear()
		#print name
		if name in ("normaldirs", "buddydirs"):
			if name == "normaldirs":
				dict = self.SharedDirs["normal"]
			elif name == "buddydirs":
				dict = self.SharedDirs["buddy"]
			parents = self.parents[name] = {}
			num = 0
			for key in dict:
				parents[num] =  store.append(None, [key, num])
				num += 1
			self.parents[name] = parents
			
		elif name in self.config:
			dict = self.config[name]
			
			parents = self.parents[name] = {}
			num = 0
			for key, value in dict.items():

				if name in ("interfaces.bind", "autojoin", "alerts"):
					parents[num] =  store.append(None, [key, num])
				else:
					parents[num] =  store.append(None, [key, value, num])
				num += 1
			self.parents[name] = parents
		else:
			self.config[name] = {}
			
	def refreshConfigDisplay(self):
		self.populate_dialogs()
		
	def populate_dialogs(self):
		self.parents= {}
		# Server
		if self.config == {}:
			self.notebook1.set_sensitive( False)
			self.EntryServerHost.set_text("")
			self.ServerPort.set_value(0)
			self.EntryServerPassword.set_text("")
			self.EntryServerUsername.set_text("")
			self.defaultEncoding.child.set_text("")
			self.filesystemEncoding.child.set_text("")
			
			self.networkEncoding.child.set_text("")
			self.LastPort.set_value(0)
			self.FirstPort.set_value(0)
			
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
			#if "buddy.shares" not in self.config:
				#self.config["buddy.shares"] = {"database": config_path+".buddyshares"}
			self.EntryBuddyOnlyShares.set_text("")
			# Alerts
			self.AlertsTreestore.clear()
			self.NormalDirTreestore.clear()
			self.BuddyDirTreestore.clear()
			#self.OnRefreshBuddyDirs(None)
			self.notebook1.set_sensitive( False)
			return
		try:
			self.notebook1.set_sensitive(True)
			self.CONFIG_PATH = self.murmur_config["museekd"]["configfile"]
			#if self.CONFIG_PATH.rfind('.') > self.CONFIG_PATH.rfind('/'):
				#config_path = '.'.join(self.CONFIG_PATH.split('.')[:-1])
			self.EntryServerHost.set_text(self.config["server"]["host"])
			self.ServerPort.set_value(int(self.config["server"]["port"]))
			self.EntryServerPassword.set_text(self.config["server"]["password"])
			self.EntryServerUsername.set_text(self.config["server"]["username"])
			if "default" in self.config["encoding"]:
				self.defaultEncoding.child.set_text(str(self.config["encoding"]["default"]))
			if "filesystem" in self.config["encoding"]:
				self.filesystemEncoding.child.set_text(self.config["encoding"]["filesystem"])
			if "network" in self.config["encoding"]:
				self.networkEncoding.child.set_text(self.config["encoding"]["network"])
			self.LastPort.set_value(int(self.config["clients.bind"]["last"]))
			self.FirstPort.set_value(int(self.config["clients.bind"]["first"]))
			
			# Museek Clients
			self.interfacePassword.set_text(str(self.config["interfaces"]["password"]))
			self.createTreeFor( self.interfacesTreestore, self.interfacesTreeview, "interfaces.bind")	

			# Transfers
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
			self.uploadSlots.set_value(int(self.config["transfers"]["upload_slots"]))
			self.EntryDownloadDIr.set_text(str(self.config["transfers"]["download-dir"]))
			self.EntryIncompleteDir.set_text(str(self.config["transfers"]["incomplete-dir"]))
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

			self.userinfoImage.set_from_file(str(self.config["userinfo"]["image"]))
			self.EntryImage.set_text(str(self.config["userinfo"]["image"]))

			s= self.userinfoBuffer.get_start_iter()
			self.userinfoBuffer.insert(s, str(self.config["userinfo"]["text"]))
			self.importinguserinfo =0
			# Shares DBs
			
	
			self.EntryNormalShares.set_text(str(self.config["shares"]["database"]))
			if "buddy.shares" not in self.config:
				self.config["buddy.shares"] = {"database": config_path+".buddyshares"}
			self.EntryBuddyOnlyShares.set_text(str(self.config["buddy.shares"]["database"]))
			# Alerts
			self.createTreeFor( self.AlertsTreestore, self.AlertsTreeview, "alerts")
			self.OnRefreshNormalDirs(None)
			self.OnRefreshBuddyDirs(None)
		except Exception, e:
			self.Bug(_("Populating the Dialogs with config data failed:"), str(e))
			

	def read_config(self):
		self.SharedDirs = {"normal":[], "buddy":[] }
		self.config = {}
		for section in self.app.ProcessMessages.config.keys():
			self.config[section] = {}
			for key, value in self.app.ProcessMessages.config[section].items():
				self.config[section][key] = value
		if self.murmur_config["connection"]["interface"] != None:
			self.Interface.set_text(self.murmur_config["connection"]["interface"])
		else:
			self.Interface.set_text("Input a Museek Interface")
		if self.murmur_config["connection"]["passw"] != None:
			self.Password.set_text(self.murmur_config["connection"]["passw"])
		else:
			self.Password.set_text("Input a password")
		#if self.app.ProcessMessages.config == {}:
			#self.Username.set_sensitive( False)
			#self.Username.set_text("")
		#else:
			#self.Username.set_editable(True)
			#self.Username.set_sensitive( True)
			#self.Username.set_text(self.app.ProcessMessages.config["server"]["username"])

		if self.murmur_config["murmur"]["tooltips"] == "yes":
			self.TooltipCheck.set_active(True)
		else:
			self.TooltipCheck.set_active(False)
		self.populate_dialogs()
		#if self.app.ProcessMessages.config["userinfo"]["image"] != None:	
		
		
	def trayapp_check(self, string, zing):
		if "trayapp" not in self.murmur_config["murmur"]:
			self.check_button2.set_active(False)
			self.murmur_config["murmur"]["trayapp"] = "no"
			return
		if self.murmur_config["murmur"]["trayapp"] == "no":
			self.murmur_config["murmur"]["trayapp"] = "yes"
			
		else:
			self.murmur_config["murmur"]["trayapp"] = "no"



	def tooltip_check(self, string, zing):
		if "tooltips" not in self.murmur_config["murmur"]:
			self.TooltipCheck.set_active(False)
			self.app.tooltips.disable()
			self.murmur_config["murmur"]["tooltips"] = "no"
			return
		if self.TooltipCheck.get_active() == False:
			self.murmur_config["murmur"]["tooltips"] = "no"
			#Mapp.tooltips.disable()
		else:
			self.murmur_config["murmur"]["tooltips"] = "yes"
			#Mapp.tooltips.enable()
			
	def quit(self, w=None, event=None):
		self.hide()
		#self.destroy()
		#gtk.main_quit()
	def update_config(self):
		oldconfig = self.app.ProcessMessages.config
		for section in self.config:
			for key, value in self.config[section].items():
				#if self.config
				if key not in oldconfig[section].keys():
					self.app.ProcessMessages.Send(messages.ConfigSet(section, key, str(value)))
					print "Added", section, key, value
					continue
				else:
					if str(self.config[section][key]) != str(oldconfig[section][key]):
						print "new", self.config[section][key] 
						print "old", oldconfig[section][key]
						self.app.ProcessMessages.Send(messages.ConfigSet(section, key, str(value)))
						print "Changed", section, key, value
						continue
					#else:
						#print self.config[section][key], oldconfig[section][key]
						
			
	def click(self, button):
		#self.ret = self.entry.get_text()
		self.murmur_config["connection"]["interface"] = self.Interface.get_text()
		self.murmur_config["connection"]["passw"] = self.Password.get_text()
		self.update_config()
		self.quit()

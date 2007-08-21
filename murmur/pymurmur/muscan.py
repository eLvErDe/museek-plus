# This is part of the Murmur Museek Client, and distributed under the GPLv2
# Copyright (c) 2007 daelstorm.
import threading
import os

## Muscan commands, threads
#
class Muscan:
	## Constructor
	# Launch a local muscan process with special options from the setup buttons,
	# Or from /commands
	## @author daelstorm
	# @param self Muscan (Class)
	# @param frame Murmur (Class)
	def __init__(self, frame):
		## @var frame
		# Murmur (Class)
		self.frame = frame
		## @var timer
		# threading timer that calls ThreadMuscan
		self.timer = threading.Timer(1.0, self.ThreadMuscan)
		## @var command
		# once set, a list of command options for subprocess
		self.command = []
		self.configfile = None
		
	def GetConfig(self):
		self.configfile = None
		if self.frame.Networking.config["shares"]["database"] != "":
			fileprefix = self.frame.Networking.config["shares"]["database"].rsplit(".", 1)
			if len(fileprefix) == 2:
				file = fileprefix[0]+".xml"
				if os.path.exists(file):
					self.configfile = file
					
	## Clear timer and restart it in one second
	# @param self Muscan (Class)
	def RestartTimer(self):
		
		self.timer.cancel()
		if self.configfile != None:
			self.timer = threading.Timer(1.0, self.ThreadMuscan)
			self.timer.start()

	## Set command and call RestartTimer
	# @param self Muscan (Class)
	# @param command List of command options for subprocess
	def Command(self, command):
		self.command = command
		self.RestartTimer()

	## Spawn Subprocess and accept stdout and stderr
	# @param self Muscan (Class)
	def ThreadMuscan(self):
		try:
			self.timer.cancel()
			if self.frame.subprocess_fail:
				self.frame.AppendToLogWindow("This feature requires Python 2.4")
				return
			if self.configfile == None:
				self.frame.AppendToLogWindow("Museekd config file not found")
				return
			if self.frame.Config["connection"]["interface"][:9] in ("localhost", "/tmp/muse") and self.command != [] :
				p = "/usr/bin/muscan"
				if os.path.exists(p):
				
					z = self.frame.subprocess.Popen( self.command, stdout=self.frame.subprocess.PIPE, stderr=self.frame.subprocess.PIPE)
					stdout_text, stderr_text = z.communicate()
					z.wait()
					
					stdout_text = stdout_text.split('\n')
					stderr_text = stderr_text.split('\n')
					for line in stdout_text:
						if line.isspace() or line == '':
							pass
						else:
							self.frame.AppendToLogWindow(line)
					for line in stderr_text:
						if line.isspace() or line == '':
							pass
						else:
							self.frame.AppendToLogWindow(line)
					self.frame.AppendToLogWindow("Finished with shares.")
			else:
				self.frame.AppendToLogWindow("Your Museekd is either running remotely or already running a command, cancelling.")
		
			self.command = []
		except Exception,e:
			self.frame.AppendToLogWindow("Error: ThreadMuscan: " + str(e), 1)

	## Change setup input mode, so directory paths can be inputted
	# @param self Muscan (Class)
	# @param inputmode string that is set as Spl["setup_input"] variable
	def ChangeInput(self, inputmode):
		self.frame.Setup.input = inputmode
		self.frame.Setup.Mode()

	## Output list of Normal shared directories to debug log
	# @param self Muscan (Class)
	def ListNormal(self):
		self.command = ["muscan", "-c", self.configfile, "-l"]
		self.RestartTimer()
		self.frame.AppendToLogWindow("Listing normal shares with muscan:")

	## Output list of Buddy-only shared directories to debug log
	# @param self Muscan (Class)
	def ListBuddy(self):
		self.command = ["muscan", "-c", self.configfile, "-b", "-l"]
		self.RestartTimer()
		self.frame.AppendToLogWindow("Listing buddy shares with muscan:")

	## Rescan Buddy-only Shares (rebuilds shares from scratch)
	# @param self Muscan (Class)
	def RescanBuddy(self):
		self.command = ["muscan", "-c", self.configfile, "-v", "-b", "-r"]
		self.RestartTimer()
		self.frame.AppendToLogWindow("Rescanning buddy shares with muscan, don't forget to Reload them. Please wait for muscan to complete.")

	## Update Buddy-only Shares (checks mtimes and updates only newer dirs/files)
	# @param self Muscan (Class)
	def UpdateBuddy(self):
		self.command = ["muscan", "-c", self.configfile, "-v", "-b"]
		self.RestartTimer()
		self.frame.AppendToLogWindow("Updating buddy shares with muscan, don't forget to Reload them. Please wait for muscan to complete.")

	## Update Normal Shares (checks mtimes and updates only new dirs) 
	# @param self Muscan (Class)
	def UpdateNormal(self):
		self.command = ["muscan", "-c", self.configfile, "-v"]
		self.RestartTimer()
		self.frame.AppendToLogWindow("Updating shares with muscan, don't forget to Reload them. Please wait for muscan to complete.")

	## Rescan Normal Shares (rebuilds shares from scratch)
	# @param self Muscan (Class)
	def RescanNormal(self):
		self.command = ["muscan", "-c", self.configfile, "-v", "-r"]
		self.RestartTimer()
		self.frame.AppendToLogWindow("Rescanning shares with muscan, don't forget to Reload them. Please wait for muscan to complete.")
		
	def AddNormalDirectory(self, directory):
		dir = os.path.expanduser(directory)
		if os.path.exists(dir):
			
			self.command = ["muscan", "-c", self.configfile, "-v", "-s", dir]
			self.frame.AppendToLogWindow( "Adding %s to normal shares. Scanning will begin." % dir)
			self.RestartTimer()
		else:
			self.frame.AppendToLogWindow( "Warning: Directory does not exist: %s" % dir)
	def RemoveNormalDirectory(self, directory):
		dir = os.path.expanduser(directory)
		self.command = ["muscan", "-c", self.configfile, "-v", "-u", dir]
		self.frame.AppendToLogWindow( "Removing %s from normal shares. Please rescan or update." % dir)
		self.RestartTimer()
			
	def AddBuddyDirectory(self, directory):
		dir = os.path.expanduser(directory)
		if os.path.exists(dir):
			self.command = ["muscan", "-c", self.configfile, "-v", "-b", "-s", os.path.expanduser(directory)]
			self.frame.AppendToLogWindow( "Adding %s to buddy shares. Scanning will begin." % dir)
			self.RestartTimer()
		else:
			self.frame.AppendToLogWindow( "Warning: Directory does not exist: %s" % dir)
	def RemoveBuddyDirectory(self, directory):

		dir = os.path.expanduser(directory)
		self.command = ["muscan", "-c", self.configfile, "-v", "-b", "-u", dir]
		self.frame.AppendToLogWindow( "Removing %s from buddy shares. Please rescan or update." % dir)
		self.RestartTimer()
# This is part of the Mucous Museek Client, and distributed under the GPLv2
# Copyright (c) 2006 daelstorm. 
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
	# @param mucous Mucous (Class)
	def __init__(self, mucous):
		## @var mucous
		# Mucous (Class)
		self.mucous = mucous
		## @var timer
		# threading timer that calls ThreadMuscan
		self.timer = threading.Timer(1.0, self.ThreadMuscan)
		## @var command
		# once set, a list of command options for subprocess
		self.command = []

	## Clear timer and restart it in one second
	# @param self Muscan (Class)
	def RestartTimer(self):
		self.timer.cancel()
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
			if self.mucous.subprocess_fail:
				self.mucous.Help.Log("status", "This feature requires Python 2.4")
				return
			if self.mucous.Config["connection"]["interface"][:9] in ("localhost", "/tmp/muse") and self.command != [] :
				p = "/usr/bin/muscan"
				if os.path.exists(p):
				
					z = self.mucous.subprocess.Popen( self.command, stdout=self.mucous.subprocess.PIPE, stderr=self.mucous.subprocess.PIPE)
					stdout_text, stderr_text = z.communicate()
					z.wait()
					
					stdout_text = stdout_text.split('\n')
					stderr_text = stderr_text.split('\n')
					for line in stdout_text:
						if line.isspace() or line == '':
							pass
						else:
							self.mucous.Help.Log("status", line)
					for line in stderr_text:
						if line.isspace() or line == '':
							pass
						else:
							self.mucous.Help.Log("status", line)
					self.mucous.Help.Log("status", "Finished with shares.")
			else:
				self.mucous.Help.Log("status", "Your Museekd is either running remotely or already running a command, cancelling.")
		
			self.command = []
		except Exception,e:
			self.mucous.Help.Log("debug", "ThreadMuscan: " + str(e))

	## Change setup input mode, so directory paths can be inputted
	# @param self Muscan (Class)
	# @param inputmode string that is set as Spl["setup_input"] variable
	def ChangeInput(self, inputmode):
		self.mucous.Setup.input = inputmode
		self.mucous.Setup.Mode()

	## Output list of Normal shared directories to debug log
	# @param self Muscan (Class)
	def ListNormal(self):
		self.command = ["muscan", "-c", self.mucous.Spl["museekconfigfile"], "-l"]
		self.RestartTimer()
		self.mucous.Help.Log("status", "Listing normal shares with muscan:")

	## Output list of Buddy-only shared directories to debug log
	# @param self Muscan (Class)
	def ListBuddy(self):
		self.command = ["muscan", "-c", self.mucous.Spl["museekconfigfile"], "-b", "-l"]
		self.RestartTimer()
		self.mucous.Help.Log("status", "Listing buddy shares with muscan: %s" % self.mucous.Spl["museekconfigfile"])

	## Rescan Buddy-only Shares (rebuilds shares from scratch)
	# @param self Muscan (Class)
	def RescanBuddy(self):
		self.command = ["muscan", "-c", self.mucous.Spl["museekconfigfile"], "-v", "-b", "-r"]
		self.RestartTimer()
		self.mucous.Help.Log("status", "Rescanning buddy shares with muscan, don't forget to Reload them. Please wait for muscan to complete.")

	## Update Buddy-only Shares (checks mtimes and updates only newer dirs/files)
	# @param self Muscan (Class)
	def UpdateBuddy(self):
		self.command = ["muscan", "-c", self.mucous.Spl["museekconfigfile"], "-v", "-b"]
		self.RestartTimer()
		self.mucous.Help.Log("status", "Updating buddy shares with muscan, don't forget to Reload them. Please wait for muscan to complete.")

	## Update Normal Shares (checks mtimes and updates only new dirs) 
	# @param self Muscan (Class)
	def UpdateNormal(self):
		self.command = ["muscan", "-c", self.mucous.Spl["museekconfigfile"], "-v"]
		self.RestartTimer()
		self.mucous.Help.Log("status", "Updating shares with muscan, don't forget to Reload them. Please wait for muscan to complete.")

	## Rescan Normal Shares (rebuilds shares from scratch)
	# @param self Muscan (Class)
	def RescanNormal(self):
		self.command = ["muscan", "-c", self.mucous.Spl["museekconfigfile"], "-v", "-r"]
		self.RestartTimer()
		self.mucous.Help.Log("status", "Rescanning shares with muscan, don't forget to Reload them. Please wait for muscan to complete.")
		

# pymuseekd - Python tools for museekd
#
# Copyright (C) 2003-2004 Hyriand <hyriand@thegraveyard.org>
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

import socket, struct

class museekdsock(socket.socket):
	def __init__(self, *args):
		socket.socket.__init__(self, *args)

	def put(self, m):
		data = m.make()
		self.send(m.pack_int(len(data)))
		self.send(data)

	def get(self):
		l = struct.unpack("<i", self.recv(4, socket.MSG_WAITALL))[0]
		if l < 4:
			print "invalid message received"
			sys.stdout.flush()
			self.close()
			return
		code = struct.unpack("<i", self.recv(4, socket.MSG_WAITALL))[0]
		data = self.recv(l - 4, socket.MSG_WAITALL)
		return code, data

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

from museekdmsgs import museekdchallenge, museekdlogin, museekdserverstate
from museekdsock import museekdsock
import socket
import sha


def login(server, mask, disp):
	disp("Connecting to museekd (%s)..." % server)
	
	server = server.split(":")
	if server[0][:1] == '/':
		s = museekdsock(socket.AF_UNIX, socket.SOCK_STREAM)
		s.connect(server[0])
	else:
		s = museekdsock(socket.AF_INET, socket.SOCK_STREAM)
		s.connect((server[0], int(server[1])))
	passwd = server[-1]
	
	disp("Waiting for authentication challenge...")
	code, data = s.get()
	
	if code != museekdchallenge.code:
		disp("ERROR: Invalid message type received...")
		return None, -1
	
	m = museekdchallenge().parse(data)
	if m.version != 3:
		disp("ERROR: Incompatible interface protocol revision...")
		return None, -1
	
	chresponse = sha.sha(m.challenge + passwd).hexdigest()
	
	
	disp("Authenticating to museekd...")
	s.put(museekdlogin("SHA1", chresponse, mask))

	disp("Waiting for reply...")
	code, data = s.get()

	if code != museekdlogin.code:
		disp("ERROR: Invalid message type received...")
		return None, -1

	m = museekdlogin().parse(data)
	if m.result != 1:
		disp("Login failed, reason: %s" % m.msg)
		return None, -1
	
	
	disp("Logged in, awaiting server state")
	
	code, data = s.get()
	if code != museekdserverstate.code:
		disp("ERROR: Invalid message type received...")
		return None, -1
	m = museekdserverstate().parse(data)
	
	disp("Login sequence completed")
	
	return s, m

states = [
	"Finished",
	"Transferring",
	"Negotiating",
	"Waiting",
	"Connecting",
	"Queued",
	"Getting address",
	"Getting status",
	"Awaiting user",
	"Connection closed",
	"Cancelled",
	"Error",
]

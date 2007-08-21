#!/usr/bin/python
import os
import sys, string
print "Copy theme icons from images/theme subdirectory to images/ directory before running this script."
table = [
	["icon.png", "icon"],
	["away.png", "away"],
	["away_trusted.png", "away_trusted"],
	["away_banned.png", "away_banned"],
	["away_buddied.png", "away_buddied"],
	["away_ignored.png", "away_ignored"],
	["online.png", "online"],
	["online_trusted.png", "online_trusted"],
	["online_banned.png", "online_banned"],
	["online_ignored.png", "online_ignored"],
	["online_buddied.png", "online_buddied"],
	["offline.png", "offline"],
	["offline_trusted.png", "offline_trusted"],
	["offline_banned.png", "offline_banned"],
	["offline_ignored.png", "offline_ignored"],
	["offline_buddied.png", "offline_buddied"],
	["noexist.png", "noexist"],
	["red.png", "red"],
	["empty.png", "empty"],	
	["green.png", "green"],
	["yellow.png", "yellow"],
	["close.png", "close"],
	["logo.png", "logo"],
	["hilite.png", "hilite"],	
]
path = ""
if len (sys.argv[1:]) == 1:
    path = sys.argv[1]
outf = open(os.path.join("pymurmur","imagedata.py"), "w")
for image in table:
	print image[0]
	if os.path.exists(os.path.join(os.path.join("images", path), image[0])):
	    filename = os.path.join(os.path.join("images", path), image[0])
	else:
	    filename = os.path.join("images", image[0])
	
	f = open(filename, "rb")
	d = f.read()
	f.close()
	outf.write("%s = %s\n" % (image[1], `d`))
outf.close()

import re

def getVersion():
    versionfile = open('pymurmur/utils.py','r')
    for line in versionfile.readlines():
        line = line.strip()
        if re.match(r'^version = ',line):
            return line.split('=')[1].strip().replace('"','')

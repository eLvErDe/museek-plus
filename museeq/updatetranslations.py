#!/usr/bin/python
import os, sys
# Ripped out of the SConscript

def UpdateTranslations():
	global tr_dir, translations, lrelease

	make_qm(ts_templates)
	
	if ts_templates != []:
		if os.system(QTDIR+"/bin/"+"lrelease -help 2> /dev/null") == 0:
			print "Qt translation app 'lrelease' found in",QTDIR+"/bin/" 
			lrelease = QTDIR+"/bin/"+"lrelease"
		else:
			if os.system("lrelease -help 2> /dev/null") == 0:
				print "Qt translation app 'lrelease' found in the path."
				lrelease = "lrelease"
			else:
				print "Qt translation app 'lrelease' not found in the path or QTDIR", QTDIR+"/bin/ Aborting!"
				sys.exit()
	else:
		print "No Museeq translations will be built."
	for lang in translations:
		lsources = []
		mess = ""
		if os.path.exists(tr_dir+"museeq_"+lang+".ts"):
			lsources.append( tr_dir+"museeq_"+lang+".ts" )
			mess += "Museeq "+lang+" translation found... "
		else:
			mess += "Museeq "+lang+" translation NOT found! "
		if os.path.exists(tr_dir+"qt_"+lang+".ts"):
			lsources.append(tr_dir+"qt_"+lang+".ts")
			mess += "Qt "+lang+" translation found... "
		else:
			mess += "Qt "+lang+" translation NOT found!"
		if lsources == []:
			print "Neither museeq_"+lang+".ts nor qt_"+lang+".ts found. Translation skipped.."
			continue
		else:
			print mess
		translation_build("workdir/translations/"+"museeq_"+lang+".qm", lsources )

		
def make_qm(ts_templates):
	global translations
	for lang in ts_templates:
		translations.append(lang)

def translation_build(target, source):
	global lrelease
 	print source[0], target
	ss = ""
	for s in source:
		ss +=  str(s)+" "
	os.system(lrelease+" "+ str(ss)+" -qm "+ str(target))
	
	return 0

if __name__ == '__main__' :
	if len(sys.argv) == 1:
		print "Convert .ts files to .qm"
		print "Syntax: updatetranslations.py /path/to/qtdir languages"
		print "Example: ./updatetranslations.py /opt/qt fr de"
		print "If no translations are inputed, several will be selected automatically."
		sys.exit(1)
	QTDIR = sys.argv[1]
	trs = sys.argv[2:]
	
	translations = []
	ts_templates = []
	
	lrelease = ""
	tr_dir = "translations/"
	destdir = "workdir/translations/"
	if not os.path.exists(destdir):
		os.mkdir(destdir)
	if len(trs) == 0:
		trs = ['fr','de','es','it','pl','ru','ro','pt_BR','ja','zh','sk','he', 'ar', 'cs' ]
		
	for translation in trs:
		ts_templates.append(translation)
		
	UpdateTranslations()

	
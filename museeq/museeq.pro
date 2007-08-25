# daelstorm was here
# Created 2007, March, 20th

CONFIG += qt 
TARGET = museeq
QMAKE_COPY = cp -pf
TEMPLATE = app
DEPENDPATH += Mucipher
INCLUDEPATH += Mucipher
MUSEEQDATA = /share/museek/museeq
OBJECTS_DIR = workdir
DESTDIR     = workdir
translations.path = $${MUSEEQDATA}/translations
translations.files = workdir/translations/museeq_*.qm
# museeq.target = museeq
TARGET.path = /bin
# museeq.commands = touch $$museeq.target
TARGET.files = museeq
# Icons
ICONS_birdies.path = $${MUSEEQDATA}/icons/birdies
ICONS_birdies.files = birdies/*.png
ICONS_default.path = $${MUSEEQDATA}/icons/default
ICONS_default.files = default/*.png
ICONS_bluebox.path = $${MUSEEQDATA}/icons/bluebox
ICONS_bluebox.files = bluebox/*.png
ICONS_mikelabo_silk.path = $${MUSEEQDATA}/icons/mikelabo-silk
ICONS_mikelabo_silk.files = mikelabo-silk/*.png
ICONS_mikelabo_triangles.path = $${MUSEEQDATA}/icons/mikelabo-tri
ICONS_mikelabo_triangles.files = mikelabo-tri/*.png
INSTALLS += ICONS_mikelabo_triangles  ICONS_mikelabo_silk ICONS_bluebox ICONS_default ICONS_birdies
INSTALLS += translations
INSTALLS += TARGET
# Comment the next two lines out, if they cause problems
UNAME = $$system(uname -s)
message( You appear to be running $$UNAME $$system(uname -r) )
# HELP
message('qmake build options are: "CONFIG+=QSA BINRELOC TRAYICON" PREFIX="/path"')
message('Read the INSTALL file more more information.')
###############################################################################
# DEFINES                                                                     #
###############################################################################
isEmpty(PREFIX) {
	PREFIX = /usr
}

BINRELOC {
	DEFINES += ENABLE_BINRELOC=1
}
!BINRELOC { DEFINES += DATADIR=\"\\\"$${PREFIX}$${MUSEEQDATA}\\\"\"
}
QSA {
	DEFINES += HAVE_QSA=1 HAVE_QSA_DIALOG=1 HAVE_QSA_UTIL=1
	LIBS += -lqsa
	scripts.path = $${MUSEEQDATA}
	scripts.files = *.qs
	INSTALLS += scripts
}
TRAYICON {
	DEFINES += HAVE_TRAYICON=1
        SOURCES += trayicon.cpp
	HEADERS += trayicon.h
}
unix {
	DEFINES += HAVE_SYS_UN_H=1
}
###############################################################################


# Input
HEADERS += aclineedit.h \
           banlist.h \
           browser.h \
           browsers.h \
           buddylist.h \
           chatpanel.h \
           chatroom.h \
           chatrooms.h \
           chattext.h \
           chatticker.h \
           codeccombo.h \
           connect.h \
           iconlistbox.h \
           ignorelist.h \
           images.h \
           interestlist.h \
           interestlistitem.h \
           interestlistview.h \
           interests.h \
           ipdialog.h \
           mainwin.h \
           marquee.h \
           museekdriver.h \
           museekmessages.h \
           museeq.h \
           museeqtypes.h \
           onlinealert.h \
           prefix.h \
           privatechat.h \
           privatechats.h \
           recommendsitem.h \
           recommendsview.h \
           roomlist.h \
           roomlistitem.h \
           roomlistview.h \
           search.h \
           searches.h \
           searchfilter.h \
           searchlistview.h \
           settingsdialog.h \
           slskdrag.h \
           system.h \
           tabwidget.h \
           tickerdialog.h \
           transferlistitem.h \
           transferlistview.h \
           transfers.h \
           trustlist.h \
           userinfo.h \
           userinfos.h \
           userlistitem.h \
           userlistview.h \
           usermenu.h \
           usertabwidget.h \
           util.h \
           Mucipher/mucipher.h \
	   
SOURCES += aclineedit.cpp \
           banlist.cpp \
           browser.cpp \
           browsers.cpp \
           buddylist.cpp \
           chatpanel.cpp \
           chatroom.cpp \
           chatrooms.cpp \
           chattext.cpp \
           chatticker.cpp \
           codeccombo.cpp \
           connect.cpp \
           iconlistbox.cpp \
           ignorelist.cpp \
           images.cpp \
           interestlist.cpp \
           interestlistitem.cpp \
           interestlistview.cpp \
           interests.cpp \
           ipdialog.cpp \
           mainwin.cpp \
           marquee.cpp \
           museekdriver.cpp \
           museeq.cpp \
           onlinealert.cpp \
           prefix.c \
           privatechat.cpp \
           privatechats.cpp \
           recommendsitem.cpp \
           recommendsview.cpp \
           roomlist.cpp \
           roomlistitem.cpp \
           roomlistview.cpp \
           search.cpp \
           searches.cpp \
           searchfilter.cpp \
           searchlistview.cpp \
           settingsdialog.cpp \
           slskdrag.cpp \
           tabwidget.cpp \
           tickerdialog.cpp \
           transferlistitem.cpp \
           transferlistview.cpp \
           transfers.cpp \
           trustlist.cpp \
           userinfo.cpp \
           userinfos.cpp \
           userlistitem.cpp \
           userlistview.cpp \
           usermenu.cpp \
           usertabwidget.cpp \
           util.cpp \
           Mucipher/aes.c \
           Mucipher/hexdigest.c \
           Mucipher/md5.c \
           Mucipher/sha.c \
           Mucipher/sha256.c \
           Mucipher/wraphelp.c


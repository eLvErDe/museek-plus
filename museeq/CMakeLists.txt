project(museeq) # the name of your project

MESSAGE("DECLARATIONS: PREFIX (/usr), MANDIR (PREFIX/man), DATADIR (PREFIX/share), BINRELOC, TRAYICON
Prefix declarations with -D and set them with either 1, 0 or a path.
QSA is detected automatically
")


cmake_minimum_required(VERSION 2.4.0)
include(CheckIncludeFiles)
include(CheckTypeSize)
add_subdirectory(Mucipher)
find_package(Qt3 REQUIRED) # find and setup Qt3 for this project

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
add_definitions(${QT_DEFINITIONS} )
# tell cmake where to search for Q headers:
include_directories( ${QT_INCLUDE_DIR})
include_directories(.)
MACRO(ADD_MOC_FILES _sources)
   FOREACH (_current_FILE ${ARGN})

      GET_FILENAME_COMPONENT(_tmp_FILE ${_current_FILE} ABSOLUTE)

      GET_FILENAME_COMPONENT(_basename ${_tmp_FILE} NAME_WE)
      SET(_moc ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc.cpp)

      ADD_CUSTOM_COMMAND(OUTPUT ${_moc}
         COMMAND ${QT_MOC_EXECUTABLE}
         ARGS ${_tmp_FILE} -o ${_moc}
         DEPENDS ${_tmp_FILE}
      )

      SET(${_sources} ${${_sources}} ${_moc})

   ENDFOREACH (_current_FILE)
ENDMACRO(ADD_MOC_FILES)

MACRO(ADD_TRANSLATION_FILES _sources )

    FOREACH (_current_FILE ${ARGN})
      GET_FILENAME_COMPONENT(_in ${_current_FILE} ABSOLUTE)
      GET_FILENAME_COMPONENT(_basename ${_current_FILE} NAME_WE)

      SET(_out ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.qm)

      ADD_CUSTOM_COMMAND(
         OUTPUT ${_out}
         COMMAND lrelease #${QT_LRELEASE_EXECUTABLE}
         ARGS -verbose ${_in} -qm ${_out}
         DEPENDS ${_in}
      )

      SET(${_sources} ${${_sources}} ${_out} )
   ENDFOREACH (_current_FILE)
ENDMACRO(ADD_TRANSLATION_FILES)

IF(PREFIX)
  SET(CMAKE_INSTALL_PREFIX ${PREFIX})
ELSE(PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr")
ENDIF(PREFIX)

IF (NOT DATADIR)
  set(DATADIR ${CMAKE_INSTALL_PREFIX}/share)
ENDIF (NOT DATADIR)
ADD_DEFINITIONS(-DDATADIR=\\\"${DATADIR}\\\")

if (MANDIR)
  set(MANUAL_DIRECTORY ${MANDIR}/man1)
else (MANDIR)
  set(MANUAL_DIRECTORY "man/man1")
endif (MANDIR)

if (BINRELOC)
 ADD_DEFINITIONS(-DENABLE_BINRELOC=1)
endif (BINRELOC)


check_include_files(stdlib.h HAVE_STDLIB_H)
check_include_files(stdio.h HAVE_STDIO_H)
check_include_files(stdarg.h HAVE_STDARG_H)
check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(stddef.h HAVE_STDDEF_H)
check_include_files(unistd.h HAVE_UNISTD_H)
check_include_files(string.h HAVE_STRING_H)
check_include_files(strings.h HAVE_STRINGS_H)
check_include_files(errno.h HAVE_ERRNO_H)
check_include_files(sys/socket.h HAVE_SYS_SOCKET_H)
check_include_files(sys/poll.h HAVE_SYS_POLL_H)
check_include_files(sys/epoll.h HAVE_EPOLL_CTL)
check_include_files(sys/signal.h HAVE_SIGNAL_H)
check_include_files(sys/un.h HAVE_SYS_UN_H)
check_include_files(qsproject.h HAVE_QSA)

# Check for some standard types and type sizes
check_type_size("unsigned char" UNSIGNED_CHAR)
check_type_size("unsigned short" UNSIGNED_SHORT)
check_type_size("unsigned int" UNSIGNED_INT)
check_type_size("unsigned long" UNSIGNED_LONG)
check_type_size("unsigned long long" UNSIGNED_LONG_LONG)
check_type_size("uint" UINT)
check_type_size("uint32" UINT32)

# Determine the type to use as uint32.
if(HAVE_UINT32)
  set(UINT32_TYPE uint32)
elseif(UNSIGNED_CHAR EQUAL 4)
  set(UINT32_TYPE "unsigned char")
elseif(UNSIGNED_SHORT EQUAL 4)
  set(UINT32_TYPE "unsigned short")
elseif(UNSIGNED_INT EQUAL 4)
  set(UINT32_TYPE "unsigned int")
elseif(UNSIGNED_LONG EQUAL 4)
  set(UINT32_TYPE "unsigned long")
elseif(UNSIGNED_LONG_LONG EQUAL 4)
  set(UINT32_TYPE "unsigned long long")
else(UNSIGNED_LONG_LONG EQUAL 4)
  message(FATAL_ERROR "Couldn't determine 32bit unsigned interger type.")
endif(HAVE_UINT32)

set(MUSEEQ_SOURCES
  aclineedit.cpp
  banlist.cpp
  browser.cpp
  browsers.cpp
  buddylist.cpp
  chatpanel.cpp
  chatroom.cpp
  chatrooms.cpp
  chattext.cpp
  chatticker.cpp
  codeccombo.cpp
  connect.cpp
  iconlistbox.cpp
  ignorelist.cpp
  images.cpp
  interestlist.cpp
  interestlistitem.cpp
  interestlistview.cpp
  interests.cpp
  ipdialog.cpp
  mainwin.cpp
  marquee.cpp
  museekdriver.cpp
  museeq.cpp
  onlinealert.cpp
  prefix.c
  privatechat.cpp
  privatechats.cpp
  recommendsitem.cpp
  recommendsview.cpp
  roomlist.cpp
  roomlistitem.cpp
  roomlistview.cpp
  search.cpp
  searches.cpp
  searchfilter.cpp
  searchlistview.cpp
  settingsdialog.cpp
  slskdrag.cpp
  tabwidget.cpp
  tickerdialog.cpp
  transferlistitem.cpp
  transferlistview.cpp
  transfers.cpp
  trustlist.cpp
  userinfo.cpp
  userinfos.cpp
  userlistitem.cpp
  userlistview.cpp
  usermenu.cpp
  usertabwidget.cpp
  util.cpp
)

set(MUSEEQ_MOC_HEADERS
  aclineedit.h
  banlist.h
  browser.h
  browsers.h
  buddylist.h
  chatpanel.h
  chatroom.h
  chatrooms.h
  chattext.h
  chatticker.h
  codeccombo.h
  connect.h
  iconlistbox.h
  ignorelist.h
  interestlist.h
  interestlistview.h
  interests.h
  ipdialog.h
  mainwin.h
  marquee.h
  museekdriver.h
  museeq.h
  onlinealert.h
  privatechat.h
  privatechats.h
  recommendsview.h
  roomlist.h
  roomlistview.h
  search.h
  searches.h
  searchfilter.h
  searchlistview.h
  settingsdialog.h
  slskdrag.h
  tabwidget.h
  tickerdialog.h
  transferlistview.h
  transfers.h
  trustlist.h
  userinfo.h
  userinfos.h
  userlistview.h
  usermenu.h
  usertabwidget.h

)

IF(TRAYICON)
  ADD_DEFINITIONS(-DHAVE_TRAYICON=1)
  set(MUSEEQ_MOC_HEADERS ${MUSEEQ_MOC_HEADERS} trayicon.h)
  set(MUSEEQ_SOURCES ${MUSEEQ_SOURCES} trayicon.cpp)
endif (TRAYICON)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/system.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/system.h)

add_definitions(-DHAVE_SYSTEM_H)

ADD_MOC_FILES(MUSEEQ_SOURCES ${MUSEEQ_MOC_HEADERS})


add_executable(museeq ${MUSEEQ_SOURCES} ${MUSEEQ_MOC_HEADERS})

target_link_libraries(museeq Mucipher ${QT_LIBRARIES} )

install(
  TARGETS museeq
  DESTINATION bin
)
# file(GLOB_RECURSE TRANSLATIONS translations/museeq*.ts)
set(TRANSLATIONS 
  translations/museeq_fr.ts
  translations/museeq_pl.ts
  translations/museeq_it.ts
  translations/museeq_de.ts
  translations/museeq_pt_BR.ts
  translations/museeq_es.ts
  translations/museeq_ro.ts
  translations/museeq_nl.ts
)

ADD_TRANSLATION_FILES (QM_FILES ${TRANSLATIONS})
ADD_CUSTOM_TARGET (translations ALL DEPENDS ${QM_FILES})
install(
  FILES ${QM_FILES}
  DESTINATION ${DATADIR}/museek/museeq/translations
)

file(GLOB_RECURSE THEME_DEFAULT default/*.png)
install(FILES ${THEME_DEFAULT} DESTINATION ${DATADIR}/museek/museeq/default)
file(GLOB_RECURSE THEME_BLUEBOX bluebox/*.png)
install(FILES ${THEME_BLUEBOX} DESTINATION ${DATADIR}/museek/museeq/bluebox)
file(GLOB_RECURSE THEME_SILK mikelabo-silk/*.png)
install(FILES ${THEME_SILK} DESTINATION ${DATADIR}/museek/museeq/mikelabo-silk)
file(GLOB_RECURSE THEME_TRI mikelabo-tri/*.png)
install(FILES ${THEME_TRI} DESTINATION ${DATADIR}/museek/museeq/mikelabo-tri)

set(ICONS
  museeq.png
)

install(
  FILES ${ICONS}
  DESTINATION "share/pixmaps"
)
set(DESKTOP
  museeq.desktop
)

install(
  FILES ${DESKTOP}
  DESTINATION "share/applications"
)
install(
  FILES "museeq.1"
  DESTINATION ${MANUAL_DIRECTORY}
)
MESSAGE("--> Museeq will be installed.. [NO_MUSEEQ=1]")
if (NOT BINRELOC)
  MESSAGE(" * without Binary Relocation.. [BINRELOC=1]")
else (NOT BINRELOC)
  MESSAGE(" * with Binary Relocation.. [BINRELOC=0]")
endif (NOT BINRELOC)
if (NOT TRAYICON)
  MESSAGE(" * without a Trayicon.. [TRAYICON=1]")
else (NOT TRAYICON)
  MESSAGE(" * with a Trayicon.. [TRAYICON=0]")
endif (NOT TRAYICON)
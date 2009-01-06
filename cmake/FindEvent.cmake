# Find Libevent
# http://monkey.org/~provos/libevent/
# http://www.wallinfire.net/picviz/browser/trunk/cmake/FindEvent.cmake?rev=351
#
# Once done, this will define:
#
#  Event_FOUND - system has Event
#  Event_INCLUDE_DIRS - the Event include directories
#  Event_LIBRARIES - link these to use Event
#

if (Event_INCLUDE_DIRS AND Event_LIBRARIES)
  # Already in cache, be silent
  set(Event_FIND_QUIETLY TRUE)
endif (Event_INCLUDE_DIRS AND Event_LIBRARIES)

find_path(Event_INCLUDE_DIRS event.h PATHS ${_EventIncDir} PATH_SUFFIXES event)

find_library(Event_LIBRARIES NAMES event PATHS ${_EventLinkDir})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Event DEFAULT_MSG Event_INCLUDE_DIRS Event_LIBRARIES)

mark_as_advanced(Event_INCLUDE_DIRS Event_LIBRARIES)


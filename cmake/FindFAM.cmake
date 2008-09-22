# - Try to find the fam libraries
# Once done this will define
#
# FAM_FOUND - system supports fam
# FAM_INCLUDE_DIR - the fam include directory
# FAM_LIBRARIES - libfam library

FIND_PATH(FAM_INCLUDE_DIR fam.h )
FIND_LIBRARY(FAM_LIBRARIES NAMES fam )

FIND_LIBRARY(GAMIN_LIBRARIES NAMES gamin gamin-1)
 
IF (NOT GAMIN_LIBRARIES AND FAM_LIBRARIES)
  message (STATUS "Please use Gamin instead of FAM if possible") 
ENDIF (NOT GAMIN_LIBRARIES AND FAM_LIBRARIES)

if (GAMIN_LIBRARIES)
  message(STATUS "Found Gamin: good choice, it's better then FAM")
  set(FAM_LIBRARIES ${GAMIN_LIBRARIES})
endif (GAMIN_LIBRARIES)

IF(FAM_INCLUDE_DIR AND FAM_LIBRARIES)
  SET(FAM_FOUND 1)
  if(NOT FAM_FIND_QUIETLY)
    if (GAMIN_LIBRARIES)
      message(STATUS "Found FAM (provided by Gamin): ${FAM_LIBRARIES}")
    else (GAMIN_LIBRARIES)
      message(STATUS "Found FAM: ${FAM_LIBRARIES}")
    endif (GAMIN_LIBRARIES)
  endif(NOT FAM_FIND_QUIETLY)
ELSE(FAM_INCLUDE_DIR AND FAM_LIBRARIES)
  SET(FAM_FOUND 0 CACHE BOOL "Not found FAM")
  message(STATUS "NOT Found FAM, disabling it")
ENDIF(FAM_INCLUDE_DIR AND FAM_LIBRARIES)

MARK_AS_ADVANCED(FAM_INCLUDE_DIR FAM_LIBRARIES)


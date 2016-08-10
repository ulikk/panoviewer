#
# Try to find GLEW library and include path.
# Once done this will define
#
# GLEW_FOUND
# GLEW_INCLUDE_PATH
# GLEW_LIBRARY
# 

IF (WIN32)
# static
	FIND_PATH( GLEW_INCLUDE_PATH GL/glew.h
		$ENV{PROGRAMFILES}/GLEW/include
		$ENV{GLEW_INCLUDE_DIR}
		$ENV{GLEW_INCLUDE_PATH}	
		${PROJECT_SOURCE_DIR}/src/nvgl/glew/include
		DOC "The directory where GL/glew.h resides")
	FIND_FILE( GLEW_SRC_FILE glew.c
	    PATHS ${GLEW_INCLUDE_PATH}/../src/
		)
	#message(STATUS, "glew src ${GLEW_SRC_FILE}")
	ADD_DEFINITIONS(-DGLEW_STATIC)
	
	#FIND_LIBRARY( GLEW_LIBRARY
#		NAMES glew GLEW glew32 glew32s
#		PATHS
#		$ENV{PROGRAMFILES}/GLEW/lib
#		${PROJECT_SOURCE_DIR}/src/nvgl/glew/bin
#		${PROJECT_SOURCE_DIR}/src/nvgl/glew/lib
#		DOC "The GLEW library")
ELSE (WIN32)
    SET(GLEW_SRC_FILE,"")
	FIND_PATH( GLEW_INCLUDE_PATH GL/glew.h
		PATHS /usr/include
		/usr/local/include
		/sw/include
		/opt/local/include
		DOC "The directory where GL/glew.h resides")
        #message("GLEW include: ${GLEW_INCLUDE_PATH}")
	FIND_LIBRARY( GLEW_LIBRARY
		NAMES GLEW glew
		PATHS
		/usr/lib64
		/usr/lib
		/usr/local/lib64
		/usr/local/lib
		/sw/lib
		/opt/local/lib
		DOC "The GLEW library")
	#message("GLEW lib: ${GLEW_LIBRARY}")
ENDIF (WIN32)

IF (DEFINED GLEW_INCLUDE_PATH)
	SET( GLEW_FOUND 1 CACHE STRING "Set to 1 if GLEW is found, 0 otherwise")
ELSE (DEFINED GLEW_INCLUDE_PATH)
	SET( GLEW_FOUND 0 CACHE STRING "Set to 1 if GLEW is found, 0 otherwise")
ENDIF (DEFINED GLEW_INCLUDE_PATH)

if(GLEW_FOUND)
  message(STATUS "GLEW found!")
else(GLEW_FOUND)
  message(STATUS "GLEW NOT found!")
endif(GLEW_FOUND)

MARK_AS_ADVANCED( GLEW_FOUND )

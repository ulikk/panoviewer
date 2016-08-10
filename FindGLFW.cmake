# Locate the glfw library
# This module defines the following variables:
# GLFW_LIBRARY, the name of the library;
# GLFW_INCLUDE_DIR, where to find glfw include files.
# GLFW_FOUND, true if both the GLFW_LIBRARY and GLFW_INCLUDE_DIR have been found.
#
# To help locate the library and include file, you could define an environment variable called
# GLFW_ROOT which points to the root of the glfw library installation. This is pretty useful
# on a Windows platform.
#
#
# Usage example to compile an "executable" target to the glfw library:
#
# FIND_PACKAGE (glfw REQUIRED)
# INCLUDE_DIRECTORIES (${GLFW_INCLUDE_DIR})
# ADD_EXECUTABLE (executable ${EXECUTABLE_SRCS})
# TARGET_LINK_LIBRARIES (executable ${GLFW_LIBRARY})
#
# TODO:
# Allow the user to select to link to a shared library or to a static library.

#Search for the include file...
FIND_PATH(GLFW_INCLUDE_DIR GLFW/glfw3.h
  DOC "Path to GLFW include directory."
  HINTS $ENV{GLFW_ROOT}
  PATHS /usr/include /usr/local/include /usr/include/GL /usr/local/include/GL ${GLFW_ROOT_DIR}/include
  $ENV{GLFW_ROOT}/include
  PATH_SUFFIXES include 
)

FIND_LIBRARY(GLFW_LIBRARY DOC "Absolute path to GLFW library."
  NAMES libglfw3 glfw3 GLFW.lib
  HINTS $ENV{GLFW_ROOT}
  PATH_SUFFIXES lib/win32 
  PATHS /usr/local/lib /usr/lib ${GLFW_ROOT_DIR}/lib-msvc100/release
)

SET(GLFW_FOUND 0)
IF(GLFW_LIBRARY AND GLFW_INCLUDE_DIR)
  SET(GLFW_FOUND 1)
  message(STATUS "GLFW found!")
ELSE(GLFW_LIBRARY AND GLFW_INCLUDE_DIR)
  message(STATUS "GLFW NOT found!")
ENDIF(GLFW_LIBRARY AND GLFW_INCLUDE_DIR)

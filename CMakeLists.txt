cmake_minimum_required (VERSION 2.8) 
project(PanoViewer)

# look for modules in this dir
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}" ${CMAKE_MODULE_PATH} )

FIND_PACKAGE(OpenGL REQUIRED)
IF(MSVC)
  FIND_PACKAGE(GLEWW REQUIRED)
ELSE(MSVC)
  FIND_PACKAGE(GLEW REQUIRED)
ENDIF(MSVC)

FIND_PACKAGE(glfw3 REQUIRED)

IF(MSVC)
   SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ox /Ob2 /GS- /MP /fp:fast")
   # no expensive range checking
   ADD_DEFINITIONS(-D_SECURE_SCL=0)
   ADD_DEFINITIONS(-D_ITERATOR_DEBUG_LEVEL=0)
   ADD_DEFINITIONS(-DUNICODE)
   ADD_DEFINITIONS(-D_UNICODE)
   ADD_DEFINITIONS(-DGLEW_STATIC)
      
   # suppress MSVC security warnings
   ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
ELSE(MSVC)
   add_definitions(-std=c++0x -Wall -g)
ENDIF(MSVC)

if(UNIX)
  find_package(X11 REQUIRED)
  find_package(Threads REQUIRED)
endif(UNIX)

  # includes
  INCLUDE_DIRECTORIES( "./src/" )
  INCLUDE_DIRECTORIES( ${GLEW_INCLUDE_PATH} )
  INCLUDE_DIRECTORIES( ${GLFW_INCLUDE_DIR} )

  # libjpeg
  ADD_SUBDIRECTORY(libjpeg)
  INCLUDE_DIRECTORIES("./libjpeg")
  include_directories("${CMAKE_CURRENT_BINARY_DIR}/libjpeg")

  SOURCE_GROUP(glew FILES ${GLEW_SRC_FILE})

SET(SRC_PANOVIEWER 
  src/camera.h
  src/glfont.h src/glfont.cpp
  src/image.h src/imgjpg.h src/quaterniont.h src/vec3t.h
  src/pnm.h src/pnm.cpp src/glutil.h src/glutil.cpp
  src/TiledImage.h src/TiledImage.cpp
  src/main.cpp
  )
SOURCE_GROUP(PanoViewer FILES ${SRC_PANOVIEWER})

# WIN32
ADD_EXECUTABLE(PanoViewer 
  ${SRC_PANOVIEWER}
  ${GLEW_SRC_FILE}
)

SET(WINLIBS winmm.lib)
SET(LIBRARIES ${OPENGL_LIBRARIES} ${OPENGL_LIBRARY} ${GLEW_LIBRARY} glfw ${X11_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})

IF(MSVC)
  TARGET_LINK_LIBRARIES(PanoViewer ${LIBRARIES} ${WINLIBS} libjpeg)
ELSE(MSVC)
  TARGET_LINK_LIBRARIES(PanoViewer ${LIBRARIES} libjpeg)
ENDIF(MSVC)

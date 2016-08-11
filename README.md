# PanoViewer #

a small panoramic image viewer for equirectangular projections written
in C++ using OpenGL. Projection is done directly in the fragment shader
on the GPU, and it uses a tiling method to be able to load large images 
that exceed the maximum size of a texture on the graphics card.

## Dependencies ##

PanoViewer depends on GLEW http://glew.sourceforge.net/ and GLFW 3 http://www.glfw.org/ for platform independent OpenGL and libjpeg http://www.ijg.org/ for loading jpg's.

Libjpeg is statically included in the code, just put the content of
jpegsr<x>.zip from the IJG in the "libjpeg" subdirectory before running
CMake

## Usage ##

drag and rop any .jpg file on the panoviewer window to load the image

mouse: 
- hold LMB and move : drag screen
- hold RMB and move : scroll in direction

keyboard:
- H to toggle on screen help text
- W,S to rotate view up/down
- Q,E to de/increase field of view
- C toggle compatibility render mode (shaders on/off)
- SPACE toggle on-screen text

#ifndef _GLFONT_HPP_
#define _GLFONT_HPP_

// simple OpenGL bitmap font
//  Ulrich Krispel        uli@krispel.net

#include <string>
#include <GL/glew.h>
#include "image.h"

namespace GLFONT
{

    struct GLFont
    {
     Image img;
     GLuint textureID;
     // font table, in texture coordinates
     float X[256];
     float Y[256];
     float W[256];
     float H[256];
     float OX[256];
     float OY[256];
     float ascender;
     float descender;

     // needed for glortho mode, update on screen resize!
     int screenw, screenh;
     int lineheight;

     GLFont(const Image &im) : img(im), textureID(-1), lineheight(0) {}
     GLFont() : textureID(-1),lineheight(0) {}

     void initialize();
	 void cleanup();
     void prepareGL() const;
     void drawText(const int left, const int top, const std::string &text) const;
     void finishGL() const;
     // print single line with GL state changes
     void printLine(const int left, const int top, const std::string &text) const;
     bool valid() const;
    };

// namespace functions

  // create a fixed size monospace font out of an image
  // it is assumed that the image contains 16x16 evenly sized characters
  GLFont createMonospacedFont(const Image &img);

  // this should be able to parse the LUA font definition from
  // FontBuilder
  // https://github.com/andryblack/fontbuilder
  GLFont loadVWFFont(std::istream &pnmimage, std::istream &luametadata, int background = -1);
  GLFont loadVWFFont(const std::string &filebase, int background=-1);

} // namespace GLFONT

#endif

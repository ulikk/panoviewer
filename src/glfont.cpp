#include "glfont.h"
#include <cassert>
#include <iostream>
#include <fstream>

#include "pnm.h"

namespace GLFONT
{

bool GLFont::valid() const
{
    return lineheight!=0;
}

void GLFont::initialize()
{
    GLint vp [4];
    glGetIntegerv (GL_VIEWPORT, vp);

    screenw = vp[2];
    screenh = vp[3];
    glGenTextures(1, &textureID );
    glBindTexture(GL_TEXTURE_2D, textureID);

    GLenum format = (img.bpp()==24) ? GL_RGB : GL_RGBA;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width(), img.height(), 0,
                 format, GL_UNSIGNED_BYTE, img.data());
#ifdef _DEBUG
    std::cout << "Font initialized: " << img.width() << "x" << img.height()
              << " " << img.bpp() << "-bit texture used."  << std::endl;
#endif
    glBindTexture(GL_TEXTURE_2D, 0);
    
}

void GLFont::cleanup()
{
    glDeleteTextures(1, &textureID);
}

void GLFont::prepareGL() const
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, screenw, screenh, 0.0f, -1.0f, 1.0f);
    glDisable (GL_DEPTH_TEST);
    if(img.bpp()==32)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    glDisable(GL_LIGHTING);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, textureID);

}

void GLFont::finishGL() const
{
    // clean up
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    glEnable (GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    if(img.bpp()==32)
    {
        glDisable(GL_BLEND);
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindTexture(GL_TEXTURE_2D, 0);

}

void GLFont::drawText(const int left, const int top, const std::string &text) const
{
    std::vector<float> vdata;
    std::vector<float> tcdata;

       float x=(float)left, y=(float)top;
       auto addVertex=[&vdata,&tcdata](const float x, const float y,
                             const float u, const float v)
       {
          vdata.push_back(x);
          vdata.push_back(y);
          vdata.push_back(0);
          tcdata.push_back(u);
          tcdata.push_back(v);
       };

       for (const char& c : text)
       {
          int i=c;
          switch (c)
          {
            case ' ' : x+=5; break;
            case '\n': y+=H[i]; x=(float)left; break;

        default:
        {
            float xl=x;//+OX[i];
            float yl=y+ascender-OY[i];
            float xr=xl+W[i]*(float)img.width();
            float yr=yl+H[i]*(float)img.height();
            // a character consists of two triangles
            addVertex(xl, yl, X[i],     Y[i]);
            addVertex(xr, yl, X[i]+W[i],Y[i]);
            addVertex(xl, yr, X[i]     ,Y[i]+H[i]);

            addVertex(xr, yl, X[i]+W[i],Y[i]);
            addVertex(xr, yr, X[i]+W[i],Y[i]+H[i]);
            addVertex(xl, yr, X[i],     Y[i]+H[i]);

            x+=W[i]*(float)img.width()+1;
        }
        }
    }

    // enable arrays
    glVertexPointer(3, GL_FLOAT, 0, &vdata[0]);
    glTexCoordPointer(2, GL_FLOAT, 0, &tcdata[0]);

    glDrawArrays(GL_TRIANGLES, 0, vdata.size()/3);

#if 0
// debugging: draw font texture on screen
    glBegin(GL_QUADS);
    glTexCoord2f(0,0);
    glVertex2f(100,100);
    glTexCoord2f(0,1);
    glVertex2f(100,100+128);
    glTexCoord2f(1,1);
    glVertex2f(100+128,100+128);
    glTexCoord2f(1,0);
    glVertex2f(100+128,100);
    glEnd();
#endif
}


void GLFont::printLine(const int left, const int top, const std::string &text) const
{
    prepareGL();
    drawText(left,top,text);
    finishGL();
}


GLFont createMonospacedFont(const Image &img)
{
    GLFont font(img);

    float fontwidth=img.width()/16.0f;
    float fontheight=img.height()/16.0f;

    unsigned int ci=0; // character index

    for (unsigned int y=0; y<16; ++y)
        for (unsigned int x=0; x<16; ++x)
        {
            // initialize each letter
            font.X[ci] = x*fontwidth/(float)img.width();
            font.Y[ci] = y*fontwidth/(float)img.height();
            font.W[ci] = fontwidth/(float)img.width();
            font.H[ci] = fontheight/(float)img.height();
            font.OX[ci] = 0;
            font.OY[ci] = 0;
            ++ci;
        }
    font.lineheight = (int) fontheight;

    return font;
}

// this parses the LUA font definition from FontBuilder
// https://github.com/andryblack/fontbuilder

GLFont loadVWFFont(const std::string &filebase, int background)
{
    std::string pnmfile(filebase);
    pnmfile.append(".pnm");

    // parse font metadata
    std::string luafile(filebase);
    luafile.append(".lua");
    std::ifstream luastream(luafile);

    std::ifstream pnmstream(pnmfile, std::ios::binary);
    return loadVWFFont(pnmstream, luastream);

}

GLFont loadVWFFont(std::istream &pnmimage, std::istream &in, int background)
{

    Image img = PNM::loadPNM(pnmimage, background);
    if (!img.isValid())
    {
        std::cout << "ERR: cannot load font bitmap." << std::endl;
        return GLFont();
    }
    GLFont font(img);

    if (in.eof()) {
        std::cout << "ERR: cannot load font metadata " << std::endl;
        return GLFont();
    }


    auto parseKeyValue = [](const std::string &asn) ->
                         std::pair<std::string,std::string>
    {
        size_t n = asn.find("=");
        if (n != std::string::npos)
        {
            return std::pair<std::string,std::string>(
                asn.substr(0,n), asn.substr(n+1)
            );
        }
        return std::pair<std::string,std::string>();
    };

    auto parseInt = [](const std::string &str) -> int
    {
        int value;
        std::istringstream SS(str);
        SS >> value;
        return value;
    };

    struct fontchar
    {
        int x,y,w,h,ox,oy;
        fontchar() : x(0), y(0), w(0), h(0), ox(0), oy(0) {}
    };

    int ascender=0,descender=0;

    while(!in.eof())
    {
        std::string line = PNM::readNextLine(in);
        size_t pos=-1;

        if ( (pos = line.find("ascender")) != std::string::npos )
        {
            auto kv=parseKeyValue(line);
            ascender = parseInt(kv.second);
            font.ascender = ascender / (float)img.height();
        }
        if ( (pos = line.find("descender")) != std::string::npos )
        {
            auto kv=parseKeyValue(line);
            descender = parseInt(kv.second);
            font.descender = descender / (float)img.height();
        }
        if ( (pos = line.find("char=")) != std::string::npos )
        {
            //{char=" ",width=3,x=1,y=14,w=0,h=0,ox=0,oy=0}
            char character = line.at(pos + 6);
            size_t n = line.find("\",", pos);
            fontchar C;
            pos=n+2;
            while(pos < line.length())
            {
                n = line.find(",", pos);
                if (n != std::string::npos)
                {
                    std::string next = line.substr(pos,n-pos);
                    auto kv = parseKeyValue(next);
                    pos=n+1;
                    if (kv.first.compare("x") == 0)
                    {
                        C.x = parseInt(kv.second);
                    }
                    if (kv.first.compare("y") == 0)
                    {
                        C.y = parseInt(kv.second);
                    }
                    if (kv.first.compare("w") == 0)
                    {
                        C.w = parseInt(kv.second);
                    }
                    if (kv.first.compare("h") == 0)
                    {
                        C.h = parseInt(kv.second);
                    }
                    if (kv.first.compare("ox") == 0)
                    {
                        C.ox = parseInt(kv.second);
                    }
                    if (kv.first.compare("oy") == 0)
                    {
                        C.oy = parseInt(kv.second);
                    }
                }
                else
                {
                    pos = line.length();
                }
            }
            if (font.lineheight < C.h)
                font.lineheight=C.h;

            // create character
            int ci=character;
            font.X[ci] = (C.x)/(float)img.width();
            font.Y[ci] = (C.y)/(float)img.height();
            font.W[ci] = (C.w)/(float)img.width();
            font.H[ci] = (C.h)/(float)img.height();
            font.OX[ci] = (float)(C.ox);
            font.OY[ci] = (float)(C.oy);
        }
    }
    return font;
}

} // namespace GLFONT

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// PANOVIEWER
//  Ulrich Krispel        uli@krispel.net

#include <cmath>
#include <string>
#include <vector>

#include "vec3t.h"
#include "quaterniont.h"
#include "camera.h"

#include <sstream>
#include <chrono>
#include <thread>

#include "imgjpg.h"
#include "TiledImage.h"

// font stuff
#include "glfont.h"
#define USE_INTERNAL_FONT
#ifdef USE_INTERNAL_FONT
#include "fontdata.h"
#endif

#include "glutil.h"

#define PI 3.141592653589793238462643
#define PIf 3.14159265358979323846264f

using namespace GLUTIL;

void spherical_to_cartesian(const double theta, const double phi, Vec3d &v) {
  v[0] = sin(theta) * cos(phi);
  v[1] = sin(theta) * sin(phi);
  v[2] = cos(theta);
}

// GLOBALS
GLFWwindow *window;
Camera<double> camera;

// GLuint m_texturename(0xFFFFFFFF);

int m_framecounter;
int m_fps;
int m_timec;

int screenw, screenh;
double fov_azimuth = 360.0, fov_elevation = 180.0;

GLint maxTexSize;
GLint iUnits;

// NAVIGATION
enum NavigationMode { NAV_NONE = 0, 
	                  NAV_DRAG,			// LMB: drag screen
	                  NAV_FLY };		// RMB: scroll screen

Vec2d m_mousepos;   // current mouse position
Vec2d m_mousepress; // position where mouse button was pressed
NavigationMode m_navmode = NAV_NONE;

// DISPLAY
bool m_showText = true, m_showHelp = false;

// PROGRAM MODES
bool f_compatibilityMode = false;

// SHADER VARIABLES

GLuint m_glslprogram;
GLuint m_vertexshader;
GLuint m_fragmentshader;

GLint unLocTileBoundary;
GLint unTex;

// Vertex Shader
// calculates the screen position for the fragment shader (gl_Position)
// plus states the world coordinate for each vertex as varying (position)
// so the world coordinate becomes accessible in the fragment shader
const std::string m_glsl_vertexshadersrc =
    "  uniform mat4 gl_ModelViewMatrix;"
    "  uniform mat4 gl_ProjectionMatrix;"
    "  varying vec3 position;"
    "  void main() {"
    "     gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;"
    "     position = vec3(gl_Vertex.x, gl_Vertex.y, gl_Vertex.z);"
    "   }";

// Fragment Shader
// using the fragment world coordinate (position) the intersection with the unit
// sphere is calculated (simply by normalizing)
// from this position on the sphere (spherepos) the texture coordinates are
// calculated (spherecoords)
// using equirectangular projection (conversion from spherical to cartesian to
// coordinates)
// x = phi = atan2(y,x) normalized to [0,1]
// y = theta = arccos(z/sqrt(x^2+y^2+z^2)) normalized to [-1,1], norm can be
// neglected
//                                         since (x,y,z) is already normalized
//
std::string m_glsl_fragmentshadersrc =
    " const float pi = 3.141592653589793;"
    " varying vec3 position;"
    " uniform sampler2D tex;"
    " uniform vec4 tileboundary;"
    "   void main() {"
    "   vec2 spherecoords;"
    "   vec3 spherepos = normalize(position);"
    "   spherecoords.x = (atan(spherepos.y, -spherepos.x)+pi)/(2.0*pi);"
    "   spherecoords.y = acos(spherepos.z)/(pi);"
    "   if ( (spherecoords.x >= tileboundary.x) && (spherecoords.x <= "
    "tileboundary.y) && "
    "        (spherecoords.y >= tileboundary.z) && (spherecoords.y <= "
    "tileboundary.w) ) { "
    "           vec2 texpos;"
    "           texpos.x = "
    "(spherecoords.x-tileboundary.x)/(tileboundary.y-tileboundary.x); "
    "           texpos.y = "
    "(spherecoords.y-tileboundary.z)/(tileboundary.w-tileboundary.z); "
    "           gl_FragColor = texture2D(tex,texpos);"
    //"           gl_FragColor = vec4(1.0,0.0,0.0,1.0);"
    //"           gl_FragColor = vec4(position.x,position.y,position.z,1.0);"
    "   } else { "
    "     discard; "
    "   }"

    //"           gl_FragColor = vec4(1.0,0.0,0.0,1.0);"
    " }";

/*
" const float pi = 3.141592653589793; \
varying vec3 position;\
uniform sampler2D tex; \
void main() {\
vec2 spherecoords;\
vec3 spherepos = normalize(position);\
spherecoords.x = -atan(spherepos.y,spherepos.x)/(2.0*pi);\
spherecoords.y = acos(spherepos.z)/pi;\
gl_FragColor = texture2D(tex,spherecoords);\
}")
*/

// gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
// discard;

TiledImage panodata;

GLFONT::GLFont font;

std::string m_image_path;

void cleanup() {
  // if (m_texturename != 0xFFFFFFFF)
  //{
  //   glDeleteTextures(1, &m_texturename);
  //}
}

void printProgramInfoLog(GLuint obj) {
  int infologLength = 0;
  int charsWritten = 0;
  glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
  if (infologLength > 1) {
    std::vector<char> infoLog;
    infoLog.resize(infologLength);
    glGetProgramInfoLog(obj, infologLength, &charsWritten, &infoLog[0]);
    std::cout << "program info log:" << std::endl;
    std::cout << (char *)&infoLog[0] << std::endl;
  } else {
    // std::cout << "Program Info Log: OK" << std::endl;
  }
}

// in case that no texture has been delivered - create dummy texture
void generateDummyTexture(ImageT<unsigned char> &img) {
  const int dt_width = 512;
  const int dt_height = 512;
  const int dt_gridspacing = 32;

  img.resize(dt_width, dt_height);
  img.clear();

  // create grid structure
  // vertical lines
  for (unsigned int x = 0; x < dt_width; x += dt_gridspacing) {
    for (unsigned int y = 0; y < dt_width; y++) {
      // write white pixel
      img(x, y, 0) = 0xFF;
      img(x, y, 1) = 0xFF;
      img(x, y, 2) = 0xFF;
    }
  }
  // horizontal lines
  for (unsigned int y = 0; y < dt_width; y += dt_gridspacing) {
    for (unsigned int x = 0; x < dt_width; x++) {
      // write white pixel
      img(x, y, 0) = 0xFF;
      img(x, y, 1) = 0xFF;
      img(x, y, 2) = 0xFF;
    }
  }
}

bool setupGL() {
  checkGLError("enter setupGL");
  m_fps = -1;

  if (glewInit() != GLEW_OK) {
    return false;
  }

  int gl_major, gl_minor;
  glGetIntegerv(GL_MAJOR_VERSION, &gl_major);
  glGetIntegerv(GL_MINOR_VERSION, &gl_minor);
  std::cout << "OpenGL reports version " << gl_major << "." << gl_minor << std::endl;

  if (!f_compatibilityMode) {
    if (glewIsSupported("glCreateShader")) {
      std::cout << "WARNING: No shader support, using compatibility mode!"
                << std::endl;
      f_compatibilityMode = false;
    }
  }

  std::cout << "Compatibility mode: " << (f_compatibilityMode ? "ON" : "OFF")
            << std::endl;

  checkGLError("init");
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
  checkGLError("clear color");
  glClearDepth(1.0f); // Depth Buffer Setup
  checkGLError("clear depth");
  glDisable(GL_DEPTH_TEST); // Enables Depth Testing
  checkGLError("GL_DEPTH_TEST");
  glDepthFunc(GL_LEQUAL); // The Type Of Depth Testing To Do
  checkGLError("GL_LEQUAL");
  glHint(GL_PERSPECTIVE_CORRECTION_HINT,
         GL_NICEST); // Really Nice Perspective Calculations
  checkGLError("glHinth");

  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
  panodata.setTileSize(2048);
  glGetIntegerv(GL_MAX_TEXTURE_UNITS, &iUnits);
  {
    // std::ostringstream os;
    // os << "Card has " << iUnits << " texture units and a maximum texture size
    // of " << maxTexSize << " pixels." << std::endl;;
    // std::cout << os.str();
  }

// load font
#ifdef USE_INTERNAL_FONT
  {
    struct membuf : std::streambuf {
      membuf(char const *base, size_t size) {
        char *p(const_cast<char *>(base));
        this->setg(p, p, p + size);
      }
    };
    struct imemstream : virtual membuf, std::istream {
      imemstream(char const *base, size_t size)
          : membuf(base, size),
            std::istream(static_cast<std::streambuf *>(this)) {}
    };
    imemstream pnmstream((const char *)FONTDATA::FONT_PNM, FONT_PNM_SIZE);
    imemstream luastream((const char *)FONTDATA::FONT_LUA, FONT_LUA_SIZE);
    font = GLFONT::loadVWFFont(pnmstream, luastream, 0x808080);
  }
#else
  font = GLFONT::loadVWFFont("font", 0x000000FF);
#endif

  if (font.valid()) {
    font.initialize();
  }

  // load pano and register textures
  panodata.loadFromJPEG(m_image_path);
  if (!panodata.isValid()) {
    panodata.generateDummyTexture();
  }

  {
    std::cout << "Image size is " << panodata.width() << "x"
              << panodata.height() << " OpenGL reports maximum texture size of "
              << maxTexSize << "px." << std::endl;
    std::cout << "Tile size is " << panodata.getTileSize() << ", using "
              << panodata.numTilesX() << "x" << panodata.numTilesY()
              << " tiles." << std::endl;
  }

  // select modulate to mix texture with color for shading
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  //// when texture area is small, bilinear filter the closest mipmap
  // glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  //// when texture area is large, bilinear filter the original
  // glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

  //// the texture wraps over at the edges (repeat)
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // initialize rotation position
  checkGLError("Initializing stage");



  if (!f_compatibilityMode) {
    // compile the shaders
    m_vertexshader = glCreateShader(GL_VERTEX_SHADER);
    checkGLError("creating vertex shader");
    m_fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
    checkGLError("creating fragment shader");

    const char *vs = m_glsl_vertexshadersrc.c_str();
    const char *fs = m_glsl_fragmentshadersrc.c_str();
    glShaderSource(m_vertexshader, 1, &vs, NULL);
    checkGLError("loading vertex shader");
    glShaderSource(m_fragmentshader, 1, &fs, NULL);
    checkGLError("loading fragment shader");

    glCompileShader(m_vertexshader);
    checkGLError("compiling vertex shader");
    glCompileShader(m_fragmentshader);
    checkGLError("compiling fragment shader");
    m_glslprogram = glCreateProgram();
    checkGLError("create glsl program");

    glAttachShader(m_glslprogram, m_vertexshader);
    checkGLError("attach vertex shader");
    glAttachShader(m_glslprogram, m_fragmentshader);
    checkGLError("attach vertex shader");

    glLinkProgram(m_glslprogram);
    checkGLError("link program");
    printProgramInfoLog(m_glslprogram);

    int param = 0;
    glGetProgramiv(m_glslprogram, GL_LINK_STATUS, &param);
    if (param == GL_TRUE) {

      glUseProgram(m_glslprogram);
      checkGLError("use program");

      unLocTileBoundary = glGetUniformLocation(m_glslprogram, "tileboundary");
      checkGLError("get uniform tileboundary location");
      unTex = glGetUniformLocation(m_glslprogram, "tex");
      checkGLError("get uniform texture");
    } else {
      std::cout << "problem linking shaders, enabling compatibility mode"
                << std::endl;

      // disable shaders
      f_compatibilityMode = true;
    }
  }

  glDisable(GL_DEPTH_TEST);
  glPolygonMode(GL_FRONT, GL_FILL);

  checkGLError("exit setupGL");

  return true;
}

void setupViewport(const int left, const int top, const int width,
                   const int height) {
  glViewport(left, top, width, height);
  camera.changeViewport(Camera<double>::ViewPort(
      (double)left, (double)top, (double)left + width, (double)top + height));
}

bool draw() {
  checkGLError("enter draw");

  glClear(GL_COLOR_BUFFER_BIT); // Clear The Screen
  glDisable(GL_DEPTH_TEST);

  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixd(camera.world2cam());
  checkGLError("set modelview");

  glMatrixMode(GL_PROJECTION); // Select The Projection Matrix
  glLoadMatrixd(camera.getProjection());

  if (f_compatibilityMode) {

    glEnable(GL_BLEND);
    checkGLError("enable blend");
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    checkGLError("set blending");
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);

    // patch coordinates
    Vec3d lu, ru, ld, rd; // left up, right up, left down, right down

    // compatibility mode: just draw quadratic patches on a sphere
    const int SPHERESAMPLING = 30;
    glColor3f(1.0f, 1.0f, 1.0f);

    // manually calculate sphere patches and equirectangular texture coords
    for (int ty = 0, tym = panodata.numTilesY(); ty < tym; ++ty) {
      for (int tx = 0, txm = panodata.numTilesX(); tx < txm; ++tx) {
        int texname = panodata.getTile(tx, ty);
        glBindTexture(GL_TEXTURE_2D, texname);
        checkGLError("bind tile texture");
        float tilexmin, tilexmax, tileymin, tileymax;
        panodata.getNormalizedTileCoordinates(tx, ty, tilexmin, tilexmax,
                                              tileymin, tileymax);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                        GL_CLAMP_TO_BORDER); // GL_CLAMP_TO_BORDER
        // checkGLError();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        // checkGLError();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // checkGLError();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        // checkGLError();
        const float color[] = {0.0f, 0.0f, 0.0f, 0.0f};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
        // checkGLError();

        glBegin(GL_QUADS); // Start Drawing Quads

        for (int i_theta = 0; i_theta < SPHERESAMPLING; i_theta++) {
          for (int i_phi = 0; i_phi < SPHERESAMPLING; i_phi++) {
            const double theta = (double)i_theta / (SPHERESAMPLING);
            const double theta1 = (double)(i_theta + 1) / SPHERESAMPLING;
            const double phi = (double)i_phi / SPHERESAMPLING;
            const double phi1 = (double)(i_phi + 1) / SPHERESAMPLING;

            // calculate patch points
            spherical_to_cartesian(PI * theta, 2.0 * PI * phi - PI, lu);
            spherical_to_cartesian(PI * theta, 2.0 * PI * phi1 - PI, ru);
            spherical_to_cartesian(PI * theta1, 2.0 * PI * phi - PI, ld);
            spherical_to_cartesian(PI * theta1, 2.0 * PI * phi1 - PI, rd);

            //
            double tcx0 = (phi - tilexmin) / (tilexmax - tilexmin);
            double tcx1 = (phi1 - tilexmin) / (tilexmax - tilexmin);
            double tcy0 = (theta - tileymin) / (tileymax - tileymin);
            double tcy1 = (theta1 - tileymin) / (tileymax - tileymin);

            glTexCoord2d(tcx0, tcy0);
            glVertex3dv(lu);
            glTexCoord2d(tcx0, tcy1);
            glVertex3dv(ld);
            glTexCoord2d(tcx1, tcy1);
            glVertex3dv(rd);
            glTexCoord2d(tcx1, tcy0);
            glVertex3dv(ru);
          }
        }
        glEnd();
        checkGLError("end quads");
      }
    }

  } else {
    glDisable(GL_BLEND);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    const Camera<double>::ViewFrustum VF = camera.getViewFrustum();

    checkGLError("enter fast rendering");

    const Vec3d QuadWorld[4] = {
        camera.cam2world(Vec3d(VF.max[0], VF.min[1], -VF.min[2] - 0.0001)),
        camera.cam2world(Vec3d(VF.max[0], VF.max[1], -VF.min[2] - 0.0001)),
        camera.cam2world(Vec3d(VF.min[0], VF.max[1], -VF.min[2] - 0.0001)),
        camera.cam2world(Vec3d(VF.min[0], VF.min[1], -VF.min[2] - 0.0001))};
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);

    glUseProgram(m_glslprogram);
    checkGLError("use shader");

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(unTex, 0);
    checkGLError("set texture");

    glEnableClientState(GL_VERTEX_ARRAY);
    checkGLError("set vertexarray");
    glVertexPointer(3, GL_DOUBLE, 0, QuadWorld);
    checkGLError("set vertexpointer");

    float xmin, xmax, ymin, ymax;
    for (int ty = 0, tym = panodata.numTilesY(); ty < tym; ++ty) {
      for (int tx = 0, txm = panodata.numTilesX(); tx < txm; ++tx) {
        // activate tile
        int texname = panodata.getTile(tx, ty);
        glBindTexture(GL_TEXTURE_2D, texname);
        checkGLError("activate tile texture");

        // shader needs to know the tile position on the sphere
        panodata.getNormalizedTileCoordinates(tx, ty, xmin, xmax, ymin, ymax);
        // std::cout << " tx: " << tx << " ty: " << ty << " texname: " <<
        // texname << " xmin: " << xmin << " xmax: " << xmax << " ymin:" << ymin
        // << " ymax: " << ymax << std::endl;
        glUniform4f(unLocTileBoundary, xmin, xmax, ymin, ymax);
        checkGLError("set tile boundary");

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        checkGLError("draw arrays");
      }
    }

    glUseProgram(0);
  }

#if 0
      // bind texture
      
      glBindTexture( GL_TEXTURE_2D, m_texturename );

      // patch coordinates
      Vec3d lu, ru, ld, rd;		// left up, right up, left down, right down

      if (f_useShader == false)
      {
         // "cheaty version"
         // create quadratic patches on a sphere
         const int SPHERESAMPLING = 30;
         // manually calculate sphere patches and equirectangular texture coords
         glColor3f(1.0f,1.0f,1.0f);
         for (int i_theta = 0; i_theta < SPHERESAMPLING/2; i_theta++)
         {
            for (int i_phi = 0; i_phi < SPHERESAMPLING; i_phi++) 
            {
               // calculate patch points
               spherical_to_cartesian(2.0*PI*((double)i_theta)/SPHERESAMPLING,2.0*PI*i_phi/SPHERESAMPLING,lu);
               spherical_to_cartesian(2.0*PI*((double)i_theta)/SPHERESAMPLING,2.0*PI*(i_phi+1)/SPHERESAMPLING,ru);
               spherical_to_cartesian(2.0*PI*((double)(i_theta+1))/SPHERESAMPLING,2.0*PI*i_phi/SPHERESAMPLING,ld);
               spherical_to_cartesian(2.0*PI*((double)(i_theta+1))/SPHERESAMPLING,2.0*PI*(i_phi+1)/SPHERESAMPLING,rd);

               glBegin(GL_QUADS);					// Start Drawing Quads
                 glTexCoord2d((double)i_phi/SPHERESAMPLING,(double)i_theta/(SPHERESAMPLING/2)); glVertex3d(lu.x,lu.y,lu.z);
                 glTexCoord2d((double)i_phi/SPHERESAMPLING,(double)(i_theta+1)/(SPHERESAMPLING/2)); glVertex3d(ld.x,ld.y,ld.z);
                 glTexCoord2d((double)(i_phi+1)/SPHERESAMPLING,(double)(i_theta+1)/(SPHERESAMPLING/2)); glVertex3d(rd.x,rd.y,rd.z);
                 glTexCoord2d((double)(i_phi+1)/SPHERESAMPLING,(double)i_theta/(SPHERESAMPLING/2)); glVertex3d(ru.x,ru.y,ru.z);
               glEnd();
            }
         }
      } else {

         // "SHADER" version
         // calculate near view plane coordinates in world coordinates and render one quad
         // assuming the camera is always set at the origin then each fragment world coordinate corresponds
         // to a viewing ray, and the normalized coordinate corresponds to the position/intersection
         // on the unit sphere

         glPolygonMode( GL_FRONT, GL_FILL );
         //glDisable( GL_TEXTURE_2D );
         glColor3f(0.8f,0.9f,1.0f);
         glBegin(GL_QUADS);
            Vec3d p,q;

            p.assign(m_xmax, m_ymin, -m_zNear);
            q = m_rotation * p;
            glVertex3d(q.x,q.y,q.z);

            p.assign(m_xmax, m_ymax, -m_zNear);
            q = m_rotation * p;
            glVertex3d(q.x,q.y,q.z);
        
            p.assign(m_xmin, m_ymax, -m_zNear);
            q = m_rotation * p;
            glVertex3d(q.x,q.y,q.z);

            p.assign(m_xmin, m_ymin, -m_zNear);
            q = m_rotation * p;
            glVertex3d(q.x,q.y,q.z);
         glEnd();
      }
#endif
  m_framecounter++;
  typedef std::chrono::duration<double> dsec;
  static std::chrono::time_point<std::chrono::high_resolution_clock> lastT =
      std::chrono::high_resolution_clock::now();

  auto T = std::chrono::high_resolution_clock::now();
  dsec dt = T - lastT;

  if (dt.count() > 1.0) {
    m_fps = m_framecounter;
    m_framecounter = 0;
    lastT = T;
  }

  // ON SCREEN FONT DISPLAY
  if ((m_fps >= 0) && font.valid() && m_showText) {
    std::ostringstream os;
    os << "FPS:" << m_fps;

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    int x = 10;
    int y = 35;

    auto printLine = [&x, &y](const std::string &text) {
      font.drawText(x, y, text);
      y += font.lineheight;
    };

    font.prepareGL();
    printLine(os.str());
    if (m_showHelp) {
      printLine("W, S :  rotate view up/down");
      printLine("A, D : rotate view left/right");
      printLine("Q, E : de-/increase field of view (zoom)");
      printLine("H : en-/disable help text");
      std::string s("C : en-/disable compatibility render mode (");
      if (f_compatibilityMode)
        s.append("ON)");
      else
        s.append("OFF)");
      printLine(s);
      printLine("SPACE: en-/disable all on-screen text");
    } else {
      printLine("press 'h' for help.");
    }
    font.finishGL();
    checkGLError("after font print");
  }

  if (m_navmode == NAV_FLY) {
    Vec2d delta = (m_mousepress - m_mousepos);
    delta[0] /= screenw * 25.0;
    delta[1] /= screenh * 25.0;
    camera.pitch((double)delta[1]);
    camera.rotateAxis(Vec3d(0, 0, 1), (double)delta[0]);
  }
  return true;
}

void onKeyPress(GLFWwindow *w, int key, int scancode, int action, int mods) {
  // rotate up/down
  Quaterniond rot;
  switch (action) {
  // case GLFW_PRESS:
  //    {
  //    };
  //    break;
  case GLFW_PRESS:
  case GLFW_REPEAT: {
    switch (key) {
    // up/down
    case GLFW_KEY_W: {
      camera.pitch(PIf / 10.0f);
      break;
    }
    case GLFW_KEY_S: {
      camera.pitch(-PIf / 10.0f);
      break;
    }
    // left/right, rotate around upvector
    case GLFW_KEY_A: {
      camera.rotateAxis(Vec3d(0, 0, 1), 0.1);
      break;
    }
    case GLFW_KEY_D: {
      camera.rotateAxis(Vec3d(0, 0, 1), -0.1);
      break;
    }
    // change fov
    case GLFW_KEY_Q: {
      camera.changeFOV((float)camera.getFOV() - 1.0f);
      break;
    }
    case GLFW_KEY_E: {
      camera.changeFOV((float)camera.getFOV() + 1.0f);
      break;
    }
    // onscreen text
    case GLFW_KEY_SPACE: {
      m_showText = !m_showText;
      break;
    }
    case GLFW_KEY_H: {
      m_showHelp = !m_showHelp;
      break;
    }
    case GLFW_KEY_C: {
      f_compatibilityMode = !f_compatibilityMode;
      setupGL();
      break;
    }
    default:
      // nope
      break;
    }
  } break;
  }
}

void onMouseMove(GLFWwindow *w, double x, double y) {

  // const double dx = ((double)x - m_mouse_position_x) / m_windowwidth;
  // const double dy = ((double)y - m_mouse_position_y) / m_windowheight;
  switch (m_navmode) {
  case NAV_DRAG: {
    Vec2d delta = (Vec2d(x, y) - m_mousepos);
    delta[0] /= (double)screenw;
    delta[1] /= (double)screenh;
    m_mousepos.assign(x, y);
    camera.pitch((float)delta[1] * (2.6 / 360.0 * PIf * camera.getFOV()));
    camera.rotateAxis(Vec3d(0, 0, 1), delta[0] * (2.6 / 360.0 * PIf * camera.getFOV() ));
  } break;
  case NAV_NONE:
  default:
    break;
  }
  m_mousepos.assign(x, y);
}

void onMouseButton(GLFWwindow *w, int button, int action, int mods) {
  switch (action) {
  case GLFW_PRESS: {
    m_mousepress = m_mousepos;
    switch (button) {
    case GLFW_MOUSE_BUTTON_LEFT:
      m_navmode = NAV_DRAG;
      break;
    case GLFW_MOUSE_BUTTON_RIGHT:
      m_navmode = NAV_FLY;
      break;
    }
  } break;

  case GLFW_RELEASE: {
    m_navmode = NAV_NONE;
  } break;
  }
}

void onFileDragDrop(GLFWwindow *win, int count, const char **files) {
  m_image_path = files[0];
  setupGL();
}

void resizeCB(GLFWwindow *wnd, int x, int y) {
	std::cout << "new window size: " << x << "x" << y << std::endl;
  screenw = x;
  screenh = y;
  font.screenw = x;
  font.screenh = y;
  setupViewport(0, 0, x, y);
}

void Main_Loop(void) {
  // this just loops as long as the program runs
  while (!glfwWindowShouldClose(window)) {
    // escape to quit, arrow keys to rotate view
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      break;

    draw();

    // swap back and front buffers
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}



void Shut_Down(int return_code) {
  glfwTerminate();
  exit(return_code);
}

void Init() {
  if (glfwInit() != GL_TRUE)
    Shut_Down(1);
  screenw = 640;
  screenh = 480;

  {
    std::ostringstream ss;
    ss << "PanoViewer by uk - ";
    ss << __TIMESTAMP__;
    window = glfwCreateWindow(screenw, screenh, ss.str().c_str(), NULL, NULL);
  }
  if (!window) {
    std::cout << "could not open glfw window" << std::endl;
    Shut_Down(1);
  }

  camera.setupPinholeCamera(Camera<double>::ViewPort(0, 0, screenw, screenh),
                            0.1, 1.0, 45.0);

  camera.setPosition(Vec3d(0.0f, 0.0f, 0.0f));
  // default orientation: camera looks into Y world coordinate (Z up)
  camera.setOrientation(Vec3d(1, 0, 0), Vec3d(0, 0, 1), Vec3d(0, -1, 0));

  glfwMakeContextCurrent(window);
  glfwSetWindowSizeCallback(window, &resizeCB);
  glfwSetCursorPosCallback(window, onMouseMove);
  glfwSetMouseButtonCallback(window, onMouseButton);
  glfwSetKeyCallback(window, onKeyPress);
  glfwSetDropCallback(window, onFileDragDrop);

  resizeCB(window, screenw, screenh);
  if (glewInit() != GLEW_OK) {
    std::cout << "could not initialize GLEW" << std::endl;
    Shut_Down(1);
  }
  setupGL();
}

int main(int argc, char *argv[]) {
  if (argc >= 2) {
    m_image_path = argv[1];
  }
  Init();
  Main_Loop();
  if (window)
    glfwDestroyWindow(window);
  Shut_Down(0);
}

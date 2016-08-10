#include <fstream>

#include "TiledImage.h"
#include "imgjpg.h"


void TiledImage::cleanup() {
  // reset to default values;
  m_azimuth = 360.0;
  m_elevation = 180.0;
  if (m_tiles.width() > 0) {
    glDeleteTextures((GLsizei)m_tiles.getData().size(), m_tiles.data());
    m_tiles.clear();
  }
}

TiledImage::~TiledImage() { cleanup(); }

bool TiledImage::loadFromJPEG(std::string filename) {

  // load image data
  if (IMG::loadJPEG<Image>(filename.c_str(), base)) {
    generateTiles();

    IMG::EXIF::EXIFTAGS exiftags =
        IMG::EXIF::parseExif<IMG::EXIF::EXIFTAGS>(filename);
    if (exiftags.find(IMG::EXIF::UserComment) != exiftags.end()) {
      const std::string &UC = exiftags[IMG::EXIF::UserComment];
      // try to parse FOV from hugin-style comment
      std::string token =
          UC.substr(UC.find_first_of("FOV") + 4, UC.find_first_of("Ev") - 4);
      std::sscanf(token.c_str(), "%lf x %lf", &m_azimuth, &m_elevation);
      std::cout << "found field of view, azimuth:" << m_azimuth
                << " elevation:" << m_elevation << std::endl;
    }
    return true;
  }
  return false;
}

void TiledImage::generateTiles() {
  // calculate number of tiles
  int horizontalTiles = (int)ceil((double)base.width() / (double)tileSize);
  int verticalTiles = (int)ceil((double)base.height() / (double)tileSize);

  // clear opengl data
  if (!m_tiles.getData().empty()) {
    cleanup();
  }

  m_tiles.resize(horizontalTiles, verticalTiles, 1);

  // register OpenGL tiles
  // std::vector<unsigned char> v_texdata( tileSize*tileSize );
  glEnable(GL_TEXTURE_2D);
  glGenTextures((GLsizei)m_tiles.getData().size(), (GLuint *)m_tiles.data());

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  for (int ty = 0; ty < verticalTiles; ++ty) {
    int tileHeight = getTileHeight(ty);
    for (int tx = 0; tx < horizontalTiles; ++tx) {
      int tileWidth = getTileWidth(tx);
      ImageT<unsigned char> texData;
      texData.resize(tileWidth, tileHeight, 3);
      // copy tile content, *very* inefficient, should be done by memcpy rows
      for (int y = 0; y < tileHeight; ++y) {
        for (int x = 0; x < tileWidth; ++x) {
          texData(x, y, 0) = base(tx * tileSize + x, ty * tileSize + y, 0);
          texData(x, y, 1) = base(tx * tileSize + x, ty * tileSize + y, 1);
          texData(x, y, 2) = base(tx * tileSize + x, ty * tileSize + y, 2);
        }
      }
      // load texture to opengl
      GLuint texname = getTile(tx, ty);
      glBindTexture(GL_TEXTURE_2D, texname);

      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      // checkGLError();
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      // checkGLError();
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tileWidth, tileHeight, 0, GL_RGB,
                   GL_UNSIGNED_BYTE, texData.data());
    }
  }
}

// in case that no texture has been delivered - create dummy texture
void TiledImage::generateDummyTexture() {
  const int dt_width = 512;
  const int dt_height = 512;
  const int dt_gridspacing = 32;

  base.resize(dt_width, dt_height);
  base.clear();

  // create grid structure
  // vertical lines
  for (unsigned int x = 0; x < dt_width; x += dt_gridspacing) {
    for (unsigned int y = 0; y < dt_width; y++) {
      // write white pixel
      base(x, y, 0) = 0xFF;
      base(x, y, 1) = 0xFF;
      base(x, y, 2) = 0xFF;
    }
  }
  // horizontal lines
  for (unsigned int y = 0; y < dt_width; y += dt_gridspacing) {
    for (unsigned int x = 0; x < dt_width; x++) {
      // write white pixel
      base(x, y, 0) = 0xFF;
      base(x, y, 1) = 0xFF;
      base(x, y, 2) = 0xFF;
    }
  }
  generateTiles();
}

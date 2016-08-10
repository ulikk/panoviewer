//
// TiledImage - image represented by tiled opengl textures
//

#define USE_JPEG

#include "image.h"
#include <GL/glew.h>
#include "vec3t.h"

//  Ulrich Krispel        uli@krispel.net
//
// The Image is loaded into Memory and tiled to OpenGL Textures
//

class TiledImage {
  Image base;
  int tileSize;
  ImageT<GLuint> m_tiles;
  double m_azimuth, m_elevation;

public:
  TiledImage(const int tSize = 1024) : tileSize(tSize){};
  ~TiledImage();

  inline bool isValid() const { return base.isValid(); }
  inline int width() const { return base.width(); }
  inline int height() const { return base.height(); }

  inline void setTileSize(int tsize) { tileSize = tsize; }

  inline int getTileSize() const { return tileSize; }

  inline GLuint getTile(const int x, const int y) const {
    return m_tiles(x, y);
  }
  inline int getTileWidth(const int x) const {
    int width =
        (x == (m_tiles.width() - 1)) ? (base.width() % tileSize) : tileSize;
    return (width == 0) ? tileSize : width;
  }
  inline int getTileHeight(const int y) const {
    int height =
        (y == (m_tiles.height() - 1)) ? (base.height() % tileSize) : tileSize;
    return (height == 0) ? tileSize : height;
  }
  inline int numTilesX() const { return m_tiles.width(); }
  inline int numTilesY() const { return m_tiles.height(); }

  void generateTiles();
  void generateDummyTexture();
  void cleanup();

  bool loadFromJPEG(std::string filename);
  inline void getNormalizedTileCoordinates(const int tx, const int ty,
                                           float &xmin, float &xmax,
                                           float &ymin, float &ymax) {
    xmin = ((float)tx * tileSize);
    xmax = xmin + (float)getTileWidth(tx);
    ymin = ((float)ty * tileSize);
    ymax = ymin + (float)getTileHeight(ty);
    // normalize
    xmin /= base.width(); // xmin -= 0.5f;
    xmax /= base.width(); // xmax -= 0.5f;
    ymin /= base.height();
    ymax /= base.height();
  }
};

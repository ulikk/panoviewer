#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <GL/glew.h>
#include <cmath>
#include <vec3t.h>
#include <matrix.h>
#include <tuple>

// header only library
template <class T>
class Camera
{
typedef MathVector<T,4> Vec4;
typedef MathVector<T,3> Vec3;
typedef MathVector<T,2> Vec2;

// A simple Camera utility class for opengl
//  Ulrich Krispel        uli@krispel.net
// The camera coordinate system is the same as in OpenGL:
// X: right, Y: up, -Z: view
//
// transformation from camera to world coordinates:
//
// [wx]          [x_x]      [y_x]      [z_x]             [cx]
// [wy] = O + cx*[x_y] + cy*[y_y] + cz*[z_y] = [X|Y|Z] * [cy] + O
// [wz]          [x_z]      [y_z]      [z_z]             [cz]
//                X        Y        Z          A
//
// with O being the camera origin and [x][y][z] being three
// normalized orthogonal axis directions in world coordinates.
// Therefore, the transformation matrix A consists of the three
// axis vectors as columns.

public:

// View Frustum class, used for pinhole and orthogonal in camera coordinates
// min[0..1],max[0..1] -> x and y bounds on near plane
// min[2],max[2]       -> znear and zfar
struct ViewFrustum
{
  Vec3 min;
  Vec3 max;
};

// 2d viewport size
struct ViewPort
{
    Vec2 min;
    Vec2 max;
    ViewPort(T x0, T y0, T x1, T y1) : min({x0,y0}), max({x1,y1}) {}
    T width()  const { return max[0]-min[0]; }
    T height() const { return max[1]-min[1]; }
};

private:
    // camera coordinate system
    Vec3 O;          // origin
    Vec3 X;          // right
    Vec3 Y;          // up
    Vec3 Z;          // -view

    // view parameters
    T m_fov;
    bool isOrtho;

    typedef std::tuple< bool, Matrix<T,4> > CamMatrix;

    CamMatrix m_projection;
    CamMatrix m_cam2world;
    CamMatrix m_world2cam;

    inline void invalidateProjection()
    {
        std::get<0>(m_projection) = true;
    }
    inline void invalidateTransformation()
    {
        std::get<0>(m_cam2world) = true;
        std::get<0>(m_world2cam) = true;
    }
    inline void invalidateMatrices()
    {
        invalidateProjection();
        invalidateTransformation();
    }

    ViewFrustum m_frustum;
    ViewPort    m_viewport;

 public:
    Camera() :
    O(0,0,0), X(1,0,0), Y(0,1,0), Z(0,0,-1), m_viewport(0,0,100,100)
    {
        // set default frustum
        setupPinholeCamera(m_viewport, 0.01f, 100.0f, 45.0f);
    }

    inline Vec3 getPosition() const { return O; }
    inline Vec3 getX()        const { return X; }
    inline Vec3 getY()        const { return Y; }
    inline Vec3 getZ()        const { return Z; }
    inline T    getFOV()      const { return m_fov; }
    inline bool isOrthoCam()  const { return isOrtho; }
    inline ViewPort getViewPort() const { return m_viewport; }
    inline ViewFrustum getViewFrustum() const { return m_frustum;  }
    // the camera can either be defined by:
    // viewport, znear, zfar, fov

    inline void setupPinholeCamera(const ViewPort &vp,
             const T znear, const T zfar, const T fov)
    {
        isOrtho = false;
        m_fov = fov;
        m_viewport = vp;
        // setup the view frustum
        if (vp.width() >= vp.height())
        {
           T aspect = m_viewport.width()/m_viewport.height();
           m_frustum.max[1] = znear * tan(fov * (T)3.141592653589793 / (T)360.0);
           m_frustum.min[1] = -m_frustum.max[1];
           m_frustum.min[0] = m_frustum.min[1] * aspect;
           m_frustum.max[0] = m_frustum.max[1] * aspect;
        } else {
           T aspect = m_viewport.height()/m_viewport.width();
           m_frustum.max[0] = znear * tan(fov * (T)3.141592653589793 / (T)360.0);
           m_frustum.min[0] = -m_frustum.max[0];
           m_frustum.min[1] = m_frustum.min[0] * aspect;
           m_frustum.max[1] = m_frustum.max[0] * aspect;
        }
        m_frustum.min[2] = znear;
        m_frustum.max[2] = zfar;
        invalidateMatrices();
    }

    inline void changeFOV(const T newfov)
    {
        setupPinholeCamera(m_viewport, m_frustum.min[2], m_frustum.max[2], newfov);
    }

    inline void setupOrthographicCamera(const ViewPort &vp,
             const T znear, const T zfar)
    {
        isOrtho = true;
        m_fov = 0.0;
        m_viewport = vp;
        m_frustum.min[0]=m_viewport.min[0];
        m_frustum.max[0]=m_viewport.max[0];
        m_frustum.min[1]=m_viewport.max[1];
        m_frustum.max[1]=m_viewport.min[1];
        m_frustum.min[2] = znear;
        m_frustum.max[2] = zfar;
        invalidateMatrices();
    }

    inline void setupOrthographicCamera(const ViewFrustum &vf, const ViewPort &vp,
             const T znear, const T zfar)
    {
        m_frustum = vf;
        m_viewport = vp;
        isOrtho = true;
        m_frustum.min[2] = znear;
        m_frustum.max[2] = zfar;
        invalidateMatrices();
    }

    // change viewport
    inline void changeViewport(const ViewPort &vp)
    {
        // setup the camera frustum and viewport
        if (isOrtho)
        {
            setupOrthographicCamera(vp, m_frustum.min[2],m_frustum.max[2]);
        } 
        else 
        {
            setupPinholeCamera(vp, m_frustum.min[2],m_frustum.max[2], m_fov);
        }
    }

    // camera -> world coordinates  
    // Wc = O + cx*[X] + cy*[Y] + cz*[Z] = O + [X|Y|Z] * [cc]
    inline const Matrix<T,4> & cam2world()
    {
        if (std::get<0>(m_cam2world))
        {
            Matrix<T,4> mv;
            mv(0)=X[0]; mv(4)=Y[0]; mv(8) =Z[0]; mv(12)=O[0];
            mv(1)=X[1]; mv(5)=Y[1]; mv(9) =Z[1]; mv(13)=O[1];
            mv(2)=X[2]; mv(6)=Y[2]; mv(10)=Z[2]; mv(14)=O[2];
            mv(3)=0.0; mv(7)=0.0; mv(11)=0.0; mv(15)=1.0;
            std::get<0>(m_cam2world) = false;
            std::get<1>(m_cam2world) = mv;
        }
        return std::get<1>(m_cam2world);
    }
    inline Vec3 cam2world(const Vec3 &c)
    {
        const Vec4 v(c[0],c[1],c[2],1.0f);
        const Vec4 w = cam2world() * v;
        return Vec3(w[0]/w[3],w[1]/w[3],w[2]/w[3]);
    }

    // world -> camera coordinates
    // inverse of cam->world transform
    // cc = [X|Y|Z]^-1 * (wc-O)
    // due to orthogonality of XYZ:
    // transformation matrix = [X|Y|Z]^T * wc - ([X|Y|Z]^T*O)
    inline const Matrix<T,4> & world2cam()
    {
        if (std::get<0>(m_world2cam))
        {
            std::get<0>(m_world2cam) = false;
            Matrix<T,4> mv;
            mv(0)=X[0]; mv(4)=X[1]; mv(8) =X[2]; mv(12)=-X * O;
            mv(1)=Y[0]; mv(5)=Y[1]; mv(9) =Y[2]; mv(13)=-Y * O;
            mv(2)=Z[0]; mv(6)=Z[1]; mv(10)=Z[2]; mv(14)=-Z * O;
            mv(3)=0.0; mv(7)=0.0; mv(11)=0.0; mv(15)=1.0;
            std::get<1>(m_world2cam) = mv;
        }
        return std::get<1>(m_world2cam);
    }

    inline Vec3 world2cam(const Vec3 &w) 
    {
        const Vec4 v(w[0],w[1],w[2],1.0f);
        const Vec4 c = world2cam() * v;
        return Vec3(c[0]/c[3],c[1]/c[3],c[2]/c[3]);
    }

    // projection matrix
    inline Matrix<T,4> getProjection()
    {
        if (std::get<0>(m_projection))
        {
            Matrix<T,4> p;
            if(isOrtho)
            {
                // orthographical camera
                p(0) = (T)2.0/(m_frustum.max[0]-m_frustum.min[0]);
                p(1) = 0;
                p(2) = 0;
                p(3) = 0;
                p(4) = 0;
                p(5) = (T)2.0/(m_frustum.max[1]-m_frustum.min[1]);
                p(6) = 0;
                p(7) = 0;
                p(8) = 0;
                p(9) = 0;
                p(10) = (T)-2.0/(m_frustum.max[2]-m_frustum.min[2]);
                p(11) = 0;
                p(12) = (T)-(m_frustum.max[0]+m_frustum.min[0])/(m_frustum.max[0]-m_frustum.min[0]);
                p(13) = (T)-(m_frustum.max[1]+m_frustum.min[1])/(m_frustum.max[1]-m_frustum.min[1]);
                p(14) = (T)-(m_frustum.max[2]+m_frustum.min[2])/(m_frustum.max[2]-m_frustum.min[2]);
                p(15) = 1.0;
            } else {
                // classical pinhole camera
                p(0) = (T)(2.0*m_frustum.min[2])/(m_frustum.max[0]-m_frustum.min[0]);
                p(1) = 0.0;
                p(2) = 0.0;
                p(3) = 0.0;
                p(4) = 0.0;
                p(5) = (T)(2.0*m_frustum.min[2])/(m_frustum.max[1]-m_frustum.min[1]);
                p(6) = 0.0;
                p(7) = 0.0;
                p(8) = (T)(m_frustum.max[0]+m_frustum.min[0])/(m_frustum.max[0]-m_frustum.min[0]);
                p(9) = (T)(m_frustum.max[1]+m_frustum.min[1])/(m_frustum.max[1]-m_frustum.min[1]);
                p(10) = (T)-(m_frustum.max[2]+m_frustum.min[2])/(m_frustum.max[2]-m_frustum.min[2]);
                p(11) = -1.0;
                p(12) = 0.0; p(13) = 0.0;
                p(14) = (T)-2.0*m_frustum.max[2]*m_frustum.min[2]/(m_frustum.max[2]-m_frustum.min[2]);
                p(15) = 0.0;
            }
            std::get<0>(m_projection) = false;
            std::get<1>(m_projection) = p;
        }
        return std::get<1>(m_projection);
    }

    // movement
    void setPosition(const Vec3 &pos)
    {
      O = pos;
      invalidateTransformation();
    }

    void setOrientation(const Vec3 &x, const Vec3 &y, const Vec3 &z)
    {
        X = x; Y = y; Z = z;
        invalidateTransformation();
    }

    void move(const Vec3 &m)
    {
      O += X * m[0] + Y * m[1] + Z * m[2];
      invalidateTransformation();
    }

    // rotations
    void yaw(const T angle)
    {
      X = RotateVec(X,Y,angle);
      Z = RotateVec(Z,Y,angle);
      invalidateTransformation();
    }
    void pitch(const T angle)
    {
      Y = RotateVec(Y,X,angle);
      Z = RotateVec(Z,X,angle);
      invalidateTransformation();
    }
    void roll(const T angle)
    {
      X = RotateVec(X,Z,angle);
      Y = RotateVec(Y,Z,angle);
      invalidateTransformation();
    }

    void rotateAxis(const Vec3 &axis, const T angle)
    {
        X = RotateVec(X, axis, angle);
        Y = RotateVec(Y, axis, angle);
        Z = RotateVec(Z, axis, angle);
        invalidateTransformation();
    }

    // std output stream support
    friend std::ostream& operator<<(std::ostream& os, const ViewPort &vp)
    {
        os << vp.min << " | " << vp.max << " : " << vp.width() << " x " << vp.height();
        return os;
    }


    friend std::ostream& operator<<(std::ostream& os, const Camera& c)
    {
        os << "O: " << c.O << " X:" << c.X << " Y:" << c.Y << " Z:" << c.Z << std::endl;
        return os;
    }

};

#endif

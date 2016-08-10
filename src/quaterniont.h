#ifndef _QUATERNION_HPP_
#define _QUATERNION_HPP_

//  Ulrich Krispel        uli@krispel.net

#include <cmath>
#include "vec3t.h"

template<class T> class QuaternionT {
public:
   // a + b * i + c * j + d * k
   T a;
   T b;
   T c;
   T d;

   // constructor
   QuaternionT<T>() : a(0.0), b(0.0), c(0.0), d(1.0) { }
   QuaternionT<T>(T e, T f, T g, T h) : a(e), b(f), c(g), d(h) { }
   QuaternionT<T>(const QuaternionT<T> &other) : 
      a(other.a), b(other.b), c(other.c), d(other.d) { }

   // set quaternion from axis/angle representation
   inline void setAxisAngle(T angle, MathVector<T,3> axis) {
      T sina = sin(angle/2.0);
      a = cos(angle/2.0);
      // normalize vector
      T n = 1.0/sqrt(axis.x*axis.x+axis.y*axis.y+axis.z*axis.z);
      b = axis.x * sina * n;
      c = axis.y * sina * n;
      d = axis.z * sina * n;
      normalize();
   }

   // unary * for multiplying two quaternions
   inline QuaternionT<T> operator*(const QuaternionT<T> &q) const { 
      return QuaternionT(
         a*q.a-b*q.b-c*q.c-d*q.d,
         a*q.b+b*q.a+c*q.d-d*q.c,
         a*q.c-b*q.d+c*q.a+d*q.b,
         a*q.d+b*q.c-c*q.b+d*q.a
       );
   }

   // normalize the quaternion
   inline QuaternionT<T>& normalize() 
   {
      T norm=sqrt(a*a+b*b+c*c+d*d);
      a /= norm;
      b /= norm;
      c /= norm;
      d /= norm;
      return *this;
   }

   // conjugate
   inline QuaternionT<T>& conjugate()
   {
      b = -b;
      c = -c;
      d = -d;
      return *this;
   }

   // convert quaternion to a 4x4 matrix usable in OpenGL
   inline void toRotationMatrix(float *matrix) {
      // need normalized quaternion
      normalize();

      matrix[0] = (float) (a*a + b*b - c*c - d*d);
      matrix[4] = (float) (2.0*b*c + 2.0*a*d);
      matrix[8] = (float) (2.0*b*d - 2.0*a*c);
      matrix[12] = (float) (0.0f);

      matrix[1] = (float) (2.0*b*c - 2.0*a*d);
      matrix[5] = (float) (a*a - b*b + c*c - d*d);
      matrix[9] = (float) (2.0*c*d + 2.0*a*b);
      matrix[13] = (float) (0.0f);

      matrix[2] = (float) (2.0*b*d + 2.0*a*c);
      matrix[6] = (float) (2.0*c*d - 2.0*a*b);
      matrix[10] = (float) (a*a - b*b - c*c + d*d);
      matrix[14] = (float) (0.0f);

      matrix[3] = (float) (0.0f);
      matrix[7] = (float) (0.0f);
      matrix[11] = (float) (0.0f);
      matrix[15] = (float) (1.0f);
   }

   // get conjugate
   QuaternionT<T> getConjugate() const {
      return QuaternionT<T>(a, -b, -c, -d);
   }

   // Multiplying a quaternion q with a vector v applies the q-rotation to v
   MathVector<T,3> operator* (const MathVector<T,3> &vec) const
   {
      MathVector<T,3> vn(vec);
      vn.normalize();
    
      QuaternionT<T> vecQuat, resQuat;
      vecQuat.b = vn.x;
      vecQuat.c = vn.y;
      vecQuat.d = vn.z;
      vecQuat.a = 0.0f;
    
      resQuat = vecQuat * getConjugate();
      resQuat = *this * resQuat;
    
      return (MathVector<T,3>(resQuat.b, resQuat.c, resQuat.d));
   }
};

typedef QuaternionT<double> Quaterniond;
typedef QuaternionT<float>  Quaternionf;

#endif
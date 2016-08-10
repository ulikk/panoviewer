#ifndef _VEC3T_HPP_
#define _VEC3T_HPP_

//! Type-templated 3-dimensional vector
//  Ulrich Krispel        uli@krispel.net

#include <iostream>
#include <cassert>
#include <cmath>
#include <limits>

template <class T> struct NumberTraits { };
#define NT_EPSILON 1e-6
template<> struct NumberTraits<double> {
   static inline double MIN()          { return std::numeric_limits<double>::min(); }
   static inline double MAX()          { return std::numeric_limits<double>::max(); }
   static inline double ABS(double v)  { return std::abs(v); }
//   static inline double SQRT(double a) { return sqrt(a);     }
   static inline bool   EQUAL(double a, double b) { return std::abs(a-b) < NT_EPSILON; }
   static inline bool   ZERO(double v)            { return std::abs(v) < NT_EPSILON; }
   static inline bool   LEQ(double a, double b)   { return EQUAL(a,b) || ((b-a)>NT_EPSILON); }
   static inline bool   LESS(double a, double b)  { return a<(b-NT_EPSILON); }
   static inline void   PRINT(std::ostream& os, double val) { os << val; }
   static inline int    SGN(double v)             { return (v < -NT_EPSILON) ? -1 : (v > NT_EPSILON) ? 1 : 0; }
};
template<> struct NumberTraits<float> {
   static inline float MIN()          { return -1e6;        }
   static inline float MAX()          { return 1e6;         }
   static inline float ABS(float v)   { return std::abs(v); }
//   static inline double SQRT(double a) { return sqrt(a);     }
   static inline bool   EQUAL(float a, float b) { return std::abs(a-b) < NT_EPSILON; }
   static inline bool   ZERO(float v)           { return std::abs(v) < NT_EPSILON; }
   static inline bool   LEQ(float a, float b)   { return EQUAL(a,b) || ((b-a)>NT_EPSILON); }
   static inline bool   LESS(float a, float b)  { return a<(b-NT_EPSILON); }
   static inline void   PRINT(std::ostream& os, float val) { os << val; }
   static inline int    SGN(float v)            { return (v < -NT_EPSILON) ? -1 : (v > NT_EPSILON) ? 1 : 0; }
};

template<> struct NumberTraits<int> {
   static inline int    MIN()               { return -100000; }
   static inline int    MAX()               { return 100000; }
   static inline int    ABS(int v)          { return std::abs(v); }
//   static inline int    SQRT(int a)         { std::abort(); return sqrt(a);     }
   static inline bool   EQUAL(int a, int b) { return a==b; }
   static inline bool   ZERO(int v)         { return v==0; }
   static inline bool   LEQ(int a, int b)   { return a<=b; }
   static inline bool   LESS(int a, int b)  { return a<b; }
   static inline void   PRINT(std::ostream& os, double val) { os << val; }
   static inline int    SGN(int v)          { return (v<0) ? -1 : (v>0) ? 1 : 0; }
};


template <class T, int D>
class MathVector
{
public:
   typedef T SCALAR_T;
   typedef NumberTraits<T> NT;
   const static int TYPE_D=D;
   /// value
   T x[D];

   template <int I> inline       T& at()       { return x[I]; }
   template <int I> inline const T& at() const { return x[I]; }

   inline void assign(const T &v)
   { if (D>0) x[0]=v; }
   inline void assign(const T &v1, const T &v2)
   { if (D>0) x[0]=v1; if (D>1) x[1]=v2; }
   inline void assign(const T &v1, const T &v2, const T &v3)
   { if (D>0) x[0]=v1; if (D>1) x[1]=v2; if (D>2) x[2]=v3; }
   inline void assign(const T &v1, const T &v2, const T &v3, const T &v4)
   { if (D>0) x[0]=v1; if (D>1) x[1]=v2; if (D>2) x[2]=v3; if (D>3) x[3]=v4; }

   MathVector()                    { }
   MathVector(T v)                 { assign(v); }
   MathVector(T v1,T v2)           { assign(v1,v2); }
   MathVector(T v1,T v2,T v3)      { assign(v1,v2,v3); }
   MathVector(T v1,T v2,T v3,T v4) { assign(v1,v2,v3,v4); }

   inline MathVector & assignEach(const T&v)
   {
      for(int i=0;i<D;++i) x[i]=v;
      return *this;
   }

   inline MathVector & setZero() {
      for(int i=0;i<D;++i) x[i]=0;
      return *this;
   }

   /// inplace addition
   inline MathVector& operator+=(const MathVector& v)
   { for(int i=0; i<D; ++i) { x[i]+=v.x[i]; } return *this; }

   /// inplace subtraction
   inline MathVector& operator-=(const MathVector& v)
   { for(int i=0; i<D; ++i) { x[i]-=v.x[i]; } return *this; }

   /// unary addition
   inline MathVector operator+(const MathVector &v) const
   { return MathVector(*this)+=v; }

   /// unary subtraction
   inline MathVector operator-(const MathVector &v) const
   { return MathVector(*this)-=v; }
   inline MathVector operator-() const
   { MathVector v; for(int i=0; i<D; ++i) { v.x[i]=-x[i]; } return v; }

   /// inplace scalar multiplication
   inline MathVector& operator*=(const T &v)
   { for(int i=0;i<D;++i) { x[i]*=v; } return *this; }

   /// scalar multiplication
   inline MathVector operator*(const T &v) const
   { return MathVector(*this)*=v; }

   /// inplace scalar division
   inline MathVector& operator/=(const T &v)
   { for(int i=0;i<D;++i) { x[i]/=v; } return *this; }

   /// scalar division
   inline MathVector operator/(const T &v) const
   { return MathVector(*this)/=v; }

   /// array access
   inline const T& operator[](const int i) const {
      return x[i];
   }
   inline T& operator[](const int i) {
      return x[i];
   }

   /// project - reduce by Dimension
   template <int PD> MathVector<T,D-1> project() const
   {
      MathVector<T,D-1> projection;
      int dd=0;
      for(int d=0;d<D;++d) if (d != PD) projection[dd++] = x[d];
      return projection;
   }

   /// dot product
   inline T operator*(const MathVector &v) const
   {
      T r=0;
      for(int i=0; i<D; ++i) { r+=x[i]*v.x[i]; }
      return r;
   }

   /// test if all elements are zero
   inline bool isZero() const
   {
      for (int i=0; i<D; ++i) { if(!NT::ZERO(x[i])) return false; }
      return true;
   }

   /// euclidean length
   inline T length() const
   {
       T l = 0; 
       for (int i = 0; i<D; ++i) { l += x[i] * x[i]; }
       return sqrt(l);
   }

   /// L1 norm
   inline T l1norm() const
   {
       T n = 0;
       for (int i = 0; i<D; ++i) { n += x[i] < 0 ? -x[i] : x[i]; }
       return n;
   }

   /// normalize
   inline MathVector & normalize()
   {
      T l = length();
      for(int i=0; i<D; ++i) { x[i]/=l; }
      return *this;
   }

   // set each MathVector element to its maximum/minimum (depending on sign)
   inline MathVector & elongate()
   {
      for (unsigned int i=0;i<D;++i)
      {
         if (NT::LEQ(x[i],0)) { x[i] = NT::MIN(); }
         else                 { x[i] = NT::MAX(); }
      }
      return *this;
   }
   inline MathVector & negate()
   {
      for(int i=0; i<D; ++i) { x[i]=-x[i]; }
      return *this;
   }

   // return dimension of element with maximum absolute value
   inline int maxdim() const {
      int maxd=0;
      T maxvalue = 0;
      for(unsigned int i=0;i<D;++i)
      {
         const T nv = NT::ABS(x[i]);
         if (nv > maxvalue) {
            maxvalue = nv;
            maxd=i;
         }
      }
      return maxd;
   }
   // comparison operators
   inline const bool operator==(const MathVector &other) const
   {
      for(int i=0;i<D;++i)
      {
         if (!NT::EQUAL(x[i],other.x[i])) return false;
      }
      return true;
   }

   inline const bool operator!=(const MathVector &other) const
   {
      for(int i=0;i<D;++i)
      {
         if (!NT::EQUAL(x[i],other.x[i])) return true;
      }
      return false;
   }

   /// < operator accounts for lexicographical sorting
   inline const bool operator<(const MathVector &other) const
   {
      for(int i=0;i<D;++i)
      {
         if (x[i] < other.x[i]) return true;
         if (x[i] > other.x[i]) return false;
      }
      return false;
   }

   /// conversion to pointer
   inline operator const T* () const { return x; }
};

template <typename T, int D> inline std::ostream& operator<<
(std::ostream &os, const MathVector<T,D> &v)
{
    os << "[" << v[0];
    for (int i=1; i<D; ++i)
    {
        os << "," << v[i];
    }
    os << "]";
    return os;
}

typedef MathVector<int,2>       Vec2i;
typedef MathVector<long long,2> Vec2l;
typedef MathVector<float,2>     Vec2f;
typedef MathVector<double,2>    Vec2d;

typedef MathVector<int,3>       Vec3i;
typedef MathVector<long long,3> Vec3l;
typedef MathVector<float,3>     Vec3f;
typedef MathVector<double,3>    Vec3d;

typedef MathVector<int,4>       Vec4i;
typedef MathVector<long long,4> Vec4l;
typedef MathVector<float,4>     Vec4f;
typedef MathVector<double,4>    Vec4d;

// define cross product equivalent for 2D
template <typename T>
T inline static CrossProduct( const MathVector<T,2> &a, const MathVector<T,2> &b)
{
   return a.x[0]*b.x[1]-a.x[1]*b.x[0];
}


// define cross product for 3D
template <typename T>
MathVector<T,3> inline static CrossProduct(
                const MathVector<T,3> &a, const MathVector<T,3> &b)
{
   return MathVector<T,3>(
      a.x[1]*b.x[2] - a.x[2]*b.x[1],
      a.x[2]*b.x[0] - a.x[0]*b.x[2],
      a.x[0]*b.x[1] - a.x[1]*b.x[0]
   );
}

 // rodrigues' rotation
template <typename T>
MathVector<T,3> inline static RotateVec(
                const MathVector<T,3> &vec,
                const MathVector<T,3> &axis, const T angle)
 {
   //assert(axis == MathVector<3,T>(axis).normalize());
   return vec * cos(angle)
       + CrossProduct(axis,vec) * sin(angle)
       + axis * (axis*vec) * (1-cos(angle));
 }

#endif

#ifndef _MATRIX_HPP_
#define _MATRIX_HPP_

#include <cstring>
#include <cassert>
#include <vec3t.h>

// DxD matrix with elements of type T 
// in column major order (opengl-ready)
//  Ulrich Krispel        uli@krispel.net

template <class T, int D>
class Matrix
{
    T e[D*D];    // elements
public:
    inline void identity()
    {
       for(int i=0,ie=D*D;i<ie;++i) e[i] = (i%(D+1)) ? (T)0.0 : (T)1.0;
    }

    // default constructor: initialize with identity
    inline Matrix()
    {
        identity();
    }

    // constructor with matrix elements
    Matrix(T E[16])
    {
        std::copy(E,E+16,e);
    }
    static inline Matrix ZERO() {
        T I[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        return Matrix(I);
    }

    /// conversion to pointer
    inline const T* ptr() const       { return e; }		// method access
    inline operator const T* () const { return e; }     // cast

    inline Matrix transpose() const
    {
        Matrix TR;   // transposed
        for (int r = 0; r < D; ++r)
            for (int c = 0; c < D; ++c)
                TR(c, r) = (*this)(r, c);
        return TR;
    }

    //! array access operator
    inline T& operator()(int row, int col) {
       assert((row >= 0) && (row < D));   // bounds check
       assert((col >= 0) && (col < D));   // bounds check
       return e[col*D+row];
    }
    inline const T& operator()(int row, int col) const {
       assert((row >= 0) && (row < D));   // bounds check
       assert((col >= 0) && (col < D));   // bounds check
       return e[col*D+row];
    } 

    inline T& operator()(int index) {
       assert((index >= 0) && (index < (D*D))); // bounds check
       return e[index];
    }
    inline T& operator[](int index) {
       assert((index >= 0) && (index < (D*D))); // bounds check
       return e[index];
    }

    /// matrix - matrix multiplication R = A * B
    template <class RT>
    inline const RT operator*(const RT &B) const
    {
        RT R = RT::ZERO();
        for (int r=0;r<D;++r)
            for(int c=0;c<D;++c) {
                for (int d=0;d<D;++d)
                    R(r,c) += (*this)(r,d) * B(d,c);
            }
        return R;
    }


    /// matrix - vector multiplication
    template <class V>
    inline MathVector<V,D> operator*(const MathVector<V,D> &v) const
    {
        MathVector<V,D> m;           // result
        for (int r = 0; r < D; ++r)  // row
        {
            m[r] = 0;
            for (int d = 0; d < D; ++d) { m[r] += e[d*D+r] * v[d]; }
        }
        return m;
    }

    // type conversion
    template <class CT>
    inline Matrix<CT,D> convert() const
    {
        Matrix<CT,D> M;
        for (int i = 0; i < D*D; ++i) M[i] = (CT)(*this)[i];
        return M;
    }


    // std output stream support
    friend std::ostream& operator<<(std::ostream& os, const Matrix& m)
    {
       for (int row=0;row<D;row++)
       {
         std::cout << "|";
         for (int col=0;col<D;col++)
         {
           os << m(row,col) << " ";
         }
         std::cout << "|" << std::endl;
       }  
       return os;
    }
};

typedef Matrix<float,4> Matrix4f;
typedef Matrix<double,4> Matrix4d;

#endif

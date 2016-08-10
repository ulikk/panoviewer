#ifndef _IMAGE_H_
#define _IMAGE_H_

// simple image class
//  Ulrich Krispel        uli@krispel.net

#include <vector>
#include <cstring>   // for memset
#include <cassert>
#include <algorithm>

  template <class T>
    struct ImageT
    {
    protected:
        unsigned int W;
        unsigned int H;
        unsigned int BPP;
        std::vector<T> pdata;   // pixel data
    public:
    
        ImageT() : W(0), H(0), BPP(0)
        {
        }

        inline const unsigned int rgb(const int x, const int y)
        {
           unsigned int offset = (y*W + x) * (BPP/8);
           assert(offset < pdata.size());
           return pdata[offset]|(pdata[offset+1]<<8)|(pdata[offset+2]<<16);
        }

        inline bool isValid()  const { return W != 0 && H != 0 && BPP != 0; }
        inline int width()     const { return W;   }
        inline int height()    const { return H;   }
        inline int bpp()       const { return BPP; }
        inline int chan()  const { return BPP / (sizeof(T)* 8); }
        inline const T *data() const { return &pdata[0]; }
        inline const std::vector<T>& getData() const { return pdata; }
        inline long long buffersize() const {  return W*H*chan();  }

        inline std::vector<T>& unsafeData() { return pdata; }

        inline void initialize(int width, int height, int bpp)
        {
            W = width;
            H = height;
            BPP = bpp;
            pdata.resize((unsigned)buffersize());
        }

        inline void resize(int width, int height, int channels = 3)
        {
           W   = width;
           H   = height;
           BPP = channels * (sizeof(T)*8);
           const unsigned bsize = (unsigned) buffersize();
           pdata.resize(bsize);
        }

        // blit image
        inline void drawImage(const int l, const int t, const ImageT &img)
        {
            assert(chan() == img.chan());
            const int b = std::min(t+img.height(), height());
            const int r = std::min(l+img.width(), width());
            for(int y=t; y<b; ++y)
            {
                for (int x=l; x<r; ++x)
                {
                    for (int c=0;c<chan();++c)
                        setPixel(x,y,c,img(x-l,y-t,c));
                }
            }
        }

        // pixel accessors
        inline T &operator()(int offset)
        {
            assert(offset >= 0 && offset < buffersize());
            return pdata[offset];
        }

        inline void setPixel(const int x, const int y, const int ch, const T value)
        {
            if(x>=0 && x<W && y>=0 && y<H)
            {
                pdata[(y * W + x) * BPP / (sizeof(T)*8) + ch] = value;
            }
        }

        inline T &operator()(int x, int y, int ch = 0) 
        { 
            assert(x >= 0 && x<(int)W && y >= 0 && y<(int)H);
            return pdata[(y * W + x) * BPP / (sizeof(T)*8) + ch];
        }

        inline const T &operator()(int x, int y, int ch = 0) const 
        { 
            assert(x >= 0 && x<(int)W && y >= 0 && y<(int)H);
            return pdata[(y * W + x) * BPP / (sizeof(T)*8) + ch];
        }
        void clear(T value = 0) 
        {
            memset(&pdata[0], value, W*H*BPP/(sizeof(T)*8));
        }
    };

    typedef ImageT<unsigned char> Image;
    typedef ImageT<float> ImageF;

#endif

#ifndef _USE_IMGJPG_H_
#define _USE_IMGJPG_H_

// wrapper for jpeglib
//  Ulrich Krispel        uli@krispel.net

#include "image.h"
#include "jpeglib.h"

#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <functional>

namespace IMG
{
    template <class IMGTYPE>
    bool loadJPEG(const char *fname, IMGTYPE &img)
    {
        struct jpeg_decompress_struct cinfo;
        struct jpeg_error_mgr jerr;

        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_decompress(&cinfo);

        FILE *infile = fopen(fname, "rb");

        if (infile == NULL || ferror(infile)) 
        {
            fprintf(stderr, "can't open %s\n", fname);
            return false;
        }
        jpeg_stdio_src(&cinfo, infile);
        jpeg_read_header(&cinfo, TRUE);

        // assume RGB
        img.resize(cinfo.image_width, cinfo.image_height);
        jpeg_start_decompress(&cinfo);

        // create vector with scanline start ptrs
        std::vector<JSAMPROW> rowptr(cinfo.image_height);
        for (unsigned int i=0; i<cinfo.image_height; ++i) 
        { 
            rowptr[i]=( &img(0,i) ); //     &m_data[i * cinfo.image_width * m_channels]
        }

        // read scanlines
        while (cinfo.output_scanline < cinfo.output_height) 
        {
            jpeg_read_scanlines(&cinfo, &rowptr[cinfo.output_scanline], 10);
        }
    
        jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);

        return true;
   }


    template <class IMGTYPE>
    bool saveJPEG(const char *fname, const IMGTYPE &img, const int quality = 80)
    {
        struct jpeg_compress_struct cinfo;
        struct jpeg_error_mgr jerr;
        FILE * outfile;
        JSAMPROW row_pointer[1];
        int row_stride;

        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_compress(&cinfo);

        if ((outfile = fopen(fname, "wb")) == NULL) {
            fprintf(stderr, "error opening file %s for writing\n", fname);
            return false;
        }
        jpeg_stdio_dest(&cinfo, outfile);

        cinfo.image_width = img.width();
        cinfo.image_height = img.height();
        cinfo.input_components = img.channels();
        cinfo.in_color_space = img.channels() == 3 ? JCS_RGB : JCS_GRAYSCALE;

        jpeg_set_defaults(&cinfo);
        jpeg_set_quality(&cinfo, quality, TRUE);
        jpeg_start_compress(&cinfo, TRUE);

        row_stride = img.width() * img.channels();
        while (cinfo.next_scanline < cinfo.image_height) {
            row_pointer[0] = (JSAMPROW)&img.getData()[cinfo.next_scanline * row_stride];
            (void)jpeg_write_scanlines(&cinfo, row_pointer, 1);
        }

        jpeg_finish_compress(&cinfo);
        fclose(outfile);
        jpeg_destroy_compress(&cinfo);
        return true;
    }


    // EXIF handling
    namespace EXIF
    {
        // helpers for big / little endian access
        struct SHORT
        {
        private:
            unsigned char data[2];
            //unsigned short int data;
        public:
            unsigned short int value(bool isBigEndian = false) const
            {
                if (isBigEndian) { return (int)data[1] + ((int)data[0] << 8); }
                else             { return (int)data[0] + ((int)data[1] << 8); }
            }
        };

        struct LONG
        {
        private:
            unsigned char data[4];
        public:
            unsigned int value(bool isBigEndian = false) const
            {
                if (isBigEndian)
                {
                    return (int)data[3] + ((int)data[2] << 8) + ((int)data[1] << 16) + ((int)data[0] << 24);
                }
                else
                {
                    return (int)data[0] + ((int)data[1] << 8) + ((int)data[2] << 16) + ((int)data[3] << 24);
                }
            }
        };

        typedef std::map<unsigned int, std::string> EXIFTAGS;

        // parse EXIF for field of view information
        // http://www.media.mit.edu/pia/Research/deepview/exif.html
        struct Marker
        {
            unsigned char FF;	 // always 0xFF
            unsigned char type;  // marker type
            SHORT size;			 // data size
        };

        enum IFDDataFormat {
            UnsignedByte = 1, AsciiStrings = 2, UnsignedShort = 3,
            UnsignedLong = 4, UnsignedRational = 5, SignedByte = 6,
            Undefined = 7, SignedShort = 8, SignedLong = 9, 
            SingleFloat = 11, DoubleFloat = 12
        };


        enum EXIFTag {
            ImageWidth = 0x100,
            ImageLength = 0x101,
            BitsPerSample = 0x102,
            Compression = 0x103,
            PhotometricInterpretation = 0x106,
            FillOrder = 0x10A,
            DocumentName = 0x10D,
            ImageDescription = 0x10E,
            Make = 0x10F,
            Model = 0x110,
            StripOffsets = 0x111,
            Orientation = 0x112,
            SamplesPerPixel = 0x115,
            RowsPerStrip = 0x116,
            StripByteCounts = 0x117,
            XResolution = 0x11A,
            YResolution = 0x11B,
            PlanarConfiguration = 0x11C,
            ResolutionUnit = 0x128,
            TransferFunction = 0x12D,
            Software = 0x131,
            DateTime = 0x132,
            Artist = 0x13B,
            WhitePoint = 0x13E,
            PrimaryChromaticities = 0x13F,
            TransferRange = 0x156,
            JPEGProc = 0x200,
            JPEGInterchangeFormat = 0x201,
            JPEGInterchangeFormatLength = 0x202,
            YCbCrCoefficients = 0x211,
            YCbCrSubSampling = 0x212,
            YCbCrPositioning = 0x213,
            ReferenceBlackWhite = 0x214,
            BatteryLevel = 0x828F,
            Copyright = 0x8298,
            ExposureTime = 0x829A,
            FNumber = 0x829D,
            ExifIFDPointer = 0x8769,
            InterColorProfile = 0x8773,
            ExposureProgram = 0x8822,
            SpectralSensitivity = 0x8824,
            GPSInfoIFDPointer = 0x8825,
            ISOSpeedRatings = 0x8827,
            OECF = 0x8828,
            ExifVersion = 0x9000,
            DateTimeOriginal = 0x9003,
            DateTimeDigitized = 0x9004,
            ComponentsConfiguration = 0x9101,
            CompressedBitsPerPixel = 0x9102,
            ShutterSpeedValue = 0x9201,
            ApertureValue = 0x9202,
            BrightnessValue = 0x9203,
            ExposureBiasValue = 0x9204,
            MaxApertureValue = 0x9205,
            SubjectDistance = 0x9206,
            MeteringMode = 0x9207,
            LightSource = 0x9208,
            Flash = 0x9209,
            FocalLength = 0x920A,
            SubjectArea = 0x9214,
            MakerNote = 0x927C,
            UserComment = 0x9286,
            SubSecTime = 0x9290,
            SubSecTimeOriginal = 0x9291,
            SubSecTimeDigitized = 0x9292,
            FlashPixVersion = 0xA000,
            ColorSpace = 0xA001,
            PixelXDimension = 0xA002,
            PixelYDimension = 0xA003,
            RelatedSoundFile = 0xA004,
            InteroperabilityIFDPointer = 0xA005,
            FlashEnergy = 0xA20B,
            SpatialFrequencyResponse = 0xA20C,
            FocalPlaneXResolution = 0xA20E,
            FocalPlaneYResolution = 0xA20F,
            FocalPlaneResolutionUnit = 0xA210,
            SubjectLocation = 0xA214,
            ExposureIndex = 0xA215,
            SensingMethod = 0xA217,
            FileSource = 0xA300,
            SceneType = 0xA301,
            CFAPattern = 0xA302,
            CustomRendered = 0xA401,
            ExposureMode = 0xA402,
            WhiteBalance = 0xA403,
            DigitalZoomRatio = 0xA404,
            FocalLengthIn35mmFilm = 0xA405,
            SceneCaptureType = 0xA406,
            GainControl = 0xA407,
            Contrast = 0xA408,
            Saturation = 0xA409,
            Sharpness = 0xA40A,
            DeviceSettingDescription = 0xA40B,
            SubjectDistanceRange = 0xA40C,
            ImageUniqueID = 0xA420
        };

        struct IFDEntry
        {
            SHORT tag;			// type
            SHORT format;		// data format
            LONG  numcomponents;
            LONG  data;
        };

        
        template <typename RESULT>
        RESULT parseExif(const std::string &filename) 
        {
            std::streamoff EXIFSTART;
            RESULT result;
            bool BE;

            Marker marker;
            std::ifstream infile(filename.c_str(), std::ios::binary | std::ios::in);
            //long long offset = 0;
            if (infile.is_open())
            {
                unsigned char FFD8[2];
                infile.read((char *)FFD8, 2);
                
                if (FFD8[0] != 0xFF || FFD8[1] != 0xD8)
                    return result;

                auto insertTag = [&result,&BE,&infile,&EXIFSTART](const IFDEntry &e)
                {
                    std::ostringstream os;
                    switch (e.format.value(BE))
                    {
                    case UnsignedByte:
                        os << (unsigned char)e.data.value(BE);
                        result[e.tag.value(BE)] = os.str();
                        break;
                    case AsciiStrings:
                    {
                        std::streamoff cur = infile.tellg();
                        int ifdoff = e.data.value(BE);
                        infile.seekg(EXIFSTART + ifdoff);
                        std::vector<char> thestring(e.numcomponents.value(BE)+1);
                        infile.read((char *)&thestring[0], e.numcomponents.value(BE));
                        thestring.at(e.numcomponents.value(BE)) = 0;
                        result[e.tag.value(BE)] = std::string(&thestring[0]);
                        infile.seekg(cur);
                    }
                        break;
                    case UnsignedShort:
                        os << (unsigned short)e.data.value(BE);
                        result[e.tag.value(BE)] = os.str();
                        break;
                    case UnsignedLong:
                        os << (unsigned long)e.data.value(BE);
                        result[e.tag.value(BE)] = os.str();
                        break;
                    case UnsignedRational:
                        break;
                    case SignedByte:
                        os << (char)e.data.value(BE);
                        result[e.tag.value(BE)] = os.str();
                        break;
                    case Undefined:
                    {
                        if (e.tag.value(BE) == 0x9286)  // parse UserComment
                        {
                            std::streamoff cur = infile.tellg();
                            int ifdoff = e.data.value(BE);
                            infile.seekg(EXIFSTART + ifdoff);
                            char format[8];
                            infile.read(format, 8);
                            int stsize = e.numcomponents.value(BE);
                            assert(stsize > 8);
                            // assume ascii
                            std::vector<char> thestring(stsize + 1 - 8);
                            infile.read((char *)&thestring[0], stsize -8);
                            thestring.at(e.numcomponents.value(BE)-8) = 0;
                            result[e.tag.value(BE)] = std::string(&thestring[0]);
                            infile.seekg(cur);
                        }
                    }
                        break;
                    case SignedShort:
                        os << (short)e.data.value(BE);
                        break;
                    case SignedLong:
                        os << (long)e.data.value(BE);
                        break;
                    case SingleFloat:
                        os << (float)e.data.value(BE);
                        break;
                    case DoubleFloat:
                        os << (double)e.data.value(BE);
                        break;
                    default:
                        std::cout << "[EXIF ERROR] unknown format value" << std::endl;
                    }
                };


                std::function<void(void)> readDirectory = [&infile, &BE, &readDirectory, &insertTag, &EXIFSTART]()
                {
                    SHORT entries;
                    infile.read((char *)&entries, 2);
                    const int numEntries = entries.value(BE);
                    //std::cout << "Number of IFD entries:" << numEntries << std::endl;
                    std::vector<IFDEntry> IFDDirectory;
                    IFDDirectory.resize(numEntries);
                    infile.read((char *)&IFDDirectory[0], numEntries * 12);
                    //int n = 0;
                    for (const IFDEntry &e : IFDDirectory)
                    {
                        //std::cout << n++ << " : TAG:" << e.tag.value(BE)
                        //          << " FORMAT:" << e.format.value(BE)
                        //          << " NUMCOMPONENTS:" << e.numcomponents.value(BE)
                        //          << " VALUE:" << e.data.value(BE) << std::endl;

                        if (e.tag.value(BE) == ExifIFDPointer)  // sub IFD
                        {
                            int ifdoff = e.data.value(BE);
                            infile.seekg(EXIFSTART + ifdoff);
                            readDirectory();
                        }
                        else
                        {
                            insertTag(e);
                        }
                    }
                };

                while (!infile.eof()) {
                    infile.read((char *)&marker, sizeof(Marker));
                    if (marker.FF != 0xFF)
                    {
                        result.clear();
                        return result;
                    }
                    //assert(marker.FF == 0xFF);
                    //std::cout << "Marker: " << std::hex << (int)marker.FF << ":" << (int)marker.type << " size:" << marker.size.value(true) << std::endl;
                    if (marker.type == 0xE1)
                    {
                        //std::cout << "EXIF header found:" << std::endl;

                        char exif[6];
                        char exifheader[8];
                        infile.read((char *)exif, 6);
                        EXIFSTART = infile.tellg();
                        infile.read((char *)exifheader, 8);
                        BE = exifheader[0] == 0x4D;

                        readDirectory();
                        return result;
                    }
                    else {
                        infile.seekg(marker.size.value(true) - 2, std::ios_base::cur);
                    }
                }
            }
            return result;
        }
    }

}

#endif

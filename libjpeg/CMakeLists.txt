# just place the contents of libjpeg src (without the subdir) 
# from http://www.ijg.org/ in this directory

project (libjpeg)

if(MSVC)
  configure_file(jconfig.vc jconfig.h COPYONLY)
endif(MSVC)

if(CMAKE_COMPILER_IS_GNUCC)
  configure_file(jconfig.txt jconfig.h COPYONLY)
endif(CMAKE_COMPILER_IS_GNUCC)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(libjpeg
  jaricom.c
  jcapimin.c
  jcapistd.c
  jcarith.c
  jccoefct.c
  jccolor.c
  jcdctmgr.c
  jchuff.c
  jcinit.c
  jcmainct.c
  jcmarker.c
  jcmaster.c
  jcomapi.c
  jcparam.c
  jcprepct.c
  jcsample.c
  jctrans.c
  jdapimin.c
  jdapistd.c
  jdarith.c
  jdatadst.c
  jdatasrc.c
  jdcoefct.c
  jdcolor.c
  jddctmgr.c
  jdhuff.c
  jdinput.c
  jdmainct.c
  jdmarker.c
  jdmaster.c
  jdmerge.c
  jdpostct.c
  jdsample.c
  jdtrans.c
  jerror.c
  jfdctflt.c
  jfdctfst.c
  jfdctint.c
  jidctflt.c
  jidctfst.c
  jidctint.c
  jmemmgr.c
  jmemnobs.c
  jquant1.c
  jquant2.c
  jutils.c
  jconfig.h
  jdct.h
  jerror.h
  jinclude.h
  jmemsys.h
  jmorecfg.h
  jpegint.h
  jpeglib.h
  jversion.h
)

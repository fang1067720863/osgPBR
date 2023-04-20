#pragma once


#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__)  || defined( __MWERKS__)
#  if defined( OE_MATERIAL_STATIC )
#    define OSG_EXPORT
#  elif defined( OE_MATERIAL_LIBRARY )
#    define OE_MATERIAL_PULGIN   __declspec(dllexport)
#  else
#    define OE_MATERIAL_PULGIN   __declspec(dllimport)
#  endif
#else
#  define OSG_EXPORT
#endif




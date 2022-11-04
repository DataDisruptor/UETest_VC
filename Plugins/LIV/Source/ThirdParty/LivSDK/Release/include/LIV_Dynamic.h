/**
 * \file LIV_Dynamic.h
 * \brief Header containing export configuration for symbols
 * \copyright 2020-2022 LIV Inc. S.r.o.
 */

#ifndef __LIV_DYNAMIC
#define __LIV_DYNAMIC

#ifdef LIV_DYNAMIC
# ifdef LIV_EXPORT
#  define LIV_DECLSPEC __declspec(dllexport)
# else //if !LIV_EXPORT
#  define LIV_DECLSPEC __declspec(dllimport)
# endif //LIV_EXPORT
# else //if !LIV_DYNAMIC
#  define LIV_DECLSPEC
#endif //LIV_DYNAMIC

#endif //__LIV_DYNAMIC
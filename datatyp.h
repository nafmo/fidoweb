// datatyp.h
// Defines data types that are independent of the compiler specific
// char/short/int/long types.

#ifndef __DATATYP_H
#define __DATATYP_H
#include <limits.h>

#if (UCHAR_MAX == 255)
    typedef unsigned char UINT8;
    typedef signed char SINT8;
#else
    #error No 8-bit integer type found
#endif

#if (UCHAR_MAX == 65535)
    typedef unsigned char UINT16;
    typedef signed char SINT16;
#else
    #if (USHRT_MAX == 65535)
        typedef unsigned short UINT16;
        typedef signed short SINT16;
    #else
        #if (UINT_MAX == 65535)
            typedef unsigned int UINT16;
            typedef signed int SINT16;
        #else
            #error No 16-bit integer type found
        #endif
    #endif
#endif

#if (UCHAR_MAX == 4294967295)
    typedef unsigned char UINT32;
    typedef signed char SINT32;
#else
    #if (USHRT_MAX == 4294967295)
        typedef unsigned short UINT32;
        typedef signed short SINT32;
    #else
        #if (UINT_MAX == 4294967295)
            typedef unsigned int UINT32;
            typedef signed int SINT32;
        #else
            #if (ULONG_MAX == 4294967295)
                typedef unsigned long UINT32;
                typedef signed long SINT32;
            #else
                #error No 32-bit integer type found
            #endif
        #endif
    #endif
#endif
#endif

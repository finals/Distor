#ifndef __TYPES_H__
#define __TYPES_H__

typedef char               char8_t;
typedef unsigned char      uchar8_t;
typedef short int          int16_t;
typedef unsigned short int uint16_t; 
typedef int                int32_t;
typedef unsigned int       uint32_t;
#if __WORDSIZE == 64
typedef long int           int64_t;
typedef unsigned long int  uint64_t;
#else
typedef long long int      int64_t;
typedef unsigned long long int uint64_t;
#endif

#endif

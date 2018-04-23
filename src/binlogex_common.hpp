#ifndef __BINLOGEX_COMMON__
#define __BINLOGEX_COMMON__

#include "sys/types.h"
#include "sys/stat.h"
#include "getopt.h"
#include "unistd.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <boost/regex.hpp>
#include "dirent.h"
#include "time.h"
#include "map"
#include "string"
#include "iostream"

#include "version.hpp"

using namespace std;

typedef unsigned char	uchar;	/* Short for unsigned char */
typedef signed char int8;       /* Signed integer >= 8  bits */
typedef unsigned char uint8;    /* Unsigned integer >= 8  bits */
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;

#define sint2korr(A)	(int16) (*((int16 *) (A)))
#define sint3korr(A)	((int32) ((((uchar) (A)[2]) & 128) ? \
				  (((uint32) 255L << 24) | \
				   (((uint32) (uchar) (A)[2]) << 16) |\
				   (((uint32) (uchar) (A)[1]) << 8) | \
				   ((uint32) (uchar) (A)[0])) : \
				  (((uint32) (uchar) (A)[2]) << 16) |\
				  (((uint32) (uchar) (A)[1]) << 8) | \
				  ((uint32) (uchar) (A)[0])))
#define sint4korr(A)	(int32)  (*((int32 *) (A)))
#define uint2korr(A)	(uint16) (*((uint16 *) (A)))
#define uint3korr(A)	(uint32) (((uint32) ((uchar) (A)[0])) +\
				  (((uint32) ((uchar) (A)[1])) << 8) +\
				  (((uint32) ((uchar) (A)[2])) << 16))
#define uint4korr(A)	(uint32) (*((uint32 *) (A)))




#endif

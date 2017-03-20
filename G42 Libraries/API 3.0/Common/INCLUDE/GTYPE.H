// GTYPE.H - basic type definitions for all group42 programs 
#if !defined(GTYPE_H)
#define GTYPE_H
/*-----------------------------------------------------------------------------
	file:			gtype.h
	author:		Guy Schalnat
	date:			10/1/96
	revision:	3.0
	Basic type definition file to be included in all group 42's libraries.
	This file detects what platform is being compiled for, which compiler
	is being used, and sets flags appropriately.  It also includes the
	standard libraries necessary for that platform.
	This file defines the following types:
		uintf			an unsigned fast integer (16 bits or more)
		intf			an signed fast integer (16 bits or more)
		int8        a 8 bit signed integer
		uint8       a 8 bit unsigned integer
		byte        a uint8
		int16			a 16 bit signed integer
		int32			a 32 bit signed integer
		uint16		a 32 bit unsigned integer
		uint32		a 32 bit unsigned integer
	The following are defined only if the compiler doesn't support them
	yet:
		bool			if not defined, an int - only true or false
		true      	1
		false     	0
		min			a template that returns the maximum value
		max			a template that returns the minimum value
	Defines that tend to get set here:
		MSWIN						Microsoft Windows OS
		MACOS						Mac OS
		XWIN						X Windows OS
		INT_IS_16				int type is 16 bits
		INTEL_BYTE_ORDER		least significant byte first if true
		G42_HAVE_INT16			int16 and uint16 is already defined
		G42_HAVE_INT32			int32 and uint32 is already defined
		G42_NEED_BOOL			need to define bool
		G42_NEED_MIN_MAX		need to define min and max
		G42_NEW_DOESNT_THROW	we need to check new (see gexcept.h)
		G42_NEED_XALLOC		we need to define xalloc
	Finally, the file includes gexcept.h
-----------------------------------------------------------------------------*/
//	detect which operating system we are in, and set appropriate variables
#ifdef __MSDOS__
#  define MSWIN
#  define INT_IS_16
#  define INTEL_BYTE_ORDER
#  define G42_HAVE_INT16
#  define G42_HAVE_INT32
#endif
#ifdef __WIN32__
#  define MSWIN
#  define INTEL_BYTE_ORDER
#  define G42_HAVE_INT16
#  define G42_HAVE_INT32
#endif
// Watcom v. 11 or later
#if defined(__WATCOMC__)
#  define G42_NEED_MIN_MAX
#  define G42_NEW_DOESNT_THROW
#  define G42_NEED_XALLOC
#  define MSWIN
#  if defined(_WINDOWS)
#    define INT_IS_16
#  endif
#  define INTEL_BYTE_ORDER
#endif
// (MWERKS means MetroWerks Code Warrior)
#if defined (__MWERKS__) || defined (MAC)
#  define MACOS
#  define G42_NEW_DOESNT_THROW
#  define G42_NEED_XALLOC
//#  define G42_NEED_BOOL
#  define G42_NEED_MIN_MAX
#endif
#if !defined(MSWIN) && !defined(MACOS)
#  define XWIN
#endif
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <typeinfo.h>
#ifdef __TURBOC__
#  include <alloc.h>
#  include <systypes.h>
#  include <mem.h>
#endif
#ifdef INT_IS_16
#  define G42HUGE huge
#  define G42FAR far
#else
#  define G42HUGE
#  define G42FAR
#endif
typedef unsigned int uintf;
typedef int intf;
typedef unsigned char byte;
#ifdef G42_NEED_BOOL
typedef int bool;
#define true 1
#define false 0
#endif
#ifdef G42_NEED_MIN_MAX
//  I haven't found these in Metrowerks yet.  D.I.
#define min(x,y) ((x > y) ? y : x)
#define max(x,y) ((x > y) ? x : y)
#endif
// save for use later if needed
#if 0
template <class T> inline const T& min (const T& t1, const T& t2)
{
	return t1 > t2 ? t2 : t1;
}
template <class T> inline const T& max (const T& t1, const T& t2)
{
	return t1 > t2 ? t1 : t2;
}
#endif
#if !defined(G42_HAVE_INT16)
#ifdef INT_IS_16
typedef int int16;
typedef unsigned int uint16;
#else
typedef short int16;
typedef unsigned short uint16;
#endif
#endif
#if !defined(G42_HAVE_INT32)
#ifdef INT_IS_16
typedef long int32;
typedef unsigned long uint32;
#else
// protect ourselves against 64 bit machines (alpha)
typedef int int32;
typedef unsigned int uint32;
#endif
#endif
typedef signed char int8;
typedef unsigned char uint8;
typedef uint8 byte;
template <class T> inline void swap(T & t1, T & t2)
{
	T t = t1;
   t1 = t2;
   t2 = t;
}
// macros for MACOS operating system
#ifdef MACOS
#define WINBITMAP(w)			(((GrafPtr)(w))->portBits)
#define WINPORTRECT(w)		((((WindowPeek)(w))->port).portRect)
#define WINDOWWIDTH(w)		(WINPORTRECT(w).right - WINPORTRECT(w).left)
#define WINDOWHEIGHT(w)		(WINPORTRECT(w).bottom - WINPORTRECT(w).top)
#define WINCONTENTRECT(w)	((**((WindowPeek)(w))->contRgn).rgnBBox)
#define WINCONTENTRGN(w)	(((WindowPeek)(w))->contRgn)
#define WINVISIBLERGN(w)	(((WindowPeek)(w))->port.visRgn)
#define WINSTRUCTRECT(w)	((**((WindowPeek)(w))->strucRgn).rgnBBox)
#define WINSTRUCTRGN(w)		(((WindowPeek)(w))->strucRgn)
#define WINUPDATERECT(w)	((**((WindowPeek)(w))->updateRgn).rgnBBox)
#define WINUPDATERGN(w)		(((WindowPeek)(w))->updateRgn)
#endif // MACOS
#include <gexcept.h>
// do not put anything past this line
#endif /* GTYPE_H */

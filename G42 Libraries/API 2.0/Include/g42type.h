/* group42.h - basic definitions for all group42 programs */
#ifndef G42_TYPE_H
#define G42_TYPE_H
/*	Basic file to be included in all group 42's programs.
	This file includes the following standard libraries:
		stdlib
		stdio + others
	defines the following types:
      uint        an unsigned integer
      int8        a 8 bit signed integer
      uint8       a 8 bit unsigned integer
      byte        a uint8
		int16			a 16 bit signed integer
		int32			a 32 bit signed integer
		uint16		a 32 bit unsigned integer
		uint32		a 32 bit unsigned integer
		bool			if not defined, an int - only true or false
	and the following constants:
      true      1
      false     0
	*/
/*
	detect which operating system we are in, and set appropriate variables
*/
#ifdef __MSDOS__
#  define MSWIN
#  define INT_IS_16
#  define INTEL_BYTE_ORDER
#endif
#ifdef __WIN32__
#  define MSWIN
#  define INTEL_BYTE_ORDER
#endif
#ifndef MSWIN
#  define FAR
#  define HUGE
#  define XWIN
#endif
#if defined (__MWERKS__) || defined (MAC)	// (MWERKS means MetroWerks Code Warrior)
#define MACOS
#undef XWIN
#endif
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#ifdef MSWIN
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
#ifdef MACOS		// (m.2)
typedef unsigned int uint;
typedef unsigned char byte;
typedef int bool;
//  I haven't found these in Metrowerks yet..  D.I.
#define min(x,y) ((x > y) ? y : x)
#define max(x,y) ((x > y) ? x : y)
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;
#endif		// MACOS
#ifndef MSWIN
#ifdef INT_IS_16
typedef int int16;
typedef unsigned int uint16;
#else
typedef short int16;
typedef unsigned short uint16;
#endif
#ifdef INT_IS_16
typedef long int32;
typedef unsigned long uint32;
#else
/* protect ourselves against 64 bit machines (alpha) */
typedef int int32;
typedef unsigned int uint32;
#endif
typedef signed char int8;
typedef unsigned char uint8;
#endif
typedef uint8 byte;
typedef unsigned int uint;
#if	0
typedef int bool;
const int True = 1;
const int False = 0;
#if !defined(__WINDOWS_H)
#  define true 1
#  define false 0
#endif
#endif
/* do not put anything past this line */
#endif /* G42_TYPE_H */

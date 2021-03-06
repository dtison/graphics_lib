// gidlzw.h - lzw decompresser
/*************************************************************
	File:          gidlzw.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   Decompresses lzw streams
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
				Jan 15 1997 Guy Converted to 3.0 libs
***************************************************************/
#ifndef GIDLZW_H
#define GIDLZW_H
#include "gidcomp.h"
#define LZW_NO_CODE -1
class G42LzwException : public G42Exception
{
public:
	G42LzwException() : G42Exception(0) {}
	virtual ~G42LzwException() {}
};
class G42LzwDecompressor : public G42Decompressor
{
	public:
		enum G42LzwType {LzwGif, LzwTiff};
		G42LzwDecompressor(G42DecompressorOutput & output, uintf row_bytes,
			G42LzwType type = LzwTiff, intf code_size = 8);
		virtual ~G42LzwDecompressor(void);
		virtual void ProcessBuffer(void);
		void SetCodeSize(intf code_size);
	protected:
		intf Bits;
		intf Bits2;  	    /* Bits plus 1 */
		intf CodeSize;       /* Current code size in bits */
		intf CodeSize2;      /* Next codesize */
		intf NextCode;       /* Next available table entry */
		intf ThisCode;       /* Code being expanded */
		intf OldToken;       /* Last symbol decoded */
		intf CurrentCode;    /* Code just read */
		intf OldCode;        /* Code read before this one */
		intf BitsAvailable;  /* Number of bits left in *p */
		intf BitsNeeded;     /* Number of bits needed to complete code */
		intf SaveCode;       /* Code from previous byte */
		intf Shift;
		intf Mask;
		byte * u;            /* Stack pointer into firstcodestack */
		byte FirstCodeStack[4096];  /* Stack for first codes */
		byte LastCodeStack[4096];   /* Statck for previous code */
		intf CodeStack[4096];        /* Stack for links */
		byte InputBuffer[256];
		G42LzwType LzwType;
};
#endif // GIDLZW_H

// g42lzw.h - lzw decompresser
/*************************************************************
	File:          g42lzw.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   Decompresses lzw streams
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#ifndef G42LZW_H
#define G42LZW_H
#include "g42comp.h"
#define LZW_NO_CODE -1
class G42Lzw : public G42Compression
{
	public:
		enum G42LzwType {LzwGif, LzwTiff};
		G42Lzw(int row_bytes, G42LzwType type = LzwTiff, int code_size = 8);
		virtual ~G42Lzw();
		virtual void ProcessBuffer(byte * input_buffer, int num_input_buffer);
	protected:
		int Bits;
		int Bits2;  	    /* Bits plus 1 */
		int CodeSize;       /* Current code size in bits */
		int CodeSize2;      /* Next codesize */
		int NextCode;       /* Next available table entry */
		int ThisCode;       /* Code being expanded */
		int OldToken;       /* Last symbol decoded */
		int CurrentCode;    /* Code just read */
		int OldCode;        /* Code read before this one */
		int BitsAvailable;  /* Number of bits left in *p */
		int BitsNeeded;     /* Number of bits needed to complete code */
		int SaveCode;       /* Code from previous byte */
		int Shift;
		int Mask;
		byte * LineBuffer;
		int BytePtr;
		byte * u;            /* Stack pointer into firstcodestack */
		byte FirstCodeStack[4096];  /* Stack for first codes */
		byte LastCodeStack[4096];   /* Statck for previous code */
		int CodeStack[4096];        /* Stack for links */
      byte InputBuffer[256];
		G42LzwType LzwType;
};
#endif // G42LZW_H

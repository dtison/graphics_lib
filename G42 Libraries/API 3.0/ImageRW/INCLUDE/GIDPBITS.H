// gidpbits.h - pack bits decompression
/*************************************************************
	File:          gidpbits.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   Derived class of G42Decompressor that handles PackBits
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#ifndef GIDPBITS_H
#define GIDPBITS_H
#include "gidcomp.h"
class G42PackBitsDecompressor : public G42Decompressor
{
	public:
		G42PackBitsDecompressor(G42DecompressorOutput & output, uintf row_bytes);
		virtual ~G42PackBitsDecompressor();
		virtual void ProcessBuffer();
	protected:
		uintf Count;
		enum G42PackBitsMode {NewMode, RunMode, SequenceMode};
		G42PackBitsMode PackBitsMode;
};
#endif // GIDPBITS_H

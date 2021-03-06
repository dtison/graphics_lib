// gicpbits.h - pack bits compression
/*************************************************************
	File:          g42wpbts.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   Derived class of G42Compressor that handles PackBits
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#ifndef GICPBITS_H
#define GICPBITS_H
#include "gicomp.h"
class G42PackBitsCompressor : public G42Compressor
{
	public:
		G42PackBitsCompressor(G42CompressorOutput & output,
			uintf output_buffer_size = 4096);
		virtual ~G42PackBitsCompressor();
		virtual void ProcessBuffer(byte * buffer, uintf buffer_size);
};
#endif // G42PBITS_H

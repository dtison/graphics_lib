// g42wpbts.h - pack bits compression
/*************************************************************
	File:          g42wpbts.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   Derived class of G42WriteCompression that handles PackBits
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#ifndef G42WPBTS_H
#define G42WPBTS_H
#include "g42wcomp.h"
class G42WritePackBits : public G42WriteCompression
{
	public:
		G42WritePackBits(G42WriteCompressionOutput * output,
			uint output_buffer_size = 4096);
		virtual ~G42WritePackBits();
		virtual void ProcessBuffer(byte * buffer, uint buffer_size);
};
#endif // G42PBITS_H

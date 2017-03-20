// g42pbits.h - pack bits compression
/*************************************************************
	File:          g42pbits.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   Derived class of G42Compression that handles PackBits
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#ifndef G42PBITS_H
#define G42PBITS_H
#include "g42comp.h"
class G42PackBits : public G42Compression
{
	public:
		G42PackBits(int row_bytes);
		virtual ~G42PackBits();
		virtual void ProcessBuffer(byte * buffer, int buffer_size);
	protected:
		int Count;
		enum G42PackBitsMode {NewMode, RunMode, SequenceMode};
		G42PackBitsMode PackBitsMode;
};
#endif // G42PBITS_H

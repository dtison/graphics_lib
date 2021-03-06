// gidpbits.cpp - pack bits compression
/*************************************************************
	File:          gidpbits.cpp
	Copyright (c) 1996, Group 42, Inc.
	Description:   Derived class of G42Compression that handles PackBits
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "gidpbits.h"
G42PackBitsDecompressor::G42PackBitsDecompressor(
	G42DecompressorOutput & output, uintf row_bytes)
:
	G42Decompressor(output, row_bytes), PackBitsMode(NewMode), Count(0)
{
}
G42PackBitsDecompressor::~G42PackBitsDecompressor(void)
{
}
void
G42PackBitsDecompressor::ProcessBuffer(void)
{
	while (InputBufferLeft())
	{
		switch (PackBitsMode)
		{
			case NewMode:
			{
				byte b = GetByte();
				if (b > 0x80)
				{
					Count = ((~b) & 0xff) + 2;
					PackBitsMode = RunMode;
				}
				else if (b < 0x80)
				{
					Count = (b & 0xff) + 1;
					PackBitsMode = SequenceMode;
				}
			}
			break;
			case RunMode:
			{
				byte b = GetByte();
				PutRun(b, Count);
				if (IsDone())
					return;
				PackBitsMode = NewMode;
			}
			break;
			case SequenceMode:
			{
				uintf size = min((uintf)(InputEnd - InputPtr), Count);
				PutSequence(InputPtr, size);
				if (IsDone())
            	return;
				InputPtr += size;
				Count -= size;
				if (!Count)
				{
					PackBitsMode = NewMode;
				}
			}
			break;
		}
	}
}

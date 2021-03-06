// g42pbits.cpp - pack bits compression
/*************************************************************
	File:          g42pbits.cpp
	Copyright (c) 1996, Group 42, Inc.
	Description:   Derived class of G42Compression that handles PackBits
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "g42itype.h"
#include "g42pbits.h"
G42PackBits::G42PackBits(int row_bytes) :
	G42Compression(row_bytes), PackBitsMode(NewMode), Count(0)
{
}
G42PackBits::~G42PackBits()
{
}
void
G42PackBits::ProcessBuffer(byte * buffer, int buffer_size)
{
	while (buffer_size)
	{
		switch (PackBitsMode)
		{
			case NewMode:
			{
				byte b = *buffer++;
				buffer_size--;
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
				byte b = *buffer++;
				buffer_size--;
				PutRun(b, Count);
            if (IsDone())
            	return;
				PackBitsMode = NewMode;
			}
			break;
			case SequenceMode:
			{
				int size = min(buffer_size, Count);
				PutSequence(buffer, size);
            if (IsDone())
            	return;
				buffer_size -= size;
				buffer += size;
				Count -= size;
				if (!Count)
					PackBitsMode = NewMode;
			}
			break;
		}
	}
}

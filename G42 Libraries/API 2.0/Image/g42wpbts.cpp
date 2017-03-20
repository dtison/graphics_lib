// g42wpbts.cpp - pack bits compression
/*************************************************************
	File:          g42wpbts.cpp
	Copyright (c) 1996, Group 42, Inc.
	Description:   Derived class of G42Compression that handles PackBits
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "g42itype.h"
#include "g42wpbts.h"
G42WritePackBits::G42WritePackBits(
	G42WriteCompressionOutput * output,	uint output_buffer_size)
:
	G42WriteCompression(output, output_buffer_size)
{
}
G42WritePackBits::~G42WritePackBits()
{
}
void
G42WritePackBits::ProcessBuffer(byte * buffer, uint buffer_size)
{
	byte * bp = buffer + 1;
	uint bytes_left = buffer_size - 1;
	byte * start_sequence = buffer;
	int count = 1;
	bool mode_defined = false;
	bool repeat_mode = false;
	while (bytes_left)
	{
		bool same = (*bp == *(bp - 1));
		if (!mode_defined)
		{
			repeat_mode = same;
			mode_defined = true;
			count++;
			bytes_left--;
			bp++;
		}
		else if (same && repeat_mode && count < 128)
		{
			count++;
			bytes_left--;
			bp++;
		}
		else if (!same && !repeat_mode && count < 128)
		{
			count++;
			bytes_left--;
			bp++;
		}
		else
		{
			if (repeat_mode)
			{
				PutByte((byte)(256 - (count - 1)));
				PutByte(*start_sequence);
				mode_defined = false;
				start_sequence = bp;
				count = 1;
			}
			else if (count == 128)
			{
				PutByte((byte)(count - 1));
				PutSequence(start_sequence, count);
				mode_defined = false;
				start_sequence = bp;
				count = 1;
			}
			else // found repeat, don't use this or last pixel and set count = 2
			{
				PutByte((byte)(count - 2));
				PutSequence(start_sequence, count - 1);
				mode_defined = true;
				repeat_mode = true;
				start_sequence = bp - 1;
				count = 2;
			}
			bytes_left--;
			bp++;
		}
	}
	if (!mode_defined || !repeat_mode)
	{
		PutByte((byte)(count - 1));
		PutSequence(start_sequence, count);
	}
	else
	{
		PutByte((byte)(256 - (count - 1)));
		PutByte(*start_sequence);
	}
}

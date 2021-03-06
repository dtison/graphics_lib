// g42comp.cpp - compression base class
/*************************************************************
	File:          g42comp.cpp
	Copyright (c) 1996, Group 42, Inc.
	Description:   Base class for compression classes
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "g42itype.h"
#include "g42comp.h"
G42Compression::G42Compression(int row_bytes) :
	RowBytes(row_bytes), Valid(true), Done(false), LineBuffer(0), BytePtr(0)
{
	LineBuffer = new byte [RowBytes];
}
G42Compression::~G42Compression()
{
	if (LineBuffer)
		delete[] LineBuffer;
	LineBuffer = 0;
}
bool
G42Compression::OutputRow(byte *)
{
	return false;
}
void
G42Compression::PutSequence(byte * buffer, uint length)
{
	while (length)
	{
		uint num = min(length, RowBytes - BytePtr);
		memcpy(LineBuffer + BytePtr, buffer, num);
		BytePtr += num;
		length -= num;
		buffer += num;
		if (BytePtr >= RowBytes)
		{
			if (OutputRow(LineBuffer))
         {
         	Done = true;
         	break;
         }
			BytePtr = 0;
		}
	}
}
void
G42Compression::PutRun(byte b, uint number)
{
	while (number)
	{
		uint num = min(number, RowBytes - BytePtr);
		memset(LineBuffer + BytePtr, b, num);
		BytePtr += num;
		number -= num;
		if (BytePtr >= RowBytes)
		{
			if (OutputRow(LineBuffer))
         {
         	Done = true;
         	break;
         }
			BytePtr = 0;
		}
	}
}
G42NoCompression::G42NoCompression(int width) :
	G42Compression(width)
{
}
G42NoCompression::~G42NoCompression()
{
}
void
G42NoCompression::ProcessBuffer(byte * buffer, int buffer_size)
{
	PutSequence(buffer, buffer_size);
}

// gidcomp.cpp - decompression base class
/*************************************************************
	File:          gidcomp.cpp
	Copyright (c) 1996, Group 42, Inc.
	Description:   Base class for decompression classes
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "gtype.h"
#include "gidcomp.h"
G42Decompressor::G42Decompressor(G42DecompressorOutput & output,
	uintf row_bytes)
:
	RowBytes(row_bytes), Done(false), Output(output),
	OutputBuffer(0), OutputEnd(0), OutputPtr(0)
{
}
G42Decompressor::~G42Decompressor()
{
}
void
G42Decompressor::SetOutputBuffer(byte * buffer)
{
	OutputBuffer = buffer;
	OutputPtr = OutputBuffer;
	OutputEnd = OutputBuffer + RowBytes;
}
void
G42Decompressor::PutSequence(const byte * buffer, uintf length)
{
	while (length && !Done)
	{
		uintf num = min(length, (uintf)(OutputEnd - OutputPtr));
		memcpy(OutputPtr, buffer, num);
		OutputPtr += num;
		length -= num;
		buffer += num;
		CheckOutput();
	}
}
void
G42Decompressor::PutRun(byte b, uintf length)
{
	while (length && !Done)
	{
		uintf num = min(length, (uintf)(OutputEnd - OutputPtr));
		memset(OutputPtr, b, num);
		OutputPtr += num;
		length -= num;
		CheckOutput();
	}
}
uintf
G42Decompressor::ProcessBuffer(const byte * buffer, uintf buffer_size)
{
	InputPtr = buffer;
	InputEnd = buffer + buffer_size;
	ProcessBuffer();
	return (uintf)(InputPtr - buffer);
}
G42NoDecompressor::G42NoDecompressor(
	G42DecompressorOutput & output, uintf row_bytes)
:
	G42Decompressor(output, row_bytes)
{
}
G42NoDecompressor::~G42NoDecompressor()
{
}
void
G42NoDecompressor::ProcessBuffer()
{
	PutSequence(InputPtr, (uintf)(InputEnd - InputPtr));
}

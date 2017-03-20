// gicomp.cpp - compression base class
/*************************************************************
	File:          gicomp.cpp
	Copyright (c) 1996, Group 42, Inc.
	Description:   Base class for compression classes
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "gicomp.h"
G42Compressor::G42Compressor(G42CompressorOutput & output,
	uintf output_buffer_size)
:
	OutputBufferSize(output_buffer_size), Valid(true), Done(false),
	OutputBuffer(0), BytePtr(0), Output(output)
{
	OutputBuffer = new byte [OutputBufferSize];
}
G42Compressor::~G42Compressor()
{
	if (OutputBuffer)
		delete[] OutputBuffer;
	OutputBuffer = 0;
}
void
G42Compressor::PutSequence(byte * buffer, uintf length)
{
	while (length)
	{
		uintf num = min(length, OutputBufferSize - BytePtr);
		memcpy(OutputBuffer + BytePtr, buffer, num);
		BytePtr += num;
		length -= num;
		buffer += num;
		if (BytePtr >= OutputBufferSize)
		{
			HandleOutputBuffer(OutputBuffer, OutputBufferSize);
			BytePtr = 0;
		}
	}
}
void
G42Compressor::PutRun(byte b, uintf number)
{
	while (number)
	{
		uintf num = min(number, OutputBufferSize - BytePtr);
		memset(OutputBuffer + BytePtr, b, num);
		BytePtr += num;
		number -= num;
		if (BytePtr >= OutputBufferSize)
		{
			HandleOutputBuffer(OutputBuffer, OutputBufferSize);
			BytePtr = 0;
		}
	}
}
void
G42Compressor::Flush(void)
{
	if (BytePtr)
	{
		HandleOutputBuffer(OutputBuffer, BytePtr);
		BytePtr = 0;
	}
}
G42NoCompressor::G42NoCompressor(
	G42CompressorOutput & output,	uintf output_buffer_size)
:
	G42Compressor(output, output_buffer_size)
{
}
G42NoCompressor::~G42NoCompressor()
{
}
void
G42NoCompressor::ProcessBuffer(byte * buffer, uintf buffer_size)
{
	PutSequence(buffer, buffer_size);
}
G42CompressorOutput::G42CompressorOutput()
{
}
G42CompressorOutput::~G42CompressorOutput()
{
}

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
#include "g42wcomp.h"
G42WriteCompression::G42WriteCompression(G42WriteCompressionOutput * output,
	uint output_buffer_size)
:
	OutputBufferSize(output_buffer_size), Valid(true), Done(false),
	OutputBuffer(0), BytePtr(0), Output(output)
{
	OutputBuffer = new byte [OutputBufferSize];
}
G42WriteCompression::~G42WriteCompression()
{
	if (OutputBuffer)
		delete[] OutputBuffer;
	OutputBuffer = 0;
}
void
G42WriteCompression::HandleOutputBuffer(byte * buffer, uint length)
{
	Output->OutputBuffer(buffer, length);
}
void
G42WriteCompression::PutSequence(byte * buffer, uint length)
{
	while (length)
	{
		uint num = min(length, OutputBufferSize - BytePtr);
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
G42WriteCompression::PutRun(byte b, uint number)
{
	while (number)
	{
		uint num = min(number, OutputBufferSize - BytePtr);
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
G42WriteCompression::Flush(void)
{
	if (BytePtr)
	{
		HandleOutputBuffer(OutputBuffer, BytePtr);
		BytePtr = 0;
	}
}
G42WriteNoCompression::G42WriteNoCompression(
	G42WriteCompressionOutput * output,	uint output_buffer_size)
:
	G42WriteCompression(output, output_buffer_size)
{
}
G42WriteNoCompression::~G42WriteNoCompression()
{
}
void
G42WriteNoCompression::ProcessBuffer(byte * buffer, uint buffer_size)
{
	PutSequence(buffer, buffer_size);
}
G42WriteCompressionOutput::G42WriteCompressionOutput()
{
}
G42WriteCompressionOutput::~G42WriteCompressionOutput()
{
}

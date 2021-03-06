// ribase.cpp - raster image base class
/*************************************************************
	File:          g42ribas.cpp
	Copyright (c) 1996, Group 42, Inc.
	Description:   Base class for reading raster files
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "g42itype.h"
#include "g42wibas.h"
G42RasterWriterBase::G42RasterWriterBase(G42RasterImageWriter & parent) :
	Parent(parent), Width(0), Height(0), RowBytes(0), Depth(0),
	CurrentRow(0), Valid(true), IntegerFormat(Motorola),
	OutputBuffer(0), OutputBufferSize(4096), OutputBufferLeft(0),
   NextOutput(0)
{
	OutputBuffer = new byte [OutputBufferSize];
	if (!OutputBuffer)
	{
		Valid = false;
      return;
	}
	OutputBufferLeft = OutputBufferSize;
   NextOutput = OutputBuffer;
}
G42RasterWriterBase::~G42RasterWriterBase()
{
	if (OutputBuffer)
	{
		delete OutputBuffer;
      OutputBuffer = false;
	}
	Valid = false;
}
bool
G42RasterWriterBase::WriteInfo(G42ImageInfo * info)
{
	Width = info->Width;
	Height = info->Height;
	Depth = info->PixelDepth;
	RowBytes = ((Width * (uint32)Depth + 7) >> 3);
	return Valid;
}
bool
G42RasterWriterBase::WriteRow(const byte * row, const byte *)
{
	uint32 bytes_left = RowBytes;
	const byte * row_ptr = row;
	while (bytes_left)
	{
		uint32 copy_bytes = min(bytes_left, (uint32)OutputBufferLeft);
		PutBytes(row_ptr, (uint)copy_bytes);
		row_ptr += copy_bytes;
		bytes_left -= copy_bytes;
	}
	CurrentRow++;
	return Valid;
}
uint32
G42RasterWriterBase::NextRow(void)
{
	return CurrentRow;
}
bool
G42RasterWriterBase::NeedRow(void)
{
	return (CurrentRow < Height);
}
bool
G42RasterWriterBase::WriteEnd(G42ImageInfo *)
{
	PutEnd();
	return Valid;
}
void
G42RasterWriterBase::PutBytes(const byte * buffer, uint size)
{
	while (size)
	{
		uint copy_size = min(size, OutputBufferLeft);
		memcpy(NextOutput, buffer, copy_size);
		OutputBufferLeft -= copy_size;
		size -= copy_size;
		buffer += copy_size;
		if (OutputBufferLeft)
			NextOutput += copy_size;
		else
			FlushBuffer(); // resets NextOutput
	}
}
void
G42RasterWriterBase::PutByteRun(byte value, uint number)
{
	while (number)
	{
		uint copy_size = min(number, OutputBufferLeft);
		memset(NextOutput, value, copy_size);
		OutputBufferLeft -= copy_size;
		number -= copy_size;
		if (OutputBufferLeft)
			NextOutput += copy_size;
		else
			FlushBuffer(); // resets NextOutput
	}
}
void
G42RasterWriterBase::FlushBuffer(void)
{
	if (OutputBufferLeft < OutputBufferSize)
	{
		Parent.Output(OutputBuffer, OutputBufferSize - OutputBufferLeft);
		OutputBufferLeft = OutputBufferSize;
		NextOutput = OutputBuffer;
	}
}
void
G42RasterWriterBase::PutEnd(void)
{
	FlushBuffer();
	Parent.OutputFinished();
}
void
G42RasterWriterBase::PutUInt32(uint32 number, G42IntType type)
{
	if (type == Default)
		type = IntegerFormat;
	if (type == Motorola)
	{
		PutByte((byte)((number >> 24) & 0xff));
		PutByte((byte)((number >> 16) & 0xff));
		PutByte((byte)((number >> 8) & 0xff));
		PutByte((byte)((number) & 0xff));
	}
	else if (type == Intel)
	{
		PutByte((byte)((number) & 0xff));
		PutByte((byte)((number >> 8) & 0xff));
		PutByte((byte)((number >> 16) & 0xff));
		PutByte((byte)((number >> 24) & 0xff));
	}
}
void
G42RasterWriterBase::PutUInt16(uint16 number, G42IntType type)
{
	if (type == Default)
		type = IntegerFormat;
	if (type == Motorola)
	{
		PutByte((byte)((number >> 8) & 0xff));
		PutByte((byte)((number) & 0xff));
	}
	else if (type == Intel)
	{
		PutByte((byte)((number) & 0xff));
		PutByte((byte)((number >> 8) & 0xff));
	}
}
int
G42RasterWriterBase::GetDepth(void)
{
	return Depth;
}

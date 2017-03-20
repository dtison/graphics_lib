// g42pcx.cpp - Pcx File
/*************************************************************
	File:          g42pcx.cpp
	Copyright (c) 1996, Group 42, Inc.
	Description:   Derived class of g42ribase that handles Pbm files
   Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
      Code   Date   Name and Description
***************************************************************/
#include "girxwd.h"
static char pattern[2] = {0, 0};
G42ImageReaderXwd::G42ImageReaderXwd(G42HeapManager & heap,
	G42Notifier & notifier, G42File * file, G42BaseOptions * options)
:
	G42ImageReaderBase(heap, notifier, file, options),
	ReadMode(ReadHeaderSizeMode),
	CurrentRow(0), CurrentPalette(0)
{
	// we use testtype to be sure
	SetInfo(G42XwdTypeId, "xwd", "Xwd", pattern, 2);
}
G42ImageReaderXwd::G42ImageReaderXwd(void)
:
	G42ImageReaderBase(), ReadMode(ReadHeaderSizeMode),
	CurrentRow(0), CurrentPalette(0)
{
	// we use testtype to be sure
	SetInfo(G42XwdTypeId, "xwd", "Xwd", pattern, 2);
}
G42ImageReaderXwd::~G42ImageReaderXwd(void)
{
}
void
G42ImageReaderXwd::ReadHeaderSize()
{
	if (!CheckBuffer(4, true))
	{
		SaveBuffer();
		return;
	}
	IntegerFormat = Motorola;
	HeaderSize = (uintf)GetUInt32();
	if (HeaderSize > 1000) // if this is true, this isn't a XWD
	{
		throw G42IRWNotAnImageException();
	}
	ReadMode = ReadHeaderMode;
}
void
G42ImageReaderXwd::ReadHeader()
{
	if (!CheckBuffer(HeaderSize - 4, true))
	{
		SaveBuffer();
		return;
	}
	Version = (uintf)GetUInt32();
	if (Version != 7)
	{
		ReadMode = ReadErrorMode;
		throw G42IRWInvalidImageException();
	}
	SkipData(4);
	Depth = (uintf)GetUInt32();
	if (Depth != 1 && Depth != 4 && Depth != 8 && Depth != 32)
	{
		ReadMode = ReadErrorMode;
		throw G42IRWInvalidImageException();
	}
	Width = GetUInt32();
	Height = GetUInt32();
	if (!Width || !Height)
	{
		ReadMode = ReadErrorMode;
		throw G42IRWInvalidImageException();
	}
	SkipData(16);
	uintf pad_bytes = ((uintf)GetUInt32() >> 3);
	SkipData(4);
	uint32 bytes_per_line = GetUInt32();
	if (!bytes_per_line)
	{
		bytes_per_line = ((Width * (uint32)Depth + 7) >> 3);
		if (pad_bytes == 2)
			bytes_per_line = ((bytes_per_line + 1) & ~1);
		else if (pad_bytes == 4)
			bytes_per_line = ((bytes_per_line + 3) & ~3);
	}
	RowBytes = bytes_per_line;
	VisualType = (uintf)GetUInt32();
	if (VisualType > 5)
	{
		ReadMode = ReadErrorMode;
		throw G42IRWInvalidImageException();
	}
	SkipData(20);
	NumPalette = (uintf)GetUInt32();
	if (NumPalette > 256)
	{
		ReadMode = ReadErrorMode;
		throw G42IRWInvalidImageException();
	}
	if (NumPalette || Depth > 8)
	{
		Color = true;
	}
	else
	{
		Color = false;
	}
	SkipData(HeaderSize - 80);
	SetupDescriptor();
	if (NumPalette)
	{
		ReadMode = ReadPaletteMode;
		CurrentPalette = 0;
	}
	else
	{
		ReadMode = ReadHasInfoMode;
	}
}
void
G42ImageReaderXwd::ReadPalette(void)
{
	// read until palette is complete or buffer is empty
	CheckBuffer(12 * NumPalette, true);
	while (BufferSize >= 12 && CurrentPalette < NumPalette)
	{
		SkipData(4);
		ImagePalette[CurrentPalette].Red = (byte)(GetUInt16() >> 8);
		ImagePalette[CurrentPalette].Green = (byte)(GetUInt16() >> 8);
		ImagePalette[CurrentPalette].Blue = (byte)(GetUInt16() >> 8);
		SkipData(2);
		CurrentPalette++;
	}
	if (CurrentPalette == NumPalette)
	{
		ReadMode = ReadHasInfoMode;
	}
	else if (BufferSize)
	{
		SaveBuffer(); // save for rest of data
	}
}
void
G42ImageReaderXwd::ReadHasInfo()
{
	SendDescriptor();
	ReadMode = ReadImageMode;
}
void
G42ImageReaderXwd::ReadImage(void)
{
	ProcessUncompressedData(); // base can handle uncompressed rows
}
void
G42ImageReaderXwd::ConvertRow(void)
{
// this being Xwindow's format, no translation needs to be done for it
#if !defined(XWIN)
	if (Depth > 8)
	{
		if (GetImageDepth() == 24)
		{
			byte * sp = RowBuffer;
			G42Color24Iterator dp(OutputRowPtr);
			for (uintf i = 0; i < Width; i++)
			{
				*dp = G42Color(*sp, *(sp + 1), *(sp + 2));
				dp++;
				sp += 4;
			}
		}
		else
		{
			byte * sp = RowBuffer;
			G42Color32Iterator dp(OutputRowPtr);
			for (uintf i = 0; i < Width; i++)
			{
				*dp = G42Color(*sp, *(sp + 1), *(sp + 2));
				dp++;
				sp += 4;
			}
		}
	}
#endif
	G42ImageReaderBase::ConvertRow();
}
void
G42ImageReaderXwd::BeginProcessing(void)
{
	while (ReadMode != ReadDoneMode && !Finished)
	{
		switch (ReadMode)
		{
			case ReadHeaderSizeMode:
				ReadHeaderSize();
				break;
			case ReadHeaderMode:
				ReadHeader();
				break;
			case ReadPaletteMode:
				ReadPalette();
				break;
			case ReadHasInfoMode:
				ReadHasInfo();
				break;
			case ReadImageMode:
				ReadImage();
				break;
			default:
				ReadMode = ReadDoneMode;
				break;
		}
	}
}
void
G42ImageReaderXwd::SendExtendedInfo(void)
{
	while (ReadMode != ReadDoneMode && !Finished)
	{
		switch (ReadMode)
		{
			case ReadHeaderSizeMode:
				ReadHeaderSize();
				break;
			case ReadHeaderMode:
				ReadHeader();
				break;
			case ReadPaletteMode:
				ReadPalette();
				break;
			case ReadHasInfoMode:
				ReadHasInfo();
				break;
/*
			case ReadImageMode:
				ReadImage();
				break;
*/
			default:
				ReadMode = ReadDoneMode;
				break;
		}
	}
}
void
G42ImageReaderXwd::ProcessStreamData(void)
{
	while (BufferSize)
	{
		switch (ReadMode)
		{
			case ReadHeaderSizeMode:
				ReadHeaderSize();
				break;
			case ReadHeaderMode:
				ReadHeader();
				break;
			case ReadPaletteMode:
				ReadPalette();
				break;
			case ReadHasInfoMode:
				ReadHasInfo();
				break;
			case ReadImageMode:
				ReadImage();
				break;
			default:
				EatBuffer();
				break;
		}
	}
}
bool
G42ImageReaderXwd::TestType(void)
{
	File->Rewind();
	byte buf[8];
	File->Read(buf, 8);
	File->Rewind();
	return TestType(buf, 8);
}
bool
G42ImageReaderXwd::TestType(const byte * buffer, uint32 buffer_size)
{
	if (buffer_size < 5)
	{ // here we guess.  Xwd files have the length in the first
	  // four bytes, and the version in the next four.  The
	  // length is 100 or more.  We give it up to 356 before we
	  // decide it isn't a xwd anymore
		if ((buffer[0] == 0) &&
			(buffer_size < 2 || buffer[1] == 0) &&
			(buffer_size < 3 || buffer[1] <= 1) &&
			(buffer_size < 4 || buffer[1] >= 100))
			return true;
	}
	else
	{
		if ((buffer[4] == 0) &&
			(buffer_size < 6 || buffer[5] == 0) &&
			(buffer_size < 7 || buffer[6] == 0) &&
			(buffer_size < 8 || buffer[7] == 7))
			return true;
	}
	return false;
}

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
#include "g42itype.h"
#include "g42idata.h"
#include "g42xwd.h"
#include "g42iter.h"
G42XwdImage::G42XwdImage(G42RasterImage & parent) :
	G42RasterBase(parent), ReadMode(ReadHeaderSizeMode),
	CurrentRow(0), CurrentPalette(0)
{
	Valid = true;
}
G42XwdImage::~G42XwdImage()
{
}
void
G42XwdImage::ReadHeaderSize()
{
	if (BufferSize < 4)
	{
		SaveBuffer();
		return;
	}
	IntegerFormat = Motorola;
	HeaderSize = (uint)GetUInt32();
	if (HeaderSize > 1000) // if this is true, this isn't a XWD
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		return;
	}
	ReadMode = ReadHeaderMode;
}
void
G42XwdImage::ReadHeader()
{
	if (BufferSize < HeaderSize - 4)
	{
		SaveBuffer();
		return;
	}
	Version = (uint)GetUInt32();
	if (Version != 7)
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		return;
	}
	SkipData(4);
	Depth = (uint)GetUInt32();
	if (Depth != 1 && Depth != 4 && Depth != 8 && Depth != 32)
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		return;
	}
	Width = GetUInt32();
	Height = GetUInt32();
	if (!Width || !Height)
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		return;
	}
	SkipData(16);
	uint pad_bytes = ((uint)GetUInt32() >> 3);
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
	VisualType = (uint)GetUInt32();
	if (VisualType > 5)
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		return;
	}
	SkipData(20);
	NumPalette = (uint)GetUInt32();
	if (NumPalette > 256)
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		return;
	}
	SkipData(HeaderSize - 80);
	if (NumPalette)
	{
		ReadMode = ReadPaletteMode;
		G42Color * pal = new G42Color [NumPalette];
		if (!pal)
		{
			Valid = false;
			ReadMode = ReadErrorMode;
			return;
		}
		Info.ImagePalette.Set(pal);
		CurrentPalette = 0;
	}
	else
	{
		ReadMode = ReadHasInfoMode;
	}
}
void
G42XwdImage::ReadPalette(void)
{
	// read until palette is complete or buffer is empty
	G42Color * pal = Info.ImagePalette;
	while (BufferSize >= 12 && CurrentPalette < NumPalette)
	{
		SkipData(4);
		pal[CurrentPalette].red = (byte)(GetUInt16() >> 8);
		pal[CurrentPalette].green = (byte)(GetUInt16() >> 8);
		pal[CurrentPalette].blue = (byte)(GetUInt16() >> 8);
		SkipData(2);
		CurrentPalette++;
	}
	if (CurrentPalette == NumPalette)
	{
		ReadMode = ReadHasInfoMode;
	}
	else if (BufferSize)
		SaveBuffer(); // save for rest of data
}
void
G42XwdImage::ReadHasInfo()
{
	Info.Width = Width;
	Info.Height = Height;
	if (Depth > 8)
		Info.PixelDepth = 8 * TrueColorPixelSize;
	else
		Info.PixelDepth = Depth;
	Info.NumPalette = NumPalette;
	if (VisualType < 2)
		Info.ColorType = G42ImageInfo::ColorTypeGrayscale;
	else if (VisualType < 4)
		Info.ColorType = G42ImageInfo::ColorTypePalette;
	else
		Info.ColorType = G42ImageInfo::ColorTypeTrueColor;
	Info.ImageType = Xwd;
	Info.Compressed = false;
	Parent.HasInfo(Info);
	ReadMode = ReadImageMode;
}
void
G42XwdImage::ReadImage(void)
{
	ProcessUncompressedData(); // base can handle uncompressed rows
}
void
G42XwdImage::OutputRow(void)
{
// this being Xwindow's format, no translation needs to be done for it
#if !defined(XWIN)
	if (Depth > 8)
	{
		byte * sp = RowBuffer;
		G42Byte24BitIterator dp(RowBuffer);
		for (uint i = 0; i < Width; i++)
		{
			dp.Set(*(sp + 2), *(sp + 1), *(sp + 0));
			dp.Increment();
			sp += 4;
		}
	}
#endif
	G42RasterBase::OutputRow();
	CurrentRow++;
	if (CurrentRow >= Height)
	{
		EatBuffer();
		ReadMode = ReadDoneMode;
		ForceFileEnd();
	}
}
void
G42XwdImage::ProcessData(byte * buffer, size_t buffer_size)
{
	RestoreBuffer(buffer, buffer_size);
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
bool G42IsXwd(byte * buffer, size_t buffer_size)
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

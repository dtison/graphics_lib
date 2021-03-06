// g42bmp.cpp - Bmp File
/*************************************************************
	File:          g42bmp.h
   Copyright (c) 1996, Group 42, Inc.
	Description:   Derived class of g42ribase that handles bmp files
   Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
   Modification History:
      Code   Date   Name and Description
***************************************************************/
#include "g42itype.h"
#include "g42bmp.h"
#include "g42idata.h"
#include "g42iter.h"
G42BmpImage::G42BmpImage(G42RasterImage & parent) :
	G42RasterBase(parent), ReadMode(ReadHeaderMode),
	CurrentRow(0)
{
}
G42BmpImage::~G42BmpImage()
{
}
// count rows, and mark done when last row is output
void
G42BmpImage::OutputRow(void)
{
// this being mswindow's format, no translation needs to be done for it
#if !defined(MSWIN)
	if (Depth > 8)
	{
		byte * sp = RowBuffer + (Width - 1) * 3;
		G42Byte24BitIterator dp(RowBuffer);
		dp.Forward(Width - 1);
		for (uint i = 0; i < Width; i++)
		{
			dp.Set(*sp, *(sp + 1), *(sp + 2));
			dp.Decrement();
			sp -= 3;
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
G42BmpImage::ProcessData(byte * buffer, size_t buffer_size)
{
	RestoreBuffer(buffer, buffer_size);
	while (BufferSize)
	{
		switch (ReadMode)
		{
			case ReadHeaderMode:
				ReadHeader();
				break;
			case ReadPaletteMode:
				ReadPalette();
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
void
G42BmpImage::ReadHeader(void)
{
   if (BufferSize < 80) // 80 is worst case behavior
	{
		SaveBuffer();
		return;
	}
  	IntegerFormat = Intel;
	// check header ('BM')
	if (GetByte() != 0x42)
		return;
	if (GetByte() != 0x4d)
		return;
	// skip to start of good info
	SkipData(12);
	// number of bytes of header, used to tell which version bmp
	uint offset = (uint)GetUInt32();
	if (offset == 40)
		osType = Win3x;
	else if (offset == 12)
		osType = OS21x;
	else if (offset == 64)
		osType = OS22x;
	else
	{
		osType = Undefined;
		Valid = false;
		ReadMode = ReadErrorMode;
		return;
	}
	if (osType == OS21x)
	{
		Width = GetUInt16();
		Height = GetUInt16();
		SkipData(2);
		Depth = GetUInt16();
		if (Depth <= 8)
			NumPalette = (1 << Depth);
		else
			NumPalette = 0;
	}
	else
	{
		Width = GetUInt32();
		Height = GetUInt32();
		SkipData(2);
		Depth = GetUInt16();
		uint compression = (uint)GetUInt32(); // we only do uncompressed for now
		if (compression != 0) /* if not BI_RGB */
		{
			Valid = false;
			ReadMode = ReadErrorMode;
			return;
		}
		if (osType == OS22x) // skip to palette or image
			SkipData(44);
		else
			SkipData(20);
	}
	if (Depth <= 8)
		NumPalette = (1 << Depth);
	else
	{
		NumPalette = 0;
	}
	RowBytes = ((Width * Depth + 7) >> 3);
	if (RowBytes & 3) // expand to 32 bit boundries
		RowBytes += (4 - (RowBytes & 3));
	Color = true; // bmp's are always paletted files
	TopLineFirst = false; // bmp's are bottom up files
	if (NumPalette)
	{
		G42Color * pal = new G42Color[NumPalette]; // need to read in palette
		if (!pal)
		{
			Valid = false;
			ReadMode = ReadErrorMode;
			return;
		}
		Info.ImagePalette.Set(pal);
	}
	Valid = true;
	CurrentPalette = 0;
	Info.Width = Width;
	Info.Height = Height;
	if (Depth > 8)
		Info.PixelDepth = 8 * TrueColorPixelSize;
	else
		Info.PixelDepth = Depth;
	Info.NumPalette = NumPalette;
	if (Depth > 8)
		Info.ColorType = G42ImageInfo::ColorTypeTrueColor;
	else
		Info.ColorType = G42ImageInfo::ColorTypePalette;
	if (osType == Win3x)
		Info.ImageType = BmpWin;
	else
		Info.ImageType = BmpOS2;
	Info.Compressed = false;
	if (NumPalette)
		ReadMode = ReadPaletteMode;
	else
	{
		ReadMode = ReadImageMode; // no palette, so info is complete
		Parent.HasInfo(Info);         // give user the file dimensions
	}
}
void
G42BmpImage::ReadPalette(void)
{
	// read until palette is complete or buffer is empty
	while (BufferSize >= 4 && CurrentPalette < NumPalette)
	{
		((G42Color *)(Info.ImagePalette))[CurrentPalette].blue = GetByte();
		((G42Color *)(Info.ImagePalette))[CurrentPalette].green = GetByte();
		((G42Color *)(Info.ImagePalette))[CurrentPalette].red = GetByte();
		if (osType == Win3x || osType == OS22x)
			SkipData();
		CurrentPalette++;
	}
	if (CurrentPalette == NumPalette)
	{
		ReadMode = ReadImageMode;
		Parent.HasInfo(Info);
	}
	else if (BufferSize)
		SaveBuffer(); // save for rest of data
}
void
G42BmpImage::ReadImage(void)
{
	ProcessUncompressedData(); // base can handle uncompressed rows
}
// key for bmp files is the letters 'BM' in the first two characters
bool G42IsBmp(byte * buffer, size_t buffer_size)
{
	if ((buffer[0] == 0x42) &&
		(buffer_size < 2 || buffer[1] == 0x4d))
		return true;
	return false;
}

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
#include "g42iter.h"
#include "g42ras.h"
G42RasImage::G42RasImage(G42RasterImage & parent) :
	G42RasterBase(parent), ReadMode(ReadHeaderMode), ColorOrderRGB(false),
	CurrentPalette(0), RunCount(0), CurrentRow(0),
	DecompressionMode(StartDecompression)
{
	Valid = true;
}
G42RasImage::~G42RasImage()
{
}
void
G42RasImage::ProcessData(byte * buffer, size_t buffer_size)
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
G42RasImage::ReadHeader()
{
	if (BufferSize < 32)
	{
		SaveBuffer(); // save for rest of data
		return;
	}
	IntegerFormat = Motorola;
	uint32 magic = GetUInt32();
	if (magic != (uint32)0x59a66a95L && magic != (uint32)0x956aa659L)
	{
		Valid = false;
		return;
	}
	if (magic == (uint32)0x956aa659L)
		IntegerFormat = Intel; // perhaps for x86 of Solaris
	Width = GetUInt32();
	Height = GetUInt32();
	Depth = GetUInt32();
	SkipData(4);
	uint32 type = GetUInt32();
	uint32 mtype = GetUInt32();
	uint32 palette_length = GetUInt32();
	if (!Width || !Height || type > 3 || mtype > 1)
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		return;
	}
	if (Depth != 1 && Depth != 8 && Depth != 24 && Depth != 32)
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		return;
	}
	RowBytes = ((Width * Depth + 7) >> 3);
	RowBytes = ((RowBytes + 1) & ~1L);
	if (type == 2)
		Info.Compressed = true;
	else
		Info.Compressed = false;
	if (type == 3)
		ColorOrderRGB = true;
	else
		ColorOrderRGB = false;
	DecompressionMode = StartDecompression;
	if (mtype == 1 && Depth <= 8)
		NumPalette = palette_length / 3;
	if (NumPalette > 256)
		NumPalette = 256;
	Info.Width = Width;
	Info.Height = Height;
	Info.ImageType = Ras;
	if (Depth <= 8)
		Info.PixelDepth = Depth;
	else
		Info.PixelDepth = 8 * TrueColorPixelSize;
	if (Depth > 8)
		Info.ColorType = G42ImageInfo::ColorTypeTrueColor;
	else if (NumPalette)
		Info.ColorType = G42ImageInfo::ColorTypePalette;
	else
		Info.ColorType = G42ImageInfo::ColorTypeGrayscale;
	Info.NumPalette = NumPalette;
	if (NumPalette && NumPalette <= 256)
	{
		G42Color * pal = new G42Color [NumPalette];
		Info.ImagePalette.Set(pal);
		CurrentPalette = 0;
		ReadMode = ReadPaletteMode;
	}
	else
	{
		Parent.HasInfo(Info);
		ReadMode = ReadImageMode;
	}
}
void
G42RasImage::ReadPalette(void)
{
	// read until palette is complete or buffer is empty
	G42Color * pal = Info.ImagePalette;
	while (BufferSize && CurrentPalette < NumPalette * 3)
	{
		if (CurrentPalette < NumPalette)
			pal[CurrentPalette].red = GetByte();
		else if (CurrentPalette < (NumPalette << 1))
			pal[CurrentPalette - NumPalette].green = GetByte();
		else
			pal[CurrentPalette - (NumPalette << 1)].blue = GetByte();
		CurrentPalette++;
	}
	if (CurrentPalette == NumPalette * 3)
	{
		ReadMode = ReadImageMode;
		Parent.HasInfo(Info);
	}
	else if (BufferSize)
		SaveBuffer(); // save for rest of data
}
void
G42RasImage::ReadImage()
{
	if (Info.Compressed)
	{
		while (BufferSize && ReadMode == ReadImageMode)
		{
			switch (DecompressionMode)
			{
				case StartDecompression:
				{
					byte v = GetByte();
					if (v != 0x80)
						PutByte(v);
					else
						DecompressionMode = GetRunCount;
					break;
				}
				case GetRunCount:
				{
					RunCount = GetByte();
					if (RunCount)
					{
						DecompressionMode = GetRunValue;
					}
					else
					{
						PutByte(0x80);
						DecompressionMode = StartDecompression;
					}
					break;
				}
				case GetRunValue:
				{
					byte v = GetByte();
					PutByteRun(v, RunCount + 1);
					DecompressionMode = StartDecompression;
				}
			}
		}
	}
	else
	{
		ProcessUncompressedData(); // base can handle uncompressed rows
	}
}
void
G42RasImage::OutputRow(void)
{
	if (Depth == 24 && !ColorOrderRGB)
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
	else if (Depth == 24 && ColorOrderRGB)
	{
		byte * sp = RowBuffer + (Width - 1) * 3;
		G42Byte24BitIterator dp(RowBuffer);
		dp.Forward(Width - 1);
		for (uint i = 0; i < Width; i++)
		{
			dp.Set(*(sp + 2), *(sp + 1), *sp);
			dp.Decrement();
			sp -= 3;
		}
	}
	else if (Depth == 32 && !ColorOrderRGB)
	{
		byte * sp = RowBuffer + (Width - 1) * 4 + 1;
		G42Byte24BitIterator dp(RowBuffer);
		dp.Forward(Width - 1);
		for (uint i = 0; i < Width; i++)
		{
			dp.Set(*sp, *(sp + 1), *(sp + 2));
			dp.Decrement();
			sp -= 4;
		}
	}
	else if (Depth == 32 && ColorOrderRGB)
	{
		byte * sp = RowBuffer + (Width - 1) * 4 + 1;
		G42Byte24BitIterator dp(RowBuffer);
		dp.Forward(Width - 1);
		for (uint i = 0; i < Width; i++)
		{
			dp.Set(*(sp + 2), *(sp + 1), *sp);
			dp.Decrement();
			sp -= 4;
		}
	}
	G42RasterBase::OutputRow();
	CurrentRow++;
	if (CurrentRow >= Height)
	{
		EatBuffer();
		ReadMode = ReadDoneMode;
		ForceFileEnd();
	}
}
bool G42IsRas(byte * buffer, size_t buffer_size)
{
	if (buffer[0] == 0x59 && (buffer_size < 2 ||
		(buffer[1] == 0xa6 && (buffer_size < 3 ||
		(buffer[2] == 0x6a && (buffer_size < 4 ||
		(buffer[3] == 0x95)))))))
		return true;
	if (buffer[0] == 0x95 && (buffer_size < 2 ||
		(buffer[1] == 0x6a && (buffer_size < 3 ||
		(buffer[2] == 0xa6 && (buffer_size < 4 ||
		(buffer[3] == 0x59)))))))
		return true;
	return false;
}

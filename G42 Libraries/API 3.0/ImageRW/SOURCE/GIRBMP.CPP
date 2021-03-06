// g42bmp.cpp - Bmp File
/*************************************************************
	File:          g42bmp.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   Derived class of g42ribase that handles bmp files
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
	Code	Date				Name	Description
	GES	17 Dec 1996		GES	Converted to 3.0 library format
***************************************************************/
#include "gitype.h"
#include "girbmp.h"
#include "gidata.h"
G42ImageReaderBmp::G42ImageReaderBmp(void)
:
	G42ImageReaderBase(),
	ReadMode(ReadHeaderMode),
	CurrentRow(0), NeedSkip(0)
{
	SetInfo(G42BmpTypeId, "bmp", "Bmp", "BM", 2);
}
G42ImageReaderBmp::G42ImageReaderBmp(
	G42HeapManager & heap, G42Notifier & notifier, G42File * file,
	G42BaseOptions * options)
:
	G42ImageReaderBase(heap, notifier, file, options),
	ReadMode(ReadHeaderMode),
	CurrentRow(0), NeedSkip(0)
{
	SetInfo(G42BmpTypeId, "bmp", "Bmp", "BM", 2);
}
G42ImageReaderBmp::~G42ImageReaderBmp()
{
}
// count rows, and mark done when last row is output
void
G42ImageReaderBmp::ConvertRow(void)
{
	if (Depth == 16)
	{
		byte * sp = RowBuffer + (Width - 1) * 2;
		G42Color24Iterator dp(OutputRowPtr);
		dp += Width - 1;
		for (uintf i = 0; i < Width; i++)
		{
			*dp = G42Color(
				(byte)(((*(sp) << 3) & 0xf8) | ((*(sp) >> 2) & 7)),
				(byte)(((*(sp + 1) << 6) & 0xc0) | ((*(sp + 1) << 1) & 6) |
						((*(sp) >> 2) & 0x38) | ((*(sp) >> 7) & 1)),
				(byte)(((*(sp + 1) << 1) & 0xf8) | ((*(sp + 1) >> 4) & 7)));
			dp--;
			sp -= 2;
		}
	}
	else if (Depth == 32)
	{
		byte * sp = RowBuffer;
		G42Color24Iterator dp(OutputRowPtr);
		for (uintf i = 0; i < Width; i++)
		{
			dp = G42Color(*sp, *(sp + 1), *(sp + 2));
			dp++;
			sp += 4;
		}
	}
// this being mswindow's format, no translation needs to be done for it
#if !defined(MSWIN)
	else if (Depth == 24)
	{
		byte * sp = RowBuffer + (Width - 1) * 3;
		G42Color24Iterator dp(OutputRowPtr);
		dp += (Width - 1);
		for (uintf i = 0; i < Width; i++)
		{
			dp = G42Color(*sp, *(sp + 1), *(sp + 2));
			dp--;
			sp -= 3;
		}
	}
#endif
	G42ImageReaderBase::ConvertRow(); // handles 4 bit to 8 bit, etc.
}
void
G42ImageReaderBmp::ProcessStreamData() // streaming mode
{
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
G42ImageReaderBmp::BeginProcessing(void) // non-streaming mode
{
	while (ReadMode != ReadDoneMode && !Finished)
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
				ReadMode = ReadDoneMode;
				break;
		}
	}
}
void
G42ImageReaderBmp::SendExtendedInfo(void) // non-streaming mode, read header only
{
	while (ReadMode != ReadDoneMode && !Finished)
	{
		switch (ReadMode)
		{
			case ReadHeaderMode:
				ReadHeader();
				break;
			case ReadPaletteMode:
				ReadPalette();
				break;
			default:
				ReadMode = ReadDoneMode;
				break;
		}
	}
}
void
G42ImageReaderBmp::ReadHeader(void)
{
	// we need to support NT and 4.0 cases
	if (!CheckBuffer(80, true)) // 80 is worst case behavior
	{
		SaveBuffer();
		return;
	}
	IntegerFormat = Intel;
	// check header ('BM')
	if (GetByte() != 0x42)
		throw G42IRWNotAnImageException();
	if (GetByte() != 0x4d)
		throw G42IRWNotAnImageException();
	// skip to start of good info
	SkipData(8);
	NeedSkip = (uintf)GetUInt32();
	NeedSkip &= 0xffff; // I messed this up in the past
	NeedSkip -= 14;
	// number of bytes of header, used to tell which version bmp
	uintf offset = (uintf)GetUInt32();
	if (offset == 40)
	{
		osType = Win3x;
	}
	else if (offset == 12)
	{
		osType = OS21x;
	}
	else if (offset == 64)
	{
		osType = OS22x;
	}
	else
	{
		osType = Undefined;
		ReadMode = ReadErrorMode;
		throw G42IRWInvalidImageException();
	}
	NeedSkip -= offset;
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
		uintf compression = (uintf)GetUInt32(); // we only do uncompressed for now
		if (compression != 0) /* if not BI_RGB */
		{
			ReadMode = ReadErrorMode;
			throw G42IRWInvalidImageException();
		}
		if (osType == OS22x) // skip to palette or image
		{
			SkipData(44);
			if (Depth <= 8)
				NumPalette = (1 << Depth);
			else
				NumPalette = 0;
		}
		else
		{
			SkipData(12);
			int num_colors = (int)GetUInt32();
			if (num_colors)
				NumPalette = num_colors;
			else if (Depth <= 8)
				NumPalette = (1 << Depth);
			else
				NumPalette = 0;
			SkipData(4);
		}
	}
	RowBytes = ((Width * Depth + 7) >> 3);
	RowBytes = ((RowBytes + 3) & ~3);
	TopLineFirst = false; // bmp's are bottom up files
	Color = true; // always
	CurrentPalette = 0;
	if (Depth > 8)
	{
		TrueColorDepth = (byte)Depth;
	}
	else
	{
		TrueColorDepth = 24;
	}
	SetupDescriptor();
/*
	if (osType == Win3x)
		Info.ImageType = BmpWin;
	else
		Info.ImageType = BmpOS2;
	Info.Compressed = false;
*/
	if (NumPalette)
	{
		ReadMode = ReadPaletteMode;
	}
	else
	{
		ReadMode = ReadImageMode; // no palette, so info is complete
		SendDescriptor();
	}
}
void
G42ImageReaderBmp::ReadPalette(void)
{
	CheckBuffer(NumPalette << 2, true); // if file mode, read in palette
	// read until palette is complete or buffer is empty
	while (BufferSize >= 4 && CurrentPalette < NumPalette)
	{
		ImagePalette[CurrentPalette].Blue = GetByte();
		ImagePalette[CurrentPalette].Green = GetByte();
		ImagePalette[CurrentPalette].Red = GetByte();
		NeedSkip -= 3;
		if (osType == Win3x || osType == OS22x)
		{
			SkipData();
			NeedSkip--;
		}
		CurrentPalette++;
	}
	if (CurrentPalette == NumPalette)
	{
		ReadMode = ReadImageMode;
		SendDescriptor();
	}
	else if (BufferSize)
	{
		SaveBuffer(); // save for rest of data
	}
}
void
G42ImageReaderBmp::ReadImage(void)
{
	if (NeedSkip > 0)
	{
		if (File)
		{
			SkipData(NeedSkip);
			NeedSkip = 0;
		}
		else
		{
			int skip = min((uintf)NeedSkip, BufferSize);
			SkipData(skip);
			NeedSkip -= skip;
			if (!BufferSize)
				return;
		}
	}
	ProcessUncompressedData(); // base can handle uncompressed rows
}

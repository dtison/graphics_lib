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
#include "girras.h"
G42ImageReaderRas::G42ImageReaderRas(void)
:
	G42ImageReaderBase(),
	ReadMode(ReadHeaderMode), ColorOrderRGB(false),
	CurrentPalette(0)
{
	// two different patterns, use test type
	SetInfo(G42RasTypeId, "ras", "Ras", "", 0);
}
G42ImageReaderRas::G42ImageReaderRas(
	G42HeapManager & heap, G42Notifier & notifier, G42File * file,
	G42BaseOptions * options)
:
	G42ImageReaderBase(heap, notifier, file, options),
	ReadMode(ReadHeaderMode), ColorOrderRGB(false),
	CurrentPalette(0)
{
	// two different patterns, use test type
	SetInfo(G42RasTypeId, "ras", "Ras", "", 0);
}
G42ImageReaderRas::~G42ImageReaderRas(void)
{
}
void
G42ImageReaderRas::ProcessStreamData(void)
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
G42ImageReaderRas::BeginProcessing(void)
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
G42ImageReaderRas::SendExtendedInfo(void)
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
G42ImageReaderRas::ReadHeader()
{
	if (!CheckBuffer(32, true))
	{
		SaveBuffer(); // save for rest of data
		return;
	}
	IntegerFormat = Motorola;
	uint32 magic = GetUInt32();
	if (magic != (uint32)0x59a66a95L && magic != (uint32)0x956aa659L)
	{
		throw G42IRWNotAnImageException();
	}
	if (magic == (uint32)0x956aa659L)
	{
		IntegerFormat = Intel; // perhaps for x86 of Solaris
	}
	Width = GetUInt32();
	Height = GetUInt32();
	Depth = GetUInt32();
	SkipData(4);
	uint32 type = GetUInt32();
	uint32 mtype = GetUInt32();
	uint32 palette_length = GetUInt32();
	if (!Width || !Height || type > 3 || mtype > 1)
	{
		ReadMode = ReadErrorMode;
		throw G42IRWInvalidImageException();
	}
	if (Depth != 1 && Depth != 8 && Depth != 24 && Depth != 32)
	{
		ReadMode = ReadErrorMode;
		throw G42IRWInvalidImageException();
	}
	RowBytes = ((Width * Depth + 7) >> 3);
	RowBytes = ((RowBytes + 1) & ~1L);
	if (type == 2)
		Compressed = true;
	else
		Compressed = false;
	if (type == 3)
		ColorOrderRGB = true;
	else
		ColorOrderRGB = false;
	if (mtype == 1 && Depth <= 8)
		NumPalette = palette_length / 3;
	else
		NumPalette = 0;
	if (NumPalette > 256)
		NumPalette = 256;
	if (Depth > 8 || NumPalette)
		Color = true;
	else
		Color = false;
	if (Compressed)
	{
		Decompressor = new G42RasRLEDecompressor(*this, RowBytes);
	}
	if (Depth > 8)
	{
		TrueColorDepth = (byte)Depth;
	}
	SetupDescriptor();
	if (NumPalette && NumPalette <= 256)
	{
		CurrentPalette = 0;
		ReadMode = ReadPaletteMode;
	}
	else
	{
		SendDescriptor();
		ReadMode = ReadImageMode;
	}
}
void
G42ImageReaderRas::ReadPalette(void)
{
	CheckBuffer(NumPalette * 3, true);
	// read until palette is complete or buffer is empty
	G42Color * pal = ImagePalette;
	while (BufferSize && CurrentPalette < NumPalette * 3)
	{
		if (CurrentPalette < NumPalette)
			pal[CurrentPalette].Red = GetByte();
		else if (CurrentPalette < (NumPalette << 1))
			pal[CurrentPalette - NumPalette].Green = GetByte();
		else
			pal[CurrentPalette - (NumPalette << 1)].Blue = GetByte();
		CurrentPalette++;
	}
	if (CurrentPalette == NumPalette * 3)
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
G42ImageReaderRas::ReadImage()
{
	ProcessImageData();
/*
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
*/
}
void
G42ImageReaderRas::ConvertRow(void)
{
	if (Depth == 24 && !ColorOrderRGB)
	{
		if (GetImageDepth() == 24)
		{
#if !defined(MSWIN)
			byte * sp = RowBuffer;
			G42Color24Iterator dp(OutputRowPtr);
			for (uint32 i = 0; i < Width; i++)
			{
				*dp = G42Color(*(sp + 2), *(sp + 1), *sp);
				dp++;
				sp += 3;
			}
#endif
		}
		else
		{
			byte * sp = RowBuffer + (Width - 1) * 3;
			G42Color32Iterator dp(OutputRowPtr);
			dp += Width - 1;
			for (uint32 i = 0; i < Width; i++)
			{
				*dp = G42Color(*(sp + 2), *(sp + 1), *sp);
				dp--;
				sp -= 3;
			}
		}
	}
	else if (Depth == 24 && ColorOrderRGB)
	{
		if (GetImageDepth() == 24)
		{
			byte * sp = RowBuffer;
			G42Color24Iterator dp(OutputRowPtr);
			for (uint32 i = 0; i < Width; i++)
			{
				*dp = G42Color(*sp, *(sp + 1), *(sp + 2));
				dp++;
				sp += 3;
			}
		}
		else
		{
			byte * sp = RowBuffer + (Width - 1) * 3;
			G42Color32Iterator dp(OutputRowPtr);
			dp += Width - 1;
			for (uint32 i = 0; i < Width; i++)
			{
				*dp = G42Color(*sp, *(sp + 1), *(sp + 2));
				dp--;
				sp -= 3;
			}
		}
	}
	else if (Depth == 32 && !ColorOrderRGB)
	{
		if (GetImageDepth() == 24)
		{
			byte * sp = RowBuffer;
			G42Color24Iterator dp(OutputRowPtr);
			for (uint32 i = 0; i < Width; i++)
			{
				*dp = G42Color(*(sp + 2), *(sp + 1), *sp);
				dp++;
				sp += 4;
			}
		}
		else
		{
			byte * sp = RowBuffer;
			G42Color32Iterator dp(OutputRowPtr);
			for (uint32 i = 0; i < Width; i++)
			{
				*dp = G42Color(*(sp + 2), *(sp + 1), *sp);
				dp++;
				sp += 4;
			}
		}
	}
	else if (Depth == 32 && ColorOrderRGB)
	{
		if (GetImageDepth() == 24)
		{
			byte * sp = RowBuffer;
			G42Color24Iterator dp(OutputRowPtr);
			for (uint32 i = 0; i < Width; i++)
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
			for (uint32 i = 0; i < Width; i++)
			{
				*dp = G42Color(*sp, *(sp + 1), *(sp + 2));
				dp++;
				sp += 4;
			}
		}
	}
	G42ImageReaderBase::ConvertRow();
}
bool
G42ImageReaderRas::TestType(void)
{
	File->Rewind();
	byte buf[4];
	File->Read(buf, 4);
	File->Rewind();
	return TestType(buf, 4);
}
bool
G42ImageReaderRas::TestType(const byte * buffer, uint32 buffer_size)
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
G42RasRLEDecompressor::G42RasRLEDecompressor(
	G42DecompressorOutput & output, uintf row_bytes)
:
	G42Decompressor(output, row_bytes),
	RunCount(0), DecompressionMode(StartDecompression)
{
}
G42RasRLEDecompressor::~G42RasRLEDecompressor(void)
{
}
void
G42RasRLEDecompressor::ProcessBuffer(void)
{
	while (InputBufferLeft())
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
				PutRun(v, RunCount + 1);
				DecompressionMode = StartDecompression;
			}
		}
	}
}

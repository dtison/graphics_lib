// girpbm.cpp - Pbm File
/*************************************************************
	File:          girpbm.cpp
	Copyright (c) 1996, Group 42, Inc.
	Description:   Derived class of girbase that handles Pbm files
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "girpbm.h"
G42ImageReaderPbm::G42ImageReaderPbm(void)
:
	G42ImageReaderBase(),
	ReadMode(ReadHeaderMode), CurrentValue(0)
{
	SetInfo(G42PbmTypeId, "pbm;pgm;pnm;ppm", "Pbm", "P", 1);
}
G42ImageReaderPbm::G42ImageReaderPbm(G42HeapManager & heap, G42Notifier & notifier,
	G42File * file, G42BaseOptions * options)
:
	G42ImageReaderBase(heap, notifier, file, options),
	ReadMode(ReadHeaderMode), CurrentValue(0)
{
	SetInfo(G42PbmTypeId, "pbm;pgm;pnm;ppm", "Pbm", "P", 1);
}
G42ImageReaderPbm::~G42ImageReaderPbm()
{
}
void
G42ImageReaderPbm::ProcessStreamData(void)
{
	while (BufferSize)
	{
		switch (ReadMode)
		{
			case ReadHeaderMode:
				ReadHeader();
				break;
			case ReadWidthMode:
				ReadWidth();
				break;
			case ReadHeightMode:
				ReadHeight();
				break;
			case ReadDepthMode:
				ReadDepth();
				break;
			case ReadInfoMode:
				ReadInfo();
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
G42ImageReaderPbm::BeginProcessing(void)
{
	while (ReadMode != ReadDoneMode && !Finished)
	{
		switch (ReadMode)
		{
			case ReadHeaderMode:
				ReadHeader();
				break;
			case ReadWidthMode:
				ReadWidth();
				break;
			case ReadHeightMode:
				ReadHeight();
				break;
			case ReadDepthMode:
				ReadDepth();
				break;
			case ReadInfoMode:
				ReadInfo();
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
G42ImageReaderPbm::SendExtendedInfo(void)
{
	while (ReadMode != ReadDoneMode && !Finished)
	{
		switch (ReadMode)
		{
			case ReadHeaderMode:
				ReadHeader();
				break;
			case ReadWidthMode:
				ReadWidth();
				break;
			case ReadHeightMode:
				ReadHeight();
				break;
			case ReadDepthMode:
				ReadDepth();
				break;
			case ReadInfoMode:
				ReadInfo();
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
G42ImageReaderPbm::ReadHeader(void)
{
	if (!CheckBuffer(2, true))
	{
		SaveBuffer();
		return;
	}
	IntegerFormat = Motorola;
	// check header ('P')
	if (GetByte() != 'P')
	{
		ReadMode = ReadErrorMode;
		throw G42IRWNotAnImageException();
	}
	intf version_num = GetByte();
	if (version_num < '1' || version_num > '6')
	{
		ReadMode = ReadErrorMode;
		throw G42IRWNotAnImageException();
	}
	Version = version_num - '0';
	if (Version == ColorAscii ||
			Version == ColorBinary)
		Color = true;
	else
		Color = false;
	CurrentValue = 0;
	ReadMode = ReadWidthMode;
}
void
G42ImageReaderPbm::ReadWidth(void)
{
	while (BufferSize || File)
	{
		char c = GetByte();
		if (c >= '0' && c <= '9')
		{
			CurrentValue *= 10;
			CurrentValue += c - '0';
		}
		else
		{
			if (CurrentValue)
			{
				Width = (intf)CurrentValue;
				ReadMode = ReadHeightMode;
				CurrentValue = 0;
				break;
			}
		}
	}
}
void
G42ImageReaderPbm::ReadHeight(void)
{
	while (BufferSize || File)
	{
		char c = GetByte();
		if (c >= '0' && c <= '9')
		{
			CurrentValue *= 10;
			CurrentValue += c - '0';
		}
		else
		{
			if (CurrentValue)
			{
				Height = (intf)CurrentValue;
				if (Version == MonochromeAscii ||
					Version == MonochromeBinary)
				{
					Depth = 1;
					MaxValue = 1;
					MaxDepth = 1;
					ReadMode = ReadInfoMode;
				}
				else
				{
					ReadMode = ReadDepthMode;
					CurrentValue = 0;
				}
				break;
			}
		}
	}
}
void
G42ImageReaderPbm::ReadDepth(void)
{
	while (BufferSize || File)
	{
		char c = GetByte();
		if (c >= '0' && c <= '9')
		{
			CurrentValue *= 10;
			CurrentValue += c - '0';
		}
		else
		{
			if (CurrentValue)
			{
				MaxValue = CurrentValue;
				if (MaxValue >= 2)
					Depth = 8;
				else
					Depth = 1;
				MaxDepth = ((1 << Depth) - 1);
				if (Version == ColorAscii || Version == ColorBinary)
				{
					Depth = 24;
					MaxDepth = 255;
				}
				ReadMode = ReadInfoMode;
				break;
			}
		}
	}
}
void
G42ImageReaderPbm::ReadInfo(void)
{
	NumPalette = 0;
	if (Depth > 8)
	{
		RowBytes = ((Width * Depth + 7) >> 3);
	}
	else
	{
		RowBytes = Width;
	}
	TopLineFirst = true; // pbm's are top down files
	Compressed = false; // don't mark ascii pbm's as compressed
	if (Version < MonochromeBinary)
	{
		Decompressor = new G42PbmDecompressor(*this, RowBytes, Depth, MaxValue,
			Width);
		MaxValue = MaxDepth; // taken care of in decompressor
	}
	WriteOptions = new G42ImagePbmOptions(
		((Version < MonochromeBinary) ? PbmAscii : PbmBinary), false);
	SetupDescriptor();
	SendDescriptor();
	ReadMode = ReadImageMode; // no palette, so info is complete
}
void
G42ImageReaderPbm::ReadImage(void)
{
	ProcessImageData();
}
void
G42ImageReaderPbm::ConvertRow(void)
{
	if (Depth > 8)
	{
		if (GetImageDepth() == 24)
		{
			byte * sp = RowBuffer;
			G42Color24Iterator dp(OutputRowPtr);
			for (uintf i = 0; i < Width; i++)
			{
				byte r = *sp++;
				byte g = *sp++;
				byte b = *sp++;
				if (MaxValue != MaxDepth)
				{
					r = (byte)(((uint32)r * (MaxDepth)) / (MaxValue));
					g = (byte)(((uint32)g * (MaxDepth)) / (MaxValue));
					b = (byte)(((uint32)b * (MaxDepth)) / (MaxValue));
				}
				*dp = G42Color(r, g, b);
				dp++;
			}
		}
		else
		{
			byte * sp = RowBuffer + (Width * 3) - 1;
			G42Color32Iterator dp(OutputRowPtr);
			dp += Width - 1;
			for (uintf i = 0; i < Width; i++)
			{
				byte r = *sp--;
				byte g = *sp--;
				byte b = *sp--;
				if (MaxValue != MaxDepth)
				{
					r = (byte)(((uint32)r * (MaxDepth)) / (MaxValue));
					g = (byte)(((uint32)g * (MaxDepth)) / (MaxValue));
					b = (byte)(((uint32)b * (MaxDepth)) / (MaxValue));
				}
				*dp-- = G42Color(r, g, b);
			}
		}
	}
	else if (Depth == 1)
	{
		byte * sp = RowBuffer;
		G42BitIterator dp(OutputRowPtr);
		for (uintf i = 0; i < Width; i++)
		{
			*dp++ = *sp++;
		}
	}
	else if (MaxValue != MaxDepth && Version >= MonochromeBinary)
	{
		byte * sp = RowBuffer;
		byte * dp = OutputRowPtr;
		for (uintf i = 0; i < Width; i++)
		{
			byte g = *sp++;
			g = (byte)(((uint32)g * (MaxDepth)) / (MaxValue));
			*dp++ = g;
		}
	}
}
G42PbmDecompressor::G42PbmDecompressor(
	G42DecompressorOutput & output, uintf row_bytes, uintf depth,
	uintf max_value, uint32 width)
:
	G42Decompressor(output, row_bytes),
	Depth(depth), MaxValue(max_value), MaxDepth((1 << depth) - 1),
	Width(width), CurrentColumn(0), CurrentShift(8 - depth), CurrentByte(0),
	CurrentValue(0)
{
}
G42PbmDecompressor::~G42PbmDecompressor(void)
{
}
void
G42PbmDecompressor::ProcessBuffer(void)
{
	if (Depth == 1)
	{
		while (InputBufferLeft())
		{
			char c = GetByte();
			if (c < '0' || c > '9')
				continue;
			CurrentValue = c - '0';
			PutByte((byte)CurrentValue);
			if (Done)
			{
				break;
			}
		}
	}
	else
	{
		while (InputBufferLeft())
		{
			char c = GetByte();
			if (c >= '0' && c <= '9')
			{
				CurrentValue *= 10;
				CurrentValue += c - '0';
			}
			else
			{
				if (MaxValue != MaxDepth)
				{
					CurrentValue = (CurrentValue * MaxDepth) / MaxValue;
				}
				PutByte((byte)CurrentValue);
				if (Done)
				{
					break;
				}
				CurrentValue = 0;
			}
		}
	}
}

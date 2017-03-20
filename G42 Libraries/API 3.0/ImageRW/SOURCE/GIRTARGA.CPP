// girtarga.cpp - Targa File
/*************************************************************
	File:          girtarga.cpp
	Copyright (c) 1996, Group 42, Inc.
	Description:   Derived class of g42ribase that handles Targa files
   Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
      Code   Date   Name and Description
			4-18-96	D. Ison  Fixed some read bugs that were lurking in this file
			1-17-97	GES		Converted to 3.0
***************************************************************/
#include "girtarga.h"
G42ImageReaderTarga::G42ImageReaderTarga(void)
:
	G42ImageReaderBase(),
	ReadMode(ReadHeaderMode), CurrentPalette(0), IdLength(0)
{
	// two different patterns, use test type
	SetInfo(G42TargaTypeId, "tga", "Targa", "", 0);
}
G42ImageReaderTarga::G42ImageReaderTarga(
	G42HeapManager & heap, G42Notifier & notifier, G42File * file,
	G42BaseOptions * options)
:
	G42ImageReaderBase(heap, notifier, file, options),
	ReadMode(ReadHeaderMode), CurrentPalette(0), IdLength(0)
{
	// two different patterns, use test type
	SetInfo(G42TargaTypeId, "tga", "Targa", "", 0);
}
G42ImageReaderTarga::~G42ImageReaderTarga(void)
{
}
void
G42ImageReaderTarga::ProcessStreamData(void)
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
			case ReadIdMode:
				ReadId();
				break;
			default:
				EatBuffer();
				break;
		}
   }
}
void
G42ImageReaderTarga::BeginProcessing(void)
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
			case ReadIdMode:
				ReadId();
				break;
			default:
				ReadMode = ReadDoneMode;
				break;
		}
	}
}
void
G42ImageReaderTarga::SendExtendedInfo(void)
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
			case ReadIdMode:
				ReadId();
				break;
			default:
				ReadMode = ReadDoneMode;
				break;
		}
   }
}
void
G42ImageReaderTarga::ReadHeader()
{
	if (!CheckBuffer(18, true))
	{
		SaveBuffer(); // save for rest of data
		return;
	}
	IntegerFormat = Intel;
	IdLength = GetByte();
	intf color_map_type = GetByte();
	ImageType = GetByte();
	ColorMapStart = GetUInt16();
	ColorMapLength = GetUInt16();
	ColorMapDepth = GetByte();
#if 0
	uint16 start_x = GetUInt16();
	uint16 start_y = GetUInt16();
	Width = GetUInt16() - start_x;
	Height = GetUInt16() - start_y;
#endif
	// I seem to remember that subtracting the start is sometimes bad
	SkipData(4);
	Width = GetUInt16();
	Height = GetUInt16();
	intf pixel_depth = GetByte();
	intf image_orientation = GetByte();
	if (image_orientation & 0x20)
		UpsideDown = false;
	else
		UpsideDown = true;
	if (image_orientation & 0x10)
		Backwards = true;
	else
		Backwards = false;
	CurrentRow = 0;	// D.I.
	if (UpsideDown)
	{
		TopLineFirst = false;
		#if 0
		CurrentRow = Height - 1;
		#endif
	}
	Depth = pixel_depth;
	if (Depth == 15)
		Depth = 16;
	if (Depth != 8 && Depth != 16 && Depth != 24 && Depth != 32)
	{
		ReadMode = ReadErrorMode;
		throw G42IRWInvalidImageException();
	}
	TrueColorDepth = (byte)Depth;
	RowBytes = Width * (Depth >> 3);
	if (color_map_type == 1)
	{
		Color = true;
		if (!ColorMapLength)
		{
			ReadMode = ReadErrorMode;
			throw G42IRWInvalidImageException();
		}
		NumPalette = ColorMapLength + ColorMapStart;
		if (ColorMapDepth == 15)
			ColorMapDepth = 16;
		if (ColorMapDepth != 8 && ColorMapDepth != 16 &&
			ColorMapDepth != 24 && ColorMapDepth != 32)
		{
			ReadMode = ReadErrorMode;
			throw G42IRWInvalidImageException();
		}
	}
	else if (color_map_type == 0)
	{
		Color = false;
		NumPalette = 0;
		ColorMapLength = 0;
	}
	else
	{
		ReadMode = ReadErrorMode;
		throw G42IRWInvalidImageException();
	}
	if (ImageType > 8)
		Compressed = true;
	else
		Compressed = false;
	if (Depth > 8)
	{
		Color = true;
		if (ColorMapLength && NumPalette)
		{
			IdLength += ColorMapLength * (ColorMapDepth >> 3);
			NumPalette = 0;
		}
	}
	else
	{
		if (NumPalette)
			Color = true;
		else
			Color = false;
	}
	if (NumPalette > 256)
	{
		ReadMode = ReadErrorMode;
		throw G42IRWInvalidImageException();
	}
	if (Compressed)
	{
		Decompressor = new G42TargaRLEDecompressor(*this, RowBytes, Depth);
	}
	SetupDescriptor();
	if (NumPalette)
	{
		for (intf i = 0; i < ColorMapStart; i++)
		{
			ImagePalette[i].Red = 0;
			ImagePalette[i].Green = 0;
			ImagePalette[i].Blue = 0;
		}
		CurrentPalette = ColorMapStart;
	}
	else
	{
		SendDescriptor();
	}
	ReadMode = ReadIdMode;
}
void
G42ImageReaderTarga::ReadId()
{
	if (File)
	{
		SkipData(IdLength);
		IdLength = 0;
	}
	if (IdLength && BufferSize)
	{
		intf skip = min((uintf)IdLength, BufferSize);
		SkipData(skip);
		IdLength -= skip;
	}
	if (!IdLength)
	{
		if (NumPalette)
			ReadMode = ReadPaletteMode;
		else
			ReadMode = ReadImageMode;
	}
}
void
G42ImageReaderTarga::ReadPalette(void)
{
	// read until palette is complete or buffer is empty
	G42Color * pal = ImagePalette;
	if (ColorMapDepth == 32)
	{
		CheckBuffer(4 * (NumPalette - CurrentPalette), true);
		while (BufferSize >= 4 && CurrentPalette < NumPalette)
		{
			pal[CurrentPalette].Blue = GetByte();
			pal[CurrentPalette].Green = GetByte();
			pal[CurrentPalette].Red = GetByte();
			SkipData(1); // skip alpha info for now
			CurrentPalette++;
		}
	}
	else if (ColorMapDepth == 24)
	{
		CheckBuffer(3 * (NumPalette - CurrentPalette), true);
		while (BufferSize >= 3 && CurrentPalette < NumPalette)
		{
			pal[CurrentPalette].Blue = GetByte();
			pal[CurrentPalette].Green = GetByte();
			pal[CurrentPalette].Red = GetByte();
			CurrentPalette++;
		}
	}
	else // 16 or 15 (same thing, really)
	{
		CheckBuffer(2 * (NumPalette - CurrentPalette), true);
		while (BufferSize >= 2 && CurrentPalette < NumPalette)
		{
			uint16 v = (uint16)GetByte();
			v |= (uint16)((uint16)GetByte() << 8);
			pal[CurrentPalette].Red =
				(byte)(((v >> 8) & 0xf8) | ((v >> 13) & 0x7));
			pal[CurrentPalette].Green =
				(byte)(((v >> 3) & 0xf8) | ((v >> 8) & 0x7));
			pal[CurrentPalette].Blue =
				(byte)(((v << 2) & 0xf8) | ((v >> 5) & 0x7));
			CurrentPalette++;
		}
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
G42ImageReaderTarga::ReadImage()
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
					RunCount = (intf)(v & 0x7f) + 1;
					if (v & 0x80)
					{
						DecompressionMode = GetRunValue;
						Length = (Depth >> 3);
						Offset = 0;
					}
					else
					{
						DecompressionMode = GetSequence;
						Length = RunCount * (Depth >> 3);
					}
					break;
				}
				case GetRunValue:
				{
					if (Offset < Length)
					{
						Ref[Offset] = GetByte();
						Offset++;
					}
					if (Offset == Length)
					{
						for (intf i = 0; i < RunCount; i++)
						{
							PutSequence(Ref, Length);
							if (ReadMode != ReadImageMode)
								break;
						}
						if (ReadMode == ReadImageMode)
							DecompressionMode = StartDecompression;
					}
					break;
				}
				case GetSequence:
				{
					// shorter loop for speed - I really need to do something better
					while (Length && BufferSize && ReadMode == ReadImageMode)
					{
						PutByte(GetByte());
						Length--;
					}
					if (!Length && ReadMode == ReadImageMode)
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
G42ImageReaderTarga::ConvertRow(void)
{
	if (Depth == 24)
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
	else if (Depth == 32)
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
	else if (Depth == 16)
	{
		if (GetImageDepth() == 24)
		{
			byte * sp = RowBuffer + (Width - 1) * 2;
			G42Color24Iterator dp(OutputRowPtr);
			dp += Width - 1;
			for (uint32 i = 0; i < Width; i++)
			{
				intf v = (intf)(*sp) | ((intf)(*(sp + 1)) << 8);
				*dp = G42Color(
					(byte)(((v >> 7) & 0xf8) | ((v >> 12) & 0x7)), // red
					(byte)(((v >> 2) & 0xf8) | ((v >> 7) & 0x7)), // green
					(byte)(((v << 3) & 0xf8) | ((v >> 2) & 0x7))); // blue
				dp--;
				sp -= 2;
			}
		}
		else
		{
			byte * sp = RowBuffer + (Width - 1) * 2;
			G42Color32Iterator dp(OutputRowPtr);
			dp += Width - 1;
			for (uint32 i = 0; i < Width; i++)
			{
				intf v = (intf)(*sp) | ((intf)(*(sp + 1)) << 8);
				*dp = G42Color(
					(byte)(((v >> 7) & 0xf8) | ((v >> 12) & 0x7)), // red
					(byte)(((v >> 2) & 0xf8) | ((v >> 7) & 0x7)), // green
					(byte)(((v << 3) & 0xf8) | ((v >> 2) & 0x7))); // blue
				dp--;
				sp -= 2;
			}
		}
	}
	if (Backwards)
	{
		if (GetImageDepth() == 24)
		{
			G42Color24Iterator sp(RowBuffer);
			G42Color24Iterator dp(RowBuffer);
			dp += Width - 1;
			for (uint32 i = 0; i < (Width >> 1); i++)
			{
				G42Color c = (G42Color24)(*sp);
				*sp = *dp;
				*dp = c;
				sp++;
				dp--;
			}
		}
		else
		{
			G42Color32Iterator sp(RowBuffer);
			G42Color32Iterator dp(RowBuffer);
			dp += Width - 1;
			for (uint32 i = 0; i < (Width >> 1); i++)
			{
				G42Color c = (G42Color32)(*sp);
				*sp = *dp;
				*dp = c;
				sp++;
				dp--;
			}
		}
	}
	G42ImageReaderBase::ConvertRow();
}
bool
G42ImageReaderTarga::TestType(void)
{
	File->Rewind();
	byte buf[4];
	File->Read(buf, 4);
	File->Rewind();
	return TestType(buf, 4);
}
bool
G42ImageReaderTarga::TestType(const byte * buffer, uint32 buffer_size)
{
	// targa's don't really have a header, and the first byte is often
	// zero, but not required to be.  However, the only other way to
	// do this would be to report true or false on everything, which
	// is a bad idea (on the true at least).
	if (buffer_size < 2) // big problem here
	{
		if (*buffer == 0) // not always true, but what else can we do?
			return true;
		else
			return false;
	}
	// if it doesn't match this, it could still be an odd targa, but we won't
	// read it anyway, so we might as well return false.
	if ((buffer[1] == 0 || buffer[1] == 1) && // at least check image and map types
		(buffer_size < 3 || ((buffer[2] & 0xf7) < 4 && (buffer[2] & 0xf7) > 0)))
		return true;
	return false;
}
G42TargaRLEDecompressor::G42TargaRLEDecompressor(
	G42DecompressorOutput & output, uintf row_bytes, uintf depth)
:
	G42Decompressor(output, row_bytes),
	BytesPerPixel(depth >> 3), DecompressionMode(StartDecompression)
{
}
G42TargaRLEDecompressor::~G42TargaRLEDecompressor(void)
{
}
void
G42TargaRLEDecompressor::ProcessBuffer(void)
{
	while (InputBufferLeft() && !Done)
	{
		switch (DecompressionMode)
		{
			case StartDecompression:
			{
				byte v = GetByte();
				RunCount = (uintf)(v & 0x7f) + 1;
				if (v & 0x80)
				{
					DecompressionMode = GetRunValue;
					Length = BytesPerPixel;
					Offset = 0;
				}
				else
				{
					DecompressionMode = GetSequence;
					Length = RunCount * BytesPerPixel;
				}
				break;
			}
			case GetRunValue:
			{
				while (Offset < Length && !Done && InputBufferLeft())
				{
					Ref[Offset++] = GetByte();
				}
				if (Offset == Length)
				{
					for (uintf i = 0; i < RunCount && !Done; i++)
					{
						PutSequence(Ref, Length);
					}
					DecompressionMode = StartDecompression;
				}
				break;
			}
			case GetSequence:
			{
				uintf put_length = min(Length, (uintf)(InputEnd - InputPtr));
				PutSequence(InputPtr, put_length);
				InputPtr += put_length;
				Length -= put_length;
				if (!Length)
				{
					DecompressionMode = StartDecompression;
				}
			}
		}
	}
}

// g42pcx.cpp - Pcx File
/*************************************************************
	File:          g42pcx.cpp
	Copyright (c) 1996, Group 42, Inc.
	Description:   Derived class of g42ribase that handles Pbm files
   Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
      Code   Date   Name and Description
			4-18-96	D. Ison  Fixed some read bugs that were lurking in this file
***************************************************************/
#include "g42itype.h"
#include "g42targa.h"
#include "g42iter.h"
G42TargaImage::G42TargaImage(G42RasterImage & parent) :
	G42RasterBase(parent), ReadMode(ReadHeaderMode),
	CurrentPalette(0), IdLength(0), DecompressionMode(StartDecompression)
{
	Valid = true;
}
G42TargaImage::~G42TargaImage()
{
}
void
G42TargaImage::ProcessData(byte * buffer, size_t buffer_size)
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
G42TargaImage::ReadHeader()
{
	if (BufferSize < 18)
	{
		SaveBuffer(); // save for rest of data
		return;
	}
	IntegerFormat = Intel;
	IdLength = GetByte();
	int color_map_type = GetByte();
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
	int pixel_depth = GetByte();
	int image_orientation = GetByte();
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
	Info.Width = Width;
	Info.Height = Height;
	Info.ImageType = Tga;
	Depth = pixel_depth;
	if (Depth == 15)
		Depth = 16;
	if (Depth != 8 && Depth != 16 && Depth != 24 && Depth != 32)
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		return;
	}
	RowBytes = Width * (Depth >> 3);
	if (Depth == 16)
		Info.TargaDepth = (byte)Depth;
	else
		Info.TargaDepth = 24; // I don't write 32 anyway
	if (color_map_type == 1)
	{
		Color = true;
		if (!ColorMapLength)
		{
			Valid = false;
			ReadMode = ReadErrorMode;
			return;
		}
		NumPalette = ColorMapLength + ColorMapStart;
		if (ColorMapDepth == 15)
			ColorMapDepth = 16;
		if (ColorMapDepth != 8 && ColorMapDepth != 16 &&
			ColorMapDepth != 24 && ColorMapDepth != 32)
		{
			Valid = false;
			ReadMode = ReadErrorMode;
			return;
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
		Valid = false;
		ReadMode = ReadErrorMode;
		return;
	}
	if (ImageType > 8)
		Info.Compressed = General;
	else
		Info.Compressed = None;
	if (Depth > 8)
	{
		Info.PixelDepth = 8 * TrueColorPixelSize;
		Info.ColorType = G42ImageInfo::ColorTypeTrueColor;
		if (ColorMapLength && NumPalette)
		{
			IdLength += ColorMapLength * (ColorMapDepth >> 3);
			NumPalette = 0;
		}
	}
	else
	{
		Info.PixelDepth = Depth;
		if (NumPalette)
			Info.ColorType = G42ImageInfo::ColorTypePalette;
		else
			Info.ColorType = G42ImageInfo::ColorTypeGrayscale;
	}
	if (NumPalette > 256)
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		return;
	}
	if (NumPalette)
	{
		G42Color * pal = new G42Color [NumPalette];
		if (!pal)
		{
			Valid = false;
			ReadMode = ReadErrorMode;
			return;
		}
		for (int i = 0; i < ColorMapStart; i++)
		{
			pal[i].red = 0;
			pal[i].green = 0;
			pal[i].blue = 0;
		}
		CurrentPalette = ColorMapStart;
		Info.ImagePalette.Set(pal);
	}
	else
	{
		Parent.HasInfo(Info);
	}
//	ReadMode = ReadImageMode;	// (m.2) D.I.
	ReadMode = ReadIdMode;
}
void
G42TargaImage::ReadId()
{
	if (IdLength && BufferSize)
	{
		int skip = min((uint)IdLength, BufferSize);
		SkipData(skip);
		IdLength -= skip;
	}
	if (!IdLength)
	{
		if (Info.NumPalette)
			ReadMode = ReadPaletteMode;
		else
			ReadMode = ReadImageMode;
	}
}
void
G42TargaImage::ReadPalette(void)
{
	// read until palette is complete or buffer is empty
	G42Color * pal = Info.ImagePalette;
	if (ColorMapDepth == 32)
	{
		while (BufferSize >= 4 && CurrentPalette < NumPalette)
		{
			pal[CurrentPalette].blue = GetByte();
			pal[CurrentPalette].green = GetByte();
			pal[CurrentPalette].red = GetByte();
			SkipData(1); // skip alpha info for now
			CurrentPalette++;
		}
	}
	else if (ColorMapDepth == 24)
	{
		while (BufferSize >= 3 && CurrentPalette < NumPalette)
		{
			pal[CurrentPalette].blue = GetByte();
			pal[CurrentPalette].green = GetByte();
			pal[CurrentPalette].red = GetByte();
			CurrentPalette++;
		}
	}
	else // 16 or 15 (same thing, really)
	{
		while (BufferSize >= 2 && CurrentPalette < NumPalette)
		{
			uint16 v = (uint16)GetByte();
			v |= (uint16)((uint16)GetByte() << 8);
			pal[CurrentPalette].red =
				(byte)(((v >> 8) & 0xf8) | ((v >> 13) & 0x7));
			pal[CurrentPalette].green =
				(byte)(((v >> 3) & 0xf8) | ((v >> 8) & 0x7));
			pal[CurrentPalette].blue =
				(byte)(((v << 2) & 0xf8) | ((v >> 5) & 0x7));
			CurrentPalette++;
		}
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
G42TargaImage::ReadImage()
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
					RunCount = (int)(v & 0x7f) + 1;
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
						for (int i = 0; i < RunCount; i++)
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
}
void
G42TargaImage::OutputRow(void)
{
	if (Depth == 24)
	{
		byte * sp = RowBuffer + (Width - 1) * 3;
		G42Byte24BitIterator dp(RowBuffer);
		dp.Forward(Width - 1);
		for (uint32 i = 0; i < Width; i++)
		{
			dp.Set(*sp, *(sp + 1), *(sp + 2));
			dp.Decrement();
			sp -= 3;
		}
	}
	else if (Depth == 32)	// Has to go forward
	{
		byte * sp = RowBuffer;
		G42Byte24BitIterator dp(RowBuffer);
		for (uint32 i = 0; i < Width; i++)
		{
			dp.Set(*sp, *(sp + 1), *(sp + 2));
			dp.Increment();
			sp += 4;
		}	
		
	}
	else if (Depth == 16)
	{
		byte * sp = RowBuffer + (Width - 1) * 2;
		G42Byte24BitIterator dp(RowBuffer);
		dp.Forward(Width - 1);
		for (uint32 i = 0; i < Width; i++)
		{
			int v = (int)(*sp) | ((int)(*(sp + 1)) << 8);
			dp.Set(
				(byte)(((v << 3) & 0xf8) | ((v >> 2) & 0x7)), // blue
				(byte)(((v >> 2) & 0xf8) | ((v >> 7) & 0x7)), // green
				(byte)(((v >> 7) & 0xf8) | ((v >> 12) & 0x7))); // red
			dp.Decrement ();
			sp -= 2;
		}
	}
	if (Backwards)
	{
		G42Byte24BitIterator sp(RowBuffer);
		G42Byte24BitIterator dp(RowBuffer);
		dp.Forward(Width - 1);
		for (uint32 i = 0; i < (Width >> 1); i++)
		{
			byte r = sp.GetRed();
			byte g = sp.GetGreen();
			byte b = sp.GetBlue();
			sp.Set(dp.GetBlue(), dp.GetGreen(), dp.GetRed());
			dp.Set(b, g, r);
			sp.Increment();
			dp.Decrement();
		}
	}
	G42RasterBase::OutputRow();
		
	CurrentRow++;
	if (CurrentRow >= Height)
//	if (CurrentRow >= (Height - 2))
	{
		EatBuffer();
		ReadMode = ReadDoneMode;
		ForceFileEnd();
	}
}
bool G42IsTarga(byte * buffer, size_t buffer_size)
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

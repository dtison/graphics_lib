// g42gif.cpp - Gif File
/*************************************************************
	File:          g42gif.cpp
   Copyright (c) 1996, Group 42, Inc.
	Description:   Derived class of g42ribase that handles GIF files
   Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
   Modification History:
      Code   Date   Name and Description
***************************************************************/
#include "g42itype.h"
#include "g42gif.h"
G42GifImage::G42GifImage(G42RasterImage & parent) :
	G42RasterBase(parent), ReadMode(ReadHeaderMode),
	CurrentRow(0), Lzw(0), Pass(0), MaskBuffer(0)
{
}
G42GifImage::~G42GifImage()
{
	if (MaskBuffer)
		delete[] MaskBuffer;
	if (Lzw)
   	delete Lzw;
}
// count rows, and mark done when last row is output
void
G42GifImage::OutputRow(void)
{
	if (Interlace)
	{
		switch (Pass)
		{
			case 0:
			{
				int i;
				for (i = 0; i < 8 && CurrentRow < Height; i++)
				{
//					Parent.HasRow(RowBuffer, MaskBuffer);
					Parent.HasRow(RowBuffer, CurrentRow, 0, Width, MaskBuffer);
					CurrentRow++;
				}
				break;
			}
			case 1:
			{
				int i;
				for (i = 0; i < 4 && CurrentRow < Height; i++)
				{
//					Parent.HasRow(RowBuffer, MaskBuffer);
					Parent.HasRow(RowBuffer, CurrentRow, 0, Width, MaskBuffer);
					CurrentRow++;
				}
				for (i = 0; i < 4 && CurrentRow < Height; i++)
				{
//					Parent.HasRow(0);
					CurrentRow++;
				}
				break;
			}
			case 2:
			{
				int i;
				for (i = 0; i < 2 && CurrentRow < Height; i++)
				{
//					Parent.HasRow(RowBuffer, MaskBuffer);
					Parent.HasRow(RowBuffer, CurrentRow, 0, Width, MaskBuffer);
					CurrentRow++;
				}
				for (i = 0; i < 2 && CurrentRow < Height; i++)
				{
//					Parent.HasRow(0);
					CurrentRow++;
				}
				break;
			}
			case 3:
			{
//				Parent.HasRow(RowBuffer, MaskBuffer);
					Parent.HasRow(RowBuffer, CurrentRow, 0, Width, MaskBuffer);
				CurrentRow++;
				if (CurrentRow < Height)
				{
//					Parent.HasRow(0);
					CurrentRow++;
				}
				break;
			}
		}
	}
	else
	{
//		Parent.HasRow(RowBuffer, MaskBuffer);
		Parent.HasRow(RowBuffer, CurrentRow, 0, Width, MaskBuffer);
		CurrentRow++;
	}
	if (CurrentRow >= Height)
	{
		Pass++;
		if (Pass == NumPasses)
		{
			EatBuffer();
			ReadMode = ReadDoneMode;
			ForceFileEnd();
		}
		else
		{
			CurrentRow = 0;
			switch (Pass)
			{
				case 1:
				{
					int i;
					for (i = 0; i < 4 && CurrentRow < Height; i++)
					{
//						Parent.HasRow(0);
						CurrentRow++;
					}
					break;
				}
				case 2:
				{
					int i;
					for (i = 0; i < 2 && CurrentRow < Height; i++)
					{
//						Parent.HasRow(0);
						CurrentRow++;
					}
					break;
				}
				case 3:
				{
//					Parent.HasRow(0);
					CurrentRow++;
					break;
				}
			}
		}
	}
	RowPtr = RowBuffer;
	RowBytesLeft = RowBytes;
}
void
G42GifImage::ProcessData(byte * buffer, size_t buffer_size)
{
	RestoreBuffer(buffer, buffer_size);
	while (BufferSize)
	{
		switch (ReadMode)
		{
			case ReadHeaderMode:
				ReadHeader();
				break;
			case ReadGlobalPaletteMode:
				ReadGlobalPalette();
				break;
			case ReadChunkMode:
				ReadChunk();
				break;
			case ReadExtensionChunkMode:
				ReadExtensionChunk();
				break;
			case ReadUnknownSubBlockMode:
				ReadUnknownSubBlock();
				break;
			case ReadLocalImageMode:
				ReadLocalImage();
				break;
			case ReadLocalPaletteMode:
				ReadLocalPalette();
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
G42GifImage::ReadHeader(void)
{
	if (BufferSize < 13)
	{
		SaveBuffer();
		return;
	}
	IntegerFormat = Intel;
	// check header ('GIF')
	if (GetByte() != 'G')
		return;
	if (GetByte() != 'I')
		return;
	if (GetByte() != 'F')
		return;
	// skip version info
	SkipData(3);
	Width = GetUInt16();
	Height = GetUInt16();
	int packed = GetByte();
	Background = GetByte();
	(void)GetByte();
	Depth = (packed & 7) + 1;
	NumPalette = (1 << Depth);
	if (Depth > 1 && Depth <= 4)
		Depth = 4;
	else if (Depth > 4)
		Depth = 8;
	RowBytes = ((Width * Depth + 7) >> 3);
	Color = true; // GIF's are always paletted files
	TopLineFirst = true; // GIF's are top down files
	Info.Width = Width;
	Info.Height = Height;
	Info.PixelDepth = Depth;
	Info.HasBackground = true;
	Info.ImageType = Gif;
	Info.ColorType = G42ImageInfo::ColorTypePalette;
	Info.Compressed = true;
	if (NumPalette)
	{
		ImagePalette = new G42Color[NumPalette]; // need to read in palette
		if (!ImagePalette)
		{
			Valid = false;
			ReadMode = ReadErrorMode;
			return;
		}
	}
	Valid = true;
	CurrentPalette = 0;
	if (packed & 0x80)
		ReadMode = ReadGlobalPaletteMode;
	else
		ReadMode = ReadChunkMode;
}
void
G42GifImage::ReadChunk(void)
{
	int code = GetByte();
	if (code == 0x2c)
		ReadMode = ReadLocalImageMode;
	else if (code == 0x21)
		ReadMode = ReadExtensionChunkMode;
	else
	{
   	Valid = false;
		ReadMode = ReadErrorMode;
	}
}
void
G42GifImage::ReadExtensionChunk(void)
{
	if (BufferSize < 7) // transparent block as minimum size
	{
		SaveBuffer();
		return;
	}
	int label = GetByte();
	if (label == 0xf9) // could be transparent
	{
		SkipData();
		int flags = GetByte();
		if (flags & 1)
		{
			SkipData(2);
			TransparentPixel = GetByte();
			SkipData();
			MaskBuffer = new byte [((((Width + 7) >> 3) + 3) & ~3)];
			Transparent = true;
			Info.IsTransparent = true;
		}
		else
		{
			SkipData(4);
		}
		ReadMode = ReadChunkMode;
	}
	else
	{
		ReadMode = ReadUnknownSubBlockMode;
		SubBlockSize = 0;
	}
}
// skip the sub blocks of an unknown chunk
void
G42GifImage::ReadUnknownSubBlock(void)
{
	while (BufferSize)
	{
		if (!SubBlockSize)
			SubBlockSize = GetByte();
		if (!SubBlockSize)
		{
			ReadMode = ReadChunkMode;
			break;
		}
		else
		{
			if (BufferSize >= SubBlockSize)
			{
				SkipData(SubBlockSize);
				SubBlockSize = 0;
			}
			else
			{
				SubBlockSize -= BufferSize;
				SkipData(BufferSize);
			}
		}
	}
}
void
G42GifImage::ReadLocalImage(void)
{
	if (BufferSize < 9)
	{
		SaveBuffer();
		return;
	}
	SkipData(4);
	Width = GetUInt16();
	Height = GetUInt16();
	RowBytes = ((Width * Depth + 7) >> 3);
	int packed = GetByte();
	if (packed & 0x40)
	{
		Interlace = true;
		NumPasses = 4;
	}
	else
	{
		Interlace = false;
		NumPasses = 1;
	}
	Info.Progressive = Interlace;
	Info.Width = Width;
   Info.Height = Height;
	if (packed & 0x80)
	{
		int num_local = (packed & 7) + 1;
		if (num_local != Depth)
		{
			Depth = num_local;
			NumPalette = (1 << Depth);
			RowBytes = ((Width * Depth + 7) >> 3);
         delete [] ImagePalette;
			ImagePalette = new G42Color[NumPalette]; // need to read in palette
			if (!ImagePalette)
			{
				Valid = false;
				ReadMode = ReadErrorMode;
				return;
			}
		}
		ReadMode = ReadLocalPaletteMode;
      CurrentPalette = 0;
	}
	else
	{
		ReadMode = ReadImageMode; // no palette, so info is complete
		G42Color * pal = new G42Color[NumPalette]; // need to read in palette
		if (!pal)
		{
			Valid = false;
			ReadMode = ReadErrorMode;
			return;
		}
		for (int i = 0; i < NumPalette; i++)
      	pal[i] = ImagePalette[i];
		Info.ImagePalette.Set(pal);
      Info.NumPalette = NumPalette;
		if (Info.IsTransparent)
			Info.TransparentColor = ((G42Color *)(Info.ImagePalette))[TransparentPixel];
		Info.BackgroundColor = ((G42Color *)(Info.ImagePalette))[Background];
//		Parent.HasInfo();         // give user the file dimensions
		Parent.HasInfo(Info);
	}
}
void
G42GifImage::ReadLocalPalette(void)
{
	// read until palette is complete or buffer is empty
	while (BufferSize >= 3 && CurrentPalette < NumPalette)
	{
		ImagePalette[CurrentPalette].red = GetByte();
		ImagePalette[CurrentPalette].green = GetByte();
		ImagePalette[CurrentPalette].blue = GetByte();
		CurrentPalette++;
	}
	if (CurrentPalette == NumPalette)
	{
		ReadMode = ReadImageMode;
		G42Color * pal = new G42Color[NumPalette]; // need to read in palette
		if (!pal)
		{
			Valid = false;
			ReadMode = ReadErrorMode;
			return;
		}
		for (int i = 0; i < NumPalette; i++)
      	pal[i] = ImagePalette[i];
		Info.ImagePalette.Set(pal);
      Info.NumPalette = NumPalette;
		if (Info.IsTransparent)
			Info.TransparentColor = ((G42Color *)(Info.ImagePalette))[TransparentPixel];
		Info.BackgroundColor = ((G42Color *)(Info.ImagePalette))[Background];
		Parent.HasInfo(Info);
//		Parent.HasInfo();
	}
	else if (BufferSize)
		SaveBuffer(); // save for rest of data
}
void
G42GifImage::ReadGlobalPalette(void)
{
	// read until palette is complete or buffer is empty
	while (BufferSize >= 3 && CurrentPalette < NumPalette)
	{
		ImagePalette[CurrentPalette].red = GetByte();
		ImagePalette[CurrentPalette].green = GetByte();
		ImagePalette[CurrentPalette].blue = GetByte();
		CurrentPalette++;
	}
	if (CurrentPalette == NumPalette)
	{
		ReadMode = ReadChunkMode;
	}
	else if (BufferSize)
		SaveBuffer(); // save for rest of data
}
void
G42GifImage::ReadImage(void)
{
	if (!BufferSize)
		return;
	if (!Lzw)
	{
		int code_size = GetByte();
		Lzw = new G42GifLzw(this, Width, code_size);
		BlockSize = 0;
		if (!BufferSize)
			return;
	}
	if (!BlockSize)
	{
		BlockSize = GetByte();
		if (!BlockSize)
		{
			ReadMode = ReadDoneMode;
			ForceFileEnd();
			return;
		}
	}
	if (BlockSize && BufferSize)
	{
		int size = min(BlockSize, (int)BufferSize);
		FillBuffer(InputBuffer, size);
		Lzw->ProcessBuffer(InputBuffer, size);
		BlockSize -= size;
	}
}
void
G42GifImage::ProcessRow(byte * row)
{
	if (MaskBuffer)
	{
		byte * sp = row;
		byte * dp = MaskBuffer;
		int m = 0x80;
		for (int i = 0; i < Width; i++)
		{
			if (*sp == TransparentPixel)
			{
//				*sp = Background;
				*dp |= m;
			}
			else
				*dp &= ~m;
			sp++;
			if (m == 1)
			{
				m = 0x80;
				dp++;
			}
			else
				m >>= 1;
		}
	}
	if (Depth == 1)
	{
		byte * sp = row;
		byte * dp = row;
		int i = 0;
		int m = 0x80;
		for (i = 0; i < Width; i++)
		{
			if (*sp)
				*dp |= m;
			else
				*dp &= ~m;
			sp++;
			if (m == 1)
			{
				m = 0x80;
				dp++;
			}
			else
				m >>= 1;
		}
	}
	else if (Depth <= 4)
	{
		byte * sp = row + 1;
		byte * dp = row;
		(*row) <<= 4;
		int i;
		int s = 0;
		for (i = 1; i < Width; i++)
		{
			*dp |= (*sp << s);
			sp++;
			if (s == 0)
			{
				s = 4;
				dp++;
            *dp = 0;
			}
			else
				s = 0;
		}
	}
	PutSequence(row, RowBytes);
}
// safe for now.
bool G42IsGif(byte * buffer, size_t buffer_size)
{
	if ((buffer[0] == 'G') &&
		(buffer_size < 1 || buffer[1] == 'I') &&
		(buffer_size < 2 || buffer[2] == 'F'))
		return true;
	return false;
}

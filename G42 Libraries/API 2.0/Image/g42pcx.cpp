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
#include "g42pcx.h"
#include "g42iter.h"
G42PcxImage::G42PcxImage(G42RasterImage & parent) :
	G42RasterBase(parent), ReadMode(ReadHeaderMode), CurrentRow(0),
	PcxRowBuffer(0), DecompressionMode(StartDecompression)
{
	Valid = true;
}
G42PcxImage::~G42PcxImage()
{
	if (PcxRowBuffer)
	{
		delete[] PcxRowBuffer;
		PcxRowBuffer = 0;
	}
}
void
G42PcxImage::OutputRow(void)
{
	/*  This code pulled from the Targa reader & modified to suit PCX  */
	if (Depth == 24)
	{
		memcpy(PcxRowBuffer, RowBuffer, RowBytes);
		byte * sp = PcxRowBuffer;
		G42Byte24BitIterator dp(RowBuffer);
		for (uint32 i = 0; i < Width; i++)
		{
			dp.Set(*(sp + (PlaneBytes << 1)), *(sp + PlaneBytes), *sp);
			dp.Increment();
			sp++;
		}
	}
	else if (Depth == 4)
	{
		memcpy(PcxRowBuffer, RowBuffer, RowBytes);
		byte * sp1 = PcxRowBuffer;
		byte * sp2 = PcxRowBuffer + PlaneBytes;
		byte * sp3 = PcxRowBuffer + (PlaneBytes << 1);
		byte * sp4 = PcxRowBuffer + (PlaneBytes << 1) + PlaneBytes;
		int sm = 0x80;
		byte * dp = RowBuffer;
		*dp = 0;
		int ds = 4;
		for (uint32 i = 0; i < Width; i++)
		{
			int v = 0;
			if (*sp1 & sm)
				v |= 1;
			if (*sp2 & sm)
				v |= 2;
			if (*sp3 & sm)
				v |= 4;
			if (*sp4 & sm)
				v |= 8;
			*dp |= (byte)(v << ds);
			if (ds == 4)
			{
				ds = 0;
			}
			else
			{
				ds = 4;
				dp++;
				*dp = 0; // safe because it always allocates extra
			}
			if (sm == 1)
			{
				sp1++;
				sp2++;
				sp3++;
				sp4++;
				sm = 0x80;
			}
			else
			{
				sm >>= 1;
			}
		}
	}
	Parent.HasRow(RowBuffer, CurrentRow, 0, Width);
	RowPtr = RowBuffer;
	RowBytesLeft = RowBytes;
	CurrentRow++;
	if (CurrentRow >= Height)
	{
		if (Depth == 8)
		{
			ReadMode = CheckTrailerMode;
		}
		else
		{
			ReadMode = ReadDoneMode;
			ForceFileEnd();
		}
	}
#if 0
	#if 1	// D.I.  Force 24 bit to "finish out" even though they're not finishing yet
	if (Depth < 24)
	{
		if (CurrentRow >= Height)
		{
			ReadMode = CheckTrailerMode;
		}
	}
	else
		if (CurrentRow >= (Height - 1))
		{
			G42RasterBase::OutputRow();
			CurrentRow++;
				ReadMode = CheckTrailerMode;
		}
	#endif
#endif
	/*  Below this line are things that _may_ be needed here  */
	#ifdef STUFF_FROM_READIMAGE	// D.I. Moved this stuff from ReadImage to OutputRow()
	if (Depth == 8)
	{
		while (BufferSize && ReadMode == ReadImageMode)
		{
			*PcxBufPtr++ = GetByte();
			PcxBufUsed++;
			if (PcxBufUsed >= RowBytes)
			{
           		Parent.HasRow(PcxRowBuffer, CurrentRow, 0, Width);
				CurrentRow++;
				if (CurrentRow >= Height)
				{
					ReadMode = CheckTrailerMode;
					break;
				}
				PcxBufUsed = 0;
				PcxBufPtr = PcxRowBuffer;
			}
		}
	}
	else if (Depth == 1)
	{
		while (BufferSize && ReadMode == ReadImageMode)
		{
			*PcxBufPtr++ = GetByte();
			PcxBufUsed++;
			if (PcxBufUsed >= RowBytes)
			{
				Parent.HasRow(PcxRowBuffer, CurrentRow, 0, Width);
				CurrentRow++;
				if (CurrentRow >= Height)
				{
					ReadMode = CheckTrailerMode;
					break;
				}
				PcxBufUsed = 0;
				PcxBufPtr = PcxRowBuffer;
			}
		}
	}
	else if (Depth > 8)
	{
		while (BufferSize && ReadMode == ReadImageMode)
		{
			*PcxBufPtr = GetByte();
			PcxBufPtr += TrueColorPixelSize;
			PcxBufUsed++;
			if (PcxBufUsed >= RowBytes)
			{
				if (Pcx24Color < 2)
				{
					Pcx24Color++;
					if (Pcx24Color == 1)
						PcxOffset = TrueColorGreenOffset;
					else
						PcxOffset = TrueColorBlueOffset;
				}
				else
				{
					Parent.HasRow(PcxRowBuffer, CurrentRow, 0, Width);
					CurrentRow++;
					if (CurrentRow >= Height)
					{
						ReadMode = CheckTrailerMode;
						break;
					}
					Pcx24Color = 0;
					PcxOffset = TrueColorRedOffset;
				}
				PcxBufUsed = 0;
				PcxBufPtr = PcxRowBuffer + PcxOffset;
			}
		}
	}
	else // Depth == 4, 4 planes of monochrome data
	{
		while (BufferSize && ReadMode == ReadImageMode)
		{
			byte v = GetByte();
			for (int m = 0x80; m; m >>= 1)
			{
				if (v & m)
				{
					*PcxBufPtr |= PcxMask;
				}
				else
				{
					*PcxBufPtr &= ~PcxMask;
				}
				if (PcxMask >= 0x10)
				{
					PcxMask >>= 4;
				}
				else
				{
					PcxMask <<= 4;
					PcxBufPtr++;
					PcxBufUsed++;
				}
			}
			if (PcxBufUsed >= RowBytes)
			{
				if (PcxMask < 0x10)
					PcxMask <<= 4;
				if (PcxMask != 0x10)
				{
					PcxMask >>= 1;
				}
				else
				{
					Parent.HasRow(PcxRowBuffer, CurrentRow, 0, Width);
					CurrentRow++;
					if (CurrentRow >= Height)
					{
						ReadMode = CheckTrailerMode;
						break;
					}
					PcxMask = 0x80;
				}
				PcxBufUsed = 0;
				PcxBufPtr = PcxRowBuffer;
			}
		}
	}
	#endif
}
void
G42PcxImage::ProcessData(byte * buffer, size_t buffer_size)
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
			case CheckTrailerMode:
				CheckTrailer();
				break;
			default:
				EatBuffer();
				break;
		}
	}
}
void
G42PcxImage::ReadHeader(void)
{
   if (BufferSize < 128) // 128 is worst case behavior
	{
		SaveBuffer();
		return;
	}
  	IntegerFormat = Intel;
	// check header ('BM')
	if (GetByte() != 0x0a)
		return;
	SkipData(1); // skip version for now
	if (GetByte() != 1) // RLE compression only
		return;
	int bits_per_pixel = GetByte();
	uint16 x_start = GetUInt16();
	uint16 y_start = GetUInt16();
	Width = GetUInt16() - x_start  + 1;
	Height = GetUInt16() - y_start + 1;
	SkipData(4); // resolutions aren't useful to us
	G42Color * ega_palette = new G42Color[16];
	if (!ega_palette)
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		return;
	}
	{
		for (int i = 0; i < 16; i++)
		{
			ega_palette[i].red = GetByte();
			ega_palette[i].green = GetByte();
			ega_palette[i].blue = GetByte();
		}
	}
	SkipData(1);
	int planes = GetByte();
	PlaneBytes = GetUInt16();
	RowBytes = planes * PlaneBytes;		// D.I.
	SkipData(60); // move to the image
	if (planes == 1 && bits_per_pixel == 1)
	{
		Depth = 1;
		NumPalette = 0;
		delete [] ega_palette;
		Info.ColorType = G42ImageInfo::ColorTypeGrayscale;
		Info.PixelDepth = Depth;
		Info.NumPalette = NumPalette;
	}
	else if (planes == 4 && bits_per_pixel == 1)
	{
		Depth = 4;
		NumPalette = 16;
		Info.ImagePalette.Set(ega_palette);
		Info.ColorType = G42ImageInfo::ColorTypePalette;
		Info.PixelDepth = Depth;
		Info.NumPalette = NumPalette;
	}
	else if (planes == 1 && bits_per_pixel == 8)
	{
		Depth = 8;
		NumPalette = 256;
		delete [] ega_palette;
		G42Color * pal = new G42Color[256];
		memset(pal, 0, 256 * sizeof (G42Color));
		if (!pal)
		{
			Valid = false;
			ReadMode = ReadErrorMode;
			return;
		}
		Info.ImagePalette.Set(pal);
		Info.ColorType = G42ImageInfo::ColorTypePalette;
		Info.PixelDepth = Depth;
		Info.NumPalette = NumPalette;
	}
	else if (planes == 3 && bits_per_pixel == 8)
	{
		Depth = 24;	// D.I.
		NumPalette = 0;
		Info.ColorType = G42ImageInfo::ColorTypeTrueColor;
		Info.PixelDepth = 8 * TrueColorPixelSize;
		delete [] ega_palette;
	}
	else
	{
		delete [] ega_palette;
		Valid = false;
		ReadMode = ReadErrorMode;
		return;
	}
	PcxRowBuffer = new byte [RowBytes];
	if (!PcxRowBuffer)
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		return;
	}
	Info.Width = Width;
	Info.Height = Height;
	Valid = true;
	CurrentPalette = 0;
	Info.ImageType = Pcx;
	Info.Compressed = General;
	ReadMode = ReadImageMode; // 8 bit palette is after image
	Parent.HasInfo(Info);         // give user the file dimensions
}
void
G42PcxImage::CheckTrailer(void)
{
	if (GetByte() == 0x0C)	// D.I. Changed to 0x0C from 0xC0
	{
		ReadMode = ReadPaletteMode;
	}
	else
	{
		ReadMode = ReadDoneMode;
		ForceFileEnd();
	}
}
void
G42PcxImage::ReadPalette(void)
{
	// read until palette is complete or buffer is empty
	while (BufferSize >= 3 && CurrentPalette < NumPalette)
	{
		((G42Color *)(Info.ImagePalette))[CurrentPalette].red = GetByte();
		((G42Color *)(Info.ImagePalette))[CurrentPalette].green = GetByte();
		((G42Color *)(Info.ImagePalette))[CurrentPalette].blue = GetByte();
		#ifdef DEBUG_STUFF	// D.I. Trying to set everything black
		((G42Color *)(Info.ImagePalette))[CurrentPalette].blue = 0;
		((G42Color *)(Info.ImagePalette))[CurrentPalette].green = 0;
		((G42Color *)(Info.ImagePalette))[CurrentPalette].red = 0;
		#endif
		CurrentPalette++;
	}
	if (CurrentPalette == NumPalette)
	{
		ReadMode = ReadDoneMode;
		ForceFileEnd();
	}
	else if (BufferSize)
	{
		SaveBuffer(); // save for rest of data
	}
}
void
G42PcxImage::ReadImage(void)
{
	/*  D.I.  Decompress the input stream.  This code pulled from Targa reader
		It probably doesn't even need to be modal like the Targa since decompression
		is so simple */
	while (BufferSize && ReadMode == ReadImageMode)
	{
		switch (DecompressionMode)
		{
			case StartDecompression:
			{
				byte active_byte = GetByte();
				if ((0xC0 & active_byte) == 0xC0)
				{
					RunCount = (0x3F & active_byte);
					DecompressionMode = GetRunValue;
				}
				else
				{
					PutByte(active_byte);
				}
				break;
			}
			case GetRunValue:
			{
				byte active_byte = GetByte();
				PutByteRun(active_byte, RunCount);
				DecompressionMode = StartDecompression;
				break;
			}
		}
	}
}
bool G42IsPcx(byte * buffer, size_t)
{
	if (buffer[0] == 0x0a)
		return true;
	return false;
}

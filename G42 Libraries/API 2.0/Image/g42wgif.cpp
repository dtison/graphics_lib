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
#include "g42wgif.h"
G42GifWriter::G42GifWriter(G42RasterImageWriter & parent) :
	G42RasterWriterBase(parent), Lzw(0), CurrentPass(0), Progressive(false),
	TempRow(0), LzwOutput(0)
{
}
G42GifWriter::~G42GifWriter()
{
	if (TempRow)
	{
		delete [] TempRow;
		TempRow = 0;
	}
	if (Lzw)
	{
		delete Lzw;
      Lzw = 0;
	}
	if (LzwOutput)
	{
		delete LzwOutput;
      LzwOutput = 0;
	}
	Valid = false;
}
bool
G42GifWriter::WriteInfo(G42ImageInfo * info)
{
	if (info->PixelDepth > 8 ||
		info->ColorType == G42ImageInfo::ColorTypeTrueColor) // GIF's must be 8 bits or less
	{
		Valid = false;
		return false;
	}
	G42RasterWriterBase::WriteInfo(info); // set up base stuff
	RowBytes = Width; // always use depth of 8, even if less
	Transparent = info->IsTransparent;
	if (info->ColorType == G42ImageInfo::ColorTypePalette)
	{
		TransparentPixel = info->NumPalette;
		OpaquePixel = TransparentPixel - 1;
	}
	else
	{
		TransparentPixel = info->TransparentColor.red; // grayscale, better be ok
		if (TransparentPixel >= (1 << (Depth - 1)))
			OpaquePixel = TransparentPixel - 1;
		else
			OpaquePixel = TransparentPixel + 1;
	}
	if (!Transparent)
		TransparentPixel = 0;
	Progressive = info->Progressive;
	if (Height < 5) // just being safe
   	Progressive = false;
	uint num_palette;
	if (info->ColorType == G42ImageInfo::ColorTypeGrayscale)
	{
		num_palette = (1 << Depth);
	}
	else
	{
		num_palette = info->NumPalette;
		if (Transparent)
			num_palette++;
		if (num_palette > 256)
		{
			Valid = false;
			return false;
		}
	}
	uint code_depth = 2;
	while (code_depth < 8 && num_palette > (1 << code_depth))
		code_depth++;
	TempRow = new byte [Width];
	if (!TempRow)
	{
		Valid = false;
		  return false;
	}
	IntegerFormat = Intel;
	if (Transparent)
		PutBytes((const byte *)"GIF89a", 6); // note: comments or transparency will change this
	else
		PutBytes((const byte *)"GIF87a", 6); // note: comments or transparency will change this
	PutUInt16((uint16)Width);
	PutUInt16((uint16)Height);
	PutByte((byte)(0x80 | (code_depth - 1)));
	PutByte(TransparentPixel);
	PutByte(0);
	if (info->ColorType == G42ImageInfo::ColorTypeGrayscale)
	{
		byte buf[3];
		if (Depth == 1)
		{
			buf[0] = buf[1] = buf[2] = 0;
			PutBytes(buf, 3);
			buf[0] = buf[1] = buf[2] = 0xff;
			PutBytes(buf, 3);
			buf[0] = buf[1] = buf[2] = 0;
			PutBytes(buf, 3);
			PutBytes(buf, 3);
		}
		else if (Depth == 4)
		{
			for (int i = 0; i < 16; i++)
			{
				buf[0] = buf[1] = buf[2] = (i | (i << 4));
				PutBytes(buf, 3);
			}
		}
		else
		{
			for (int i = 0; i < 256; i++)
			{
				buf[0] = buf[1] = buf[2] = i;
				PutBytes(buf, 3);
			}
		}
	}
	else
	{
		const G42Color * pal = info->ImagePalette;
      byte buf[3];
		for (int i = 0; i < (1 << code_depth); i++)
		{
			if (Transparent && i == TransparentPixel)
			{
				buf[0] = info->TransparentColor.red;
				buf[1] = info->TransparentColor.green;
				buf[2] = info->TransparentColor.blue;
			}
			else if (i < num_palette)
			{
				buf[0] = pal[i].red;
				buf[1] = pal[i].green;
				buf[2] = pal[i].blue;
			}
			else
			{
				buf[0] = buf[1] = buf[2] = 0;
			}
			PutBytes(buf, 3);
		}
	}
	if (Transparent)
	{
		PutByte(0x21);
		PutByte(0xf9);
		PutByte(4);
		PutByte(0x1);
		PutByte(0);
		PutByte(0);
		PutByte(TransparentPixel);
		PutByte(0);
	}
	PutByte(',');
	PutByte(0);
	PutByte(0);
	PutByte(0);
	PutByte(0);
	PutUInt16((uint16)Width);
	PutUInt16((uint16)Height);
	PutByte((byte)(Progressive ? 0x40 : 0));
	LzwOutput = new G42GifLzwOutput(*this, code_depth);
	Lzw = new G42LzwOutput(LzwOutput, G42LzwOutput::LzwGif, code_depth);
   return Valid;
}
bool
G42GifWriter::WriteRow(const byte * row, const byte * mask)
{
	const byte * buffer;
	if (Depth < 8)
	{
		buffer = TempRow;
		if (Depth == 1)
		{
			const byte * sp = row;
			byte * dp = TempRow;
			int m = 0x80;
			for (uint32 i = 0; i < Width; i++)
			{
				if (*sp & m)
					*dp++ = 1;
				else
					*dp++ = 0;
				if (m == 1)
				{
					m = 0x80;
               sp++;
				}
				else
					m >>= 1;
			}
		}
		else if (Depth == 4)
		{
			const byte * sp = row;
			byte * dp = TempRow;
			int s = 4;
			for (uint32 i = 0; i < Width; i++)
			{
         	*dp++ = ((*sp >> s) & 0xf);
				if (!s)
				{
					s = 4;
					sp++;
				}
				else
					s = 0;
			}
		}
	}
	else
	{
		memcpy(TempRow, row, Width);
	}
	if (Transparent && mask)
	{
		byte * sp = TempRow;
		const byte * mp = mask;
		int m = 0x80;
		for (uint32 i = 0; i < Width; i++)
		{
			if (*mp & m)
				*sp = TransparentPixel;
			else if (*sp == TransparentPixel)
				*sp = OpaquePixel;
			sp++;
			if (m == 1)
			{
				m = 0x80;
				mp++;
			}
			else
			{
				m >>= 1;
			}
		}
	}
	Lzw->ProcessBuffer(TempRow, Width);  // this calls PutBytes directly
	AdvanceCurrentRow();
	return Valid;
}
bool
G42GifWriter::WriteEnd(G42ImageInfo * info)
{
	Lzw->Flush();
	PutByte(0);  // empty data block
	PutByte(';'); // end of file
	return G42RasterWriterBase::WriteEnd(info);
}
void
G42GifWriter::AdvanceCurrentRow(void)
{
	if (Progressive)
	{
		switch (CurrentPass)
		{
		case 0:
			CurrentRow += 8;
			if (CurrentRow >= Height)
			{
				CurrentRow = 4;
				CurrentPass++;
			}
			break;
		case 1:
			CurrentRow += 8;
			if (CurrentRow >= Height)
			{
				CurrentRow = 2;
				CurrentPass++;
			}
			break;
		case 2:
			CurrentRow += 4;
			if (CurrentRow >= Height)
			{
				CurrentRow = 1;
				CurrentPass++;
			}
			break;
		case 3:
			CurrentRow += 2;
			break;
		}
	}
	else
	{
		CurrentRow++;
	}
}
G42GifLzwOutput::G42GifLzwOutput(G42RasterWriterBase & base, int code_depth) :
	Base(base)
{
	Base.PutByte((byte)code_depth);
}
G42GifLzwOutput::~G42GifLzwOutput()
{
}
void
G42GifLzwOutput::OutputBuffer(byte * buffer, uint size)
{
	while (size)
	{
		uint block_size = min(size, (uint)255);
		Base.PutByte(block_size);
		Base.PutBytes(buffer, block_size);
		buffer += block_size;
		size -= block_size;
	}
}

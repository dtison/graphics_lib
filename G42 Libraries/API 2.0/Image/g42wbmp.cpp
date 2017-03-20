// g42wbmp.cpp - Bmp File
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
#include "g42wbmp.h"
#include "g42idata.h"
#include "g42iter.h"
G42BmpWriter::G42BmpWriter(G42RasterImageWriter & parent) :
	G42RasterWriterBase(parent), TempRow(0), CurrentBmpRow(0)
{
}
G42BmpWriter::~G42BmpWriter()
{
	if (TempRow)
	{
		delete [] TempRow;
		TempRow = 0;
	}
	Valid = false;
}
bool
G42BmpWriter::WriteInfo(G42ImageInfo * info)
{
	G42RasterWriterBase::WriteInfo(info); // set up base stuff
   CurrentBmpRow = (int32)Height - 1;
	if (Depth > 8)
	{
		Depth = 24;
		RowBytes = Width * 3;
	}
	RowBytes = ((RowBytes + 3) & ~3);
	uint num_palette;
	if (info->ColorType == G42ImageInfo::ColorTypeGrayscale)
		num_palette = (1 << (uint)Depth);
	else
		num_palette = info->NumPalette;
	TempRow = new byte [(size_t)RowBytes];
	if (!TempRow)
	{
		Valid = false;
		return false;
	}
	IntegerFormat = Intel;
	PutByte('B');
	PutByte('M');
	uint32 image_size = Width;
	if (Depth > 8)
		image_size *= 3;
	else
		image_size = ((image_size * (uint32)Depth + 7) >> 3);
	image_size = ((image_size + 3) & ~3);
	image_size *= Height;
	uint32 image_bytes = image_size;
	image_bytes += 14;
	if (info->ImageType == BmpWin)
		image_bytes += 40;
	else
		image_bytes += 64;
	/* add size of palette 		*/
	if (Depth <= 8)
		image_bytes += (uint32)((1 << (uint)Depth) * 4);
	PutUInt32(image_bytes);
	PutUInt16(0);
	PutUInt16(0);
	uint16 num_pal_entries;
	if (Depth > 8)
		num_pal_entries = 0;
	else
		num_pal_entries = (uint16)(1 << (uint)Depth);
	if (info->ImageType == BmpWin)
	{
		PutUInt32(54 + (num_pal_entries << 2));
		PutUInt32(40);
	}
	else
	{
		PutUInt32(78 + (num_pal_entries << 2));
		PutUInt32(64);
	}
	PutUInt32(Width);
	PutUInt32(Height);
	PutUInt16(1);
	PutUInt16((uint16)Depth);
	PutUInt32(0);
	PutUInt32(0);
	PutUInt32(0);
	PutUInt32(0);
	PutUInt32(num_pal_entries);
	PutUInt32(num_palette);
	if (info->ImageType == BmpOS2)
	{
		PutUInt16(0);
		PutUInt16(0);
		PutUInt16(0);
		PutUInt16(0);
		PutUInt32(0);
		PutUInt32(0);
		PutUInt32(0);
		PutUInt32(0);
	}
	if (info->ColorType == G42ImageInfo::ColorTypeGrayscale)
	{
		byte buf[4];
		buf[3] = 0;
		if (Depth == 1)
		{
			buf[0] = buf[1] = buf[2] = 0;
			PutBytes(buf, 4);
			buf[0] = buf[1] = buf[2] = 0xff;
			PutBytes(buf, 4);
		}
		else if (Depth == 4)
		{
			for (int i = 0; i < 16; i++)
			{
				buf[0] = buf[1] = buf[2] = (i | (i << 4));
				PutBytes(buf, 4);
			}
		}
		else
		{
			for (int i = 0; i < 256; i++)
			{
				buf[0] = buf[1] = buf[2] = i;
				PutBytes(buf, 4);
			}
		}
	}
	else if (Depth <= 8)
	{
		const G42Color * pal = info->ImagePalette;
		byte buf[4];
		buf[3] = 0;
		for (int i = 0; i < (int)(1 << (uint)Depth); i++)
		{
			if (i < num_palette)
			{
				buf[2] = pal[i].red;
				buf[1] = pal[i].green;
				buf[0] = pal[i].blue;
			}
			else
			{
				buf[0] = buf[1] = buf[2] = 0;
			}
			PutBytes(buf, 4);
		}
	}
	return Valid;
}
bool
G42BmpWriter::WriteRow(const byte * row, const byte *)
{
#if !defined(MSWIN)
// MSWIN doesn't need translation, this being it's native format
	if (Depth > 8)
	{
		G42ConstByte24BitIterator sp(row);
		byte * dp = TempRow;
		for (uint32 i = 0; i < Width; i++)
		{
			*dp++ = sp.GetBlue();
			*dp++ = sp.GetGreen();
			*dp++ = sp.GetRed();
			sp.Increment();
		}
	}
	else
#endif
	{
		memcpy(TempRow, row, (size_t)RowBytes);
	}
	G42RasterWriterBase::WriteRow(TempRow, 0);
	CurrentBmpRow--;
	return Valid;
}
uint32
G42BmpWriter::NextRow(void)
{
	return CurrentBmpRow;
}
bool
G42BmpWriter::NeedRow(void)
{
	return (CurrentBmpRow >= 0);
}

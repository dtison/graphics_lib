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
#include "giwbmp.h"
G42ImageWriterBmp::G42ImageWriterBmp(G42Notifier & notifier,
	G42BaseOptions * options)
:
	G42ImageWriterBase(notifier, options), TempRow(0), CurrentBmpRow(0)
{
	SetInfo(G42BmpTypeId, "bmp", "Bmp");
}
G42ImageWriterBmp::G42ImageWriterBmp(void)
:
	G42ImageWriterBase(), TempRow(0), CurrentBmpRow(0)
{
	SetInfo(G42BmpTypeId, "bmp", "Bmp");
}
G42ImageWriterBmp::~G42ImageWriterBmp()
{
	if (TempRow)
	{
		delete [] TempRow;
		TempRow = 0;
	}
	Valid = false;
}
bool
G42ImageWriterBmp::WriteInfo(G42ImageDescriptor * info)
{
	G42ImageWriterBase::WriteInfo(info); // set up base stuff
   CurrentBmpRow = (int32)Height - 1;
	if (Depth > 8)
	{
		Depth = 24;
		RowBytes = Width * 3;
	}
	RowBytes = ((RowBytes + 3) & ~3);
	uintf num_palette;
	if (!info->IsColor())
		num_palette = (1 << (uintf)Depth);
	else
		num_palette = info->GetNumPalette();
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
	image_bytes += 40;
	/* add size of palette 		*/
	if (Depth <= 8)
		image_bytes += (uint32)((1 << (uintf)Depth) * 4);
	PutUInt32(image_bytes);
	PutUInt16(0);
	PutUInt16(0);
	uint16 num_pal_entries;
	if (Depth > 8)
		num_pal_entries = 0;
	else
		num_pal_entries = (uint16)(1 << (uintf)Depth);
	PutUInt32(54 + (num_pal_entries << 2));
	PutUInt32(40);
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
	if (!info->IsColor())
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
			for (intf i = 0; i < 16; i++)
			{
				buf[0] = buf[1] = buf[2] = (byte)(i | (i << 4));
				PutBytes(buf, 4);
			}
		}
		else
		{
			for (intf i = 0; i < 256; i++)
			{
				buf[0] = buf[1] = buf[2] = (byte)i;
				PutBytes(buf, 4);
			}
		}
	}
	else if (Depth <= 8)
	{
		const G42Color * pal = info->GetPalette();
		byte buf[4];
		buf[3] = 0;
		for (intf i = 0; i < (intf)(1 << (uintf)Depth); i++)
		{
			if (i < num_palette)
			{
				buf[2] = pal[i].Red;
				buf[1] = pal[i].Green;
				buf[0] = pal[i].Blue;
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
G42ImageWriterBmp::WriteRow(const byte * row, const byte *)
{
#if !defined(MSWIN)
// MSWIN doesn't need translation, this being it's native format
	if (Depth > 8)
	{
		if (Descriptor->GetDepth() == 24)
		{
			G42ConstColor24Iterator sp(row);
			byte * dp = TempRow;
			for (uint32 i = 0; i < Width; i++)
			{
				*dp++ = sp.GetBlue();
				*dp++ = sp.GetGreen();
				*dp++ = sp.GetRed();
				sp.Increment();
			}
			G42ImageWriterBase::WriteRow(TempRow, 0);
		}
		else
		{
			G42ConstColor32Iterator sp(row);
			byte * dp = TempRow;
			for (uint32 i = 0; i < Width; i++)
			{
				*dp++ = sp.GetBlue();
				*dp++ = sp.GetGreen();
				*dp++ = sp.GetRed();
				sp.Increment();
			}
			G42ImageWriterBase::WriteRow(TempRow, 0);
		}
	}
	else
#endif
	{
//		memcpy(TempRow, row, (size_t)RowBytes);
//		G42ImageWriterBase::WriteRow(TempRow, 0);
		G42ImageWriterBase::WriteRow(row, 0);
	}
	CurrentBmpRow--;
	return Valid;
}
uint32
G42ImageWriterBmp::NextRow(void)
{
	return CurrentBmpRow;
}
bool
G42ImageWriterBmp::NeedRow(void)
{
	return (CurrentBmpRow >= 0);
}

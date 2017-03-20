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
#include "giwpcx.h"
G42ImageWriterPcx::G42ImageWriterPcx(void) :
	G42ImageWriterBase(), TempRow(0)
{
	SetInfo(G42PcxTypeId, "pcx", "Pcx");
}
G42ImageWriterPcx::G42ImageWriterPcx(G42Notifier & notifier,
	G42BaseOptions * options) :
	G42ImageWriterBase(notifier, options), TempRow(0)
{
	SetInfo(G42PcxTypeId, "pcx", "Pcx");
}
G42ImageWriterPcx::~G42ImageWriterPcx()
{
	if (TempRow)
	{
		delete [] TempRow;
		TempRow = 0;
	}
	Valid = false;
}
bool
G42ImageWriterPcx::WriteInfo(G42ImageDescriptor * info)
{
	G42ImageWriterBase::WriteInfo(info); // set up base stuff
	if (Depth > 8)
		Depth = 24;
	if (Depth < 8)
		RowBytes = (Width + 7) >> 3;
	else
		RowBytes = Width;
	RowBytes = ((RowBytes + 1) & ~1L);
	TempRow = new byte [(size_t)RowBytes];
	if (!TempRow)
	{
		Valid = false;
		return false;
	}
	IntegerFormat = Intel;
	PutByte(0xa);
	PutByte(5);
	PutByte(1);
	if (Depth < 8)
		PutByte(1);
	else
		PutByte(8);
	PutUInt16(0);
	PutUInt16(0);
	PutUInt16((uint16)(Width - 1));
	PutUInt16((uint16)(Height - 1));
	PutUInt16(0);
	PutUInt16(0);
	if (Depth == 4)
	{
		if (!info->IsColor())
		{
			for (int i = 0; i < 256; i += 17)
			{
				PutByteRun((byte)i, 3);
			}
		}
		else
		{
			const G42Color * pal = info->GetPalette();
			for (int i = 0; i < 16; i++)
			{
				if (i < info->GetNumPalette())
				{
					PutByte(pal[i].Red);
					PutByte(pal[i].Green);
					PutByte(pal[i].Blue);
				}
				else
				{
					PutByteRun(0, 3);
				}
			}
		}
	}
	else
	{
		PutByteRun(0, 48);
	}
	PutByte(0);
	if (Depth == 1 || Depth == 8)
		PutByte(1);
	else if (Depth > 8)
		PutByte(3);
	else
		PutByte(4);
	PutUInt16((uint16)RowBytes);
	if (!info->IsColor())
		PutUInt16(2);
	else
		PutUInt16(1);
	PutUInt16((uint16)(Width - 1));
	PutUInt16((uint16)(Height - 1));
	PutByteRun(0, 54);
	return Valid;
}
bool
G42ImageWriterPcx::WriteRow(const byte * row, const byte *)
{
	if (Depth == 24)
	{
		if (Descriptor->GetDepth() == 24)
		{
			G42ConstColor24Iterator sp(row);
			byte * dp = TempRow;
			uint32 i;
			for (i = 0; i < Width; i++)
			{
				*dp++ = (byte)G42ConstColor24(*sp).GetRed();
				sp++;
			}
			if (RowBytes > Width)
				*dp = *(dp - 1); // fill in with duplicate byte
			CompressRow(TempRow);
		}
		else
		{
			G42ConstColor32Iterator sp(row);
			byte * dp = TempRow;
			uint32 i;
			for (i = 0; i < Width; i++)
			{
				*dp++ = (byte)G42ConstColor32(*sp).GetRed();
				sp++;
			}
			if (RowBytes > Width)
				*dp = *(dp - 1); // fill in with duplicate byte
			CompressRow(TempRow);
		}
		if (Descriptor->GetDepth() == 24)
		{
			G42ConstColor24Iterator sp(row);
			byte * dp = TempRow;
			uint32 i;
			for (i = 0; i < Width; i++)
			{
				*dp++ = (byte)G42ConstColor24(*sp).GetGreen();
				sp++;
			}
			if (RowBytes > Width)
				*dp = *(dp - 1); // fill in with duplicate byte
			CompressRow(TempRow);
		}
		else
		{
			G42ConstColor32Iterator sp(row);
			byte * dp = TempRow;
			uint32 i;
			for (i = 0; i < Width; i++)
			{
				*dp++ = (byte)G42ConstColor32(*sp).GetGreen();
				sp++;
			}
			if (RowBytes > Width)
				*dp = *(dp - 1); // fill in with duplicate byte
			CompressRow(TempRow);
		}
		if (Descriptor->GetDepth() == 24)
		{
			G42ConstColor24Iterator sp(row);
			byte * dp = TempRow;
			uint32 i;
			for (i = 0; i < Width; i++)
			{
				*dp++ = (byte)G42ConstColor24(*sp).GetBlue();
				sp++;
			}
			if (RowBytes > Width)
				*dp = *(dp - 1); // fill in with duplicate byte
			CompressRow(TempRow);
		}
		else
		{
			G42ConstColor32Iterator sp(row);
			byte * dp = TempRow;
			uint32 i;
			for (i = 0; i < Width; i++)
			{
				*dp++ = (byte)G42ConstColor32(*sp).GetBlue();
				sp++;
			}
			if (RowBytes > Width)
				*dp = *(dp - 1); // fill in with duplicate byte
			CompressRow(TempRow);
		}
	}
	else if (Depth == 8)
	{
		memcpy(TempRow, row, Width);
		if (RowBytes > Width)
			TempRow[RowBytes - 1] = TempRow[Width - 1];
		CompressRow(TempRow);
	}
	else if (Depth == 4)
	{
		for (int start_mask = 0x10; start_mask < 0x100; start_mask <<= 1)
		{
			const byte * sp = row;
			int mask = start_mask;
			byte * dp = TempRow;
			int dm = 0x80;
			byte v = 0;
			for (uint32 i = 0; i < Width; i++)
			{
				if (*sp & mask)
					v |= (byte)dm;
				if (mask < 0x10)
				{
					mask <<= 4;
					sp++;
				}
				else
				{
					mask >>= 4;
				}
				if (dm == 1)
				{
					*dp++ = v;
					v = 0;
					dm = 0x80;
				}
				else
				{
					dm >>= 1;
				}
			}
			if (dm != 0x80)
			{
				*dp++ = v;
			}
			if (RowBytes > ((Width + 7) >> 3))
			{
				*dp = *(dp - 1);
			}
			CompressRow(TempRow);
		}
	}
	else
	{
		memcpy(TempRow, row, ((Width + 7) >> 3));
		if (RowBytes > ((Width + 7) >> 3))
			TempRow[RowBytes - 1] = TempRow[((Width + 7) >> 3) - 1];
		CompressRow(TempRow);
	}
	CurrentRow++; // increment the base's current row
	return Valid;
}
void
G42ImageWriterPcx::CompressRow(const byte * row)
{
	const byte * rp = row + 1;
	uint32 bytes_left = RowBytes - 1;
	int count = 1;
	byte ref = *row;
	while (bytes_left)
	{
		if (*rp == ref && count < 63)
		{
			count++;
			bytes_left--;
			rp++;
		}
		else
		{
			if (count > 1 || ((ref & 0xc0) == 0xc0))
			{
				PutByte((byte)(count | 0xc0));
				PutByte(ref);
			}
			else
			{
				PutByte(ref);
			}
			ref = *rp++;
			bytes_left--;
			count = 1;
		}
	}
	if (count > 1 || ((ref & 0xc0) == 0xc0))
	{
		PutByte((byte)(count | 0xc0));
		PutByte(ref);
	}
	else
	{
		PutByte(ref);
	}
}
bool
G42ImageWriterPcx::WriteEnd(void)
{
	if (Depth == 8)
	{
		PutByte(0xc);
		if (!Descriptor->IsColor())
		{
			for (int i = 0; i < 256; i++)
			{
				PutByteRun((byte)i, 3);
			}
		}
		else
		{
			const G42Color * pal = Descriptor->GetPalette();
			for (int i = 0; i < 256; i++)
			{
				if (i < Descriptor->GetNumPalette())
				{
					PutByte(pal[i].Red);
					PutByte(pal[i].Green);
					PutByte(pal[i].Blue);
				}
				else
				{
					PutByteRun(0, 3);
				}
			}
		}
	}
	return G42ImageWriterBase::WriteEnd();
}

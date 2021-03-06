// g42ras.cpp - Sun Raster File
/*************************************************************
	File:          g42wras.cpp
	Copyright (c) 1996, Group 42, Inc.
	Description:   Derived class of g42wibase that handles Sun Raster files
   Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
      Code   Date   Name and Description
			6-24-96	D. Ison Bug Fixes
***************************************************************/
#include "g42itype.h"
#include "g42iter.h"
#include "g42wras.h"
G42RasWriter::G42RasWriter(G42RasterImageWriter & parent) :
	G42RasterWriterBase(parent), TempRow(0)
{
}
G42RasWriter::~G42RasWriter()
{
	if (TempRow)
	{
		delete [] TempRow;
		TempRow = 0;
	}
	Valid = false;
}
bool
G42RasWriter::WriteInfo(G42ImageInfo * info)
{
	G42RasterWriterBase::WriteInfo(info); // set up base stuff
	TrueDepth = (int)Depth;
	if (Depth > 8)
	{
		Depth = 24;
		RowBytes = Width * 3;
	}
	else if (Depth > 1)
	{
		Depth = 8;
		RowBytes = Width;
	}
	else
	{
		RowBytes = ((Width + 7) >> 3);
	}
	RowBytes = ((RowBytes + 1) & ~1L);
	TempRow = new byte [(size_t)RowBytes];
	if (!TempRow)
	{
		Valid = false;
		return false;
	}
	IntegerFormat = Motorola;
	PutUInt32(0x59a66a95L);
	PutUInt32(Width);
	PutUInt32(Height);
	PutUInt32(Depth);
	PutUInt32(Height * RowBytes);
	if (info->Compressed)
	{
		PutUInt32(2);
		Compressed = true;
	}
	else
	{
		PutUInt32(1);
		Compressed = false;
	}
	if (Depth == 24)
	{
		PutUInt32(0);
		PutUInt32(0);
	}
	else if (Depth == 8)
	{
		PutUInt32(1);
		PutUInt32(768);
	}
	else // Depth == 1
	{
		PutUInt32(1);
		PutUInt32(6);
	}
	if (info->ColorType == G42ImageInfo::ColorTypeGrayscale)
	{
		if (Depth == 1)
		{
			byte buf[6];
			buf[0] = buf[2] = buf[4] = 0;
			buf[1] = buf[3] = buf[5] = 0xff;
			PutBytes(buf, 6);
		}
		else
		{
			for (int c = 0; c < 3; c++)
			{
				for (int i = 0; i < 256; i++)
				{
					PutByte(i);
				}
			}
		}
	}
	else if (Depth <= 8)
	{
		const G42Color * pal = info->ImagePalette;
		int num_palette = info->NumPalette;
		for (int i = 0; i < (int)(1 << (uint)Depth); i++)
		{
			if (i < num_palette)
				PutByte(pal[i].red);
			else
				PutByte(0);
		}
		for (int i = 0; i < (int)(1 << (uint)Depth); i++)
		{
			if (i < num_palette)
				PutByte(pal[i].green);
			else
				PutByte(0);
		}
		for (int i = 0; i < (int)(1 << (uint)Depth); i++)
		{
			if (i < num_palette)
				PutByte(pal[i].blue);
			else
				PutByte(0);
		}
	}
	return Valid;
}
bool
G42RasWriter::WriteRow(const byte * row, const byte *)
{
	if (Depth == 24)
	{
		G42ConstByte24BitIterator sp(row);
		byte * dp = TempRow;
		uint32 i;
		for (i = 0; i < Width; i++)
		{
			*dp++ = (byte)sp.GetBlue();
			*dp++ = (byte)sp.GetGreen();
			*dp++ = (byte)sp.GetRed();
			sp.Increment();
		}
		if (RowBytes > Width)
			*dp = *(dp - 1); // fill in with duplicate byte
		CompressRow(TempRow);
	}
	else if (Depth == 8)
	{
		if (TrueDepth == 4)
		{
			const byte * sp = row;
			byte * dp = TempRow;
			int s = 4;
			for (uint32 i = 0; i < Width; i++)
			{
				*dp++ = (byte)((*sp >> s) & 0xf);
				if (s)
				{
					s = 0;
				}
				else
				{
					s = 4;
					sp++;
				}
			}
			if (RowBytes > ((Width + 1) >> 1))
				*dp = *(dp - 1);
			CompressRow(TempRow);
		}
		else
		{
			memcpy(TempRow, row, Width);
			if (RowBytes > Width)
				TempRow[RowBytes - 1] = TempRow[Width - 1];
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
G42RasWriter::CompressRow(const byte * row)
{
	if (Compressed)
	{
		const byte * rp = row + 1;
		uint32 bytes_left = RowBytes - 1;
		int count = 1;
		byte ref = *row;
		while (bytes_left)
		{
			if (*rp == ref && count < 255)
			{
				count++;
				bytes_left--;
				rp++;
			}
			else
			{
				if (count > 1 || ref == 0x80)
				{
					PutByte(0x80);
					PutByte((byte)(count - 1));
					if (count > 1)
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
		if (count > 1 || ref == 0x80)
		{
			PutByte(0x80);
			PutByte((byte)(count - 1));
			if (count > 1)
				PutByte(ref);
		}
		else
		{
			PutByte(ref);
		}
	}
	else
	{
		uint32 bytes_left = RowBytes;
		const byte * row_ptr = row;
		while (bytes_left)
		{
			uint32 copy_bytes = min(bytes_left, (uint32)OutputBufferLeft);
			PutBytes(row_ptr, (uint)copy_bytes);
			row_ptr += copy_bytes;
			bytes_left -= copy_bytes;
		}
	}
}

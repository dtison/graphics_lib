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
#include "giwras.h"
G42ImageWriterRas::G42ImageWriterRas(void)
:
	G42ImageWriterBase(), TempRow(0)
{
	SetInfo(G42RasTypeId, "ras", "Sun Raster", true);
}
G42ImageWriterRas::G42ImageWriterRas(G42Notifier & notifier,
	G42BaseOptions * options)
:
	G42ImageWriterBase(notifier, options), TempRow(0)
{
	SetInfo(G42RasTypeId, "ras", "Sun Raster", true);
}
G42ImageWriterRas::~G42ImageWriterRas()
{
	if (TempRow)
	{
		delete [] TempRow;
		TempRow = 0;
	}
	Valid = false;
}
bool
G42ImageWriterRas::WriteInfo(G42ImageDescriptor * info)
{
	G42ImageWriterBase::WriteInfo(info); // set up base stuff
	if (!Options || !dynamic_cast<G42ImageReadWriteOptions *>(Options))
	{
		G42ImageReadWriteOptions * options =
			new G42ImageReadWriteOptions(G42RasTypeId);
		G42ThrowIfNull(options);
		AskForOptions(options);
	}
	G42ImageReadWriteOptions * options;
	if (Options)
	{
		options = dynamic_cast<G42ImageReadWriteOptions *>(Options);
	}
	if (options)
	{
		Compressed = options->IsCompressed();
	}
	else
	{
		Compressed = false;
	}
	TrueDepth = (intf)Depth;
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
   G42ThrowIfNull(TempRow);
	IntegerFormat = Motorola;
	PutUInt32(0x59a66a95L);
	PutUInt32(Width);
	PutUInt32(Height);
	PutUInt32(Depth);
	PutUInt32(Height * RowBytes);
	if (Compressed)
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
	if (!info->IsColor())
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
			for (intf c = 0; c < 3; c++)
			{
				for (intf i = 0; i < 256; i++)
				{
					PutByte((byte)i);
				}
			}
		}
	}
	else if (info->IsPaletted())
	{
		const G42Color * pal = info->GetPalette();
		intf num_palette = info->GetNumPalette();
		intf i;
		for (i = 0; i < (intf)(1 << (uintf)Depth); i++)
		{
			if (i < num_palette)
				PutByte(pal[i].Red);
			else
				PutByte(0);
		}
		for (i = 0; i < (intf)(1 << (uintf)Depth); i++)
		{
			if (i < num_palette)
				PutByte(pal[i].Green);
			else
				PutByte(0);
		}
		for (i = 0; i < (intf)(1 << (uintf)Depth); i++)
		{
			if (i < num_palette)
				PutByte(pal[i].Blue);
			else
				PutByte(0);
		}
	}
	return Valid;
}
bool
G42ImageWriterRas::WriteRow(const byte * row, const byte *)
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
				G42ConstColor24 c(*sp);
				*dp++ = (byte)c.GetBlue();
				*dp++ = (byte)c.GetGreen();
				*dp++ = (byte)c.GetRed();
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
				G42ConstColor32 c(*sp);
				*dp++ = (byte)c.GetBlue();
				*dp++ = (byte)c.GetGreen();
				*dp++ = (byte)c.GetRed();
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
G42ImageWriterRas::CompressRow(const byte * row)
{
	if (Compressed)
	{
		const byte * rp = row + 1;
		uint32 bytes_left = RowBytes - 1;
		intf count = 1;
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
			PutBytes(row_ptr, (uintf)copy_bytes);
			row_ptr += copy_bytes;
			bytes_left -= copy_bytes;
		}
	}
}

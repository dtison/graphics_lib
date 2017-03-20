// g42tga.cpp - Targa File
/*************************************************************
	File:          g42pcx.cpp
	Copyright (c) 1996, Group 42, Inc.
	Description:   Derived class of g42wibase that handles Targa files
   Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
      Code   Date   Name and Description
			6-25-96	D. Ison  Fixed some save bugs that were in this file
***************************************************************/
#include "g42itype.h"
#include "g42wtga.h"
#include "g42iter.h"
// note: the support for writing 8 bit tga files is almost present,
// but it will never be used currently, so the if () statement forces
// us to only use 16 and 24 bit flavors
G42TargaWriter::G42TargaWriter(G42RasterImageWriter & parent) :
	G42RasterWriterBase(parent), TempRow(0)
{
}
G42TargaWriter::~G42TargaWriter()
{
	if (TempRow)
	{
		delete [] TempRow;
		TempRow = 0;
	}
	Valid = false;
}
bool
G42TargaWriter::WriteInfo(G42ImageInfo * info)
{
	G42RasterWriterBase::WriteInfo(info); // set up base stuff
	if (info->TargaDepth == 16)
	{
		Depth = 16;
		RowBytes = Width * 2;
	}
	else
	{
		Depth = 24;
		RowBytes = Width * 3;
	}
	TempRow = new byte [(size_t)RowBytes];
	if (!TempRow)
	{
		Valid = false;
		return false;
	}
	IntegerFormat = Intel;
//	ColorType = info->ColorType;
	ColorType = G42ImageInfo::ColorTypeTrueColor;
	Compressed = info->Compressed;
	PutByte(0);
	if (ColorType == G42ImageInfo::ColorTypePalette)
		PutByte(1);
	else
		PutByte(0);
	if (Compressed)
	{
		if (Depth > 8)
			PutByte(10);
		else if (ColorType == G42ImageInfo::ColorTypeGrayscale)
			PutByte(11);
		else
			PutByte(9);
	}
	else
	{
		if (Depth > 8)
			PutByte(2);
		else if (ColorType == G42ImageInfo::ColorTypeGrayscale)
			PutByte(1);
		else
			PutByte(3);
	}
	PutUInt16(0);
	if (ColorType == G42ImageInfo::ColorTypePalette)
	{
		PutUInt16((uint16)(info->NumPalette));
		PutByte(24);
	}
	else
	{
		PutUInt16(0);
		PutByte(0);
	}
	PutUInt16(0);
	PutUInt16(0);
	PutUInt16((uint16)Width);
	PutUInt16((uint16)Height);
	PutByte((byte)Depth);
	PutByte(0x20);	// Origin is upper left instead of usual lower left
	
	if (ColorType == G42ImageInfo::ColorTypePalette)
	{
		const G42Color * pal = info->ImagePalette;
		for (int i = 0; i < info->NumPalette; i++)
		{
			PutByte(pal[i].red);
			PutByte(pal[i].green);
			PutByte(pal[i].blue);
		}
	}
	return Valid;
}
bool
G42TargaWriter::WriteRow(const byte * row, const byte *)
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
		#if 0
		if (RowBytes > Width)
			*dp = *(dp - 1); // fill in with duplicate byte
		#endif
		CompressRow(TempRow);
	}
	else if (Depth == 16)
	{
		G42ConstByte24BitIterator sp(row);
		byte * dp = TempRow;
		uint32 i;
		uint16 v;
		for (i = 0; i < Width; i++)
		{
			#if 1
			v = (uint16)(sp.GetBlue() >> 3);
			v |= (uint16)((sp.GetGreen() & 0xf8) << 2);
			v |= (uint16)((sp.GetRed() & 0xf8) << 7);
			v |= (uint16)0x8000L;
			#endif
			#if 0	// I did all this because I couldn't tell what was _not_ working
			byte blue 	= sp.GetBlue();
			byte green 	= sp.GetGreen();
			byte red 	= sp.GetRed();
			v = (red >> 3); 
			v <<= 5;
			v |= (green >> 3);
			v <<= 5; 
			v |= (blue>> 3);
			#endif
			#if 0	// For some reason this doesn't work on Mac,
			*dp++ = (byte)(v & 0xff);
			*dp++ = (byte)((v & 0xff) >> 8);
			#endif
					// But this does
			*dp++ = (byte) v;
			*dp++ = (byte) (v >> 8);			
			sp.Increment();
		}
		#if 0
		if (RowBytes > Width)
			*dp = *(dp - 1); // fill in with duplicate byte
		#endif
		CompressRow(TempRow);
	}
	else // (Depth == 8)
	{
		memcpy(TempRow, row, Width);
		#if 0
		if (RowBytes > Width)
			TempRow[RowBytes - 1] = TempRow[Width - 1];
		#endif
		CompressRow(TempRow);
	}
	CurrentRow++; // increment the base's current row
	return Valid;
}
void
G42TargaWriter::CompressRow(const byte * row)
{
	if (Compressed)
	{
		int pixel_size = (Depth >> 3);
		const byte * rp = row + pixel_size;
		uint32 bytes_left = RowBytes - pixel_size;
		const byte * start_sequence = row;
		int count = 1;
		bool mode_defined = false;
		bool repeat_mode = false;
		while (bytes_left)
		{
//			bool same = !memcmp(rp, rp - pixel_size, pixel_size);
			bool same = true;
			for (int i = 0; i < pixel_size; i++)
			{
				if (*(rp + i) != *(rp - pixel_size + i))
				{
					same = false;
					break;
				}
			}
			if (!mode_defined)
			{
				repeat_mode = same;
				mode_defined = true;
				count++;
				bytes_left -= pixel_size;
				rp += pixel_size;
			}
			else if (same && repeat_mode && count < 128)
			{
				count++;
				bytes_left -= pixel_size;
				rp += pixel_size;
			}
			else if (!same && !repeat_mode && count < 128)
			{
				count++;
				bytes_left -= pixel_size;
				rp += pixel_size;
			}
			else
			{
				if (repeat_mode)
				{
					PutByte((byte)((count - 1) | 0x80));
					PutBytes(start_sequence, pixel_size);
					mode_defined = false;
					start_sequence = rp;
					count = 1;
				}
				else if (count == 128)
				{
					PutByte((byte)(count - 1));
					PutBytes(start_sequence, pixel_size * count);
					mode_defined = false;
					start_sequence = rp;
					count = 1;
				}
				else // found repeat, don't use this or last pixel and set count = 2
				{
					PutByte((byte)(count - 2));
					PutBytes(start_sequence, pixel_size * (count - 1));
					mode_defined = true;
					repeat_mode = true;
					start_sequence = rp - pixel_size;
					count = 2;
				}
				bytes_left -= pixel_size;
				rp += pixel_size;
			}
		}
		if (!mode_defined || !repeat_mode)
		{
			PutByte((byte)(count - 1));
			PutBytes(start_sequence, pixel_size * count);
		}
		else
		{
			PutByte((byte)((count - 1) | 0x80));
			PutBytes(start_sequence, pixel_size);
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

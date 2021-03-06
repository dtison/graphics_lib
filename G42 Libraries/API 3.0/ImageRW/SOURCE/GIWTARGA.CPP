// g42targa.cpp - Targa File
/*************************************************************
	File:          g42targa.cpp
	Copyright (c) 1996, Group 42, Inc.
	Description:   Derived class of g42wibase that handles Targa files
   Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
      Code   Date   Name and Description
			6-25-96	D. Ison  Fixed some save bugs that were in this file
***************************************************************/
#include "giwtarga.h"
// note: the support for writing 8 bit tga files is almost present,
// but it will never be used currently, so the if () statement forces
// us to only use 16 and 24 bit flavors
G42ImageWriterTarga::G42ImageWriterTarga(void)
:
	G42ImageWriterBase(), TempRow(0)
{
	SetInfo(G42TargaTypeId, "tga", "Targa", true);
}
G42ImageWriterTarga::G42ImageWriterTarga(G42Notifier & notifier,
	G42BaseOptions * options)
:
	G42ImageWriterBase(notifier, options), TempRow(0)
{
	SetInfo(G42TargaTypeId, "tga", "Targa", true);
}
G42ImageWriterTarga::~G42ImageWriterTarga()
{
	if (TempRow)
	{
		delete [] TempRow;
		TempRow = 0;
	}
	Valid = false;
}
bool
G42ImageWriterTarga::WriteInfo(G42ImageDescriptor * info)
{
	G42ImageWriterBase::WriteInfo(info); // set up base stuff
// we need a way to set the true color depth
/*
	if (info->TargaDepth == 16)
	{
		Depth = 16;
		RowBytes = Width * 2;
	}
	else
*/
	{
		Depth = 24;
		RowBytes = Width * 3;
	}
	TempRow = new byte [(size_t)RowBytes];
	G42ThrowIfNull(TempRow);
	IntegerFormat = Intel;
//	ColorType = info->ColorType;
//	ColorType = G42ImageInfo::ColorTypeTrueColor;
	if (!Options || !dynamic_cast<G42ImageReadWriteOptions *>(Options))
	{
		G42ImageReadWriteOptions * options =
			new G42ImageReadWriteOptions(G42TargaTypeId);
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
		TrueColorDepth = options->GetTrueColorDepth();
	}
	else
	{
		Compressed = false;
		TrueColorDepth = 24;
	}
	if (TrueColorDepth > 24)
	{
		TrueColorDepth = 24;
	}
	Depth = TrueColorDepth;
	RowBytes = Width * (Depth >> 3);
	PutByte(0);
/*	if (ColorType == G42ImageInfo::ColorTypePalette)
		PutByte(1);
	else
*/
		PutByte(0);
	if (Compressed)
	{
//		if (Depth > 8)
			PutByte(10);
/*
		else if (ColorType == G42ImageInfo::ColorTypeGrayscale)
			PutByte(11);
		else
			PutByte(9);
*/
	}
	else
	{
//		if (Depth > 8)
			PutByte(2);
/*		else if (ColorType == G42ImageInfo::ColorTypeGrayscale)
			PutByte(1);
		else
			PutByte(3);
*/
	}
	PutUInt16(0);
/*	if (ColorType == G42ImageInfo::ColorTypePalette)
	{
		PutUInt16((uint16)(info->NumPalette));
		PutByte(24);
	}
	else
*/
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
	
/*	if (ColorType == G42ImageInfo::ColorTypePalette)
	{
		const G42Color * pal = info->ImagePalette;
		for (int i = 0; i < info->NumPalette; i++)
		{
			PutByte(pal[i].red);
			PutByte(pal[i].green);
			PutByte(pal[i].blue);
		}
	}
*/
	return Valid;
}
bool
G42ImageWriterTarga::WriteRow(const byte * row, const byte *)
{
	if (TrueColorDepth == 24)
	{
		if (Descriptor->GetDepth() == 24)
		{
			G42ConstColor24Iterator sp(row);
			byte * dp = TempRow;
			uint32 i;
			for (i = 0; i < Width; i++)
			{
				G42ConstColor24 c(*sp);
				*dp++ = c.GetBlue();
				*dp++ = c.GetGreen();
				*dp++ = c.GetRed();
				sp++;
			}
			#if 0
			if (RowBytes > Width)
				*dp = *(dp - 1); // fill in with duplicate byte
			#endif
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
				*dp++ = c.GetBlue();
				*dp++ = c.GetGreen();
				*dp++ = c.GetRed();
				sp++;
			}
			#if 0
			if (RowBytes > Width)
				*dp = *(dp - 1); // fill in with duplicate byte
			#endif
			CompressRow(TempRow);
		}
	}
	else // if (TrueColorDepth == 16)
	{
		if (Descriptor->GetDepth() == 24)
		{
			G42ConstColor24Iterator sp(row);
			byte * dp = TempRow;
			uint32 i;
			uint16 v;
			for (i = 0; i < Width; i++)
			{
				G42ConstColor24 c(*sp);
				v = (uint16)(c.GetBlue() >> 3);
				v |= (uint16)((c.GetGreen() & 0xf8) << 2);
				v |= (uint16)((c.GetRed() & 0xf8) << 7);
				v |= (uint16)0x8000L;
				*dp++ = (byte) v;
				*dp++ = (byte) (v >> 8);
				sp++;
			}
			#if 0
			if (RowBytes > Width)
				*dp = *(dp - 1); // fill in with duplicate byte
			#endif
			CompressRow(TempRow);
		}
		else
		{
			G42ConstColor32Iterator sp(row);
			byte * dp = TempRow;
			uint32 i;
			uint16 v;
			for (i = 0; i < Width; i++)
			{
				G42ConstColor32 c(*sp);
				v = (uint16)(c.GetBlue() >> 3);
				v |= (uint16)((c.GetGreen() & 0xf8) << 2);
				v |= (uint16)((c.GetRed() & 0xf8) << 7);
				v |= (uint16)0x8000L;
				*dp++ = (byte) v;
				*dp++ = (byte) (v >> 8);
				sp++;
			}
			#if 0
			if (RowBytes > Width)
				*dp = *(dp - 1); // fill in with duplicate byte
			#endif
			CompressRow(TempRow);
		}
	}
/*
	else // (Depth == 8)
	{
		memcpy(TempRow, row, Width);
		#if 0
		if (RowBytes > Width)
			TempRow[RowBytes - 1] = TempRow[Width - 1];
		#endif
		CompressRow(TempRow);
	}
*/
	CurrentRow++; // increment the base's current row
	return Valid;
}
void
G42ImageWriterTarga::CompressRow(const byte * row)
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
		PutBytes(row, RowBytes);
	}
}

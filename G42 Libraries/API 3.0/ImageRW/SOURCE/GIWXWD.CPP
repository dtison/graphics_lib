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
#include "giwxwd.h"
G42ImageWriterXwd::G42ImageWriterXwd(void) :
	G42ImageWriterBase(), TempRow(0)
{
	SetInfo(G42XwdTypeId, "xwd", "Xwd");
}
G42ImageWriterXwd::G42ImageWriterXwd(G42Notifier & notifier,
	G42BaseOptions * options)
:
	G42ImageWriterBase(notifier, options), TempRow(0)
{
	SetInfo(G42XwdTypeId, "xwd", "Xwd");
}
G42ImageWriterXwd::~G42ImageWriterXwd()
{
	if (TempRow)
	{
		delete [] TempRow;
		TempRow = 0;
	}
	Valid = false;
}
bool
G42ImageWriterXwd::WriteInfo(G42ImageDescriptor * info)
{
	G42ImageWriterBase::WriteInfo(info); // set up base stuff
	if (Depth > 8)
	{
		Depth = 32;
		RowBytes = Width * 4;
		TempRow = new byte [RowBytes];
		if (!TempRow)
		{
			Valid = false;
			return false;
		}
	}
	IntegerFormat = Motorola;
	PutUInt32(100 /* + strlen(name) + 1 */);
	PutUInt32(7);
	PutUInt32(2);
	PutUInt32(Depth);
	PutUInt32(Width);
	PutUInt32(Height);
	PutUInt32(0);
	PutUInt32(1);
	PutUInt32(max((uint32)8, Depth));
	PutUInt32(Depth);
	PutUInt32(max((uint32)8, Depth));
	PutUInt32(Depth);
	PutUInt32(RowBytes);
	if (info->IsPaletted())
		PutUInt32(3);
	else if (info->IsColor())
		PutUInt32(4);
	else
		PutUInt32(0);
	PutUInt32(0xff0000L);
	PutUInt32(0xff00L);
	PutUInt32(0xff);
	PutUInt32(8);
	if (Depth > 8)
	{
		PutUInt32(0);
		PutUInt32(0);
	}
	else
	{
		PutUInt32(1 << (int)Depth);
		PutUInt32(1 << (int)Depth);
	}
	PutUInt32(Width);
	PutUInt32(Height);
	PutUInt32(0);
	PutUInt32(0);
	PutUInt32(0);
//	fwrite(name, 1, strlen(name) + 1, fp);
	if (info->IsPaletted())
	{
		const G42Color * pal = info->GetPalette();
		for (int i = 0; i < (1 << (int)Depth); ++i)
		{
			if (i < info->GetNumPalette())
			{
				PutUInt32(i);
				// all these casts are ugly, but I'm assuring this works correctly,
				// and removing any warnings
				PutUInt16(((uint16)pal[i].Red) |
					(uint16)((uint16)pal[i].Red << 8));
				PutUInt16(((uint16)pal[i].Green) |
					(uint16)((uint16)pal[i].Green << 8));
				PutUInt16(((uint16)pal[i].Blue) |
					(uint16)((uint16)pal[i].Blue << 8));
				PutUInt16(0);
			}
			else
			{
				PutUInt32(i);
				PutUInt16(0);
				PutUInt16(0);
				PutUInt16(0);
				PutUInt16(0);
			}
		}
	}
	else if (!info->IsColor())
	{
		for (int i = 0; i < (1 << (int)Depth); i++)
		{
			uint16 pal;
			pal = (uint16)(((uint32)i * (uint32)255) /
				(uint32)((1 << (int)Depth) - 1));
			pal |= (uint16)(pal << 8);
			PutUInt32(i);
			PutUInt16(pal);
			PutUInt16(pal);
			PutUInt16(pal);
			PutUInt16(0);
		}
	}
	return Valid;
}
bool
G42ImageWriterXwd::WriteRow(const byte * row, const byte *)
{
	if (Depth > 8)
	{
		if (Descriptor->GetDepth() == 24)
		{
			G42ConstColor24Iterator sp(row);
			byte * dp = TempRow;
			for (uint32 i = 0; i < Width; i++)
			{
				G42ConstColor24 c(*sp);
				*dp++ = c.GetRed();
				*dp++ = c.GetGreen();
				*dp++ = c.GetBlue();
				*dp++ = 0xff;
				sp++;
			}
		}
		else
		{
			G42ConstColor32Iterator sp(row);
			byte * dp = TempRow;
			for (uint32 i = 0; i < Width; i++)
			{
				G42ConstColor32 c(*sp);
				*dp++ = c.GetRed();
				*dp++ = c.GetGreen();
				*dp++ = c.GetBlue();
				*dp++ = 0xff;
				sp++;
			}
		}
		G42ImageWriterBase::WriteRow(TempRow, 0);
	}
	else
	{
		G42ImageWriterBase::WriteRow(row, 0);
	}
	return Valid;
}

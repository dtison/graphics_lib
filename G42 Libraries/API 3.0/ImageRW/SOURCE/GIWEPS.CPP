// g42wps.cpp - Postscript File
/*************************************************************
	File:          g42wps.h
   Copyright (c) 1996, Group 42, Inc.
	Description:   Derived class of g42ribase that handles bmp files
   Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
   Modification History:
      Code   Date   Name and Description
***************************************************************/
#include "giweps.h"
G42ImageWriterEps::G42ImageWriterEps(G42Notifier & notifier,
	G42BaseOptions * options)
:
	G42ImageWriterBase(notifier, options), TempRow(0), CurrentEpsRow(0),
	WriteMode(EpsMode), TempBuf(0), Landscape(false), Info(0)
{
	SetInfo(G42EpsTypeId, "eps", "Eps", true);
}
G42ImageWriterEps::G42ImageWriterEps(void)
:
	G42ImageWriterBase(), TempRow(0), CurrentEpsRow(0),
	WriteMode(EpsMode), TempBuf(0), Landscape(false), Info(0)
{
	SetInfo(G42EpsTypeId, "eps", "Eps", true);
}
G42ImageWriterEps::~G42ImageWriterEps()
{
	if (TempRow)
	{
		delete [] TempRow;
		TempRow = 0;
	}
	if (TempBuf)
	{
		delete [] TempBuf;
		TempBuf = 0;
	}
	if (Info)
	{
		delete Info;
		Info = 0;
	}
	Valid = false;
}
// check types and fix depth.  Be sure to convert from color to grayscale
bool
G42ImageWriterEps::WriteInfo(G42ImageDescriptor * info)
{
	G42ImageWriterBase::WriteInfo(info); // set up base stuff
	Info = new G42ImageDescriptor(*info);
	if (!Options || !dynamic_cast<G42ImageEpsOptions *>(Options))
	{
		G42ImageEpsOptions * options =
			new G42ImageEpsOptions();
		G42ThrowIfNull(options);
		AskForOptions(options);
	}
	G42ImageEpsOptions * options;
	if (Options)
	{
		options = dynamic_cast<G42ImageEpsOptions *>(Options);
	}
	if (options)
	{
		IsEps = options->IsEps();
		Level = options->GetLevel();
	}
	else
	{
		IsEps = true;
		Level = 2;
	}
#if 0
	if (IsEps)
	{
		PutByte(0xc5);
		PutByte(0xd0);
		PutByte(0xd3);
		PutByte(0xc6);
		PutUInt32(32, Motorola);
		PutUInt32(pslen, Motorola);
		PutUInt32(0, Motorola);
		PutUInt32(0, Motorola);
		PutUInt32(pslen + 32, Motorola);
		PutUInt32(tifflen, Motorola);
		PutUInt32(0, Motorola);
	}
#endif
	if (!info->IsColor())
	{
		Depth = 8;
		Level = 1; // no need for level 2 on grayscale images
	}
	else if (Depth <= 8) // really should convert palette to gray seperately
	{
		Depth = 24; // if level one, we'll reduce to gray ourselves
	}
	RowBytes = (Width * (Depth >> 3));
	Landscape = false;
	if (!IsEps && Width > Height)
		Landscape = true;
	PageX = 72;
	PageY = 72;
	if (Landscape)
	{
		PageWidth = 648;
		PageHeight = 468;
	}
	else
	{
		PageWidth = 468;
		PageHeight = 648;
	}
	if (IsEps)
	{
		PageX = 0;
		PageY = 0;
		PageWidth = Width;
		PageHeight = Height;
	}
	else if (Width * PageHeight < Height * PageWidth)
	{
		uint32 new_page_width = (Width * PageHeight) / Height;
		PageX += ((PageWidth - new_page_width) >> 1);
		PageWidth = new_page_width;
	}
	else
	{
		uint32 new_page_height = (Height * PageWidth) / Width;
		PageY += ((PageHeight - new_page_height) >> 1);
		PageHeight = new_page_height;
	}
	TempRow = new byte [(size_t)RowBytes];
	G42ThrowIfNull(TempRow);
	TempBuf = new byte [80];
	G42ThrowIfNull(TempBuf);
	/*  (tempBuf is a char * pointer to satisfy the Metrowerks compiler D.I.)  */
	char *tempBuf = (char *) TempBuf;
	if (IsEps)
		sprintf(tempBuf, "%%!PS-Adobe-3.0 EPSF-3.0\n");
	else
		sprintf(tempBuf, "%%!PS-Adobe-3.0\n");
	PutBytes(TempBuf, strlen(tempBuf));
	sprintf(tempBuf, "%%%%Title: None\n");
	PutBytes(TempBuf, strlen(tempBuf));
	sprintf(tempBuf, "%%%%Pages: 1\n");
	PutBytes(TempBuf, strlen(tempBuf));
	if (Landscape)
		sprintf(tempBuf, "%%%%Orientation: LANDSCAPE\n");
	else
		sprintf(tempBuf, "%%%%Orientation: PORTRAIT\n");
	PutBytes(TempBuf, strlen(tempBuf));
	sprintf(tempBuf, "%%%%BoundingBox: %d %d %d %d\n",
		PageX, PageY, PageX + PageWidth, PageY + PageHeight);
	PutBytes(TempBuf, strlen(tempBuf));
	sprintf(tempBuf, "%%%%EndComments\n");
	PutBytes(TempBuf, strlen(tempBuf));
	sprintf(tempBuf, "%%%%BeginSetup\n");
	PutBytes(TempBuf, strlen(tempBuf));
	if (!IsEps)
	{
		sprintf(tempBuf, "/#copies 1 def\n\n");
		PutBytes(TempBuf, strlen(tempBuf));
	}
	sprintf(tempBuf, "%%%%EndSetup\n");
	PutBytes(TempBuf, strlen(tempBuf));
	if (Landscape)
	{
		sprintf(tempBuf, "612 0 translate\n90 rotate\n");
		PutBytes(TempBuf, strlen(tempBuf));
	}
	if (Level == 1)
		sprintf(tempBuf, "/picstr %d string def\n", Width);
	else
		sprintf(tempBuf, "/picstr %d string def\n", Width * 3);
	PutBytes(TempBuf, strlen(tempBuf));
	if (PageX || PageY)
	{
		sprintf(tempBuf, "%d %d translate\n", PageX, PageY);
		PutBytes(TempBuf, strlen(tempBuf));
	}
	sprintf(tempBuf, "%d %d scale\n", PageWidth, PageHeight);
	PutBytes(TempBuf, strlen(tempBuf));
	sprintf(tempBuf, "%d %d 8\n", Width, Height);
	PutBytes(TempBuf, strlen(tempBuf));
	sprintf(tempBuf, "[%d 0 0 -%d 0 %d]\n", Width, Height, Height);
	PutBytes(TempBuf, strlen(tempBuf));
	if (Level == 1)
		sprintf(tempBuf,
			"{currentfile picstr readhexstring pop}\nimage\n");
	else
		sprintf(tempBuf,
			"{currentfile picstr readhexstring pop}\nfalse 3\ncolorimage\n");
	PutBytes(TempBuf, strlen(tempBuf));
	return Valid;
}
bool
G42ImageWriterEps::WriteRow(const byte * row, const byte *)
{
	if (Level == 1 && Depth > 8)
	{
		if (Descriptor->GetDepth() == 24)
		{
			byte * dp = TempRow;
			G42ConstColor24Iterator sp(row);
			for (uintf i = 0; i < Width; i++)
			{
				G42ConstColor24 c(*sp);
				*dp++ = (byte)(((int32)c.GetRed() * (int32)306 +
					(int32)c.GetGreen() * (int32)601 +
					(int32)c.GetBlue() * (int32)117 + (int32)512) >> 10);
				sp++;
			}
		}
		else
		{
			byte * dp = TempRow;
			G42ConstColor32Iterator sp(row);
			for (uintf i = 0; i < Width; i++)
			{
				G42ConstColor32 c(*sp);
				*dp++ = (byte)(((int32)c.GetRed() * (int32)306 +
					(int32)c.GetGreen() * (int32)601 +
					(int32)c.GetBlue() * (int32)117 + (int32)512) >> 10);
				sp++;
			}
		}
	}
	else if (Depth > 8)
	{
		memcpy(TempRow, row, RowBytes);
	}
	if (Level == 1)
	{
		byte * rp = TempRow;
		byte * bp = TempBuf;
		intf count = 0;
		for (uint32 i = 0; i < Width; i++)
		{
			sprintf((char *) bp, "%0.2x", *rp++);
			bp += 2;
			count++;
			if (count == 39)
			{
				*bp = '\n';
				count = 0;
				bp = TempBuf;
				PutBytes(TempBuf, 79);
			}
		}
		if (count)
		{
			*bp = '\n';
			PutBytes(TempBuf, (count << 1) + 1);
		}
	}
	else
	{
		byte * bp = TempBuf;
		intf count = 0;
		if (Descriptor->GetDepth() == 24)
		{
			G42ConstColor24Iterator rp(TempRow);
			for (uint32 i = 0; i < Width; i++)
			{
				G42ConstColor24 c(*rp);
				sprintf((char *) bp, "%0.2x%0.2x%0.2x", c.GetRed(), c.GetGreen(),
					c.GetBlue());
				rp++;
				bp += 6;
				count++;
				if (count == 13)
				{
					*bp = '\n';
					count = 0;
					bp = TempBuf;
					PutBytes(TempBuf, 79);
				}
			}
		}
		else
		{
			G42ConstColor32Iterator rp(TempRow);
			for (uint32 i = 0; i < Width; i++)
			{
				G42ConstColor32 c(*rp);
				sprintf((char *) bp, "%0.2x%0.2x%0.2x", c.GetRed(), c.GetGreen(),
					c.GetBlue());
				rp++;
				bp += 6;
				count++;
				if (count == 13)
				{
					*bp = '\n';
					count = 0;
					bp = TempBuf;
					PutBytes(TempBuf, 79);
				}
			}
		}
		if (count)
		{
			*bp = '\n';
			PutBytes(TempBuf, (count * 6) + 1);
		}
	}
	CurrentEpsRow++;
	if (CurrentEpsRow >= Height)
	{
		if (!IsEps)
		{
			sprintf((char *) TempBuf, "showpage\n");
			PutBytes(TempBuf, strlen((const char *) TempBuf));
		}
	}
	return Valid;
}
uint32
G42ImageWriterEps::NextRow(void)
{
	return (CurrentEpsRow);
}
bool
G42ImageWriterEps::NeedRow(void)
{
	return (CurrentEpsRow < Height);
}

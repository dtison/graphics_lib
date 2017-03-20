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
#include "g42itype.h"
#include "g42wps.h"
#include "g42idata.h"
#include "g42iter.h"
G42PsWriter::G42PsWriter(G42RasterImageWriter & parent) :
	G42RasterWriterBase(parent), TempRow(0), CurrentPsRow(0),
	WriteMode(PsMode), TempBuf(0), Landscape(false)
{
}
G42PsWriter::~G42PsWriter()
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
	Valid = false;
}
// check types and fix depth.  Be sure to convert from color to grayscale
bool
G42PsWriter::WriteInfo(G42ImageInfo * info)
{
	G42RasterWriterBase::WriteInfo(info); // set up base stuff
	Info = new G42ImageInfo(*info);
	if (Info->ImageType == EpsL1 || Info->ImageType == EpsL2)
		IsEps = true;
	else
		IsEps = false;
	if (Info->ImageType == EpsL1 || Info->ImageType == PsL2)
		Level = 1;
	else
		Level = 2;
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
	if (Info->ImageType == G42ImageInfo::ColorTypeGrayscale)
	{
		Depth = 8;
		Level = 1; // no need for level 2 on grayscale images
	}
	else
	{
		Depth = 24; // if level one, we'll reduce to gray ourselves
		if (Level == 1)
		{
			Info->ColorType = G42ImageInfo::ColorTypeGrayscale;
			Info->PixelDepth = 8;
			Info->NumPalette = 0;
		}
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
	if (!TempRow)
	{
		Valid = false;
		return false;
	}
	TempBuf = new byte [80];
	/*  (tempBuf is a char * pointer to satisfy the Metrowerks compiler D.I.)  */
	char *tempBuf = (char *) TempBuf;
	
	if (!TempBuf)
	{
		Valid = false;
		return false;
	}
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
G42PsWriter::WriteRow(const byte * row, const byte *)
{
	if (Level == 1 && Depth == 24)
	{
		byte * dp = TempRow;
		G42ConstByte24BitIterator sp(row);
		for (uint i = 0; i < Width; i++)
		{
			*dp++ = (byte)((sp.GetRed32() * (int32)306 +
				sp.GetGreen32() * (int32)601 +
				sp.GetBlue32() * (int32)117 + (int32)512) >> 10);
			sp.Increment();
		}
	}
	else if (Depth == 24)
	{
		memcpy(TempRow, row, RowBytes);
	}
	if (Level == 1)
	{
		byte * rp = TempRow;
		byte * bp = TempBuf;
		int count = 0;
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
		G42Byte24BitIterator rp(TempRow);
		byte * bp = TempBuf;
		int count = 0;
		for (uint32 i = 0; i < Width; i++)
		{
			sprintf((char *) bp, "%0.2x%0.2x%0.2x", rp.GetRed(), rp.GetGreen(), rp.GetBlue());
			rp.Increment();
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
		if (count)
		{
			*bp = '\n';
			PutBytes(TempBuf, (count * 6) + 1);
		}
	}
	CurrentPsRow++;
	if (CurrentPsRow >= Height)
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
G42PsWriter::NextRow(void)
{
	return (CurrentPsRow);
}
bool
G42PsWriter::NeedRow(void)
{
	return (CurrentPsRow < Height);
}

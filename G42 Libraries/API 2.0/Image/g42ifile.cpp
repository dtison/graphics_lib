// g42ifile.cpp - image file class
/*************************************************************
	File:          g42ifile.cpp
	Copyright (c) 1996, Group 42, Inc.
	Description:   To read an image file and present the information
						contained within in a common format.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "g42itype.h" // include standard types and configuration stuff
#include "g42ifile.h" // include header file
// This file is merely a "redirection" class.  That is, it redirects
// the information from the raster file classes up to the image class
// so it can deal with it.
void
G42ImageFile::HasRow(byte *, byte *)
{
}
void
G42ImageFile::HasRow(byte * row, uint32 row_num, uint32 offset, uint length,
	byte * mask)
{
	if (Parent)
	{
		Parent->SetRow(row, row_num, offset, length);
		if (mask)
			Parent->SetMaskRow(mask, row_num, offset, length);
		Parent->MarkRowValid(row_num);
	}
}
void
G42ImageFile::HasInfo(void)
{
}
byte *
G42ImageFile::GetRow(uint32 row)
{
	if (Parent)
		return Parent->GetRow(row, 0, Parent->GetInfo().Width);
	return 0;
}
void
G42ImageFile::HasInfo(G42ImageInfo & info)
{
	if (Parent)
		Parent->SetInfo(info);
}
void
G42ImageFile::HasEnd(G42ImageInfo & info)
{
	if (Parent)
   	Parent->EndImage(info);
}
G42ImageFile::~G42ImageFile()
{
}

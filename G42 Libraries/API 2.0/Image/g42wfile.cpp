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
#include "g42wfile.h" // include header file
#ifdef MACOS
#include "g42wpict.h"
#endif
G42ImageWriter::G42ImageWriter(G42Image * image,
	const char * file_path)
:
	Image(image), FilePath(0),
	FP(0), Finished(false), Info(0)
{
	if (file_path)
	{
		FilePath = new char [strlen(file_path) + 1];
		if (!FilePath)
		{
			Valid = false;
			return;
		}
		strcpy(FilePath, file_path);
	}
}
G42ImageWriter::~G42ImageWriter()
{
	if (FilePath)
	{
		delete [] FilePath;
		FilePath = 0;
	}
	if (FP)
	{
		fclose(FP);
		FP = 0;
	}
	if (Info)
	{
		delete Info;
      Info = 0;
	}
	Valid = false;
}
bool
G42ImageWriter::WriteImage(void)
{
	if (!Valid || !Image || !Image->HasInfo())
	{
		Valid = false;
		return false;
	}
	if (FilePath)
	{
		FP = fopen(FilePath, "wb");
		if (!FP)
		{
			Valid = false;
         return false;
		}
	}
	Info = new G42ImageInfo(Image->GetInfo());
	#ifdef MACOS
	if (Info->ImageType == Pict)
	{
		G42PictWriter pict_writer(Image, this);
		if (!pict_writer.Write())
		{
			Valid = false;
			return false;
		}
		
		return (Valid);
	}
	#endif
	if (!WriteInfo(Info) || !Valid)
	{
		Valid = false;
		return false;
	}
	
	if (Info->IsTransparent)
	{
		while (NeedRow())
		{
			uint32 row = NextRow();
			if (!WriteRow(Image->GetRow(row, 0, Info->Width, GetDepth()),
				Image->GetMaskRow(row, 0, Info->Width)) ||
				!Valid)
			{
				Valid = false;
				return false;
			}
		}
	}
	else
	{
		while (NeedRow())
		{
			if (!WriteRow(Image->GetRow(NextRow(), 0, Info->Width, GetDepth()),
				0) || !Valid)
			{
				Valid = false;
				return false;
			}
		}
	}
	if (!WriteEnd(Info))
		Valid = false;
	return (Valid);
}
bool
G42ImageWriter::Output(const byte * buffer, uint size)
{
	if (!buffer || !size)
		return true;
	if (!FP || !Valid)
	{
		Valid = false;
		return false;
	}
	uint wrote_size = fwrite(buffer, 1, size, FP);
	if (size != wrote_size)
		Valid = false;
	return (Valid);
}
void
G42ImageWriter::OutputFinished(void)
{
	Finished = true;
}

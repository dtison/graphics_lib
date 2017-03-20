// g42wpict.cpp - Pict File
/*************************************************************
	File:          g42wpict.h
   Copyright (c) 1996, Group 42, Inc.
	Description:   Derived class of g42ribase that handles bmp files
   Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
   Modification History:
      Code   Date   Name and Description
***************************************************************/
#include "g42itype.h"
#ifdef MACOS
#include "g42wpict.h"
#include "g42macfl.h"
G42PictWriter::G42PictWriter(G42Image * image, G42ImageWriter * writer) : 
	Image(image), Writer(writer), Valid(true)
{}
G42PictWriter::~G42PictWriter()
{
	Valid = false;
}
bool
G42PictWriter::Write(void)
{
	/*  Convert the Image into a PICT  */
	G42SimpleImageViewer viewer (nil);
	viewer.SetWindowSize (Image -> GetInfo().Width, Image -> GetInfo().Height);
	PicHandle pict = G42MacFile::PICTFromG42Image (Image, &viewer);
	if (pict)
	{
		/*  Save out the 512 byte header  */
		short headerSize = 512;
		long size = (long) headerSize;
		byte *buffer = new byte [headerSize];
		memset (buffer, 0, headerSize);
		Writer -> Output (buffer, headerSize);
		delete [] buffer;
		/*  Save the rest of the PICT out  */
		size = ::GetHandleSize ((Handle) pict);
		Writer -> Output ((byte *) *pict, size);
		Writer -> OutputFinished();
		::KillPicture (pict);
	}
	else
		Valid = false;
	return Valid;
}
#endif

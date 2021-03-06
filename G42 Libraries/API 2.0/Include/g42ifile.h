// g42ifile.h - image file class header
#ifndef G42IFILE_H
#define G42IFILE_H
/*************************************************************
	File:          g42ifile.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   To read an image file and present the information
						contained within in a common format.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "g42rimag.h" // raster file reader
#include "g42image.h" // class that deals with the information
// This class is just a "redirection" class.  It takes the info
// from the G42RasterImage class where it is generated, and gives
// it to the Group42ImageP class where it is converted into a
// bitmap for display
class G42ImageFile : public G42RasterImage
{
	protected:
		G42Image * Parent;
	public:
		G42ImageFile(G42Image * parent) : Parent(parent) {}
		virtual ~G42ImageFile();
		virtual void HasRow(byte * row, byte * mask = 0);
		virtual void HasInfo(void);
		virtual void HasInfo(G42ImageInfo & info);
		virtual void HasEnd(G42ImageInfo & info);
		virtual void HasRow(byte * row, uint32 row_num, uint32 offset,
			uint length, byte * mask = 0);
		virtual byte * GetRow(uint32 row);
};
#endif // G42IFILE_H

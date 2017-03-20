// g42bmp.h - bmp version of the raster file base class
#ifndef G42WPICT_H
#define G42WPICT_H
/*************************************************************
	File:          g42bmp.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for bmp's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
      Code   Date   Name and Description
***************************************************************/
#include "g42wfile.h"
class G42PictWriter
{
	public:
		G42PictWriter(G42Image * image, G42ImageWriter * writer);
		virtual ~G42PictWriter();
		bool Write(void);
		bool IsValid(void)
			{ return Valid; }
	private:
		G42Image * Image;
		G42ImageWriter * Writer;
		bool Valid;
};
#endif // G42WPICT_H

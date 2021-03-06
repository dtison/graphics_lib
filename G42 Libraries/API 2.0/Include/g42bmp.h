// g42bmp.h - bmp version of the raster file base class
#ifndef G42BMP_H
#define G42BMP_H
/*************************************************************
	File:          g42bmp.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for bmp's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
      Code   Date   Name and Description
***************************************************************/
#include "g42ribas.h"
class G42BmpImage : G42RasterBase
{
	public:
		void ProcessData(byte * buffer, size_t buffer_size);
		G42BmpImage(G42RasterImage & parent);
		virtual ~G42BmpImage();
	protected:
		enum G42BmpVersion {Win3x, OS21x, OS22x, Undefined};
		G42BmpVersion osType;
		int CurrentPalette;
		enum G42ReadMode {ReadHeaderMode, ReadPaletteMode, ReadImageMode,
			ReadDoneMode, ReadErrorMode};
		G42ReadMode ReadMode;
		void ReadHeader(void);
		void ReadPalette(void);
		void ReadImage(void);
		virtual void OutputRow(void);
		uint32 CurrentRow;
};
extern bool G42IsBmp(byte * buffer, size_t buffer_size);
#endif // G42BMP_H

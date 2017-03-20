// g42bmp.h - bmp version of the raster file base class
#ifndef G42WGIF_H
#define G42WGIF_H
/*************************************************************
	File:          g42gif.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for gif's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
      Code   Date   Name and Description
***************************************************************/
#include "g42wibas.h"
#include "g42wlzw.h"
class G42GifLzwOutput : public G42WriteCompressionOutput
{
	public:
		G42GifLzwOutput(G42RasterWriterBase & base, int code_depth);
		virtual ~G42GifLzwOutput();
		virtual void OutputBuffer(byte * buffer, uint length);
	private:
		G42RasterWriterBase & Base;
};
class G42GifWriter : G42RasterWriterBase
{
	public:
		G42GifWriter(G42RasterImageWriter & parent);
		virtual ~G42GifWriter();
		bool WriteInfo(G42ImageInfo * info);
		bool WriteRow(const byte * row, const byte * mask);
		bool WriteEnd(G42ImageInfo * info);
      void AdvanceCurrentRow(void);
	private:
		G42GifLzwOutput * LzwOutput;
		G42LzwOutput * Lzw;
		int CurrentPass;
		bool Progressive;
		bool Transparent;
		byte TransparentPixel;
		byte OpaquePixel;
      byte * TempRow;
};
#endif // G42BMP_H

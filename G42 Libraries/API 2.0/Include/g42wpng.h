// g42png.h - png version of the raster file base class
#ifndef G42WPNG_H
#define G42WPNG_H
/*************************************************************
	File:          g42png.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for Png's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "g42wibas.h"
extern "C" {
#include <png.h>
}
extern "C" void g42_png_write_error(png_structp png_ptr,
	png_const_charp message);
extern "C" void g42_png_write_warning(png_structp png_ptr,
	png_const_charp message);
extern "C" void g42_png_write_data(png_structp png_ptr, png_bytep data,
	png_uint_32 length);
extern "C" void g42_png_flush_data(png_structp png_ptr);
class G42PngWriter : G42RasterWriterBase
{
	public:
		G42PngWriter(G42RasterImageWriter & parent);
		virtual ~G42PngWriter();
		bool WriteInfo(G42ImageInfo * info);
		bool WriteRow(const byte * row, const byte * mask);
		bool WriteEnd(G42ImageInfo * info);
		void PngWriteData(byte * data, uint32 length);
		void PngWriteError(const char * message);
		void PngWriteWarning(const char * message);
#if defined(G42_USE_EXCEPTIONS)
		class G42PngError
		{
			int unused;
		};
#endif
	private:
      void Clear();
		png_info * PngInfo;
		png_struct * PngStruct;
		png_color * PngPalette;
		int NumPasses;
		int CurrentPass;
};
#endif // G42PNG_H

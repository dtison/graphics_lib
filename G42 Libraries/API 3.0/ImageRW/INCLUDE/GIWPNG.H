// giwpng.h - png version of the raster file base class
#ifndef GIWPNG_H
#define GIWPNG_H
/*************************************************************
	File:          giwpng.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for Png's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "giwbase.h"
#include "gitpng.h"
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
class G42ImageWriterPng : public G42ImageWriterBase
{
	public:
		G42ImageWriterPng(void);
		G42ImageWriterPng(G42Notifier & notifier,
			G42BaseOptions * options = 0);
		virtual ~G42ImageWriterPng();
		bool WriteInfo(G42ImageDescriptor * info);
		bool WriteRow(const byte * row, const byte * mask);
		bool WriteEnd(void);
		void PngWriteData(byte * data, uint32 length);
		void PngWriteError(const char * message);
		void PngWriteWarning(const char * message);
#if defined(G42_USE_EXCEPTIONS)
		class G42PngError
		{
			intf unused;
		};
#endif
	private:
      void Clear();
		png_info * PngInfo;
		png_struct * PngStruct;
		png_color * PngPalette;
		bool NeedDestroy;
		bool Progressive;
		intf NumPasses;
		intf CurrentPass;
};
#endif // GIWPNG_H

// g42png.h - png version of the raster file base class
#ifndef G42PNG_H
#define G42PNG_H
/*************************************************************
	File:          g42png.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for Png's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
      Code   Date   Name and Description
***************************************************************/
#include "g42ribas.h"
extern "C" {
#include <png.h>
}
extern "C" void g42_png_error(png_structp png_ptr, png_const_charp message);
extern "C" void g42_png_warning(png_structp png_ptr, png_const_charp message);
extern "C" void g42_png_info_cb(png_structp png_ptr, png_infop info);
extern "C" void g42_png_end_cb(png_structp png_ptr, png_infop info);
extern "C" void g42_png_row_cb(png_structp png_ptr, png_bytep row,
	png_uint_32 row_num, int pass);
class G42PngImage : G42RasterBase
{
	public:
		void ProcessData(byte * buffer, size_t buffer_size);
		G42PngImage(G42RasterImage & parent);
		virtual ~G42PngImage();
		void PngError(const char * message);
		void PngWarning(const char * message);
		void PngInfo(png_infop info);
		void PngEnd(png_infop info);
		void PngRow(png_bytep row, png_uint_32 row_num, int pass);
		virtual void CombineRow(byte * old_row, byte * new_row,
			int32 rowbytes);
#if defined(G42_USE_EXCEPTIONS)
		class G42PngError
		{
			int unused;
		};
#endif
	protected:
		bool Progressive;
		int CurrentRow;
		int NumPasses;
      int CurrentPass;
		png_struct LibPng;
      png_info InfoPtr;
		virtual void OutputRow(void);
};
extern bool G42IsPng(byte * buffer, size_t buffer_size);
#endif // G42PNG_H

// g42png.h - png version of the raster file base class
#ifndef GIRPNG_H
#define GIRPNG_H
/*************************************************************
	File:          girpng.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for Png's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
      Code   Date   Name and Description
***************************************************************/
#include "girbase.h"
extern "C" {
#include <png.h>
}
#include "gitpng.h"
extern "C" void g42_png_error(png_structp png_ptr, png_const_charp message);
extern "C" void g42_png_warning(png_structp png_ptr, png_const_charp message);
extern "C" void g42_png_info_cb(png_structp png_ptr, png_infop info);
extern "C" void g42_png_end_cb(png_structp png_ptr, png_infop info);
extern "C" void g42_png_row_cb(png_structp png_ptr, png_bytep row,
	png_uint_32 row_num, intf pass);
class G42ImageReaderPng : public G42ImageReaderBase
{
	public:
		G42ImageReaderPng(void);
		G42ImageReaderPng(G42HeapManager & heap, G42Notifier & notifier,
			G42File * file = 0, G42BaseOptions * options = 0);
		virtual ~G42ImageReaderPng();
		virtual void ProcessStreamData(void);
		virtual void BeginProcessing(void);
		virtual void SendExtendedInfo(void);
		virtual bool TestType(void);
		virtual bool TestType(const byte * buffer, uint32 buffer_size);
		void PngError(const char * message);
		void PngWarning(const char * message);
		void PngInfo(png_infop info);
		void PngEnd(png_infop info);
		void PngRow(png_bytep row, png_uint_32 row_num, intf pass);
//		virtual void CombineRow(byte * old_row, byte * new_row,
//			int32 rowbytes);
#if defined(G42_USE_EXCEPTIONS)
		class G42PngError
		{
			intf unused;
		};
#endif
	protected:
		png_struct LibPng;
		png_info InfoPtr;
		bool DestroyLibrary;
		bool Valid;
		byte * RowBuf;
		byte * MaskBuf;
		bool NeedConvert;
		bool NeedConvertMask;
		void PngConvertRow(byte * row, byte * mask, byte * png_row);
		void PngConvertRowBack(byte * row, byte * mask, byte * png_row);
};
#endif // GIRPNG_H

// g42png.cpp - Png File
/*************************************************************
	File:          g42png.cpp
	Copyright (c) 1996, Group 42, Inc.
	Description:   Derived class of g42ribase that handles Png files
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
   Modification History:
      Code   Date   Name and Description
***************************************************************/
#include "g42itype.h"
#include "g42png.h"
#include "g42idata.h"
#include "g42iter.h"
G42PngImage::G42PngImage(G42RasterImage & parent) :
	G42RasterBase(parent), CurrentRow(0), Progressive(false),
	CurrentPass(0)
{
#if !defined(G42_USE_EXCEPTIONS)
	if (setjmp(LibPng.jmpbuf))
	{
		return;
	}
#else
	try {
#endif
	png_set_message_fn(&LibPng, this, g42_png_error, g42_png_warning);
	png_info_init(&InfoPtr);
	png_read_init(&LibPng);
	png_set_progressive_read_fn(&LibPng, this,
		g42_png_info_cb, g42_png_row_cb, g42_png_end_cb);
	Valid = true;
#if defined(G42_USE_EXCEPTIONS)
	} catch (G42PngError)
	{
	}
#endif
}
G42PngImage::~G42PngImage()
{
#if !defined(G42_USE_EXCEPTIONS)
	if (setjmp(LibPng.jmpbuf))
	{
		return;
	}
#else
	try {
#endif
	png_read_destroy(&LibPng, &InfoPtr, (png_infop)0);
#if defined(G42_USE_EXCEPTIONS)
	} catch (G42PngError)
	{
	}
#endif
}
// count rows, and mark done when last row is output
void
G42PngImage::OutputRow(void)
{
}
void
G42PngImage::ProcessData(byte * buffer, size_t buffer_size)
{
	if (Valid)
	{
#if !defined(G42_USE_EXCEPTIONS)
		if (setjmp(LibPng.jmpbuf))
		{
			Valid = false;
			return;
		}
#else
		try {
#endif
		png_process_data(&LibPng, &InfoPtr, buffer, buffer_size);
#if defined(G42_USE_EXCEPTIONS)
		} catch (G42PngError)
		{
			Valid = false;
		}
#endif
	}
}
void
G42PngImage::PngInfo(png_infop)
{
	if (InfoPtr.interlace_type)
	{
		Progressive = true;
		NumPasses = png_set_interlace_handling(&LibPng);
	}
	else
	{
		Progressive = false;
		NumPasses = 1;
	}
	if (InfoPtr.bit_depth > 8)
		png_set_strip_16(&LibPng);
	if (InfoPtr.valid & PNG_INFO_gAMA)
	{
		png_set_gamma(&LibPng, 2.2, InfoPtr.gamma);
	}
#ifdef MSWIN
	if (InfoPtr.color_type == PNG_COLOR_TYPE_RGB ||
		InfoPtr.color_type == PNG_COLOR_TYPE_RGB_ALPHA)
		png_set_bgr(&LibPng);
#endif
#ifdef MACOS
	if (InfoPtr.color_type == PNG_COLOR_TYPE_RGB ||
		InfoPtr.color_type == PNG_COLOR_TYPE_RGB_ALPHA)
		png_set_filler(&LibPng, 0, PNG_FILLER_BEFORE);
#endif
#ifdef XWIN
	if (InfoPtr.color_type == PNG_COLOR_TYPE_RGB ||
		InfoPtr.color_type == PNG_COLOR_TYPE_RGB_ALPHA)
		png_set_filler(&LibPng, 0xff, PNG_FILLER_AFTER);
#endif
	// temporary until we can handle alpha again
	if ((InfoPtr.color_type & PNG_COLOR_MASK_ALPHA) ||
		InfoPtr.valid & PNG_INFO_tRNS)
	{
		if (InfoPtr.valid & PNG_INFO_bKGD)
		{
			png_set_background(&LibPng, &(InfoPtr.background),
				PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
		}
		else
		{
			png_color_16 my_background;
			my_background.red = 0xc0;
			my_background.green = 0xc0;
			my_background.blue = 0xc0;
			my_background.index = 0;
			png_set_background(&LibPng, &my_background,
				PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);
		}
	}
	png_read_update_info(&LibPng, &InfoPtr);
	Width = InfoPtr.width;
	Height = InfoPtr.height;
	TopLineFirst = true;
	if (InfoPtr.color_type == PNG_COLOR_TYPE_PALETTE)
	{
		NumPalette = InfoPtr.num_palette;
      Info.NumPalette = NumPalette;
		G42Color * pal = new G42Color[NumPalette]; // need to read in palette
		if (!pal)
		{
			Valid = false;
			return;
		}
		Info.ImagePalette.Set(pal);
		for (int i = 0; i < NumPalette; i++)
		{
			((G42Color *)(Info.ImagePalette))[i].red =
				InfoPtr.palette[i].red;
			((G42Color *)(Info.ImagePalette))[i].green =
				InfoPtr.palette[i].green;
			((G42Color *)(Info.ImagePalette))[i].blue =
				InfoPtr.palette[i].blue;
		}
	}
	else
	{
		NumPalette = 0;
		ImagePalette = 0;
	}
	Depth = InfoPtr.pixel_depth; // what about alpha?
	RowBytes = ((Width * Depth + 7) >> 3);
	if (InfoPtr.color_type & PNG_COLOR_MASK_COLOR)
		Color = true;
	else
		Color = false;
	Info.Width = Width;
	Info.Height = Height;
	if (Color && Depth > 8)
	{
		Info.PixelDepth = 8 * TrueColorPixelSize;
		Info.ColorType = G42ImageInfo::ColorTypeTrueColor;
	}
	else if (Color)
	{
		Info.PixelDepth = Depth;
		Info.ColorType = G42ImageInfo::ColorTypePalette;
	}
	else
	{
		Info.PixelDepth = Depth;
		Info.ColorType = G42ImageInfo::ColorTypeGrayscale;
	}
	Info.ImageType = Png;
	Info.Compressed = true;
	Info.Progressive = Progressive;
	Parent.HasInfo(Info);         // give user the file dimensions
}
void
G42PngImage::PngRow(png_bytep row, png_uint_32 row_num, int pass)
{
	if (row)
	{
		if (pass && pass < 6)
		{
			byte * old_row = Parent.GetRow(row_num);
			png_progressive_combine_row(&LibPng, old_row, row);
			Parent.HasRow(old_row, row_num, 0, Width);
		}
		else
		{
			Parent.HasRow(row, row_num, 0, Width);
		}
	}
	CurrentRow++;
	if (CurrentRow >= Height)
	{
		CurrentPass++;
		if (CurrentPass < NumPasses)
		{
			CurrentRow = 0;
		}
		else
		{
			ForceFileEnd();
		}
	}
}
void
G42PngImage::CombineRow(byte * old_row, byte * new_row, int32)
{
	png_progressive_combine_row(&LibPng, old_row, new_row);
//	memcpy(old_row, new_row, rowbytes);
}
void
G42PngImage::PngEnd(png_infop)
{
//	MessageBox(0, "Png Finished", "Trace", MB_OK);
	ForceFileEnd();
}
void
G42PngImage::PngWarning(const char * message)
{
//	MessageBox(0, message, "Warning", MB_OK);
}
void
G42PngImage::PngError(const char * message)
{
//	MessageBox(0, message, "Error", MB_OK);
	Valid = false;
#if !defined(G42_USE_EXCEPTIONS)
	longjmp(LibPng.jmpbuf, 1);
#else
	throw G42PngImage::G42PngError();
#endif
}
bool G42IsPng(byte * buffer, size_t buffer_size)
{
	return png_check_sig(buffer, buffer_size);
}
extern "C" void g42_png_error(png_structp png_ptr, png_const_charp message)
{
	G42PngImage * image = (G42PngImage *)png_get_msg_ptr(png_ptr);
	if (image)
	{
		image->PngError(message);
	}
}
extern "C" void g42_png_warning(png_structp png_ptr, png_const_charp message)
{
	G42PngImage * image = (G42PngImage *)png_get_msg_ptr(png_ptr);
	if (image)
	{
		image->PngWarning(message);
	}
}
extern "C" void g42_png_info_cb(png_structp png_ptr, png_infop info)
{
	G42PngImage * image = (G42PngImage *)png_get_msg_ptr(png_ptr);
	if (image)
	{
		image->PngInfo(info);
	}
}
extern "C" void g42_png_end_cb(png_structp png_ptr, png_infop info)
{
	G42PngImage * image = (G42PngImage *)png_get_msg_ptr(png_ptr);
	if (image)
	{
		image->PngEnd(info);
	}
}
extern "C" void g42_png_row_cb(png_structp png_ptr, png_bytep row,
	png_uint_32 row_num, int pass)
{
	G42PngImage * image = (G42PngImage *)png_get_msg_ptr(png_ptr);
	if (image)
	{
		image->PngRow(row, row_num, pass);
	}
}

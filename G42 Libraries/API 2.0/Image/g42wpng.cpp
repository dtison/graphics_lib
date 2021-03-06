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
#include "g42wpng.h"
#include "g42idata.h"
#include "g42iter.h"
extern "C" void g42_png_write_error(png_structp png_ptr,
	png_const_charp message)
{
	G42PngWriter * writer = (G42PngWriter *)png_get_io_ptr(png_ptr);
	writer->PngWriteError(message);
}
extern "C" void g42_png_write_warning(png_structp png_ptr,
	png_const_charp message)
{
	G42PngWriter * writer = (G42PngWriter *)png_get_io_ptr(png_ptr);
	writer->PngWriteWarning(message);
}
extern "C" void g42_png_write_data(png_structp png_ptr, png_bytep data,
	png_uint_32 length)
{
	G42PngWriter * writer = (G42PngWriter *)png_get_io_ptr(png_ptr);
	writer->PngWriteData(data, length);
}
extern "C" void g42_png_flush_data(png_structp)
{
}
G42PngWriter::G42PngWriter(G42RasterImageWriter & parent) :
	G42RasterWriterBase(parent), PngStruct(0), PngInfo(0), PngPalette(0),
	CurrentPass(0), NumPasses(1)
{
	PngStruct = new png_struct;
	if (!PngStruct)
	{
		Valid = false;
		return;
	}
	memset(PngStruct, 0, sizeof (png_struct)); // for png_destroy()
	PngInfo = new png_info;
	if (!PngInfo)
	{
		Valid = false;
		return;
	}
}
G42PngWriter::~G42PngWriter()
{
#if !defined(G42_USE_EXCEPTIONS)
	if (setjmp(PngStruct->jmpbuf))
	{
		Clear();
		return;
	}
#else
	try {
#endif
	png_write_destroy(PngStruct);
#if defined(G42_USE_EXCEPTIONS)
	} catch (G42PngError)
	{
	}
#endif
	Clear();
	Valid = false;
}
void
G42PngWriter::Clear(void)
{
	if (PngStruct)
	{
		delete PngStruct;
		PngStruct = 0;
	}
	if (PngInfo)
	{
		delete PngInfo;
		PngInfo = 0;
	}
	if (PngPalette)
	{
		delete [] PngPalette;
		PngPalette = 0;
	}
}
bool
G42PngWriter::WriteInfo(G42ImageInfo * info)
{
	G42RasterWriterBase::WriteInfo(info); // set up base stuff
#if !defined(G42_USE_EXCEPTIONS)
	if (setjmp(PngStruct->jmpbuf))
	{
		Valid = false;
		return false;
	}
#else
	try {
#endif
	png_info_init(PngInfo);
	png_set_message_fn(PngStruct, this,
		g42_png_write_error, g42_png_write_warning);
	png_write_init(PngStruct);
	png_set_write_fn(PngStruct, this, g42_png_write_data, g42_png_flush_data);
	PngInfo->width = info->Width;
	PngInfo->height = info->Height;
	if (info->ColorType == G42ImageInfo::ColorTypeTrueColor)
	{
		PngInfo->bit_depth = 8;
		PngInfo->color_type = PNG_COLOR_TYPE_RGB;
	}
	else if (info->ColorType == G42ImageInfo::ColorTypeGrayscale)
	{
		PngInfo->bit_depth = (png_byte)info->PixelDepth;
		PngInfo->color_type = PNG_COLOR_TYPE_GRAY;
	}
	else
	{
		PngInfo->bit_depth = (png_byte)info->PixelDepth;
		PngInfo->color_type = PNG_COLOR_TYPE_PALETTE;
		PngInfo->valid |= PNG_INFO_PLTE;
		PngInfo->num_palette = (png_uint_16)info->NumPalette;
		PngPalette = new png_color [info->NumPalette];
		if (!PngPalette)
		{
			Valid = false;
			return false;
		}
		PngInfo->palette = PngPalette;
		const G42Color * pal = info->ImagePalette;
		for (int i = 0; i < info->NumPalette; i++)
		{
			PngPalette[i].red = pal[i].red;
			PngPalette[i].green = pal[i].green;
			PngPalette[i].blue = pal[i].blue;
		}
	}
	if (info->Progressive)
		PngInfo->interlace_type = 1;
	png_write_info(PngStruct, PngInfo);
#ifdef XWIN
	png_set_filler(PngStruct, 0, PNG_FILLER_AFTER);
#endif
#ifdef MACOS
	png_set_filler(PngStruct, 0, PNG_FILLER_BEFORE);
#endif
#ifdef MSWIN
	 png_set_bgr(PngStruct);
#endif
	if (info->Progressive)
		NumPasses = png_set_interlace_handling(PngStruct);
#if defined(G42_USE_EXCEPTIONS)
	} catch (G42PngError)
	{
		Valid = false;
		return false;
	}
#endif
	return Valid;
}
bool
G42PngWriter::WriteRow(const byte * row, const byte *)
{
#if !defined(G42_USE_EXCEPTIONS)
	if (setjmp(PngStruct->jmpbuf))
	{
		Valid = false;
		return false;
	}
#else
	try {
#endif
	png_write_rows(PngStruct, (byte **)&row, 1);
#if defined(G42_USE_EXCEPTIONS)
	} catch (G42PngError)
	{
		Valid = false;
		return false;
	}
#endif
	CurrentRow++;
	if (CurrentRow >= Height)
	{
		CurrentPass++;
		if (CurrentPass < NumPasses)
			CurrentRow = 0;
	}
	return Valid;
}
bool
G42PngWriter::WriteEnd(G42ImageInfo * info)
{
#if !defined(G42_USE_EXCEPTIONS)
	if (setjmp(PngStruct->jmpbuf))
	{
		Valid = false;
		return false;
	}
#else
	try {
#endif
	png_write_end(PngStruct, 0);
#if defined(G42_USE_EXCEPTIONS)
	} catch (G42PngError)
	{
		Valid = false;
		return false;
	}
#endif
	return G42RasterWriterBase::WriteEnd(info);
}
void
G42PngWriter::PngWriteData(byte * data, uint32 length)
{
	PutBytes(data, length);
}
void
G42PngWriter::PngWriteWarning(const char *)
{
}
void
G42PngWriter::PngWriteError(const char *)
{
#if !defined(G42_USE_EXCEPTIONS)
	longjmp(PngStruct->jmpbuf, 1);
#else
	throw G42PngImage::G42PngError();
#endif
}

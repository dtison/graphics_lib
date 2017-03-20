// giwpng.cpp - Png File
/*************************************************************
	File:          giwpng.cpp
	Copyright (c) 1996, Group 42, Inc.
	Description:   Derived class of giwbase that handles Png files
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "giwpng.h"
extern "C" void g42_png_write_error(png_structp png_ptr,
	png_const_charp message)
{
	G42ImageWriterPng * writer = (G42ImageWriterPng *)png_get_io_ptr(png_ptr);
	writer->PngWriteError(message);
}
extern "C" void g42_png_write_warning(png_structp png_ptr,
	png_const_charp message)
{
	G42ImageWriterPng * writer = (G42ImageWriterPng *)png_get_io_ptr(png_ptr);
	writer->PngWriteWarning(message);
}
extern "C" void g42_png_write_data(png_structp png_ptr, png_bytep data,
	png_uint_32 length)
{
	G42ImageWriterPng * writer = (G42ImageWriterPng *)png_get_io_ptr(png_ptr);
	writer->PngWriteData(data, length);
}
extern "C" void g42_png_flush_data(png_structp)
{
}
G42ImageWriterPng::G42ImageWriterPng(void)
:
	G42ImageWriterBase(), PngStruct(0), PngInfo(0), PngPalette(0),
	NumPasses(1), CurrentPass(0), NeedDestroy(false)
{
	SetInfo(G42PngTypeId, "png", "Png", true);
}
G42ImageWriterPng::G42ImageWriterPng(G42Notifier & notifier,
	G42BaseOptions * options)
:
	G42ImageWriterBase(notifier, options),
	PngStruct(0), PngInfo(0), PngPalette(0),
	NumPasses(1), CurrentPass(0), NeedDestroy(false)
{
	SetInfo(G42PngTypeId, "png", "Png", true);
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
G42ImageWriterPng::~G42ImageWriterPng()
{
	if (NeedDestroy)
	{
#if !defined(G42_USE_EXCEPTIONS)
		if (setjmp(PngStruct->jmpbuf))
		{
			Clear();
			return;
		}
#else
		try
		{
#endif
			png_write_destroy(PngStruct);
#if defined(G42_USE_EXCEPTIONS)
		}
		catch (G42PngError)
		{
		}
#endif
	}
	Clear();
	Valid = false;
}
void
G42ImageWriterPng::Clear(void)
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
G42ImageWriterPng::WriteInfo(G42ImageDescriptor * info)
{
	G42ImageWriterBase::WriteInfo(info); // set up base stuff
#if !defined(G42_USE_EXCEPTIONS)
	if (setjmp(PngStruct->jmpbuf))
	{
		Valid = false;
		return false;
	}
#else
	try {
#endif
	if (!Options || !dynamic_cast<G42ImageReadWriteOptions *>(Options))
	{
		G42ImageReadWriteOptions * options =
			new G42ImageReadWriteOptions(G42PngTypeId);
		G42ThrowIfNull(options);
		AskForOptions(options);
	}
	G42ImageReadWriteOptions * options;
	if (Options)
	{
		options = dynamic_cast<G42ImageReadWriteOptions *>(Options);
	}
	if (options)
	{
		Progressive = options->IsProgressive();
	}
	else
	{
		Progressive = false;
	}
	png_info_init(PngInfo);
	png_set_message_fn(PngStruct, this,
		g42_png_write_error, g42_png_write_warning);
	png_write_init(PngStruct);
	NeedDestroy = true;
	png_set_write_fn(PngStruct, this, g42_png_write_data, g42_png_flush_data);
	PngInfo->width = Width;
	PngInfo->height = Height;
	if (info->GetDepth() > 8)
	{
		PngInfo->bit_depth = 8;
		PngInfo->color_type = PNG_COLOR_TYPE_RGB;
		if (info->IsTransparent())
		{
			if (info->HasAlphaChannel())
			{
				PngInfo->color_type = PNG_COLOR_TYPE_RGB_ALPHA;
			}
			else
			{
				PngInfo->valid |= PNG_INFO_tRNS;
				PngInfo->trans_values.red = info->GetTransparentColor().Red;
				PngInfo->trans_values.green = info->GetTransparentColor().Green;
				PngInfo->trans_values.blue = info->GetTransparentColor().Blue;
			}
		}
	}
	else if (!info->IsColor())
	{
		PngInfo->bit_depth = (png_byte)info->GetDepth();
		PngInfo->color_type = PNG_COLOR_TYPE_GRAY;
		if (info->IsTransparent())
		{
			if (info->HasAlphaChannel())
			{
				PngInfo->color_type = PNG_COLOR_TYPE_RGB_ALPHA;
			}
			else
			{
				PngInfo->valid |= PNG_INFO_tRNS;
				intf transparent_pixel = info->GetTransparentColor().Red; // grayscale, better be ok
				transparent_pixel = (intf)(((int32)transparent_pixel *
					(int32)((1 << info->GetDepth()) - 1)) / (int32)(255));
				PngInfo->trans_values.gray = (png_byte)transparent_pixel;
			}
		}
	}
	else
	{
		PngInfo->bit_depth = (png_byte)info->GetDepth();
		/*
		if (info->NumPalette <= 4 && PngInfo->bit_depth == 4)
		{
			PngInfo->bit_depth = 2;
			Depth = 2;
//			RowBytes = ((Width * (uint32)Depth + 7) >> 3);
		}
		*/
		PngInfo->color_type = PNG_COLOR_TYPE_PALETTE;
		PngInfo->valid |= PNG_INFO_PLTE;
		intf num_palette = info->GetNumPalette();
		intf trans_pixel = 0;
		if (info->IsTransparent())
		{
			PngInfo->valid |= PNG_INFO_tRNS;
			if (info->HasTransparentPixel())
				trans_pixel = info->GetTransparentPixel();
			else if (num_palette < 256)
				trans_pixel = num_palette++;
			else
				PngInfo->valid &= ~PNG_INFO_tRNS; // give up, as we just don't
															// handle this internally good
															// enough to do anything reasonable
															// hope that they chose a color not
															// in the image (bad, but no choice)
			PngInfo->num_trans = (png_uint_16)(trans_pixel + 1);
			PngInfo->trans = new png_byte [PngInfo->num_trans];
			for (png_uint_16 i = 0; i < PngInfo->num_trans - 1; i++)
				PngInfo->trans[i] = 0xff;
			PngInfo->trans[PngInfo->num_trans - 1] = 0;
		}
		PngInfo->num_palette = (png_uint_16)num_palette;
		PngPalette = new png_color [num_palette];
		G42ThrowIfNull(PngPalette);
		PngInfo->palette = PngPalette;
		const G42Color * pal = info->GetPalette();
		for (intf i = 0; i < num_palette; i++)
		{
			if (i < info->GetNumPalette())
			{
				PngPalette[i].red = pal[i].Red;
				PngPalette[i].green = pal[i].Green;
				PngPalette[i].blue = pal[i].Blue;
			}
			else
			{
				PngPalette[i].red = info->GetTransparentColor().Red;
				PngPalette[i].green = info->GetTransparentColor().Green;
				PngPalette[i].blue = info->GetTransparentColor().Blue;
			}
		}
	}
	if (Progressive)
		PngInfo->interlace_type = 1;
#if 0
	if (info->NumComments)
	{
		PngInfo->num_text = info->NumComments;
		PngInfo->max_text = info->NumComments;
		PngInfo->text = new png_text [info->NumComments];
		for (intf i = 0; i < info->NumComments; i++)
		{
			PngInfo->text[i].compression = -1;
			G42Comment * comment = info->Comments[i];
			if (comment->GetKey())
			{
				PngInfo->text[i].key =
					new char [strlen(comment->GetKey()) + 1];
				strcpy(PngInfo->text[i].key, comment->GetKey());
			}
			else
			{
				PngInfo->text[i].key = new char [8];
				strcpy(PngInfo->text[i].key, "Comment");
			}
			if (comment->GetText())
			{
				PngInfo->text[i].text_length = strlen(comment->GetText());
				PngInfo->text[i].text =
					new char [PngInfo->text[i].text_length + 1];
				strcpy(PngInfo->text[i].text, comment->GetText());
			}
			else
			{
				PngInfo->text[i].text = 0;
				PngInfo->text[i].text_length = 0;
			}
		}
	}
#endif
	png_write_info(PngStruct, PngInfo);
#ifdef MSWIN
	 png_set_bgr(PngStruct);
#endif
	if (info->GetDepth() == 32)
	{
#ifdef MSWIN
		png_set_filler(PngStruct, 0, PNG_FILLER_AFTER);
#endif
#ifdef XWIN
		png_set_filler(PngStruct, 0, PNG_FILLER_AFTER);
#endif
#ifdef MACOS
		png_set_filler(PngStruct, 0, PNG_FILLER_BEFORE);
#endif
	}
	if (Progressive)
	{
		NumPasses = png_set_interlace_handling(PngStruct);
	}
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
G42ImageWriterPng::WriteRow(const byte * row, const byte * mask)
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
	if (PngInfo->color_type & PNG_COLOR_MASK_ALPHA)
	{
		if (PngInfo->color_type == PNG_COLOR_TYPE_RGB_ALPHA)
		{
			byte * buf = new byte [PngInfo->width * 4];
			if (Descriptor->GetDepth() == 24)
			{
				G42ConstColor24Iterator sp(row);
				byte * dp = buf;
				const byte * mp = mask;
				intf m = 0x80;
				for (intf i = 0; i < PngInfo->width; i++)
				{
					G42ConstColor24 c(*sp);
					*dp++ = c.GetRed();
					*dp++ = c.GetGreen();
					*dp++ = c.GetBlue();
					sp++;
					if (*mp & m)
						*dp++ = 0xff;
					else
						*dp++ = 0;
					if (m == 1)
					{
						m = 0x80;
						mp++;
					}
					else
					{
						m >>= 1;
					}
				}
			}
			else
			{
				G42ConstColor32Iterator sp(row);
				byte * dp = buf;
				const byte * mp = mask;
				intf m = 0x80;
				for (intf i = 0; i < PngInfo->width; i++)
				{
					G42ConstColor32 c(*sp);
					*dp++ = c.GetRed();
					*dp++ = c.GetGreen();
					*dp++ = c.GetBlue();
					sp++;
					if (*mp & m)
						*dp++ = 0xff;
					else
						*dp++ = 0;
					if (m == 1)
					{
						m = 0x80;
						mp++;
					}
					else
					{
						m >>= 1;
					}
				}
			}
			png_write_rows(PngStruct, (byte **)&buf, 1);
			delete [] buf;
		}
		else
		{
			byte * buf = new byte [PngInfo->width * 2];
			const byte * sp = row;
			byte * dp = buf;
			const byte * mp = mask;
			intf m = 0x80;
			for (intf i = 0; i < PngInfo->width; i++)
			{
				*dp++ = *sp++;
				if (*mp & m)
					*dp++ = 0xff;
				else
					*dp++ = 0;
				if (m == 1)
				{
					m = 0x80;
					mp++;
				}
				else
				{
					m >>= 1;
				}
			}
			png_write_rows(PngStruct, (byte **)&buf, 1);
			delete [] buf;
		}
	}
	else
	{
		png_write_rows(PngStruct, (byte **)&row, 1);
	}
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
G42ImageWriterPng::WriteEnd(void)
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
	return G42ImageWriterBase::WriteEnd();
}
void
G42ImageWriterPng::PngWriteData(byte * data, uint32 length)
{
	PutBytes(data, length);
}
void
G42ImageWriterPng::PngWriteWarning(const char *)
{
}
void
G42ImageWriterPng::PngWriteError(const char *)
{
#if !defined(G42_USE_EXCEPTIONS)
	longjmp(PngStruct->jmpbuf, 1);
#else
	throw G42PngImage::G42PngError();
#endif
}

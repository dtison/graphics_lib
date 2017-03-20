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
#include "girpng.h"
#include "gidata.h"
G42ImageReaderPng::G42ImageReaderPng(void) // won't actually read
:
	G42ImageReaderBase(),
	RowBuf(0), MaskBuf(0), NeedConvert(false), NeedConvertMask(false),
	DestroyLibrary(false), Valid(true)
{
	SetInfo(G42PngTypeId, "png", "Png", "", 0); // use lib checker
}
G42ImageReaderPng::G42ImageReaderPng(G42HeapManager & heap,
	G42Notifier & notifier, G42File * file, G42BaseOptions * options)
:
	G42ImageReaderBase(heap, notifier, file, options),
	RowBuf(0), MaskBuf(0), NeedConvert(false), NeedConvertMask(false),
	DestroyLibrary(false), Valid(true)
{
	SetInfo(G42PngTypeId, "png", "Png", "", 0); // use lib checker
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
	DestroyLibrary = true;
	png_set_progressive_read_fn(&LibPng, this,
		g42_png_info_cb, g42_png_row_cb, g42_png_end_cb);
#if defined(G42_USE_EXCEPTIONS)
	} catch (G42PngError)
	{
	}
#endif
}
G42ImageReaderPng::~G42ImageReaderPng()
{
	if (DestroyLibrary)
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
}
void
G42ImageReaderPng::BeginProcessing()
{
	// we really need to implement the non-streaming side of the lib
	uintf buf_size = 32767;
	G42APtr<byte> buf(new byte [buf_size]);
#if !defined(G42_USE_EXCEPTIONS)
	if (setjmp(LibPng.jmpbuf))
	{
		Valid = false;
		return;
	}
#else
	try {
#endif
		while (!Finished)
		{
			uintf size = File->Read(buf, buf_size, false);
			if (!size)
			{
				break;
			}
			png_process_data(&LibPng, &InfoPtr, buf, size);
		}
#if defined(G42_USE_EXCEPTIONS)
	} catch (G42PngError)
	{
		Valid = false;
	}
#endif
}
void
G42ImageReaderPng::SendExtendedInfo()
{
	// we really need to implement the non-streaming side of the lib
	uintf buf_size = 32767;
	G42APtr<byte> buf(new byte [buf_size]);
#if !defined(G42_USE_EXCEPTIONS)
	if (setjmp(LibPng.jmpbuf))
	{
		Valid = false;
		return;
	}
#else
	try {
#endif
		while (!Finished)
		{
			uintf size = File->Read(buf, buf_size, false);
			if (!size)
			{
				break;
			}
			png_process_data(&LibPng, &InfoPtr, buf, size);
		}
#if defined(G42_USE_EXCEPTIONS)
	} catch (G42PngError)
	{
		Valid = false;
	}
#endif
}
void
G42ImageReaderPng::ProcessStreamData()
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
		png_process_data(&LibPng, &InfoPtr, (byte *)CurrentBuffer, BufferLeft);
#if defined(G42_USE_EXCEPTIONS)
		} catch (G42PngError)
		{
			Valid = false;
		}
#endif
	}
}
void
G42ImageReaderPng::PngInfo(png_infop)
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
	if (InfoPtr.bit_depth < 8 && InfoPtr.bit_depth > 1 &&
		InfoPtr.color_type == PNG_COLOR_TYPE_GRAY)
	{
		png_set_expand(&LibPng);
	}
	if (InfoPtr.bit_depth < 8 &&
		InfoPtr.color_type == PNG_COLOR_TYPE_PALETTE)
	{
		png_set_packing(&LibPng);
	}
	// rgb alpha types should stay rgb because we need to convert them
	if (InfoPtr.color_type == PNG_COLOR_TYPE_RGB)
	{
#ifdef MSWIN
		png_set_bgr(&LibPng);
#endif
#ifdef MACOS
		png_set_filler(&LibPng, 0, PNG_FILLER_BEFORE);
#endif
#ifdef XWIN
		png_set_filler(&LibPng, 0xff, PNG_FILLER_AFTER);
#endif
	}
/*
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
*/
	png_read_update_info(&LibPng, &InfoPtr);
	Width = InfoPtr.width;
	Height = InfoPtr.height;
	TopLineFirst = true;
	Depth = InfoPtr.pixel_depth;
	RowBytes = ((Width * Depth + 7) >> 3);
	Compressed = true;
	if (InfoPtr.color_type & PNG_COLOR_MASK_ALPHA)
	{
		Transparent = true;
		NeedConvert = true;
	}
	else if (InfoPtr.valid & PNG_INFO_tRNS)
	{
		Transparent = true;
		NeedConvertMask = true;
	}
	if (InfoPtr.color_type == PNG_COLOR_TYPE_PALETTE)
	{
		NumPalette = InfoPtr.num_palette;
		Color = true;
		SetupDescriptor();
		for (intf i = 0; i < NumPalette; i++)
		{
			ImagePalette[i].Red = InfoPtr.palette[i].red;
			ImagePalette[i].Green = InfoPtr.palette[i].green;
			ImagePalette[i].Blue = InfoPtr.palette[i].blue;
		}
	}
	else if (!(InfoPtr.color_type & PNG_COLOR_MASK_COLOR)) // grayscale
	{
		NumPalette = 0;
		if (Depth > 8) // alpha channel
			Depth = 8;
//		RowBytes = ((Width * Depth + 7) >> 3);
		Color = false;
		SetupDescriptor();
	}
	else // true color
	{
		NumPalette = 0;
		Depth = 24;
//		RowBytes = ((Width * Depth + 7) >> 3);
		Color = true;
		SetupDescriptor();
	}
	if (InfoPtr.color_type & PNG_COLOR_MASK_ALPHA)
	{
		AlphaChannel = true;
	}
	else if (InfoPtr.valid & PNG_INFO_tRNS)
	{
		if (InfoPtr.color_type == PNG_COLOR_TYPE_GRAY)
		{
			TransparentPixel = InfoPtr.trans_values.gray;
		}
		else if (InfoPtr.color_type != PNG_COLOR_TYPE_PALETTE)
		{
			TransparentColor.Red = InfoPtr.trans_values.red;
			TransparentColor.Green = InfoPtr.trans_values.green;
			TransparentColor.Blue = InfoPtr.trans_values.blue;
			TransparentPixel = -1;
		}
		else // paletted file
		{
			bool single_trans = false;
			intf trans_pixel = -1;
			if (InfoPtr.num_trans == 1 && InfoPtr.trans[0] < 128)
			{
				single_trans = true;
				trans_pixel = 0;
			}
			else if (InfoPtr.num_trans > 1)
			{
				single_trans = true;
				for (intf i = 0; i < InfoPtr.num_trans; i++)
				{
					if (InfoPtr.trans[i] < 128)
					{
						if (trans_pixel < 0)
						{
							trans_pixel = i;
						}
						else
						{
							single_trans = false;
							break;
						}
					}
				}
			}
			if (!single_trans)
			{
				AlphaChannel = true;
				TransparentPixel = -1;
			}
			else
			{
				TransparentPixel = trans_pixel;
			}
		}
	}
	if (InfoPtr.valid & PNG_INFO_bKGD)
	{
		Background = true;
		if (InfoPtr.color_type == PNG_COLOR_TYPE_GRAY ||
			InfoPtr.color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		{
			BackgroundPixel = InfoPtr.background.gray;
		}
		else if (InfoPtr.color_type != PNG_COLOR_TYPE_PALETTE)
		{
			BackgroundColor.Red = InfoPtr.background.red;
			BackgroundColor.Green = InfoPtr.background.green;
			BackgroundColor.Blue = InfoPtr.background.blue;
			BackgroundPixel = -1;
		}
		else // palette
		{
			BackgroundPixel = InfoPtr.background.index;
		}
	}
#if 0 // comments later
	// add comments
	if (InfoPtr.num_text)
	{
		Info.NumComments = 0;
		for (intf i = 0; i < InfoPtr.num_text; i++)
		{
			G42Comment * comment = new G42Comment();
			comment->SetComment(InfoPtr.text[i].key,
				InfoPtr.text[i].text);
			Info.Comments[Info.NumComments++].Set(comment);
		}
	}
#endif
	SendDescriptor();
}
void
G42ImageReaderPng::PngRow(png_bytep row, png_uint_32, intf pass)
{
	if (row)
	{
		if (pass && pass < 6)
		{
			if (NeedConvert)
			{
				PngConvertRowBack(RowBuffer, OutputTransRowPtr, OutputRowPtr);
				png_progressive_combine_row(&LibPng, RowBuffer, row);
			}
			else
			{
				png_progressive_combine_row(&LibPng, RowBuffer, row);
			}
			if (NeedConvert)
			{
				PngConvertRow(OutputRowPtr, OutputTransRowPtr, RowBuffer);
				OutputRow();
			}
			else if (NeedConvertMask)
			{
				PngConvertRow(OutputRowPtr, OutputTransRowPtr, RowBuffer);
				OutputRow();
			}
			else
			{
				OutputRow();
			}
		}
		else
		{
			if (NeedConvert)
			{
				PngConvertRow(OutputRowPtr, OutputTransRowPtr, row);
				OutputRow();
			}
			else if (NeedConvertMask)
			{
				PngConvertRow(OutputRowPtr, OutputTransRowPtr, row);
				OutputRow();
			}
			else
			{
				memcpy(OutputRowPtr, row, RowBytes);
				OutputRow();
			}
		}
	}
	else
	{
		OutputRow();
	}
}
void
G42ImageReaderPng::PngConvertRowBack(byte * png_row, byte * mask, byte * row)
{
	if (InfoPtr.color_type & PNG_COLOR_MASK_ALPHA)
	{
		if (Color)
		{
			if (GetImageDepth() == 24)
			{
				G42Color24Iterator rp(row);
				G42BitIterator mp(mask);
				byte * dp = png_row;
				for (intf i = 0; i < Width; i++)
				{
					G42Color c = (G42Color24)(*rp++);
					*dp++ = c.Red;
					*dp++ = c.Green;
					*dp++ = c.Blue;
					*dp++ = (byte)(*mp ? 0xff : 0);
					mp++; // this does better incrementing after
				}
			}
			else
			{
				G42Color32Iterator rp(row);
				G42BitIterator mp(mask);
				byte * dp = png_row;
				for (intf i = 0; i < Width; i++)
				{
					G42Color c = (G42Color32)(*rp++);
					*dp++ = c.Red;
					*dp++ = c.Green;
					*dp++ = c.Blue;
					*dp++ = (byte)(*mp ? 0xff : 0);
					mp++; // this does better incrementing after
				}
			}
		}
		else
		{
			byte * rp = row;
			G42BitIterator mp(mask);
			byte * dp = png_row;
			for (intf i = 0; i < Width; i++)
			{
				*dp++ = *rp++;
				*dp++ = (byte)(*mp ? 0xff : 0);
				mp++; // this does better incrementing after
			}
		}
	}
}
void
G42ImageReaderPng::PngConvertRow(byte * row, byte * mask, byte * png_row)
{
	if (InfoPtr.color_type & PNG_COLOR_MASK_ALPHA)
	{
		if (Color)
		{
			if (GetImageDepth() == 24)
			{
				G42Color24Iterator rp(row);
				G42BitIterator mp(mask);
				byte * sp = png_row;
				for (intf i = 0; i < Width; i++)
				{
					*rp++ = G42Color(*sp, *(sp + 1), *(sp + 2));
					sp += 3;
					*mp++ = (*sp++ >= 128);
				}
			}
			else
			{
				G42Color32Iterator rp(row);
				G42BitIterator mp(mask);
				byte * sp = png_row;
				for (intf i = 0; i < Width; i++)
				{
					*rp++ = G42Color(*sp, *(sp + 1), *(sp + 2));
					sp += 3;
					*mp++ = (*sp++ >= 128);
				}
			}
		}
		else
		{
			byte * rp = row;
			G42BitIterator mp(mask);
			byte * sp = png_row;
			for (intf i = 0; i < Width; i++)
			{
				*rp++ = *sp++;
				*mp++ = (*sp++ >= 128);
			}
		}
	}
	else
	{
		if (NeedConvertMask && AlphaChannel)
		{
			byte * data = (NeedConvert ? row : png_row);
			G42BitIterator mp(mask);
			if (Depth == 1)
			{
				G42BitIterator dp(data);
				for (intf i = 0; i < Width; i++)
				{
					intf pixel = (*dp++ ? 1 : 0);
					*mp++ = (pixel >= InfoPtr.num_trans ||
						 InfoPtr.trans[pixel] >= 128);
				}
			}
			else // Depth == 8
			{
				byte * dp = data;
				for (intf i = 0; i < Width; i++)
				{
					intf pixel = *dp++;
					*mp++ = (pixel >= InfoPtr.num_trans ||
						 InfoPtr.trans[pixel] >= 128);
				}
			}
		}
		else if (NeedConvertMask) // simple transparency, build mask
		{
			byte * data = (NeedConvert ? row : png_row);
			G42BitIterator mp(mask);
			if (Depth == 1)
			{
				G42BitIterator dp(data);
				for (intf i = 0; i < Width; i++)
				{
					intf pixel = (*dp++ ? 1 : 0);
					*mp++ = (pixel == TransparentPixel);
				}
			}
			else if (Depth == 8)
			{
				byte * dp = data;
				for (intf i = 0; i < Width; i++)
				{
					intf pixel = *dp++;
					*mp++ = (pixel == TransparentPixel);
				}
			}
			else if (GetImageDepth() == 24)
			{
				G42Color24Iterator dp(data);
				for (intf i = 0; i < Width; i++)
				{
					*mp++ = (TransparentColor == (G42Color)(G42Color24)(*dp++));
				}
			}
			else if (GetImageDepth() == 32)
			{
				G42Color32Iterator dp(data);
				for (intf i = 0; i < Width; i++)
				{
					*mp++ = (TransparentColor == (G42Color)(G42Color32)(*dp++));
				}
			}
		}
	}
}
void
G42ImageReaderPng::PngEnd(png_infop)
{
//	MessageBox(0, "Png Finished", "Trace", MB_OK);
	ForceFileEnd();
}
void
G42ImageReaderPng::PngWarning(const char * message)
{
	#ifdef MACOS
	if (message) {}	// Quiet warnings
	#endif
	
//	MessageBox(0, message, "Warning", MB_OK);
}
void
G42ImageReaderPng::PngError(const char * message)
{
//	MessageBox(0, message, "Error", MB_OK);
	Valid = false;
	#ifdef MACOS
	if (message) {}	// Quiet warnings
	#endif
#if !defined(G42_USE_EXCEPTIONS)
	longjmp(LibPng.jmpbuf, 1);
#else
	throw G42ImageReaderPng::G42PngError();
#endif
}
bool
G42ImageReaderPng::TestType(void)
{
	File->Rewind();
	byte buf[8];
	File->Read(buf, 8);
	File->Rewind();
	return TestType(buf, 8);
}
bool
G42ImageReaderPng::TestType(const byte * buffer, uint32 buffer_size)
{
	return png_check_sig((byte *)buffer, buffer_size);
}
extern "C" void g42_png_error(png_structp png_ptr, png_const_charp message)
{
	G42ImageReaderPng * image = (G42ImageReaderPng *)png_get_msg_ptr(png_ptr);
	if (image)
	{
		image->PngError(message);
	}
}
extern "C" void g42_png_warning(png_structp png_ptr, png_const_charp message)
{
	G42ImageReaderPng * image = (G42ImageReaderPng *)png_get_msg_ptr(png_ptr);
	if (image)
	{
		image->PngWarning(message);
	}
}
extern "C" void g42_png_info_cb(png_structp png_ptr, png_infop info)
{
	G42ImageReaderPng * image = (G42ImageReaderPng *)png_get_msg_ptr(png_ptr);
	if (image)
	{
		image->PngInfo(info);
	}
}
extern "C" void g42_png_end_cb(png_structp png_ptr, png_infop info)
{
	G42ImageReaderPng * image = (G42ImageReaderPng *)png_get_msg_ptr(png_ptr);
	if (image)
	{
		image->PngEnd(info);
	}
}
extern "C" void g42_png_row_cb(png_structp png_ptr, png_bytep row,
	png_uint_32 row_num, intf pass)
{
	G42ImageReaderPng * image = (G42ImageReaderPng *)png_get_msg_ptr(png_ptr);
	if (image)
	{
		image->PngRow(row, row_num, pass);
	}
}

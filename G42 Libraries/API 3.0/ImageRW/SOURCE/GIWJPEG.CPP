// giwjpeg.cpp - Jpeg File
/*************************************************************
	File:          giwjpeg.cpp
	Copyright (c) 1996, Group 42, Inc.
	Description:   Derived class of giwbase that handles Jpeg files
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "giwjpeg.h"
G42ImageWriterJpeg::G42ImageWriterJpeg(void)
:
	G42ImageWriterBase(), Progressive(false), NeedDestroy(false),
	TempRow(0), OutputBuffer(0), OutputBufferSize(4092)
{
	SetInfo(G42JpegTypeId, "jpg", "Jpeg", true);
	LibJpeg.ptr = this;
}
G42ImageWriterJpeg::G42ImageWriterJpeg(G42Notifier & notifier,
	G42BaseOptions * options)
:
	G42ImageWriterBase(notifier, options), Progressive(false),
	TempRow(0), OutputBuffer(0), OutputBufferSize(4092), NeedDestroy(false)
{
	SetInfo(G42JpegTypeId, "jpg", "Jpeg", true);
	OutputBuffer = new byte [OutputBufferSize];
	LibJpeg.ptr = this;
}
G42ImageWriterJpeg::~G42ImageWriterJpeg()
{
	if (NeedDestroy)
	{
#if !defined(G42_USE_EXCEPTIONS)
		if (setjmp(LibJpegError.setjmp_buf))
		{
			if (TempRow)
			{
				delete [] TempRow;
				TempRow = 0;
			}
			if (OutputBuffer)
			{
				delete [] OutputBuffer;
				OutputBuffer = 0;
			}
			return;
		}
#else
		try
		{
#endif
			jpeg_destroy_compress(&(LibJpeg.j));
#if defined(G42_USE_EXCEPTIONS)
		}
		catch (G42JpegError)
		{
		}
#endif
	}
	if (TempRow)
	{
		delete [] TempRow;
		TempRow = 0;
	}
	if (OutputBuffer)
	{
		delete [] OutputBuffer;
		OutputBuffer = 0;
	}
	Valid = false;
}
bool
G42ImageWriterJpeg::WriteInfo(G42ImageDescriptor * info)
{
	LibJpeg.j.err = jpeg_std_error(&(LibJpegError.j));
	LibJpegError.j.error_exit = g42_jpeg_write_error_exit;
	LibJpegError.j.output_message = g42_jpeg_write_output_message;
	LibJpegError.j.emit_message = g42_jpeg_write_emit_message;
	LibJpegError.ptr = this;
#if !defined(G42_USE_EXCEPTIONS)
	if (setjmp(LibJpegError.setjmp_buf))
	{
		Valid = false;
		return false;
	}
#else
	try {
#endif
	jpeg_create_compress(&(LibJpeg.j));
	NeedDestroy = true;
	LibJpegDest.ptr = this;
	LibJpegDest.j.init_destination = g42_jpeg_init_destination;
	LibJpegDest.j.term_destination = g42_jpeg_term_destination;
	LibJpegDest.j.empty_output_buffer = g42_jpeg_empty_output_buffer;
	LibJpeg.j.dest = &(LibJpegDest.j);
	G42ImageWriterBase::WriteInfo(info); // set up base stuff
	ImagePalette = info->GetPalette();
	NumPalette = info->GetNumPalette();
	TrueDepth = (uintf)Depth;
	LibJpeg.j.image_width = Width;
	LibJpeg.j.image_height = Height;
	if (!info->IsColor())
	{
		Depth = 8;
		RowBytes = Width;
		LibJpeg.j.input_components = 1;
		LibJpeg.j.in_color_space = JCS_GRAYSCALE;
	}
	else
	{
		Depth = 24;
		RowBytes = Width * 3;
		LibJpeg.j.input_components = 3;
		LibJpeg.j.in_color_space = JCS_RGB;
	}
	TempRow = new byte [(size_t)RowBytes];
	if (!TempRow)
	{
		Valid = false;
		return false;
	}
	jpeg_set_defaults(&(LibJpeg.j));
	Progressive = false;
	if (Options && dynamic_cast<G42ImageReadWriteOptions *>(Options))
	{
		G42ImageReadWriteOptions * options =
			dynamic_cast<G42ImageReadWriteOptions *>(Options);
		if (options)
		{
			Progressive = options->IsProgressive();
		}
	}
	if (!Options || !dynamic_cast<G42ImageJpegOptions *>(Options))
	{
		G42ImageJpegOptions * options =
			new G42ImageJpegOptions(Progressive);
		G42ThrowIfNull(options);
		AskForOptions(options);
	}
	G42ImageJpegOptions * options;
	if (Options)
	{
		options = dynamic_cast<G42ImageJpegOptions *>(Options);
	}
	if (options)
	{
		Quality = options->GetQuality();
		Photo = options->IsPhoto();
		Progressive = options->IsProgressive();
	}
	else
	{
		if (Options && dynamic_cast<G42ImageReadWriteOptions *>(Options))
		{
			G42ImageReadWriteOptions * options =
				dynamic_cast<G42ImageReadWriteOptions *>(Options);
			if (options)
			{
				Progressive = options->IsProgressive();
			}
		}
		Quality = 90;
		Photo = true;
	}
	jpeg_set_quality(&(LibJpeg.j), Quality, TRUE);
	if (Progressive)
		jpeg_simple_progression(&(LibJpeg.j));
	if (Depth > 8 && !Photo)
	{
		LibJpeg.j.comp_info[0].h_samp_factor = 1;
		LibJpeg.j.comp_info[0].v_samp_factor = 1;
		LibJpeg.j.comp_info[1].h_samp_factor = 1;
		LibJpeg.j.comp_info[1].v_samp_factor = 1;
		LibJpeg.j.comp_info[2].h_samp_factor = 1;
		LibJpeg.j.comp_info[2].v_samp_factor = 1;
	}
	jpeg_start_compress(&(LibJpeg.j), TRUE);
#if defined(G42_USE_EXCEPTIONS)
	} catch (G42JpegError)
	{
		Valid = false;
		return false;
	}
#endif
	return Valid;
}
bool
G42ImageWriterJpeg::WriteRow(const byte * row, const byte *)
{
	const byte * buffer = row;
	if (Depth == 24)
	{
		if (Descriptor->GetDepth() == 24)
		{
			G42ConstColor24Iterator sp(row);
			byte * dp = TempRow;
			for (uint32 i = 0; i < Width; i++)
			{
				G42ConstColor24 c(*sp);
				*dp++ = c.GetRed();
				*dp++ = c.GetGreen();
				*dp++ = c.GetBlue();
				sp++;
			}
			buffer = TempRow;
		}
		else
		{
			G42ConstColor32Iterator sp(row);
			byte * dp = TempRow;
			for (uint32 i = 0; i < Width; i++)
			{
				G42ConstColor32 c(*sp);
				*dp++ = c.GetRed();
				*dp++ = c.GetGreen();
				*dp++ = c.GetBlue();
				sp++;
			}
			buffer = TempRow;
		}
	}
#if !defined(G42_USE_EXCEPTIONS)
	if (setjmp(LibJpegError.setjmp_buf))
	{
		Valid = false;
		return false;
	}
#else
	try {
#endif
	intf s = jpeg_write_scanlines(&(LibJpeg.j), (byte **)&buffer, 1);
	if (s != 1)
	{
		Valid = false;
		return false;
	}
#if defined(G42_USE_EXCEPTIONS)
	} catch (G42JpegError)
	{
		Valid = false;
		return false;
	}
#endif
	CurrentRow++;
	return Valid;
}
bool
G42ImageWriterJpeg::WriteEnd(void)
{
#if !defined(G42_USE_EXCEPTIONS)
	if (setjmp(LibJpegError.setjmp_buf))
	{
		Valid = false;
		return false;
	}
#else
	try {
#endif
	jpeg_finish_compress(&(LibJpeg.j));
#if defined(G42_USE_EXCEPTIONS)
	} catch (G42JpegError)
	{
		Valid = false;
		return false;
	}
#endif
	return G42ImageWriterBase::WriteEnd();
}
void
G42ImageWriterJpeg::JpegOutputMessage(void)
{
}
void
G42ImageWriterJpeg::JpegEmitMessage(intf)
{
}
void
G42ImageWriterJpeg::JpegErrorExit(void)
{
#if !defined(G42_USE_EXCEPTIONS)
	longjmp(LibJpegError.setjmp_buf, 1);
#else
	throw G42JpegImage::G42JpegError();
#endif
}
extern "C" void g42_jpeg_write_output_message(j_common_ptr cinfo)
{
	g42_jpeg_write_error_mgr * j = (g42_jpeg_write_error_mgr *)(cinfo->err);
	j->ptr->JpegOutputMessage();
}
extern "C" void g42_jpeg_write_emit_message(j_common_ptr cinfo, int msg_level)
{
	g42_jpeg_write_error_mgr * j = (g42_jpeg_write_error_mgr *)(cinfo->err);
	j->ptr->JpegEmitMessage(msg_level);
}
extern "C" void g42_jpeg_write_error_exit(j_common_ptr cinfo)
{
	g42_jpeg_write_error_mgr * j = (g42_jpeg_write_error_mgr *)(cinfo->err);
	j->ptr->JpegErrorExit();
}
extern "C" void g42_jpeg_init_destination(j_compress_ptr cinfo)
{
	g42_jpeg_compress_struct * j = (g42_jpeg_compress_struct *)(cinfo);
	j->ptr->InitDestination();
}
extern "C" boolean g42_jpeg_empty_output_buffer(j_compress_ptr cinfo)
{
	g42_jpeg_compress_struct * j = (g42_jpeg_compress_struct *)(cinfo);
	return j->ptr->EmptyOutputBuffer();
}
extern "C" void g42_jpeg_term_destination(j_compress_ptr cinfo)
{
	g42_jpeg_compress_struct * j = (g42_jpeg_compress_struct *)(cinfo);
	j->ptr->TermDestination();
}
void
G42ImageWriterJpeg::InitDestination()
{
	LibJpeg.j.dest->next_output_byte = OutputBuffer;
	LibJpeg.j.dest->free_in_buffer = OutputBufferSize;
}
boolean
G42ImageWriterJpeg::EmptyOutputBuffer()
{
	if (LibJpeg.j.dest->free_in_buffer != OutputBufferSize)
		PutBytes(OutputBuffer, OutputBufferSize);
	LibJpeg.j.dest->next_output_byte = OutputBuffer;
	LibJpeg.j.dest->free_in_buffer = OutputBufferSize;
	return (TRUE);
}
void
G42ImageWriterJpeg::TermDestination()
{
	if (LibJpeg.j.dest->free_in_buffer != OutputBufferSize)
		PutBytes(OutputBuffer, OutputBufferSize - LibJpeg.j.dest->free_in_buffer);
	LibJpeg.j.dest->next_output_byte = OutputBuffer;
	LibJpeg.j.dest->free_in_buffer = OutputBufferSize;
}

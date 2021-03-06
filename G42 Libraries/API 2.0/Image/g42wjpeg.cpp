// g42jpeg.cpp - Jpeg File
/*************************************************************
	File:          g42jpeg.cpp
	Copyright (c) 1996, Group 42, Inc.
	Description:   Derived class of g42ribase that handles Jpeg files
   Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
   Modification History:
      Code   Date   Name and Description
***************************************************************/
#include "g42itype.h"
#include "g42wjpeg.h"
#include "g42idata.h"
#include "g42iter.h"
G42JpegWriter::G42JpegWriter(G42RasterImageWriter & parent) :
	G42RasterWriterBase(parent), Progressive(false),
	TempRow(0), OutputBuffer(0), OutputBufferSize(4092)
{
	OutputBuffer = new byte [OutputBufferSize];
	LibJpeg.ptr = this;
}
G42JpegWriter::~G42JpegWriter()
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
	try {
#endif
	jpeg_destroy_compress(&(LibJpeg.j));
#if defined(G42_USE_EXCEPTIONS)
	} catch (G42JpegError)
	{
	}
#endif
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
G42JpegWriter::WriteInfo(G42ImageInfo * info)
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
#if defined(G42_USE_EXCEPTIONS)
	} catch (G42JpegError)
	{
		Valid = false;
		return false;
	}
#endif
	LibJpegDest.ptr = this;
	LibJpegDest.j.init_destination = g42_jpeg_init_destination;
	LibJpegDest.j.term_destination = g42_jpeg_term_destination;
	LibJpegDest.j.empty_output_buffer = g42_jpeg_empty_output_buffer;
	LibJpeg.j.dest = &(LibJpegDest.j);
	G42RasterWriterBase::WriteInfo(info); // set up base stuff
	ColorType = info->ColorType;
	ImagePalette = info->ImagePalette;
	NumPalette = info->NumPalette;
	TrueDepth = (uint)Depth;
	LibJpeg.j.image_width = Width;
	LibJpeg.j.image_height = Height;
	if (info->ColorType == G42ImageInfo::ColorTypeGrayscale)
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
	// (m.2)  This is a bug (add .j)
	jpeg_set_quality(&(LibJpeg.j), info->JpegQuality, TRUE);
	if (info->Progressive)
		jpeg_simple_progression(&(LibJpeg.j));
	if (Depth > 8 && !info->JpegSubSampling)
	{
		LibJpeg.j.comp_info[0].h_samp_factor = 1;
		LibJpeg.j.comp_info[0].v_samp_factor = 1;
		LibJpeg.j.comp_info[1].h_samp_factor = 1;
		LibJpeg.j.comp_info[1].v_samp_factor = 1;
		LibJpeg.j.comp_info[2].h_samp_factor = 1;
		LibJpeg.j.comp_info[2].v_samp_factor = 1;
	}
	jpeg_start_compress(&(LibJpeg.j), TRUE);
	return Valid;
}
bool
G42JpegWriter::WriteRow(const byte * row, const byte *)
{
	const byte * buffer = row;
	if (Depth == 24)
	{
		G42ConstByte24BitIterator sp(row);
		byte * dp = TempRow;
		for (uint32 i = 0; i < Width; i++)
		{
			*dp++ = sp.GetRed();
			*dp++ = sp.GetGreen();
			*dp++ = sp.GetBlue();
			sp.Increment();
		}
		buffer = TempRow;
	}
#if 0
	// this is now handled in G42Image::GetRow()
	if (Depth == 24)
	{
		if (TrueDepth == 1)
		{
			const byte * sp = row;
			byte * dp = TempRow;
			int m = 0x80;
			for (uint32 i = 0; i < Width; i++)
			{
				byte v = (byte)((*sp & m) ? 1 : 0);
				*dp++ = ImagePalette[v].red;
				*dp++ = ImagePalette[v].green;
				*dp++ = ImagePalette[v].blue;
				if (m > 1)
				{
					m >>= 1;
				}
				else
				{
					sp++;
					m = 0x80;
				}
			}
		}
		else if (TrueDepth == 4)
		{
			const byte * sp = row;
			byte * dp = TempRow;
			int s = 4;
			for (uint32 i = 0; i < Width; i++)
			{
				byte v = (byte)((*sp >> s) & 0xf);
				*dp++ = ImagePalette[v].red;
				*dp++ = ImagePalette[v].green;
				*dp++ = ImagePalette[v].blue;
				if (s)
				{
					s = 0;
				}
				else
				{
					sp++;
					s = 4;
				}
			}
		}
		else if (TrueDepth == 8)
		{
			const byte * sp = row;
			byte * dp = TempRow;
			for (uint32 i = 0; i < Width; i++)
			{
				byte v = *sp++;
				*dp++ = ImagePalette[v].red;
				*dp++ = ImagePalette[v].green;
				*dp++ = ImagePalette[v].blue;
			}
		}
		else
		{
			G42ConstByte24BitIterator sp(row);
			byte * dp = TempRow;
			for (uint32 i = 0; i < Width; i++)
			{
				*dp++ = sp.GetRed();
				*dp++ = sp.GetGreen();
				*dp++ = sp.GetBlue();
				sp.Increment();
			}
		}
		buffer = TempRow;
	}
	else // grayscale and monochrome
	{
		if (TrueDepth == 1)
		{
			const byte * sp = row;
			byte * dp = TempRow;
			int m = 0x80;
			for (uint32 i = 0; i < Width; i++)
			{
				if (*sp & m)
					*dp++ = 0xff;
				else
					*dp++ = 0;
				if (m > 1)
				{
					m >>= 1;
				}
				else
				{
					sp++;
					m = 0x80;
				}
			}
			buffer = TempRow;
		}
		else if (TrueDepth == 4)
		{
			const byte * sp = row;
			byte * dp = TempRow;
			int s = 4;
			for (uint32 i = 0; i < Width; i++)
			{
				byte v = (byte)((*sp >> s) & 0xf);
				*dp++ = (byte)((v << 4) | v);
				if (s)
				{
					s = 0;
				}
				else
				{
					sp++;
					s = 4;
				}
			}
			buffer = TempRow;
		}
	}
#endif
#if !defined(G42_USE_EXCEPTIONS)
	if (setjmp(LibJpegError.setjmp_buf))
	{
		Valid = false;
		return false;
	}
#else
	try {
#endif
	int s = jpeg_write_scanlines(&(LibJpeg.j), (byte **)&buffer, 1);
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
G42JpegWriter::WriteEnd(G42ImageInfo * info)
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
	return G42RasterWriterBase::WriteEnd(info);
}
void
G42JpegWriter::JpegOutputMessage(void)
{
}
void
G42JpegWriter::JpegEmitMessage(int)
{
}
void
G42JpegWriter::JpegErrorExit(void)
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
G42JpegWriter::InitDestination()
{
	LibJpeg.j.dest->next_output_byte = OutputBuffer;
	LibJpeg.j.dest->free_in_buffer = OutputBufferSize;
}
boolean
G42JpegWriter::EmptyOutputBuffer()
{
	if (LibJpeg.j.dest->free_in_buffer != OutputBufferSize)
		PutBytes(OutputBuffer, OutputBufferSize);
	LibJpeg.j.dest->next_output_byte = OutputBuffer;
	LibJpeg.j.dest->free_in_buffer = OutputBufferSize;
	return (TRUE);
}
void
G42JpegWriter::TermDestination()
{
	if (LibJpeg.j.dest->free_in_buffer != OutputBufferSize)
		PutBytes(OutputBuffer, OutputBufferSize - LibJpeg.j.dest->free_in_buffer);
	LibJpeg.j.dest->next_output_byte = OutputBuffer;
	LibJpeg.j.dest->free_in_buffer = OutputBufferSize;
}

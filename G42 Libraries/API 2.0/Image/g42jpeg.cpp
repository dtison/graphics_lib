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
#include "g42jpeg.h"
#include "g42idata.h"
#include "g42iter.h"
#ifdef G42JPEG_H
G42JpegImage::G42JpegImage(G42RasterImage & parent) :
	G42RasterBase(parent), ReadMode(ReadInitHeaderMode),
	CurrentRow(0), InputBufferSize(4096), InputBuffer(0),
	BytesToSkip(0), Progressive(false), NeedRefill(false)
{
	LibJpegSource.j.bytes_in_buffer = 0;
	LibJpegSource.j.next_input_byte = NULL;
	InputBuffer = new byte [InputBufferSize];
	Valid = true;
	LibJpegError.ptr = this;
	LibJpeg.ptr = this;
	LibJpegSource.ptr = this;
}
G42JpegImage::~G42JpegImage()
{
	delete[] InputBuffer;
#if !defined(G42_USE_EXCEPTIONS)
	if (setjmp(LibJpegError.setjmp_buf))
	{
		return;
	}
#else
	try {
#endif
	jpeg_destroy_decompress(&(LibJpeg.j));
#if defined(G42_USE_EXCEPTIONS)
	} catch (G42JpegError)
	{
	}
#endif
}
// count rows, and mark done when last row is output
void
G42JpegImage::OutputRow(void)
{
//	Parent.HasRow(RowBuffer);
	Parent.HasRow(RowBuffer, CurrentRow, 0, Width);
	CurrentRow++;
	RowPtr = RowBuffer;
	RowBytesLeft = RowBytes;
	if (CurrentRow >= Height)
	{
		if (Progressive)
		{
			ReadMode = ReadEndProgressivePassMode;
         CurrentRow = 0;
		}
		else
		{
			ReadMode = ReadDoneMode;
			ForceFileEnd();
		}
	}
}
void
G42JpegImage::ProcessData(byte * buffer, size_t buffer_size)
{
	RestoreBuffer(buffer, buffer_size);
	if (BytesToSkip >= BufferSize)
	{
		BytesToSkip -= BufferSize;
		return;
	}
	else if (BytesToSkip)
	{
		SkipData((int)BytesToSkip);
		BytesToSkip = 0;
	}
	NeedRefill = true;
	while ((BufferSize || !NeedRefill) && !Finished && Valid)
	{
		if (NeedRefill && Valid)
		{
			if (LibJpegSource.j.bytes_in_buffer)
			{
				int n = LibJpegSource.j.bytes_in_buffer;
				byte * sp = (byte *)LibJpegSource.j.next_input_byte;
				byte * dp = InputBuffer;
				while (n--)
				{
					*dp++ = *sp++;
				}
			}
			if (InputBufferSize <= LibJpegSource.j.bytes_in_buffer + 256)
			{
				byte * tmp = InputBuffer;
				int new_size = LibJpegSource.j.bytes_in_buffer + 256;
				InputBuffer = new byte [new_size];
				if (InputBuffer)
				{
					memcpy(InputBuffer, tmp, InputBufferSize);
					InputBufferSize = new_size;
					delete[] tmp;
				}
				else
				{
					Valid = false;
					EatBuffer();
					return;
				}
			}
			int min_size = min(BufferSize,
				InputBufferSize - LibJpegSource.j.bytes_in_buffer);
			FillBuffer(InputBuffer + LibJpegSource.j.bytes_in_buffer,
				min_size);
			LibJpegSource.j.next_input_byte = InputBuffer;
			LibJpegSource.j.bytes_in_buffer += min_size;
         NeedRefill = false;
		}
		switch (ReadMode)
		{
			case ReadHeaderMode:
				ReadHeader();
				break;
			case ReadInitHeaderMode:
				ReadInitHeader();
				break;
			case ReadStartImageMode:
				ReadStartImage();
				break;
			case ReadStartProgressiveImageMode:
				ReadStartProgressiveImage();
				break;
			case ReadStartProgressivePassMode:
				ReadStartProgressivePass();
				break;
			case ReadProgressivePassMode:
				ReadProgressivePass();
				break;
			case ReadEndProgressivePassMode:
				ReadEndProgressivePass();
				break;
			case ReadImageMode:
				ReadImage();
				break;
			default:
				EatBuffer();
				if (ReadMode != ReadDoneMode)
            	Valid = false;
				break;
		}
	}
}
void
G42JpegImage::ReadInitHeader(void)
{
	LibJpeg.j.err = jpeg_std_error(&(LibJpegError.j));
	LibJpegError.j.error_exit = g42_jpeg_error_exit;
	LibJpegError.j.output_message = g42_jpeg_output_message;
	LibJpegError.j.emit_message = g42_jpeg_emit_message;
	LibJpegError.ptr = this;
#if !defined(G42_USE_EXCEPTIONS)
	if (setjmp(LibJpegError.setjmp_buf))
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		EatBuffer();
		return;
	}
#else
	try {
#endif
	jpeg_create_decompress(&(LibJpeg.j));
#if defined(G42_USE_EXCEPTIONS)
	} catch (G42JpegError)
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		EatBuffer();
		return;
	}
#endif
	LibJpegSource.j.init_source = g42_jpeg_init_source;
	LibJpegSource.j.fill_input_buffer = g42_jpeg_fill_input_buffer;
	LibJpegSource.j.skip_input_data = g42_jpeg_skip_input_data;
	LibJpegSource.j.resync_to_restart = jpeg_resync_to_restart;
	LibJpegSource.j.term_source = g42_jpeg_term_source;
	LibJpeg.j.src = &(LibJpegSource.j);
	ReadMode = ReadHeaderMode;
}
void
G42JpegImage::ReadHeader(void)
{
#if !defined(G42_USE_EXCEPTIONS)
	if (setjmp(LibJpegError.setjmp_buf))
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		EatBuffer();
		return;
	}
#else
	try {
#endif
	int result = jpeg_read_header(&(LibJpeg.j), TRUE);
	if (result != JPEG_SUSPENDED)
	{
		Width = LibJpeg.j.image_width;
		Height = LibJpeg.j.image_height;
		TopLineFirst = true;
		NumPalette = 0;
		if (LibJpeg.j.num_components > 1)
		{
			Depth = 24;
			RowBytes = Width * 3;
			Color = true;
		}
		else
		{
			Depth = 8;
			RowBytes = Width;
			Color = false;
		}
		if (jpeg_has_multiple_scans(&(LibJpeg.j)))
		{
			ReadMode = ReadStartProgressiveImageMode;
			Progressive = true;
		}
		else
		{
			ReadMode = ReadStartImageMode;
			Progressive = false;
		}
		Info.Width = Width;
		Info.Height = Height;
		if (Depth > 8)
			Info.PixelDepth = 8 * TrueColorPixelSize;
		else
      	Info.PixelDepth = Depth;
		Info.Progressive = Progressive;
		if (Color)
			Info.ColorType = G42ImageInfo::ColorTypeTrueColor;
		else
			Info.ColorType = G42ImageInfo::ColorTypeGrayscale;
		Info.ImageType = Jpeg;
		Info.Compressed = true;
		Parent.HasInfo(Info);         // give user the file dimensions
	}
	else
	{
		NeedRefill = true;
	}
#if defined(G42_USE_EXCEPTIONS)
	} catch (G42JpegError)
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		EatBuffer();
		return;
	}
#endif
}
void
G42JpegImage::ReadStartImage(void)
{
#if !defined(G42_USE_EXCEPTIONS)
	if (setjmp(LibJpegError.setjmp_buf))
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		EatBuffer();
		return;
	}
#else
	try {
#endif
	if (jpeg_start_decompress(&(LibJpeg.j)))
	{
		ReadMode = ReadImageMode;
	}
	else
	{
		NeedRefill = true;
	}
#if defined(G42_USE_EXCEPTIONS)
	} catch (G42JpegError)
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		EatBuffer();
		return;
	}
#endif
}
void
G42JpegImage::ReadStartProgressiveImage(void)
{
	LibJpeg.j.buffered_image = TRUE;
#if !defined(G42_USE_EXCEPTIONS)
	if (setjmp(LibJpegError.setjmp_buf))
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		EatBuffer();
		return;
	}
#else
	try {
#endif
	if (jpeg_start_decompress(&(LibJpeg.j)))
	{
		ReadMode = ReadStartProgressivePassMode;
	}
	else
	{
		NeedRefill = true;
	}
#if defined(G42_USE_EXCEPTIONS)
	} catch (G42JpegError)
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		EatBuffer();
		return;
	}
#endif
}
void
G42JpegImage::ReadStartProgressivePass(void)
{
#if !defined(G42_USE_EXCEPTIONS)
	if (setjmp(LibJpegError.setjmp_buf))
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		EatBuffer();
		return;
	}
#else
	try {
#endif
	int ret;
	do
	{
		ret = jpeg_consume_input(&(LibJpeg.j));
	} while (ret != JPEG_REACHED_EOI && ret != JPEG_SUSPENDED);
	if (jpeg_start_output(&(LibJpeg.j), LibJpeg.j.input_scan_number))
	{
		ReadMode = ReadProgressivePassMode;
	}
	else
	{
		NeedRefill = true;
	}
#if defined(G42_USE_EXCEPTIONS)
	} catch (G42JpegError)
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		EatBuffer();
		return;
	}
#endif
}
void
G42JpegImage::ReadEndProgressivePass(void)
{
#if !defined(G42_USE_EXCEPTIONS)
	if (setjmp(LibJpegError.setjmp_buf))
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		EatBuffer();
		return;
	}
#else
	try {
#endif
	int ret;
	do
	{
		ret = jpeg_consume_input(&(LibJpeg.j));
	} while (ret != JPEG_REACHED_EOI && ret != JPEG_SUSPENDED);
	if (jpeg_finish_output(&(LibJpeg.j)))
	{
		if (jpeg_input_complete(&(LibJpeg.j)) &&
			LibJpeg.j.input_scan_number == LibJpeg.j.output_scan_number)
		{
			ForceFileEnd();
			ReadMode = ReadDoneMode;
		}
		else
		{
			ReadMode = ReadStartProgressivePassMode;
		}
	}
	else
	{
		NeedRefill = true;
	}
#if defined(G42_USE_EXCEPTIONS)
	} catch (G42JpegError)
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		EatBuffer();
		return;
	}
#endif
}
void
G42JpegImage::ReadProgressivePass(void)
{
#if !defined(G42_USE_EXCEPTIONS)
	if (setjmp(LibJpegError.setjmp_buf))
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		EatBuffer();
		return;
	}
#else
	try {
#endif
	int ret;
	do
	{
		ret = jpeg_consume_input(&(LibJpeg.j));
	} while (ret != JPEG_REACHED_EOI && ret != JPEG_SUSPENDED);
	if (!RowBuffer)
		InitRow();
	while (ReadMode == ReadProgressivePassMode)
	{
		int rows_read = jpeg_read_scanlines(&(LibJpeg.j), &RowBuffer, 1);
		if (!rows_read)
		{
			NeedRefill = true;
			break;
		}
		ProcessRow(RowBuffer);
	}
#if defined(G42_USE_EXCEPTIONS)
	} catch (G42JpegError)
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		EatBuffer();
		return;
	}
#endif
}
void
G42JpegImage::ReadImage(void)
{
#if !defined(G42_USE_EXCEPTIONS)
	if (setjmp(LibJpegError.setjmp_buf))
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		EatBuffer();
		return;
	}
#else
	try {
#endif
	if (!RowBuffer)
		InitRow();
	while (!Finished)
	{
		int rows_read = jpeg_read_scanlines(&(LibJpeg.j), &RowBuffer, 1);
		if (!rows_read)
		{
			NeedRefill = true;
			break;
		}
		ProcessRow(RowBuffer);
	}
#if defined(G42_USE_EXCEPTIONS)
	} catch (G42JpegError)
	{
		Valid = false;
		ReadMode = ReadErrorMode;
		EatBuffer();
		return;
	}
#endif
}
void
G42JpegImage::ProcessRow(byte * row)
{
	if (Depth > 8)
	{
		byte * sp = row + (Width - 1) * 3;
		G42Byte24BitIterator dp(row);
		dp.Forward(Width - 1);
		uint i = 0;
		while (true)
		{
			dp.Set(*(sp + 2), *(sp + 1), *sp);
			i++;
			if (i == Width)
				break;
			dp.Decrement();
			sp -= 3;
		}
	}
	PutSequence(row, RowBytes);
}
void
G42JpegImage::InitSource(void)
{
}
boolean
G42JpegImage::FillInputBuffer(void)
{
	return FALSE;
}
void
G42JpegImage::SkipInputData(long num_bytes)
{
	if (LibJpegSource.j.bytes_in_buffer >= num_bytes)
	{
		LibJpegSource.j.bytes_in_buffer -= (int)num_bytes;
		LibJpegSource.j.next_input_byte += (int)num_bytes;
	}
	else
	{
		BytesToSkip = num_bytes - LibJpegSource.j.bytes_in_buffer;
		LibJpegSource.j.bytes_in_buffer = 0;
	}
}
void
G42JpegImage::TermSource(void)
{
}
void
G42JpegImage::JpegOutputMessage(void)
{
}
void
G42JpegImage::JpegEmitMessage(int)
{
}
void
G42JpegImage::JpegErrorExit(void)
{
#if !defined(G42_USE_EXCEPTIONS)
	longjmp(LibJpegError.setjmp_buf, 1);
#else
	throw G42JpegImage::G42JpegError();
#endif
}
// key for Jpeg files is 0xff and 0xd8 in the first two characters
bool G42IsJpeg(byte * buffer, size_t buffer_size)
{
	if ((buffer[0] == 0xff) &&
		(buffer_size < 1 || buffer[1] == 0xd8))
		return true;
	return false;
}
extern "C" void g42_jpeg_init_source(j_decompress_ptr cinfo)
{
	g42_jpeg_decompress_struct * j = (g42_jpeg_decompress_struct *)cinfo;
	j->ptr->InitSource();
}
extern "C" boolean g42_jpeg_fill_input_buffer(j_decompress_ptr cinfo)
{
	g42_jpeg_decompress_struct * j = (g42_jpeg_decompress_struct *)cinfo;
	return j->ptr->FillInputBuffer();
}
extern "C" void g42_jpeg_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
	g42_jpeg_decompress_struct * j = (g42_jpeg_decompress_struct *)cinfo;
	j->ptr->SkipInputData(num_bytes);
}
extern "C" void g42_jpeg_term_source(j_decompress_ptr cinfo)
{
	g42_jpeg_decompress_struct * j = (g42_jpeg_decompress_struct *)cinfo;
	j->ptr->TermSource();
}
extern "C" void g42_jpeg_output_message(j_common_ptr cinfo)
{
	g42_jpeg_error_mgr * j = (g42_jpeg_error_mgr *)(cinfo->err);
	j->ptr->JpegOutputMessage();
}
extern "C" void g42_jpeg_emit_message(j_common_ptr cinfo, int msg_level)
{
	g42_jpeg_error_mgr * j = (g42_jpeg_error_mgr *)(cinfo->err);
	j->ptr->JpegEmitMessage(msg_level);
}
extern "C" void g42_jpeg_error_exit(j_common_ptr cinfo)
{
	g42_jpeg_error_mgr * j = (g42_jpeg_error_mgr *)(cinfo->err);
	j->ptr->JpegErrorExit();
}
#endif // G42JPEG_H

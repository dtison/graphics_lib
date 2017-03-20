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
#include "girjpeg.h"
G42ImageReaderJpeg::G42ImageReaderJpeg(void)
:
	G42ImageReaderBase(),
	ReadMode(ReadInitHeaderMode),
	InputBufferSize(4096), InputBuffer(0),
	BytesToSkip(0), NeedRefill(false), JpegCreated(false)
{
/* don't need any of this, as it won't really be called upon to read
	LibJpegSource.j.bytes_in_buffer = 0;
	LibJpegSource.j.next_input_byte = NULL;
//	InputBuffer = new byte [InputBufferSize];
//	G42ThrowIfNull(InputBuffer);
	LibJpegError.ptr = this;
	LibJpeg.ptr = this;
	LibJpegSource.ptr = this;
*/
	SetInfo(G42JpegTypeId, "jpg;jpe;jpeg", "Jpeg", "\377\330", 2); // 0xff, 0xd8
}
G42ImageReaderJpeg::G42ImageReaderJpeg(G42HeapManager & heap,
	G42Notifier & notifier, G42File * file, G42BaseOptions * options)
:
	G42ImageReaderBase(heap, notifier, file, options),
	ReadMode(ReadInitHeaderMode),
	InputBufferSize(4096), InputBuffer(0),
	BytesToSkip(0), NeedRefill(false), JpegCreated(false)
{
	LibJpegSource.j.bytes_in_buffer = 0;
	LibJpegSource.j.next_input_byte = NULL;
//	InputBuffer = new byte [InputBufferSize];
//	G42ThrowIfNull(InputBuffer);
	LibJpegError.ptr = this;
	LibJpeg.ptr = this;
	LibJpegSource.ptr = this;
	SetInfo(G42JpegTypeId, "jpg;jpe;jpeg", "Jpeg", "\377\330", 2); // 0xff, 0xd8
}
G42ImageReaderJpeg::~G42ImageReaderJpeg()
{
	if (InputBuffer)
	{
		delete[] InputBuffer;
		InputBuffer = 0;
	}
	if (JpegCreated)
	{
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
}
// count rows, and mark done when last row is output
void
G42ImageReaderJpeg::ConvertRow(void)
{
	if (Depth > 8)
	{
		if (GetImageDepth() == 24)
		{
			byte * sp = RowBuffer;
			G42Color24Iterator dp(OutputRowPtr);
			for (uintf i = 0; i < Width; i++)
			{
				*dp = G42Color(*sp, *(sp + 1), *(sp + 2));
				dp++;
				sp += 3;
			}
		}
		else
		{
			byte * sp = RowBuffer + (Width - 1) * 3;
			G42Color32Iterator dp(OutputRowPtr);
			dp += Width - 1;
			for (uintf i = 0; i < Width; i++)
			{
				*dp = G42Color(*sp, *(sp + 1), *(sp + 2));
				dp--;
				sp -= 3;
			}
		}
	}
	G42ImageReaderBase::ConvertRow();
}
void
G42ImageReaderJpeg::EndPass(void)
{
	if (Progressive)
	{
		ReadMode = ReadEndProgressivePassMode;
	}
	else
	{
		ReadMode = ReadDoneMode;
	}
	G42ImageReaderBase::EndPass();
}
void
G42ImageReaderJpeg::ProcessStreamData(void)
{
	if (BytesToSkip >= BufferSize)
	{
		BytesToSkip -= BufferSize;
		EatBuffer();
		return;
	}
	else if (BytesToSkip)
	{
		SkipData((int)BytesToSkip);
		BytesToSkip = 0;
	}
//	NeedRefill = true;
	LibJpegSource.j.next_input_byte = CurrentBuffer;
	LibJpegSource.j.bytes_in_buffer = BufferLeft;
	NeedRefill = false;
	while (!NeedRefill && !Finished)
	{
/* this is taken care of by the buffer in G42Reader
		if (NeedRefill)
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
				G42ThrowIfNull(InputBuffer);
				memcpy(InputBuffer, tmp, InputBufferSize);
				InputBufferSize = new_size;
				delete[] tmp;
			}
			int min_size = min(BufferSize,
				InputBufferSize - LibJpegSource.j.bytes_in_buffer);
			FillBuffer(InputBuffer + LibJpegSource.j.bytes_in_buffer,
				min_size);
			LibJpegSource.j.next_input_byte = InputBuffer;
			LibJpegSource.j.bytes_in_buffer += min_size;
			NeedRefill = false;
		}
*/
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
				break;
		}
	}
	if (NextByte == CurrentBuffer) // if EatBuffer has not been called
	{
		SkipData((uintf)(BufferLeft - LibJpegSource.j.bytes_in_buffer));
	}
}
void
G42ImageReaderJpeg::BeginProcessing(void)
{
	while (ReadMode != ReadDoneMode && !Finished)
	{
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
				ReadMode = ReadDoneMode;
				break;
		}
	}
}
void
G42ImageReaderJpeg::SendExtendedInfo(void)
{
	while (ReadMode != ReadDoneMode && !Finished)
	{
		switch (ReadMode)
		{
			case ReadHeaderMode:
				ReadHeader();
				break;
			case ReadInitHeaderMode:
				ReadInitHeader();
				break;
/* not sure this is correct
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
*/				
			default:
				ReadMode = ReadDoneMode;
				break;
		}
	}
}
void
G42ImageReaderJpeg::ReadInitHeader(void)
{
	LibJpeg.j.err = jpeg_std_error(&(LibJpegError.j));
	LibJpegError.j.error_exit = g42_jpeg_error_exit;
	LibJpegError.j.output_message = g42_jpeg_output_message;
	LibJpegError.j.emit_message = g42_jpeg_emit_message;
	LibJpegError.ptr = this;
#if !defined(G42_USE_EXCEPTIONS)
	if (setjmp(LibJpegError.setjmp_buf))
	{
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
		ReadMode = ReadErrorMode;
		EatBuffer();
		return;
	}
#endif
	JpegCreated = true;
	LibJpegSource.j.init_source = g42_jpeg_init_source;
	LibJpegSource.j.fill_input_buffer = g42_jpeg_fill_input_buffer;
	LibJpegSource.j.skip_input_data = g42_jpeg_skip_input_data;
	LibJpegSource.j.resync_to_restart = jpeg_resync_to_restart;
	LibJpegSource.j.term_source = g42_jpeg_term_source;
	LibJpeg.j.src = &(LibJpegSource.j);
	ReadMode = ReadHeaderMode;
}
void
G42ImageReaderJpeg::ReadHeader(void)
{
#if !defined(G42_USE_EXCEPTIONS)
	if (setjmp(LibJpegError.setjmp_buf))
	{
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
			NumPasses = 0; // unknown number of passes
		}
		else
		{
			ReadMode = ReadStartImageMode;
			Progressive = false;
		}
		Compressed = true;
		SetupDescriptor();
		SendDescriptor();
	}
	else
	{
		NeedRefill = true;
	}
#if defined(G42_USE_EXCEPTIONS)
	} catch (G42JpegError)
	{
		ReadMode = ReadErrorMode;
		EatBuffer();
		return;
	}
#endif
}
void
G42ImageReaderJpeg::ReadStartImage(void)
{
#if !defined(G42_USE_EXCEPTIONS)
	if (setjmp(LibJpegError.setjmp_buf))
	{
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
		ReadMode = ReadErrorMode;
		EatBuffer();
		return;
	}
#endif
}
void
G42ImageReaderJpeg::ReadStartProgressiveImage(void)
{
	LibJpeg.j.buffered_image = TRUE;
#if !defined(G42_USE_EXCEPTIONS)
	if (setjmp(LibJpegError.setjmp_buf))
	{
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
		ReadMode = ReadErrorMode;
		EatBuffer();
		return;
	}
#endif
}
void
G42ImageReaderJpeg::ReadStartProgressivePass(void)
{
#if !defined(G42_USE_EXCEPTIONS)
	if (setjmp(LibJpegError.setjmp_buf))
	{
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
		ReadMode = ReadErrorMode;
		EatBuffer();
		return;
	}
#endif
}
void
G42ImageReaderJpeg::ReadEndProgressivePass(void)
{
#if !defined(G42_USE_EXCEPTIONS)
	if (setjmp(LibJpegError.setjmp_buf))
	{
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
		ReadMode = ReadErrorMode;
		EatBuffer();
		return;
	}
#endif
}
void
G42ImageReaderJpeg::ReadProgressivePass(void)
{
#if !defined(G42_USE_EXCEPTIONS)
	if (setjmp(LibJpegError.setjmp_buf))
	{
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
	if (!RowPtr)
	{
		InitRow();
	}
	while (ReadMode == ReadProgressivePassMode && !Finished)
	{
		int rows_read = jpeg_read_scanlines(&(LibJpeg.j), &RowBuffer, 1);
		if (!rows_read)
		{
			NeedRefill = true;
			break;
		}
		OutputRow();
	}
	if (Finished)
	{
		ReadMode = ReadEndProgressivePassMode;
	}
#if defined(G42_USE_EXCEPTIONS)
	} catch (G42JpegError)
	{
		ReadMode = ReadErrorMode;
		EatBuffer();
		return;
	}
#endif
}
void
G42ImageReaderJpeg::ReadImage(void)
{
#if !defined(G42_USE_EXCEPTIONS)
	if (setjmp(LibJpegError.setjmp_buf))
	{
		ReadMode = ReadErrorMode;
		EatBuffer();
		return;
	}
#else
	try {
#endif
	if (!RowPtr)
	{
		InitRow();
	}
	while (!Finished)
	{
		int rows_read = jpeg_read_scanlines(&(LibJpeg.j), &RowBuffer, 1);
		if (!rows_read)
		{
			NeedRefill = true;
			break;
		}
		OutputRow();
	}
#if defined(G42_USE_EXCEPTIONS)
	} catch (G42JpegError)
	{
		ReadMode = ReadErrorMode;
		EatBuffer();
		return;
	}
#endif
}
void
G42ImageReaderJpeg::InitSource(void)
{
	if (File)
	{
		InputBuffer = new byte [InputBufferSize];
		G42ThrowIfNull(InputBuffer);
	}
}
boolean
G42ImageReaderJpeg::FillInputBuffer(void)
{
	if (!File) // suspend
	{
		NeedRefill = true;
		return FALSE;
	}
	if (BytesToSkip)
	{
		File->SetPosition(BytesToSkip, G42File::FromCurrent);
		BytesToSkip = 0;
	}
	
	uintf min_size = File->Read(InputBuffer, InputBufferSize, false);
	LibJpegSource.j.next_input_byte = InputBuffer;
	LibJpegSource.j.bytes_in_buffer = min_size;
	return TRUE;
}
void
G42ImageReaderJpeg::SkipInputData(long num_bytes)
{
	if (LibJpegSource.j.bytes_in_buffer >= num_bytes)
	{
		LibJpegSource.j.bytes_in_buffer -= (int)num_bytes;
		LibJpegSource.j.next_input_byte += (int)num_bytes;
	}
	else
	{
		BytesToSkip = num_bytes - LibJpegSource.j.bytes_in_buffer;
		LibJpegSource.j.next_input_byte += LibJpegSource.j.bytes_in_buffer;
		LibJpegSource.j.bytes_in_buffer = 0;
	}
}
void
G42ImageReaderJpeg::TermSource(void)
{
}
void
G42ImageReaderJpeg::JpegOutputMessage(void)
{
}
void
G42ImageReaderJpeg::JpegEmitMessage(int)
{
}
void
G42ImageReaderJpeg::JpegErrorExit(void)
{
#if !defined(G42_USE_EXCEPTIONS)
	longjmp(LibJpegError.setjmp_buf, 1);
#else
	throw G42ImageReaderJpeg::G42JpegError();
#endif
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

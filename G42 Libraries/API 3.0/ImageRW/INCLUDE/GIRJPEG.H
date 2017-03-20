// girjpeg.h - jpeg version of the raster file base class
#ifndef GIRJPEG_H
#define GIRJPEG_H
/*************************************************************
	File:          girjpeg.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for Jpeg's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "girbase.h"
#include "gitjpeg.h"
extern "C" {
#include <jpeglib.h>
}
#if !defined(G42_USE_EXCEPTIONS)
#include <setjmp.h>
#endif
class G42ImageReaderJpeg;
struct g42_jpeg_decompress_struct
{
	jpeg_decompress_struct j;
	G42ImageReaderJpeg * ptr;
};
struct g42_jpeg_error_mgr
{
	struct jpeg_error_mgr j;
#if !defined(G42_USE_EXCEPTIONS)
	jmp_buf setjmp_buf;
#endif
	G42ImageReaderJpeg * ptr;
};
struct g42_jpeg_source_mgr
{
	jpeg_source_mgr j;
	G42ImageReaderJpeg * ptr;
};
extern "C" void g42_jpeg_init_source(j_decompress_ptr cinfo);
extern "C" boolean g42_jpeg_fill_input_buffer(j_decompress_ptr cinfo);
extern "C" void g42_jpeg_skip_input_data(j_decompress_ptr cinfo, long num_bytes);
extern "C" void g42_jpeg_term_source(j_decompress_ptr cinfo);
extern "C" void g42_jpeg_output_message(j_common_ptr cinfo);
extern "C" void g42_jpeg_emit_message(j_common_ptr cinfo, int msg_level);
extern "C" void g42_jpeg_error_exit(j_common_ptr cinfo);
class G42ImageReaderJpeg : public G42ImageReaderBase
{
	public:
		G42ImageReaderJpeg(void);
		G42ImageReaderJpeg(G42HeapManager & heap, G42Notifier & notifier,
			G42File * file = 0, G42BaseOptions * options = 0);
		virtual ~G42ImageReaderJpeg();
		virtual void ProcessStreamData(void);
		virtual void BeginProcessing(void);
		virtual void SendExtendedInfo(void);
		void InitSource();
		boolean FillInputBuffer();
		void SkipInputData(long num_bytes);
		void TermSource();
		void JpegOutputMessage();
		void JpegEmitMessage(int msg_level);
		void JpegErrorExit();
#if defined(G42_USE_EXCEPTIONS)
		class G42JpegError
		{
			int unused;
		};
#endif
	protected:
		byte * InputBuffer;
		int InputBufferSize;
		long BytesToSkip;
//		bool Progressive;
		bool NeedRefill;
		bool JpegCreated;
		g42_jpeg_decompress_struct LibJpeg;
		g42_jpeg_error_mgr LibJpegError;
		g42_jpeg_source_mgr LibJpegSource;
		enum G42ReadMode {ReadHeaderMode, ReadInitHeaderMode, ReadImageMode,
			ReadStartImageMode, ReadStartProgressiveImageMode,
			ReadStartProgressivePassMode, ReadProgressivePassMode,
			ReadEndProgressivePassMode,
			ReadDoneMode, ReadErrorMode};
		G42ReadMode ReadMode;
		void ReadHeader(void);
		void ReadInitHeader(void);
		void ReadImage(void);
		void ReadStartImage(void);
		void ReadStartProgressiveImage(void);
		void ReadStartProgressivePass(void);
		void ReadProgressivePass(void);
		void ReadEndProgressivePass(void);
		void ProcessRow(byte * row);
		virtual void ConvertRow(void);
		virtual void EndPass(void);
};
#endif // G42JPEG_H

// g42jpeg.h - jpeg version of the raster file base class
#ifndef G42WJPEG_H
#define G42WJPEG_H
/*************************************************************
	File:          g42jpeg.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for Jpeg's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
      Code   Date   Name and Description
***************************************************************/
#include "g42wibas.h"
extern "C" {
#include <jpeglib.h>
}
#include <setjmp.h>
class G42JpegWriter;
struct g42_jpeg_compress_struct
{
	jpeg_compress_struct j;
	G42JpegWriter * ptr;
};
struct g42_jpeg_write_error_mgr
{
	struct jpeg_error_mgr j;
#if !defined(G42_USE_EXCEPTIONS)
	jmp_buf setjmp_buf;
#endif
	G42JpegWriter * ptr;
};
struct g42_jpeg_dest_mgr
{
	jpeg_destination_mgr j;
	G42JpegWriter * ptr;
};
extern "C" void g42_jpeg_init_destination(j_compress_ptr cinfo);
extern "C" boolean g42_jpeg_empty_output_buffer(j_compress_ptr cinfo);
extern "C" void g42_jpeg_term_destination(j_compress_ptr cinfo);
extern "C" void g42_jpeg_write_output_message(j_common_ptr cinfo);
extern "C" void g42_jpeg_write_emit_message(j_common_ptr cinfo, int msg_level);
extern "C" void g42_jpeg_write_error_exit(j_common_ptr cinfo);
class G42JpegWriter : G42RasterWriterBase
{
	public:
		G42JpegWriter(G42RasterImageWriter & parent);
		virtual ~G42JpegWriter();
		bool WriteInfo(G42ImageInfo * info);
		bool WriteRow(const byte * row, const byte * mask);
		bool WriteEnd(G42ImageInfo * info);
		void InitDestination();
		boolean EmptyOutputBuffer();
		void TermDestination();
		void JpegOutputMessage();
		void JpegEmitMessage(int msg_level);
		void JpegErrorExit();
#if defined(G42_USE_EXCEPTIONS)
		class G42JpegError
		{
			int unused;
		};
#endif
	private:
		g42_jpeg_compress_struct LibJpeg;
		g42_jpeg_write_error_mgr LibJpegError;
		g42_jpeg_dest_mgr LibJpegDest;
		bool Progressive;
		byte * TempRow;
		byte * OutputBuffer;
		int OutputBufferSize;
		G42ImageInfo::G42ColorType ColorType;
      const G42Color * ImagePalette; // reference only, do not delete
		uint NumPalette;
      uint TrueDepth;
};
#endif // G42JPEG_H

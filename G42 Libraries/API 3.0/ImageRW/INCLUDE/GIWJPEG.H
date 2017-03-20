// giwjpeg.h - jpeg version of the raster file base class
#ifndef GIWJPEG_H
#define GIWJPEG_H
/*************************************************************
	File:          giwjpeg.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for Jpeg's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
      Code   Date   Name and Description
***************************************************************/
#include "giwbase.h"
#include "gitjpeg.h"
extern "C" {
#include <jpeglib.h>
}
#if !defined(G42_USE_EXCEPTIONS)
#include <setjmp.h>
#endif
class G42ImageWriterJpeg;
struct g42_jpeg_compress_struct
{
	jpeg_compress_struct j;
	G42ImageWriterJpeg * ptr;
};
struct g42_jpeg_write_error_mgr
{
	struct jpeg_error_mgr j;
#if !defined(G42_USE_EXCEPTIONS)
	jmp_buf setjmp_buf;
#endif
	G42ImageWriterJpeg * ptr;
};
struct g42_jpeg_dest_mgr
{
	jpeg_destination_mgr j;
	G42ImageWriterJpeg * ptr;
};
extern "C" void g42_jpeg_init_destination(j_compress_ptr cinfo);
extern "C" boolean g42_jpeg_empty_output_buffer(j_compress_ptr cinfo);
extern "C" void g42_jpeg_term_destination(j_compress_ptr cinfo);
extern "C" void g42_jpeg_write_output_message(j_common_ptr cinfo);
extern "C" void g42_jpeg_write_emit_message(j_common_ptr cinfo, int msg_level);
extern "C" void g42_jpeg_write_error_exit(j_common_ptr cinfo);
class G42ImageWriterJpeg : public G42ImageWriterBase
{
	public:
		G42ImageWriterJpeg(void);
		G42ImageWriterJpeg(G42Notifier & notifier,
			G42BaseOptions * options = 0);
		virtual ~G42ImageWriterJpeg(void);
		bool WriteInfo(G42ImageDescriptor * info);
		bool WriteRow(const byte * row, const byte * mask);
		bool WriteEnd(void);
		void InitDestination();
		boolean EmptyOutputBuffer();
		void TermDestination();
		void JpegOutputMessage();
		void JpegEmitMessage(intf msg_level);
		void JpegErrorExit();
#if defined(G42_USE_EXCEPTIONS)
		class G42JpegError
		{
			intf unused;
		};
#endif
	private:
		g42_jpeg_compress_struct LibJpeg;
		g42_jpeg_write_error_mgr LibJpegError;
		g42_jpeg_dest_mgr LibJpegDest;
		bool Progressive;
		byte Quality;
		bool Photo;
		byte * TempRow;
		byte * OutputBuffer;
		intf OutputBufferSize;
      const G42Color * ImagePalette; // reference only, do not delete
		uintf NumPalette;
		uintf TrueDepth;
		bool NeedDestroy;
};
#endif // GIWJPEG_H

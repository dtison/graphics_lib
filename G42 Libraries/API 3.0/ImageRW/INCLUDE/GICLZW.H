// giclzw.h - lzw decompresser
/*************************************************************
	File:          g42lzw.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   Decompresses lzw streams
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#ifndef GICLZW_H
#define GICLZW_H
#include "gicomp.h"
#define LZW_NO_CODE -1
class G42LzwCompressor : public G42Compressor
{
	public:
		enum G42LzwType { LzwGif, LzwTiff }; // leaving as Gif and Tiff uses image types
		G42LzwCompressor(G42CompressorOutput & output,
			G42LzwType type = LzwTiff, intf code_depth = 8,
			uintf output_buffer_size = 4096);
		virtual ~G42LzwCompressor();
		virtual void ProcessBuffer(byte * buffer, uintf buffer_size);
		virtual void Flush(void);
		intf GetCodeDepth(void) const
			{ return min_code_size; }
	private:
		void init_table(intf min_code_size);
		void flush(intf n);
		void write_code(intf code);
		G42LzwType Type; // lzwtype;
		enum { largest_code = 4095 };		/* largest possible code */
		enum { table_size = 5003 }; /* table dimensions */
		byte code_buffer[259];		/* where the codes go */
		intf oldcode[table_size];	/* the table */
		intf currentcode[table_size];
		byte newcode[table_size];
		intf code_size;
		intf clear_code;
		intf eof_code;
		intf bit_offset;
		intf byte_offset;
		intf bits_left;
		intf max_code;
		intf free_code;
		intf prefix_code;
		intf suffix_char;
		intf hx, d;
		uintf min_code_size;
		bool firstChar;
		byte *lzw_clip_buf;
		byte *lzw_clip_ptr;
		intf lzw_clip_size;
		intf lzw_clip_nbuf;
};
#endif // GICLZW_H

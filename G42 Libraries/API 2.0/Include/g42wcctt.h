// g42lzw.h - lzw decompresser
/*************************************************************
	File:          g42wcctt.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   Decompresses lzw streams
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#ifndef G42WCCTT_H
#define G42WCCTT_H
#include "g42wcomp.h"
class G42CcittOutput : public G42WriteCompression
{
	public:
		G42CcittOutput(G42WriteCompressionOutput * output,
			int compression_type, uint width,
			uint output_buffer_size = 4096);
		virtual ~G42CcittOutput();
		virtual void ProcessBuffer(byte * buffer, uint buffer_size);
		virtual void Flush(void);
	private:
		int CompressionType;
		int Width;
		int RowBytes;
		int write_buf;
		int write_mask;
		int current_bit;
		int current_count;
		int cur_loc;
		bool in_horiz_mode;
		char * write_ref_buf;
		char * next_buf;
		int write_ref_size;
		int write_ref_loc;
		int next_loc;
		void start_write_ccitt_code(void);
		void write_ccitt_code(int bit, int count);
		void write_ccitt_3d_code(int count);
		void write_ccitt_eol(void);
		void end_write_ccitt_char(void);
		void write_ccitt_count(int current_count, int num_bits,
			int compression);
		void init_ccitt(int compression, int width);
		void start_ccitt_row(int num_bits, int compression);
		void write_ccitt_char(int buf, int num_bits, int compression,
			int width);
		void finish_ccitt_row(int compression, int width);
		void finish_ccitt(int compression);
		int get_ref_count(int cur, long max, char * buf, int pixel);
};
#endif // G42WCCTT_H

// gicccitt.h - ccitt compresser
/*************************************************************
	File:          gicccitt.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   Decompresses lzw streams
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#ifndef GICCCITT_H
#define GICCCITT_H
#include "gicomp.h"
class G42CcittCompressor : public G42Compressor
{
	public:
		G42CcittCompressor(G42CompressorOutput & output,
			intf compression_type, uintf width,
			uintf output_buffer_size = 4096);
		virtual ~G42CcittCompressor();
		virtual void ProcessBuffer(byte * buffer, uintf buffer_size);
		virtual void Flush(void);
	private:
		intf CompressionType;
		intf Width;
		intf RowBytes;
		intf write_buf;
		intf write_mask;
		intf current_bit;
		intf current_count;
		intf cur_loc;
		bool in_horiz_mode;
		char * write_ref_buf;
		char * next_buf;
		intf write_ref_size;
		intf write_ref_loc;
		intf next_loc;
		void start_write_ccitt_code(void);
		void write_ccitt_code(intf bit, intf count);
		void write_ccitt_3d_code(intf count);
		void write_ccitt_eol(void);
		void end_write_ccitt_char(void);
		void write_ccitt_count(intf current_count, intf num_bits,
			intf compression);
		void init_ccitt(intf compression, intf width);
		void start_ccitt_row(intf num_bits, intf compression);
		void write_ccitt_char(intf buf, intf num_bits, intf compression,
			intf width);
		void finish_ccitt_row(intf compression, intf width);
		void finish_ccitt(intf compression);
		intf get_ref_count(intf cur, long max, char * buf, intf pixel);
};
#endif // GICCCITT_H

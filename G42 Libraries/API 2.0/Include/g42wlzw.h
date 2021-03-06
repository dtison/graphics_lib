// g42lzw.h - lzw decompresser
/*************************************************************
	File:          g42lzw.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   Decompresses lzw streams
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#ifndef G42WLZW_H
#define G42WLZW_H
#include "g42wcomp.h"
#define LZW_NO_CODE -1
class G42LzwOutput : public G42WriteCompression
{
	public:
		enum G42LzwType { LzwGif, LzwTiff }; // leaving as Gif and Tiff uses image types
		G42LzwOutput(G42WriteCompressionOutput * output,
			G42LzwType type = LzwTiff, int code_depth = 8,
			uint output_buffer_size = 4096);
		virtual ~G42LzwOutput();
		virtual void ProcessBuffer(byte * buffer, uint buffer_size);
		virtual void Flush(void);
	private:
		void init_table(int min_code_size);
		void flush(int n);
		void write_code(int code);
		G42LzwType Type; // lzwtype;
		enum { largest_code = 4095 };		/* largest possible code */
		enum { table_size = 5003 }; /* table dimensions */
		byte code_buffer[259];		/* where the codes go */
		int oldcode[table_size];	/* the table */
		int currentcode[table_size];
		byte newcode[table_size];
		int code_size;
		int clear_code;
		int eof_code;
		int bit_offset;
		int byte_offset;
		int bits_left;
		int max_code;
		int free_code;
		int prefix_code;
		int suffix_char;
		int hx, d;
		uint min_code_size;
		bool firstChar;
		byte *lzw_clip_buf;
		byte *lzw_clip_ptr;
		int lzw_clip_size;
		int lzw_clip_nbuf;
};
#if 0
class G42LzwWrite
{
	public:
		enum G42LzwType {LzwGif, LzwTiff};
		G42LzwWrite(FILE * fp, G42LzwType type = LzwTiff, int user_code_size = 8);
		virtual ~G42LzwWrite();
		void Write(byte * buffer, uint length);
	protected:
		void init_table(int min_code_size);
		void flush(int n);
		void write_code(int code);
		FILE * Fp;
		G42LzwType Type; // lzwtype;
		enum { largest_code = 4095 };		/* largest possible code */
		enum { table_size = 5003 }; /* table dimensions */
		byte code_buffer[259];		/* where the codes go */
		int oldcode[table_size];	/* the table */
		int currentcode[table_size];
		byte newcode[table_size];
		int code_size;
		int clear_code;
		int eof_code;
		int bit_offset;
		int byte_offset;
		int bits_left;
		int max_code;
		int free_code;
		int prefix_code;
		int suffix_char;
		int hx, d;
		uint min_code_size;
		bool firstChar;
		byte *lzw_clip_buf;
		byte *lzw_clip_ptr;
		int lzw_clip_size;
		int lzw_clip_nbuf;
};
inline void
G42LzwWrite::write_code(int code)
{
	long temp;
	int i;
	byte_offset = bit_offset >> 3;
	bits_left = bit_offset & 7;
	if(byte_offset >= 252)
	{
		flush(byte_offset);
		code_buffer[0] = code_buffer[byte_offset];
		bit_offset = bits_left;
		byte_offset = 0;
	}
	if (Type == LzwGif)
	{
		if (bits_left)
			code_buffer[byte_offset] |= (code << bits_left) & 255;
		else
			code_buffer[byte_offset] = code & 255;
		bits_left = code_size - (8 - bits_left);
		while (bits_left > 0)
		{
			byte_offset++;
			code_buffer[byte_offset] = (code >> (code_size -
				bits_left)) & 255;
			bits_left -= 8;
		}
	}
	else
	{
		if (bits_left)
		{
			code_buffer[byte_offset++] |=
				code >> (bits_left + code_size - 8);
			bits_left = bits_left + code_size - 8;
		}
		else
			bits_left = code_size;
		if (bits_left > 8)
		{
			code_buffer[byte_offset++] =
				(code >> (bits_left - 8)) & 0xff;
			bits_left -= 8;
		}
		code_buffer[byte_offset] = (code << (8 - bits_left)) & 0xff;
	}
	bit_offset += code_size;
}
inline void
G42LzwWrite::flush(int n)
{
	byte b;
	if (!n)
		return;
	if (Type == LzwTiff)
	{
		fwrite(code_buffer,1,n,Fp);
	}
	else if (Type == LzwGif)
	{
		b = (byte)n;
		fwrite(&b,1,1,Fp);
		fwrite(code_buffer,1,n,Fp);
	}
}
#endif
#endif // G42LZW_H

// g42lzw.cpp - Lzw decompressor
/*************************************************************
	File:          g42lzw.cpp
   Copyright (c) 1996, Group 42, Inc.
	Description:   Decompresses lzw streams
   Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
   Modification History:
      Code   Date   Name and Description
***************************************************************/
#include "g42itype.h"
#include "g42wlzw.h"
G42LzwOutput::G42LzwOutput(G42WriteCompressionOutput * output, G42LzwType type,
	int user_code_size, uint output_buffer_size)
:
	G42WriteCompression(output, output_buffer_size),
	lzw_clip_buf(0), lzw_clip_ptr(0), lzw_clip_size(0), lzw_clip_nbuf(0),
	firstChar(false), Type(type), code_size(user_code_size)
{
	if (Type == LzwGif)
	{
		min_code_size = max(2, user_code_size);
		/* initialize the encoder */
		bit_offset = 0;
		init_table(min_code_size);
		write_code(clear_code);
		firstChar = true;
	}
	else
	{
		/* initialize the encoder */
		min_code_size = 8;
		bit_offset = 0;
		init_table(min_code_size);
		write_code(clear_code);
		firstChar = true;
	}
}
void
G42LzwOutput::init_table(int min_code_size)
{
	code_size = min_code_size + 1;
	clear_code = (1 << min_code_size);
	eof_code = clear_code + 1;
	free_code = clear_code + 2;
	max_code = (1 << code_size);
	for (int i = 0; i < table_size; i++)
		currentcode[i] = 0;
}
G42LzwOutput::~G42LzwOutput()
{
}
void
G42LzwOutput::Flush(void)
{
	/* write the prefix code */
	write_code(prefix_code);
	/* and the end of file code */
	write_code(eof_code);
	/* flush the buffer */
	if (bit_offset > 0)
		flush((bit_offset + 7) / 8);
	G42WriteCompression::Flush();
}
void
G42LzwOutput::ProcessBuffer(byte * buffer, uint length)
{
	for (const byte * bp = buffer; bp < buffer + length; bp++)
	{
		byte c = *bp;
		int i;
		suffix_char = c;
		if (firstChar)
		{
			prefix_code = suffix_char;
			firstChar = false;
			continue;
		}
		/* derive an index into the code table */
		hx=(prefix_code ^ ((int)suffix_char << 5)) % table_size;
		d=1;
		for(;;) {
			/* see if the code is in the table */
			if(currentcode[hx] == 0) {
				/* if not, put it there */
				write_code(prefix_code);
				d = free_code;
				/* find the next free code */
				if(free_code <= largest_code) {
					oldcode[hx] = prefix_code;
					newcode[hx] = suffix_char;
					currentcode[hx] = free_code;
					free_code++;
				}
				/* expand the code size or scrap the table */
				if (Type == LzwGif)
					i = 0;
				else
					i = 1;
				if(d >= max_code - i) {
					if(code_size < 12) {
						code_size++;
						max_code <<= 1;
					}
					else {
						write_code(clear_code);
						init_table(min_code_size);
					}
				}
				prefix_code = suffix_char;
				break;
			}
			if(oldcode[hx] == prefix_code &&
				newcode[hx] == suffix_char) {
				prefix_code = currentcode[hx];
				break;
			}
			hx += d;
			d += 2;
			if(hx >= table_size) hx -= table_size;
		}
	}
}
void
G42LzwOutput::write_code(int code)
{
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
void
G42LzwOutput::flush(int n)
{
	if (!n)
		return;
	PutSequence(code_buffer, n);
}

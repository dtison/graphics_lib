// g42wcctt.cpp - Ccitt compressor
/*************************************************************
	File:          g42wcctt.cpp
	Copyright (c) 1996, Group 42, Inc.
	Description:   Compresses ccitt streams
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "gicccitt.h"
G42CcittCompressor::G42CcittCompressor(G42CompressorOutput & output,
	intf compression_type, uintf width, uintf output_buffer_size)
:
	G42Compressor(output, output_buffer_size),
	CompressionType(compression_type),
	Width(width), RowBytes(0), write_buf(0), write_mask(0x80),
	current_bit(0), current_count(0), cur_loc(0), in_horiz_mode(false),
	write_ref_buf(0), next_buf(0), write_ref_size(0), write_ref_loc(0),
	next_loc(0)
{
	RowBytes = ((Width + 7) >> 3);
	init_ccitt(CompressionType, Width);
}
G42CcittCompressor::~G42CcittCompressor()
{
	if (write_ref_buf)
	{
		delete [] write_ref_buf;
		write_ref_buf = 0;
	}
	if (next_buf)
	{
		delete [] next_buf;
		next_buf = 0;
	}
}
void
G42CcittCompressor::Flush(void)
{
	finish_ccitt(CompressionType);
	G42Compressor::Flush();
}
void
G42CcittCompressor::ProcessBuffer(byte * buffer, uintf length)
{
	start_ccitt_row(8, CompressionType);
	uintf width_left = Width;
	for (uintf i = 0; i < length; i++)
	{
		write_ccitt_char(buffer[i], min((uintf)8, width_left),
			CompressionType, Width);
		width_left -= 8;
	}
	finish_ccitt_row(CompressionType, Width);
}
#define NUM_CODES 105
typedef struct
{
	intf repeat;     /* the number of pixels represented by the code */
	intf size;       /* the size in bits of the code */
	byte array[13]; /* the bits (left justified) */
} ccitt_code_def;
/* the first set of codes is for white pixels, the second is for black,
	the third is the 2d modes */
intf pixel_codes[3] = {NUM_CODES, NUM_CODES, 9};
ccitt_code_def ccitt_code[3][NUM_CODES] =
{
	{
		/* white codes */
		{0, 8, {0, 0, 1, 1, 0, 1, 0, 1}},
		{1, 6, {0, 0, 0, 1, 1, 1}},
		{2, 4, {0, 1, 1, 1}},
		{3, 4, {1, 0, 0, 0}},
		{4, 4, {1, 0, 1, 1}},
		{5, 4, {1, 1, 0, 0}},
		{6, 4, {1, 1, 1, 0}},
		{7, 4, {1, 1, 1, 1}},
		{8, 5, {1, 0, 0, 1, 1}},
		{9, 5, {1, 0, 1, 0, 0}},
		{10, 5, {0, 0, 1, 1, 1}},
		{11, 5, {0, 1, 0, 0, 0}},
		{12, 6, {0, 0, 1, 0, 0, 0}},
		{13, 6, {0, 0, 0, 0, 1, 1}},
		{14, 6, {1, 1, 0, 1, 0, 0}},
		{15, 6, {1, 1, 0, 1, 0, 1}},
		{16, 6, {1, 0, 1, 0, 1, 0}},
		{17, 6, {1, 0, 1, 0, 1, 1}},
		{18, 7, {0, 1, 0, 0, 1, 1, 1}},
		{19, 7, {0, 0, 0, 1, 1, 0, 0}},
		{20, 7, {0, 0, 0, 1, 0, 0, 0}},
		{21, 7, {0, 0, 1, 0, 1, 1, 1}},
		{22, 7, {0, 0, 0, 0, 0, 1, 1}},
		{23, 7, {0, 0, 0, 0, 1, 0, 0}},
		{24, 7, {0, 1, 0, 1, 0, 0, 0}},
		{25, 7, {0, 1, 0, 1, 0, 1, 1}},
		{26, 7, {0, 0, 1, 0, 0, 1, 1}},
		{27, 7, {0, 1, 0, 0, 1, 0, 0}},
		{28, 7, {0, 0, 1, 1, 0, 0, 0}},
		{29, 8, {0, 0, 0, 0, 0, 0, 1, 0}},
		{30, 8, {0, 0, 0, 0, 0, 0, 1, 1}},
		{31, 8, {0, 0, 0, 1, 1, 0, 1, 0}},
		{32, 8, {0, 0, 0, 1, 1, 0, 1, 1}},
		{33, 8, {0, 0, 0, 1, 0, 0, 1, 0}},
		{34, 8, {0, 0, 0, 1, 0, 0, 1, 1}},
		{35, 8, {0, 0, 0, 1, 0, 1, 0, 0}},
		{36, 8, {0, 0, 0, 1, 0, 1, 0, 1}},
		{37, 8, {0, 0, 0, 1, 0, 1, 1, 0}},
		{38, 8, {0, 0, 0, 1, 0, 1, 1, 1}},
		{39, 8, {0, 0, 1, 0, 1, 0, 0, 0}},
		{40, 8, {0, 0, 1, 0, 1, 0, 0, 1}},
		{41, 8, {0, 0, 1, 0, 1, 0, 1, 0}},
		{42, 8, {0, 0, 1, 0, 1, 0, 1, 1}},
		{43, 8, {0, 0, 1, 0, 1, 1, 0, 0}},
		{44, 8, {0, 0, 1, 0, 1, 1, 0, 1}},
		{45, 8, {0, 0, 0, 0, 0, 1, 0, 0}},
		{46, 8, {0, 0, 0, 0, 0, 1, 0, 1}},
		{47, 8, {0, 0, 0, 0, 1, 0, 1, 0}},
		{48, 8, {0, 0, 0, 0, 1, 0, 1, 1}},
		{49, 8, {0, 1, 0, 1, 0, 0, 1, 0}},
		{50, 8, {0, 1, 0, 1, 0, 0, 1, 1}},
		{51, 8, {0, 1, 0, 1, 0, 1, 0, 0}},
		{52, 8, {0, 1, 0, 1, 0, 1, 0, 1}},
		{53, 8, {0, 0, 1, 0, 0, 1, 0, 0}},
		{54, 8, {0, 0, 1, 0, 0, 1, 0, 1}},
		{55, 8, {0, 1, 0, 1, 1, 0, 0, 0}},
		{56, 8, {0, 1, 0, 1, 1, 0, 0, 1}},
		{57, 8, {0, 1, 0, 1, 1, 0, 1, 0}},
		{58, 8, {0, 1, 0, 1, 1, 0, 1, 1}},
		{59, 8, {0, 1, 0, 0, 1, 0, 1, 0}},
		{60, 8, {0, 1, 0, 0, 1, 0, 1, 1}},
		{61, 8, {0, 0, 1, 1, 0, 0, 1, 0}},
		{62, 8, {0, 0, 1, 1, 0, 0, 1, 1}},
		{63, 8, {0, 0, 1, 1, 0, 1, 0, 0}},
		{64, 5, {1, 1, 0, 1, 1}},
		{128, 5, {1, 0, 0, 1, 0}},
		{192, 6, {0, 1, 0, 1, 1, 1}},
		{256, 7, {0, 1, 1, 0, 1, 1, 1}},
		{320, 8, {0, 0, 1, 1, 0, 1, 1, 0}},
		{384, 8, {0, 0, 1, 1, 0, 1, 1, 1}},
		{448, 8, {0, 1, 1, 0, 0, 1, 0, 0}},
		{512, 8, {0, 1, 1, 0, 0, 1, 0, 1}},
		{576, 8, {0, 1, 1, 0, 1, 0, 0, 0}},
		{640, 8, {0, 1, 1, 0, 0, 1, 1, 1}},
		{704, 9, {0, 1, 1, 0, 0, 1, 1, 0, 0}},
		{768, 9, {0, 1, 1, 0, 0, 1, 1, 0, 1}},
		{832, 9, {0, 1, 1, 0, 1, 0, 0, 1, 0}},
		{896, 9, {0, 1, 1, 0, 1, 0, 0, 1, 1}},
		{960, 9, {0, 1, 1, 0, 1, 0, 1, 0, 0}},
		{1024, 9, {0, 1, 1, 0, 1, 0, 1, 0, 1}},
		{1088, 9, {0, 1, 1, 0, 1, 0, 1, 1, 0}},
		{1152, 9, {0, 1, 1, 0, 1, 0, 1, 1, 1}},
		{1216, 9, {0, 1, 1, 0, 1, 1, 0, 0, 0}},
		{1280, 9, {0, 1, 1, 0, 1, 1, 0, 0, 1}},
		{1344, 9, {0, 1, 1, 0, 1, 1, 0, 1, 0}},
		{1408, 9, {0, 1, 1, 0, 1, 1, 0, 1, 1}},
		{1472, 9, {0, 1, 0, 0, 1, 1, 0, 0, 0}},
		{1536, 9, {0, 1, 0, 0, 1, 1, 0, 0, 1}},
		{1600, 9, {0, 1, 0, 0, 1, 1, 0, 1, 0}},
		{1664, 6, {0, 1, 1, 0, 0, 0}},
		{1728, 9, {0, 1, 0, 0, 1, 1, 0, 1, 1}},
		{-1, 12, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}},
		{1792, 11, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0}},
		{1856, 11, {0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0}},
		{1920, 11, {0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1}},
		{1984, 12, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0}},
		{2048, 12, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1}},
		{2112, 12, {0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0}},
		{2176, 12, {0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1}},
		{2240, 12, {0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0}},
		{2304, 12, {0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1}},
		{2368, 12, {0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0}},
		{2432, 12, {0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1}},
		{2496, 12, {0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0}},
		{2560, 12, {0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1}},
	},
	{
		/* black codes */
		{0, 10, {0, 0, 0, 0, 1, 1, 0, 1, 1, 1}},
		{1, 3, {0, 1, 0}},
		{2, 2, {1, 1}},
		{3, 2, {1, 0}},
		{4, 3, {0, 1, 1}},
		{5, 4, {0, 0, 1, 1}},
		{6, 4, {0, 0, 1, 0}},
		{7, 5, {0, 0, 0, 1, 1}},
		{8, 6, {0, 0, 0, 1, 0, 1}},
		{9, 6, {0, 0, 0, 1, 0, 0}},
		{10, 7, {0, 0, 0, 0, 1, 0, 0}},
		{11, 7, {0, 0, 0, 0, 1, 0, 1}},
		{12, 7, {0, 0, 0, 0, 1, 1, 1}},
		{13, 8, {0, 0, 0, 0, 0, 1, 0, 0}},
		{14, 8, {0, 0, 0, 0, 0, 1, 1, 1}},
		{15, 9, {0, 0, 0, 0, 1, 1, 0, 0, 0}},
		{16, 10, {0, 0, 0, 0, 0, 1, 0, 1, 1, 1}},
		{17, 10, {0, 0, 0, 0, 0, 1, 1, 0, 0, 0}},
		{18, 10, {0, 0, 0, 0, 0, 0, 1, 0, 0, 0}},
		{19, 11, {0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1}},
		{20, 11, {0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0}},
		{21, 11, {0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0}},
		{22, 11, {0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1}},
		{23, 11, {0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0}},
		{24, 11, {0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1}},
		{25, 11, {0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0}},
		{26, 12, {0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0}},
		{27, 12, {0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1}},
		{28, 12, {0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0}},
		{29, 12, {0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1}},
		{30, 12, {0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0}},
		{31, 12, {0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1}},
		{32, 12, {0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0}},
		{33, 12, {0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1}},
		{34, 12, {0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0}},
		{35, 12, {0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1}},
		{36, 12, {0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0}},
		{37, 12, {0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1}},
		{38, 12, {0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0}},
		{39, 12, {0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1}},
		{40, 12, {0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0}},
		{41, 12, {0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1}},
		{42, 12, {0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 0}},
		{43, 12, {0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1}},
		{44, 12, {0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0}},
		{45, 12, {0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1}},
		{46, 12, {0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0}},
		{47, 12, {0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1}},
		{48, 12, {0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0}},
		{49, 12, {0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1}},
		{50, 12, {0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0}},
		{51, 12, {0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1}},
		{52, 12, {0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0}},
		{53, 12, {0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1}},
		{54, 12, {0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0}},
		{55, 12, {0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1}},
		{56, 12, {0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0}},
		{57, 12, {0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0}},
		{58, 12, {0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1}},
		{59, 12, {0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1}},
		{60, 12, {0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0}},
		{61, 12, {0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0}},
		{62, 12, {0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0}},
		{63, 12, {0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1}},
		{64, 10, {0, 0, 0, 0, 0, 0, 1, 1, 1, 1}},
		{128, 12, {0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0}},
		{192, 12, {0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1}},
		{256, 12, {0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1}},
		{320, 12, {0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1}},
		{384, 12, {0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0}},
		{448, 12, {0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1}},
		{512, 13, {0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0}},
		{576, 13, {0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1}},
		{640, 13, {0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0}},
		{704, 13, {0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1}},
		{768, 13, {0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0}},
		{832, 13, {0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1}},
		{896, 13, {0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0}},
		{960, 13, {0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1}},
		{1024, 13, {0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0}},
		{1088, 13, {0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1}},
		{1152, 13, {0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0}},
		{1216, 13, {0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1}},
		{1280, 13, {0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0}},
		{1344, 13, {0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1}},
		{1408, 13, {0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0}},
		{1472, 13, {0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1}},
		{1536, 13, {0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0}},
		{1600, 13, {0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1}},
		{1664, 13, {0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0}},
		{1728, 13, {0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1}},
		{-1, 11, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
		{1792, 11, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0}},
		{1856, 11, {0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0}},
		{1920, 11, {0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1}},
		{1984, 12, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0}},
		{2048, 12, {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1}},
		{2112, 12, {0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0}},
		{2176, 12, {0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1}},
		{2240, 12, {0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0}},
		{2304, 12, {0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1}},
		{2368, 12, {0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0}},
		{2432, 12, {0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1}},
		{2496, 12, {0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0}},
		{2560, 12, {0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1}},
	},
	{
		/* 2d mode codes */
		{ 5, 3, {0, 0, 1}},
		{ 4, 4, {0, 0, 0, 1}},
		{ 3, 7, {0, 0, 0, 0, 0, 1, 1}},
		{ 2, 6, {0, 0, 0, 0, 1, 1}},
		{ 1, 3, {0, 1, 1}},
		{ 0, 1, {1}},
		{-1, 3, {0, 1, 0}},
		{-2, 6, {0, 0, 0, 0, 1, 0}},
		{-3, 7, {0, 0, 0, 0, 0, 1, 0}},
	}
};
/* 2d special codes */
#define CCITT_HORIZ 5
#define CCITT_PASS 4
/* used to write 2d modes */
#define CCITT_2D 2
/* reset to byte boundry */
void
G42CcittCompressor::start_write_ccitt_code(void)
{
	write_buf = 0;
	write_mask = 0x80;
}
/* write a 1d code for a bit count */
void
G42CcittCompressor::write_ccitt_code(intf bit, intf count)
{
	intf count_index, i;
	/* keep writing codes until you write one < 64 */
	do
	{
		/* write the maximum code available */
		for (count_index = NUM_CODES - 1; count_index >= 0;
			count_index--)
		{
			/* skip eol */
			if (ccitt_code[bit][count_index].repeat == -1)
				continue;
			if (ccitt_code[bit][count_index].repeat <= count)
			{
				/* found a code, write one bit at a time */
				for (i = 0;
					i < ccitt_code[bit][count_index].size;
					i++)
				{
					if (ccitt_code[bit][count_index].array[
						i])
						write_buf |= write_mask;
					else
						write_buf &= ~write_mask;
					write_mask >>= 1;
					if (!write_mask)
					{
						PutByte((byte)write_buf);
						write_mask = 0x80;
						write_buf = 0;
					}
				}
				/* subtract code from total count, break from
				   for loop to try again */
				count -= ccitt_code[bit][count_index].repeat;
				break;
			}
		}
	}
	while (count_index >= 64);
}
/* write a 3d code, simpler then above */
void
G42CcittCompressor::write_ccitt_3d_code(intf count)
{
	intf count_index, i;
	/* search for correct code */
	for (count_index = pixel_codes[CCITT_2D] - 1;
		count_index >= 0; count_index--)
	{
		if (ccitt_code[CCITT_2D][count_index].repeat == count)
		{
			/* if found, write one bit at a time */
			for (i = 0; i < ccitt_code[CCITT_2D][count_index].size;
				i++)
			{
				if (ccitt_code[CCITT_2D][count_index].array[i])
					write_buf |= write_mask;
				else
					write_buf &= ~write_mask;
				write_mask >>= 1;
				if (!write_mask)
				{
					PutByte((byte)write_buf);
					write_mask = 0x80;
					write_buf = 0;
				}
			}
			break;
		}
	}
}
/* write group 3 eol (11 0's and a 1) */
void
G42CcittCompressor::write_ccitt_eol(void)
{
	intf i;
	for (i = 0; i < 12; i++)
	{
		if (i == 11)
			write_buf |= write_mask;
		else
			write_buf &= ~write_mask;
		write_mask >>= 1;
		if (!write_mask)
		{
			PutByte((byte)write_buf);
			write_mask = 0x80;
			write_buf = 0;
		}
	}
}
/* make sure you write out the last partial byte */
void
G42CcittCompressor::end_write_ccitt_char(void)
{
	if (write_mask < 0x80)
	{
		PutByte((byte)write_buf);
	}
}
/* got a span, write it out */
void
G42CcittCompressor::write_ccitt_count(intf current_count, intf num_bits,
	intf compression)
{
	intf ref, next_ref;
	/* 1d compression is easy, just write 1d code */
	if (compression == 2 || compression == 3)
	{
		write_ccitt_code(current_bit, current_count);
	}
	else if (compression == 4)
	{
		/* 2d compression */
		if (in_horiz_mode)
		{
			/* write 2nd half of horizontal mode 1d compression */
			write_ccitt_code(current_bit, current_count);
			in_horiz_mode = false;
		}
		else
		{
			/* get span of reference buffer */
			write_ref_loc = cur_loc;
			ref = get_ref_count(write_ref_loc, (long)num_bits,
				write_ref_buf, current_bit);
			/* if not first time in row, and no span found,
				search buffer until you find pixel color, then
				record that span */
			if (!ref && (cur_loc || current_bit))
			{
				write_ref_loc += get_ref_count(write_ref_loc,
					(long)num_bits, write_ref_buf, !current_bit);
				ref = get_ref_count(write_ref_loc, (long)num_bits,
					write_ref_buf, current_bit);
			}
			/* check to see if we have a pass code */
			next_ref = get_ref_count(write_ref_loc + ref,
				(long)num_bits, write_ref_buf, !current_bit);
			/* this is a mess.  while you have 2 ref spans that are
				less then the current span, and no vertical code
				applies, issue a pass code and update all this
				stuff */
			while (/* ref && */ next_ref && ref + next_ref + write_ref_loc
				<= current_count + cur_loc &&
				ref + next_ref + write_ref_loc < num_bits &&
				(ref + write_ref_loc < current_count + cur_loc -
					3 || ref + write_ref_loc >
					current_count + cur_loc + 3))
			{
				write_ccitt_3d_code(CCITT_PASS);
				/* update count */
				current_count -= ref + next_ref +
					write_ref_loc - cur_loc;
				/* get new ref info */
				cur_loc = ref + next_ref + write_ref_loc;
				write_ref_loc = cur_loc;
				ref = get_ref_count(write_ref_loc, (long)num_bits,
					write_ref_buf, current_bit);
				if (!ref)
				{
					write_ref_loc += get_ref_count(
						write_ref_loc, (long)num_bits,
						write_ref_buf, !current_bit);
					ref = get_ref_count(write_ref_loc,
						(long)num_bits, write_ref_buf,
						current_bit);
				}
				next_ref = get_ref_count(write_ref_loc + ref,
					(long)num_bits, write_ref_buf, !current_bit);
			}
			/* see if we can describe the count in terms of
				the ref count */
			if (abs(ref + write_ref_loc - current_count -
				cur_loc) <= 3)
			{
				write_ccitt_3d_code(current_count + cur_loc -
					(ref + write_ref_loc));
			}
			else
			{
				/* 1d include this span and the next */
				write_ccitt_3d_code(CCITT_HORIZ);
				write_ccitt_code(current_bit,
					current_count);
				in_horiz_mode = true;
			}
		}
		/* update where we are in the row */
		cur_loc += current_count;
	}
}
void
G42CcittCompressor::init_ccitt(intf compression, intf width)
{
	/* if (group 3 or 4, need to setup reference buffers and row counters */
	if (compression == 4 || compression == 3)
	{
		start_write_ccitt_code();
		current_bit = 0;
		current_count = 0;
		if (compression == 4)
		{
			write_ref_buf = new char [(width + 8) >> 3];
			next_buf = new char [(width + 8) >> 3];
			memset(write_ref_buf, 0, (width + 8) >> 3);
			memset(next_buf, 0, (width + 8) >> 3);
		}
	}
}
void
G42CcittCompressor::start_ccitt_row(intf, intf compression)
{
	/* reset row locators at beginning of row */
	if (compression == 2)
	{
		start_write_ccitt_code();
	}
	else if (compression == 3)
	{
		if (write_mask < 0x40)
		{
			end_write_ccitt_char();
		}
		write_mask = 0x40;
		write_ccitt_eol();
	}
	else if (compression == 4)
	{
		write_ref_loc = 0;
		next_loc = 0;
		in_horiz_mode = false;
		cur_loc = 0;
	}
	current_bit = 0;
	current_count = 0;
}
/* got a char, count bit runs */
void
G42CcittCompressor::write_ccitt_char(intf buf, intf num_bits, intf compression,
	intf width)
{
	intf mask, bit, i;
	/* check each valid bit */
	for (mask = 0x80, i = 0; mask && i < num_bits; mask >>= 1, i++)
	{
		/* get bit */
		if (mask & buf)
			bit = 0;
		else
			bit = 1;
		/* if bit same as current run, keep counting */
		if ((bit && current_bit) || (!bit && !current_bit))
		{
			current_count++;
		}
		else
		{
			/* found a different color, write old run,
				and reset to current color.  If count is 0,
				we still need to call it */
			write_ccitt_count(current_count, width,
				compression);
			current_bit = bit;
			current_count = 1;
		}
	}
	/* save character for reference buffer */
	if (compression == 4)
		next_buf[next_loc++] = (char)(~buf);
}
/* clean up row */
void
G42CcittCompressor::finish_ccitt_row(intf compression, intf width)
{
	if (current_count)
	{
		write_ccitt_count(current_count, width, compression);
	}
	if (compression == 2)
	{
		end_write_ccitt_char();
	}
	/* save reference buffer */
	if (compression == 4)
	{
		if (in_horiz_mode)
		{
			write_ccitt_code(!current_bit, 0);
			in_horiz_mode = false;
		}
		memcpy(write_ref_buf, next_buf, next_loc);
	}
}
/* finished, if group 4, write 2 eol's and free reference buffers */
void
G42CcittCompressor::finish_ccitt(intf compression)
{
	if (compression == 4)
	{
		write_ccitt_eol();
		write_ccitt_eol();
		end_write_ccitt_char();
	}
	else if (compression == 3)
	{
		write_ccitt_eol();
		end_write_ccitt_char();
	}
}
/* find out how long the next run of bits is */
intf
G42CcittCompressor::get_ref_count(intf cur, long max, char * buf, intf pixel)
{
	char * bptr;
	intf mask, count;
	bptr = buf + (cur >> 3);
	mask = 0x80 >> (cur & 7);
	count = 0;
	if ((!pixel && (*bptr & mask)) ||
		(pixel && !(*bptr & mask)))
		return 0;
	while (count < max - cur)
	{
		mask >>= 1;
		if (!mask)
		{
			mask = 0x80;
			bptr++;
		}
		count++;
		if ((!pixel && (*bptr & mask)) ||
			(pixel && !(*bptr & mask)))
		{
			break;
		}
	}
	return count;
}

// gidccitt.cpp - CCITT Compression
/*************************************************************
	File:          gidccitt.cpp
	Copyright (c) 1996, Group 42, Inc.
	Description:   Class that handles ccitt compression
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "gidccitt.h"
#define GET_CCITT_BIT_MALE(BIT) \
	if(!Mask)\
	{\
		Mask = 0x80;\
		if(RegPtr >= BufferEnd)\
			OutOfData = true;\
		else\
		{\
			SavedByte = *RegPtr;\
			RegPtr++;\
			BIT = (SavedByte & Mask) ? 1 : 0;\
			Mask >>= 1;\
		}\
	}\
	else\
	{\
		BIT = (SavedByte & Mask) ? 1 : 0;\
		Mask >>= 1;\
	}
#define GET_CCITT_BIT_FEMALE(BIT)  \
	if(Mask > 0x80)\
   {\
		Mask = 1;\
		if(RegPtr >= BufferEnd)\
			OutOfData = true;\
		else\
		{\
			SavedByte = *RegPtr;\
			RegPtr++;\
			BIT = (SavedByte & Mask) ? 1 : 0;\
			Mask <<= 1;\
		}\
	}\
	else\
	{\
		BIT = (SavedByte & Mask) ? 1 : 0;\
		Mask <<= 1;\
	}
char G42CcittDecompressor::obuf_mask_b[9] =
   {0, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff};
char G42CcittDecompressor::obuf_mask_w[9] =
   {0, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0x00};
G42CcittDecompressor::ccitt_node G42CcittDecompressor::ccitt_table2[2][104] =
{
	{
		{{1, 2}, {0, 0}},	/* 0 */
		{{3, 4}, {0, 0}},	/* 1 */
		{{93, 94}, {0, 0}},	/* 2 */
		{{5, 6}, {0, 0}},	/* 3 */
		{{56, 57}, {0, 0}},	/* 4 */
		{{7, 8}, {0, 0}},	/* 5 */
		{{39, 40}, {0, 0}},	/* 6 */
		{{9, 10}, {0, 0}},	/* 7 */
		{{30, 31}, {0, 0}},	/* 8 */
		{{11, 12}, {0, 0}},	/* 9 */
		{{28, -1}, {0, 13}},	/* 10 */
		{{13, 14}, {0, 0}},
		{{27, -1}, {0, 22}},
		{{-4, 15}, {0, 0}},
		{{-1, -1}, {29, 30}},
		{{16, 17}, {0, 0}},
		{{18, 19}, {0, 0}},
		{{23, 24}, {0, 0}},
		{{-1, 20}, {1792, 0}},
		{{21, 22}, {0, 0}},
		{{-1, -1}, {1984, 2048}},
		{{-1, -1}, {2112, 2176}},
		{{-1, -1}, {2240, 2304}},
		{{-1, -1}, {1856, 1920}},
		{{25, 26}, {0, 0}},
		{{-1, -1}, {2368, 2432}},
		{{-1, -1}, {2496, 2560}},
		{{-1, -1}, {45, 46}},
		{{-1, 29}, {23, 0}},
		{{-1, -1}, {47, 48}},
		{{32, 33}, {0, 0}},
		{{37, -1}, {0, 1}},
		{{-1, 34}, {20, 0}},
		{{35, 36}, {0, 0}},
		{{-1, -1}, {33, 34}},
		{{-1, -1}, {35, 36}},
		{{-1, -1}, {37, 38}},
		{{-1, 38}, {19, 0}},
		{{-1, -1}, {31, 32}},
		{{41, 42}, {0, 0}},
		{{50, -1}, {0, 10}},
		{{-1, 43}, {12, 0}},
		{{45, 46}, {0, 0}},
		{{44, -1}, {0, 26}},
		{{-1, -1}, {53, 54}},
		{{47, 48}, {0, 0}},
		{{49, -1}, {0, 21}},
		{{-1, -1}, {39, 40}},
		{{-1, -1}, {41, 42}},
		{{-1, -1}, {43, 44}},
		{{51, 52}, {0, 0}},
		{{-1, 53}, {28, 0}},
		{{54, 55}, {0, 0}},
		{{-1, -1}, {61, 62}},
		{{-1, -1}, {63, 0}},
		{{-1, -1}, {320, 384}},
		{{58, 59}, {0, 0}},
		{{76, -1}, {0, 2}},
		{{-1, 60}, {11, 0}},
		{{67, 68}, {0, 0}},
		{{61, 62}, {0, 0}},
		{{-1, 63}, {27, 0}},
		{{64, -1}, {0, 18}},
		{{-1, -1}, {59, 60}},
		{{65, 66}, {0, 0}},
		{{-1, -1}, {1472, 1536}},
		{{-1, -1}, {1600, 1728}},
		{{69, 70}, {0, 0}},
		{{73, -1}, {0, 192}},
		{{-1, 71}, {24, 0}},
		{{72, -1}, {0, 25}},
		{{-1, -1}, {49, 50}},
		{{-1, -1}, {51, 52}},
		{{74, 75}, {0, 0}},
		{{-1, -1}, {55, 56}},
		{{-1, -1}, {57, 58}},
		{{77, 78}, {0, 0}},
		{{-1, 79}, {1664, 0}},
		{{83, 84}, {0, 0}},
		{{80, 81}, {0, 0}},
		{{-1, -1}, {448, 512}},
		{{82, -1}, {0, 640}},
		{{-1, -1}, {704, 768}},
		{{85, 86}, {0, 0}},
		{{90, -1}, {0, 256}},
		{{-1, 87}, {576, 0}},
		{{88, 89}, {0, 0}},
		{{-1, -1}, {832, 896}},
		{{-1, -1}, {960, 1024}},
		{{-1, -1}, {1088, 1152}},
		{{91, 92}, {0, 0}},
		{{-1, -1}, {1216, 1280}},
		{{-1, -1}, {1344, 1408}},
		{{95, 96}, {0, 0}},
		{{100, 101}, {0, 0}},
		{{-1, 97}, {3, 0}},
		{{98, -1}, {0, 4}},
		{{-1, -1}, {128, 8}},
		{{-1, 99}, {9, 0}},
		{{-1, -1}, {16, 17}},
		{{-1, 102}, {5, 0}},
		{{-1, -1}, {6, 7}},
		{{103, -1}, {0, 64}},
		{{-1, -1}, {14, 15}}
	},
	{
		{{1, 2}, {0, 0}},
		{{3, 4}, {0, 0}},
		{{-1, -1}, {3, 2}},
		{{5, 6}, {0, 0}},
		{{-1, -1}, {1, 4}},
		{{7, 8}, {0, 0}},
		{{-1, -1}, {6, 5}},
		{{9, 10}, {0, 0}},
		{{103, -1}, {0, 7}},
		{{11, 12}, {0, 0}},
		{{83, 84}, {0, 0}},
		{{13, 14}, {0, 0}},
		{{59, 60}, {0, 0}},
		{{-4, 15}, {0, 0}},
		{{27, 28}, {0, 0}},
		{{16, 17}, {0, 0}},
		{{18, 19}, {0, 0}},
		{{23, 24}, {0, 0}},
		{{-1, 20}, {1792, 0}},
		{{21, 22}, {0, 0}},
		{{-1, -1}, {1984, 2048}},
		{{-1, -1}, {2112, 2176}},
		{{-1, -1}, {2240, 2304}},
		{{-1, -1}, {1856, 1920}},
		{{25, 26}, {0, 0}},
		{{-1, -1}, {2368, 2432}},
		{{-1, -1}, {2496, 2560}},
		{{29, 30}, {0, 0}},
		{{44, 45}, {0, 0}},
		{{-1, 31}, {18, 0}},
		{{36, 37}, {0, 0}},
		{{32, 33}, {0, 0}},
		{{-1, 34}, {52, 0}},
		{{35, -1}, {0, 55}},
		{{-1, -1}, {640, 704}},
		{{-1, -1}, {768, 832}},
		{{38, 39}, {0, 0}},
		{{42, -1}, {0, 24}},
		{{-1, 40}, {56, 0}},
		{{41, -1}, {0, 59}},
		{{-1, -1}, {1280, 1344}},
		{{-1, -1}, {1408, 1472}},
		{{-1, 43}, {60, 0}},
		{{-1, -1}, {1536, 1600}},
		{{46, 47}, {0, 0}},
		{{53, -1}, {0, 64}},
		{{-1, 48}, {25, 0}},
		{{50, 51}, {0, 0}},
		{{49, -1}, {0, 320}},
		{{-1, -1}, {1664, 1728}},
		{{-1, -1}, {384, 448}},
		{{52, -1}, {0, 53}},
		{{-1, -1}, {512, 576}},
		{{54, 55}, {0, 0}},
		{{-1, 56}, {54, 0}},
		{{57, 58}, {0, 0}},
		{{-1, -1}, {896, 960}},
		{{-1, -1}, {1024, 1088}},
		{{-1, -1}, {1152, 1216}},
		{{-1, 61}, {13, 0}},
		{{72, -1}, {0, 14}},
		{{62, 63}, {0, 0}},
		{{64, 65}, {0, 0}},
		{{69, -1}, {0, 16}},
		{{-1, 66}, {23, 0}},
		{{67, 68}, {0, 0}},
		{{-1, -1}, {50, 51}},
		{{-1, -1}, {44, 45}},
		{{-1, -1}, {46, 47}},
		{{70, 71}, {0, 0}},
		{{-1, -1}, {57, 58}},
		{{-1, -1}, {61, 256}},
		{{73, 74}, {0, 0}},
		{{-1, 75}, {17, 0}},
		{{78, 79}, {0, 0}},
		{{76, 77}, {0, 0}},
		{{-1, -1}, {48, 49}},
		{{-1, -1}, {62, 63}},
		{{80, 81}, {0, 0}},
		{{82, -1}, {0, 22}},
		{{-1, -1}, {30, 31}},
		{{-1, -1}, {32, 33}},
		{{-1, -1}, {40, 41}},
		{{-1, -1}, {10, 11}},
		{{85, -1}, {0, 12}},
		{{86, 87}, {0, 0}},
		{{-1, 88}, {15, 0}},
		{{94, 95}, {0, 0}},
		{{89, 90}, {0, 0}},
		{{91, 92}, {0, 0}},
		{{93, -1}, {0, 19}},
		{{-1, -1}, {128, 192}},
		{{-1, -1}, {26, 27}},
		{{-1, -1}, {28, 29}},
		{{96, 97}, {0, 0}},
		{{101, -1}, {0, 0}},
		{{-1, 98}, {20, 0}},
		{{99, 100}, {0, 0}},
		{{-1, -1}, {34, 35}},
		{{-1, -1}, {36, 37}},
		{{-1, -1}, {38, 39}},
		{{-1, 102}, {21, 0}},
		{{-1, -1}, {42, 43}},
		{{-1, -1}, {9, 8}}
	}
};
G42CcittDecompressor::G42CcittDecompressor(G42DecompressorOutput & output,
	intf width, intf compression, intf g3opts, intf fillorder, intf num_rows)
:
	G42Decompressor(output, (width + 7) >> 3),
	Mask(0), SavedByte(0), LastRegPtr(0), LookBackBuffer(0), WorkBuffer(0),
	BufferEnd(0), RegPtr(0), Compression(compression), G3Opts(g3opts),
	FillOrder(fillorder), Width(width), Pixel(0), ReadWidth(32767),
	TotalRepeat(0), Pos(0), NextPos(0), obuf_mask_ptr(8), BytePtr(0),
	SpanArray(0), NewSpanArray(0), NumRows(num_rows), RowsRead(0),
	FirstTime(true), LastSavedByte(0), LastMask(0), Valid(true)
{
	if (FillOrder != 1)
	{
		 Mask = 0x100;
		 LastMask = 0x100;
	}
// following is done in ProcessBuffer
//	memset(OutputBuffer, 0, ((Width + 7) >> 3));
	if (Compression == 4 || (Compression == 3 && G3Opts & 1))
	{
		SpanArray = new intf [Width + 1];
		NewSpanArray = new intf [Width + 1];
		SpanArray[0] = 0;
		SpanArray[1] = Width;
	}
}
G42CcittDecompressor::~G42CcittDecompressor(void)
{
	if (LookBackBuffer)
		delete [] LookBackBuffer;
	LookBackBuffer = 0;
	if (WorkBuffer)
		delete [] WorkBuffer;
	WorkBuffer = 0;
	if (SpanArray)
		delete [] SpanArray;
	SpanArray = 0;
	if (NewSpanArray)
		delete [] NewSpanArray;
	NewSpanArray = 0;
}
void
G42CcittDecompressor::ProcessBuffer(void)
{
	if (!Valid)
	{
		return;
	}
	OutOfData = false;
	G42VirtualBufferWrap vb_wrap(VBuffer);
	// the byte * cast here is a kludge, G42VirtualBuffer should use const
	VBuffer.AddBytes((byte *)InputPtr, (int32)(InputEnd - InputPtr));
#if 0
	if (LookBackBuffer != 0)
	{
		Mask = LastMask;
		SavedByte = LastSavedByte;
		if (WorkBuffer)
			delete [] WorkBuffer;
		WorkBuffer = 0;
		WorkBuffer = new char [buffer_size + LookBackSize];
		memcpy(WorkBuffer, LookBackBuffer, LookBackSize);
		memcpy(WorkBuffer + LookBackSize, buffer,
			buffer_size);
		BufferEnd = WorkBuffer + (buffer_size + LookBackSize);
		BytePtr = 0;
		memset(OutputBuffer, 0, ((Width + 7) >> 3));
	}
	else
	{
		Mask = LastMask;
		SavedByte = LastSavedByte;
		if (WorkBuffer)
			delete [] WorkBuffer;
		WorkBuffer = 0;
		WorkBuffer = new char [buffer_size];
		memcpy(WorkBuffer, buffer, buffer_size);
		BufferEnd = WorkBuffer + buffer_size;
		BytePtr = 0;
		memset(OutputBuffer, 0, ((Width + 7) >> 3));
	}
#endif
	Mask = LastMask;
	SavedByte = LastSavedByte;
	WorkBuffer = (char *)(byte *)VBuffer;
	BufferEnd = WorkBuffer + VBuffer.GetNumBytes();
	BytePtr = 0;
	memset(OutputBuffer, 0, ((Width + 7) >> 3));
	RegPtr = WorkBuffer;
	LastRegPtr = RegPtr;
	LastBufferEnd = BufferEnd;
	if (FirstTime)
	{
		/* get first eol */
		if (Compression == 3)
		{
			if (FillOrder == 1)
				ReadEolMale();
			else
				ReadEolFemale();
		}
		FirstTime = false;
	}
	ReadWidth = 32767;
	while (!OutOfData)
	{
	intf do1dcomp, mode, num_pixels;
	if (Compression == 3 && (G3Opts & 1))
	{
		if(FillOrder == 1)
		{
			GET_CCITT_BIT_MALE(do1dcomp)
			if (OutOfData) break;
		}
		else
		{
			GET_CCITT_BIT_FEMALE(do1dcomp)
			if (OutOfData) break;
		}
	}
	else if (Compression == 4)
		do1dcomp = 0;
	else
		do1dcomp = 1;
	if (Compression == 4 || (Compression == 3 &&
		G3Opts & 1))
	{
		NewSpanArray[0] = 0;
		NewSpanPtr = 1;
	}
	if (!do1dcomp)
	{
		/* 2d encoding */
		CurrentRefLoc = 0;
		CurrentSpanPtr = 0;
		if (ReadWidth >= Width)
		{
			ReadWidth = 0;
			Pixel = 0;
			obuf_mask_ptr = 8;
		}
		while (1)
		{
			/* get 2d mode */
			if(FillOrder == 1)
				mode = Get2dModeMale();
			else
				mode = Get2dModeFemale();
			if (mode < -3)
			{
				if (OutOfData) break;
				Valid = false;
				return;
			}
			if (mode == CCITT_PASS)
			{
				/* skip next two spans in reference line */
				CurrentRefLoc += GetRefCount();
				CurrentRefLoc += GetRefCount();
//GES((ges, "setting %d @ %d\n", CurrentRefLoc - ReadWidth, Pixel))
				if(Pixel)
					SetPixelsBlack(CurrentRefLoc - ReadWidth);
				else
					SetPixelsWhite(CurrentRefLoc - ReadWidth);
				NewSpanArray[NewSpanPtr] = ReadWidth;
			}
			else if (mode == CCITT_HORIZ)
			{
				/* next 2 spans are 1d encoded */
				num_pixels = GetPixel();
				if (num_pixels < 0)
					break;
				if (num_pixels > (Width - ReadWidth))
					num_pixels = Width - ReadWidth;
//GES((ges, "setting %d @ %d\n", num_pixels, Pixel))
				if(Pixel)
					SetPixelsBlack(num_pixels);
				else
					SetPixelsWhite(num_pixels);
				NewSpanArray[NewSpanPtr] = ReadWidth;
				Pixel = (Pixel + 1) & 1;
				NewSpanPtr++;
				num_pixels = GetPixel();
#if 0
				if (ReadWidth >= Width)
				{
					RowsRead++;
					if (Compression == 3 && (RowsRead < NumRows))
					{
						if(FillOrder == 1)
						{
							if (ReadEolMale() == -4) break;
						}
						else
						{
							if (ReadEolFemale() == -4) break;
						}
					}
					LastMask = Mask;
					LastSavedByte = SavedByte;
					LastRegPtr = RegPtr;
					LastBufferEnd = BufferEnd;
//GES((ges, "Sending row.\n"))
					if (OutputRow(OutputBuffer))
					{
						Valid = false;
						return;
					}
					BytePtr = 0;
					memset(OutputBuffer, 0, ((Width + 7) >> 3));
					memcpy(SpanArray, NewSpanArray, Width * sizeof(intf));
					break;
				}
#endif
				// only add second span if there is room (if not, num_pixels
				// should be zero anyway)
				if (ReadWidth < Width)
				{
					if (num_pixels < 0)
						break;
					if (num_pixels > (Width - ReadWidth))
						num_pixels = Width - ReadWidth;
//GES((ges, "setting %d @ %d\n", num_pixels, Pixel))
					if(Pixel)
						SetPixelsBlack(num_pixels);
					else
						SetPixelsWhite(num_pixels);
					NewSpanArray[NewSpanPtr] = ReadWidth;
					Pixel = (Pixel + 1) & 1;
					NewSpanPtr++;
				}
			}
			else
			{
				/* span is different than reference buffer
					by mode bytes (-3 to 3) */
				intf i = GetRefCount();
				if (CurrentRefLoc + i + mode - ReadWidth > 0)
				{
//GES((ges, "setting %d @ %d\n", CurrentRefLoc + i + mode - ReadWidth, Pixel))
					if(Pixel)
						SetPixelsBlack(CurrentRefLoc + i + mode - ReadWidth);
					else
						SetPixelsWhite(CurrentRefLoc + i + mode - ReadWidth);
				}
				NewSpanArray[NewSpanPtr] = ReadWidth;
				Pixel = (Pixel + 1) & 1;
				NewSpanPtr++;
			}
			/* move the reference pointer */
			CurrentRefLoc = ReadWidth;
			/* if we are in the wrong color, move to the correct
				one */
			//span_pixel = Pixel;
			CurrentRefLoc = SyncSpanArray();
			if (ReadWidth >= Width)
			{
				RowsRead++;
				if (Compression == 3 && (RowsRead < NumRows))
				{
					if(FillOrder == 1)
					{
						if (ReadEolMale() == -4) break;
					}
					else
					{
						if (ReadEolFemale() == -4) break;
					}
				}
				LastMask = Mask;
				LastSavedByte = SavedByte;
				LastRegPtr = RegPtr;
				LastBufferEnd = BufferEnd;
//GES((ges, "Sending row.\n"))
				if (OutputRow(OutputBuffer))
				{
					Valid = false;
					return;
				}
				BytePtr = 0;
				memset(OutputBuffer, 0, ((Width + 7) >> 3));
				memcpy(SpanArray, NewSpanArray, Width * sizeof(intf));
				break;
			}
		}
		/* if 2d compression, reserve last buffer for reference */
	}
	else /* 1d compression */
	{
		ReadWidth = 32767;
		while (1)
		{
			if (ReadWidth >= Width)
			{
				if (Compression == 2)
					ResetMask();
				ReadWidth = 0;
				Pixel = 0;
				obuf_mask_ptr = 8;
			}
			num_pixels = GetPixel();
			if (num_pixels < 0)
			{
				if (OutOfData) break;
// Trying something here
				RowsRead++;
				if (Compression == 3 && (RowsRead < NumRows))
				{
					if(FillOrder == 1)
					{
						if (ReadEolMale() == -4) break;
					}
					else
					{
						if (ReadEolFemale() == -4) break;
					}
				}
				LastMask = Mask;
				LastSavedByte = SavedByte;
				LastRegPtr = RegPtr;
				LastBufferEnd = BufferEnd;
//GES((ges, "Sending row.\n"))
				if (OutputRow(OutputBuffer))
				{
             	Valid = false;
               return;
            }
				BytePtr = 0;
				memset(OutputBuffer, 0, ((Width + 7) >> 3));
				if (Compression == 4 || (Compression == 3 && G3Opts & 1))
					memcpy(SpanArray, NewSpanArray, Width * sizeof(intf));
				break;
				//Valid = false;
				//return;
			}
//GES((ges, "setting %d @ %d\n", num_pixels, Pixel))
			if(Pixel)
				SetPixelsBlack(num_pixels);
			else
				SetPixelsWhite(num_pixels);
			if (Compression == 4 || (Compression == 3 &&	G3Opts & 1))
				NewSpanArray[NewSpanPtr++] = ReadWidth;
			Pixel = (Pixel + 1) & 1;
			if (ReadWidth >= Width)
			{
				RowsRead++;
				if (Compression == 3 && (RowsRead < NumRows))
				{
					if(FillOrder == 1)
					{
						if (ReadEolMale() == -4) break;
					}
					else
					{
						if (ReadEolFemale() == -4) break;
					}
				}
				LastMask = Mask;
				LastSavedByte = SavedByte;
				LastRegPtr = RegPtr;
				LastBufferEnd = BufferEnd;
//GES((ges, "Sending row.\n"))
				if (OutputRow(OutputBuffer))
				{
             	Valid = false;
					return;
            }
				BytePtr = 0;
				memset(OutputBuffer, 0, ((Width + 7) >> 3));
				if (Compression == 4 || (Compression == 3 && G3Opts & 1))
					memcpy(SpanArray, NewSpanArray, Width * sizeof(intf));
				break;
			}
		}
	}
	}
#if 0
	if (LookBackBuffer)
		delete [] LookBackBuffer;
	LookBackBuffer = 0;
	LookBackSize = LastBufferEnd - LastRegPtr;
	if (LookBackSize > 0)
	{
		LookBackBuffer = new char [LastBufferEnd - LastRegPtr];
		memcpy(LookBackBuffer, LastRegPtr, LookBackSize);
	}
#endif
	int32 bytes_used = (int32)(LastRegPtr - WorkBuffer);
	VBuffer.ReleaseBytes(bytes_used);
	InputPtr = InputEnd; // mark buffer used
}
void
G42CcittDecompressor::ResetMask(void)
{
	if (FillOrder == 1)
		Mask = 0;
	else
		Mask = 0x100;
}
intf
G42CcittDecompressor::GetPixel(void)
{
	/* read codes in.  if code >= 64, add codes until reaching a code
		less then 64.  return -4 if no code could be found, and -5 if
		we run out of bits.  The reason we return these high negitive
		numbers is because 2d codes go down to -3, and I wanted to
		be consistent with them.
	*/
	Pos = 0;
	NextPos = 0;
	TotalRepeat = 0;
	intf count, bit;
	if(FillOrder == 1)
	{
		do
		{
			/* this works like so.  The tables above are set up as
			a decision tree.  I get a bit, and use the tables to
			determine where to go next, or if I have reached the end
			of a valid code, or an error.  a positive code is the
			next table index, a -1 is a valid code, and a -4 is
			an error.  This table is generated by code in a function
			below this one.
			*/
			/* first index into table */
			/* set repeat to an error, in case it doesn't get changed */
			while (1)
			{
				/* get the bit */
				GET_CCITT_BIT_MALE(bit)
				if (OutOfData)
					return -4;
				/* get the next table index */
				NextPos = ccitt_table2[Pixel][Pos].next_node[bit];
				if (NextPos > 0)
				{
					/* go back and do it again */
					Pos = NextPos;
					continue;
				}
				if(NextPos == -1)
				{
					/* valid bit stream, get code */
					count = ccitt_table2[Pixel][Pos].node[bit];
				}
				else return(-4);
				/* done with loop */
				break;
			}
			/* sum repeats (except errors) */
			if (count >= 0) TotalRepeat += count;
			Pos = 0;
		} while (count >= 64);
	}
	else
	{
		do
		{
			/* this works like so.  The tables above are set up as
				a decision tree.  I get a bit, and use the tables to
				determine where to go next, or if I have reached the end
				of a valid code, or an error.  a positive code is the
				next table index, a -1 is a valid code, and a -4 is
				an error.  This table is generated by code in a function
				below this one.
				*/
			/* first index into table */
			/* set repeat to an error, in case it doesn't get changed */
			while (1)
			{
				/* get the bit */
				GET_CCITT_BIT_FEMALE(bit)
				if (OutOfData)
					return -4;
				/* get the next table index */
				NextPos = ccitt_table2[Pixel][Pos].next_node[bit];
				if (NextPos > 0)
				{
					/* go back and do it again */
					Pos = NextPos;
					continue;
				}
				if(NextPos == -1)
				{
					/* valid bit stream, get code */
					count = ccitt_table2[Pixel][Pos].node[bit];
				}
				else return(-4);
				/* done with loop */
				break;
			}
			/* sum repeats (except errors) */
			if (count >= 0) TotalRepeat += count;
         Pos = 0;
		} while (count >= 64);
	}
	intf ret_val = TotalRepeat;
	TotalRepeat = 0;
	return ret_val;
}
void
G42CcittDecompressor::SetPixelsBlack(intf num_pixels)
{
	ReadWidth += num_pixels;
	while(num_pixels)
	{
		if (BytePtr >= RowBytes)
			return;
		*(OutputBuffer + BytePtr) |= obuf_mask_b[obuf_mask_ptr];
		if(num_pixels <= obuf_mask_ptr)
		{
			obuf_mask_ptr -= num_pixels;
			num_pixels = 0;
		}
		else
		{
			num_pixels -= obuf_mask_ptr;
			obuf_mask_ptr = 0;
		}
		if(!obuf_mask_ptr)
		{
			obuf_mask_ptr = 8;
			BytePtr++;
		}
	}
}
void
G42CcittDecompressor::SetPixelsWhite(intf num_pixels)
{
	ReadWidth += num_pixels;
	if (BytePtr >= RowBytes)
		return;
	*(OutputBuffer + BytePtr) &= obuf_mask_w[obuf_mask_ptr];
	BytePtr += ((num_pixels - obuf_mask_ptr + 8) / 8);
	obuf_mask_ptr = ((num_pixels % 8) - obuf_mask_ptr);
	if (obuf_mask_ptr < 0)
		obuf_mask_ptr = -obuf_mask_ptr;
	else if (!obuf_mask_ptr)
		obuf_mask_ptr = 8;
	else
		obuf_mask_ptr = 8 - obuf_mask_ptr;
}
intf
G42CcittDecompressor::ReadEolMale(void)
{
	intf n0 = 0;
	while (1)
	{
		intf b;
		GET_CCITT_BIT_MALE(b)
		if (OutOfData)
			return -4;
		if (b == 0)
			n0++;
		else
		{
			if (n0 >= 11)
				break;
			n0 = 0;
		}
	}
	return 1;
}
intf
G42CcittDecompressor::ReadEolFemale(void)
{
	intf n0 = 0;
	while (1)
	{
		intf b;
		GET_CCITT_BIT_FEMALE(b)
		if (OutOfData)
			return -4;
		if (b == 0)
			n0++;
		else
		{
			if (n0 >= 11)
				break;
			n0 = 0;
		}
	}
   return 1;
}
intf
G42CcittDecompressor::Get2dModeMale(void)
{
   intf bit;
	GET_CCITT_BIT_MALE(bit)
	if (OutOfData)
		return -4;
	/* span is same as last one */
   if (bit)
      return (0);
	GET_CCITT_BIT_MALE(bit)
	if (OutOfData)
		return -4;
	if (bit)
	{
		GET_CCITT_BIT_MALE(bit)
		if (OutOfData)
			return -4;
		/* span is one larger (or smaller) then last one */
		if (bit)
			return (1);
		return (-1);
	}
	GET_CCITT_BIT_MALE(bit)
	if (OutOfData)
		return -4;
   /* next two spans are incoded in 1d */
	if (bit)
      return (5);
	GET_CCITT_BIT_MALE(bit)
	if (OutOfData)
		return -4;
   /* span skips next two spans in reference buffer */
   if (bit)
      return (4);
	GET_CCITT_BIT_MALE(bit)
	if (OutOfData)
		return -4;
   if (bit)
   {
		GET_CCITT_BIT_MALE(bit)
		if (OutOfData)
			return -4;
      /* span is one larger (or smaller) then last one */
      if (bit)
         return (2);
      return (-2);
	}
	GET_CCITT_BIT_MALE(bit)
	if (OutOfData)
		return -4;
   if (bit)
   {
      GET_CCITT_BIT_MALE(bit)
		if (OutOfData)
			return -4;
      /* span is one larger (or smaller) then last one */
      if (bit)
         return (3);
      return (-3);
   }
   /* invalid span */
	return (-4);
}
intf
G42CcittDecompressor::Get2dModeFemale(void)
{
	intf bit;
	GET_CCITT_BIT_FEMALE(bit)
	if (OutOfData)
		return -4;
   /* span is same as last one */
   if (bit)
      return (0);
	GET_CCITT_BIT_FEMALE(bit)
	if (OutOfData)
		return -4;
   if (bit)
   {
		GET_CCITT_BIT_FEMALE(bit)
		if (OutOfData)
			return -4;
      /* span is one larger (or smaller) then last one */
      if (bit)
         return (1);
      return (-1);
   }
	GET_CCITT_BIT_FEMALE(bit)
	if (OutOfData)
		return -4;
	/* next two spans are incoded in 1d */
   if (bit)
      return (5);
	GET_CCITT_BIT_FEMALE(bit)
	if (OutOfData)
		return -4;
   /* span skips next two spans in reference buffer */
   if (bit)
      return (4);
	GET_CCITT_BIT_FEMALE(bit)
	if (OutOfData)
		return -4;
   if (bit)
   {
		GET_CCITT_BIT_FEMALE(bit)
		if (OutOfData)
			return -4;
      /* span is one larger (or smaller) then last one */
      if (bit)
         return (2);
		return (-2);
   }
	GET_CCITT_BIT_FEMALE(bit)
	if (OutOfData)
		return -4;
	if (bit)
   {
      GET_CCITT_BIT_FEMALE(bit)
		if (OutOfData)
			return -4;
      /* span is one larger (or smaller) then last one */
		if (bit)
         return (3);
      return (-3);
   }
   /* invalid span */
	return (-4);
}
intf
G42CcittDecompressor::GetRefCount(void)
{
	if(CurrentRefLoc >= Width) return(0);
	intf count = SpanArray[++CurrentSpanPtr];
	return(count - CurrentRefLoc);
}
intf
G42CcittDecompressor::SyncSpanArray(void)
{
	if(CurrentRefLoc >= Width) return(0);
	CurrentSpanPtr--;
	if(CurrentSpanPtr < 0) CurrentSpanPtr = 0;
	while(CurrentRefLoc >= SpanArray[CurrentSpanPtr]) CurrentSpanPtr++;
	CurrentSpanPtr--;
	if(CurrentSpanPtr < 0) CurrentSpanPtr = 0;
	if((Pixel && !(CurrentSpanPtr & 0x01)) || (!Pixel && (CurrentSpanPtr & 0x01)))
	{
		CurrentSpanPtr++;
		return(SpanArray[CurrentSpanPtr]);
	}
	return(CurrentRefLoc);
}

// gidccitt.h - CCITT Decompression
/*************************************************************
	File:          gidccitt.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   Class that handles ccitt compression
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#ifndef GIDCCITT_H
#define GIDCCITT_H
#include "gidcomp.h"
#include "grbuf.h"
/* 2d special codes */
#define CCITT_HORIZ 5
#define CCITT_PASS 4
/* used to write 2d modes */
#define CCITT_2D 2
class G42CcittDecompressor : public G42Decompressor
{
	public:
		G42CcittDecompressor(G42DecompressorOutput & output, intf width,
			intf compression, intf g3opts, intf fillorder, intf num_rows);
		virtual ~G42CcittDecompressor(void);
		virtual void ProcessBuffer(void);
	private:
		class ccitt_node
		{
     	public:
			intf next_node[2]; /* next index to jump to: -1 if valid code */
			intf node[2];	  /* code */
		};
		intf Width;
		intf Compression;
		intf G3Opts;
      intf FillOrder;
		intf Mask;
		char SavedByte;
		intf LastMask;
		char LastSavedByte;
		intf ReadWidth;
		intf Pixel;
		char * BufferEnd;
		char * RegPtr;
		char * LastBufferEnd;
		char * LastRegPtr;
      intf LookBackSize;
		char * LookBackBuffer;
      char * WorkBuffer;
		intf Pos;
		intf NextPos;
		intf TotalRepeat;
		intf obuf_mask_ptr;
		bool OutOfData;
		intf * SpanArray;
		intf * NewSpanArray;
		intf CurrentRefLoc;
		intf CurrentSpanPtr;
		intf NewSpanPtr;
		intf RowsRead;
		intf NumRows;
		bool FirstTime;
		bool Valid;
		uint32 BytePtr;
		G42VirtualBuffer VBuffer;
		static char obuf_mask_b[9];
		static char obuf_mask_w[9];
		static ccitt_node ccitt_table2[2][104];
		void ResetMask(void);
      intf GetPixel(void);
		void SetPixelsBlack(intf num_pixels);
		void SetPixelsWhite(intf num_pixels);
		intf ReadEolMale(void);
		intf ReadEolFemale(void);
		intf Get2dModeMale(void);
		intf Get2dModeFemale(void);
		intf GetRefCount(void);
		intf SyncSpanArray(void);
};
#endif // GIDCCITT_H

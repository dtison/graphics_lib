// g42ccitt.h - CCITT Compression
/*************************************************************
	File:          g42ccitt.h
   Copyright (c) 1996, Group 42, Inc.
	Description:   Class that handles ccitt compression
   Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
   Modification History:
      Code   Date   Name and Description
***************************************************************/
#ifndef G42_CCITT_H
#define G42_CCITT_H
#include "g42itype.h"
#include "g42comp.h"
/* 2d special codes */
#define CCITT_HORIZ 5
#define CCITT_PASS 4
/* used to write 2d modes */
#define CCITT_2D 2
class G42CcittCompression : public G42Compression
{
	public:
		G42CcittCompression(int width, int compression, int g3opts,
			int fillorder, int num_rows);
		virtual ~G42CcittCompression();
		virtual void ProcessBuffer(byte * buffer, int buffer_size);
	private:
		class ccitt_node
		{
     	public:
			int next_node[2]; /* next index to jump to: -1 if valid code */
			int node[2];	  /* code */
		};
		int Width;
		int Compression;
		int G3Opts;
      int FillOrder;
		int Mask;
		char SavedByte;
		int LastMask;
		char LastSavedByte;
		int ReadWidth;
		int Pixel;
		char * BufferEnd;
		char * RegPtr;
		char * LastBufferEnd;
		char * LastRegPtr;
      int LookBackSize;
		char * LookBackBuffer;
      char * WorkBuffer;
		int Pos;
		int NextPos;
		int TotalRepeat;
		int obuf_mask_ptr;
		bool OutOfData;
		int * SpanArray;
		int * NewSpanArray;
		int CurrentRefLoc;
		int CurrentSpanPtr;
		int NewSpanPtr;
		int RowsRead;
		int NumRows;
      bool FirstTime;
		static char obuf_mask_b[9];
		static char obuf_mask_w[9];
		static ccitt_node ccitt_table2[2][104];
		void ResetMask(void);
      int GetPixel(void);
		void SetPixelsBlack(int num_pixels);
		void SetPixelsWhite(int num_pixels);
		int ReadEolMale(void);
		int ReadEolFemale(void);
		int Get2dModeMale(void);
		int Get2dModeFemale(void);
		int GetRefCount(void);
      int SyncSpanArray(void);
};
#endif // G42_CCITT_H

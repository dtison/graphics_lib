// g42tiff.h - tiff version of the raster file base class
#ifndef G42TIFF_H
#define G42TIFF_H
/*************************************************************
	File:          g42tiff.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   class for tiff's derived off the raster base class.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
      Code   Date   Name and Description
***************************************************************/
#include "g42ribas.h"
#include "g42comp.h"
#include "g42lzw.h"
#include "g42pbits.h"
#include "g42idata.h"
#include "g42ccitt.h"
class G42TiffLzw;
class G42TiffPackBits;
class G42TiffNoCompression;
class G42TiffCcitt;
class G42TiffImage : public G42RasterBase
{
	friend class G42TiffLzw;
	friend class G42TiffPackBits;
	friend class G42TiffNoCompression;
	friend class G42TiffCcitt;
	public:
		void ProcessData(byte * buffer, size_t buffer_size);
		G42TiffImage(G42RasterImage & parent);
		virtual ~G42TiffImage();
      bool ShouldAbort(void) const
      	{return (ReadMode != ReadImageRowsMode);}
      virtual void ForceFileEnd(void); // force image to be finished
	protected:
		int LastStoreBlock;
		int MaxStoreBlock;
//		G42ImageTile ** StoreBlockArray;
		byte ** StoreBlockArray;
      uint16 StoreBufferSize;
		uint16 StoreBufferNext;
		uint16 StoreBufferLeft;
		uint32 TotalFileRead;
		uint32 WaitOffset;
		uint32 IFDOffset;
		uint32 StripOffset;
      uint32 TiffSkip;
		uint32 ArrayOffset;
      uint32 StripBytesLeft;
		uint32 * StripSizeArray;
		uint32 * StripArray;
		int StripRow;
		int CurrentRow;
		uint Tag;
		uint Type;
		uint Count;
		G42Compression * Uncompress;
		uint RowsPerStrip;
		bool NeedMoreData;
		int NumIFDEntries;
		int CurIFDEntry;
		int BitsPerSample;
		int Compression;
		int ColorType;
		int FillOrder;
		int SamplesPerPixel;
		int T4Options;
		int T6Options;
		int Predictor;
		int ExtraSamples;
		int CurrentStrip;
		int TrueRowBytes;
		int InternalRowBytes;
		int TiffDepth;
		byte * TiffRowBuffer;
		size_t OrigBufferSize;
      byte * OrigBuffer;
		enum
		{
			TagImageWidth = 256,
			TagImageLength = 257,
			TagBitsPerSample = 258,
			TagCompression = 259,
			TagPhotometricInterpretation = 262,
			TagFillOrder = 266,
			TagStripOffsets = 273,
			TagSamplesPerPixel = 277,
			TagRowsPerStrip = 278,
			TagStripByteCounts = 279,
			TagT4Options = 292,
			TagT6Options = 293,
			TagPredictor = 317,
			TagColorMap = 320
		};
		enum
		{
			TypeByte = 1,
			TypeAscii = 2,
			TypeShort = 3,
			TypeLong = 4,
			TypeRational = 5,
			TypeSByte = 6,
			TypeUndefined = 7,
			TypeSShort = 8,
			TypeSLong = 9,
			TypeSRational = 10,
			TypeFloat = 11,
			TypeDouble = 12
		};
		enum
		{
			ColorTypeWhiteIsZero = 0,
			ColorTypeBlackIsZero = 1,
			ColorTypeRGB = 2,
         ColorTypePalette = 3
		};
		enum
		{
			NoCompression = 1,
			CCITT = 2,
			Group3 = 3,
			Group4 = 4,
			LZW = 5,
			PackBits = 32773
		};
		enum
		{
			PredictionNone = 1,
			PredictionHorizontal = 2
		};
		enum G42ReadMode {ReadHeaderMode, ReadImageRowsMode, ReadImageStripMode,
			ReadWaitMode, ReadIFDMode, ReadIFDEntryMode, ReadStripOffsetsMode,
			ReadBitsPerSampleMode, ReadStripByteCountsMode, ReadColorMapMode,
         ReadTiffHeaderMode,
			ReadDoneMode, ReadErrorMode};
		G42ReadMode ReadMode;
		G42ReadMode WaitMode;
		void ProcessRow(byte * row);
		virtual void OutputRow(void);
		void ReadHeader(void);
		void ReadTiffHeader(void);
		void ReadIFD(void);
		void ReadIFDEntry(void);
		void ReadStripOffsets(void);
		void ReadColorMap(void);
		void ReadStripByteCounts(void);
		void ReadBitsPerSample(void);
		int32 GetTiffValue(void);
		int GetValueSize(void);
		bool NeedTiffPtr(void);
		void FinishIFDEntry(void);
		void ReadImageStrip(void);
		void ReadImageRows(void);
		bool LoadBuffer(uint32 offset, uint32 buffer_size);
		void AddBuffer(byte * buffer, uint32 size);
      void ClearBuffer(void);
};
class G42TiffLzw : public G42Lzw
{
	protected:
		G42TiffImage * Parent;
	public:
		G42TiffLzw(G42TiffImage * parent, int width) :
			Parent(parent), G42Lzw(width, LzwTiff, 8)
		{
		}
		~G42TiffLzw(){}
		bool OutputRow(byte * line)
		{
			Parent->ProcessRow(line);
         return Parent->ShouldAbort();
		}
};
class G42TiffPackBits: public G42PackBits
{
	protected:
		G42TiffImage * Parent;
	public:
		G42TiffPackBits(G42TiffImage * parent, int width) :
			Parent(parent), G42PackBits(width)
		{
		}
		~G42TiffPackBits(){}
		bool OutputRow(byte * line)
		{
			Parent->ProcessRow(line);
         return Parent->ShouldAbort();
		}
};
class G42TiffNoCompression : public G42NoCompression
{
	protected:
		G42TiffImage * Parent;
	public:
		G42TiffNoCompression(G42TiffImage * parent, int width) :
			Parent(parent), G42NoCompression(width)
		{
		}
		~G42TiffNoCompression(){}
		bool OutputRow(byte * line)
		{
			Parent->ProcessRow(line);
         return Parent->ShouldAbort();
		}
};
class G42TiffCcitt : public G42CcittCompression
{
	protected:
		G42TiffImage * Parent;
	public:
		G42TiffCcitt(G42TiffImage * parent, int width,
			int compression, int g3opts, int fillorder, uint rows) :
			Parent(parent), G42CcittCompression(width, compression, g3opts,
				fillorder, rows)
		{
		}
		~G42TiffCcitt(){}
		bool OutputRow(byte * line)
		{
			Parent->ProcessRow(line);
         return Parent->ShouldAbort();
		}
};
extern bool G42IsTiff(byte * buffer, size_t buffer_size);
extern bool G42IsEps(byte * buffer, size_t buffer_size);
#endif // G42BMP_H

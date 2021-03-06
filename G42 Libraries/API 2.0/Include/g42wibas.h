// g42ribas.h - raster image base class
#ifndef G42_WIBASE_H
#define G42_WIBASE_H
/*************************************************************
	File:          g42ribas.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   Base class for reading raster files
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
   Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "g42wimag.h" // We will be refering to the factory class
class G42RasterWriterBase
{
	public:
		G42RasterWriterBase(G42RasterImageWriter & parent);
		virtual ~G42RasterWriterBase();
		bool IsValid(void) const
			{ return (Valid); }
		virtual bool WriteInfo(G42ImageInfo * info) = 0;
		virtual bool WriteRow(const byte * row, const byte * mask);
		virtual uint32 NextRow(void);
		virtual bool NeedRow(void);
		virtual bool WriteEnd(G42ImageInfo * info);
		virtual int GetDepth(void);
		enum G42IntType {Motorola, Intel, Default}; // type of integers to read
		void PutBytes(const byte * buffer, uint size = 1);
		void PutByteRun(byte value, uint number);
		void PutUInt32(uint32 number, G42IntType type = Default); // write a 32 bit integer
		void PutUInt16(uint16 number, G42IntType type = Default); // write a 16 bit integer
		void PutByte(byte number)
		{
			*NextOutput++ = number;
			if (--OutputBufferLeft == 0)
				FlushBuffer();
		}
	protected:
		G42RasterImageWriter & Parent;
		uint32 Width;
		uint32 Height;
		uint32 RowBytes;
		uint32 Depth;
		uint32 CurrentRow;
		bool Valid;
		void FlushBuffer(void);  // PutEnd will automatically do this
		void PutEnd(void);
		G42IntType IntegerFormat; // this variable holds what integer type to
										  // use if Default
		byte * OutputBuffer;
		uint OutputBufferSize;
		uint OutputBufferLeft;
      byte * NextOutput;
};
#endif // G42_RIBASE_H

// g42ribas.h - raster image base class
#ifndef G42_RIBASE_H
#define G42_RIBASE_H
/*************************************************************
	File:          g42ribas.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   Base class for reading raster files
   Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
   Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "g42rimag.h" // We will be refering to the factory class
// This acts as a base class and does much of the general work
// of a raster file.  The only thing it does not do is read in
// the header and palette.  The rest can be done all through here,
// although the derived class can chose whether to use the capabilities
// presented here or not.
class G42RasterBase
{
   public:
		G42RasterBase(G42RasterImage & parent);
      virtual ~G42RasterBase();
      virtual bool IsValid(void);
		virtual bool IsFinished(void);
		// ProcessData needs to be provided by _all_ derived classes
		virtual void ProcessData(byte * buffer, size_t buffer_size) = 0;
		virtual uint32 GetWidth(void);
		virtual uint32 GetHeight(void);
		virtual uint GetDepth(void);
		virtual uint GetNumPalette(void);
		virtual G42Color * GetPalette(void);
		virtual bool IsColor(void);
		virtual bool TopToBottom(void);
		virtual bool IsTransparent(void);
		virtual void CombineRow(byte * old_row, byte * new_row, int32 rowbytes);
		virtual void CombineMask(byte * old_row, byte * new_row, int32 rowbytes);
		G42ImageInfo & GetInfo(void);
		void FillInfo(void);
      virtual void ForceFileEnd(void); // force image to be finished
	protected:
		G42ImageInfo Info; // hold image info
		G42RasterImage & Parent; // parent for callbacks
		bool Valid; // leave true unless an error occurs
		bool Finished; // turn to true when finished
		uint32 Width;
		uint32 Height;
		uint Depth;
		uint32 RowBytes;
		bool Color;
		bool TopLineFirst; // commonly true, set to false if image arrives
								 // bottom to top.
		bool Transparent;
		G42Color * ImagePalette;
		uint NumPalette;
		uint32 CurRowNum;
		size_t NumSavedBuffer; // number of bytes saved from earlier
		size_t MaxSavedBuffer; // maximum number of bytes before re-newing
		size_t BufferSize; // number of bytes available to read
		size_t BufferLeft; // number of bytes left that haven't been used
		size_t SavedBufferLeft; // number of unused bytes in saved buffer
		byte * SavedBuffer; // bytes saved from earlier
		byte * CurrentBuffer; // buffer of data that needs processed
		byte * NextByte; // where the next unprocessed byte is
		void RestoreBuffer(byte * buffer, size_t buffer_size); // set up buffers
		void FillBuffer(byte * buffer, int size);
		byte GetByte(void) { // get a byte - inline for speed
			if (SavedBufferLeft)
			{
				byte b = *NextByte++;
				SavedBufferLeft--;
				BufferSize--;
				if (!SavedBufferLeft)
            {
               NextByte = CurrentBuffer;
            }
            return b;
         }
			else if (BufferLeft)
			{
            BufferLeft--;
            BufferSize--;
            return *NextByte++;
         }
			else
				return 0;
		}
		void SkipData(int count = 1) { // skip data in buffer
			if (count && SavedBufferLeft)
			{
				uint saved_count = min((uint)count, SavedBufferLeft);
				NextByte += saved_count;
				SavedBufferLeft -= saved_count;
				BufferSize -= saved_count;
				count -= saved_count;
				if (!SavedBufferLeft)
				{
					NextByte = CurrentBuffer;
				}
			}
			if (count && BufferLeft)
			{
				uint buffer_count = min((uint)count, BufferLeft);
				BufferLeft -= buffer_count;
				BufferSize -= buffer_count;
				NextByte += buffer_count;
			}
		}
		enum G42IntType {Motorola, Intel, Default}; // type of integers to read
		G42IntType IntegerFormat; // this variable holds what integer type to
										  // use if Default
		uint32 GetUInt32(G42IntType type = Default); // read a 32 bit integer
		uint16 GetUInt16(G42IntType type = Default); // read a 16 bit integer
		void SaveBuffer(void); // save the remaining buffer for next time
		byte * RowBuffer; // processed row buffer
		byte * RowPtr; // where the next pixel will be placed
		size_t RowBytesLeft; // space remaining in the row buffer
		size_t RowSize; // total size of row
		void ProcessUncompressedData(); // processes uncompressed rows of pixels
		void InitRow(void); // prepare row buffers
		virtual void OutputRow(void); // about to output row to parent
		virtual void EndPass(void); // finishes a one pass file
		void PutByte(byte value) { // put a byte into the row buffer
			if (!RowPtr) InitRow();
			*RowPtr++ = value;
			if (!(--RowBytesLeft))
				OutputRow();
      }
		void PutByteRun(byte value, int number); // put a run of bytes
		void PutSequence(byte *buffer, int buffer_size); // put a sequence of bytes
		void EatBuffer(); // eat the remaining bytes in the input buffer
};
#endif // G42_RIBASE_H

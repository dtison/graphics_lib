// g42comp.h - base class for compression classes
/*************************************************************
	File:          g42comp.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   Base class for compression classes
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#ifndef G42COMP_H
#define G42COMP_H
class G42Compression
{
	public:
		G42Compression(int row_bytes);
		virtual ~G42Compression();
		virtual void ProcessBuffer(byte * buffer, int buffer_size) = 0;
		virtual bool OutputRow(byte * line);
		bool IsDone(void) const {return Done;}
		bool IsValid(void) const {return Valid;}
		void PutByte(byte b);
		void PutSequence(byte * buffer, uint length);
		void PutRun(byte b, uint number);
	protected:
		int RowBytes;
		bool Done;
		bool Valid;
		byte * LineBuffer;
		uint BytePtr;
};
inline void
G42Compression::PutByte(byte b)
{
	LineBuffer[(BytePtr)++] = b;
	if (BytePtr >= RowBytes)
	{
		if (OutputRow(LineBuffer))
      	Done = true;
		BytePtr = 0;
	}
}
class G42NoCompression : public G42Compression
{
	public:
		G42NoCompression(int width);
		virtual ~G42NoCompression();
		virtual void ProcessBuffer(byte * buffer, int buffer_size);
};
#endif // G42COMP_H

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
#ifndef G42WCOMP_H
#define G42WCOMP_H
class G42WriteCompressionOutput
{
	public:
		G42WriteCompressionOutput();
		virtual ~G42WriteCompressionOutput();
		virtual void OutputBuffer(byte * buffer, uint length) = 0;
};
class G42WriteCompression
{
	public:
		G42WriteCompression(G42WriteCompressionOutput * output,
			uint output_buffer_size = 4096);
		virtual ~G42WriteCompression();
		virtual void ProcessBuffer(byte * buffer, uint buffer_size) = 0;
		virtual void Flush(void);
		bool IsDone(void) const {return Done;}
		bool IsValid(void) const {return Valid;}
	protected:
		void HandleOutputBuffer(byte * buffer, uint buffer_size);
		void PutByte(byte b);
		void PutSequence(byte * buffer, uint length);
		void PutRun(byte b, uint number);
		G42WriteCompressionOutput * Output;
		int RowBytes;
		bool Done;
		bool Valid;
		byte * OutputBuffer;
		uint OutputBufferSize;
		uint BytePtr;
};
inline void
G42WriteCompression::PutByte(byte b)
{
	OutputBuffer[(BytePtr)++] = b;
	if (BytePtr >= OutputBufferSize)
	{
		HandleOutputBuffer(OutputBuffer, OutputBufferSize);
		BytePtr = 0;
	}
}
class G42WriteNoCompression : public G42WriteCompression
{
	public:
		G42WriteNoCompression(G42WriteCompressionOutput * output,
			uint output_buffer_size = 4096);
		virtual ~G42WriteNoCompression();
		virtual void ProcessBuffer(byte * buffer, uint buffer_size);
};
#endif // G42COMP_H

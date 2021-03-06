// gicomp.h - base class for compression classes
/*************************************************************
	File:          g42comp.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   Base class for compression classes
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#ifndef GICOMP_H
#define GICOMP_H
#include "gtype.h"
class G42CompressorOutput
{
	public:
		G42CompressorOutput();
		virtual ~G42CompressorOutput();
		virtual void CompressorOutputBuffer(byte * buffer, uintf length) = 0;
};
class G42Compressor
{
	public:
		G42Compressor(G42CompressorOutput & output,
			uintf output_buffer_size = 4096);
		virtual ~G42Compressor();
		virtual void ProcessBuffer(byte * buffer, uintf buffer_size) = 0;
		virtual void Flush(void);
		bool IsDone(void) const {return Done;}
		bool IsValid(void) const {return Valid;}
	protected:
		void HandleOutputBuffer(byte * buffer, uintf buffer_size)
			{ Output.CompressorOutputBuffer(buffer, buffer_size); }
		void PutByte(byte b);
		void PutSequence(byte * buffer, uintf length);
		void PutRun(byte b, uintf number);
		G42CompressorOutput & Output;
		bool Done;
		bool Valid;
		byte * OutputBuffer;
		uintf OutputBufferSize;
		uintf BytePtr;
};
inline void
G42Compressor::PutByte(byte b)
{
	OutputBuffer[(BytePtr)++] = b;
	if (BytePtr >= OutputBufferSize)
	{
		HandleOutputBuffer(OutputBuffer, OutputBufferSize);
		BytePtr = 0;
	}
}
class G42NoCompressor : public G42Compressor
{
	public:
		G42NoCompressor(G42CompressorOutput & output,
			uintf output_buffer_size = 4096);
		virtual ~G42NoCompressor();
		virtual void ProcessBuffer(byte * buffer, uintf buffer_size);
};
#endif // G42COMP_H

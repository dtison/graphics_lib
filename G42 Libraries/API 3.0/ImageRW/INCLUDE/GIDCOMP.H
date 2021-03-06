// gidcomp.h - base class for decompression classes
/*************************************************************
	File:          gidcomp.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   Base class for decompression classes
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
				Jan 15 1997 Guy	Converted to 3.0 class
***************************************************************/
#if !defined(GIDCOMP_H)
#define GIDCOMP_H
#include "gtype.h"
class G42DecompressorOutput
{
public:
	G42DecompressorOutput() {}
	~G42DecompressorOutput() {}
	virtual bool DecompressorOutputRow(byte * row) = 0;
};
class G42Decompressor
{
	public:
		 // note: some decompressors use width in their constructors instead
		 // of rowbytes, but this base class constructor must be called
		 // with the correct rowbytes, else it won't know when a row is done
		G42Decompressor(G42DecompressorOutput & output, uintf row_bytes);
		virtual ~G42Decompressor();
		// set where output buffer goes, if same buffer, no need to reset
		virtual void SetOutputBuffer(byte * buffer);
		uintf ProcessBuffer(const byte * buffer, uintf buffer_size);
		virtual void ProcessBuffer(void) = 0;
		bool IsDone(void) const {return Done;}
		void Flush(void)
			{ if (OutputPtr != OutputBuffer) OutputRow(); }
	protected:
		byte GetByte(void)
			{ return *InputPtr++; }
		bool InputBufferLeft(void)
			{ return (InputPtr != InputEnd); }
		// inlined below for speed
		void PutByte(byte b);
		void PutSequence(const byte * buffer, uintf length);
		void PutRun(byte b, uintf length);
		void CheckOutput(void); // sends row if necessary, sets Done, etc.
		bool OutputRow(byte *) // for backwards compatability and simplicity
			{ Done = Output.DecompressorOutputRow(OutputBuffer); return Done; }
		bool OutputRow(void) // for backwards compatability and simplicity
			{ Done = Output.DecompressorOutputRow(OutputBuffer); return Done; }
		uintf RowBytes;
		bool Done;
		byte * OutputBuffer;
		byte * OutputEnd;
		byte * OutputPtr;
		G42DecompressorOutput & Output;
		const byte * InputPtr;
		const byte * InputEnd;
};
inline void
G42Decompressor::CheckOutput(void)
{
	if (OutputPtr == OutputEnd)
	{
		if (OutputRow())
			Done = true;
		OutputPtr = OutputBuffer;
	}
}
inline void
G42Decompressor::PutByte(byte b)
{
	*OutputPtr++ = b;
	CheckOutput();
}
class G42NoDecompressor : public G42Decompressor
{
	public:
		G42NoDecompressor(G42DecompressorOutput & output, uintf row_bytes);
		virtual ~G42NoDecompressor();
		virtual void ProcessBuffer();
};
#endif // GIDCOMP_H

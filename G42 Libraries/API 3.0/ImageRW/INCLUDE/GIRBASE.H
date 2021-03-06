// girbase.h - G42ImageReaderBase
#if !defined(GIRBASE_H)
#define GIRBASE_H
#include "gitype.h"
#include "girwexc.h"
#include "gidesc.h"
#include "gidata.h"
#include "giadesc.h"
#include "giaframe.h"
#include "grbase.h"
#include "gidcomp.h"
#include "girwopt.h"
/*
	While each of the file types built off of this has it's own type id,
	I'm keeping the master list here, so we be sure we don't reuse a number.
	When you create a new type, simply copy the id over to here.  The first
	few will be in alpha order, but the rest don't need to be.  Don't change
	these numbers.  Add on any new ones to the end of the list.
const intf G42BmpTypeId = 0x101;
const intf G42GifTypeId = 0x102;
const intf G42JpegTypeId = 0x103;
const intf G42PbmTypeId = 0x104;
const intf G42PcxTypeId = 0x105;
const intf G42RasTypeId = 0x106;
const intf G42TiffTypeId = 0x107;
const intf G42TargaTypeId = 0x108;
const intf G42XwdTypeId = 0x109;
const intf G42PngTypeId = 0x10a;
const intf G42EpsTypeId = 0x10b;
*/
// this will need #if define (G42_IN_RW_DLL) at times
class G42ImageReaderBase : public G42BaseReader, public G42DecompressorOutput
{
public:
	G42ImageReaderBase(void);
	G42ImageReaderBase(G42HeapManager & heap, G42Notifier & notifier,
		G42File * file = 0, G42BaseOptions * options = 0);
	~G42ImageReaderBase(void);
	virtual bool IsFinished(void);
	void SetInfo(intf type, const char * extension, const char * name,
		const char * pattern, intf pattern_length, bool last = false);
	virtual void ProcessStreamData(void) = 0; // new process data call
//	virtual void SendExtendedInfo(void) = 0;
	virtual uint32 ProcessData(const byte * buffer, uint32 buffer_size);
	virtual void TerminateStream(void)
		{ ForceFileEnd(); } // translate to 2.0 call
	virtual bool TestType(void)
		{ return true; }
	virtual bool TestType(const byte * /* buffer */, uint32 /* buffer_size */)
		{ return true; }
	virtual bool DecompressorOutputRow(byte * row);
protected:
	virtual void ForceFileEnd(void); // force image to be finished
	// various stuff to simplify reading, mostly from 2.0 Base class
	bool CheckBuffer(uint32 size, bool force = false); // prepares a buffer, returns false if can't
	bool Finished; // turns to true when finished
	uint32 Width;
	uint32 Height;
	uintf Depth; // the depth of the file, not of the image data
	uint32 RowBytes; // the rowbytes of the file, not of the image data
	bool Color; // true if color or paletted
	bool TopLineFirst; // commonly true, set to false if image arrives
								 // bottom to top.
	bool Transparent; // true if file is transparent
	bool AlphaChannel; // true if file has an alpha channel, Transparent must be true
	bool Background; // true if file has a Background Color/Pixel
	intf BackgroundPixel; // 0 - 255 if set, -1 if not set
	intf TransparentPixel; // 0 - 255 if set, -1 if not set
	G42Color BackgroundColor;
	G42Color TransparentColor;
	uintf NumPalette; // number of colors if file is paletted
	uintf NumPasses; // commonly 1, if 0 - unknown number of passes
	bool Compressed; // defaults to false
	bool Progressive; // defaults to false
	byte TrueColorDepth; // defaults to 24
	G42ImageReadOptions * ReadOptions; // merely a dynamic caste from Options in base class
	G42Ptr<G42ImageReadWriteOptions> WriteOptions;
	bool WriteOptionsSent;
	void GetReadOptions(void);
	uintf GetImageDepth(void) // returns the depth of the image data
		{ return (Descriptor ? Descriptor->GetDepth() : 0); }
	void SetupDescriptor(void); // creates the info and sets the palette
	void SendDescriptor(void); // sends the info to the user;
	G42Color * ImagePalette;
	// we'll need something to handle comments here
	// we'll need something similar for multi image files
	void SetupMultiDescriptor(void); // creates the info and sets the palette
	void SendMultiDescriptor(void); // sends the info to the user;
	G42Color * MultiPalette;
	uint16 NumMultiPalette; // number of colors if file is paletted
	bool MultiBackground; // true if file has a Background Color/Pixel
	intf MultiBackgroundPixel; // 0 - 255 if set, -1 if not set
	G42Color MultiBackgroundColor;
	uint32 MultiNumImages;
	void SetupAnimatedDescriptor(void); // creates the info and sets the palette
	void SendAnimatedDescriptor(void); // sends the info to the user;
	uint32 AnimatedWidth;
	uint32 AnimatedHeight;
	bool AnimatedRepeat;
	void SendAnimatedFrame(void); // sends the info to the user;
	G42Point FrameOffset;
	int32 FrameDelay;
	bool FrameUserInput;
	G42AnimatedImageFrame::G42Replace FrameReplace;
	G42Ptr<G42MultipleImageDescriptor> MultiDescriptor;
	bool MultiDescriptorSent;
	uint32 CurrentImage;
	bool MultiImageMode;
	virtual void NextImage(void);
	uint32 CurRowNum; // counts rows
	uintf CurPass; // current pass
	size_t NumSavedBuffer; // number of bytes saved from earlier
	size_t MaxSavedBuffer; // maximum number of bytes before re-newing
	size_t BufferSize; // number of bytes available to read
	size_t BufferLeft; // number of bytes left that haven't been used
	size_t SavedBufferLeft; // number of unused bytes in saved buffer
	byte * SavedBuffer; // bytes saved from earlier
	const byte * CurrentBuffer; // buffer of data that needs processed
	const byte * NextByte; // where the next unprocessed byte is
	void RestoreBuffer(const byte * buffer, size_t buffer_size); // set up buffers
	void FillBuffer(byte * buffer, int size);
	byte GetByte(void)
	{ // get a byte - inline for speed
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
		else if (File)
		{
			byte b = 0;
			File->Read(&b, 1, false);
			return b;
		}
		else
		{
			return 0;
		}
	}
	void SkipData(int count = 1)
	{ // skip data in buffer
		if (count && SavedBufferLeft)
		{
			uintf saved_count = min((uintf)count, SavedBufferLeft);
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
			uintf buffer_count = min((uintf)count, BufferLeft);
			BufferLeft -= buffer_count;
			BufferSize -= buffer_count;
			NextByte += buffer_count;
		}
		if (count && File)
		{
			File->SetPosition(count, G42File::FromCurrent);
		}
	}
	enum G42IntType {Motorola, Intel, Default}; // type of integers to read
	G42IntType IntegerFormat; // this variable holds what integer type to
									  // use if Default
	uint32 GetUInt32(G42IntType type = Default); // read a 32 bit integer
	uint16 GetUInt16(G42IntType type = Default); // read a 16 bit integer
	void SaveBuffer(void); // save the remaining buffer for next time
	class G42LockPtr
	{
	public:
		G42LockPtr(G42ImageReaderBase & parent, G42ImageData & data) :
			Parent(parent), Lock(0)
		{
			Lock = Parent.GetWriteImageLock(&data);
		}
		~G42LockPtr(void)
		{
			Parent.ClearWriteImageLock(Lock);
			Lock = 0;
		}
		byte * GetRow(uint32 row, bool trans = false)
		{
			return Parent.GetRowFromLock(Lock, row, trans);
		}
	private:
		G42WriteImageData * Lock;
		G42ImageReaderBase & Parent;
	};
	byte * RowBuffer; // processed row buffer
	bool DeleteRowBuffer;
	byte * RowPtr; // where the next pixel will be placed
	size_t RowBytesLeft; // space remaining in the row buffer
	size_t RowSize; // total size of row
	byte * OutputRowPtr;
	byte * OutputTransRowPtr;
	uint32 CurStripRow;
	uint32 CurRow;
	uint32 CurStrip;
	G42Ptr<G42ImageData> CurData;
	G42Ptr<G42LockPtr> CurLock;
	uint32 NumStrips;
	bool BlockDataSending;
	G42Vector<G42Ptr<G42ImageData> > DataStorage;
	
	void SendCurData(void);
	void SendDataStorage(void);
	void SendBegin(void);
	void SetupData(void);
	void GetData(uint32 strip);
	bool BeginSent;
	G42Ptr<G42Decompressor> Decompressor;
	// must be set before SendDescriptor is called
	void SetDecompressor(G42Decompressor * decompressor)
		{ Decompressor = decompressor; }
	enum G42ReadRowMode
		{ ReadStrip, ReadRow, UseTemporaryBuffer, AutoDetect };
	G42ReadRowMode ReadRowMode;
	virtual void ConvertRow(void); //allows conversions a row at a time
	void ProcessUncompressedData(); // processes uncompressed rows of pixels
	void ProcessCompressedData(); // processes uncompressed rows of pixels
	void ProcessImageData(); // calls one of the above depending on Decompressor
	void InitRow(void); // prepare row buffers
	virtual void OutputRow(void); // about to output row to parent
	virtual void EndPass(void); // finishes a one pass file
	void PutByte(byte value)
	{ // put a byte into the row buffer
		*RowPtr++ = value;
		if (!(--RowBytesLeft))
		{
			OutputRow();
		}
	}
	void PutByteRun(byte value, int number); // put a run of bytes
	void PutSequence(const byte * buffer, int buffer_size); // put a sequence of bytes
	void EatBuffer(); // eat the remaining bytes in the input buffer
	// sends any remaining data objects and an EndPath
	void SendEnd(void);
	void GetBuffer(byte * buf, uint32 buf_len); // fills a buffer
	// so you can call Fill Buffer and read straight in non-stream mode
	byte * GetRow(bool trans = false); // gets a pointer to the current row
	void NextRow(void); // goes to the next row
	void CleanupData(void); // cleans up and sends any unsent rows
	// simple class to do above in a more C++ish manner
#if 0 // that doesn't work for now
	class G42RowIterator
	{
	public:
		G42RowIterator(G42ImageReaderBase & base, const G42ImageDescriptor & desc,
				intf num_passes = 1) : Base(base)
			{ Base.SetupData(num_passes); }
		~G42RowIterator(void)
			{ Base.CleanupData(); }
		operator byte * (void)
			{ return Base.GetRow(); }
		void operator ++ (void) // don't let them do ++ and anything else
			{ Base.NextRow(); }
		void operator ++ (int) // support both pre and post increment
			{ Base.NextRow(); }
	private:
		G42ImageReaderBase & Base;
	};
#endif
	byte * GetDataPtr(void);
	byte * GetRowPtr(uint32 row);
	void SendData(void);
	void GetNewData(void);
	G42Ptr<G42ImageDescriptor> Descriptor;
	// various functions that are unique depending upon whether we are in a
	// DLL or not
public:
	#if defined(G42_IN_RW_DLL)
	#else // not in dll
	G42ImageData * GetImageData(G42ImageDescriptor::G42ImageType type,
		G42HeapManager & heap, uint32 strip_num,
		uint32 num_rows, uint32 row_bytes, uint32 strip_start_in_image,
		uint32 width, uint32 trans_row_bytes = 0);
	void ClearImageData(G42ImageData * data);
	G42WriteImageData * GetWriteImageLock(G42ImageData * data);
	void ClearWriteImageLock(G42WriteImageData * lock);
	byte * GetRowFromLock(G42WriteImageData * data, uint32 row,
		bool trans = false);
	#endif // in dll
private:
	G42ImageData * Data;
};
// use this class instead of others, in DLL, it gets lock through callback
class G42ImageDataDllWriteLock
{
public:
	G42ImageDataDllWriteLock(G42ImageData * data);
	~G42ImageDataDllWriteLock(void);
	operator G42WriteImageData * (void)
		{ return Data; }
	G42WriteImageData * operator -> (void)
		{ return Data; }
	G42WriteImageData & operator * (void)
		{ return *Data; }
private:
	G42WriteImageData * Data;
};
#if defined(G42_IN_RW_DLL)
#define G42_SETUP_DLL(Name, ...) \
... various data here ...
#else
#define G42_SETUP_DLL(Name)
#endif
#endif // sentry

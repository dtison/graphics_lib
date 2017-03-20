// GIDATA.H - G42ImageData, G42ReadImageData, G42WriteImageData
#if !defined(GIDATA_H)
#define GIDATA_H
/*-----------------------------------------------------------------------------
	file:			gidata.h
	author:		Guy Schalnat
	date:			11/06/96
	revision:	3.0
	This class holds image data.  It is a G42PathObject and a DataKey,
	and is expected to be copied and saved often.  This class only holds
	enough info to deal with itself internally, the user is expected
	to hold onto a G42ImageDescriptor to correctly understand the data.
	To access the data itself, the user needs to create one of the locked
	classes (Read or Write), and access the data through that
	More information is found in image.txt
-----------------------------------------------------------------------------*/
#include "gitype.h"
#include "gpobj.h"
#include "gdatakey.h"
//#pragma precompile_target "gidata"
//#define USE_ROWS
#define USE_OFFSETS
// the two lock classes predefined so they can be friends
class G42ReadImageData;
class G42WriteImageData;
/*-----------------------------------------------------------------------------
	class G42ImageData
-----------------------------------------------------------------------------*/
class G42ImageData : public G42PathObject, public G42DataKey
{
friend class G42ReadImageData;
friend class G42WriteImageData;
public:
	// constructors, destructor, operator =
	G42ImageData(G42HeapManager & heap, uint32 strip_num, uint32 num_rows,
		uint32 row_bytes, uint32 strip_start_in_image, uint32 width,
		uint32 trans_row_bytes = 0) throw (G42InvalidParameterException);
	virtual ~G42ImageData();
	// member functions
	uint32 GetNumRows() const
		{ return NumRows; }
	uint32 GetRowBytes() const
		{ return RowBytes; }
	uint32 GetStripNum() const
		{ return StripNum; }
	uint32 GetStripStartInImage() const
		{ return StripStartInImage; }
	uint32 GetTransRowBytes() const
		{ return TransRowBytes; }
	uint32 GetWidth() const
		{ return Width; }
	bool IsTransparent(void) const
		{ return (TransRowBytes > 0); }
	void Set(uint32 strip_num, uint32 num_rows, uint32 row_bytes,
		uint32 strip_start_in_image,  uint32 width, uint32 trans_row_bytes = 0)
			throw (G42InvalidParameterException);
	void SetInfo(uint32 strip_num, uint32 num_rows, uint32 row_bytes,
		uint32 strip_start_in_image, uint32 width, uint32 trans_row_bytes = 0)
			throw (G42InvalidParameterException);
	virtual G42ImageData * MakeCopy(void) = 0;
protected:
	void Copy(G42ImageData & ref);
	G42ImageData(G42ImageData & ref);
	G42ImageData & operator = (G42ImageData & ref);
private:
	uint32 NumRows; // number of rows in the strip
	uint32 RowBytes; // bytes per row, 4 byte aligned
#ifdef USE_ROWS
	byte ** Rows; // array of pointers to rows
#endif
#ifdef USE_OFFSETS
	uint32 * RowOffsets; // array of row offsets
#endif
	uint32 StripNum; // strip number in image
	uint32 StripStartInImage; // where in image first row starts
	uint32 TransRowBytes; // bytes per row of transparent mask, 4 byte aligned
	uint32 TransOffset; // offset of transparent data in data block
	uint32 Width; // number of pixels in a row
#ifdef USE_ROWS
	byte ** TransRows; // array of transparent rows
#endif
#ifdef USE_OFFSETS
	uint32 * TransRowOffsets; // array of transparent rows
	byte * Ptr; // base ptr for rows
#endif
	// member functions
	void BuildRows(byte * ptr); // sets the Rows and TransRows for the locks
#ifdef USE_OFFSETS
	void BuildRowOffsets(); // sets the row offsets
#endif
	void Clear(); // clear all data
	void ClearRows(void); // clears the Rows and TransRows
	void CopyData(G42ImageData & ref); // copies the data only
	void CopyInfo(G42ImageData & ref); // copies the info only
};
/*-----------------------------------------------------------------------------
	class G42ReadImageData
-----------------------------------------------------------------------------*/
class G42ReadImageData : public G42ReadLockedKey
{
public:
	// constructor and destructor
	G42ReadImageData(G42ImageData & key);
	virtual ~G42ReadImageData() {}
	// member functions
	const byte * GetPtr(bool trans_row) const;
	const byte * GetRow(uint32 index, bool trans_row = false) const;
		// throw (G42InvalidParameterException);
	G42ImageData & GetData(void) const
		{ return ImageKey; }
private:
	G42ImageData & ImageKey; // the data itself
};
/*-----------------------------------------------------------------------------
	class G42WriteImageData
-----------------------------------------------------------------------------*/
class G42WriteImageData : public G42WriteLockedKey
{
public:
	// constructor and destructor
	G42WriteImageData(G42ImageData & key);
	virtual ~G42WriteImageData() {}
	// member functions
	byte * GetPtr(bool trans_row) const;
	byte * GetRow(uint32 index, bool trans_row = false) const;
		// throw (G42InvalidParameterException);
	G42ImageData & GetData(void) const
		{ return ImageKey; }
private:
	G42ImageData & ImageKey; // the data itself
};
/******************************************/
/*-----------------------------------------------------------------------------
	Function - G42ReadImageData constructor
	Parameters:
		G42ImageData & key - the image data you want read locked
	Description:
		Locks a read data for reading and builds the row and trans
		row arrays.
-----------------------------------------------------------------------------*/
inline
G42ReadImageData::G42ReadImageData(G42ImageData & key)
:
	G42ReadLockedKey(key), ImageKey(key)
{
	ImageKey.BuildRows((byte *)(const byte *)(*this));
}
/*-----------------------------------------------------------------------------
	Function - GetPtr
	Parameters:
		bool trans_row - whether the image data or transparent mask is returned
	Description:
		returns a pointer to the requested data.  The caller is responsible
		for dealing with the inverted images of MSWIN
-----------------------------------------------------------------------------*/
inline const byte *
G42ReadImageData::GetPtr(bool trans_row) const
{
	const byte * ptr = (*this);
	if (trans_row)
	{
		ptr += ImageKey.TransOffset;
	}
	return ptr;
}
/*-----------------------------------------------------------------------------
	Function - GetRow
	Parameters:
		uint32 index - row number in strip to return
		bool trans_row - whether the image data or transparent mask is returned
	Exceptions:
		G42InvalidParameterException if row is outside of strip.
	Description:
		returns a pointer to the requested row or mask.
-----------------------------------------------------------------------------*/
inline const byte *
G42ReadImageData::GetRow(uint32 index, bool trans_row) const
	// throw (G42InvalidParameterException)
{
	if (index >= ImageKey.GetNumRows())
	{
		throw G42InvalidParameterException();
	}
#ifdef USE_ROWS
	return (trans_row ? ImageKey.TransRows[index] : ImageKey.Rows[index]);
#endif
#ifdef USE_OFFSETS
	return (trans_row ? ImageKey.Ptr + ImageKey.TransRowOffsets[index] :
		ImageKey.Ptr + ImageKey.RowOffsets[index]);
#endif
}
/*-----------------------------------------------------------------------------
	Function - G42WriteImageData constructor
	Parameters:
		G42ImageData & key - the image data you want write locked
	Description:
		Locks a read data for writing and builds the row and trans
		row arrays.
-----------------------------------------------------------------------------*/
inline
G42WriteImageData::G42WriteImageData(G42ImageData & key)
:
	G42WriteLockedKey(key), ImageKey(key)
{
	ImageKey.BuildRows(*this);
}
/*-----------------------------------------------------------------------------
	Function - GetPtr
	Parameters:
		bool trans_row - whether the image data or transparent mask is returned
	Description:
		returns a pointer to the requested data.  The caller is responsible
		for dealing with the inverted images of MSWIN
-----------------------------------------------------------------------------*/
inline byte *
G42WriteImageData::GetPtr(bool trans_row) const
{
	byte * ptr = (*this);
	if (trans_row)
	{
		ptr += ImageKey.TransOffset;
	}
	return ptr;
}
/*-----------------------------------------------------------------------------
	Function - GetRow
	Parameters:
		uint32 index - row number in strip to return
		bool trans_row - whether the image data or transparent mask is returned
	Exceptions:
		G42InvalidParameterException if row is outside of strip.
	Description:
		returns a pointer to the requested row or mask.
-----------------------------------------------------------------------------*/
inline byte *
G42WriteImageData::GetRow(uint32 index, bool trans_row) const
	// throw (G42InvalidParameterException)
{
	if (index >= ImageKey.GetNumRows())
	{
		throw G42InvalidParameterException();
	}
#ifdef USE_ROWS
	return (trans_row ? ImageKey.TransRows[index] : ImageKey.Rows[index]);
#endif
#ifdef USE_OFFSETS
	return (trans_row ? ImageKey.Ptr + ImageKey.TransRowOffsets[index] :
		ImageKey.Ptr + ImageKey.RowOffsets[index]);
#endif
}
/* Created by GES: these should be the only thing that
	needs changed between various platforms.  I put the
	SetFiller in an inline, because MSWIN doesn't need it.
*/
#ifdef MSWIN
const int G42TrueColorRedOffset = 2;
const int G42TrueColorGreenOffset = 1;
const int G42TrueColorBlueOffset = 0;
const int G42TrueColorFillerOffset = 3;
const int G42TrueColorSkipFillerOffset = 0;
const byte G42TrueColorFillerByte = 0;
#endif
#ifdef XWIN
const int G42TrueColorRedOffset = 0;
const int G42TrueColorGreenOffset = 1;
const int G42TrueColorBlueOffset = 2;
const int G42TrueColorFillerOffset = 3;
const int G42TrueColorSkipFillerOffset = 0;
const byte G42TrueColorFillerByte = 0xff;
#endif
#ifdef MACOS
const int G42TrueColorRedOffset = 0;
const int G42TrueColorGreenOffset = 1;
const int G42TrueColorBlueOffset = 2;
const int G42TrueColorFillerOffset = 0;
const int G42TrueColorSkipFillerOffset = 1;
const byte G42TrueColorFillerByte = 0;
#endif
inline void G42TrueColorSetFiller(byte * ptr)
	{*(ptr + G42TrueColorFillerOffset) = G42TrueColorFillerByte;}
class G42ConstColor24
{
public:
	G42ConstColor24(const byte * ptr) : Ptr(ptr) {}
	G42ConstColor24(const G42ConstColor24 & ref) : Ptr(ref.Ptr) {}
	byte GetRed(void) const
		{ return *(Ptr + G42TrueColorRedOffset); }
	byte GetGreen(void) const
		{ return *(Ptr + G42TrueColorGreenOffset); }
	byte GetBlue(void) const
		{ return *(Ptr + G42TrueColorBlueOffset); }
	const byte * GetPtr(void) const
		{ return Ptr; }
	G42Color GetColor(void) const
		{ return G42Color(GetRed(), GetGreen(), GetBlue()); }
	operator G42Color (void) const
		{ return GetColor(); }
private:
	const byte * Ptr;
};
class G42Color24
{
public:
	G42Color24(byte * ptr) : Ptr(ptr) {}
	G42Color24(const G42Color24 & ref) : Ptr(ref.Ptr) {}
	byte GetRed(void) const
		{ return *(Ptr + G42TrueColorRedOffset); }
	byte GetGreen(void) const
		{ return *(Ptr + G42TrueColorGreenOffset); }
	byte GetBlue(void) const
		{ return *(Ptr + G42TrueColorBlueOffset); }
	byte * GetPtr(void) const
		{ return Ptr; }
	G42Color GetColor(void) const
		{ return G42Color(GetRed(), GetGreen(), GetBlue()); }
	operator G42Color (void) const
		{ return GetColor(); }
	void SetRed(byte red)
		{ *(Ptr + G42TrueColorRedOffset) = red; }
	void SetGreen(byte green)
		{ *(Ptr + G42TrueColorGreenOffset) = green; }
	void SetBlue(byte blue)
		{ *(Ptr + G42TrueColorBlueOffset) = blue; }
	void Set(const G42Color & c)
		{ SetRed(c.Red); SetGreen(c.Green); SetBlue(c.Blue); }
	void Set(const G42Color24 & c)
		{ memcpy(Ptr, c.Ptr, 3); }
	void Set(const G42ConstColor24 & c)
		{ memcpy(Ptr, c.GetPtr(), 3); }
private:
	byte * Ptr;
};
class G42ConstColor32
{
public:
	G42ConstColor32(const byte * ptr) : Ptr(ptr) {}
	G42ConstColor32(const G42ConstColor32 & ref) : Ptr(ref.Ptr) {}
	byte GetRed(void) const
		{ return *(Ptr + G42TrueColorRedOffset + G42TrueColorSkipFillerOffset); }
	byte GetGreen(void) const
		{ return *(Ptr + G42TrueColorGreenOffset + G42TrueColorSkipFillerOffset); }
	byte GetBlue(void) const
		{ return *(Ptr + G42TrueColorBlueOffset + G42TrueColorSkipFillerOffset); }
	const byte * GetPtr(void) const
		{ return Ptr; }
	G42Color GetColor(void) const
		{ return G42Color(GetRed(), GetGreen(), GetBlue()); }
	operator G42Color (void) const
		{ return GetColor(); }
private:
	const byte * Ptr;
};
class G42Color32
{
public:
	G42Color32(byte * ptr) : Ptr(ptr) {}
	G42Color32(const G42Color32 & ref) : Ptr(ref.Ptr) {}
	byte GetRed(void) const
		{ return *(Ptr + G42TrueColorRedOffset + G42TrueColorSkipFillerOffset); }
	byte GetGreen(void) const
		{ return *(Ptr + G42TrueColorGreenOffset + G42TrueColorSkipFillerOffset); }
	byte GetBlue(void) const
		{ return *(Ptr + G42TrueColorBlueOffset + G42TrueColorSkipFillerOffset); }
	byte * GetPtr(void) const
		{ return Ptr; }
	G42Color GetColor(void) const
		{ return G42Color(GetRed(), GetGreen(), GetBlue()); }
	operator G42Color (void) const
		{ return GetColor(); }
	void SetFiller(void)
		{ G42TrueColorSetFiller(Ptr); }
	void SetRed(byte red)
		{ *(Ptr + G42TrueColorRedOffset + G42TrueColorSkipFillerOffset) = red;
			SetFiller(); }
	void SetGreen(byte green)
		{ *(Ptr + G42TrueColorGreenOffset + G42TrueColorSkipFillerOffset) = green; }
	void SetBlue(byte blue)
		{ *(Ptr + G42TrueColorBlueOffset + G42TrueColorSkipFillerOffset) = blue; }
	void Set(const G42Color & c)
		{ SetRed(c.Red); SetGreen(c.Green); SetBlue(c.Blue); }
	void Set(const G42Color32 & c)
		{ memcpy(Ptr, c.Ptr, 4); }
	void Set(const G42ConstColor32 & c)
		{ memcpy(Ptr, c.GetPtr(), 4); }
	void Set(const G42Color24 & c)
		{ memcpy(Ptr + G42TrueColorSkipFillerOffset, c.GetPtr(), 3); SetFiller(); }
	void Set(const G42ConstColor24 & c)
		{ memcpy(Ptr + G42TrueColorSkipFillerOffset, c.GetPtr(), 3); SetFiller(); }
private:
	byte * Ptr;
};
// 1 bit byte iterators
template <class T>
class G42BaseBitIterator
{
public:
	G42BaseBitIterator(void) : Ptr(0), StartPtr(0), Mask(0x80), StartMask(0x80) {}
	G42BaseBitIterator(T ptr, int offset = 0) :
		Ptr(ptr + (offset >> 3)), StartPtr(ptr), StartMask(0x80),
		Mask((byte)(1 << (7 - (offset & 7)))) {}
	G42BaseBitIterator(T ptr, T start_ptr, byte mask, byte start_mask) :
		Ptr(ptr), StartPtr(start_ptr), StartMask(start_mask),
		Mask(mask) {}
	G42BaseBitIterator(const G42BaseBitIterator<T> & ptr) :
		Ptr(ptr.Ptr), StartPtr(ptr.StartPtr), Mask(ptr.Mask),
		StartMask(ptr.StartMask) {}
	G42BaseBitIterator<T> & operator = (const G42BaseBitIterator<T> & ptr)
		{ Ptr = ptr.Ptr; StartPtr = ptr.StartPtr; Mask = ptr.Mask;
			StartMask = ptr.StartMask; return *this; }
	bool operator == (const G42BaseBitIterator<T> & ptr)
		{ return Ptr == ptr.Ptr && Mask == ptr.Mask; }
	bool operator != (const G42BaseBitIterator<T> & ptr)
		{ return Ptr != ptr.Ptr || Mask != ptr.Mask; }
	G42BaseBitIterator<T> & operator * (void)
		{ return *this; }
	operator bool (void)
		{ return (bool)(*Ptr & Mask); }
	void Inc(void)
		{ if (Mask == 1) { Mask = 0x80; Ptr++; } else Mask >>= 1; }
	void Dec(void)
		{ if (Mask == 0x80) { Mask = 1; Ptr--; } else Mask <<= 1; }
	G42BaseBitIterator<T> operator ++ (int)
		{ G42BaseBitIterator<T> i(*this); Inc(); return i; }
	G42BaseBitIterator<T> & operator ++ (void)
		{ Inc(); return *this; }
	G42BaseBitIterator<T> operator -- (int)
		{ G42BaseBitIterator<T> i(*this); Dec(); return i; }
	G42BaseBitIterator<T> & operator -- (void)
		{ Dec(); return *this; }
	G42BaseBitIterator<T> & operator += (int n)
		{ Ptr += (n >> 3); if ((Mask >> (n & 7)) == 0)
			{Ptr++; Mask <<= (7 - (n & 7)); } else Mask >>= (n & 7);
			return *this; }
	G42BaseBitIterator<T> & operator -= (int n)
		{ Ptr -= (n >> 3); if ((Mask << (n & 7)) > 0x80)
			{Ptr--; Mask >>= (7 - (n & 7)); } else Mask <<= (n & 7);
			return *this; }
	G42BaseBitIterator<T> operator + (int n)
		{ T ptr = Ptr + (n >> 3); byte mask = Mask;
			if ((mask >> (n & 7)) == 0)
			{ptr++; mask <<= (7 - (n & 7)); } else mask >>= (n & 7);
			return G42BaseBitIterator<T>(ptr, StartPtr, mask, StartMask); }
	G42BaseBitIterator<T> operator - (int n)
		{ T ptr = Ptr - (n >> 3); byte mask = Mask;
			if ((mask << (n & 7)) > 0x80)
			{ptr--; mask >>= (7 - (n & 7)); } else mask <<= (n & 7);
			return G42BaseBitIterator<T>(ptr, StartPtr, mask, StartMask); }
	int operator - (const G42BaseBitIterator<T> & ref); // function call
	G42BaseBitIterator<T> operator [] (int index)
		{ T ptr = StartPtr + (index >> 3);
			byte mask = (byte)(1 << (7 - (index & 7)));
			return G42BaseBitIterator<T>(ptr, StartPtr, mask, StartMask); }
	bool operator > (const G42BaseBitIterator<T> & ref)
		{ return Ptr > ref.Ptr && Mask > ref.Mask; }
	bool operator < (const G42BaseBitIterator<T> & ref)
		{ return Ptr < ref.Ptr && Mask < ref.Mask; }
	bool operator >= (const G42BaseBitIterator<T> & ref)
		{ return Ptr >= ref.Ptr && Mask >= ref.Mask; }
	bool operator <= (const G42BaseBitIterator<T> & ref)
		{ return Ptr <= ref.Ptr && Mask <= ref.Mask; }
protected:
	T Ptr;
	T StartPtr;
	byte Mask;
	byte StartMask;
};
template <class T>
int
G42BaseBitIterator<T>::operator - (const G42BaseBitIterator<T> & ref)
{
	int dist = 0;
	if (Ptr > ref.Ptr)
		dist += ((Ptr - ref.Ptr) << 3);
	else if (Ptr < ref.Ptr)
		dist -= ((ref.Ptr - Ptr) << 3);
	if (Mask > ref.Mask)
	{
		for (byte m = Mask; m > ref.Mask; m >>= 1)
			dist--;
	}
	else if (Mask < ref.Mask)
	{
		for (byte m = ref.Mask; m > Mask; m >>= 1)
			dist++;
	}
	return dist;
}
// the only difference between this and above is op = (byte), but
// I can't get this class to derive from the above class.  Ug.
template <class T>
class G42BaseWriteBitIterator
{
public:
	G42BaseWriteBitIterator(void) : Ptr(0), StartPtr(0), Mask(0x80), StartMask(0x80) {}
	G42BaseWriteBitIterator(T ptr, int offset = 0) :
		Ptr(ptr + (offset >> 3)), StartPtr(ptr), StartMask(0x80),
		Mask((byte)(1 << (7 - (offset & 7)))) {}
	G42BaseWriteBitIterator(T ptr, T start_ptr, byte mask, byte start_mask) :
		Ptr(ptr), StartPtr(start_ptr), StartMask(start_mask),
		Mask(mask) {}
	G42BaseWriteBitIterator(const G42BaseWriteBitIterator<T> & ptr) :
		Ptr(ptr.Ptr), StartPtr(ptr.StartPtr), Mask(ptr.Mask),
		StartMask(ptr.StartMask) {}
	G42BaseWriteBitIterator<T> & operator = (const G42BaseWriteBitIterator<T> & ptr)
		{ Ptr = ptr.Ptr; StartPtr = ptr.StartPtr; Mask = ptr.Mask;
			StartMask = ptr.StartMask; return *this; }
	bool operator == (const G42BaseWriteBitIterator<T> & ptr)
		{ return Ptr == ptr.Ptr && Mask == ptr.Mask; }
	bool operator != (const G42BaseWriteBitIterator<T> & ptr)
		{ return Ptr != ptr.Ptr || Mask != ptr.Mask; }
	G42BaseWriteBitIterator<T> & operator * (void)
		{ return *this; }
	operator bool (void)
		{ return (bool)(*Ptr & Mask); }
	void Inc(void)
		{ if (Mask == 1) { Mask = 0x80; Ptr++; } else Mask >>= 1; }
	void Dec(void)
		{ if (Mask == 0x80) { Mask = 1; Ptr--; } else Mask <<= 1; }
	G42BaseWriteBitIterator<T> operator ++ (int)
		{ G42BaseWriteBitIterator<T> i(*this); Inc(); return i; }
	G42BaseWriteBitIterator<T> & operator ++ (void)
		{ Inc(); return *this; }
	G42BaseWriteBitIterator<T> operator -- (int)
		{ G42BaseWriteBitIterator<T> i(*this); Dec(); return i; }
	G42BaseWriteBitIterator<T> & operator -- (void)
		{ Dec(); return *this; }
	G42BaseWriteBitIterator<T> & operator += (int n)
		{ Ptr += (n >> 3); if ((Mask >> (n & 7)) == 0)
			{Ptr++; Mask <<= (7 - (n & 7)); } else Mask >>= (n & 7);
			return *this; }
	G42BaseWriteBitIterator<T> & operator -= (int n)
		{ Ptr -= (n >> 3); if ((Mask << (n & 7)) > 0x80)
			{Ptr--; Mask >>= (7 - (n & 7)); } else Mask <<= (n & 7);
			return *this; }
	G42BaseWriteBitIterator<T> operator + (int n)
		{ T ptr = Ptr + (n >> 3); byte mask = Mask;
			if ((mask >> (n & 7)) == 0)
			{ptr++; mask <<= (7 - (n & 7)); } else mask >>= (n & 7);
			return G42BaseWriteBitIterator<T>(ptr, StartPtr, mask, StartMask); }
	G42BaseWriteBitIterator<T> operator - (int n)
		{ T ptr = Ptr - (n >> 3); byte mask = Mask;
			if ((mask << (n & 7)) > 0x80)
			{ptr--; mask >>= (7 - (n & 7)); } else mask <<= (n & 7);
			return G42BaseWriteBitIterator<T>(ptr, StartPtr, mask, StartMask); }
	int operator - (const G42BaseWriteBitIterator<T> & ref); // function call
	G42BaseWriteBitIterator<T> operator [] (int index)
		{ T ptr = StartPtr + (index >> 3);
			byte mask = (byte)(1 << (7 - (index & 7)));
			return G42BaseWriteBitIterator<T>(ptr, StartPtr, mask, StartMask); }
	bool operator > (const G42BaseWriteBitIterator<T> & ref)
		{ return Ptr > ref.Ptr && Mask > ref.Mask; }
	bool operator < (const G42BaseWriteBitIterator<T> & ref)
		{ return Ptr < ref.Ptr && Mask < ref.Mask; }
	bool operator >= (const G42BaseWriteBitIterator<T> & ref)
		{ return Ptr >= ref.Ptr && Mask >= ref.Mask; }
	bool operator <= (const G42BaseWriteBitIterator<T> & ref)
		{ return Ptr <= ref.Ptr && Mask <= ref.Mask; }
	bool operator = (bool value)
		{ if (value) *Ptr |= Mask; else *Ptr &= (byte)~Mask; return value; }
protected:
	T Ptr;
	T StartPtr;
	byte Mask;
	byte StartMask;
};
template <class T>
int
G42BaseWriteBitIterator<T>::operator - (const G42BaseWriteBitIterator<T> & ref)
{
	int dist = 0;
	if (Ptr > ref.Ptr)
		dist += ((Ptr - ref.Ptr) << 3);
	else if (Ptr < ref.Ptr)
		dist -= ((ref.Ptr - Ptr) << 3);
	if (Mask > ref.Mask)
	{
		for (byte m = Mask; m > ref.Mask; m >>= 1)
			dist--;
	}
	else if (Mask < ref.Mask)
	{
		for (byte m = ref.Mask; m > Mask; m >>= 1)
			dist++;
	}
	return dist;
}
typedef G42BaseWriteBitIterator<byte *> G42BitIterator;
typedef G42BaseBitIterator<const byte *> G42ConstBitIterator;
// these safe iterators return true if there is no pointer
template <class T>
class G42SafeBaseBitIterator
{
public:
	G42SafeBaseBitIterator(void) : Ptr(0), StartPtr(0), Mask(0x80), StartMask(0x80) {}
	G42SafeBaseBitIterator(T ptr, int offset = 0) :
		Ptr(ptr), StartPtr(ptr), StartMask(0x80),
		Mask((byte)(1 << (7 - (offset & 7)))) {if (Ptr) Ptr +=  (offset >> 3); }
	G42SafeBaseBitIterator(T ptr, T start_ptr, byte mask, byte start_mask) :
		Ptr(ptr), StartPtr(start_ptr), StartMask(start_mask),
		Mask(mask) {}
	G42SafeBaseBitIterator(const G42SafeBaseBitIterator<T> & ptr) :
		Ptr(ptr.Ptr), StartPtr(ptr.StartPtr), Mask(ptr.Mask),
		StartMask(ptr.StartMask) {}
	G42SafeBaseBitIterator<T> & operator = (const G42SafeBaseBitIterator<T> & ptr)
		{ Ptr = ptr.Ptr; StartPtr = ptr.StartPtr; Mask = ptr.Mask;
			StartMask = ptr.StartMask; return *this; }
	bool operator == (const G42SafeBaseBitIterator<T> & ptr)
		{ return Ptr == ptr.Ptr && Mask == ptr.Mask; }
	bool operator != (const G42SafeBaseBitIterator<T> & ptr)
		{ return Ptr != ptr.Ptr || Mask != ptr.Mask; }
	G42SafeBaseBitIterator<T> & operator * (void)
		{ return *this; }
	operator bool (void)
		{ return (Ptr ? (bool)(*Ptr & Mask) : true); }
	void Inc(void)
		{ if (Ptr) { if (Mask == 1) { Mask = 0x80; Ptr++; } else Mask >>= 1; } }
	void Dec(void)
		{ if (Ptr) { if (Mask == 0x80) { Mask = 1; Ptr--; } else Mask <<= 1; } }
	G42SafeBaseBitIterator<T> operator ++ (int)
		{ G42SafeBaseBitIterator<T> i(*this); Inc(); return i; }
	G42SafeBaseBitIterator<T> & operator ++ (void)
		{ Inc(); return *this; }
	G42SafeBaseBitIterator<T> operator -- (int)
		{ G42SafeBaseBitIterator<T> i(*this); Dec(); return i; }
	G42SafeBaseBitIterator<T> & operator -- (void)
		{ Dec(); return *this; }
	G42SafeBaseBitIterator<T> & operator += (int n)
		{ if (Ptr) { Ptr += (n >> 3); if ((Mask >> (n & 7)) == 0)
			{Ptr++; Mask <<= (7 - (n & 7)); } else Mask >>= (n & 7); }
			return *this; }
	G42SafeBaseBitIterator<T> & operator -= (int n)
		{ if (Ptr) { Ptr -= (n >> 3); if ((Mask << (n & 7)) > 0x80)
			{Ptr--; Mask >>= (7 - (n & 7)); } else Mask <<= (n & 7); }
			return *this; }
	G42SafeBaseBitIterator<T> operator + (int n)
		{ if (Ptr) { T ptr = Ptr + (n >> 3); byte mask = Mask;
			if ((mask >> (n & 7)) == 0)
			{ptr++; mask <<= (7 - (n & 7)); } else mask >>= (n & 7);
			return G42SafeBaseBitIterator<T>(ptr, StartPtr, mask, StartMask); }
			return *this; }
	G42SafeBaseBitIterator<T> operator - (int n)
		{ if (Ptr) { T ptr = Ptr - (n >> 3); byte mask = Mask;
			if ((mask << (n & 7)) > 0x80)
			{ptr--; mask >>= (7 - (n & 7)); } else mask <<= (n & 7);
			return G42SafeBaseBitIterator<T>(ptr, StartPtr, mask, StartMask); }
			return *this; }
	int operator - (const G42SafeBaseBitIterator<T> & ref); // function call
	G42SafeBaseBitIterator<T> operator [] (int index)
		{ if (Ptr) { T ptr = StartPtr + (index >> 3);
			byte mask = (byte)(1 << (7 - (index & 7)));
			return G42SafeBaseBitIterator<T>(ptr, StartPtr, mask, StartMask); }
			return *this; }
	bool operator > (const G42SafeBaseBitIterator<T> & ref)
		{ return Ptr > ref.Ptr && Mask > ref.Mask; }
	bool operator < (const G42SafeBaseBitIterator<T> & ref)
		{ return Ptr < ref.Ptr && Mask < ref.Mask; }
	bool operator >= (const G42SafeBaseBitIterator<T> & ref)
		{ return Ptr >= ref.Ptr && Mask >= ref.Mask; }
	bool operator <= (const G42SafeBaseBitIterator<T> & ref)
		{ return Ptr <= ref.Ptr && Mask <= ref.Mask; }
protected:
	T Ptr;
	T StartPtr;
	byte Mask;
	byte StartMask;
};
template <class T>
int
G42SafeBaseBitIterator<T>::operator - (const G42SafeBaseBitIterator<T> & ref)
{
	if (!Ptr || !ref.Ptr)
	{
		return 0;
	}
	int dist = 0;
	if (Ptr > ref.Ptr)
		dist += ((Ptr - ref.Ptr) << 3);
	else if (Ptr < ref.Ptr)
		dist -= ((ref.Ptr - Ptr) << 3);
	if (Mask > ref.Mask)
	{
		for (byte m = Mask; m > ref.Mask; m >>= 1)
			dist--;
	}
	else if (Mask < ref.Mask)
	{
		for (byte m = ref.Mask; m > Mask; m >>= 1)
			dist++;
	}
	return dist;
}
// the only difference between this and above is op = (byte), but
// I can't get this class to derive from the above class.  Ug.
template <class T>
class G42SafeBaseWriteBitIterator
{
public:
	G42SafeBaseWriteBitIterator(void) : Ptr(0), StartPtr(0), Mask(0x80), StartMask(0x80) {}
	G42SafeBaseWriteBitIterator(T ptr, int offset = 0) :
		Ptr(ptr + (offset >> 3)), StartPtr(ptr), StartMask(0x80),
		Mask((byte)(1 << (7 - (offset & 7)))) {}
	G42SafeBaseWriteBitIterator(T ptr, T start_ptr, byte mask, byte start_mask) :
		Ptr(ptr), StartPtr(start_ptr), StartMask(start_mask),
		Mask(mask) {}
	G42SafeBaseWriteBitIterator(const G42SafeBaseWriteBitIterator<T> & ptr) :
		Ptr(ptr.Ptr), StartPtr(ptr.StartPtr), Mask(ptr.Mask),
		StartMask(ptr.StartMask) {}
	G42SafeBaseWriteBitIterator<T> & operator = (const G42SafeBaseWriteBitIterator<T> & ptr)
		{ Ptr = ptr.Ptr; StartPtr = ptr.StartPtr; Mask = ptr.Mask;
			StartMask = ptr.StartMask; return *this; }
	bool operator == (const G42SafeBaseWriteBitIterator<T> & ptr)
		{ return Ptr == ptr.Ptr && Mask == ptr.Mask; }
	bool operator != (const G42SafeBaseWriteBitIterator<T> & ptr)
		{ return Ptr != ptr.Ptr || Mask != ptr.Mask; }
	G42SafeBaseWriteBitIterator<T> & operator * (void)
		{ return *this; }
	operator bool (void)
		{ return (Ptr ? (bool)(*Ptr & Mask) : true); }
	void Inc(void)
		{ if (Ptr) { if (Mask == 1) { Mask = 0x80; Ptr++; } else Mask >>= 1; } }
	void Dec(void)
		{ if (Ptr) { if (Mask == 0x80) { Mask = 1; Ptr--; } else Mask <<= 1; } }
	G42SafeBaseWriteBitIterator<T> operator ++ (int)
		{ G42SafeBaseWriteBitIterator<T> i(*this); Inc(); return i; }
	G42SafeBaseWriteBitIterator<T> & operator ++ (void)
		{ Inc(); return *this; }
	G42SafeBaseWriteBitIterator<T> operator -- (int)
		{ G42SafeBaseWriteBitIterator<T> i(*this); Dec(); return i; }
	G42SafeBaseWriteBitIterator<T> & operator -- (void)
		{ Dec(); return *this; }
	G42SafeBaseWriteBitIterator<T> & operator += (int n)
		{ if (Ptr) { Ptr += (n >> 3); if ((Mask >> (n & 7)) == 0)
			{Ptr++; Mask <<= (7 - (n & 7)); } else Mask >>= (n & 7); }
			return *this; }
	G42SafeBaseWriteBitIterator<T> & operator -= (int n)
		{ if (Ptr) { Ptr -= (n >> 3); if ((Mask << (n & 7)) > 0x80)
			{Ptr--; Mask >>= (7 - (n & 7)); } else Mask <<= (n & 7); }
			return *this; }
	G42SafeBaseWriteBitIterator<T> operator + (int n)
		{ if (Ptr) { T ptr = Ptr + (n >> 3); byte mask = Mask;
			if ((mask >> (n & 7)) == 0)
			{ptr++; mask <<= (7 - (n & 7)); } else mask >>= (n & 7);
			return G42SafeBaseWriteBitIterator<T>(ptr, StartPtr, mask, StartMask); }
			return *this; }
	G42SafeBaseWriteBitIterator<T> operator - (int n)
		{ if (Ptr) { T ptr = Ptr - (n >> 3); byte mask = Mask;
			if ((mask << (n & 7)) > 0x80)
			{ptr--; mask >>= (7 - (n & 7)); } else mask <<= (n & 7);
			return G42SafeBaseWriteBitIterator<T>(ptr, StartPtr, mask, StartMask); }
			return *this; }
	int operator - (const G42SafeBaseWriteBitIterator<T> & ref); // function call
	G42SafeBaseWriteBitIterator<T> operator [] (int index)
		{ if (Ptr) { T ptr = StartPtr + (index >> 3);
			byte mask = (byte)(1 << (7 - (index & 7)));
			return G42SafeBaseWriteBitIterator<T>(ptr, StartPtr, mask, StartMask); }
			return *this; }
	bool operator > (const G42SafeBaseWriteBitIterator<T> & ref)
		{ return Ptr > ref.Ptr && Mask > ref.Mask; }
	bool operator < (const G42SafeBaseWriteBitIterator<T> & ref)
		{ return Ptr < ref.Ptr && Mask < ref.Mask; }
	bool operator >= (const G42SafeBaseWriteBitIterator<T> & ref)
		{ return Ptr >= ref.Ptr && Mask >= ref.Mask; }
	bool operator <= (const G42SafeBaseWriteBitIterator<T> & ref)
		{ return Ptr <= ref.Ptr && Mask <= ref.Mask; }
	bool operator = (bool value)
		{ if (Ptr) { if (value) *Ptr |= Mask; else *Ptr &= (byte)~Mask; }
			return value; }
protected:
	T Ptr;
	T StartPtr;
	byte Mask;
	byte StartMask;
};
template <class T>
int
G42SafeBaseWriteBitIterator<T>::operator - (const G42SafeBaseWriteBitIterator<T> & ref)
{
	if (!Ptr || !ref.Ptr)
	{
		return 0;
	}
	int dist = 0;
	if (Ptr > ref.Ptr)
		dist += ((Ptr - ref.Ptr) << 3);
	else if (Ptr < ref.Ptr)
		dist -= ((ref.Ptr - Ptr) << 3);
	if (Mask > ref.Mask)
	{
		for (byte m = Mask; m > ref.Mask; m >>= 1)
			dist--;
	}
	else if (Mask < ref.Mask)
	{
		for (byte m = ref.Mask; m > Mask; m >>= 1)
			dist++;
	}
	return dist;
}
typedef G42SafeBaseWriteBitIterator<byte *> G42SafeBitIterator;
typedef G42SafeBaseBitIterator<const byte *> G42ConstSafeBitIterator;
// not supported in library, used mainly in readers and writers
class G42Bit2Iterator
{
public:
	G42Bit2Iterator(void) : Ptr(0), Shift(6) {}
	G42Bit2Iterator(byte * ptr, int offset = 0) :
		Ptr(ptr + (offset >> 2)), Shift((int8)(6 - ((offset & 3) << 1))) {}
	G42Bit2Iterator(const G42Bit2Iterator & ptr) :
		Ptr(ptr.Ptr), Shift(ptr.Shift) {}
	G42Bit2Iterator & operator = (const G42Bit2Iterator & ptr)
		{ Ptr = ptr.Ptr; Shift = ptr.Shift; return *this; }
	bool operator == (const G42Bit2Iterator & ptr) const
		{ return Ptr == ptr.Ptr && Shift == ptr.Shift; }
	bool operator != (const G42Bit2Iterator & ptr) const
		{ return Ptr != ptr.Ptr || Shift != ptr.Shift; }
	G42Bit2Iterator & operator * (void)
		{ return *this; }
	operator byte (void)
		{ return (byte)(((*Ptr) >> Shift) & 0x3); }
	void Inc(void)
		{ if (!Shift) { Shift = 6; Ptr++; } else {Shift -= (int8)2; } }
	void Dec(void)
		{ if (Shift == 6) { Shift = 0; Ptr--; } else { Shift += (int8)2; } }
	G42Bit2Iterator operator ++ (int)
		{ G42Bit2Iterator i(*this); Inc(); return i; }
	G42Bit2Iterator & operator ++ (void)
		{ Inc(); return *this; }
	G42Bit2Iterator operator -- (int)
		{ G42Bit2Iterator i(*this); Dec(); return i; }
	G42Bit2Iterator & operator -- (void)
		{ Dec(); return *this; }
	G42Bit2Iterator & operator += (int n)
		{ Ptr += (n >> 2); Shift -= (int8)((n & 3) << 1);
			if (Shift < 0) { Shift += (int8)8; Ptr++; }
			return *this; }
	G42Bit2Iterator & operator -= (int n)
		{ Ptr -= (n >> 2); Shift += (int8)((n & 3) << 1);
			if (Shift > 6) { Shift -= (int8)8; Ptr--; }
			return *this; }
	G42Bit2Iterator operator + (int n)
		{ G42Bit2Iterator i(*this); i += n; return i; }
	G42Bit2Iterator operator - (int n)
		{ G42Bit2Iterator i(*this); i -= n; return i; }
	int operator - (const G42Bit2Iterator & ref)
		{ return (((Ptr - ref.Ptr) << 2) + ((ref.Shift - Shift) >> 1)); }
	G42Bit2Iterator operator [] (int index)
		{ G42Bit2Iterator i(*this); i += index; return *i; }
	bool operator > (const G42Bit2Iterator & ref)
		{ return Ptr > ref.Ptr && Shift > ref.Shift; }
	bool operator < (const G42Bit2Iterator & ref)
		{ return Ptr < ref.Ptr && Shift < ref.Shift; }
	bool operator >= (const G42Bit2Iterator & ref)
		{ return Ptr >= ref.Ptr && Shift >= ref.Shift; }
	bool operator <= (const G42Bit2Iterator & ref)
		{ return Ptr <= ref.Ptr && Shift <= ref.Shift; }
	byte operator = (byte value)
		{ *Ptr &= (byte)((0xff3f >> (6 - Shift)) & 0xff);
			*Ptr |= (byte)(value << Shift); return value; }
protected:
	byte * Ptr;
	int8 Shift;
};
class G42Bit4Iterator
{
public:
	G42Bit4Iterator(void) : Ptr(0), Shift(4) {}
	G42Bit4Iterator(byte * ptr, int offset = 0) :
		Ptr(ptr + (offset >> 1)), Shift((int8)(4 - ((offset & 1) << 2))) {}
	G42Bit4Iterator(const G42Bit4Iterator & ptr) :
		Ptr(ptr.Ptr), Shift(ptr.Shift) {}
	G42Bit4Iterator & operator = (const G42Bit4Iterator & ptr)
		{ Ptr = ptr.Ptr; Shift = ptr.Shift; return *this; }
	bool operator == (const G42Bit4Iterator & ptr) const
		{ return Ptr == ptr.Ptr && Shift == ptr.Shift; }
	bool operator != (const G42Bit4Iterator & ptr) const
		{ return Ptr != ptr.Ptr || Shift != ptr.Shift; }
	G42Bit4Iterator & operator * (void)
		{ return *this; }
	operator byte (void)
		{ return (byte)((*Ptr >> Shift) & 0xf); }
	void Inc(void)
		{ if (!Shift) { Shift = 4; Ptr++; } else {Shift = 0; } }
	void Dec(void)
		{ if (Shift) { Shift = 0; Ptr--; } else { Shift = 4; } }
	G42Bit4Iterator operator ++ (int)
		{ G42Bit4Iterator i(*this); Inc(); return i; }
	G42Bit4Iterator & operator ++ (void)
		{ Inc(); return *this; }
	G42Bit4Iterator operator -- (int)
		{ G42Bit4Iterator i(*this); Dec(); return i; }
	G42Bit4Iterator & operator -- (void)
		{ Dec(); return *this; }
	G42Bit4Iterator & operator += (int n)
		{ Ptr += (n >> 1); Shift -= (int8)((n & 1) << 2);
			if (Shift < 0) { Shift = 4; Ptr++; }
			return *this; }
	G42Bit4Iterator & operator -= (int n)
		{ Ptr -= (n >> 1); Shift += (int8)((n & 1) << 2);
			if (Shift > 4) { Shift = 0; Ptr--; }
			return *this; }
	G42Bit4Iterator operator + (int n)
		{ G42Bit4Iterator i(*this); i += n; return i; }
	G42Bit4Iterator operator - (int n)
		{ G42Bit4Iterator i(*this); i -= n; return i; }
	int operator - (const G42Bit4Iterator & ref)
		{ return (((Ptr - ref.Ptr) << 1) + ((ref.Shift - Shift) >> 2)); }
	G42Bit4Iterator operator [] (int index)
		{ G42Bit4Iterator i(*this); i += index; return *i; }
	bool operator > (const G42Bit4Iterator & ref)
		{ return Ptr > ref.Ptr && Shift > ref.Shift; }
	bool operator < (const G42Bit4Iterator & ref)
		{ return Ptr < ref.Ptr && Shift < ref.Shift; }
	bool operator >= (const G42Bit4Iterator & ref)
		{ return Ptr >= ref.Ptr && Shift >= ref.Shift; }
	bool operator <= (const G42Bit4Iterator & ref)
		{ return Ptr <= ref.Ptr && Shift <= ref.Shift; }
	byte operator = (byte value)
		{ *Ptr &= (byte)(0xf << (4 - Shift));
			*Ptr |= (byte)(value << Shift); return value; }
protected:
	byte * Ptr;
	int8 Shift;
};
template <class T>
class G42BaseByteIterator
{
public:
	G42BaseByteIterator(void) : Ptr(0), StartPtr(0) {}
	G42BaseByteIterator(T ptr, int offset = 0) :
		Ptr(ptr + offset), StartPtr(ptr) {}
	G42BaseByteIterator(const G42BaseByteIterator & ptr) :
		Ptr(ptr.Ptr), StartPtr(ptr.StartPtr) {}
	G42BaseByteIterator & operator = (const G42BaseByteIterator & ptr)
		{ Ptr = ptr.Ptr; StartPtr = ptr.StartPtr; return *this; }
	bool operator == (const G42BaseByteIterator & ptr)
		{ return Ptr == ptr.Ptr; }
	bool operator != (const G42BaseByteIterator & ptr)
		{ return Ptr != ptr.Ptr; }
	operator byte (void)
		{ return *Ptr; }
	G42BaseByteIterator & operator * (void)
		{ return *this; }
	G42BaseByteIterator operator ++ (int)
		{ Ptr++; return G42BaseByteIterator<T>(StartPtr, Ptr - StartPtr - 1); }
	G42BaseByteIterator & operator ++ (void)
		{ ++Ptr; return *this; }
	G42BaseByteIterator operator -- (int)
		{ Ptr--; return G42BaseByteIterator<T>(StartPtr, Ptr - StartPtr + 1); }
	G42BaseByteIterator & operator -- (void)
		{ --Ptr; return *this; }
	G42BaseByteIterator & operator += (int n)
		{ Ptr += n; return *this; }
	G42BaseByteIterator & operator -= (int n)
		{ Ptr -= n; return *this; }
	G42BaseByteIterator operator + (int n)
		{ return G42BaseByteIterator<T>(Ptr + n); }
	G42BaseByteIterator operator - (int n)
		{ return G42BaseByteIterator<T>(Ptr - n); }
	int operator - (const G42BaseByteIterator & ref)
		{ return Ptr - ref.Ptr; }
	G42BaseByteIterator operator [] (int index)
		{ return G42BaseByteIterator<T>(StartPtr, index); }
	bool operator > (const G42BaseByteIterator & ref)
		{ return Ptr > ref.Ptr; }
	bool operator < (const G42BaseByteIterator & ref)
		{ return Ptr < ref.Ptr; }
	bool operator >= (const G42BaseByteIterator & ref)
		{ return Ptr >= ref.Ptr; }
	bool operator <= (const G42BaseByteIterator & ref)
		{ return Ptr <= ref.Ptr; }
protected:
	T Ptr;
	T StartPtr;
};
// the only difference between this and above is op = (byte), but
// I can't get this class to derive from the above class.  Ug.
template <class T>
class G42BaseWriteByteIterator
{
public:
	G42BaseWriteByteIterator(void) : Ptr(0), StartPtr(0) {}
	G42BaseWriteByteIterator(T ptr, int offset = 0) :
		Ptr(ptr + offset), StartPtr(ptr) {}
	G42BaseWriteByteIterator(const G42BaseWriteByteIterator & ptr) :
		Ptr(ptr.Ptr), StartPtr(ptr.StartPtr) {}
	G42BaseWriteByteIterator & operator = (const G42BaseWriteByteIterator & ptr)
		{ Ptr = ptr.Ptr; StartPtr = ptr.StartPtr; return *this; }
	bool operator == (const G42BaseWriteByteIterator & ptr)
		{ return Ptr == ptr.Ptr; }
	bool operator != (const G42BaseWriteByteIterator & ptr)
		{ return Ptr != ptr.Ptr; }
	operator byte (void)
		{ return *Ptr; }
	G42BaseWriteByteIterator & operator * (void)
		{ return *this; }
	G42BaseWriteByteIterator operator ++ (int)
		{ Ptr++; return G42BaseWriteByteIterator<T>(StartPtr, Ptr - StartPtr - 1); }
	G42BaseWriteByteIterator & operator ++ (void)
		{ ++Ptr; return *this; }
	G42BaseWriteByteIterator operator -- (int)
		{ Ptr--; return G42BaseWriteByteIterator<T>(StartPtr, Ptr - StartPtr + 1); }
	G42BaseWriteByteIterator & operator -- (void)
		{ --Ptr; return *this; }
	G42BaseWriteByteIterator & operator += (int n)
		{ Ptr += n; return *this; }
	G42BaseWriteByteIterator & operator -= (int n)
		{ Ptr -= n; return *this; }
	G42BaseWriteByteIterator operator + (int n)
		{ return G42BaseWriteByteIterator<T>(Ptr + n); }
	G42BaseWriteByteIterator operator - (int n)
		{ return G42BaseWriteByteIterator<T>(Ptr - n); }
	int operator - (const G42BaseWriteByteIterator & ref)
		{ return Ptr - ref.Ptr; }
	G42BaseWriteByteIterator operator [] (int index)
		{ return G42BaseWriteByteIterator<T>(StartPtr, index); }
	bool operator > (const G42BaseWriteByteIterator & ref)
		{ return Ptr > ref.Ptr; }
	bool operator < (const G42BaseWriteByteIterator & ref)
		{ return Ptr < ref.Ptr; }
	bool operator >= (const G42BaseWriteByteIterator & ref)
		{ return Ptr >= ref.Ptr; }
	bool operator <= (const G42BaseWriteByteIterator & ref)
		{ return Ptr <= ref.Ptr; }
	byte operator = (byte value)
		{ *Ptr = value; return value; }
protected:
	T Ptr;
	T StartPtr;
};
#if 0
// this is what I _want_ to do
template <class T>
class G42ByteWriteIterator : public G42BaseByteIterator<T>
{
public:
	G42ByteWriteIterator(void) : G42BaseByteIterator<T>() {}
	G42ByteWriteIterator(T ptr, int offset = 0) :
		G42BaseByteIterator<T>(ptr, offset) {}
	G42ByteWriteIterator(const G42ByteWriteIterator<T> & ptr) :
		G42BaseByteIterator<T>(ptr) {}
	byte operator = (byte value)
		{ *Ptr = value; return value; }
};
#endif
typedef G42BaseWriteByteIterator<byte *> G42ByteIterator;
typedef G42BaseByteIterator<const byte *> G42ConstByteIterator;
// safe iterators, returns 0xff if no pointer - for transparency
template <class T>
class G42SafeBaseByteIterator
{
public:
	G42SafeBaseByteIterator(void) : Ptr(0), StartPtr(0) {}
	G42SafeBaseByteIterator(T ptr, int offset = 0) :
		Ptr(ptr), StartPtr(ptr) { if (Ptr) { Ptr += offset; } }
	G42SafeBaseByteIterator(const G42SafeBaseByteIterator & ptr) :
		Ptr(ptr.Ptr), StartPtr(ptr.StartPtr) {}
	G42SafeBaseByteIterator & operator = (const G42SafeBaseByteIterator & ptr)
		{ Ptr = ptr.Ptr; StartPtr = ptr.StartPtr; return *this; }
	bool operator == (const G42SafeBaseByteIterator & ptr)
		{ return Ptr == ptr.Ptr; }
	bool operator != (const G42SafeBaseByteIterator & ptr)
		{ return Ptr != ptr.Ptr; }
	operator byte (void)
		{ return (Ptr ? *Ptr : (byte)0xff); }
	G42SafeBaseByteIterator & operator * (void)
		{ return *this; }
	G42SafeBaseByteIterator operator ++ (int)
		{ if (Ptr) { Ptr++;
			return G42SafeBaseByteIterator<T>(StartPtr, Ptr - StartPtr - 1); }
			return *this; }
	G42SafeBaseByteIterator & operator ++ (void)
		{ if (Ptr) { ++Ptr; } return *this; }
	G42SafeBaseByteIterator operator -- (int)
		{ if (Ptr) { Ptr--;
			return G42SafeBaseByteIterator<T>(StartPtr, Ptr - StartPtr + 1); }
			return *this; }
	G42SafeBaseByteIterator & operator -- (void)
		{ if (Ptr) { --Ptr; } return *this; }
	G42SafeBaseByteIterator & operator += (int n)
		{ if (Ptr) { Ptr += n; } return *this; }
	G42SafeBaseByteIterator & operator -= (int n)
		{ if (Ptr) { Ptr -= n; } return *this; }
	G42SafeBaseByteIterator operator + (int n)
		{ if (Ptr) { return G42SafeBaseByteIterator<T>(Ptr + n); }
			return *this; }
	G42SafeBaseByteIterator operator - (int n)
		{ if (Ptr) { return G42SafeBaseByteIterator<T>(Ptr - n); }
			return *this; }
	int operator - (const G42SafeBaseByteIterator & ref)
		{ if (Ptr && ref.Ptr) { return Ptr - ref.Ptr; } return 0; }
	G42SafeBaseByteIterator operator [] (int index)
		{ return G42SafeBaseByteIterator<T>(StartPtr, index); }
	bool operator > (const G42SafeBaseByteIterator & ref)
		{ return Ptr > ref.Ptr; }
	bool operator < (const G42SafeBaseByteIterator & ref)
		{ return Ptr < ref.Ptr; }
	bool operator >= (const G42SafeBaseByteIterator & ref)
		{ return Ptr >= ref.Ptr; }
	bool operator <= (const G42SafeBaseByteIterator & ref)
		{ return Ptr <= ref.Ptr; }
protected:
	T Ptr;
	T StartPtr;
};
// the only difference between this and above is op = (byte), but
// I can't get this class to derive from the above class.  Ug.
template <class T>
class G42SafeBaseWriteByteIterator
{
public:
	G42SafeBaseWriteByteIterator(void) : Ptr(0), StartPtr(0) {}
	G42SafeBaseWriteByteIterator(T ptr, int offset = 0) :
		Ptr(ptr), StartPtr(ptr) { if (Ptr) { Ptr += offset; } }
	G42SafeBaseWriteByteIterator(const G42SafeBaseWriteByteIterator & ptr) :
		Ptr(ptr.Ptr), StartPtr(ptr.StartPtr) {}
	G42SafeBaseWriteByteIterator & operator = (const G42SafeBaseWriteByteIterator & ptr)
		{ Ptr = ptr.Ptr; StartPtr = ptr.StartPtr; return *this; }
	bool operator == (const G42SafeBaseWriteByteIterator & ptr)
		{ return Ptr == ptr.Ptr; }
	bool operator != (const G42SafeBaseWriteByteIterator & ptr)
		{ return Ptr != ptr.Ptr; }
	operator byte (void)
		{ return ( Ptr ? *Ptr : (byte)0xff); }
	G42SafeBaseWriteByteIterator & operator * (void)
		{ return *this; }
	G42SafeBaseWriteByteIterator operator ++ (int)
		{ if (Ptr) { Ptr++;
			return G42SafeBaseWriteByteIterator<T>(StartPtr, Ptr - StartPtr - 1); }
			return *this; }
	G42SafeBaseWriteByteIterator & operator ++ (void)
		{ if (Ptr) { ++Ptr; } return *this; }
	G42SafeBaseWriteByteIterator operator -- (int)
		{ if (Ptr) { Ptr--;
			return G42SafeBaseWriteByteIterator<T>(StartPtr, Ptr - StartPtr + 1); }
			return *this; }
	G42SafeBaseWriteByteIterator & operator -- (void)
		{ if (Ptr) { --Ptr; } return *this; }
	G42SafeBaseWriteByteIterator & operator += (int n)
		{ if (Ptr) { Ptr += n; } return *this; }
	G42SafeBaseWriteByteIterator & operator -= (int n)
		{ if (Ptr) { Ptr -= n; } return *this; }
	G42SafeBaseWriteByteIterator operator + (int n)
		{ if (Ptr) { return G42SafeBaseWriteByteIterator<T>(Ptr + n); }
			return *this; }
	G42SafeBaseWriteByteIterator operator - (int n)
		{ if (Ptr) { return G42SafeBaseWriteByteIterator<T>(Ptr - n); }
			return *this; }
	int operator - (const G42SafeBaseWriteByteIterator & ref)
		{ if (Ptr) { return Ptr - ref.Ptr; } return 0; }
	G42SafeBaseWriteByteIterator operator [] (int index)
		{ if (Ptr) { return G42SafeBaseWriteByteIterator<T>(StartPtr, index); }
			return *this; }
	bool operator > (const G42SafeBaseWriteByteIterator & ref)
		{ return Ptr > ref.Ptr; }
	bool operator < (const G42SafeBaseWriteByteIterator & ref)
		{ return Ptr < ref.Ptr; }
	bool operator >= (const G42SafeBaseWriteByteIterator & ref)
		{ return Ptr >= ref.Ptr; }
	bool operator <= (const G42SafeBaseWriteByteIterator & ref)
		{ return Ptr <= ref.Ptr; }
	byte operator = (byte value)
		{ if (Ptr) { *Ptr = value; } return value; }
protected:
	T Ptr;
	T StartPtr;
};
typedef G42SafeBaseWriteByteIterator<byte *> G42SafeByteIterator;
typedef G42SafeBaseByteIterator<const byte *> G42ConstSafeByteIterator;
template <class T, int SkipByte, int SequenceBytes>
class G42BaseSkipByteIterator
{
public:
	G42BaseSkipByteIterator(void) : Ptr(0), StartPtr(0), SkipCount(0) {}
	G42BaseSkipByteIterator(T ptr, int offset = 0) :
		Ptr(ptr + offset), StartPtr(ptr), SkipCount(offset % SequenceBytes) {}
	G42BaseSkipByteIterator(const G42BaseSkipByteIterator & ptr) :
		Ptr(ptr.Ptr), StartPtr(ptr.StartPtr), SkipCount(ptr.SkipCount) {}
	G42BaseSkipByteIterator & operator =
		(const G42BaseSkipByteIterator & ptr)
		{ Ptr = ptr.Ptr; StartPtr = ptr.StartPtr; SkipCount = ptr.SkipCount;
			return *this; }
	bool operator == (const G42BaseSkipByteIterator & ptr)
		{ return Ptr == ptr.Ptr; }
	bool operator != (const G42BaseSkipByteIterator & ptr)
		{ return Ptr != ptr.Ptr; }
	operator byte (void)
		{ return (byte)((SkipCount == SkipByte) ? 0 : *Ptr); }
	G42BaseSkipByteIterator & operator * (void)
		{ return *this; }
	G42BaseSkipByteIterator operator ++ (int)
		{
			G42BaseSkipByteIterator<T, SkipByte, SequenceBytes> i = *this;
			Ptr++; SkipCount++; if (SkipCount == SequenceBytes) SkipCount = 0;
			return i;
		}
	G42BaseSkipByteIterator & operator ++ (void)
		{ Ptr++; SkipCount++; if (SkipCount == SequenceBytes) SkipCount = 0;
			return *this; }
	G42BaseSkipByteIterator operator -- (int)
		{
			G42BaseSkipByteIterator<T, SkipByte, SequenceBytes> i = *this;
			Ptr--; if (!SkipCount) SkipCount = SequenceBytes - 1;
			else SkipCount--; return i;
		}
	G42BaseSkipByteIterator & operator -- (void)
		{
			Ptr--; if (!SkipCount) SkipCount = SequenceBytes - 1;
			else SkipCount--; return *this;
		}
	G42BaseSkipByteIterator & operator += (int n)
		{ Ptr += n; SkipCount += n; SkipCount %= SequenceBytes; return *this; }
	G42BaseSkipByteIterator & operator -= (int n)
		{ Ptr -= n; SkipCount += n; SkipCount %= SequenceBytes;
			if (SkipCount < 0) SkipCount += SequenceBytes; return *this; }
	G42BaseSkipByteIterator operator + (int n)
		{ return G42BaseSkipByteIterator<
			T, SkipByte, SequenceBytes>(Ptr + n); }
	G42BaseSkipByteIterator operator - (int n)
		{ return G42BaseSkipByteIterator<
			T, SkipByte, SequenceBytes>(Ptr - n); }
	int operator - (const G42BaseSkipByteIterator & ref)
		{ return Ptr - ref.Ptr; }
	G42BaseSkipByteIterator operator [] (int index)
		{ return G42BaseSkipByteIterator<
			T, SkipByte, SequenceBytes>(StartPtr, index); }
	bool operator > (const G42BaseSkipByteIterator & ref)
		{ return Ptr > ref.Ptr; }
	bool operator < (const G42BaseSkipByteIterator & ref)
		{ return Ptr < ref.Ptr; }
	bool operator >= (const G42BaseSkipByteIterator & ref)
		{ return Ptr >= ref.Ptr; }
	bool operator <= (const G42BaseSkipByteIterator & ref)
		{ return Ptr <= ref.Ptr; }
protected:
	T Ptr;
	T StartPtr;
	intf SkipCount;
};
template <class T, int SkipByte, int SequenceBytes>
class G42BaseWriteSkipByteIterator
{
public:
	G42BaseWriteSkipByteIterator(void) : Ptr(0), StartPtr(0), SkipCount(0) {}
	G42BaseWriteSkipByteIterator(T ptr, int offset = 0) :
		Ptr(ptr + offset), StartPtr(ptr), SkipCount(offset % SequenceBytes) {}
	G42BaseWriteSkipByteIterator(const G42BaseWriteSkipByteIterator & ptr) :
		Ptr(ptr.Ptr), StartPtr(ptr.StartPtr), SkipCount(ptr.SkipCount) {}
	G42BaseWriteSkipByteIterator & operator =
		(const G42BaseWriteSkipByteIterator & ptr)
		{ Ptr = ptr.Ptr; StartPtr = ptr.StartPtr; SkipCount = ptr.SkipCount;
			return *this; }
	bool operator == (const G42BaseWriteSkipByteIterator & ptr)
		{ return Ptr == ptr.Ptr; }
	bool operator != (const G42BaseWriteSkipByteIterator & ptr)
		{ return Ptr != ptr.Ptr; }
	operator byte (void)
		{ return (byte)((SkipCount == SkipByte) ? 0 : *Ptr); }
	G42BaseWriteSkipByteIterator & operator * (void)
		{ return *this; }
	G42BaseWriteSkipByteIterator operator ++ (int)
		{
			G42BaseWriteSkipByteIterator<T, SkipByte, SequenceBytes> i = *this;
			Ptr++; SkipCount++; if (SkipCount == SequenceBytes) SkipCount = 0;
			return i;
		}
	G42BaseWriteSkipByteIterator & operator ++ (void)
		{ Ptr++; SkipCount++; if (SkipCount == SequenceBytes) SkipCount = 0;
			return *this; }
	G42BaseWriteSkipByteIterator operator -- (int)
		{
			G42BaseWriteSkipByteIterator<T, SkipByte, SequenceBytes> i = *this;
			Ptr--; if (!SkipCount) SkipCount = SequenceBytes - 1;
			else SkipCount--; return i;
		}
	G42BaseWriteSkipByteIterator & operator -- (void)
		{
			Ptr--; if (!SkipCount) SkipCount = SequenceBytes - 1;
			else SkipCount--; return *this;
		}
	G42BaseWriteSkipByteIterator & operator += (int n)
		{ Ptr += n; SkipCount += n; SkipCount %= SequenceBytes; return *this; }
	G42BaseWriteSkipByteIterator & operator -= (int n)
		{ Ptr -= n; SkipCount += n; SkipCount %= SequenceBytes;
			if (SkipCount < 0) SkipCount += SequenceBytes; return *this; }
	G42BaseWriteSkipByteIterator operator + (int n)
		{ return G42BaseWriteSkipByteIterator<
			T, SkipByte, SequenceBytes>(Ptr + n); }
	G42BaseWriteSkipByteIterator operator - (int n)
		{ return G42BaseWriteSkipByteIterator<
			T, SkipByte, SequenceBytes>(Ptr - n); }
	int operator - (const G42BaseWriteSkipByteIterator & ref)
		{ return Ptr - ref.Ptr; }
	G42BaseWriteSkipByteIterator operator [] (int index)
		{ return G42BaseWriteSkipByteIterator<
			T, SkipByte, SequenceBytes>(StartPtr, index); }
	bool operator > (const G42BaseWriteSkipByteIterator & ref)
		{ return Ptr > ref.Ptr; }
	bool operator < (const G42BaseWriteSkipByteIterator & ref)
		{ return Ptr < ref.Ptr; }
	bool operator >= (const G42BaseWriteSkipByteIterator & ref)
		{ return Ptr >= ref.Ptr; }
	bool operator <= (const G42BaseWriteSkipByteIterator & ref)
		{ return Ptr <= ref.Ptr; }
	byte operator = (byte value)
		{ if (SkipCount == SkipByte) *Ptr = G42TrueColorFillerByte;
			else *Ptr = value;
			return value; }
protected:
	T Ptr;
	T StartPtr;
	intf SkipCount;
};
typedef G42BaseWriteSkipByteIterator<byte *, G42TrueColorFillerOffset, 4>
	G42SkipByteIterator;
typedef G42BaseSkipByteIterator<const byte *, G42TrueColorFillerOffset, 4>
	G42ConstSkipByteIterator;
template <class T, class ColorType>
class G42BaseColor24Iterator
{
public:
	G42BaseColor24Iterator(void) : Ptr(0), StartPtr(0) {}
	G42BaseColor24Iterator(T ptr, int offset = 0) :
		Ptr(ptr + offset + (offset << 1)), StartPtr(ptr) {}
	G42BaseColor24Iterator(const G42BaseColor24Iterator & ptr) :
		Ptr(ptr.Ptr), StartPtr(ptr.StartPtr) {}
	G42BaseColor24Iterator & operator = (const G42BaseColor24Iterator & ptr)
		{ Ptr = ptr.Ptr; StartPtr = ptr.StartPtr; return *this; }
	bool operator == (const G42BaseColor24Iterator & ptr)
		{ return Ptr == ptr.Ptr; }
	bool operator != (const G42BaseColor24Iterator & ptr)
		{ return Ptr != ptr.Ptr; }
	G42BaseColor24Iterator & operator * (void)
		{ return *this; }
	operator ColorType (void)
		{ return ColorType(Ptr); }
	G42BaseColor24Iterator operator ++ (int)
		{ G42BaseColor24Iterator<T, ColorType> i(*this); Ptr += 3; return i; }
	G42BaseColor24Iterator & operator ++ (void)
		{ Ptr += 3; return *this; }
	G42BaseColor24Iterator operator -- (int)
		{ G42BaseColor24Iterator<T, ColorType> i(*this); Ptr -= 3; return i; }
	G42BaseColor24Iterator & operator -- (void)
		{ Ptr -= 3; return *this; }
	G42BaseColor24Iterator & operator += (int n)
		{ Ptr += (n + (n << 1)); return *this; }
	G42BaseColor24Iterator & operator -= (int n)
		{ Ptr -= (n + (n << 1)); return *this; }
	G42BaseColor24Iterator operator + (int n)
		{ G42BaseColor24Iterator<T, ColorType> i(*this); i += n; return i; }
	G42BaseColor24Iterator operator - (int n)
		{ G42BaseColor24Iterator<T, ColorType> i(*this); i -= n; return i; }
	int operator - (const G42BaseColor24Iterator & ref)
		{ return (Ptr - ref.Ptr) / 3; }
	G42BaseColor24Iterator operator [] (int index)
		{ return G42BaseColor24Iterator<T, ColorType>(StartPtr, index); }
	bool operator > (const G42BaseColor24Iterator & ref)
		{ return Ptr > ref.Ptr; }
	bool operator < (const G42BaseColor24Iterator & ref)
		{ return Ptr < ref.Ptr; }
	bool operator >= (const G42BaseColor24Iterator & ref)
		{ return Ptr >= ref.Ptr; }
	bool operator <= (const G42BaseColor24Iterator & ref)
		{ return Ptr <= ref.Ptr; }
protected:
	T Ptr;
	T StartPtr;
};
// the only difference between this and above is op = (color) etc., but
// I can't get this class to derive from the above class.  Ug.
template <class T, class ColorType>
class G42BaseWriteColor24Iterator
{
public:
	G42BaseWriteColor24Iterator(void) : Ptr(0), StartPtr(0) {}
	G42BaseWriteColor24Iterator(T ptr, int offset = 0) :
		Ptr(ptr + offset + (offset << 1)), StartPtr(ptr) {}
	G42BaseWriteColor24Iterator(const G42BaseWriteColor24Iterator & ptr) :
		Ptr(ptr.Ptr), StartPtr(ptr.StartPtr) {}
	G42BaseWriteColor24Iterator & operator = (const G42BaseWriteColor24Iterator & ptr)
		{ Ptr = ptr.Ptr; StartPtr = ptr.StartPtr; return *this; }
	bool operator == (const G42BaseWriteColor24Iterator & ptr)
		{ return Ptr == ptr.Ptr; }
	bool operator != (const G42BaseWriteColor24Iterator & ptr)
		{ return Ptr != ptr.Ptr; }
	G42BaseWriteColor24Iterator & operator * (void)
		{ return *this; }
	operator ColorType (void)
		{ return ColorType(Ptr); }
	G42BaseWriteColor24Iterator operator ++ (int)
		{ G42BaseWriteColor24Iterator<T, ColorType> i(*this); Ptr += 3; return i; }
	G42BaseWriteColor24Iterator & operator ++ (void)
		{ Ptr += 3; return *this; }
	G42BaseWriteColor24Iterator operator -- (int)
		{ G42BaseWriteColor24Iterator<T, ColorType> i(*this); Ptr -= 3; return i; }
	G42BaseWriteColor24Iterator & operator -- (void)
		{ Ptr -= 3; return *this; }
	G42BaseWriteColor24Iterator & operator += (int n)
		{ Ptr += (n + (n << 1)); return *this; }
	G42BaseWriteColor24Iterator & operator -= (int n)
		{ Ptr -= (n + (n << 1)); return *this; }
	G42BaseWriteColor24Iterator operator + (int n)
		{ G42BaseWriteColor24Iterator<T, ColorType> i(*this); i += n; return i; }
	G42BaseWriteColor24Iterator operator - (int n)
		{ G42BaseWriteColor24Iterator<T, ColorType> i(*this); i -= n; return i; }
	int operator - (const G42BaseWriteColor24Iterator & ref)
		{ return (Ptr - ref.Ptr) / 3; }
	G42BaseWriteColor24Iterator operator [] (int index)
		{ return G42BaseWriteColor24Iterator<T, ColorType>(StartPtr, index); }
	bool operator > (const G42BaseWriteColor24Iterator & ref)
		{ return Ptr > ref.Ptr; }
	bool operator < (const G42BaseWriteColor24Iterator & ref)
		{ return Ptr < ref.Ptr; }
	bool operator >= (const G42BaseWriteColor24Iterator & ref)
		{ return Ptr >= ref.Ptr; }
	bool operator <= (const G42BaseWriteColor24Iterator & ref)
		{ return Ptr <= ref.Ptr; }
	G42Color24 operator = (G42Color24 value)
//		{ G42Color24(Ptr).Set(value); return value; }
		{ G42Color24 p(Ptr); p.Set(value); return value; }
	G42ConstColor24 operator = (G42ConstColor24 value)
//		{ G42Color24(Ptr).Set(value); return value; }
		{ G42Color24 p(Ptr); p.Set(value); return value; }
	G42Color32 operator = (G42Color32 value)
		{ G42Color24 p(Ptr); p.Set(value); return value; }
	G42ConstColor32 operator = (G42ConstColor32 value)
		{ G42Color24 p(Ptr); p.Set(value); return value; }
	const G42Color & operator = (const G42Color & value)
		{ G42Color24 p(Ptr); p.Set(value); return value; }
protected:
	T Ptr;
	T StartPtr;
};
typedef G42BaseWriteColor24Iterator<byte *, G42Color24> G42Color24Iterator;
typedef G42BaseColor24Iterator<const byte *, G42ConstColor24>
	G42ConstColor24Iterator;
template <class T, class ColorType>
class G42BaseColor32Iterator
{
public:
	G42BaseColor32Iterator(void) : Ptr(0), StartPtr(0) {}
	G42BaseColor32Iterator(T ptr, int offset = 0) :
		Ptr(ptr + (offset << 2)), StartPtr(ptr) {}
	G42BaseColor32Iterator(const G42BaseColor32Iterator & ptr) :
		Ptr(ptr.Ptr), StartPtr(ptr.StartPtr) {}
	G42BaseColor32Iterator & operator = (const G42BaseColor32Iterator & ptr)
		{ Ptr = ptr.Ptr; StartPtr = ptr.StartPtr; return *this; }
	bool operator == (const G42BaseColor32Iterator & ptr)
		{ return Ptr == ptr.Ptr; }
	bool operator != (const G42BaseColor32Iterator & ptr)
		{ return Ptr != ptr.Ptr; }
	operator ColorType (void)
		{ return ColorType(Ptr); }
	G42BaseColor32Iterator & operator * (void)
		{ return *this; }
	G42BaseColor32Iterator operator ++ (int)
		{ G42BaseColor32Iterator<T, ColorType> i(*this); Ptr += 4; return i; }
	G42BaseColor32Iterator & operator ++ (void)
		{ Ptr += 4; return *this; }
	G42BaseColor32Iterator operator -- (int)
		{ G42BaseColor32Iterator<T, ColorType> i(*this); Ptr -= 4; return i; }
	G42BaseColor32Iterator & operator -- (void)
		{ Ptr -= 4; return *this; }
	G42BaseColor32Iterator & operator += (int n)
		{ Ptr += (n << 2); return *this; }
	G42BaseColor32Iterator & operator -= (int n)
		{ Ptr -= (n << 2); return *this; }
	G42BaseColor32Iterator operator + (int n)
		{ G42BaseColor32Iterator<T, ColorType> i(*this); i += n; return i; }
	G42BaseColor32Iterator operator - (int n)
		{ G42BaseColor32Iterator<T, ColorType> i(*this); i -= n; return i; }
	int operator - (const G42BaseColor32Iterator & ref)
		{ return (Ptr - ref.Ptr) >> 2; }
	G42BaseColor32Iterator operator [] (int index)
		{ return G42BaseColor32Iterator<T, ColorType>(StartPtr, index); }
	bool operator > (const G42BaseColor32Iterator & ref)
		{ return Ptr > ref.Ptr; }
	bool operator < (const G42BaseColor32Iterator & ref)
		{ return Ptr < ref.Ptr; }
	bool operator >= (const G42BaseColor32Iterator & ref)
		{ return Ptr >= ref.Ptr; }
	bool operator <= (const G42BaseColor32Iterator & ref)
		{ return Ptr <= ref.Ptr; }
protected:
	T Ptr;
	T StartPtr;
};
// the only difference between this and above is op = (color) etc., but
// I can't get this class to derive from the above class.  Ug.
template <class T, class ColorType>
class G42BaseWriteColor32Iterator
{
public:
	G42BaseWriteColor32Iterator(void) : Ptr(0), StartPtr(0) {}
	G42BaseWriteColor32Iterator(T ptr, int offset = 0) :
		Ptr(ptr + (offset << 2)), StartPtr(ptr) {}
	G42BaseWriteColor32Iterator(const G42BaseWriteColor32Iterator & ptr) :
		Ptr(ptr.Ptr), StartPtr(ptr.StartPtr) {}
	G42BaseWriteColor32Iterator & operator = (const G42BaseWriteColor32Iterator & ptr)
		{ Ptr = ptr.Ptr; StartPtr = ptr.StartPtr; return *this; }
	bool operator == (const G42BaseWriteColor32Iterator & ptr)
		{ return Ptr == ptr.Ptr; }
	bool operator != (const G42BaseWriteColor32Iterator & ptr)
		{ return Ptr != ptr.Ptr; }
	operator ColorType (void)
		{ return ColorType(Ptr); }
	G42BaseWriteColor32Iterator & operator * (void)
		{ return *this; }
	G42BaseWriteColor32Iterator operator ++ (int)
		{ G42BaseWriteColor32Iterator<T, ColorType> i(*this); Ptr += 4; return i; }
	G42BaseWriteColor32Iterator & operator ++ (void)
		{ Ptr += 4; return *this; }
	G42BaseWriteColor32Iterator operator -- (int)
		{ G42BaseWriteColor32Iterator<T, ColorType> i(*this); Ptr -= 4; return i; }
	G42BaseWriteColor32Iterator & operator -- (void)
		{ Ptr -= 4; return *this; }
	G42BaseWriteColor32Iterator & operator += (int n)
		{ Ptr += (n << 2); return *this; }
	G42BaseWriteColor32Iterator & operator -= (int n)
		{ Ptr -= (n << 2); return *this; }
	G42BaseWriteColor32Iterator operator + (int n)
		{ G42BaseWriteColor32Iterator<T, ColorType> i(*this); i += n; return i; }
	G42BaseWriteColor32Iterator operator - (int n)
		{ G42BaseWriteColor32Iterator<T, ColorType> i(*this); i -= n; return i; }
	int operator - (const G42BaseWriteColor32Iterator & ref)
		{ return (Ptr - ref.Ptr) >> 2; }
	G42BaseWriteColor32Iterator operator [] (int index)
		{ return G42BaseWriteColor32Iterator<T, ColorType>(StartPtr, index); }
	bool operator > (const G42BaseWriteColor32Iterator & ref)
		{ return Ptr > ref.Ptr; }
	bool operator < (const G42BaseWriteColor32Iterator & ref)
		{ return Ptr < ref.Ptr; }
	bool operator >= (const G42BaseWriteColor32Iterator & ref)
		{ return Ptr >= ref.Ptr; }
	bool operator <= (const G42BaseWriteColor32Iterator & ref)
		{ return Ptr <= ref.Ptr; }
	G42Color24 operator = (G42Color24 value)
		{ G42Color32 p(Ptr); p.Set(value); return value; }
	G42ConstColor24 operator = (G42ConstColor24 value)
		{ G42Color32 p(Ptr); p.Set(value); return value; }
	G42Color32 operator = (G42Color32 value)
		{ G42Color32 p(Ptr); p.Set(value); return value; }
	G42ConstColor32 operator = (G42ConstColor32 value)
		{ G42Color32 p(Ptr); p.Set(value); return value; }
	const G42Color & operator = (const G42Color & value)
		{ G42Color32 p(Ptr); p.Set(value); return value; }
protected:
	T Ptr;
	T StartPtr;
};
typedef G42BaseWriteColor32Iterator<byte *, G42Color32> G42Color32Iterator;
typedef G42BaseColor32Iterator<const byte *, G42ConstColor32>
	G42ConstColor32Iterator;
// an image row, where G42Iterator is the G42Iterator it returns and BytePtr
// is either byte * or const byte *
template <class IteratorType, class BytePtr>
class G42ImageRow
{
public:
	typedef IteratorType G42Iterator;
	G42ImageRow(BytePtr ptr, uint32 width) : Ptr(ptr), Width(width) {}
	G42ImageRow() : Ptr(0), Width(0) {}
	G42ImageRow(const G42ImageRow & ref) :
		Ptr(ref.Ptr), Width(ref.Width) {}
	G42ImageRow & operator = (const G42ImageRow & ptr)
		{ Ptr = ptr.Ptr; Width = ptr.Width; return *this; }
	~G42ImageRow(void) {}
	G42Iterator GetBegin(void)
		{ return G42Iterator(Ptr); }
	G42Iterator GetEnd(void)
		{ return G42Iterator(Ptr, Width); }
	BytePtr GetPtr(void)
		{ return Ptr; }
	uint32 GetWidth(void)
		{ return Width; }
private:
	BytePtr Ptr;
	uint32 Width;
};
template <class RowType, class LockType>
class G42ImageStripIterator
{
public:
	typedef RowType G42Row;
	G42ImageStripIterator(void) : Ptr(0), Row(0) {}
	G42ImageStripIterator(LockType & ptr, int index = 0) :
		Ptr(&ptr), Row(index) {}
	G42ImageStripIterator(const G42ImageStripIterator & ptr) :
		Ptr(ptr.Ptr), Row(ptr.Row) {}
	G42ImageStripIterator & operator = (const G42ImageStripIterator & ptr)
		{ Ptr = ptr.Ptr; Row = ptr.Row; return *this; }
	~G42ImageStripIterator(void) {}
	bool operator == (const G42ImageStripIterator & ptr)
		{ return (Row == ptr.Row); }
	bool operator != (const G42ImageStripIterator & ptr)
		{ return (Row != ptr.Row); }
	G42Row operator * (void)
		{ return G42Row(Ptr->GetRow(Row), Ptr->GetData().GetWidth()); }
	G42ImageStripIterator operator ++ (int)
		{ return G42ImageStripIterator<RowType, LockType>(*Ptr, Row++); }
	G42ImageStripIterator & operator ++ (void)
		{ Row++; return *this; }
	G42ImageStripIterator operator -- (int)
		{ return G42ImageStripIterator<RowType, LockType>(*Ptr, Row--); }
	G42ImageStripIterator & operator -- (void)
		{ Row--; return *this; }
	G42ImageStripIterator & operator += (int n)
		{ Row += n; return *this; }
	G42ImageStripIterator & operator -= (int n)
		{ Row -= n; return *this; }
	G42ImageStripIterator operator + (int n)
		{ return G42ImageStripIterator<RowType, LockType>(*Ptr, Row + n); }
	G42ImageStripIterator operator - (int n)
		{ return G42ImageStripIterator<RowType, LockType>(*Ptr, Row - n); }
	int operator - (const G42ImageStripIterator & ref)
		{ return Row - ref.Row; }
	G42Row operator [] (int index)
		{ return G42Row(Ptr->GetRow(index), Ptr->GetData().GetWidth()); }
	bool operator > (const G42ImageStripIterator & ref)
		{ return Row > ref.Row; }
	bool operator < (const G42ImageStripIterator & ref)
		{ return Row < ref.Row; }
	bool operator >= (const G42ImageStripIterator & ref)
		{ return Row >= ref.Row; }
	bool operator <= (const G42ImageStripIterator & ref)
		{ return Row <= ref.Row; }
protected:
	LockType * Ptr;
	uint32 Row;
};
// these are the same as above except for operator * and operator []
// but I can't get these to derive from above
template <class RowType, class LockType>
class G42ImageStripByte1Iterator
{
public:
	typedef RowType G42Row;
	G42ImageStripByte1Iterator(void) : Ptr(0), Row(0) {}
	G42ImageStripByte1Iterator(LockType & ptr, int index = 0) :
		Ptr(&ptr), Row(index) {}
	G42ImageStripByte1Iterator(const G42ImageStripByte1Iterator & ptr) :
		Ptr(ptr.Ptr), Row(ptr.Row) {}
	G42ImageStripByte1Iterator & operator = (const G42ImageStripByte1Iterator & ptr)
		{ Ptr = ptr.Ptr; Row = ptr.Row; return *this; }
	~G42ImageStripByte1Iterator(void) {}
	bool operator == (const G42ImageStripByte1Iterator & ptr)
		{ return (Row == ptr.Row); }
	bool operator != (const G42ImageStripByte1Iterator & ptr)
		{ return (Row != ptr.Row); }
	G42Row operator * (void)
		{ return G42Row(Ptr->GetRow(Row), ((Ptr->GetData().GetWidth() + 7) >> 3)); }
	G42ImageStripByte1Iterator operator ++ (int)
		{ return G42ImageStripByte1Iterator<RowType, LockType>(*Ptr, Row++); }
	G42ImageStripByte1Iterator & operator ++ (void)
		{ Row++; return *this; }
	G42ImageStripByte1Iterator operator -- (int)
		{ return G42ImageStripByte1Iterator<RowType, LockType>(*Ptr, Row--); }
	G42ImageStripByte1Iterator & operator -- (void)
		{ Row--; return *this; }
	G42ImageStripByte1Iterator & operator += (int n)
		{ Row += n; return *this; }
	G42ImageStripByte1Iterator & operator -= (int n)
		{ Row -= n; return *this; }
	G42ImageStripByte1Iterator operator + (int n)
		{ return G42ImageStripByte1Iterator<RowType, LockType>(*Ptr, Row + n); }
	G42ImageStripByte1Iterator operator - (int n)
		{ return G42ImageStripByte1Iterator<RowType, LockType>(*Ptr, Row - n); }
	int operator - (const G42ImageStripByte1Iterator & ref)
		{ return Row - ref.Row; }
	G42Row operator [] (int index)
		{ return G42Row(Ptr->GetRow(index),
			((Ptr->GetData().GetWidth() + 7) >> 3)); }
	bool operator > (const G42ImageStripByte1Iterator & ref)
		{ return Row > ref.Row; }
	bool operator < (const G42ImageStripByte1Iterator & ref)
		{ return Row < ref.Row; }
	bool operator >= (const G42ImageStripByte1Iterator & ref)
		{ return Row >= ref.Row; }
	bool operator <= (const G42ImageStripByte1Iterator & ref)
		{ return Row <= ref.Row; }
protected:
	LockType * Ptr;
	uint32 Row;
};
template <class RowType, class LockType>
class G42ImageStripByte24Iterator
{
public:
	typedef RowType G42Row;
	G42ImageStripByte24Iterator(void) : Ptr(0), Row(0) {}
	G42ImageStripByte24Iterator(LockType & ptr, int index = 0) :
		Ptr(&ptr), Row(index) {}
	G42ImageStripByte24Iterator(const G42ImageStripByte24Iterator & ptr) :
		Ptr(ptr.Ptr), Row(ptr.Row) {}
	G42ImageStripByte24Iterator & operator = (const G42ImageStripByte24Iterator & ptr)
		{ Ptr = ptr.Ptr; Row = ptr.Row; return *this; }
	~G42ImageStripByte24Iterator(void) {}
	bool operator == (const G42ImageStripByte24Iterator & ptr)
		{ return (Row == ptr.Row); }
	bool operator != (const G42ImageStripByte24Iterator & ptr)
		{ return (Row != ptr.Row); }
	G42Row operator * (void)
		{ return G42Row(Ptr->GetRow(Row), Ptr->GetData().GetWidth() +
			(Ptr->GetData().GetWidth() << 1)); }
	G42ImageStripByte24Iterator operator ++ (int)
		{ return G42ImageStripByte24Iterator<RowType, LockType>(*Ptr, Row++); }
	G42ImageStripByte24Iterator & operator ++ (void)
		{ Row++; return *this; }
	G42ImageStripByte24Iterator operator -- (int)
		{ return G42ImageStripByte24Iterator<RowType, LockType>(*Ptr, Row--); }
	G42ImageStripByte24Iterator & operator -- (void)
		{ Row--; return *this; }
	G42ImageStripByte24Iterator & operator += (int n)
		{ Row += n; return *this; }
	G42ImageStripByte24Iterator & operator -= (int n)
		{ Row -= n; return *this; }
	G42ImageStripByte24Iterator operator + (int n)
		{ return G42ImageStripByte24Iterator<RowType, LockType>(*Ptr, Row + n); }
	G42ImageStripByte24Iterator operator - (int n)
		{ return G42ImageStripByte24Iterator<RowType, LockType>(*Ptr, Row - n); }
	int operator - (const G42ImageStripByte24Iterator & ref)
		{ return Row - ref.Row; }
	G42Row operator [] (int index)
		{ return G42Row(Ptr->GetRow(index),
			Ptr->GetData().GetWidth() +
			(Ptr->GetData().GetWidth() << 1)); }
	bool operator > (const G42ImageStripByte24Iterator & ref)
		{ return Row > ref.Row; }
	bool operator < (const G42ImageStripByte24Iterator & ref)
		{ return Row < ref.Row; }
	bool operator >= (const G42ImageStripByte24Iterator & ref)
		{ return Row >= ref.Row; }
	bool operator <= (const G42ImageStripByte24Iterator & ref)
		{ return Row <= ref.Row; }
protected:
	LockType * Ptr;
	uint32 Row;
};
template <class RowType, class LockType>
class G42ImageStripByte32Iterator
{
public:
	typedef RowType G42Row;
	G42ImageStripByte32Iterator(void) : Ptr(0), Row(0) {}
	G42ImageStripByte32Iterator(LockType & ptr, int index = 0) :
		Ptr(&ptr), Row(index) {}
	G42ImageStripByte32Iterator(const G42ImageStripByte32Iterator & ptr) :
		Ptr(ptr.Ptr), Row(ptr.Row) {}
	G42ImageStripByte32Iterator & operator = (const G42ImageStripByte32Iterator & ptr)
		{ Ptr = ptr.Ptr; Row = ptr.Row; return *this; }
	~G42ImageStripByte32Iterator(void) {}
	bool operator == (const G42ImageStripByte32Iterator & ptr)
		{ return (Row == ptr.Row); }
	bool operator != (const G42ImageStripByte32Iterator & ptr)
		{ return (Row != ptr.Row); }
	G42Row operator * (void)
		{ return G42Row(Ptr->GetRow(Row), (Ptr->GetData().GetWidth() << 2)); }
	G42ImageStripByte32Iterator operator ++ (int)
		{ return G42ImageStripByte32Iterator<RowType, LockType>(*Ptr, Row++); }
	G42ImageStripByte32Iterator & operator ++ (void)
		{ Row++; return *this; }
	G42ImageStripByte32Iterator operator -- (int)
		{ return G42ImageStripByte32Iterator<RowType, LockType>(*Ptr, Row--); }
	G42ImageStripByte32Iterator & operator -- (void)
		{ Row--; return *this; }
	G42ImageStripByte32Iterator & operator += (int n)
		{ Row += n; return *this; }
	G42ImageStripByte32Iterator & operator -= (int n)
		{ Row -= n; return *this; }
	G42ImageStripByte32Iterator operator + (int n)
		{ return G42ImageStripByte32Iterator<RowType, LockType>(*Ptr, Row + n); }
	G42ImageStripByte32Iterator operator - (int n)
		{ return G42ImageStripByte32Iterator<RowType, LockType>(*Ptr, Row - n); }
	int operator - (const G42ImageStripByte32Iterator & ref)
		{ return Row - ref.Row; }
	G42Row operator [] (int index)
		{ return G42Row(Ptr->GetRow(index),
			(Ptr->GetData().GetWidth() << 2)); }
	bool operator > (const G42ImageStripByte32Iterator & ref)
		{ return Row > ref.Row; }
	bool operator < (const G42ImageStripByte32Iterator & ref)
		{ return Row < ref.Row; }
	bool operator >= (const G42ImageStripByte32Iterator & ref)
		{ return Row >= ref.Row; }
	bool operator <= (const G42ImageStripByte32Iterator & ref)
		{ return Row <= ref.Row; }
protected:
	LockType * Ptr;
	uint32 Row;
};
template <class RowType, class LockType>
class G42ImageStripTransIterator
{
public:
	typedef RowType G42Row;
	G42ImageStripTransIterator(void) : Ptr(0), Row(0) {}
	G42ImageStripTransIterator(LockType & ptr, int index = 0) :
		Ptr(&ptr), Row(index) {}
	G42ImageStripTransIterator(const G42ImageStripTransIterator & ptr) :
		Ptr(ptr.Ptr), Row(ptr.Row) {}
	G42ImageStripTransIterator & operator = (const G42ImageStripTransIterator & ptr)
		{ Ptr = ptr.Ptr; Row = ptr.Row; return *this; }
	~G42ImageStripTransIterator(void) {}
	bool operator == (const G42ImageStripTransIterator & ptr)
		{ return (Row == ptr.Row); }
	bool operator != (const G42ImageStripTransIterator & ptr)
		{ return (Row != ptr.Row); }
	G42Row operator * (void)
		{ return G42Row(
			(Ptr->GetData().IsTransparent() ? Ptr->GetRow(Row, true) : 0),
			Ptr->GetData().GetWidth()); }
	G42ImageStripTransIterator operator ++ (int)
		{ return G42ImageStripTransIterator<RowType, LockType>(*Ptr, Row++); }
	G42ImageStripTransIterator & operator ++ (void)
		{ Row++; return *this; }
	G42ImageStripTransIterator operator -- (int)
		{ return G42ImageStripTransIterator<RowType, LockType>(*Ptr, Row--); }
	G42ImageStripTransIterator & operator -- (void)
		{ Row--; return *this; }
	G42ImageStripTransIterator & operator += (int n)
		{ Row += n; return *this; }
	G42ImageStripTransIterator & operator -= (int n)
		{ Row -= n; return *this; }
	G42ImageStripTransIterator operator + (int n)
		{ return G42ImageStripTransIterator<RowType, LockType>(*Ptr, Row + n); }
	G42ImageStripTransIterator operator - (int n)
		{ return G42ImageStripTransIterator<RowType, LockType>(*Ptr, Row - n); }
	int operator - (const G42ImageStripTransIterator & ref)
		{ return Row - ref.Row; }
	G42Row operator [] (int index)
		{ return G42Row(
			(Ptr->GetData().IsTransparent() ? Ptr->GetRow(index, true) : 0),
			Ptr->GetData().GetWidth()); }
	bool operator > (const G42ImageStripTransIterator & ref)
		{ return Row > ref.Row; }
	bool operator < (const G42ImageStripTransIterator & ref)
		{ return Row < ref.Row; }
	bool operator >= (const G42ImageStripTransIterator & ref)
		{ return Row >= ref.Row; }
	bool operator <= (const G42ImageStripTransIterator & ref)
		{ return Row <= ref.Row; }
protected:
	LockType * Ptr;
	uint32 Row;
};
// these are the same as above except for operator * and operator []
// but I can't get these to derive from above
template <class RowType, class LockType>
class G42ImageStripTransByteIterator
{
public:
	typedef RowType G42Row;
	G42ImageStripTransByteIterator(void) : Ptr(0), Row(0) {}
	G42ImageStripTransByteIterator(LockType & ptr, int index = 0) :
		Ptr(&ptr), Row(index) {}
	G42ImageStripTransByteIterator(const G42ImageStripTransByteIterator & ptr) :
		Ptr(ptr.Ptr), Row(ptr.Row) {}
	G42ImageStripTransByteIterator & operator = (const G42ImageStripTransByteIterator & ptr)
		{ Ptr = ptr.Ptr; Row = ptr.Row; return *this; }
	~G42ImageStripTransByteIterator(void) {}
	bool operator == (const G42ImageStripTransByteIterator & ptr)
		{ return (Row == ptr.Row); }
	bool operator != (const G42ImageStripTransByteIterator & ptr)
		{ return (Row != ptr.Row); }
	G42Row operator * (void)
		{ return G42Row(
			(Ptr->GetData().IsTransparent() ? Ptr->GetRow(Row, true) : 0),
			((Ptr->GetData().GetWidth() + 7) >> 3)); }
	G42ImageStripTransByteIterator operator ++ (int)
		{ return G42ImageStripTransByteIterator<RowType, LockType>(*Ptr, Row++); }
	G42ImageStripTransByteIterator & operator ++ (void)
		{ Row++; return *this; }
	G42ImageStripTransByteIterator operator -- (int)
		{ return G42ImageStripTransByteIterator<RowType, LockType>(*Ptr, Row--); }
	G42ImageStripTransByteIterator & operator -- (void)
		{ Row--; return *this; }
	G42ImageStripTransByteIterator & operator += (int n)
		{ Row += n; return *this; }
	G42ImageStripTransByteIterator & operator -= (int n)
		{ Row -= n; return *this; }
	G42ImageStripTransByteIterator operator + (int n)
		{ return G42ImageStripTransByteIterator<RowType, LockType>(*Ptr, Row + n); }
	G42ImageStripTransByteIterator operator - (int n)
		{ return G42ImageStripTransByteIterator<RowType, LockType>(*Ptr, Row - n); }
	int operator - (const G42ImageStripTransByteIterator & ref)
		{ return Row - ref.Row; }
	G42Row operator [] (int index)
		{ return G42Row(
			(Ptr->GetData().IsTransparent() ? Ptr->GetRow(index, true) : 0),
			((Ptr->GetData().GetWidth() + 7) >> 3)); }
	bool operator > (const G42ImageStripTransByteIterator & ref)
		{ return Row > ref.Row; }
	bool operator < (const G42ImageStripTransByteIterator & ref)
		{ return Row < ref.Row; }
	bool operator >= (const G42ImageStripTransByteIterator & ref)
		{ return Row >= ref.Row; }
	bool operator <= (const G42ImageStripTransByteIterator & ref)
		{ return Row <= ref.Row; }
protected:
	LockType * Ptr;
	uint32 Row;
};
#if 0
template <class RowType, class LockType>
class G42ImageStripByte1Iterator :
	public G42ImageStripIterator<RowType, LockType>
{
public:
	G42ImageStripByte1Iterator(void) :
		G42ImageStripIterator<RowType, LockType> () {}
	G42ImageStripByte1Iterator(LockType & ptr, int index = 0) :
		G42ImageStripIterator<RowType, LockType> (ptr, index) {}
	G42ImageStripByte1Iterator(const G42ImageStripByte1Iterator & ptr) :
		G42ImageStripIterator<RowType, LockType> (ptr) {}
	G42Row operator * (void)
		{ return G42Row(Ptr->GetRow(Row), ((Ptr->GetData().GetWidth() + 7) >> 3)); }
	G42Row operator [] (int index)
		{ return G42Row(Ptr->GetRow(index),
			((Ptr->GetData().GetWidth() + 7) >> 3)); }
};
template <class RowType, class LockType>
class G42ImageStripByte24Iterator :
	public G42ImageStripIterator<RowType, LockType>
{
public:
	G42ImageStripByte24Iterator(void) :
		G42ImageStripIterator<RowType, LockType> () {}
	G42ImageStripByte24Iterator(LockType & ptr, int index = 0) :
		G42ImageStripIterator<RowType, LockType> (ptr, index) {}
	G42ImageStripByte24Iterator(const G42ImageStripByte24Iterator & ptr) :
		G42ImageStripIterator<RowType, LockType> (ptr) {}
	G42Row operator * (void)
		{ return G42Row(Ptr->GetRow(Row), Ptr->GetData().GetWidth() +
			(Ptr->GetData().GetWidth() << 1)); }
	G42Row operator [] (int index)
		{ return G42Row(Ptr->GetRow(index),
			Ptr->GetData().GetWidth() +
			(Ptr->GetData().GetWidth() << 1)); }
};
template <class RowType, class LockType>
class G42ImageStripByte32Iterator :
	public G42ImageStripIterator<RowType, LockType>
{
public:
	G42ImageStripByte32Iterator(void) :
		G42ImageStripIterator<RowType, LockType> () {}
	G42ImageStripByte32Iterator(LockType & ptr, int index = 0) :
		G42ImageStripIterator<RowType, LockType> (ptr, index) {}
	G42ImageStripByte32Iterator(const G42ImageStripByte32Iterator & ptr) :
		G42ImageStripIterator<RowType, LockType> (ptr) {}
	G42Row operator * (void)
		{ return G42Row(Ptr->GetRow(Row), (Ptr->GetData().GetWidth() << 2)); }
	G42Row operator [] (int index)
		{ return G42Row(Ptr->GetRow(index),
			(Ptr->GetData().GetWidth() << 2)); }
};
#endif
class G42ImageDataSafeBase : public G42ImageData
{
public:
	// constructors, destructor, operator =
	G42ImageDataSafeBase(G42HeapManager & heap, uint32 strip_num, uint32 num_rows,
		uint32 row_bytes, uint32 strip_start_in_image, uint32 width,
		uint32 trans_row_bytes = 0) // throw (G42InvalidParameterException)
		:
		G42ImageData(heap, strip_num, num_rows, row_bytes, strip_start_in_image,
			width, trans_row_bytes) {}
	virtual ~G42ImageDataSafeBase() {}
	G42ImageDataSafeBase(G42ImageDataSafeBase & ref) : G42ImageData(ref) {}
	typedef G42ImageStripTransIterator<G42ImageRow<G42SafeBitIterator, byte *>,
		G42WriteImageData> G42SafeTransIterator;
	typedef G42ImageStripTransIterator<G42ImageRow<G42ConstSafeBitIterator,
		const byte *>, G42ReadImageData> G42ConstSafeTransIterator;
	typedef G42ImageStripTransByteIterator<G42ImageRow<G42SafeByteIterator, byte *>,
		G42WriteImageData> G42SafeByteTransIterator;
	typedef G42ImageStripTransByteIterator<G42ImageRow<G42ConstSafeByteIterator,
		const byte *>, G42ReadImageData> G42ConstSafeByteTransIterator;
	G42SafeTransIterator GetBegin(G42WriteImageData & ptr)
		{ return G42SafeTransIterator(ptr); }
	G42SafeTransIterator GetEnd(G42WriteImageData & ptr)
		{ return G42SafeTransIterator(ptr, GetNumRows()); }
	G42SafeByteTransIterator GetByteBegin(G42WriteImageData & ptr)
		{ return G42SafeByteTransIterator(ptr); }
	G42SafeByteTransIterator GetByteEnd(G42WriteImageData & ptr)
		{ return G42SafeByteTransIterator(ptr, GetNumRows()); }
	G42SafeTransIterator GetReverseBegin(G42WriteImageData & ptr)
		{ return GetEnd(ptr); }
	G42SafeTransIterator GetReverseEnd(G42WriteImageData & ptr)
		{ return GetBegin(ptr); }
	G42SafeByteTransIterator GetReverseByteBegin(G42WriteImageData & ptr)
		{ return GetByteEnd(ptr); }
	G42SafeByteTransIterator GetReverseByteEnd(G42WriteImageData & ptr)
		{ return GetByteBegin(ptr); }
	G42ConstSafeTransIterator GetConstBegin(G42ReadImageData & ptr)
		{ return G42ConstSafeTransIterator(ptr); }
	G42ConstSafeTransIterator GetConstEnd(G42ReadImageData & ptr)
		{ return G42ConstSafeTransIterator(ptr, GetNumRows()); }
	G42ConstSafeByteTransIterator GetConstByteBegin(G42ReadImageData & ptr)
		{ return G42ConstSafeByteTransIterator(ptr); }
	G42ConstSafeByteTransIterator GetConstByteEnd(G42ReadImageData & ptr)
		{ return G42ConstSafeByteTransIterator(ptr, GetNumRows()); }
	G42ConstSafeTransIterator GetConstReverseBegin(G42ReadImageData & ptr)
		{ return GetConstEnd(ptr); }
	G42ConstSafeTransIterator GetConstReverseEnd(G42ReadImageData & ptr)
		{ return GetConstBegin(ptr); }
	G42ConstSafeByteTransIterator GetConstReverseByteBegin(G42ReadImageData & ptr)
		{ return GetConstByteEnd(ptr); }
	G42ConstSafeByteTransIterator GetConstReverseByteEnd(G42ReadImageData & ptr)
		{ return GetConstByteBegin(ptr); }
};
/*-----------------------------------------------------------------------------
	class G42ImageData
-----------------------------------------------------------------------------*/
class G42ImageDataBase : public G42ImageDataSafeBase
{
public:
	// constructors, destructor, operator =
	G42ImageDataBase(G42HeapManager & heap, uint32 strip_num, uint32 num_rows,
		uint32 row_bytes, uint32 strip_start_in_image, uint32 width,
		uint32 trans_row_bytes = 0) // throw (G42InvalidParameterException)
		:
		G42ImageDataSafeBase(heap, strip_num, num_rows, row_bytes,
			strip_start_in_image, width, trans_row_bytes) {}
	virtual ~G42ImageDataBase() {}
	G42ImageDataBase(G42ImageDataBase & ref) : G42ImageDataSafeBase(ref) {}
/*	typedef G42ImageStripIterator<G42ImageRow<G42BitIterator, byte *>,
		G42WriteImageData> G42IteratorT;
	typedef G42ImageStripIterator<G42ImageRow<G42ConstBitIterator, const byte *>,
		G42ReadImageData> G42ConstIteratorT;
	typedef G42ImageStripByte1Iterator<G42ImageRow<G42ByteIterator, byte *>,
		G42WriteImageData> G42ByteIteratorT;
	typedef G42ImageStripByte1Iterator<G42ImageRow<G42ConstByteIterator,
		const byte *>, G42ReadImageData> G42ConstByteIteratorT;
*/
	typedef G42ImageStripTransIterator<G42ImageRow<G42BitIterator, byte *>,
		G42WriteImageData> G42TransIterator;
	typedef G42ImageStripTransIterator<G42ImageRow<G42ConstBitIterator,
		const byte *>, G42ReadImageData> G42ConstTransIterator;
	typedef G42ImageStripTransByteIterator<G42ImageRow<G42ByteIterator, byte *>,
		G42WriteImageData> G42ByteTransIterator;
	typedef G42ImageStripTransByteIterator<G42ImageRow<G42ConstByteIterator,
		const byte *>, G42ReadImageData> G42ConstByteTransIterator;
	G42TransIterator GetBegin(G42WriteImageData & ptr)
		{ return G42TransIterator(ptr); }
	G42TransIterator GetEnd(G42WriteImageData & ptr)
		{ return G42TransIterator(ptr, GetNumRows()); }
	G42ByteTransIterator GetByteBegin(G42WriteImageData & ptr)
		{ return G42ByteTransIterator(ptr); }
	G42ByteTransIterator GetByteEnd(G42WriteImageData & ptr)
		{ return G42ByteTransIterator(ptr, GetNumRows()); }
	G42TransIterator GetReverseBegin(G42WriteImageData & ptr)
		{ return GetEnd(ptr); }
	G42TransIterator GetReverseEnd(G42WriteImageData & ptr)
		{ return GetBegin(ptr); }
	G42ByteTransIterator GetReverseByteBegin(G42WriteImageData & ptr)
		{ return GetByteEnd(ptr); }
	G42ByteTransIterator GetReverseByteEnd(G42WriteImageData & ptr)
		{ return GetByteBegin(ptr); }
	G42ConstTransIterator GetConstBegin(G42ReadImageData & ptr)
		{ return G42ConstTransIterator(ptr); }
	G42ConstTransIterator GetConstEnd(G42ReadImageData & ptr)
		{ return G42ConstTransIterator(ptr, GetNumRows()); }
	G42ConstByteTransIterator GetConstByteBegin(G42ReadImageData & ptr)
		{ return G42ConstByteTransIterator(ptr); }
	G42ConstByteTransIterator GetConstByteEnd(G42ReadImageData & ptr)
		{ return G42ConstByteTransIterator(ptr, GetNumRows()); }
	G42ConstTransIterator GetConstReverseBegin(G42ReadImageData & ptr)
		{ return GetConstEnd(ptr); }
	G42ConstTransIterator GetConstReverseEnd(G42ReadImageData & ptr)
		{ return GetConstBegin(ptr); }
	G42ConstByteTransIterator GetConstReverseByteBegin(G42ReadImageData & ptr)
		{ return GetConstByteEnd(ptr); }
	G42ConstByteTransIterator GetConstReverseByteEnd(G42ReadImageData & ptr)
		{ return GetConstByteBegin(ptr); }
#if 0 // old way with Trans in iterators and Gets
	typedef G42ImageStripTransIterator<G42ImageRow<G42BitIterator, byte *>,
		G42WriteImageData> G42TransIterator;
	typedef G42ImageStripTransIterator<G42ImageRow<G42ConstBitIterator,
		const byte *>, G42ReadImageData> G42ConstTransIterator;
	typedef G42ImageStripTransByteIterator<G42ImageRow<G42ByteIterator, byte *>,
		G42WriteImageData> G42ByteTransIterator;
	typedef G42ImageStripTransByteIterator<G42ImageRow<G42ConstByteIterator,
		const byte *>, G42ReadImageData> G42ConstByteTransIterator;
	G42TransIterator GetTransBegin(G42WriteImageData & ptr)
		{ return G42TransIterator(ptr); }
	G42TransIterator GetTransEnd(G42WriteImageData & ptr)
		{ return G42TransIterator(ptr, GetNumRows()); }
	G42ByteTransIterator GetByteTransBegin(G42WriteImageData & ptr)
		{ return G42ByteTransIterator(ptr); }
	G42ByteTransIterator GetByteTransEnd(G42WriteImageData & ptr)
		{ return G42ByteTransIterator(ptr, GetNumRows()); }
	G42TransIterator GetReverseTransBegin(G42WriteImageData & ptr)
		{ return GetTransEnd(ptr); }
	G42TransIterator GetReverseTransEnd(G42WriteImageData & ptr)
		{ return GetTransBegin(ptr); }
	G42ByteTransIterator GetReverseByteTransBegin(G42WriteImageData & ptr)
		{ return GetByteTransEnd(ptr); }
	G42ByteTransIterator GetReverseByteTransEnd(G42WriteImageData & ptr)
		{ return GetByteTransBegin(ptr); }
	G42ConstTransIterator GetConstTransBegin(G42ReadImageData & ptr)
		{ return G42ConstTransIterator(ptr); }
	G42ConstTransIterator GetConstTransEnd(G42ReadImageData & ptr)
		{ return G42ConstTransIterator(ptr, GetNumRows()); }
	G42ConstByteTransIterator GetConstByteTransBegin(G42ReadImageData & ptr)
		{ return G42ConstByteTransIterator(ptr); }
	G42ConstByteTransIterator GetConstByteTransEnd(G42ReadImageData & ptr)
		{ return G42ConstByteTransIterator(ptr, GetNumRows()); }
	G42ConstTransIterator GetConstReverseTransBegin(G42ReadImageData & ptr)
		{ return GetConstTransEnd(ptr); }
	G42ConstTransIterator GetConstReverseTransEnd(G42ReadImageData & ptr)
		{ return GetConstTransBegin(ptr); }
	G42ConstByteTransIterator GetConstReverseByteTransBegin(G42ReadImageData & ptr)
		{ return GetConstByteTransEnd(ptr); }
	G42ConstByteTransIterator GetConstReverseByteTransEnd(G42ReadImageData & ptr)
		{ return GetConstByteTransBegin(ptr); }
#endif
};
/*-----------------------------------------------------------------------------
	class G42ImageDataGray8 - derived class for iterators
-----------------------------------------------------------------------------*/
class G42ImageDataColor8;
class G42ImageDataGray8 : public G42ImageDataBase
{
public:
	// constructors, destructor, operator =
	G42ImageDataGray8(G42ImageDataGray8 & ref) : G42ImageDataBase(ref) {}
	G42ImageDataGray8(G42ImageDataColor8 & ref);
	G42ImageDataGray8(G42HeapManager & heap, uint32 strip_num, uint32 num_rows,
		uint32 row_bytes, uint32 strip_start_in_image, uint32 width,
		uint32 trans_row_bytes = 0) // throw (G42InvalidParameterException)
	:
		G42ImageDataBase(heap, strip_num, num_rows, row_bytes, strip_start_in_image,
			width, trans_row_bytes)
	{}
	G42ImageDataGray8 & operator = (G42ImageDataGray8 & ref)
	{
		G42ImageData::operator = (ref);
		return *this;
	}
	virtual ~G42ImageDataGray8() {}
	virtual G42ImageData * MakeCopy(void);
	typedef G42ImageStripIterator<G42ImageRow<G42ByteIterator, byte *>,
		G42WriteImageData> G42Iterator;
	typedef G42ImageStripIterator<G42ImageRow<G42ConstByteIterator, const byte *>,
		G42ReadImageData> G42ConstIterator;
	typedef G42Iterator G42ByteIterator; // byte G42Iterator is same in this case
	typedef G42ConstIterator G42ConstByteIterator; // const byte G42Iterator is same in this case
	G42Iterator GetBegin(G42WriteImageData & ptr)
		{ return G42Iterator(ptr); }
	G42Iterator GetEnd(G42WriteImageData & ptr)
		{ return G42Iterator(ptr, GetNumRows()); }
	G42ByteIterator GetByteBegin(G42WriteImageData & ptr)
		{ return G42ByteIterator(ptr); }
	G42ByteIterator GetByteEnd(G42WriteImageData & ptr)
		{ return G42ByteIterator(ptr, GetNumRows()); }
	G42Iterator GetReverseBegin(G42WriteImageData & ptr)
		{ return GetEnd(ptr); }
	G42Iterator GetReverseEnd(G42WriteImageData & ptr)
		{ return GetBegin(ptr); }
	G42ByteIterator GetReverseByteBegin(G42WriteImageData & ptr)
		{ return GetByteEnd(ptr); }
	G42ByteIterator GetReverseByteEnd(G42WriteImageData & ptr)
		{ return GetByteBegin(ptr); }
	G42ConstIterator GetConstBegin(G42ReadImageData & ptr)
		{ return G42ConstIterator(ptr); }
	G42ConstIterator GetConstEnd(G42ReadImageData & ptr)
		{ return G42ConstIterator(ptr, GetNumRows()); }
	G42ConstByteIterator GetConstByteBegin(G42ReadImageData & ptr)
		{ return G42ConstByteIterator(ptr); }
	G42ConstByteIterator GetConstByteEnd(G42ReadImageData & ptr)
		{ return G42ConstByteIterator(ptr, GetNumRows()); }
	G42ConstIterator GetConstReverseBegin(G42ReadImageData & ptr)
		{ return GetConstEnd(ptr); }
	G42ConstIterator GetConstReverseEnd(G42ReadImageData & ptr)
		{ return GetConstBegin(ptr); }
	G42ConstByteIterator GetConstReverseByteBegin(G42ReadImageData & ptr)
		{ return GetConstByteEnd(ptr); }
	G42ConstByteIterator GetConstReverseByteEnd(G42ReadImageData & ptr)
		{ return GetConstByteBegin(ptr); }
	// some function object converters
	class G42ConvertToGray8
	{
	public:
		byte operator () (byte pixel)
			{ return pixel; }
	};
	class G42ConvertToG42Color
	{
	public:
		G42Color operator () (byte pixel)
			{ return G42Color(pixel, pixel, pixel); }
	};
	class G42ConvertFromGray8
	{
	public:
		byte operator () (byte pixel)
			{ return pixel; }
	};
	
	class G42ConvertFromG42Color // assumes it's a gray
	{
	public:
		byte operator () (const G42Color & c)
			{ return c.Green; }
	};
	/*  Special null converter  */
	class G42ConvertNull
	{
	public:
		byte operator () (byte pixel)
			{ return pixel; }
	};
};
/*-----------------------------------------------------------------------------
	class G42ImageDataColor8 - derived class for iterators
-----------------------------------------------------------------------------*/
class G42ImageDataColor8 : public G42ImageDataBase
{
public:
	// constructors, destructor, operator =
	G42ImageDataColor8(G42ImageDataColor8 & ref) : G42ImageDataBase(ref) {}
	G42ImageDataColor8(G42ImageDataGray8 & ref) : G42ImageDataBase(ref) {}
	G42ImageDataColor8(G42HeapManager & heap, uint32 strip_num, uint32 num_rows,
		uint32 row_bytes, uint32 strip_start_in_image, uint32 width,
		uint32 trans_row_bytes = 0) // throw (G42InvalidParameterException)
	:
		G42ImageDataBase(heap, strip_num, num_rows, row_bytes, strip_start_in_image,
			width, trans_row_bytes)
	{}
	G42ImageDataColor8 & operator = (G42ImageDataColor8 & ref)
	{
		G42ImageData::operator = (ref);
		return *this;
	}
	virtual ~G42ImageDataColor8() {}
	virtual G42ImageData * MakeCopy(void);
	typedef G42ImageStripIterator<G42ImageRow<G42ByteIterator, byte *>,
		G42WriteImageData> G42Iterator;
	typedef G42ImageStripIterator<G42ImageRow<G42ConstByteIterator, const byte *>,
		G42ReadImageData> G42ConstIterator;
	typedef G42Iterator G42ByteIterator; // byte G42Iterator is same in this case
	typedef G42ConstIterator G42ConstByteIterator; // const byte G42Iterator is same in this case
	G42Iterator GetBegin(G42WriteImageData & ptr)
		{ return G42Iterator(ptr); }
	G42Iterator GetEnd(G42WriteImageData & ptr)
		{ return G42Iterator(ptr, GetNumRows()); }
	G42ByteIterator GetByteBegin(G42WriteImageData & ptr)
		{ return G42ByteIterator(ptr); }
	G42ByteIterator GetByteEnd(G42WriteImageData & ptr)
		{ return G42ByteIterator(ptr, GetNumRows()); }
	G42Iterator GetReverseBegin(G42WriteImageData & ptr)
		{ return GetEnd(ptr); }
	G42Iterator GetReverseEnd(G42WriteImageData & ptr)
		{ return GetBegin(ptr); }
	G42ByteIterator GetReverseByteBegin(G42WriteImageData & ptr)
		{ return GetByteEnd(ptr); }
	G42ByteIterator GetReverseByteEnd(G42WriteImageData & ptr)
		{ return GetByteBegin(ptr); }
	G42ConstIterator GetConstBegin(G42ReadImageData & ptr)
		{ return G42ConstIterator(ptr); }
	G42ConstIterator GetConstEnd(G42ReadImageData & ptr)
		{ return G42ConstIterator(ptr, GetNumRows()); }
	G42ConstByteIterator GetConstByteBegin(G42ReadImageData & ptr)
		{ return G42ConstByteIterator(ptr); }
	G42ConstByteIterator GetConstByteEnd(G42ReadImageData & ptr)
		{ return G42ConstByteIterator(ptr, GetNumRows()); }
	G42ConstIterator GetConstReverseBegin(G42ReadImageData & ptr)
		{ return GetConstEnd(ptr); }
	G42ConstIterator GetConstReverseEnd(G42ReadImageData & ptr)
		{ return GetConstBegin(ptr); }
	G42ConstByteIterator GetConstReverseByteBegin(G42ReadImageData & ptr)
		{ return GetConstByteEnd(ptr); }
	G42ConstByteIterator GetConstReverseByteEnd(G42ReadImageData & ptr)
		{ return GetConstByteBegin(ptr); }
	// some function object converters
	class G42ConvertToGray8
	{
	public:
		G42ConvertToGray8(const G42Color * palette, intf num_palette);
		byte operator () (byte pixel)
			{ return GrayPalette[pixel]; }
		~G42ConvertToGray8(void);
	private:
		const G42Color * ColorPalette;
		G42APtr<byte> GrayPalette;
	};
	class G42ConvertToG42Color
	{
	public:
		G42ConvertToG42Color(const G42Color * palette) : ColorPalette(palette) {}
		const G42Color & operator () (byte pixel)
			{ return ColorPalette[(byte)pixel]; }
		~G42ConvertToG42Color(void) {}
	private:
		const G42Color * ColorPalette;
	};
	/*  Special null converter  */
	class G42ConvertNull
	{
	public:
		byte operator () (byte pixel)
			{ return pixel; }
	};
};
inline
G42ImageDataGray8::G42ImageDataGray8(G42ImageDataColor8 & ref)
:
	G42ImageDataBase(ref)
{
}
/*-----------------------------------------------------------------------------
	class G42ImageDataGray1 - derived class for iterators
-----------------------------------------------------------------------------*/
class G42ImageDataGray1 : public G42ImageDataBase
{
public:
	// constructors, destructor, operator =
	G42ImageDataGray1(G42ImageDataGray1 & ref) : G42ImageDataBase(ref) {}
	G42ImageDataGray1(G42HeapManager & heap, uint32 strip_num, uint32 num_rows,
		uint32 row_bytes, uint32 strip_start_in_image, uint32 width,
		uint32 trans_row_bytes = 0) // throw (G42InvalidParameterException)
	:
		G42ImageDataBase(heap, strip_num, num_rows, row_bytes, strip_start_in_image,
			width, trans_row_bytes)
	{}
	G42ImageDataGray1 & operator = (G42ImageDataGray1 & ref)
	{
		G42ImageData::operator = (ref);
		return *this;
	}
	virtual ~G42ImageDataGray1() {}
	virtual G42ImageData * MakeCopy(void);
	typedef G42ImageStripIterator<G42ImageRow<G42BitIterator, byte *>,
		G42WriteImageData> G42Iterator;
	typedef G42ImageStripIterator<G42ImageRow<G42ConstBitIterator, const byte *>,
		G42ReadImageData> G42ConstIterator;
	typedef G42ImageStripByte1Iterator<G42ImageRow<G42ByteIterator, byte *>,
		G42WriteImageData> G42ByteIterator;
	typedef G42ImageStripByte1Iterator<G42ImageRow<G42ConstByteIterator,
		const byte *>, G42ReadImageData> G42ConstByteIterator;
	G42Iterator GetBegin(G42WriteImageData & ptr)
		{ return G42Iterator(ptr); }
	G42Iterator GetEnd(G42WriteImageData & ptr)
		{ return G42Iterator(ptr, GetNumRows()); }
	G42ByteIterator GetByteBegin(G42WriteImageData & ptr)
		{ return G42ByteIterator(ptr); }
	G42ByteIterator GetByteEnd(G42WriteImageData & ptr)
		{ return G42ByteIterator(ptr, GetNumRows()); }
	G42Iterator GetReverseBegin(G42WriteImageData & ptr)
		{ return GetEnd(ptr); }
	G42Iterator GetReverseEnd(G42WriteImageData & ptr)
		{ return GetBegin(ptr); }
	G42ByteIterator GetReverseByteBegin(G42WriteImageData & ptr)
		{ return GetByteEnd(ptr); }
	G42ByteIterator GetReverseByteEnd(G42WriteImageData & ptr)
		{ return GetByteBegin(ptr); }
	G42ConstIterator GetConstBegin(G42ReadImageData & ptr)
		{ return G42ConstIterator(ptr); }
	G42ConstIterator GetConstEnd(G42ReadImageData & ptr)
		{ return G42ConstIterator(ptr, GetNumRows()); }
	G42ConstByteIterator GetConstByteBegin(G42ReadImageData & ptr)
		{ return G42ConstByteIterator(ptr); }
	G42ConstByteIterator GetConstByteEnd(G42ReadImageData & ptr)
		{ return G42ConstByteIterator(ptr, GetNumRows()); }
	G42ConstIterator GetConstReverseBegin(G42ReadImageData & ptr)
		{ return GetConstEnd(ptr); }
	G42ConstIterator GetConstReverseEnd(G42ReadImageData & ptr)
		{ return GetConstBegin(ptr); }
	G42ConstByteIterator GetConstReverseByteBegin(G42ReadImageData & ptr)
		{ return GetConstByteEnd(ptr); }
	G42ConstByteIterator GetConstReverseByteEnd(G42ReadImageData & ptr)
		{ return GetConstByteBegin(ptr); }
	// some function object converters
	class G42ConvertToGray8
	{
	public:
		byte operator () (bool pixel)
			{ return (byte)(pixel ? 0xff : 0); }
	};
	class G42ConvertToG42Color
	{
	public:
		G42Color operator () (bool pixel)
			{ if (pixel) return G42Color(0xff, 0xff, 0xff);
				return G42Color(0, 0, 0); }
	};
	class G42ConvertFromGray8
	{
	public:
		bool operator () (byte pixel)
			{ return (bool)(pixel & 0x80); }
	};
	class G42ConvertFromG42Color // assumes it's black or white
	{
	public:
		bool operator () (const G42Color c)
			{ return (bool)(c.Green & 0x80); }
	};
	
	/*  Special null converter  */
	class G42ConvertNull
	{
	public:
		bool operator () (bool pixel)
			{ return pixel; }
	};
	
};
/*-----------------------------------------------------------------------------
	class G42ImageDataColor24 - derived class for iterators
-----------------------------------------------------------------------------*/
class G42ImageDataColor24 : public G42ImageDataBase
{
public:
	// constructors, destructor, operator =
	G42ImageDataColor24(G42ImageDataColor24 & ref) : G42ImageDataBase(ref) {}
	G42ImageDataColor24(G42HeapManager & heap, uint32 strip_num, uint32 num_rows,
		uint32 row_bytes, uint32 strip_start_in_image, uint32 width,
		uint32 trans_row_bytes = 0) // throw (G42InvalidParameterException)
	:
		G42ImageDataBase(heap, strip_num, num_rows, row_bytes, strip_start_in_image,
			width, trans_row_bytes)
	{}
	G42ImageDataColor24 & operator = (G42ImageDataColor24 & ref)
	{
		G42ImageData::operator = (ref);
		return *this;
	}
	virtual ~G42ImageDataColor24() {}
	virtual G42ImageData * MakeCopy(void);
	typedef G42ImageStripIterator<G42ImageRow<G42Color24Iterator, byte *>,
		G42WriteImageData> G42Iterator;
	typedef G42ImageStripIterator<G42ImageRow<G42ConstColor24Iterator,
		const byte *>, G42ReadImageData> G42ConstIterator;
	typedef G42ImageStripByte24Iterator<G42ImageRow<G42ByteIterator, byte *>,
		G42WriteImageData> G42ByteIterator;
	typedef G42ImageStripByte24Iterator<G42ImageRow<G42ConstByteIterator,
		const byte *>, G42ReadImageData> G42ConstByteIterator;
	G42Iterator GetBegin(G42WriteImageData & ptr)
		{ return G42Iterator(ptr); }
	G42Iterator GetEnd(G42WriteImageData & ptr)
		{ return G42Iterator(ptr, GetNumRows()); }
	G42ByteIterator GetByteBegin(G42WriteImageData & ptr)
		{ return G42ByteIterator(ptr); }
	G42ByteIterator GetByteEnd(G42WriteImageData & ptr)
		{ return G42ByteIterator(ptr, GetNumRows()); }
	G42Iterator GetReverseBegin(G42WriteImageData & ptr)
		{ return GetEnd(ptr); }
	G42Iterator GetReverseEnd(G42WriteImageData & ptr)
		{ return GetBegin(ptr); }
	G42ByteIterator GetReverseByteBegin(G42WriteImageData & ptr)
		{ return GetByteEnd(ptr); }
	G42ByteIterator GetReverseByteEnd(G42WriteImageData & ptr)
		{ return GetByteBegin(ptr); }
	G42ConstIterator GetConstBegin(G42ReadImageData & ptr)
		{ return G42ConstIterator(ptr); }
	G42ConstIterator GetConstEnd(G42ReadImageData & ptr)
		{ return G42ConstIterator(ptr, GetNumRows()); }
	G42ConstByteIterator GetConstByteBegin(G42ReadImageData & ptr)
		{ return G42ConstByteIterator(ptr); }
	G42ConstByteIterator GetConstByteEnd(G42ReadImageData & ptr)
		{ return G42ConstByteIterator(ptr, GetNumRows()); }
	G42ConstIterator GetConstReverseBegin(G42ReadImageData & ptr)
		{ return GetConstEnd(ptr); }
	G42ConstIterator GetConstReverseEnd(G42ReadImageData & ptr)
		{ return GetConstBegin(ptr); }
	G42ConstByteIterator GetConstReverseByteBegin(G42ReadImageData & ptr)
		{ return GetConstByteEnd(ptr); }
	G42ConstByteIterator GetConstReverseByteEnd(G42ReadImageData & ptr)
		{ return GetConstByteBegin(ptr); }
	// some function object converters
	class G42ConvertToGray8
	{
	public:
		byte operator () (const G42Color24 & c)
			{ return (byte)((
				(uint32)c.GetRed() * (uint32)306 +
				(uint32)c.GetGreen() * (uint32)601 +
				(uint32)c.GetBlue() * (uint32)117 +
				(uint32)512) >> 10); }
		byte operator () (const G42ConstColor24 & c)
			{ return (byte)((
				(uint32)c.GetRed() * (uint32)306 +
				(uint32)c.GetGreen() * (uint32)601 +
				(uint32)c.GetBlue() * (uint32)117 +
				(uint32)512) >> 10); }
	};
	class G42ConvertToG42Color
	{
	public:
		G42Color operator () (const G42Color24 & c)
			{ return c; }
		G42Color operator () (const G42ConstColor24 & c)
			{ return c; }
	};
	class G42ConvertFromGray8
	{
	public:
		G42Color operator () (byte pixel)
			{ return G42Color(pixel, pixel, pixel); }
	};
	
	class G42ConvertFromG42Color // assumes it's a gray
	{
	public:
		G42Color operator () (const G42Color & c)
			{ return c; }
	};
	/*  Special null converter  */
	class G42ConvertNull
	{
	public:
		G42Color24 operator () (const G42Color24 & pixel)
			{ return (G42Color24 &) pixel; }
	};
	
};
/*-----------------------------------------------------------------------------
	class G42ImageDataColor32 - derived class for iterators
-----------------------------------------------------------------------------*/
class G42ImageDataColor32 : public G42ImageDataBase
{
public:
	// constructors, destructor, operator =
	G42ImageDataColor32(G42ImageDataColor32 & ref) : G42ImageDataBase(ref) {}
	G42ImageDataColor32(G42HeapManager & heap, uint32 strip_num, uint32 num_rows,
		uint32 row_bytes, uint32 strip_start_in_image, uint32 width,
		uint32 trans_row_bytes = 0) // throw (G42InvalidParameterException)
	:
		G42ImageDataBase(heap, strip_num, num_rows, row_bytes, strip_start_in_image,
			width, trans_row_bytes)
	{}
	G42ImageDataColor32 & operator = (G42ImageDataColor32 & ref)
	{
		G42ImageData::operator = (ref);
		return *this;
	}
	virtual ~G42ImageDataColor32() {}
	virtual G42ImageData * MakeCopy(void);
	typedef G42ImageStripIterator<G42ImageRow<G42Color32Iterator, byte *>,
		G42WriteImageData> G42Iterator;
	typedef G42ImageStripIterator<G42ImageRow<G42ConstColor32Iterator,
		const byte *>, G42ReadImageData> G42ConstIterator;
	typedef G42ImageStripByte32Iterator<G42ImageRow<G42SkipByteIterator, byte *>,
		G42WriteImageData> G42ByteIterator;
	typedef G42ImageStripByte32Iterator<G42ImageRow<G42ConstSkipByteIterator,
		const byte *>, G42ReadImageData> G42ConstByteIterator;
	G42Iterator GetBegin(G42WriteImageData & ptr)
		{ return G42Iterator(ptr); }
	G42Iterator GetEnd(G42WriteImageData & ptr)
		{ return G42Iterator(ptr, GetNumRows()); }
	G42ByteIterator GetByteBegin(G42WriteImageData & ptr)
		{ return G42ByteIterator(ptr); }
	G42ByteIterator GetByteEnd(G42WriteImageData & ptr)
		{ return G42ByteIterator(ptr, GetNumRows()); }
	G42Iterator GetReverseBegin(G42WriteImageData & ptr)
		{ return GetEnd(ptr); }
	G42Iterator GetReverseEnd(G42WriteImageData & ptr)
		{ return GetBegin(ptr); }
	G42ByteIterator GetReverseByteBegin(G42WriteImageData & ptr)
		{ return GetByteEnd(ptr); }
	G42ByteIterator GetReverseByteEnd(G42WriteImageData & ptr)
		{ return GetByteBegin(ptr); }
	G42ConstIterator GetConstBegin(G42ReadImageData & ptr)
		{ return G42ConstIterator(ptr); }
	G42ConstIterator GetConstEnd(G42ReadImageData & ptr)
		{ return G42ConstIterator(ptr, GetNumRows()); }
	G42ConstByteIterator GetConstByteBegin(G42ReadImageData & ptr)
		{ return G42ConstByteIterator(ptr); }
	G42ConstByteIterator GetConstByteEnd(G42ReadImageData & ptr)
		{ return G42ConstByteIterator(ptr, GetNumRows()); }
	G42ConstIterator GetConstReverseBegin(G42ReadImageData & ptr)
		{ return GetConstEnd(ptr); }
	G42ConstIterator GetConstReverseEnd(G42ReadImageData & ptr)
		{ return GetConstBegin(ptr); }
	G42ConstByteIterator GetConstReverseByteBegin(G42ReadImageData & ptr)
		{ return GetConstByteEnd(ptr); }
	G42ConstByteIterator GetConstReverseByteEnd(G42ReadImageData & ptr)
		{ return GetConstByteBegin(ptr); }
	// some function object converters
	class G42ConvertToGray8
	{
	public:
		byte operator () (const G42Color32 & c)
			{ return (byte)((
				(uint32)c.GetRed() * (uint32)306 +
				(uint32)c.GetGreen() * (uint32)601 +
				(uint32)c.GetBlue() * (uint32)117 +
				(uint32)512) >> 10); }
		byte operator () (const G42ConstColor32 & c)
			{ return (byte)((
				(uint32)c.GetRed() * (uint32)306 +
				(uint32)c.GetGreen() * (uint32)601 +
				(uint32)c.GetBlue() * (uint32)117 +
				(uint32)512) >> 10); }
	};
	class G42ConvertToG42Color
	{
	public:
		G42Color operator () (const G42Color32 & c)
			{ return c; }
		G42Color operator () (const G42ConstColor32 & c)
			{ return c; }
	};
	class G42ConvertFromGray8
	{
	public:
		G42Color operator () (byte pixel)
			{ return G42Color(pixel, pixel, pixel); }
	};
	
	class G42ConvertFromG42Color // assumes it's a gray
	{
	public:
		G42Color operator () (const G42Color & c)
			{ return c; }
	};
	/*  Special null converter  */
	class G42ConvertNull
	{
	public:
		G42Color32 operator () (const G42Color32 & pixel)
			{ return (G42Color32 &) pixel; }
	};
	
};
// some simple functions to work with the above iterators
// convert gets and returns a pixel
// convert overrides operator ()
template <class Iterator, class Converter>
void G42ImageDataConvert(Iterator GetBegin, Iterator GetEnd, Converter convert)
{
	for (Iterator i = GetBegin; i != GetEnd; i++)
	{
		Iterator::G42Row row(*i);
		for (Iterator::G42Row::G42Iterator j = row.GetBegin();
			j != row.GetEnd(); j++)
		{
			*j = convert(*j);
		}
	}
}
// convert gets an input pixel and returns an output pixel
// convert overrides operator ()
template <class Iterator, class OutputIterator, class Converter>
OutputIterator
G42ImageDataConvertCopy(Iterator GetBegin, Iterator GetEnd,
	OutputIterator output, Converter convert)
{
	for (Iterator i = GetBegin; i != GetEnd; i++)
	{
		OutputIterator::G42Row output_row(*output);
		Iterator::G42Row row(*i);
		OutputIterator::G42Row::G42Iterator out_it(output_row.GetBegin());
		for (Iterator::G42Row::G42Iterator j = row.GetBegin();
			j != row.GetEnd(); j++)
		{
			*out_it = convert(*j);
			out_it++;
		}
		output++;
	}
	return output;
}
// convert gets and returns a pixel
// convert overrides operator () and operator ++ (int)
template <class Iterator, class RowConverter>
void G42ImageDataRowConvert(Iterator GetBegin, Iterator GetEnd,
	RowConverter convert)
{
	for (Iterator i = GetBegin; i != GetEnd; i++)
	{
		Iterator::G42Row row(*i);
		for (Iterator::G42Row::G42Iterator j = row.GetBegin();
			j != row.GetEnd(); j++)
		{
			*j = convert(*j);
		}
		convert++;
	}
}
// convert gets an input pixel and returns an output pixel
// convert overrides operator () and operator ++ (int)
template <class Iterator, class OutputIterator, class RowConverter>
OutputIterator
G42ImageDataRowConvertCopy(Iterator GetBegin, Iterator GetEnd,
	OutputIterator output, RowConverter convert)
{
	for (Iterator i = GetBegin; i != GetEnd; i++)
	{
		OutputIterator::G42Row output_row(*output);
		Iterator::G42Row row(*i);
		OutputIterator::G42Row::G42Iterator out_it(output_row.GetBegin());
		for (Iterator::G42Row::G42Iterator j = row.GetBegin();
			j != row.GetEnd(); j++)
		{
			*out_it = convert(*j);
			out_it++;
		}
		output++;
		convert++;
	}
	return output;
}
#endif // GIDATA_H

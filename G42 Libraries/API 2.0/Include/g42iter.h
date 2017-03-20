#if !defined(G42ITER_H)
#define G42ITER_H
#include "g42idata.h"
/* Created by GES: these should be the only thing that
	needs changed between various platforms.  I put the
   SetFiller in an inline, because MSWIN doesn't need it.
*/
#ifdef MSWIN
const int TrueColorPixelSize = 3;
const int TrueColorRedOffset = 2;
const int TrueColorGreenOffset = 1;
const int TrueColorBlueOffset = 0;
inline void TrueColorSetFiller(byte * ptr)
	{}
#endif
#ifdef XWIN
const int TrueColorPixelSize = 4;
const int TrueColorRedOffset = 0;
const int TrueColorGreenOffset = 1;
const int TrueColorBlueOffset = 2;
inline void TrueColorSetFiller(byte * ptr)
	{*(ptr + 3) = 0xff;}
#endif
#ifdef MACOS
const int TrueColorPixelSize = 4;
const int TrueColorRedOffset = 1;
const int TrueColorGreenOffset = 2;
const int TrueColorBlueOffset = 3;
inline void TrueColorSetFiller(byte * ptr)
	{*ptr = 0;}
#endif
class G42ImageData24BitIterator
{
protected:
	G42ImageData *		Data;
	G42ModifiableImageTile *	Tile;
	G42ImageRow *		Row;
	unsigned char *	CurrentPtr;
   unsigned char *	MaxPtr;
	int				  	CurrentRow;
	int				  	CurrentColumn;
	int					CurrentTileRow;
	int					Offset;
   int					MaxRowInTile;
	int					MinOffset;
	int					MaxOffset;
	int					MinColumn;
	int					MinRow;
	int					MaxColumn;
	int					MaxRow;
	int					MinTileRow;
   int					MaxTileRow;
public:
	G42ImageData24BitIterator(G42ImageData * data, bool init = true);
	virtual ~G42ImageData24BitIterator(void);
	void Reset(void);
	int GetCellSize(void) const
		{return TrueColorPixelSize;}
	int GetBlueOffset(void) const
		{return TrueColorBlueOffset;}
	int GetGreenOffset(void) const
		{return TrueColorGreenOffset;}
	int GetRedOffset(void) const
		{return TrueColorRedOffset;}
	unsigned char GetBlue(void)
		{return *(CurrentPtr + TrueColorBlueOffset);}
	unsigned char GetGreen(void)
		{return *(CurrentPtr + TrueColorGreenOffset);}
	unsigned char GetRed(void)
		{return *(CurrentPtr + TrueColorRedOffset);}
	int32 GetBlue32(void)
		{return (int32)(*(CurrentPtr + TrueColorBlueOffset));}
	int32 GetGreen32(void)
		{return (int32)(*(CurrentPtr + TrueColorGreenOffset));}
	int32 GetRed32(void)
		{return (int32)(*(CurrentPtr + TrueColorRedOffset));}
	void SetArea(int left, int top, int right, int bottom);
	void Set(unsigned char blue, unsigned char green, unsigned char red)
		{*(CurrentPtr + TrueColorBlueOffset) = blue;
		*(CurrentPtr + TrueColorGreenOffset) = green;
		*(CurrentPtr + TrueColorRedOffset) = red;
		TrueColorSetFiller(CurrentPtr);}
	bool Increment(void);
	bool IncRow(void);
	bool Decrement(void);
	bool DecRow(void);
	bool NextRow(void);
	void ResetRow(void);
	void SetRowEnd(void);
	void SetLastRow(void);
   void ToTopRow(void);
	unsigned char * GetPtr(void)
		{return CurrentPtr;}
};
class G42LockedID24BitIterator
{
protected:
	G42ImageData *		Data;
	G42LockedImageTile *	Tile;
	G42ImageRow *		Row;
	unsigned char *	CurrentPtr;
	unsigned char *	MaxPtr;
	int				  	CurrentRow;
	int				  	CurrentColumn;
	int					CurrentTileRow;
	int					Offset;
   int					MaxRowInTile;
	int					MinOffset;
	int					MaxOffset;
	int					MinColumn;
	int					MinRow;
	int					MaxColumn;
	int					MaxRow;
	int					MinTileRow;
   int					MaxTileRow;
public:
	G42LockedID24BitIterator(G42ImageData * data, bool init = true);
	~G42LockedID24BitIterator(void);
	void Reset(void);
	int GetCellSize(void) const
		{return TrueColorPixelSize;}
	int GetBlueOffset(void) const
		{return TrueColorBlueOffset;}
	int GetGreenOffset(void) const
		{return TrueColorGreenOffset;}
	int GetRedOffset(void) const
		{return TrueColorRedOffset;}
	unsigned char GetBlue(void)
		{return *(CurrentPtr + TrueColorBlueOffset);}
	unsigned char GetGreen(void)
		{return *(CurrentPtr + TrueColorGreenOffset);}
	unsigned char GetRed(void)
		{return *(CurrentPtr + TrueColorRedOffset);}
	int32 GetBlue32(void)
		{return (int32)(*(CurrentPtr + TrueColorBlueOffset));}
	int32 GetGreen32(void)
		{return (int32)(*(CurrentPtr + TrueColorGreenOffset));}
	int32 GetRed32(void)
		{return (int32)(*(CurrentPtr + TrueColorRedOffset));}
	void SetArea(int left, int top, int right, int bottom);
	void Set(unsigned char blue, unsigned char green, unsigned char red)
		{*(CurrentPtr + TrueColorBlueOffset) = blue;
		*(CurrentPtr + TrueColorGreenOffset) = green;
		*(CurrentPtr + TrueColorRedOffset) = red;
		TrueColorSetFiller(CurrentPtr);}
	bool Increment(void);
	bool IncRow(void);
	bool Decrement(void);
	bool DecRow(void);
	bool NextRow(void);
	void ResetRow(void);
	void SetLastRow(void);
	unsigned char * GetPtr(void)
		{return CurrentPtr;}
};
class G42ImageData8BitIterator
{
protected:
	G42ImageData *		Data;
	G42ModifiableImageTile *		Tile;
	G42ImageRow *		Row;
	unsigned char *	CurrentPtr;
   unsigned char *	MaxPtr;
	int				  	CurrentRow;
	int				  	CurrentColumn;
	int					CurrentTileRow;
	int					Offset;
   int					MaxRowInTile;
	int					MinOffset;
	int					MaxOffset;
	int					MinColumn;
	int					MinRow;
	int					MaxColumn;
	int					MaxRow;
	int					MinTileRow;
   int					MaxTileRow;
public:
	G42ImageData8BitIterator(G42ImageData * data, bool init = true);
	~G42ImageData8BitIterator(void);
	void Reset(void);
	void Set(unsigned char val)
		{*CurrentPtr = val;}
	bool Increment(void);
	bool IncRow(void);
	bool Decrement(void);
	bool DecRow(void);
	bool NextRow(void);
	void ResetRow(void);
   void SetRowEnd(void);
	void SetLastRow(void);
   void ToTopRow(void);
	void SetArea(int left, int top, int right, int bottom);
	unsigned char * GetPtr(void)
		{return CurrentPtr;}
	operator unsigned char * (void) const
		{return CurrentPtr;}
	operator unsigned char (void) const
   	{return *CurrentPtr;}
};
class G42LockedID8BitIterator
{
protected:
	G42ImageData *		Data;
	G42LockedImageTile *		Tile;
	G42ImageRow *		Row;
	unsigned char *	CurrentPtr;
   unsigned char *	MaxPtr;
	int				  	CurrentRow;
	int				  	CurrentColumn;
	int					CurrentTileRow;
	int					Offset;
   int					MaxRowInTile;
	int					MinOffset;
	int					MaxOffset;
	int					MinColumn;
	int					MinRow;
	int					MaxColumn;
	int					MaxRow;
	int					MinTileRow;
   int					MaxTileRow;
public:
	G42LockedID8BitIterator(G42ImageData * data, bool init = true);
	~G42LockedID8BitIterator(void);
	void Reset(void);
	void Set(unsigned char val)
		{*CurrentPtr = val;}
	bool Increment(void);
	bool IncRow(void);
	bool Decrement(void);
	bool DecRow(void);
	bool NextRow(void);
	void ResetRow(void);
   void SetRowEnd(void);
	void SetLastRow(void);
	void SetArea(int left, int top, int right, int bottom);
	unsigned char * GetPtr(void)
		{return CurrentPtr;}
	operator unsigned char * (void) const
		{return CurrentPtr;}
	operator unsigned char (void) const
   	{return *CurrentPtr;}
};
class G42ImageData4BitIterator
{
protected:
	G42ImageData *		Data;
	G42ModifiableImageTile *		Tile;
	G42ImageRow *		Row;
	unsigned char *	CurrentPtr;
   unsigned char *	MaxPtr;
	int				  	CurrentRow;
	int				  	CurrentColumn;
	int					CurrentTileRow;
	int					Offset;
   bool					HiNibble;
   bool					MaxHiNibble;
   bool					MinHiNibble;
   int					MaxRowInTile;
	int					MinOffset;
	int					MaxOffset;
	int					MinColumn;
	int					MinRow;
	int					MaxColumn;
	int					MaxRow;
	int					MinTileRow;
   int					MaxTileRow;
public:
	G42ImageData4BitIterator(G42ImageData * data, bool init = true);
	~G42ImageData4BitIterator(void);
	void Reset(void);
	void Set(unsigned char val)
		{*CurrentPtr = val;}
	void Set4(unsigned char val);
	void SetArea(int left, int top, int right, int bottom);
	bool Increment(void);
	bool IncRow(void);
	bool Decrement(void);
	bool DecRow(void);
	bool NextRow(void);
	void ResetRow(void);
   void SetRowEnd(void);
	void SetLastRow(void);
   void ToTopRow(void);
	unsigned char * GetPtr(void)
		{return CurrentPtr;}
	operator unsigned char * (void) const
		{return CurrentPtr;}
	operator unsigned char (void) const
		{if (HiNibble) return ((*CurrentPtr) >> 4); return ((*CurrentPtr) & 0x0f);}
};
class G42LockedID4BitIterator
{
private:
	G42ImageData *		Data;
	G42LockedImageTile *		Tile;
	G42ImageRow *		Row;
	unsigned char *	CurrentPtr;
   unsigned char *	MaxPtr;
	int				  	CurrentRow;
	int				  	CurrentColumn;
	int					CurrentTileRow;
	int					Offset;
   bool					HiNibble;
   bool					MaxHiNibble;
   bool					MinHiNibble;
   int					MaxRowInTile;
	int					MinOffset;
	int					MaxOffset;
	int					MinColumn;
	int					MinRow;
	int					MaxColumn;
	int					MaxRow;
	int					MinTileRow;
   int					MaxTileRow;
public:
	G42LockedID4BitIterator(G42ImageData * data, bool init = true);
	~G42LockedID4BitIterator(void);
	void Reset(void);
	void Set(unsigned char val)
		{*CurrentPtr = val;}
	void SetArea(int left, int top, int right, int bottom);
	bool Increment(void);
	bool IncRow(void);
	bool Decrement(void);
	bool DecRow(void);
	bool NextRow(void);
	void ResetRow(void);
	void SetLastRow(void);
	unsigned char * GetPtr(void)
		{return CurrentPtr;}
	operator unsigned char * (void) const
		{return CurrentPtr;}
	operator unsigned char (void) const
		{if (HiNibble) return ((*CurrentPtr) >> 4); return ((*CurrentPtr) & 0x0f);}
};
class G42ImageData1BitIterator
{
private:
	G42ImageData *		Data;
	G42ModifiableImageTile *		Tile;
	G42ImageRow *		Row;
	unsigned char *	CurrentPtr;
	unsigned char *	MaxPtr;
	int				  	CurrentRow;
	int				  	CurrentColumn;
	int					CurrentTileRow;
	int					Offset;
   unsigned char		Mask;
   unsigned char		MaxMask;
   unsigned char		MinMask;
   int					MaxRowInTile;
	int					MinOffset;
	int					MaxOffset;
	int					MinColumn;
	int					MinRow;
	int					MaxColumn;
	int					MaxRow;
	int					MinTileRow;
   int					MaxTileRow;
public:
	G42ImageData1BitIterator(G42ImageData * data, bool init = true);
	~G42ImageData1BitIterator(void);
	void Reset(void);
	void Set(unsigned char val)
		{*CurrentPtr = val;}
 	void Set8(int val);
	void SetArea(int left, int top, int right, int bottom);
	bool Increment(void);
	bool Increment8(void);
	bool IncRow(void);
	bool Decrement(void);
	bool DecRow(void);
	bool NextRow(void);
	void ResetRow(void);
   void SetRowEnd(void);
	void SetLastRow(void);
   void ToTopRow(void);
	unsigned char * GetPtr(void)
		{return CurrentPtr;}
	operator unsigned char * (void) const
		{return CurrentPtr;}
	operator unsigned char (void) const
		{return *CurrentPtr;}
};
class G42LockedID1BitIterator
{
private:
	G42ImageData *		Data;
	G42LockedImageTile *		Tile;
	G42ImageRow *		Row;
	unsigned char *	CurrentPtr;
	unsigned char *	MaxPtr;
	int				  	CurrentRow;
	int				  	CurrentColumn;
	int					CurrentTileRow;
	int					Offset;
   unsigned char		Mask;
   unsigned char		MaxMask;
   unsigned char		MinMask;
   int					MaxRowInTile;
	int					MinOffset;
	int					MaxOffset;
	int					MinColumn;
	int					MinRow;
	int					MaxColumn;
	int					MaxRow;
	int					MinTileRow;
   int					MaxTileRow;
public:
	G42LockedID1BitIterator(G42ImageData * data, bool init = true);
	~G42LockedID1BitIterator(void);
	void Reset(void);
	void Set(unsigned char val)
		{*CurrentPtr = val;}
	void SetArea(int left, int top, int right, int bottom);
	bool Increment(void);
	bool Increment8(void);
	bool IncRow(void);
	bool Decrement(void);
	bool DecRow(void);
	bool NextRow(void);
	void ResetRow(void);
	void SetLastRow(void);
	unsigned char * GetPtr(void)
		{return CurrentPtr;}
	operator unsigned char * (void) const
		{return CurrentPtr;}
	operator unsigned char (void) const
		{return ((*CurrentPtr & Mask) ? 255 : 0);}
};
class G42ImageRow24BitIterator
{
private:
	G42ImageRow *		Row;
	unsigned char *	CurrentPtr;
public:
	G42ImageRow24BitIterator(G42ImageRow * row) : Row(row), CurrentPtr(*Row)
		{}
	~G42ImageRow24BitIterator(void)
		{}
	unsigned char GetBlue(void)
		{return *(CurrentPtr + TrueColorBlueOffset);}
	unsigned char GetGreen(void)
		{return *(CurrentPtr + TrueColorGreenOffset);}
	unsigned char GetRed(void)
		{return *(CurrentPtr + TrueColorRedOffset);}
	int32 GetBlue32(void)
		{return (int32)(*(CurrentPtr + TrueColorBlueOffset));}
	int32 GetGreen32(void)
		{return (int32)(*(CurrentPtr + TrueColorGreenOffset));}
	int32 GetRed32(void)
		{return (int32)(*(CurrentPtr + TrueColorRedOffset));}
	void Set(unsigned char blue, unsigned char green, unsigned char red)
		{*(CurrentPtr + TrueColorBlueOffset) = blue;
		*(CurrentPtr + TrueColorGreenOffset) = green;
		*(CurrentPtr + TrueColorRedOffset) = red;
		TrueColorSetFiller(CurrentPtr);}
	void Increment(void)
		{CurrentPtr += TrueColorPixelSize;}
	void Decrement(void)
		{CurrentPtr -= TrueColorPixelSize;}
};
class G42Byte24BitIterator
{
private:
	byte *	CurrentPtr;
public:
	G42Byte24BitIterator(byte * row) : CurrentPtr(row)
		{}
	~G42Byte24BitIterator(void)
		{}
	unsigned char GetBlue(void)
		{return *(CurrentPtr + TrueColorBlueOffset);}
	unsigned char GetGreen(void)
		{return *(CurrentPtr + TrueColorGreenOffset);}
	unsigned char GetRed(void)
		{return *(CurrentPtr + TrueColorRedOffset);}
	int32 GetBlue32(void)
		{return (int32)(*(CurrentPtr + TrueColorBlueOffset));}
	int32 GetGreen32(void)
		{return (int32)(*(CurrentPtr + TrueColorGreenOffset));}
	int32 GetRed32(void)
		{return (int32)(*(CurrentPtr + TrueColorRedOffset));}
	void Set(unsigned char blue, unsigned char green, unsigned char red)
		{*(CurrentPtr + TrueColorBlueOffset) = blue;
		*(CurrentPtr + TrueColorGreenOffset) = green;
		*(CurrentPtr + TrueColorRedOffset) = red;
		TrueColorSetFiller(CurrentPtr);}
	void Increment(void)
		{CurrentPtr += TrueColorPixelSize;}
	void Decrement(void)
		{CurrentPtr -= TrueColorPixelSize;}
	void Forward(int num)
		{CurrentPtr += num * TrueColorPixelSize;}
	void Backward(int num)
		{CurrentPtr -= num * TrueColorPixelSize;}
};
class G42ConstByte24BitIterator
{
private:
	const byte *	CurrentPtr;
public:
	G42ConstByte24BitIterator(const byte * row) : CurrentPtr(row)
		{}
	~G42ConstByte24BitIterator(void)
		{}
	unsigned char GetBlue(void)
		{return *(CurrentPtr + TrueColorBlueOffset);}
	unsigned char GetGreen(void)
		{return *(CurrentPtr + TrueColorGreenOffset);}
	unsigned char GetRed(void)
		{return *(CurrentPtr + TrueColorRedOffset);}
	int32 GetBlue32(void)
		{return (int32)(*(CurrentPtr + TrueColorBlueOffset));}
	int32 GetGreen32(void)
		{return (int32)(*(CurrentPtr + TrueColorGreenOffset));}
	int32 GetRed32(void)
		{return (int32)(*(CurrentPtr + TrueColorRedOffset));}
	void Increment(void)
		{CurrentPtr += TrueColorPixelSize;}
	void Decrement(void)
		{CurrentPtr -= TrueColorPixelSize;}
	void Forward(int num)
		{CurrentPtr += num * TrueColorPixelSize;}
	void Backward(int num)
   	{CurrentPtr -= num * TrueColorPixelSize;}
};
#endif // G42ITER_H

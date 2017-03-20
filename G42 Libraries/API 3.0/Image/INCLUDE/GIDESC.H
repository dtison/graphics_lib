// GIDESC.H - G42ImageDescriptor
#if !defined(GIDESC_H)
#define GIDESC_H
/*-----------------------------------------------------------------------------
	file:			gidesc.h
	author:		Guy Schalnat
	date:			11/06/96
	revision:	3.0
	This class holds all the information needed to deal with image data.
	It is a G42PathObject, and is expected to be copied and saved often,
	as the image data itself does not have all this information.
	More information is found in image.txt
-----------------------------------------------------------------------------*/
#include "gpobj.h"
#include "gmem.h"
#include "gitype.h"
#include "gnotify.h"
//#include "girgn.h"
class G42ImageData; // for CreateData()
class G42Region;
class G42ImageDescriptor : public G42PathObject, public G42NotifyMessage
{
public:
	// These are all the combinations allowed in this version of the library.
	enum {	ColorMask = 0x40, // true on all color images
				DepthMask = 0x3f, // part of code that contains pixel depth
				PaletteMask = 0x80 }; // true on paletted image types
	enum G42ImageType {	Gray1 = 1,
								Gray8 = 8,
								Color8 = 8 | ColorMask | PaletteMask,
								Color24 = 24 | ColorMask,
								Color32 = 32 | ColorMask};
	// constructors, destructor, operator =
	G42ImageDescriptor(uint32 width, uint32 height, G42ImageType type,
		G42HeapManager & heap, bool trans = false);
	G42ImageDescriptor(G42ImageDescriptor & ref);
	G42ImageDescriptor(G42ImageDescriptor & ref, uint32 width, uint32 height);
	G42ImageDescriptor(G42ImageDescriptor & ref, uint32 width, uint32 height,
		G42ImageType type);
	G42ImageDescriptor & operator = (G42ImageDescriptor & ref);
	virtual ~G42ImageDescriptor(void);
	// member functions
	void ClearBackgroundColor(void);
	void ClearRegion (void) { Region.Clear(); }
	void ClearTransparentColor(void);
	void ClearTransparency(void);
	void Copy(G42ImageDescriptor & ref);
	G42ImageData * CreateData(uint32 strip_num) const;
	const G42Color & GetBackgroundColor(void) const
		{ return BackgroundColor; }
	byte GetBackgroundPixel (void) const
		{ return (byte)BackgroundPixel; }
	byte GetDepth(void) const
		{ return PixelDepth; }
	G42HeapManager & GetHeap(void) const
		{ return Heap; }
	uint32 GetHeight(void) const
		{ return Height; }
	G42ImageType GetImageType(void) const
		{ return ImageType; }
	uint16 GetNumPalette(void) const
		{ return NumPalette; }
	uint32 GetNumStrips(void) const
		{ return NumStrips; }
	const G42Color * GetPalette (void) const
		{ return ImagePalette; }
	G42Color * GetPalette (void)
		{ return ImagePalette; }
	G42Region & GetRegion (void)
		{ return *Region; }
	uint32 GetRowBytes(void) const
		{ return RowBytes; }
	uint32 GetRowNumInStrip(uint32 row_num) const;
	uint32 GetRowsInStrip(uint32 strip_num) const;
	uint32 GetRowsPerStrip(void) const
		{ return RowsPerStrip; }
	uint16 GetStripMask(void) const
		{ return StripMask; }
	uint32 GetStripNum(uint32 row_num) const;
	uint16 GetStripShift(void) const
		{ return StripShift; }
	uint32 GetStripStartInImage(uint32 strip_num) const;
	const G42Color & GetTransparentColor(void) const
		{ return TransparentColor; }
	byte GetTransparentPixel (void) const
		{ return (byte)TransparentPixel; }
	uint32 GetTransRowBytes(void) const
		{ return TransRowBytes; }
	uint32 GetWidth(void) const
		{ return Width; }
	bool HasAlphaChannel(void) const
		{ return AlphaChannel; }
	bool HasBackgroundColor(void) const
		{ return BackgroundColorSet; }
	bool HasBackgroundPixel (void) const
		{ return (BackgroundPixel >= 0); }
	bool HasRegion (void) const
		{ return Region; }
	bool HasTransparentColor(void) const
		{ return TransparentColorSet; }
	bool HasTransparentPixel (void) const
		{ return (TransparentPixel >= 0); }
	bool IsColor(void) const
		{ return (bool)(ImageType & ColorMask); }
	bool IsPaletted (void) const
		{ return (bool)(ImageType & PaletteMask); }
	bool IsTransparent (void) const
		{ return Transparent; }
	void SetAlphaChannel(bool trans = true);
	void SetBackgroundColor(G42Color & back_color);
	void SetBackgroundPixel (int16 back_pixel);
	void SetNumPalette (uint16 num_palette);
		// throw(G42InvalidParameterException)
	void SetTransparentPixel (int16 trans_pixel);
		// throw(G42InvalidParameterException)
	void SetTransparentColor(const G42Color & trans_color);
		// throw(G42InvalidParameterException)
	void SetTransparency(bool trans = true);
		// throw(G42InvalidParameterException)
	void SetRegion (const G42Region & region); 
private:
	// non-calculated variables
	bool						AlphaChannel; // image has alpha channel if Transparent is true
	G42Color					BackgroundColor; // the color of the background
	bool						BackgroundColorSet; // true if background color valid
	int16						BackgroundPixel; // >= 0 if color in palette, -1 otherwise
	uint32					FirstStripRemainder; // calculated, how much shorter first strip is
	uint32					FirstStripHeight; // height of first strip (<= RowsPerStrip)
	uint32					Height; // height of image
	G42HeapManager &		Heap;
	G42APtr <G42Color>	ImagePalette; // palette for image, req. if Color8, optional otherwise
	G42ImageType			ImageType; // image type
	uint32					LastStripHeight; // calculated - height of last strip
	uint16					NumPalette; // number of colors in palette
	uint32					NumStrips; // calculated - number of strips
	byte						PixelDepth; // calculated - depth in bits of a pixel
	G42Ptr <G42Region>	Region; // region of image to be displayed and used
	uint32					RowBytes; // calculated - bytes in a row, 4 byte aligned
	uint32					RowsPerStrip; // calculated - rows in middle strips
	uint16					StripMask; // calculated - RowsPerStrip - 1
	byte						StripShift; // RowsPerStrip = (1 << StripShift)
	bool						Transparent; // true if image has transparent mask
	G42Color					TransparentColor; // Color (if any) that is transparent
	bool						TransparentColorSet; // true if transparency based off of a color
	int16						TransparentPixel; // pixel in palette for Transparent Color, -1 otherwise
	uint32					TransRowBytes; // calculated - row bytes of transparent mask
	uint32					Width; // width of image
	// private functions
	
	void Set(uint32 width, uint32 height, G42ImageType type,
		byte strip_shift, uint32 first_strip_height = 0);
	void Set(uint32 width, uint32 height, G42ImageType type,
		bool trans = false); // sets strip size
	void Calculate(void); // builds all precalculated variables
	void CalculateRowBytes(void); // does row bytes and depth
	void CalculateStrips(); // fills in calculated strip info
	void CalculateStripShift(int32 max_strip_bytes, bool trans); // gets StripShift
};
/*-----------------------------------------------------------------------------
	Function - GetStripNum
	Parameters:
		uint32 row_num - row number you wish to find strip for
	Exceptions:
		G42InvalidParameterException if row is outside of image.
	Description:
		Returns strip that contains row_num
-----------------------------------------------------------------------------*/
inline uint32
G42ImageDescriptor::GetStripNum(uint32 row_num) const
	// throw(G42InvalidParameterException)
{
	if (row_num >= Height)
	{
		throw G42InvalidParameterException();
	}
	return ((row_num + FirstStripRemainder) >> StripShift);
}
/*-----------------------------------------------------------------------------
	Function - GetRowNumInStrip
	Parameters:
		uint32 row_num - row number you wish to find row number in strip for
	Exceptions:
		G42InvalidParameterException if row is outside of image.
	Description:
		Returns row in strip that contains row_num
-----------------------------------------------------------------------------*/
inline uint32
G42ImageDescriptor::GetRowNumInStrip(uint32 row_num) const
	// throw(G42InvalidParameterException)
{
	if (row_num >= Height)
	{
		throw G42InvalidParameterException();
	}
	return (row_num < FirstStripHeight ? row_num :
		((row_num + FirstStripRemainder) & StripMask));
}
/*-----------------------------------------------------------------------------
	Function - GetStripStartInImage
	Parameters:
		uint32 strip_num - strip number you wish to starting row for
	Exceptions:
		G42InvalidParameterException if strip is outside of image.
	Description:
		Returns row in image that strip starts
-----------------------------------------------------------------------------*/
inline uint32
G42ImageDescriptor::GetStripStartInImage(uint32 strip_num) const
	// throw(G42InvalidParameterException)
{
	if (strip_num >= NumStrips)
	{
		throw G42InvalidParameterException();
	}
	return (strip_num ?
		(strip_num - 1) * RowsPerStrip + FirstStripHeight :
		0);
}
/*-----------------------------------------------------------------------------
	Function - GetRowsInStrip
	Parameters:
		uint32 strip_num - strip number you wish to find number of rows for
	Exceptions:
		G42InvalidParameterException if strip is outside of image.
	Description:
		Returns number of rows in strip
-----------------------------------------------------------------------------*/
inline uint32
G42ImageDescriptor::GetRowsInStrip(uint32 strip_num) const
	// throw(G42InvalidParameterException)
{
	if (strip_num >= NumStrips)
	{
		throw G42InvalidParameterException();
	}
	if (!strip_num)
	{
		return FirstStripHeight;
	}
	if (strip_num == NumStrips - 1)
	{
		return LastStripHeight;
	}
	return RowsPerStrip;
}
/*-----------------------------------------------------------------------------
	Function - G42GetImageType
	Parameters:
		byte depth - minimum depth of data
		bool color - true if data is color
	Description:
		Returns minimum image type that will hold data of requested
		depth and channels.  You can pass in anything reasonable here,
		and it will return the correct type.  On Windows, it will not
		return Color32.  On Mac, it currently only returns Color32, but
		Color24 is expected sooner or later to help memory concerns.
-----------------------------------------------------------------------------*/
G42ImageDescriptor::G42ImageType G42GetImageType(byte depth, bool color);
#include "girgn.h"
#endif // GIDESC_H

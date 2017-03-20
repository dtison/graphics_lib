#if !defined(G42ITERF_H)
#define G42ITERF_H
/*  G42iterf.cp	-- 	"Fast" Iterators: Routines for use when large amounts of data 
					will be processed and speed is imperative.  D. Ison 6-96 
	
	Notes:	1.  The scanline routines provide services that eventually be part of
				the iterators.
			2.  A "FastIncrement()" and "FastDecrement()" group of functions can
				be made that would utilize the scanline data that's either stored
				in the iterator or perhaps passed in.  They also would be inlines.				
			3.  Compatibility with the existing iterators can be achieved once
				fast operation is accomplished.
 			4.  There is a _slight_ memory-tradeoff for using the fast iterators.
 */
#include "g42iter.h"
/*  Generic class with functionality that most iterators will need  */
// TODO:  1. initialize cachebuffer in constructor for this class
class G42FastIterator
{
	public:
	#if 0
				G42FastIterator (void);
	virtual		~G42FastIterator (void);
	#endif
	void		GetScanlineFromTiles (G42ImageData * data, byte *scanline, 
					int numPixels, short bytesPerPixel, int rowNumber);
	void		PutScanlineIntoTiles (G42ImageData * data, byte *scanline, 
					int numPixels, short bytesPerPixel, int rowNumber);
	protected:
	int			FastStartCol;
};
class G42FastLockedID24BitIterator: public G42LockedID24BitIterator,
									public G42FastIterator
{
public:
	G42FastLockedID24BitIterator(G42ImageData * data, bool init = true); 
	virtual ~G42FastLockedID24BitIterator (void);
	bool	FastNextRow (void);
	byte	FastGetRed (void) {return *(CurrentPtr + TrueColorRedOffset);}	
	byte	FastGetGreen (void) {return *(CurrentPtr + TrueColorGreenOffset);}	
	byte	FastGetBlue (void) {return *(CurrentPtr + TrueColorBlueOffset);}	
	void	FastSetArea (int left, int top, int right, int bottom);
	G42Color&		operator++(int) 
	{
		#ifdef MACOS	// Slightly optimized because byte order is convenient
		*(long *) &CurrentColor = *(long *) (CurrentPtr + 1);
		#else
		CurrentColor.red 	= CurrentPtr [TrueColorRedOffset];
		CurrentColor.green 	= CurrentPtr [TrueColorGreenOffset];
		CurrentColor.blue 	= CurrentPtr [TrueColorBlueOffset];
		#endif
		CurrentPtr += TrueColorPixelSize;
		return CurrentColor;
	};
	/*  Temporary functions (?)  */
	byte 	*GetCacheBuffer (void) {return CacheBuffer;};
private:
	byte		*CacheBuffer;
	int			NumPixels;
	G42Color	CurrentColor;
	int			FastMaxRow;
	int			FastMaxCol;	
};
class G42FastLockedID8BitIterator: public G42LockedID8BitIterator,
									public G42FastIterator
{
public:
	G42FastLockedID8BitIterator(G42ImageData * data, bool init = true); 
	virtual ~G42FastLockedID8BitIterator (void);
	bool	FastNextRow (void);
	void	FastSetArea (int left, int top, int right, int bottom);
	byte 	operator++(int) {return *CurrentPtr++;};
private:
	byte		*CacheBuffer;
	int			NumPixels;
	int			FastMaxRow;
	int			FastMaxCol;	
};
class G42FastImageData8BitIterator: public G42ImageData8BitIterator,
									public G42FastIterator
{
public:
	G42FastImageData8BitIterator (G42ImageData * data, bool init = true);
	virtual		~G42FastImageData8BitIterator (void);
	bool		FastNextRow (void);
	byte 		operator++(int) {return *CurrentPtr++;};
	void		FastSetArea (int left, int top, int right, int bottom);
private:
	byte		*CacheBuffer;
	int			NumPixels;
	int			FastMaxRow;
	int			FastMaxCol;	
};
#if 0
class G42FastImageData4BitIterator: public G42ImageData4BitIterator,
									public G42FastIterator
{
public:
	G42FastImageData4BitIterator (G42ImageData * data, bool init = true);
	virtual	~G42FastImageData4BitIterator (void);
	bool	FastNextRow (void);
	byte 	operator++(int) 
	{
		int index	= (CurrentPixel >> 1);
		byte value = CacheBuffer [index];
		if ((CurrentPixel % 2) != 0)
			value >>= 4;
		else
			value = (value & 0x00ff);
		CurrentPixel++;
		return value;
	};
private:
	byte	*CacheBuffer;
	int		NumPixels;
	int		CurrentPixel;
};
#endif
#endif // G42ITERF_H

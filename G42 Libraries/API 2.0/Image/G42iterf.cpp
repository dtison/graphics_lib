#include "g42iterf.h"
/* --- G42FastIterator Functions ---*/
/*  The pixelBytes parameter is a little wierd because you pass in *negative* the number
	of bits to shift for 4 and 1 bit data [aka -1 and -3].  But, it handles all cases  */
void
G42FastIterator::GetScanlineFromTiles (G42ImageData *data, byte *scanline, int numPixels, 
	short pixelBytes, int rowNumber)
{
	int	bytesThisTile 	= 0;
	int bytesCopied 	= 0;
	int	pixelsProcessed = 0;
	int	imageX 			= FastStartCol;
	/*  Go across the tiles converting their lines into 1 scanline  */
	do
	{
		int tileRow = data -> GetTileRowIndex (rowNumber);
		int tileCol = data -> GetTileColumnIndex (imageX);
		G42ImageTile *tile = data -> GetTile (tileRow, tileCol);
		int tileOffsetX = tile -> GetTileOffsetX();
		int tileOffsetY = tile -> GetTileOffsetY();
		int rowInTile 	= rowNumber - tile -> GetImageOffsetY() + tileOffsetY;
		int colInTile 	= imageX - tile -> GetImageOffsetX() + tileOffsetX;
		int tilePixels 	= tile -> GetTileWidth() - colInTile + tileOffsetX;
		bytesThisTile	= pixelBytes > 0 ? (tilePixels * pixelBytes) : tilePixels >> -pixelBytes;
		{
			G42LockedImageTile lTile (tile);	// Read-only tile
			G42ImageRow *row = tile -> GetRow (rowInTile);
			#ifdef OLDWAY
			byte *sourceBuffer = (*row) + ((pixelBytes > 0) ? (tileOffsetX * pixelBytes)
			: (tileOffsetX >> -pixelBytes));
			sourceBuffer += colInTile * pixelBytes; 
			#endif
			byte *sourceBuffer = (*row) + (colInTile * pixelBytes);
			memcpy ((scanline + bytesCopied), sourceBuffer, bytesThisTile);
		}
		bytesCopied 	+= bytesThisTile;
		imageX 			+= tilePixels;
		pixelsProcessed += tilePixels;
	}  while (pixelsProcessed < numPixels);
}
void
G42FastIterator::PutScanlineIntoTiles (G42ImageData * data, byte *scanline, int numPixels, 
	short pixelBytes, int rowNumber)
{
	int	bytesThisTile 	= 0;
	int bytesCopied 	= 0;
	int	pixelsProcessed = 0;
	int	imageX 			= FastStartCol;
	/*  Go across the tiles converting their lines into 1 scanline  */
	do
	{
		int tileRow = data -> GetTileRowIndex (rowNumber);
		int tileCol = data -> GetTileColumnIndex (imageX);
		G42ImageTile *tile = data -> GetTile (tileRow, tileCol);
		int tileOffsetX = tile -> GetTileOffsetX();
		int tileOffsetY = tile -> GetTileOffsetY();
		int rowInTile 	= rowNumber - tile -> GetImageOffsetY() + tileOffsetY;
		int colInTile 	= imageX - tile -> GetImageOffsetX() + tileOffsetX;
		int tilePixels 	= tile -> GetTileWidth() - colInTile + tileOffsetX;
		bytesThisTile	= pixelBytes > 0 ? (tilePixels * pixelBytes) : tilePixels >> -pixelBytes;
		{
			G42ModifiableImageTile lTile (tile);	// Read-write tile
			G42ImageRow *row = tile -> GetRow (rowInTile);
			#ifdef OLDWAY
			byte *destBuffer = (*row) + ((pixelBytes > 0) ? (tileOffsetX * pixelBytes) 
				: (tileOffsetX >> -pixelBytes)); 
			destBuffer += colInTile * pixelBytes; 
			#endif
			byte *destBuffer = (*row) + (colInTile * pixelBytes);
			memcpy (destBuffer, (scanline + bytesCopied), bytesThisTile);
			lTile.MarkModified();
		}
		bytesCopied 	+= bytesThisTile;
		imageX 			+= tilePixels;
		pixelsProcessed += tilePixels;
	}  while (pixelsProcessed < numPixels);
}
/* --- G42FastLockedID24BitIterator Functions --- (Read-only iterator) */
G42FastLockedID24BitIterator::G42FastLockedID24BitIterator (G42ImageData * data, bool init) 
	: G42LockedID24BitIterator (data, init), CacheBuffer (0), NumPixels (data -> GetWidth())
{	
	CacheBuffer = new byte [NumPixels * TrueColorPixelSize];
   if (init)
		FastSetArea (0, 0, data -> GetWidth() - 1, data -> GetHeight() - 1);
};
G42FastLockedID24BitIterator::~G42FastLockedID24BitIterator (void)
{
	if (CacheBuffer)
	{
		delete [] CacheBuffer;
		CacheBuffer = 0;
	}
}
bool
G42FastLockedID24BitIterator::FastNextRow (void)
{
	bool retVal = true;
	if (CurrentRow < FastMaxRow)
	{
		CurrentRow++;
		GetScanlineFromTiles (Data, CacheBuffer, NumPixels, TrueColorPixelSize, CurrentRow);
		CurrentPtr = CacheBuffer;
	}
	else
		retVal = false;
	return (retVal);
}
void
G42FastLockedID24BitIterator::FastSetArea (int left, int top, int right, int bottom)
{
	CurrentRow 		= top;
	FastMaxRow 		= bottom;
	FastMaxCol 		= right;
	FastStartCol 	= left;
	NumPixels		= right - left + 1;
	GetScanlineFromTiles (Data, CacheBuffer, NumPixels, TrueColorPixelSize, CurrentRow);
	CurrentPtr = CacheBuffer;
}
/* --- G42FastLockedID8BitIterator Functions --- (Read-only iterator)  */
G42FastLockedID8BitIterator::G42FastLockedID8BitIterator (G42ImageData * data, bool init)
	: G42LockedID8BitIterator (data, init), CacheBuffer (0), NumPixels (data -> GetWidth())
{
	CacheBuffer = new byte [NumPixels];
   if (init)
		FastSetArea (0, 0, data -> GetWidth() - 1, data -> GetHeight() - 1);
};
G42FastLockedID8BitIterator::~G42FastLockedID8BitIterator (void)
{
	if (CacheBuffer)
	{
		delete [] CacheBuffer;
		CacheBuffer = 0;
	}
}
bool
G42FastLockedID8BitIterator::FastNextRow (void)
{
	bool retVal = true;
	if (CurrentRow < FastMaxRow)
	{
		CurrentRow++;
		GetScanlineFromTiles (Data, CacheBuffer, NumPixels, 1, CurrentRow);
		CurrentPtr = CacheBuffer;
	}
	else
		retVal = false;
	return (retVal);
}
void
G42FastLockedID8BitIterator::FastSetArea (int left, int top, int right, int bottom)
{
	CurrentRow 		= top;
	FastMaxRow 		= bottom;
	FastMaxCol 		= right;
	FastStartCol 	= left;
	NumPixels		= right - left + 1;
	GetScanlineFromTiles (Data, CacheBuffer, NumPixels, 1, CurrentRow);
	CurrentPtr = CacheBuffer;
}
/* --- G42FastImageData8BitIterator Functions --- (Write-only iterator) */
G42FastImageData8BitIterator::G42FastImageData8BitIterator (G42ImageData * data, bool init) 
	: G42ImageData8BitIterator (data, init), CacheBuffer (0), NumPixels (data -> GetWidth())
{	
	CacheBuffer = new byte [NumPixels];
   if (init)
		FastSetArea (0, 0, data -> GetWidth() - 1, data -> GetHeight() - 1);
};
G42FastImageData8BitIterator::~G42FastImageData8BitIterator (void)
{
	if (CacheBuffer)
	{
		delete [] CacheBuffer;
		CacheBuffer = 0;
	}
}
bool
G42FastImageData8BitIterator::FastNextRow (void)
{
	bool retVal = true;
	if (CurrentRow < FastMaxRow)
	{
		PutScanlineIntoTiles (Data, CacheBuffer, NumPixels, 1, CurrentRow);
		CurrentRow++;
		CurrentPtr = CacheBuffer;
	}
   else if (CurrentRow == FastMaxRow)
   {
		PutScanlineIntoTiles (Data, CacheBuffer, NumPixels, 1, CurrentRow);
		retVal = false;
   }
	else
		retVal = false;
	return (retVal);
}
void
G42FastImageData8BitIterator::FastSetArea (int left, int top, int right, int bottom)
{
	CurrentRow 		= top;
	FastMaxRow 		= bottom;
	FastMaxCol 		= right;
	FastStartCol 	= left;
	NumPixels		= right - left + 1;
	GetScanlineFromTiles (Data, CacheBuffer, NumPixels, 1, CurrentRow);
	CurrentPtr = CacheBuffer;
}
#if 0
/* --- G42FastImageData4BitIterator Functions ---*/
G42FastImageData4BitIterator::G42FastImageData4BitIterator (G42ImageData * data, bool init)
	: G42ImageData4BitIterator (data, init) , CacheBuffer (0), NumPixels (data -> GetWidth()),
	CurrentPixel (0)
{
	CacheBuffer = new byte [NumPixels * TrueColorPixelSize];
	GetScanlineFromTiles (Data, CacheBuffer, NumPixels, -1, CurrentRow);
	CurrentPtr = CacheBuffer;
};
G42FastImageData4BitIterator::~G42FastImageData4BitIterator (void)
{
	if (CacheBuffer)
	{
		delete [] CacheBuffer;
		CacheBuffer = 0;
	}
}
bool
G42FastImageData4BitIterator::FastNextRow (void)
{
	bool retVal = true;
	if (CurrentRow < (Data -> GetHeight() - 1))
	{
		PutScanlineIntoTiles (Data, CacheBuffer, NumPixels, -1, CurrentRow);
		CurrentRow++;
		GetScanlineFromTiles (Data, CacheBuffer, NumPixels, -1, CurrentRow);
		CurrentPtr = CacheBuffer;
	}
	else
		retVal = false;
	return (retVal);
}
#endif

// g42dithr.h - dithering routines
/*************************************************************
	File:          g42dithr.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   classes to handle dithering
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
      Code   Date   Name and Description
***************************************************************/
#ifndef G42DITHR_H
#define G42DITHR_H
#include "g42opal.h"
class G42FSDither
{
	public:
		struct ColorError
		{
      	int16 red, green, blue;
		};
		G42FSDither(int rowbytes, G42OptimizedPalette * optimized_palette) :
			rerr(0), gerr(0), berr(0), CurrentError(0), NextError(0),
			NumBytes(rowbytes), OptimizedPalette(optimized_palette)
		{
			CurrentError = new ColorError [NumBytes + 2];
			memset(CurrentError, 0, (NumBytes + 2) * sizeof (ColorError));
			NextError = new ColorError [NumBytes + 2];
			memset(NextError, 0, (NumBytes + 2) * sizeof (ColorError));
			ResetRow();
		}
		void ResetRow(void)
		{
			ColorError * temp = CurrentError;
			CurrentError = NextError;
			NextError = temp;
			memset(NextError, 0, (NumBytes + 2) * sizeof (ColorError));
			CurrentPtr = CurrentError + 1;
			NextPtr = NextError + 1;
			CurBytes = 0;
			rerr = gerr = berr = 0;
		}
		~G42FSDither()
		{
			if (CurrentError)
				delete[] CurrentError;
			if (NextError)
				delete[] NextError;
		}
		byte GetDither(byte c)
		{
			if (OptimizedPalette)
				return (OptimizedPalette->GetPixel(c));
			return c;
		}
		byte GetDither(const G42Color & c)
		{
			int p = 0;
			berr += (int)CurrentPtr->blue;
			if (berr > 48)
				berr = 32;
			else if (berr > 16)
				berr = ((berr - 16) >> 1) + 16;
			if (berr < -48)
				berr = -32;
			else if (berr < -16)
				berr = -((-berr - 16) >> 1) - 16;
			int b = (int)(c.blue) + berr;
			if (b < 0)
				b = 0;
			if (b > 255)
				b = 255;
			gerr += (int)CurrentPtr->green;
			if (gerr > 48)
				gerr = 32;
			else if (gerr > 16)
				gerr = ((gerr - 16) >> 1) + 16;
			if (gerr < -48)
				gerr = -32;
			else if (gerr < -16)
				gerr = -((-gerr - 16) >> 1) - 16;
			int g = (int)(c.green) + gerr;
			if (g < 0)
				g = 0;
			if (g > 255)
				g = 255;
			rerr += (int)CurrentPtr->red;
			if (rerr > 48)
				rerr = 32;
			else if (rerr > 16)
				rerr = ((rerr - 16) >> 1) + 16;
			if (rerr < -48)
				rerr = -32;
			else if (rerr < -16)
				rerr = -((-rerr - 16) >> 1) - 16;
			int r = (int)(c.red) + rerr;
			if (r < 0)
				r = 0;
			if (r > 255)
				r = 255;
			if (OptimizedPalette)
			{
				p = OptimizedPalette->GetPixel(G42Color(r, g, b));
#ifdef XWIN
				p += (256 - OptimizedPalette->GetNumPalette());
#endif
				rerr = r - (int)OptimizedPalette->GetPalette()[p].red;
				gerr = g - (int)OptimizedPalette->GetPalette()[p].green;
				berr = b - (int)OptimizedPalette->GetPalette()[p].blue;
			}
			else
			{
			if (r == g && r == b)
			{
				int gp = g + 8;
				gp = ((gp - (gp >> 4)) >> 4);
				rerr = gerr = berr = g - ((gp << 4) | gp);
#ifdef XWIN
				p = gp + 240;
#else
				p = gp + 192;
#endif
			}
			else
			{
				int rp = r + 26;
				rp = ((rp - (rp >> 2) - (rp >> 3)) >> 5);
				int gp = g + 18;
            gp = ((gp - (gp >> 3)) >> 5);
				int bp = b + 42;
				bp = ((bp - (bp >> 2)) >> 6);
				rerr = r - ((rp << 5) + (rp << 4) + (rp << 1) + rp);
				gerr = g - ((gp << 5) | (gp << 2) | (gp >> 1));
				berr = b - ((bp << 6) | (bp << 4) | (bp << 2) | bp);
#ifdef XWIN
				p = (byte)(((rp << 5) | (gp << 2) | bp) + 48);
#else
				p = (byte)((rp << 5) | (gp << 2) | bp);
#endif
			}
         }
			int rerr1 = (rerr >> 4);
			int rerr3 = (rerr >> 2) + rerr1;
			int rerr7 = (rerr >> 1) + rerr3;
			int gerr1 = (gerr >> 4);
			int gerr3 = (gerr >> 2) + gerr1;
			int gerr7 = (gerr >> 1) + gerr3;
			int berr1 = (berr >> 4);
			int berr3 = (berr >> 2) + berr1;
			int berr7 = (berr >> 1) + berr3;
			rerr -= (rerr1 + rerr3 + rerr7);
			gerr -= (gerr1 + gerr3 + gerr7);
			berr -= (berr1 + berr3 + berr7);
			NextPtr--;
			NextPtr->red += rerr3;
			NextPtr->green += berr3;
			NextPtr->blue += gerr3;
			NextPtr++;
			NextPtr->red += rerr7;
			NextPtr->green += gerr7;
			NextPtr->blue += berr7;
			NextPtr++;
			NextPtr->red += rerr1;
			NextPtr->green += gerr1;
			NextPtr->blue += berr1;
			CurrentPtr++;
			CurBytes++;
			if (CurBytes == NumBytes)
				ResetRow();
			return p;
		}
	protected:
		int rerr, gerr, berr;
		ColorError * CurrentError;
		ColorError * NextError;
		ColorError * CurrentPtr;
		ColorError * NextPtr;
      G42OptimizedPalette * OptimizedPalette;
		int CurBytes;
		int NumBytes;
};
class G42Dither
{
	public:
		G42Dither() : rerr(0), gerr(0), berr(0) {}
		~G42Dither() {}
		byte GetDither(const G42Color & c)
		{
			int p = 0;
			int b = (int)(c.blue) + berr;
			if (b < 0)
				b = 0;
			if (b > 255)
				b = 255;
			int g = (int)(c.green) + gerr;
			if (g < 0)
				g = 0;
			if (g > 255)
				g = 255;
			int r = (int)(c.red) + rerr;
			if (r < 0)
				r = 0;
			if (r > 255)
				r = 255;
			if (r == g && r == b)
			{
				int gp = g + 8;
				gp = ((gp - (gp >> 4)) >> 4);
				rerr = gerr = berr = g - ((gp << 4) | gp);
#ifdef XWIN
				p = gp + 240;
#else
				p = gp + 192;
#endif
			}
			else
			{
				int rp = r + 26;
				rp = ((rp - (rp >> 2) - (rp >> 3)) >> 5);
				int gp = g + 18;
            gp = ((gp - (gp >> 3)) >> 5);
				int bp = b + 42;
				bp = ((bp - (bp >> 2)) >> 6);
				rerr = r - ((rp << 5) + (rp << 4) + (rp << 1) + rp);
				gerr = g - ((gp << 5) | (gp << 2) | (gp >> 1));
				berr = b - ((bp << 6) | (bp << 4) | (bp << 2) | bp);
#ifdef XWIN
				p = (byte)(((rp << 5) | (gp << 2) | bp) + 48);
#else
				p = (byte)((rp << 5) | (gp << 2) | bp);
#endif
			}
			return p;
		}
	protected:
		int rerr, gerr, berr;
};
class G42GrayDither
{
	public:
		G42GrayDither() : gerr(0) {}
		~G42GrayDither() {}
		byte GetDither(const byte gray)
		{
			int g = (int)(gray) + gerr;
			if (g < 0)
				g = 0;
			if (g > 255)
				g = 255;
			int gp = g + 8;
			gp = ((gp - (gp >> 4)) >> 4);
			gerr = g - ((gp << 4) | gp);
#ifdef XWIN
			int p = gp + 240;
#else
			int p = gp + 192;
#endif
			return p;
		}
	protected:
		int gerr;
};
class G42ColorToGray
{
	public:
		G42ColorToGray() {}
		~G42ColorToGray() {}
		byte GetGray(const G42Color & c)
		{
			return (byte)((
				(long)c.red * (long)306 +
				(long)c.green * (long)601 +
				(long)c.blue * (long)117 +
				(long)512) >> 10);
		}
};
#endif // G42DITHR_H

// g42opal.h - optimized palette
/*************************************************************
	File:          g42opal.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   Optimize a palette for an image
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#ifndef G42OPAL_H
#define G42OPAL_H
#include "g42idata.h"
class G42OptimizedPalette
{
	public:
		G42OptimizedPalette(G42ImageData * data, int num_palette,
			bool need_white = false, bool standard_palette = false,
			bool full_color_only = true, G42ImageData * sub_data = 0);
		~G42OptimizedPalette();
		byte GetPixel(G42Color & c) {
			return DitherTable[(((int)c.red & 0xf0) << 4) |
				(((int)c.green & 0xf8)) | ((int)c.blue >> 5)];
		}
		byte GetDataTranslation(byte b) {
			return DitherTable[b];
		}
		byte GetSubDataTranslation(byte b) {
			return DitherTable[b + 256];
		}
		byte GetTranslation(byte b, bool sub_data = false) {
			return DitherTable[b + (sub_data ? 256 : 0)];
		}
		bool GetNeedFullColor(void) { return NeedOptimize; }
		G42Color * GetPalette(void) { return DitherPalette; }
		int GetNumPalette(void) { return NumPalette; }
#ifdef XWIN
		int GetWhitePixel(void)
			{ return (256 - NumPalette + WhitePixel); }
#else
		int GetWhitePixel(void) { return WhitePixel; }
#endif
	protected:
		struct G42PaletteBox
		{
			int rmin, gmin, bmin, rmax, gmax, bmax;
			uint16 nh;
			uint16 *h;
			void ResetBoxLimits(void);
		};
		uint16 ColorTo16(G42Color & c) {
			return (uint16)(((uint16)(c.red & 0xf0) << 4) |
				(uint16)(c.green & 0xf8) |
				(uint16)(c.blue >> 5));
		}
		uint16 RGBTo16(int red, int green, int blue) {
			return (uint16)(((uint16)(red & 0xf0) << 4) |
				((uint16)(green & 0xf8)) |
				(uint16)(blue >> 5));
		}
		uint16 ByteTo16(byte * ptr) {
			return (uint16)((((uint16)*(ptr + 2) & 0xf0) << 4) |
				(((uint16)*(ptr + 1) & 0xf8)) |
				((uint16)*ptr >> 5));
		}
		int CalcDistance(int dr, int dg, int db)
		{
			if (dr < 0)
				dr = -dr;
			if (dg < 0)
				dg = -dg;
			if (db < 0)
				db = -db;
			int dm = (max(dg, max(dr, db)) >> 1);
			int div = 0;
			int d = 0;
			if (dr > dm)
			{
				d += dr;
				div++;
			}
			if (dg > dm)
			{
				d += dg;
				div++;
			}
			if (db > dm)
			{
				d += db;
				div++;
			}
			if (div == 3)
				return (d >> 1);
			if (div == 2)
				return (d - ((d + 2) >> 2) - ((d + 8) >> 4) -
					((d + 32) >> 6) - ((d + 128) >> 8) -
					((d + 512) >> 10)); // aprox. 2/3
			if (div == 1)
				return d;
			return 0;
		}
		void CreateStandardPalette(void);
		void InvestigateData(G42ImageData * data);
		void AddColor(G42Color c);
		void AddGray(byte g)
			{G42Color c(g, g, g); AddColor(c); }
		void OptimizeData(void);
		void PrepareTable(void);
		byte * DitherUnique;
		byte * DitherTable;
		G42Color * DitherPalette;
		G42ImageData * Data;
		G42ImageData * SubData;
		int NumPalette;
		int MaxPalette;
		bool FullColorOnly;
		int WhitePixel;
		bool NeedOptimize;
		bool NeedWhite;
		// PaletteBox should be allocated and deleted, as it isn't needed
		// outside of the constructor
		G42PaletteBox PaletteBox[256];
		// these three are lowercase for historical reasons.  They need changed,
		// but carefully
		uint16 * h; // histogram - really used for various things
		uint16 nh; // number of histogram elements used
};
#endif // G42OPAL_H

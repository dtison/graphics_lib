// g42opal.h - optimized palette
/*************************************************************
	File:          giopal.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   Optimize a palette for an image
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#ifndef GIOPAL_H
#define GIOPAL_H
#include "gitype.h"
#include "gpcomp.h"
#include "gidesc.h"
#include "gidata.h"
#include "gtools.h"
#include "gilab.h"
// to change the number of bits this uses per color channel, just change
// NUM_BITS.  It should work for any number from 4 to 8.  It won't work
// for less then 4.
#define NUM_BITS 5
#define NUM_BITS2 (NUM_BITS << 1)
#define BLUE_SHIFT (8 - NUM_BITS)
#define GREEN_SHIFT ((NUM_BITS - 4) << 1)
#define RED_SHIFT (NUM_BITS * 3 - 8)
#define COLOR_MASK (256 - (1 << BLUE_SHIFT))
#define CHANNEL_MASK (0xff >> BLUE_SHIFT)
#define NUM_COLORS (1 << (NUM_BITS * 3))
#define NUM_COLORS_IN_CHANNEL (1 << NUM_BITS)
class G42OptimizedPalette : public G42PathComponent
{
	public:
		enum G42PaletteType { PaletteWindows16, PaletteNetscape, PaletteStandard };
#if 0
		G42OptimizedPalette(const G42Color * pal, intf num_pal,
			G42Color * add_color, intf num_add, intf num_avoid);
#endif
		G42OptimizedPalette(intf num_pal, bool auto_finish = true); // goes into a path
		G42OptimizedPalette(G42PaletteType pal_type, G42Color * add_color,
			intf num_add, intf num_avoid);
		G42OptimizedPalette(G42PaletteType pal_type,
			intf num_pal);
		~G42OptimizedPalette();
		void Process(G42PathObject * object, G42PathComponent * sender);
		void FinishOptimization(void); // for auto_finish = false with path
		byte GetPixel(G42Color & c) const {
			return DitherTable[(((intf)c.Red & COLOR_MASK) << RED_SHIFT) |
				(((intf)c.Green & COLOR_MASK) << GREEN_SHIFT) |
				((intf)c.Blue >> BLUE_SHIFT)];
		}
		byte GetAvoidPixel(G42Color & c);
		byte GetAnyPixel(G42Color & c);
		byte GetDataTranslation(byte b) const {
			return DitherTable[b];
		}
		byte GetSubDataTranslation(byte b) const {
			return DitherTable[b + 256];
		}
		byte GetTranslation(byte b, bool sub_data = false) const
			{ return DitherTable[b + (sub_data ? 256 : 0)]; }
		bool GetNeedFullColor(void) const { return NeedOptimize; }
		G42Color * GetPalette(void) const { return DitherPalette; }
		intf GetNumPalette(void) const { return NumPalette; }
#ifdef XWIN
		intf GetWhitePixel(void) const
			{ return (256 - NumPalette + WhitePixel); }
#else
		intf GetWhitePixel(void) const { return WhitePixel; }
#endif
		void SetForceColor(G42Color c);
	protected:
		struct G42PaletteBox
		{
			uint16 rmin, gmin, bmin, rmax, gmax, bmax;
			uint16 frmin, fgmin, fbmin, frmax, fgmax, fbmax;
			uint16 num_force;
			byte * force_index;
			uint32 diag_squared;
			uint32 nh;
			uint32 *h;
			void ResetBoxLimits(void);
			void ResetForceBoxLimits(uint32 * force_h);
			void ResetBoxInfo(void);
			bool IsInBox(uint32 i) const
			{
				uint16 r = (uint16)((i >> NUM_BITS2) & CHANNEL_MASK);
				uint16 g = (uint16)((i >> NUM_BITS) & CHANNEL_MASK);
				uint16 b = (uint16)(i & CHANNEL_MASK);
				return (r <= rmax && r >= rmin && g <= gmax && g >= gmin &&
					b <= bmax && b >= bmin);
			}
		};
		uint32 ColorTo16(G42Color & c) {
			return (uint32)(((uint32)(c.Red & COLOR_MASK) << RED_SHIFT) |
				(uint32)((c.Green & COLOR_MASK) << GREEN_SHIFT) |
				(uint32)(c.Blue >> BLUE_SHIFT));
		}
		uint32 RGBTo16(intf red, intf green, intf blue) {
			return (uint32)(((uint32)(red & COLOR_MASK) << RED_SHIFT) |
				((uint32)((green & COLOR_MASK) << GREEN_SHIFT)) |
				(uint32)(blue >> BLUE_SHIFT));
		}
		uint32 ByteTo16(byte * ptr) {
			return (uint32)((((uint32)*(ptr + 2) & COLOR_MASK) << RED_SHIFT) |
				(((uint32)(*(ptr + 1) & COLOR_MASK) << GREEN_SHIFT)) |
				((uint32)*ptr >> BLUE_SHIFT));
		}
		intf CalcDistance(intf dr, intf dg, intf db)
		{
			if (dr < 0)
				dr = -dr;
			if (dg < 0)
				dg = -dg;
			if (db < 0)
				db = -db;
			intf dm = (max(dg, max(dr, db)) >> 1);
			intf div = 0;
			intf d = 0;
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
		void HandleAvoid(void);
		void AddAvoid(void);
		void SetupAvoid(void);
		byte * DitherUnique;
		byte * DitherTable;
		G42Color * DitherPalette;
		G42Vector<G42Lab> LabPalette; // lazyness here, for speed I should
												// allocate it myself
#if 0
		G42ImageData * Data;
		G42ImageData * SubData;
		G42ImageData ** DataArray;
#endif
		intf NumDataArray;
		intf NumPalette;
		intf MaxPalette;
		bool FullColorOnly;
		intf WhitePixel;
		bool NeedOptimize;
		bool NeedWhite;
		G42PaletteType StandardPaletteType;
		// PaletteBox should be allocated and deleted, as it isn't needed
		// outside of the constructor
		G42PaletteBox PaletteBox[256];
		// these three are lowercase for historical reasons.  They need changed,
		// but carefully
		uint32 * h; // histogram - really used for various things
		uint32 nh; // number of histogram elements used
		// new for add/avoid colors
		G42Color * AddColorList;
		intf NumAdd;
		// which colors to avoid, and which pixels they correspond to
		G42Color * AvoidColor;
		intf * AvoidPixel;
		intf NumAvoid;
		bool * Avoid;
		G42Ptr<G42ImageDescriptor> Descriptor;
		uint32 Ticks;
		bool UseLab;
		bool AutoFinish;
		uint16 NumForceH;
		uint16 NumForce;
		G42Color ForceColor[256];
		uint32 ForceH[256];
		byte BoxForceIndex[256];
		bool ForceInH[256];
		byte ForceHToColor[256];
};
#endif // GIOPAL_H

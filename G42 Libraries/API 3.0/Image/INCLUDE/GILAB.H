// gilab.h - In-progress xyz/lab color class, including conversions to/from G42Color
#if !defined(GILAB_H)
#define GILAB_H
#include "gitype.h"
/*
	There is a .CPP file that goes with this and has the conversion code.
	To convert an RGB color to LAB:
	1.  Convert monitor RGB color to 1.0 gamma (take off monitor's gamma)
	2.  Convert RGB (1.0 Gamma) to XYZ using RGBToXYZ()
	3.  Convert XYZ color to LAB using XYZToLAB()
	(In this implementation XYZ colors are 0..1, floating point.)
	LAB Colors are represented as in the Tiff 6.0 specification.
		L* 1 byte. Unsigned. Ranges 0..255.  Represents 0.100 theoretical lightness.
		a  1 byte. Signed.  Ranges -128..127.  Represents theoretically unbounded a* value.
		b  1 byte. Signed.  Ranges -128..127. Represents theoretically unbounded b* value.
*/
/*  Turn this off to use integer arithmetic with slight accuracy tradeoff  */
#define USE_FLOATING_POINT
// this controls the accuracy of the tables we use to speed things up
// at 5 bits, the tables are off by an average of 0.6%, and use 192KB
#define LAB_BITS 5
#define LAB_NUM (1 << LAB_BITS)
#define LAB_BITS2 (LAB_BITS << 1)
#define LAB_BITS3 (LAB_BITS + LAB_BITS2)
#define LAB_MASK (0x100 - (1 << (8 - LAB_BITS)))
#define FLAT2SCREEN_SHIFT 2
#define FLAT2SCREEN_NUM (1 << (16 - FLAT2SCREEN_SHIFT))
/*  Parent class for CIE colors.  It knows how to convert to and from screen gamma.
	 It has no components, ie, x, y z, or RGB. */
class G42CIEColor
{
public:
	G42CIEColor (void);
	static uint16 	ScreenToFlat (byte screen)
		{ return ScreenToFlatTable[screen]; }
	static byte 	FlatToScreen (uint16 screen)
		{ return FlatToScreenTable[screen >> FLAT2SCREEN_SHIFT]; }
	static double 	GetScreenGamma (void)
		{ return ScreenGamma; }
	static void 	SetScreenGamma (double gamma);
private:
	static uint16 	ScreenToFlatTable [256];
	static G42APtr <byte> FlatToScreenTable;
	static double 	ScreenGamma;
	static bool		ScreenGammaSet;
};
/*  XYZ colors are the basis for several CIE color spaces  */
class G42XYZColor : public G42CIEColor
{
public:
	#ifdef USE_FLOATING_POINT
	float X;
	float Y;
	float Z;
	#else
	int32 X;
	int32 Y;
	int32 Z;
	#endif
	G42XYZColor (void) {}
	#ifdef USE_FLOATING_POINT
	G42XYZColor (float x, float y, float z) :
	#else
	G42XYZColor (int32 x, int32 y, int32 z) :
	#endif
		X (x), Y (y), Z (z) {}
	G42XYZColor (const G42XYZColor & ref)
		: X (ref.X), Y (ref.Y), Z (ref.Z) {}
	bool operator == (const G42XYZColor & ref)
		{return X == ref.X && Y == ref.Y && Z == ref.Z;}
	G42XYZColor & operator = (const G42XYZColor & ref)
		{X = ref.X; Y = ref.Y; Z = ref.Z; return *this;}
	G42XYZColor (const G42Color & color);
	operator G42Color (void);
	G42XYZColor & operator = (G42XYZColor & xyz);
	G42XYZColor & operator = (const G42Color & color);
	/*  Static things last  */
	static G42XYZColor 	RGBToXYZ (G42Color16 & color);
	static G42Color16		XYZToRGB (G42XYZColor & color);
};
/*  LAB colors support most of the conversions we want  */
class G42LABColor : public G42XYZColor
{
public:
	byte L;
	int8 A;
	int8 B;
	G42LABColor (void) {}
	G42LABColor (byte l, int8 a, int8 b) :
		L (l), A (a), B (b) {}
	G42LABColor (const G42LABColor & ref)
		: L (ref.L), A (ref.A), B (ref.B) {}
	bool operator == (const G42LABColor & ref)
		{return L == ref.L && A == ref.A && B == ref.B;}
	G42LABColor & operator = (const G42LABColor & ref)
		{L = ref.L; A = ref.A; B = ref.B; return *this; }
	G42LABColor (const G42Color & color);
	operator G42Color (void);
	G42LABColor & operator = (G42LABColor & lab);	// MW won't compile w/const
	G42LABColor & operator = (const G42Color & color);
	/*  Static things last  */
	static G42LABColor 	XYZToLAB (G42XYZColor & color);
	static G42XYZColor	LABToXYZ (G42LABColor & color);
	static void 			SetUseTable (bool flag) {UseTable = flag;}
	static bool				GetUseTable (void) { return UseTable; }
private:
	void 						RgbToLab (const G42Color & c,
									byte & l, int8 & a, int8 & b);
	void 						LabToRgb (byte l, int8 a, int8 b, G42Color & c);
	uintf 					RgbToUintf(byte r, byte g, byte b);
	void 						BuildTables (void);
	void 						RgbToLabBuilder (const G42Color & c,
									byte & l, int8 & a, int8 & b);
	void						LabToRgbBuilder(byte l, int8 a, int8 b, byte & red,
									byte & green, byte & blue);
	/*  Static things last  */
	static G42APtr <byte>	RgbToLTable;
	static G42APtr <int8>	RgbToATable;
	static G42APtr <int8>	RgbToBTable;
	static G42APtr <byte>	LabToRTable;
	static G42APtr <byte>	LabToGTable;
	static G42APtr <byte>	LabToBTable;
	static bool 			UseTable;
	static bool 			TablesBuilt;
};
typedef G42LABColor G42Lab;
inline uintf
G42LABColor::RgbToUintf(byte r, byte g, byte b)
{
	return ((uintf) (((uintf)r & LAB_MASK) << (LAB_BITS3 - 8)) |
		(uintf) (((uintf)g & LAB_MASK) << ((LAB_BITS - 4) << 1)) |
		(uintf) ((uintf)b >> (8 - LAB_BITS)));
};
#endif // GILAB_H

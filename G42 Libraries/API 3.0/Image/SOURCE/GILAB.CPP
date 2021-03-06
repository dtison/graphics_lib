#include "gilab.h"
#include "math.h"
/*  Remaining dislikes:
	1.  Don't like static allocations from Gamma and LAB LUT's.  Use G42Ptr?  
	2.  Using inline for some reason was problematic under CodeWarrior.
	3.  Is this affecting efficiency?	*/
const int8 CIEXIndex 		= 0; 
const int8 CIEYIndex 		= 1; 
const int8 CIEZIndex 		= 2; 
const int8 CIERedIndex 		= 0; 
const int8 CIEGreenIndex 	= 1; 
const int8 CIEBlueIndex 	= 2; 
const double RefWhite_X		= 0.9502;
const double RefWhite_Y		= 1.0;
const double RefWhite_Z		= 1.0877;
const double CIELAB_Threshold	= 0.008856;
const double CIE_Cube_Root 	= (double) 1 / (double) 3;
const double CIE_Cube 			= (double) 3;
// Returns G42Color with X Y Z normalized to 0 255
// CCIR ref 709 Color spec., D6500 ref whitepoint
#ifdef USE_FLOATING_POINT
double XYZMatrix [3] [3] =
		{{0.41235, 0.212507, 0.019319},
		{0.357675, 0.715350, 0.119225},
		{0.180355, 0.072142, 0.949870}};
double XYZInvMatrix [3] [3] =
		{{3.242981, -0.968977, 0.055668},
		{-1.538337, 1.875492, -0.204119},
		{-0.498920, 0.041544, 1.057708}};
#else
// 16 bit shifts
int32 XYZMatrix2 [3] [3] =
		{{27024, 13927, 1266},
		{ 23441, 46881, 7814},
		{11820, 4728, 62251}};
int32 XYZInvMatrix2 [3] [3] =
		{{212532, -63502, 3648},
		{ -100815,122912, -13376},
		{-32696,  2723,  69318}};
#endif
//bool G42LABColor::UseTable 	= false;
bool G42LABColor::UseTable 	= true;
bool G42LABColor::TablesBuilt = false;
G42APtr <byte> G42LABColor::RgbToLTable;
G42APtr <int8> G42LABColor::RgbToATable;
G42APtr <int8> G42LABColor::RgbToBTable;
G42APtr <byte> G42LABColor::LabToRTable;
G42APtr <byte> G42LABColor::LabToGTable;
G42APtr <byte> G42LABColor::LabToBTable;
uint16 G42CIEColor::ScreenToFlatTable [256];
G42APtr <byte> G42CIEColor::FlatToScreenTable;
double G42CIEColor::ScreenGamma;
bool G42CIEColor::ScreenGammaSet = false;
#ifdef MACOS
#pragma  global_optimizer on
#pragma  optimization_level 4
#endif
G42CIEColor::G42CIEColor (void)
{
	#ifdef MSWIN
//	ScreenGamma  = 1.0;
	ScreenGamma  = 2.2;
//	ScreenGamma = 1.8;
	#endif
	#ifdef MACOS
	ScreenGamma = 1.8;
	#endif
	if (!ScreenGammaSet)
	{
		SetScreenGamma(ScreenGamma);
		ScreenGammaSet = true;
	}
}
void
G42CIEColor::SetScreenGamma (double gamma)
{
	ScreenGamma = gamma;
	double inverse_gamma = 1.0 / ScreenGamma;
	uintf i;
	for (i = 0; i < 256; i++)
	{
		ScreenToFlatTable[i] = (uint16)(65535.0 *
			pow((double)i / 255.0, ScreenGamma) + .5);
	}
	if (!FlatToScreenTable)
	{
		FlatToScreenTable = new byte [FLAT2SCREEN_NUM];
	}
	for (i = 0; i < FLAT2SCREEN_NUM; i++)
	{
		FlatToScreenTable[i] = (byte)(255.0 *
			pow((double)i / (double)(FLAT2SCREEN_NUM - 1), inverse_gamma) + .5);
	}
}
G42XYZColor 
G42XYZColor::RGBToXYZ (G42Color16 & color)
{
	G42XYZColor xyz;
	#ifdef USE_FLOATING_POINT
	/*  Scale RGB to 0..1  */
	double red 		= (double) color.Red / 65535.0;
	double green 	= (double) color.Green / 65535.0;
	double blue 	= (double) color.Blue / 65535.0;
	/*  Matrix muliply  */
	xyz.X = ((red * XYZMatrix [0][CIEXIndex]) +
				(green * XYZMatrix [1][CIEXIndex]) +
				(blue * XYZMatrix [2][CIEXIndex]));
	xyz.Y = ((red * XYZMatrix [0][CIEYIndex]) +
				(green * XYZMatrix [1][CIEYIndex]) +
				(blue * XYZMatrix [2][CIEYIndex]));
	xyz.Z = ((red * XYZMatrix [0][CIEZIndex]) +
				(green * XYZMatrix [1][CIEZIndex]) +
				(blue * XYZMatrix [2][CIEZIndex]));
	#else
	int32 value;
	value = ((color.Red * XYZMatrix2 [0][CIEXIndex]) + 
				(color.Green * XYZMatrix2 [1][CIEXIndex]) + 
				(color.Blue * XYZMatrix2 [2][CIEXIndex]));
	xyz.X = (value >> 16);
	value = ((color.Red * XYZMatrix2 [0][CIEYIndex]) + 
				(color.Green * XYZMatrix2 [1][CIEYIndex]) + 
				(color.Blue * XYZMatrix2 [2][CIEYIndex]));
	xyz.Y = (value >> 16);
	value = ((color.Red * XYZMatrix2 [0][CIEZIndex]) + 
				(color.Green * XYZMatrix2 [1][CIEZIndex]) + 
				(color.Blue * XYZMatrix2 [2][CIEZIndex]));
	xyz.Z = (value >> 16);
	#endif
	return xyz; 
}
G42Color16
G42XYZColor::XYZToRGB (G42XYZColor & color)
{
	G42Color16 rgb;
	#ifdef USE_FLOATING_POINT
	/*  Inverse matrix multiply and rescale to 0..255  */
	int32 value = (int32) (((color.X * XYZInvMatrix [0][CIERedIndex]) +
						(color.Y * XYZInvMatrix [1][CIERedIndex]) +
						(color.Z * XYZInvMatrix [2][CIERedIndex])) * 65535.0);
	rgb.Red = (uint16)min (max ((int32)0, value), (int32)65535);
	value = (int32) (((color.X * XYZInvMatrix [0][CIEGreenIndex]) +
				(color.Y * XYZInvMatrix [1][CIEGreenIndex]) +
				(color.Z * XYZInvMatrix [2][CIEGreenIndex])) * 65535.0);
	rgb.Green = (uint16) min (max ((int32)0, value), (int32)65535);
	value = (int32) (((color.X * XYZInvMatrix [0][CIEBlueIndex]) + 
				(color.Y * XYZInvMatrix [1][CIEBlueIndex]) + 
				(color.Z * XYZInvMatrix [2][CIEBlueIndex])) * 65535.0);
	rgb.Blue = (uint16) min (max ((int32)0, value), (int32)65535);
	#else
	
	/*  On Mac PPC this is strangely slow. 5 forward, 44 back.  Q: Why?  */
	int32 value =  (color.X * XYZInvMatrix2 [0][CIERedIndex]) + 
						(color.Y * XYZInvMatrix2 [1][CIERedIndex]) + 
						(color.Z * XYZInvMatrix2 [2][CIERedIndex]);
	value >>= 16;
	rgb.Red = (byte)min (max ((int32)0, value), (int32)255);
	
	value =  (color.X * XYZInvMatrix2 [0][CIEGreenIndex]) + 
				(color.Y * XYZInvMatrix2 [1][CIEGreenIndex]) + 
				(color.Z * XYZInvMatrix2 [2][CIEGreenIndex]);
	value >>= 16;
	rgb.Green = (byte)min (max ((int32)0, value), (int32)255);
	value =  (color.X * XYZInvMatrix2 [0][CIEBlueIndex]) + 
				(color.Y * XYZInvMatrix2 [1][CIEBlueIndex]) + 
				(color.Z * XYZInvMatrix2 [2][CIEBlueIndex]);
	value >>= 16;
	rgb.Blue = (byte)min (max ((int32)0, value), (int32)255);
	#endif
	return rgb; 
}
G42XYZColor &
G42XYZColor::operator = (G42XYZColor & xyz)
{
	X = xyz.X;
	Y = xyz.Y;
	Z = xyz.Z;
	return *this;
}
G42LABColor 
G42LABColor::XYZToLAB (G42XYZColor & color)
{
	G42LABColor lab;	
	/*  Normalize vectors  */
	#ifdef USE_FLOATING_POINT
	double x_vector =  color.X / RefWhite_X;
	double y_vector =  color.Y / RefWhite_Y;
	double z_vector =  color.Z / RefWhite_Z;	
	#else
	double x_vector = (double) color.X / 255.0 / RefWhite_X;
	double y_vector = (double) color.Y / 255.0 / RefWhite_Y;
	double z_vector = (double) color.Z / 255.0 / RefWhite_Z;	
	#endif
	#if 0
	if (x_vector < CIELAB_Threshold)
	{
		int x = 0;
	}
	if (y_vector < CIELAB_Threshold)
	{
		int x = 0;
	}
	if (z_vector < CIELAB_Threshold)
	{
		int x = 0;
	}
	#endif
	/*  Store the Y/Yn ^^ 1/3 term for optimization  */
	double cube_root_y_vector = pow (y_vector, CIE_Cube_Root);
	if (y_vector > CIELAB_Threshold) 
	{
		lab.L = (byte) ((116 * cube_root_y_vector - 16.0) / 100.0 * 255.0);
	}
	else 
	{
		lab.L = 9.033 * y_vector * 255.0;
	}	
	
	lab.A = (int8) (500.0 * (pow (x_vector, CIE_Cube_Root) - cube_root_y_vector));
	lab.B = (int8) (200.0 * (cube_root_y_vector - pow (z_vector, CIE_Cube_Root)));
	return lab; 
}
G42XYZColor 
G42LABColor::LABToXYZ (G42LABColor & color)
{
	G42XYZColor xyz;	
	double x_vector, y_vector, z_vector;
	/*  Scale L* to 0..100  */
	double temp_val = (double) (color.L / 255.0 * 100.0);
	if (color.L > 8)	// TODO:  CIELAB_Threshold ??
	{
		temp_val = (temp_val + 16) / 116.0;
		y_vector = pow (temp_val, CIE_Cube) * RefWhite_Y;
	}
	else
	{
		y_vector = temp_val / 903.3 * RefWhite_Y;
	}
	/*  Store the Y/Yn ^^ 1/3 term for optimization  */
	double cube_root_y_vector = pow (y_vector, CIE_Cube_Root);
	temp_val = ((double) color.A / 500.0) + cube_root_y_vector;
	x_vector = pow (temp_val, CIE_Cube);
	
	temp_val = cube_root_y_vector - ((double) color.B / 200.0);
	z_vector = pow (temp_val, CIE_Cube);
	#ifdef USE_FLOATING_POINT
	xyz.X = x_vector * RefWhite_X;
	xyz.Y = y_vector;
	xyz.Z = z_vector * RefWhite_Z ;
	#else
	xyz.X = (int32) (x_vector * RefWhite_X * 255.0);
	xyz.Y = (int32) (y_vector * 255.0);
	xyz.Z = (int32) (z_vector * RefWhite_Z * 255.0);
	#endif
	return xyz;
}
void
G42LABColor::BuildTables(void)
{
	RgbToLTable = new byte [1 << LAB_BITS3];
	RgbToATable = new int8 [1 << LAB_BITS3];
	RgbToBTable = new int8 [1 << LAB_BITS3];
	G42Color c;
	for (uintf r = 0; r < LAB_NUM; r++)
	{
		uintf ir = r << LAB_BITS2;
		c.Red = (byte)((r << (8 - LAB_BITS)) | (r >> ((LAB_BITS - 4) << 1)));
		for (uintf g = 0; g < LAB_NUM; g++)
		{
			uintf ig = ir | (g << LAB_BITS);
			c.Green = (byte)((g << (8 - LAB_BITS)) | (g >> ((LAB_BITS - 4) << 1)));
			for (uintf b = 0; b < LAB_NUM; b++)
			{
				uintf i = ig | b;
				c.Blue = (byte)((b << (8 - LAB_BITS)) |
					(b >> ((LAB_BITS - 4) << 1)));
				RgbToLabBuilder(c, RgbToLTable[i], RgbToATable[i], RgbToBTable[i]);
			}
		}
	}
	LabToRTable = new byte [1 << LAB_BITS3];
	LabToGTable = new byte [1 << LAB_BITS3];
	LabToBTable = new byte [1 << LAB_BITS3];
	byte lab_l;
	int8 lab_a;
	int8 lab_b;
	for (uintf l = 0; l < LAB_NUM; l++)
	{
		uintf il = l << LAB_BITS2;
		lab_l = (byte)((l << (8 - LAB_BITS)) | (l >> ((LAB_BITS - 4) << 1)));
		for (uintf a = 0; a < LAB_NUM; a++)
		{
			uintf ia = il | (a << LAB_BITS);
			lab_a = (int8)(((a << (8 - LAB_BITS)) |
				(a >> ((LAB_BITS - 4) << 1))) - 128);
			for (uintf b = 0; b < LAB_NUM; b++)
			{
				uintf i = ia | b;
				lab_b = (int8)(((b << (8 - LAB_BITS)) |
					(b >> ((LAB_BITS - 4) << 1))) - 128);
				LabToRgbBuilder(lab_l, lab_a, lab_b,
					LabToRTable[i], LabToGTable[i], LabToBTable[i]);
			}
		}
	}
	TablesBuilt = true;
#if 0
// get table error versus non-table
	double err = 0.0;
	for (intf red = 0; red < 256; red++)
	{
		for (intf green = 0; green < 256; green++)
		{
			for (intf blue = 0; blue < 256; blue++)
			{
				byte l1, l2;
				int8 a1, a2, b1, b2;
				G42Color c(red, green, blue);
				UseTable = false;
				RgbToLab(c, l1, a1, b1);
				UseTable = true;
				RgbToLab(c, l2, a2, b2);
				int32 dl = (int32)l1 - (int32)l2;
				int32 da = (int32)a1 - (int32)a2;
				int32 db = (int32)b1 - (int32)b2;
				err += sqrt((double)(dl * dl + da * da + db * db));
			}
		}
	}
	err /= 42949672.96;
	char buf[111];
	sprintf(buf, "Lab Table Error: %f%%", err); 
	::MessageBox(0, buf, "Lab Table Error", MB_OK);
#endif
}
void
G42LABColor::RgbToLab(const G42Color & c, byte & l, int8 & a, int8 & b)
{
	if (UseTable)
	{
		if (! TablesBuilt)
		{
			BuildTables();
		}
		uintf rgb = RgbToUintf(c.Red, c.Green, c.Blue);
#if 1
		intf red = rgb >> LAB_BITS2;
		red = (red << (8 - LAB_BITS)) | (red >> ((LAB_BITS - 4) << 1));
		intf dred = (intf)c.Red - red;
		intf absdred = (dred > 0 ? dred : -dred);
		if (absdred > (1 << (7 - LAB_BITS)))
		{
			if (dred > 0)
			{
				rgb += (1 << LAB_BITS2);
			}
			else
			{
				rgb -= (1 << LAB_BITS2);
			}
			red = rgb >> LAB_BITS2;
			red = (red << (8 - LAB_BITS)) | (red >> ((LAB_BITS - 4) << 1));
			dred = (intf)c.Red - red;
			absdred = (dred > 0 ? dred : -dred);
		}
		intf green = ((rgb >> LAB_BITS) & ((1 << LAB_BITS) - 1));
		green = (green << (8 - LAB_BITS)) | (green >> ((LAB_BITS - 4) << 1));
		intf dgreen = (intf)c.Green - green;
		intf absdgreen = (dgreen > 0 ? dgreen : -dgreen);
		if (abs(dgreen) > (1 << (7 - LAB_BITS)))
		{
			if (dgreen > 0)
			{
				rgb += (1 << LAB_BITS);
			}
			else
			{
				rgb -= (1 << LAB_BITS);
			}
			green = ((rgb >> LAB_BITS) & ((1 << LAB_BITS) - 1));
			green = (green << (8 - LAB_BITS)) | (green >> ((LAB_BITS - 4) << 1));
			dgreen = (intf)c.Green - green;
			absdgreen = (dgreen > 0 ? dgreen : -dgreen);
		}
		intf blue = rgb & ((1 << LAB_BITS) - 1);
		blue = (blue << (8 - LAB_BITS)) | (blue >> ((LAB_BITS - 4) << 1));
		intf dblue = (intf)c.Blue - blue;
		intf absdblue = (dblue > 0 ? dblue : -dblue);
		if (abs(dblue) > (1 << (7 - LAB_BITS)))
		{
			if (dblue > 0)
			{
				rgb++;
			}
			else
			{
				rgb--;
			}
			blue = rgb & ((1 << LAB_BITS) - 1);
			blue = (blue << (8 - LAB_BITS)) | (blue >> ((LAB_BITS - 4) << 1));
			dblue = (intf)c.Blue - blue;
			absdblue = (dblue > 0 ? dblue : -dblue);
		}
#endif
		l = RgbToLTable[rgb];
		a = RgbToATable[rgb];
		b = RgbToBTable[rgb];
#if 1
		intf dl = 0;
		intf da = 0;
		intf db = 0;
		if (dred)
		{
			uintf rgb2 = rgb;
			if (dred < 0)
				rgb2 -= (1 << LAB_BITS2);
			else // dred > 0
				rgb2 += (1 << LAB_BITS2);
			dl += ((intf)RgbToLTable[rgb2] - (intf)l) * absdred;
			da += ((intf)RgbToATable[rgb2] - (intf)a) * absdred;
			db += ((intf)RgbToBTable[rgb2] - (intf)b) * absdred;
		}
		if (dgreen)
		{
			uintf rgb2 = rgb;
			if (dgreen < 0)
				rgb2 -= (1 << LAB_BITS);
			else
				rgb2 += (1 << LAB_BITS);
			dl += ((intf)RgbToLTable[rgb2] - (intf)l) * absdgreen;
			da += ((intf)RgbToATable[rgb2] - (intf)a) * absdgreen;
			db += ((intf)RgbToBTable[rgb2] - (intf)b) * absdgreen;
		}
		if (dblue)
		{
			uintf rgb2 = rgb;
			if (dblue < 0)
				rgb2 -= 1;
			else
				rgb2 += 1;
			dl += ((intf)RgbToLTable[rgb2] - (intf)l) * absdblue;
			da += ((intf)RgbToATable[rgb2] - (intf)a) * absdblue;
			db += ((intf)RgbToBTable[rgb2] - (intf)b) * absdblue;
		}
		l += (byte)(dl >> (1 << (7 - LAB_BITS)));
		a += (int8)(da >> (1 << (7 - LAB_BITS)));
		b += (int8)(db >> (1 << (7 - LAB_BITS)));
#endif
	}
	else
	{
		G42Color16 rgb(ScreenToFlat(c.Red), ScreenToFlat(c.Green),
			ScreenToFlat(c.Blue));
		G42XYZColor xyz = G42XYZColor::RGBToXYZ(rgb);
		G42LABColor lab = G42LABColor::XYZToLAB(xyz);
		l = lab.L;
		a = lab.A;
		b = lab.B;
	}
}
void
G42LABColor::RgbToLabBuilder(const G42Color & c, byte & l, int8 & a, int8 & b)
{
	G42Color16 rgb(ScreenToFlat(c.Red), ScreenToFlat(c.Green),
		ScreenToFlat(c.Blue));
	G42XYZColor xyz = G42XYZColor::RGBToXYZ(rgb);
	G42LABColor lab = G42LABColor::XYZToLAB(xyz);
	l = lab.L;
	a = lab.A;
	b = lab.B;
}
void
G42LABColor::LabToRgbBuilder(byte l, int8 a, int8 b, byte & red,
	byte & green, byte & blue)
{
	G42LABColor lab(l, a, b);
	G42XYZColor xyz = G42LABColor::LABToXYZ(lab);
	G42Color16 rgb = G42XYZColor::XYZToRGB(xyz);
	red = FlatToScreen(rgb.Red);
	green = FlatToScreen(rgb.Green);
	blue = FlatToScreen(rgb.Blue);
}
void
G42LABColor::LabToRgb(byte lab_l, int8 lab_a, int8 lab_b, G42Color & c)
{
	if (UseTable)
	{
		if (! TablesBuilt)
		{
			BuildTables();
		}
		uintf lab = RgbToUintf(lab_l, (byte)((intf)lab_a + 128),
			(byte)((intf)lab_b + 128));
#if 1
		intf l = (byte)(lab >> LAB_BITS2);
		l = (byte)((l << (8 - LAB_BITS)) | (l >> ((LAB_BITS - 4) << 1)));
		intf dl = (intf)lab_l - (intf)l;
		intf absdl = (dl > 0 ? dl : -dl);
		if (absdl > (1 << (7 - LAB_BITS)))
		{
			if (dl > 0)
			{
				lab += (1 << LAB_BITS2);
			}
			else
			{
				lab -= (1 << LAB_BITS2);
			}
			l = (byte)(lab >> LAB_BITS2);
			l = (byte)((l << (8 - LAB_BITS)) | (l >> ((LAB_BITS - 4) << 1)));
			dl = (intf)lab_l - (intf)l;
			absdl = (dl > 0 ? dl : -dl);
		}
		byte a = (byte)(((lab >> LAB_BITS) & ((1 << LAB_BITS) - 1)));
		a = (byte)((a << (8 - LAB_BITS)) | (a >> ((LAB_BITS - 4) << 1)));
		intf da = (intf)lab_a + 128 - (intf)a;
		intf absda = (da > 0 ? da : -da);
		if (abs(da) > (1 << (7 - LAB_BITS)))
		{
			if (da > 0)
			{
				lab += (1 << LAB_BITS);
			}
			else
			{
				lab -= (1 << LAB_BITS);
			}
			a = (byte)(((lab >> LAB_BITS) & ((1 << LAB_BITS) - 1)));
			a = (byte)((a << (8 - LAB_BITS)) | (a >> ((LAB_BITS - 4) << 1)));
			da = (intf)lab_a + 128 - (intf)a;
			absda = (da > 0 ? da : -da);
		}
		byte b = (byte)(lab & ((1 << LAB_BITS) - 1));
		b = (byte)((b << (8 - LAB_BITS)) | (b >> ((LAB_BITS - 4) << 1)));
		intf db = (intf)lab_b + 128 - (intf)b;
		intf absdb = (db > 0 ? db : -db);
		if (abs(db) > (1 << (7 - LAB_BITS)))
		{
			if (db > 0)
			{
				lab++;
			}
			else
			{
				lab--;
			}
			b = (byte)(lab & ((1 << LAB_BITS) - 1));
			b = (byte)((b << (8 - LAB_BITS)) | (b >> ((LAB_BITS - 4) << 1)));
			db = (intf)lab_b + 128 - (intf)b;
			absdb = (db > 0 ? db : -db);
		}
#endif
		intf red = LabToRTable[lab];
		intf green = LabToGTable[lab];
		intf blue = LabToBTable[lab];
#if 1
		intf dred = 0;
		intf dgreen = 0;
		intf dblue = 0;
		if (dl)
		{
			uintf lab2 = lab;
			if (dl < 0)
				lab2 -= (1 << LAB_BITS2);
			else // dred > 0
				lab2 += (1 << LAB_BITS2);
			dred += ((intf)LabToRTable[lab2] - (intf)red) * absdl;
			dgreen += ((intf)LabToGTable[lab2] - (intf)green) * absdl;
			dblue += ((intf)LabToBTable[lab2] - (intf)blue) * absdl;
		}
		if (da)
		{
			uintf lab2 = lab;
			if (da < 0)
				lab2 -= (1 << LAB_BITS);
			else
				lab2 += (1 << LAB_BITS);
			dred += ((intf)LabToRTable[lab2] - (intf)red) * absda;
			dgreen += ((intf)LabToGTable[lab2] - (intf)green) * absda;
			dblue += ((intf)LabToBTable[lab2] - (intf)blue) * absda;
		}
		if (db)
		{
			uintf lab2 = lab;
			if (db < 0)
				lab2 -= 1;
			else
				lab2 += 1;
			dred += ((intf)LabToRTable[lab2] - (intf)red) * absdb;
			dgreen += ((intf)LabToGTable[lab2] - (intf)green) * absdb;
			dblue += ((intf)LabToBTable[lab2] - (intf)blue) * absdb;
		}
		red += (byte)(dred >> (1 << (7 - LAB_BITS)));
		green += (byte)(dgreen >> (1 << (7 - LAB_BITS)));
		blue += (byte)(dblue >> (1 << (7 - LAB_BITS)));
#endif
		c.Red = (byte)red;
		c.Green = (byte)green;
		c.Blue = (byte)blue;
	}
	else
	{
		G42LABColor lab(lab_l, lab_a, lab_b);
		G42XYZColor xyz = G42LABColor::LABToXYZ(lab);
		G42Color16 rgb = G42XYZColor::XYZToRGB(xyz);
		c.Red = FlatToScreen(rgb.Red);
		c.Green = FlatToScreen(rgb.Green);
		c.Blue = FlatToScreen(rgb.Blue);
	}
}
G42LABColor::G42LABColor (const G42Color & color)
{
	RgbToLab(color, L, A, B);
}
G42LABColor::operator G42Color (void)
{
	G42Color color;
	LabToRgb(L, A, B, color);
	return color;
}
G42LABColor &
G42LABColor::operator = (G42LABColor & lab)
{
	L = lab.L;
	A = lab.A;
	B = lab.B;
	return *this;
}
G42LABColor &
G42LABColor::operator = (const G42Color & color)
{
	RgbToLab(color, L, A, B);
	return *this;
}
#ifdef MACOS
#pragma  global_optimizer reset
#endif

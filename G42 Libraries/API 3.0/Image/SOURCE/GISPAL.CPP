
#include "gitype.h"
#include "gispal.h"
G42ScreenPalette::G42ScreenPalette(G42PaletteMode mode, G42Surface surface)
:
	ScreenType(TrueColor), ScreenDepth(0), Type(Group42), Mode(mode),
	NumPalette(256), Surface(surface)
{
	InvestigateScreen();
	if (NumPalette > (1 << ScreenDepth))
	{
		NumPalette = (1 << ScreenDepth);
	}
	SetOptimizedPalette();
}
G42ScreenPalette::G42ScreenPalette(G42StandardPaletteType type,
		uintf num_palette, G42Surface surface)
:
	ScreenType(TrueColor), ScreenDepth(0), Type(type), Mode(UseStandardPalette),
	NumPalette(num_palette), Surface(surface)
{
	InvestigateScreen();
	if (NumPalette > (1 << ScreenDepth))
	{
		NumPalette = (1 << ScreenDepth);
	}
	SetOptimizedPalette();
}
void
G42ScreenPalette::InvestigateScreen(void)
{
	#ifdef MSWIN
	HDC dc = GetDC(Surface);
	int bits_per_plane = GetDeviceCaps(dc, BITSPIXEL);
	int planes = GetDeviceCaps(dc, PLANES);
	ReleaseDC(0, dc);
	int depth = bits_per_plane * planes;
	if (depth <= 1)
	{
		ScreenType = Grayscale;
		ScreenDepth = 1;
	}
	else if (depth <= 4)
	{
		ScreenType = Palette;
		ScreenDepth = 4;
	}
	else if (depth <= 8)
	{
		ScreenType = Palette;
		ScreenDepth = 8;
	}
	else
	{
		ScreenType = TrueColor;
	}
	#endif
// temporary for testing
//ScreenType = Palette;
//ScreenDepth = 8;
//ScreenType = TrueColor;
}
G42ScreenPalette::~G42ScreenPalette(void)
{
}
void
G42ScreenPalette::SetPaletteMode(G42PaletteMode mode)
{
	Mode = mode;
	SetOptimizedPalette();
}
void
G42ScreenPalette::SetStandardPaletteType(G42StandardPaletteType type,
	uintf num_palette)
{
	Type = type;
	NumPalette = num_palette;
	if (NumPalette > (1 << ScreenDepth))
	{
		NumPalette = (1 << ScreenDepth);
	}
	SetOptimizedPalette();
}
void
G42ScreenPalette::SetOptimizedPalette(void)
{
	if (ScreenType == Palette)
	{
		if (Mode == UseStandardPalette)
		{
			if (Type == Group42)
			{
				OptimizedPalette = new G42OptimizedPalette(
					G42OptimizedPalette::PaletteStandard, NumPalette);
			}
			else if (Type == Netscape)
			{
				OptimizedPalette = new G42OptimizedPalette(
					G42OptimizedPalette::PaletteNetscape, NumPalette);
			}
		}
	}
}

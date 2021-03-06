#include "gimarch.h"
#ifdef MACOS
G42Pattern	G42MarchingAnts::sAntsPattern = {0xF8, 0xF1, 0xE3, 0xC7,
										0x8F, 0x1F, 0x3E, 0x7C};
#endif
int32	G42MarchingAnts::sNextAntMarchTime = 0;
#ifdef MACOS
const int16		interval_AntMarch = 3;
#endif
#ifdef MSWIN
//const int32		interval_AntMarch = 500;
const int32		interval_AntMarch = 300;
#endif
#ifdef MSWIN
HPEN 		G42MarchingAnts::Pen = 0;
HPEN		G42MarchingAnts::OldPen = 0;
COLORREF	G42MarchingAnts::OldBkColor = 0;
int		G42MarchingAnts::OldBkMode = 0;
byte		G42MarchingAnts::Forground = 255;
byte		G42MarchingAnts::Background = 0;
#endif
bool
G42MarchingAnts::BeginMarch(void)
{
	#ifdef MACOS
	Boolean	marching = (TickCount() >= sNextAntMarchTime);
	if (marching)
		UseAntsPattern(0);
	return marching;
	#endif
	#ifdef MSWIN
	bool marching = (GetTickCount() >= sNextAntMarchTime);
//	if (marching)
//		UseAntsPattern();
	return marching;
	#endif
}
void
G42MarchingAnts::EndMarch(void)
{
	#ifdef MACOS
	sNextAntMarchTime = TickCount() + interval_AntMarch;
	unsigned char	lastRow = sAntsPattern.pat[7];
	for (Int16 i = 7; i > 0; i--)
		sAntsPattern.pat[i] = sAntsPattern.pat[i - 1];
	sAntsPattern.pat[0] = lastRow;
	#endif
	#ifdef MSWIN
	sNextAntMarchTime = GetTickCount() + interval_AntMarch;
	byte t = Forground;
	Forground = Background;
	Background = t;
	#endif
}
void
G42MarchingAnts::UseAntsPattern(G42DrawLocation location)
{
	#ifdef MACOS
	::PenNormal();
	::PenPat(&sAntsPattern);
	/*  To quiet CodeWarrior warning  */
	if (location) {}
	#endif
	#ifdef MSWIN
	Pen = CreatePen(PS_DOT, 0, RGB(Forground, Forground, Forground));
//	Pen = (HPEN)GetStockObject(WHITE_PEN);
	OldPen = (HPEN)SelectObject(location, Pen);
	SelectObject(location, GetStockObject(NULL_BRUSH));
	OldBkColor = SetBkColor(location, RGB(Background, Background, Background));
	OldBkMode = SetBkMode(location, OPAQUE);
	#endif
}
void
G42MarchingAnts::CleanupAntsPattern(G42DrawLocation location)
{
	#ifdef MSWIN
	SelectObject(location, OldPen);
	DeleteObject(Pen);
	SetBkColor(location, OldBkColor);
	SetBkMode(location, OldBkMode);
	#endif
	#ifdef MACOS
	/*  To quiet CodeWarrior warning  */
	if (location) {}
	#endif
}

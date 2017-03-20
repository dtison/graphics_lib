//	Class originally inspired by PowerPlant UMarchingAnts ©1993-1996 Metrowerks Inc.
#if !defined(GIMARCH_H)
#define GIMARCH_H
#include "gitype.h"
#ifdef MSWIN
typedef int G42Pattern;
#endif
#ifdef MACOS
typedef Pattern G42Pattern;
#endif
class G42PrepareAntsToDraw;
class	G42MarchingAnts
{
	friend class G42PrepareAntsToDraw;
public:
	G42MarchingAnts(void) // this will set up stuff that is currently static
	{
	}
	~G42MarchingAnts(void)
	{
	}
	static bool			BeginMarch(void);
	static void			EndMarch(void);
protected:
	static void			UseAntsPattern(G42DrawLocation location);
	static void			CleanupAntsPattern(G42DrawLocation location);
	#ifdef MACOS
	static G42Pattern	sAntsPattern;
	#endif
	static int32		sNextAntMarchTime;
	#ifdef MSWIN
	static HPEN			Pen;
	static HPEN			OldPen;
	static COLORREF	OldBkColor;
	static int			OldBkMode;
	static byte			Forground;
	static byte			Background;
	#endif
};
class G42PrepareAntsToDraw
{
public:
	G42PrepareAntsToDraw(G42MarchingAnts & ants, G42DrawLocation location)
	:
		Location(location), Ants(ants)
	{
		Ants.UseAntsPattern(Location);
	}
	~G42PrepareAntsToDraw(void)
	{
		Ants.CleanupAntsPattern(Location);
	}
private:
	G42MarchingAnts & Ants;
	G42DrawLocation Location;
};
#endif // GIMARCH_H

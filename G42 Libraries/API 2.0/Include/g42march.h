//	Class inspired by PowerPlant UMarchingAnts ©1993-1996 Metrowerks Inc.
class	G42MarchingAnts 
{
public:
	static Boolean		BeginMarch();
	static void			EndMarch();
	static void			UseAntsPattern();
protected:
	static Pattern	sAntsPattern;
	static Int32	sNextAntMarchTime;
};

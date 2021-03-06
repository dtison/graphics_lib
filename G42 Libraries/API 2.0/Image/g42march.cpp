#include "g42march.h"
Pattern	G42MarchingAnts::sAntsPattern = {0xF8, 0xF1, 0xE3, 0xC7,
								  	   0x8F, 0x1F, 0x3E, 0x7C};
Int32	G42MarchingAnts::sNextAntMarchTime = 0;
const Int16		interval_AntMarch = 3;
Boolean
G42MarchingAnts::BeginMarch()
{
	Boolean	marching = (TickCount() >= sNextAntMarchTime);
	if (marching) 
		UseAntsPattern();
	return marching;		
}
void
G42MarchingAnts::EndMarch()
{
	sNextAntMarchTime = TickCount() + interval_AntMarch;
	unsigned char	lastRow = sAntsPattern.pat[7];
	for (Int16 i = 7; i > 0; i--) 
		sAntsPattern.pat[i] = sAntsPattern.pat[i - 1];
	sAntsPattern.pat[0] = lastRow;
}
void
G42MarchingAnts::UseAntsPattern()
{
	::PenNormal();
	::PenPat(&sAntsPattern);
}

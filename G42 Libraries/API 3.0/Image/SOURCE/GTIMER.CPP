// gtimer.cpp - timer class
#include "gtimer.h"
#if defined(MSWIN)
static G42ExpandableArray<uintf> G42TimerIDArray;
static G42ExpandableArray<G42TimerClass *> G42TimerClassArray;
static G42ExpandableArray<HWND> G42TimerWindowArray;
static G42ExpandableArray<bool> G42TimerUsedArray;
static uintf G42TimerNumArray = 0;
G42TimerClass::G42TimerClass(void)
{
}
G42TimerClass::~G42TimerClass(void)
{
}
uintf
G42TimerClass::GetTimerID(G42Surface surface)
{
	for (uintf i = 1; i < G42TimerNumArray; i++)
	{
		if (!G42TimerUsedArray[i])
		{
			G42TimerUsedArray[i] = true;
			G42TimerWindowArray[i] = surface;
			G42TimerClassArray[i] = this;
			return i;
		}
	}
	intf index = G42TimerNumArray++;
	G42TimerUsedArray[index] = true;
	G42TimerWindowArray[index] = surface;
	G42TimerClassArray[index] = this;
	return index;
}
void
G42TimerClass::ReleaseTimerID(uintf id)
{
	G42TimerUsedArray[id] = false;
}
extern "C" VOID CALLBACK G42TimerProcedure(
	HWND /* window */ , UINT, UINT id, DWORD)
{
	G42TimerClassArray[id]->TimerStopped(id); // G42TimerIDArray[id]);
#if 0 // huh?
	for (intf i = 1; i < G42TimerNumArray; i++)
	{
		if (window == G42TimerWindowArray[i] &&
			id == G42TimerIDArray[i])
		{
			G42TimerClassArray[i]->TimerStopped(G42TimerIDArray[i]);
			break;
		}
	}
#endif
}
void
G42TimerClass::StartTimer(uintf id, uint32 msec)
{
	G42TimerIDArray[id] =
		SetTimer(G42TimerWindowArray[id], id, msec, G42TimerProcedure);
}
void
G42TimerClass::StopTimer(uintf id)
{
	KillTimer(G42TimerWindowArray[id], id); // G42TimerIDArray[id]);
}
#endif // MSWIN

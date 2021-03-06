#pragma once
#include "g42event.h"
class CommProgress;
class CommProHandler : public G42EventUpdateHandler
{
public:
	CommProHandler (void) : G42EventUpdateHandler (), DelayTicks (20L), Dlg (0)  {}
	~CommProHandler (void) {}
 
	virtual void StartPercent (void);
	virtual void SetPercent (int value);   
	virtual void StopPercent (void);
private:
	CommProgress	*Dlg;
	long 			DelayTicks;
	long			StartTickCount;
};

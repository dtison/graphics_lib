#include "g42event.h"
G42EventUpdateHandler::~G42EventUpdateHandler(void)
{
}
void
G42EventUpdateHandler::Start(void)
{
	Percent = 0;
	StartPercent();
}
void
G42EventUpdateHandler::Stop(void)
{
	if (Percent != 100)
	{
		SetPercent(100);
	}
	StopPercent();
}
void
G42EventUpdateHandler::Set(int32 value)
{
	int new_percent = ((100 * value) / Denominator);
	if (new_percent != Percent)
	{
		Percent = new_percent;
		SetPercent(Percent);
	}
}
void
G42EventUpdateHandler::StartPercent(void)
{
}
void
G42EventUpdateHandler::SetPercent(int)
{
}
void
G42EventUpdateHandler::StopPercent(void)
{
}

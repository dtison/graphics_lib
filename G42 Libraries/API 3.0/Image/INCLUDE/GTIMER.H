// gtimer.h - functions to control timers - perhaps only on MSWIN
#if !defined(GTIMER_H)
#define GTIMER_H
#include "gitype.h"
class G42TimerClass
{
#if defined(MSWIN)
public:
	G42TimerClass();
	virtual ~G42TimerClass();
	virtual void TimerStopped(uintf id) = 0;
protected:
	uintf GetTimerID(G42Surface surface);
	void ReleaseTimerID(uintf id);
	void StartTimer(uintf id, uint32 msec);
	void StopTimer(uintf id);
#endif // MSWIN
};
#endif // GTIMER_H

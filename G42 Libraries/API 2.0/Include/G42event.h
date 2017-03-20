#if !defined(G42EVENT_H_)
#define G42EVENT_H_
#include "g42itype.h"
class G42EventUpdateHandler
{
protected:
	int32		Denominator;
	int		Percent;
public:
	G42EventUpdateHandler(void)
   	: Denominator(100), Percent(0) {}
   virtual ~G42EventUpdateHandler(void);
	void SetDenominator(int32 denom = 100L)
   	{Denominator = denom;}
	virtual void Start(void);
	virtual void Stop(void);
	virtual void Set(int32 value);
	virtual void StartPercent(void);
	virtual void SetPercent(int value);
	virtual void StopPercent(void);
};
class DefaultEventHandler : public G42EventUpdateHandler
{
public:
	DefaultEventHandler(void)
   	: G42EventUpdateHandler() {}
   ~DefaultEventHandler(void) {}
   void Start(void) {}
   void Stop(void) {}
   void Set(int32 value) {};
};
#endif // G42EVENT_H_

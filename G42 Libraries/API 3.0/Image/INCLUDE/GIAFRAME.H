// giaframe.h - animated image descriptor
#if !defined(GIAFRAME_H)
#define GIAFRAME_H
#include "gpobj.h"
#include "ggeomtry.h"
class G42AnimatedImageFrame : public G42PathObject
{
public:
	enum G42Replace { None, Background, Behind };
	G42AnimatedImageFrame(void);
	G42AnimatedImageFrame(G42Point & offset, int32 delay, bool user_input,
		G42Replace replace);
	G42AnimatedImageFrame(G42AnimatedImageFrame & ref);
	G42AnimatedImageFrame & operator = (G42AnimatedImageFrame & ref);
	virtual ~G42AnimatedImageFrame(void);
	void Set(G42Point & offset, int32 delay, bool user_input,
		G42Replace replace);
	void Copy(G42AnimatedImageFrame & ref);
	const G42Point & GetOffset(void) const
		{ return Offset; }
	int32 GetDelay (void) const
		{ return Delay; }
	bool WillWaitForever(void) const
		{ return (Delay < 0); }
	bool HasDelay(void) const
		{ return (Delay > 0); }
	bool GetUserInput(void) const
		{ return UserInput; }
	G42Replace GetReplace(void) const
		{ return Replace; }
private:
	G42Point				Offset;
	int32					Delay;
	bool					UserInput;
	G42Replace			Replace;
};
#endif // GIAFRAME_H

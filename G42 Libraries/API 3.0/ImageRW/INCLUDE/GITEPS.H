// giteps.h - eps image type id
#if !defined(GITEPS_H)
#define GITEPS_H
#include "girwopt.h"
const intf G42EpsTypeId = 0x10b;
class G42ImageEpsOptions : public G42ImageReadWriteOptions
{
public:
	G42ImageEpsOptions(byte level = 2, bool eps = true, bool write = true)
	:
		G42ImageReadWriteOptions(G42EpsTypeId, false, 24, false, write),
		Level(level), Eps(eps)
			{}
	virtual ~G42ImageEpsOptions(void) {}
	virtual G42BaseOptions * MakeCopy(void)
		{return new G42ImageEpsOptions(*this);}
	byte GetLevel(void) const
		{ return Level; }
	bool IsEps(void) const
		{ return Eps; }
	void SetLevel(byte level)
		{ Level = level; }
	void SetEps(bool eps)
		{ Eps = eps; }
protected:
	G42ImageEpsOptions(G42ImageEpsOptions & ref)
	:
		G42ImageReadWriteOptions(ref),
		Level(ref.Level), Eps(ref.Eps)
			{}
	byte Level;
	bool Eps;
};
#endif // GITEPS_H

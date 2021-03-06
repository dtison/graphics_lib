/*-----------------------------------------------------------------------------
	file:			ginfbase.h
	author:		John Pazdernik
	classes:		G42BaseInfo
	date:			02/26/97
	revision:	3.0
	G42BaseInfo is a parent class for the file information classes.  It
	is used simply to allow those classes to be stored, transferred, and
	referenced by a common class name.
-----------------------------------------------------------------------------*/
#if !defined(GINFBASE_H)
#define GINFBASE_H
#include "gpobj.h"
#include "gnotify.h"
/*-----------------------------------------------------------------------------
	class G42BaseInfo
-----------------------------------------------------------------------------*/
class G42BaseInfo : public G42PathObject, public G42NotifyMessage
{
public:
	// constructor/destructor
	G42BaseInfo(void)
		: G42PathObject(), G42NotifyMessage() {}
	G42BaseInfo(G42BaseInfo & ref)
		:	G42PathObject(ref), G42NotifyMessage(ref) {}
	virtual ~G42BaseInfo(void) {}
private:
	// Hidden operator=
	G42BaseInfo & operator= (G42BaseInfo &)
		{return *this;}
};
#endif // GINFBASE_H sentry

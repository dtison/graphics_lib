#if !defined(GOPBASE_H)
#define GOPBASE_H
#include "gnotify.h"
#include "gpobj.h"
class G42BaseOptions : public G42NotifyMessage, public G42PathObject
{
public:
	G42BaseOptions(intf type_id, bool write = true)
		: G42NotifyMessage(), G42PathObject(), TypeId(type_id),
		Write(write) {}
	G42BaseOptions(G42BaseOptions & ref)
		: G42NotifyMessage(ref), G42PathObject(ref), TypeId(ref.TypeId),
		Write(ref.Write) {}
	virtual ~G42BaseOptions(void) {}
	virtual G42BaseOptions * MakeCopy(void)
		{return new G42BaseOptions(*this);} 
	intf GetTypeId(void) const
		{return TypeId;}
	bool IsWrite(void) const
		{return Write;}
protected:
	intf	TypeId;
	bool	Write;
	// Hidden operator=
	G42BaseOptions & operator=(G42BaseOptions &)
		{return *this;}
};
#endif // GOPBASE_H sentry

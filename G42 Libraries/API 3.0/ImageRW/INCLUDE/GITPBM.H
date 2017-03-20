// gitpbm.h - pbm image type id
#if !defined(GITPBM_H)
#define GITPBM_H
#include "girwopt.h"
const intf G42PbmTypeId = 0x104;
enum G42PbmType { PbmAscii, PbmBinary };
class G42ImagePbmOptions : public G42ImageReadWriteOptions
{
public:
	G42ImagePbmOptions(G42PbmType type = PbmAscii, bool write = true)
	:
		G42ImageReadWriteOptions(G42PbmTypeId, false, 24, false, write),
		PbmType(type)
			{}
	virtual ~G42ImagePbmOptions(void) {}
	virtual G42BaseOptions * MakeCopy(void)
		{return new G42ImagePbmOptions(*this);}
	G42PbmType GetPbmType(void) const
		{ return PbmType; }
	void SetPbmType(G42PbmType type)
		{ PbmType = type; }
protected:
	G42ImagePbmOptions(G42ImagePbmOptions & ref)
	:
		G42ImageReadWriteOptions(ref),
		PbmType(ref.PbmType)
			{}
	G42PbmType	PbmType;
};
#endif // GITPBM_H

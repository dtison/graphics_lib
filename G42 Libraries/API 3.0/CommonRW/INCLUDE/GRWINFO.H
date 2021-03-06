#if !defined(GRWINFO_H)
#define GRWINFO_H
#include "gexcept.h"
#include "gnotify.h"
#if 0
class G42DllLoadException : public G42Exception
{
public:
	G42DllLoadException(uintf error_id = G42_ERROR_DLL_LOAD)
			: G42Exception(error_id) {}
};
class G42DllBadFuncException : public G42Exception
{
public:
	G42DllBadFuncException(uintf error_id = G42_ERROR_DLL_BADFUNC)
			: G42Exception(error_id) {}
};
#endif
class G42UnknownTypeException : public G42Exception
{
public:
	G42UnknownTypeException(uintf error_id = G42_ERROR_UNKNOWN_TYPE)
			: G42Exception(error_id) {}
};
class G42ThirdPartyException : public G42Exception
{
public:
	G42ThirdPartyException(char * error_text = 0, uintf error_id = G42_ERROR_THIRDPARTY)
		: G42Exception(error_id, error_text) {}
};
class G42RWInfo : public G42NotifyMessage
{
public:
	enum G42MediaType { UnknownMedia = 0, Image = 1, Sound = 2};
	G42RWInfo(intf type_id, const char * def_ext, const char * name_string,
		bool dll, const char * dll_name, intf major, intf minor,
		G42MediaType media_type);
	G42RWInfo(const G42RWInfo & ref);
	virtual ~G42RWInfo(void);
	G42RWInfo & operator= (G42RWInfo & ref);
	bool IsDll(void) const
		{return Dll;}
	intf GetTypeId(void) const
		{return TypeId;}
	const char * GetDefExt(intf index = 0) const
		{if (index < 0 || index >= NumExtensions)
			throw G42OutOfBoundsException();
		return DefExt[index];}
	const intf GetNumExtensions(void) const
		{return NumExtensions;}		
	const char * GetNameString(void) const
		{return NameString;}
	const char * GetDllName(void) const
		{return DllName;}
	intf GetVersionMajor(void) const
		{return VersionMajor;}
	intf GetVersionMinor(void) const
		{return VersionMinor;}
	G42MediaType GetMediaType(void) const
		{return MediaType;}
protected:
	intf		TypeId;
	char **	DefExt;
	intf		NumExtensions;
	char *	NameString;
	bool		Dll;
	char *	DllName;
	intf		VersionMajor;
	intf		VersionMinor;
	G42MediaType	MediaType;
	void Clear(void);
	void Copy(const G42RWInfo & ref);
};
class G42ReaderInfo : public G42RWInfo
{
public:
	G42ReaderInfo(intf type_id, const char * def_ext, const char * name_string,
		bool dll, const char * dll_name, intf major, intf minor,
		G42MediaType media_type, const char * pattern, intf pattern_length,
		bool last = false);
	G42ReaderInfo(const G42ReaderInfo & ref);
	virtual ~G42ReaderInfo(void);
	G42ReaderInfo & operator= (G42ReaderInfo & ref);
	const char * GetPattern(void) const
		{return Pattern;}
	intf GetPatternLength(void) const
		{return PatternLength;}
	bool IsLast(void) const
		{return Last;}
protected:
	char *	Pattern;
	intf		PatternLength;
	bool		Last;
	void Clear(void);
	void Copy(const G42ReaderInfo & ref);
};
class G42WriterInfo : public G42RWInfo
{
public:
	G42WriterInfo(intf type_id, const char * def_ext, const char * name_string,
		bool dll, const char * dll_name, intf major, intf minor,
		G42MediaType media_type, bool options);
	G42WriterInfo(const G42WriterInfo & ref);
	virtual ~G42WriterInfo(void);
	G42WriterInfo & operator= (G42WriterInfo & ref);
	bool HasOptions(void) const
		{return Options;}
protected:
	bool		Options;
	void Clear(void);
	void Copy(const G42WriterInfo & ref);
};
#endif // GRWINFO_H sentry

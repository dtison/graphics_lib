#include "grwinfo.h"
G42RWInfo::G42RWInfo(intf type_id, const char * def_ext,
	const char * name_string, bool dll, const char * dll_name, intf major, intf minor,
	G42MediaType media_type)
	: TypeId(type_id), DefExt(0), NameString(0), Dll(dll), DllName(0),
	VersionMajor(major), VersionMinor(minor), MediaType(media_type),
	NumExtensions(0)
{
	if (def_ext && def_ext[0] != 0)
	{
		NumExtensions = 1;
		char * ext = (char *)def_ext;
		char * ptr = strchr(ext, ';');
		while (ptr)
		{
			NumExtensions++;
			ptr++;
			ptr = strchr(ptr, ';');
		}
		DefExt = new char * [NumExtensions];
		for (intf n = 0; n < NumExtensions; n++)
		{
			ptr = strchr(ext, ';');
			if (ptr)
				ptr[0] = 0;
			DefExt[n] = new char [strlen(ext) + 1];
			strcpy(DefExt[n], ext);
			ext = ptr + 1;
		}
	}
	if (name_string && name_string[0] != 0)
	{
		NameString = new char [strlen(name_string) + 1];
		strcpy(NameString, name_string);
	}
	if (dll && dll_name)
	{
		DllName = new char [strlen(dll_name) + 1];
		strcpy(DllName, dll_name);
	}
}
G42RWInfo::G42RWInfo(const G42RWInfo & ref)
	: TypeId(-1), DefExt(0), NameString(0), Dll(false), DllName(0),
	VersionMajor(0), VersionMinor(0)
{
	Copy(ref);
}
G42RWInfo::~G42RWInfo(void)
{
	Clear();
}
G42RWInfo &
G42RWInfo::operator= (G42RWInfo & ref)
{
	if (this == &ref)
		return *this;
	Clear();
	Copy(ref);
	return *this;
}
void
G42RWInfo::Clear(void)
{
	for (intf n = 0; n < NumExtensions; n++)
		delete [] DefExt[n];
	NumExtensions = 0;
	if (DefExt)
		delete [] DefExt;
	DefExt = 0;
	if (NameString)
		delete [] NameString;
	NameString = 0;
	if (DllName)
		delete [] DllName;
	DllName = 0;
	TypeId = 0;
	VersionMajor = 0;
	VersionMinor = 0;
	MediaType = UnknownMedia;
}
void
G42RWInfo::Copy(const G42RWInfo & ref)
{
	TypeId = ref.TypeId;
	Dll = ref.Dll;
	VersionMajor = ref.VersionMajor;
	VersionMinor = ref.VersionMinor;
	MediaType = ref.MediaType;
	if (ref.NumExtensions)
	{
		NumExtensions = ref.NumExtensions;
		DefExt = new char * [NumExtensions];
		for (intf n = 0; n < NumExtensions; n++)
		{
			DefExt[n] = new char [strlen(ref.DefExt[n]) + 1];
			strcpy(DefExt[n], ref.DefExt[n]);
		}
	}
	if (ref.NameString && ref.NameString[0] != 0)
	{
		NameString = new char [strlen(ref.NameString) + 1];
		strcpy(NameString, ref.NameString);
	}
	if (ref.Dll && ref.DllName)
	{
		DllName = new char [strlen(ref.DllName) + 1];
		strcpy(DllName, ref.DllName);
	}
}
G42ReaderInfo::G42ReaderInfo(intf type_id, const char * def_ext,
	const char * name_string, bool dll, const char * dll_name,
	intf major, intf minor, G42MediaType media_type,
	const char * pattern, intf pattern_length, bool last)
:
	G42RWInfo(type_id, def_ext, name_string, dll, dll_name, major, minor,
		media_type),
	Pattern(0),	PatternLength(pattern_length), Last(last)
{
	if (pattern && PatternLength)
	{
		Pattern = new char [PatternLength];
		memcpy(Pattern, pattern, PatternLength);
	}
}
G42ReaderInfo::G42ReaderInfo(const G42ReaderInfo & ref)
	: G42RWInfo(ref), Pattern(0), Last(false), PatternLength(0)
{
	Copy(ref);
}
G42ReaderInfo::~G42ReaderInfo(void)
{
	Clear();
}
G42ReaderInfo &
G42ReaderInfo::operator= (G42ReaderInfo & ref)
{
	if (this == &ref)
		return *this;
	Clear();
	G42RWInfo::Copy(ref);
	Copy(ref);
	return *this;
}
void
G42ReaderInfo::Clear(void)
{
	G42RWInfo::Clear();
	if (Pattern)
		delete [] Pattern;
	Pattern = 0;
	PatternLength = 0;
	Last = false;
}
void
G42ReaderInfo::Copy(const G42ReaderInfo & ref)
{
	PatternLength = ref.PatternLength;
	if (ref.Pattern && PatternLength)
	{
		Pattern = new char [PatternLength];
		memcpy(Pattern, ref.Pattern, PatternLength);
	}
	Last = ref.Last;
}
G42WriterInfo::G42WriterInfo(intf type_id, const char * def_ext,
	const char * name_string, bool dll, const char * dll_name,
	intf major, intf minor, G42MediaType media_type, bool options)
:
	G42RWInfo(type_id, def_ext, name_string, dll, dll_name, major, minor,
		media_type), Options(options)
{}
G42WriterInfo::G42WriterInfo(const G42WriterInfo & ref)
	: G42RWInfo(ref), Options(false)
{
	Copy(ref);
}
G42WriterInfo::~G42WriterInfo(void)
{
	Clear();
}
G42WriterInfo &
G42WriterInfo::operator= (G42WriterInfo & ref)
{
	if (this == &ref)
		return *this;
	Clear();
	G42RWInfo::Copy(ref);
	Copy(ref);
	return *this;
}
void
G42WriterInfo::Clear(void)
{
	G42RWInfo::Clear();
	Options = false;
}
void
G42WriterInfo::Copy(const G42WriterInfo & ref)
{
	Options = ref.Options;
}

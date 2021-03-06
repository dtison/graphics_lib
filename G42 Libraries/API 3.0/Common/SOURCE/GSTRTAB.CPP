// gstrtab.cpp - G42StringTable class
#include "gstrtab.h"
// do all MSWIN stuff first then MACOS stuff for easier readability
#ifdef MSWIN
G42StringTable::G42StringTable(HINSTANCE instance)
:
	Instance(instance), NeedToFreeLibrary(false), Name(0)
{
}
G42StringTable::G42StringTable(const char * dll_name)
:
	Instance(0), NeedToFreeLibrary(true), Name(0)
{
	Name = new char [strlen(dll_name) + 1];
	strcpy(Name, dll_name);
	Instance = LoadLibrary(dll_name);
//	if (!Instance)
//		throw G42Exception(0);
}
G42StringTable::G42StringTable(void)
:
	Instance(0), NeedToFreeLibrary(false), Name(0)
{
}
G42StringTable::G42StringTable(const G42StringTable & ref)
:
	Instance(0), NeedToFreeLibrary(true), Name(0)
{
	if (&ref != this)
	{
		Copy(ref);
	}
}
G42StringTable::~G42StringTable(void)
{
	Clear();
}
void
G42StringTable::Copy(const G42StringTable & ref)
{
	if (ref.NeedToFreeLibrary)
	{
		NeedToFreeLibrary = true;
		Name = new char [strlen(ref.Name) + 1];
		strcpy(Name, ref.Name);
		Instance = LoadLibrary(ref.Name);
	}
	else
	{
		Instance = ref.Instance;
		NeedToFreeLibrary = false;
	}
}
void
G42StringTable::Clear(void)
{
	if (NeedToFreeLibrary && Instance)
	{
		FreeLibrary(Instance);
	}
	Instance = 0;
	NeedToFreeLibrary = false;
	if (Name)
	{
		delete [] Name;
		Name = 0;
	}
}
uintf
G42StringTable::GetString(uintf id, char * buffer, uintf buffer_length) const
{
	return LoadString(Instance, id, buffer, buffer_length);
}
#endif
#ifdef MACOS
G42StringTable::G42StringTable() // David: I don't know what you need here
{
}
G42StringTable::~G42StringTable()
{
}
uintf
G42StringTable::GetString(uintf id, char * buffer, uintf buffer_length) const
{
	return 0;
}
G42StringTable::G42StringTable(const G42StringTable & ref) {}
#endif
G42StringTable &
G42StringTable::operator = (const G42StringTable & ref)
{
	if (&ref == this)
		return *this;
	#ifdef MSWIN	// Temporary
	Clear();
	Copy(ref);
	#endif
	return *this;
}

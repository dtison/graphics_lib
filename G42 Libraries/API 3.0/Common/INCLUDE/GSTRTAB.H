// gstrtab.h - G42StringTable class
#if !defined(GSTRTAB_H)
#define GSTRTAB_H
#include "gtype.h"
#ifdef MSWIN
#define STRICT
#include "windows.h"
#endif
#ifdef MSWIN
typedef HINSTANCE G42ResourceIdentifier;
#endif
#ifdef MACOS
typedef int G42ResourceIdentifier;
#endif
class G42StringTable
{
public:
	G42StringTable(void);
	G42StringTable(const G42StringTable & ref);
	G42StringTable & operator = (const G42StringTable & ref);
	#ifdef MSWIN
	G42StringTable(HINSTANCE instance);
	G42StringTable(const char * DllName);
	G42ResourceIdentifier GetResourceIdentifier(void) const
		{ return Instance; }
	#endif
	#ifdef MACOS
	G42StringTable(int = 0) {} // David: I don't know what you need here
	G42ResourceIdentifier GetResourceIdentifier(void) const
		{ return 0; }
	#endif
	~G42StringTable();
	uintf GetString(uintf id, char * buffer, uintf buffer_length) const;
private:
	void Copy(const G42StringTable & ref);
	void Clear(void);
	#ifdef MSWIN
	HINSTANCE Instance;
	bool NeedToFreeLibrary;
	char * Name;
	#endif
};
#endif // GSTRTTAB_H

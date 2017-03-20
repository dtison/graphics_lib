#include "gfutil.h"
/*  Returns the disk space bytes free on the specified volume  */
uint32
GetDiskSpace (void)
{
	#ifdef MSWIN
	uint32 avail;
	#ifdef __FLAT__
	DWORD bsec;
	DWORD sclus;
	DWORD cavail;
	DWORD ctotal;
	if (GetDiskFreeSpace(0, &sclus, &bsec, &cavail, &ctotal))
	{
		avail = cavail * bsec * sclus;
	}
	else
	{
		avail = 0;
	}
	#else
	struct dfree diskfree;
	getdfree(0, &diskfree);
	if (diskfree.df_sclus < (unsigned)(-1))
	{
		avail = (uint32) diskfree.df_avail *
			(uint32) diskfree.df_bsec *
			(uint32) diskfree.df_sclus;
	}
	else
	{
		avail = 0;
	}
	#endif // __FLAT__
	return avail;
	#endif // MSWIN
	#ifdef MACOS
	FSSpec spec;
	::FSMakeFSSpec (-1, 0, "\p", &spec);
	return (GetDiskSpace (G42FilePath (&spec)));
	#endif
}
uint32
GetDiskSpace (G42FilePath & file_path)
{
	#ifdef MSWIN
	uint32 avail;
	#ifdef __FLAT__
	DWORD bsec;
	DWORD sclus;
	DWORD cavail;
	DWORD ctotal;
	char * root = 0;
	if (file_path && file_path[1] == ':')
	{
		root = new char [4];
		strncpy(root, file_path, 3);
		root[3] = '\0';
	}
	else if (file_path && file_path[0] == '\\' && file_path[1] == '\\')
	{
		char * p = file_path;
		intf count = 0;
		while (*p)
		{
			if (*p++ == '\\')
			{
				count++;
				if (count == 4)
					break;
			}
		}
		count = p - (char *)file_path;
		root = new char [count + 1];
		strncpy(root, file_path, count);
		root[count] = '\0';
	}
	if (GetDiskFreeSpace(root, &sclus, &bsec, &cavail, &ctotal))
	{
		avail = cavail * bsec * sclus;
	}
	else
	{
		avail = 0;
	}
	if (root)
	{
		delete [] root;
	}
	#else
	struct dfree diskfree;
	int drive;
	if (file_path && *file_path >= 'A' && *file_path <= 'Z')
		drive = *file_path - 'A' + 1;
	else if (file_path && *file_path >= 'z' && *file_path <= 'z')
		drive = *file_path - 'z' + 1;
	else
		drive = 0;
	getdfree(drive, &diskfree);
	if (diskfree.df_sclus < (unsigned)(-1))
	{
		avail = (uint32) diskfree.df_avail *
			(uint32) diskfree.df_bsec *
			(uint32) diskfree.df_sclus;
	}
	else
	{
		avail = 0;
	}
	#endif // __FLAT__
	return avail;
	#endif // MSWIN
	#ifdef MACOS
	uint32 space = 0;
	FSSpec * spec = file_path;
	Str63 name;		// Placeholder only
	OSErr err = ::GetVInfo (0, name, &spec -> vRefNum, (long *) &space);
	return space;
	#endif
}
void
Unlink (const G42FilePath & file_path)
{
#if defined(MSWIN)
	if (remove (file_path) != 0)
	{
//		throw G42FileDeleteFailedException();
	}
#elif defined(MACOS)
	OSErr err = FSpDelete (file_path);
	if (err)
	{
		throw G42FileDeleteFailedException();
	}
#else
	// NEED TODO this
	throw (G42Exception());
#endif
}

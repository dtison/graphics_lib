// GFILE.CPP - G42File
#include "gfile.h"
#ifdef MSWIN
#include <sys/stat.h>
#define STRICT
#include <windows.h>
#endif
/*-----------------------------------------------------------------------------
	class G42File
-----------------------------------------------------------------------------*/
G42File::G42File (const G42FilePath & file_path, G42OpenType type, int32 flags)
:
	Handle(0)
{
#if defined(MSWIN)
	if ((type & OpenWrite) && (flags & OpenAlways) != OpenAlways)
	{
		if ((flags & OpenAlways) == OpenCreateOnly)
		{
			struct stat stat_buf;
			if (!stat(file_path, &stat_buf))
				throw G42FileExistsException();
		}
		else
		{
			struct stat stat_buf;
			if (stat(file_path, &stat_buf))
				throw G42FileDoesNotExistException();
		}
	}
	switch (type)
	{
	case OpenRead:
		Handle = mmioOpen(file_path, NULL, MMIO_READ | MMIO_ALLOCBUF |
			MMIO_EXCLUSIVE);
		break;
	case OpenWrite:
		Handle = mmioOpen(file_path, NULL, MMIO_CREATE | MMIO_WRITE |
			MMIO_ALLOCBUF | MMIO_EXCLUSIVE);
		break;
	case OpenWriteUpdate:
		Handle = mmioOpen(file_path, NULL, MMIO_CREATE | MMIO_READWRITE |
			MMIO_ALLOCBUF | MMIO_EXCLUSIVE);
		break;
	default:
		throw G42FileOpenFailedException();
	}
	if (Handle == (HMMIO)NULL)
	{
		throw G42FileOpenFailedException();
	}
#elif defined(MACOS)
	OSErr err = 0;
	if ((type & OpenWrite) && (flags & OpenAlways) != OpenAlways)
	{
		if ((flags & OpenAlways) == OpenCreateOnly)
		{
			/*  Make sure file doesn't already exist  */
			err = ::FSpOpenDF (file_path, fsCurPerm, &Handle);
			if (! err)
			{			
				::FSClose (Handle);
				throw G42FileExistsException();
			}
		}
		else
		{
			err = ::FSpOpenDF (file_path, fsCurPerm, &Handle);
			if (err) {
				throw G42FileDoesNotExistException();
			}
			else {
				::FSClose (Handle);
			}
		}
	}
	int8 permission = fsCurPerm;
	switch (type)
	{
		/*  TODO:  What about OpenReadUpdate and OpenWriteUpdate ? */
		case OpenRead:
			permission = fsRdPerm;
		break;
		case OpenWrite:
			permission = fsWrPerm;
		break;
	}
	
	G42FilePath create_path = file_path;	// Abandons const-ness
	err = FSpCreate (create_path, create_path.GetCreator(), create_path.GetType(), 0);
		
	err = ::FSpOpenDF (file_path, permission, &Handle);	
	if (err) {
		throw G42FileOpenFailedException();
	}	
#else
	char type_string[4];
	intf type_index = 0;
	if ((type & OpenWrite) && (flags & OpenAlways) != OpenAlways)
	{
		if ((flags & OpenAlways) == OpenCreateOnly)
		{
			struct stat stat_buf;
			if (!stat(file_path, &stat_buf))
				throw G42FileExistsException();
		}
		else
		{
			struct stat stat_buf;
			if (stat(file_path, &stat_buf))
				throw G42FileDoesNotExistException();
		}
	}
	if (!(type & OpenWrite))
	{
		type_string[type_index++] = 'r';
	}
	else if (type & OpenAppendFlag)
	{
		type_string[type_index++] = 'a';
	}
	else
	{
		type_string[type_index++] = 'w';
	}
	if (!(flags & OpenAscii))
	{
		type_string[type_index++] = 'b';
	}
	else
	{
		type_string[type_index++] = 't';
	}
	if (type & OpenReadUpdate)
	{
		type_string[type_index++] = '+';
	}
	type_string[type_index] = '\0';
	Handle = fopen(file_path, type_string);
	if (!Handle)
	{
		throw G42FileOpenFailedException();
	}
#endif
}
G42File::~G42File(void)
{
#if defined(MSWIN)
	if (Handle)
	{
		mmioClose(Handle, 0);
		Handle = (HMMIO)NULL;
	}
#elif defined(MACOS)
	if (Handle)
	{
		FSClose(Handle);
		Handle = 0;
	}
#else
	if (Handle)
	{
		fclose(Handle);
		Handle = 0;
	}
#endif
}
int32
G42File::GetPosition(void) const
{
#if defined(MSWIN)
	int32 pos = mmioSeek(Handle, 0L, SEEK_CUR);
	if (pos < 0)
	{
		throw G42FileSeekFailedException();
	}
	return pos;
#elif defined(MACOS)
	long pos;
	::GetFPos (Handle, &pos);
	return (int32) pos;
#else
	int32 pos = ftell(Handle);
	if (pos < 0)
	{
		throw G42FileSeekFailedException();
	}
	return pos;
#endif
}
int32
G42File::GetSize(void) const
{
#if defined(MSWIN)
	int32 old_pos = mmioSeek(Handle, 0L, SEEK_CUR);
	int32 size = mmioSeek(Handle, 0L, SEEK_END);
	if (size == -1L)
	{
		throw G42FileGetSizeFailedException();
	}
	if (mmioSeek(Handle, old_pos, SEEK_SET) == -1)
	{
		throw G42FileGetSizeFailedException();
	}
	return (size);
#elif defined(MACOS)
	long size;
	OSErr err = ::GetEOF (Handle, &size);
	if (err) 
	{
		throw G42FileGetSizeFailedException();
	}
	return (int32) size;
#else
	// NEED TODO this
	throw G42Exception();
#endif
}
int32
G42File::Read(byte * buffer, const int32 size, bool throw_on_eof)
{
#if defined(MSWIN)
	int32 read_size = mmioRead(Handle, (HPSTR)buffer, size);
	if (size != read_size)
	{
		if (throw_on_eof)
		{
			throw G42FileReadFailedException();
		}
	}
	return read_size;
#elif defined(MACOS)
	long count = size;
	OSErr err = ::FSRead (Handle, &count, buffer);
	if (err && throw_on_eof)
	{
		throw G42FileReadFailedException();
	}
	return count;
#else
	int32 read_size = fread(buffer, size, 1, Handle);
	if (size != read_size)
	{
		if (throw_on_eof)
		{
			throw G42FileReadFailedException();
		}
	}
	return read_size;
#endif
}
void
G42File::Rewind(void)
{
#if defined(MSWIN)
	if (mmioSeek(Handle, 0L, SEEK_SET))
	{
		throw G42FileSeekFailedException();
	}
#elif defined(MACOS)
	::SetFPos (Handle, fsFromStart, 0);
#else
	if (fseek(Handle, 0L, SEEK_SET))
	{
		throw G42FileSeekFailedException();
	}
#endif
}
void
G42File::SetPosition(const int32 position, const G42FromType from)
{
#if defined(MSWIN)
	int type;
	if (from == FromBeginning)
		type = SEEK_SET; // 0
	else if (from == FromEnd)
		type = SEEK_END; // 2
	else
		type = SEEK_CUR; // 1
	if (mmioSeek(Handle, position, type) == -1)
	{
		throw G42FileSeekFailedException();
	}
#elif defined(MACOS)
	short mode = fsFromStart;
	switch (from)
	{
		case FromEnd:
			mode = fsFromLEOF;
		break;
		case FromCurrent:
			mode = fsFromMark;
		break;
	}
	::SetFPos (Handle, mode,position);
#else
	int type;
	if (from == FromBeginning)
		type = SEEK_SET; // 0
	else if (from == FromEnd)
		type = SEEK_END; // 2
	else
		type = SEEK_CUR; // 1
	if (fseek(Handle, position, type))
	{
		throw G42FileSeekFailedException();
	}
#endif
}
void
G42File::Write(const byte * buffer, const int32 size)
{
#if defined(MSWIN)
	int32 wrote_size = mmioWrite(Handle, (HPSTR)buffer, size);
	if (size != wrote_size)
	{
		throw G42FileWriteFailedException();
	}
#elif defined(MACOS)
	long count = (long) size;
	OSErr err = ::FSWrite (Handle, &count, buffer);
	if (err) {
		throw G42FileWriteFailedException();
	}
#else
	int32 wrote_size = fread(buffer, 1, size, Handle);
	if (size != wrote_size)
	{
		throw G42FileWriteFailedException();
	}
#endif
}
/***  G42FilePath Member Functions ***/
#ifdef MSWIN
G42FilePath::G42FilePath (const char * file_path)
:
	FilePath(0)
{
	Set(file_path);
 }
G42FilePath::G42FilePath (const G42FilePath & ref)
:
	FilePath(0)
{
	Set(ref.FilePath);
}
G42FilePath::~G42FilePath (void)
{
	Clear();
}
G42FilePath &
G42FilePath::operator = (const G42FilePath & ref)
{
	if (&ref != this)
	{
		Set(ref.FilePath);
	}
	return *this;
}
void
G42FilePath::Clear(void)
{
	if (FilePath)
		delete [] FilePath;
	FilePath = 0;
}
void
G42FilePath::Set(const char * file_path)
{
	Clear();
	// Store away a copy of spec
	if (file_path)
	{
		FilePath = new char [strlen(file_path) + 1];
		strcpy(FilePath, file_path);
	}
}
#endif
#ifdef MACOS
G42FilePath::G42FilePath (const FSSpec * spec, OSType creator, OSType type)
: Creator (creator), FileSpec (0), Type (type)
{
	if (spec == 0)
	{
		throw (G42InvalidParameterException (0));
	}
	Set (spec);
}
G42FilePath::G42FilePath (const G42FilePath & ref)
: Creator (ref.Creator),  FileSpec (0), Type (ref.Type)
{
	Set (ref.FileSpec);
}
G42FilePath::~G42FilePath (void)
{
	if (FileSpec)
	{
		delete FileSpec;
		FileSpec = 0;
	}
}
G42FilePath &
G42FilePath::operator = (const G42FilePath & ref)
{
	if (&ref == this)
	{
		return *this;
	}
	if (FileSpec)
	{
		delete FileSpec;
		FileSpec = 0;
	}
	Set (ref.FileSpec);
	Creator 	= ref.Creator;
	Type 		= ref.Type;
	return *this;
}
void
G42FilePath::Set(const FSSpec * spec)
{
	if (spec)
	{
		/*  Store away a copy of spec  */
		FileSpec = new FSSpec;
		::BlockMove ((FSSpec *) spec, FileSpec, sizeof (FSSpec));
	}
}
#endif

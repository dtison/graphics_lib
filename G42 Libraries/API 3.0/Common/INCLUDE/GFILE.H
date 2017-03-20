/*-----------------------------------------------------------------------------
	file:			gfile.h
	author:		Guy Schalnat
	classes:		G42File
	date:			10 Oct 1996
	revision:	3.0
	G42File wraps the various file access types of Windows (mmio), Mac (FSOpen),
	and general (fopen), so we don't have to bother with #defines around
	everything.  It also typedefs a variable to handle different methods of
	representing file names. Open a file when constructing, read from or write
	to it, and close it when destructing.  The class has exceptions so you don't
	have to check return values.
-----------------------------------------------------------------------------*/
// Compiler directives
#if !defined (GFILE_H)
#define GFILE_H
#include "gtype.h"
#if defined(MSWIN)
#define STRICT
#include <windows.h>
#include <mmsystem.h>
#endif
/*-----------------------------------------------------------------------------
	This class allows Mac programmers to pass around a FSSpec *, Type and Creator
	fields instead of having to treat it all separately.  For Windows, it wraps
	a char * pointer.  Copies of pointer are made so caller doesn't have to store
	it.
-----------------------------------------------------------------------------*/
class G42FilePath
{
public:
	#ifdef MSWIN
	G42FilePath (const char * spec = 0);
	#endif
	#ifdef MACOS
	G42FilePath (void) : Creator('Gr42'), FileSpec (0), Type ('TEXT') {}
	G42FilePath (const FSSpec * spec, OSType creator = 'Gr42', OSType type = 'TEXT');
	#endif
	G42FilePath (const G42FilePath & ref);
	#ifdef MSWIN
	operator char * (void) const
		{ return FilePath; }
	#endif
	#ifdef MACOS
	operator 	FSSpec * (void) const {return FileSpec;}
	OSType		GetCreator (void) const {return Creator;}
	OSType		GetType (void) const {return Type;}
	#endif
	~G42FilePath (void);
	G42FilePath & operator = (const G42FilePath & ref);
private:
	#ifdef MSWIN
	char 			* FilePath;
	#endif
	#ifdef MACOS
	FSSpec 		* FileSpec;
	OSType		Creator;
	OSType		Type;
	#endif
	#ifdef MSWIN
	void Set(const char * file_path);
	void Clear(void);
	#endif
	#ifdef MACOS
	void			Set (const FSSpec *);
	#endif
};
// exceptions thrown in G42file
class G42FileException : public G42Exception
{
public:
	G42FileException(uintf error_id) : G42Exception(error_id) {};
};
class G42FileExistsException : public G42FileException
{
public:
	G42FileExistsException(uintf error_id = G42_ERROR_FILE_EXISTS) :
		G42FileException(error_id) {};
};
class G42FileDoesNotExistException : public G42FileException
{
public:
	G42FileDoesNotExistException(uintf error_id = G42_ERROR_FILE_DOES_NOT_EXIST) :
		G42FileException(error_id) {};
};
class G42FileOpenFailedException : public G42FileException
{
public:
	G42FileOpenFailedException(uintf error_id = G42_ERROR_FILE_OPEN_FAILED) :
		G42FileException(error_id) {};
};
class G42FileReadFailedException : public G42FileException
{
public:
	G42FileReadFailedException(uintf error_id = G42_ERROR_FILE_READ_FAILED) :
		G42FileException(error_id) {};
};
class G42FileWriteFailedException : public G42FileException
{
public:
	G42FileWriteFailedException(uintf error_id = G42_ERROR_FILE_WRITE_FAILED) :
		G42FileException(error_id) {};
};
class G42FileCloseFailedException : public G42FileException
{
public:
	G42FileCloseFailedException(uintf error_id = G42_ERROR_FILE_CLOSE_FAILED) :
		G42FileException(error_id) {};
};
class G42FileSeekFailedException : public G42FileException
{
public:
	G42FileSeekFailedException(uintf error_id = G42_ERROR_FILE_SEEK_FAILED) :
		G42FileException(error_id) {};
};
class G42FileDeleteFailedException : public G42FileException
{
public:
	G42FileDeleteFailedException(uintf error_id = G42_ERROR_FILE_DELETE_FAILED) :
		G42FileException(error_id) {};
};
class G42FileGetSizeFailedException : public G42FileException
{
public:
	G42FileGetSizeFailedException(uintf error_id = G42_ERROR_FILE_GETSIZE_FAILED) :
		G42FileException(error_id) {};
};
class G42FileGetDiskSpaceFailedException : public G42FileException
{
public:
	G42FileGetDiskSpaceFailedException(
		uintf error_id = G42_ERROR_FILE_DISKSP_FAILED) :
		G42FileException(error_id) {};
};
/*-----------------------------------------------------------------------------
	class G42File
-----------------------------------------------------------------------------*/
class G42File
{
public:
	// enumerated data types
	enum  G42OpenFlags                  // more to be added later, when needed
			{OpenCreateOnly = 1, OpenExistingOnly = 2, OpenAlways = 3,
			 OpenAscii = 4};
	enum  G42OpenType
			{OpenRead = 0, OpenWrite = 1, OpenReadUpdate = 2,
			 OpenWriteUpdate = 3, OpenAppendFlag = 4,
			 OpenAppend = 5, OpenReadAppend = 7};
	enum  G42FromType
			{FromBeginning, FromCurrent, FromEnd};
	// constructor/destructor
	G42File (const G42FilePath & file_path, G42OpenType type = OpenRead,
		int32 OpenFlags = OpenAlways);
	~G42File(void);
	int32 GetPosition(void) const;
	int32 GetSize(void) const;
	bool	IsMMIO(void) const;
	// member functions
	int32 Read(byte * buffer, const int32 size, bool throw_on_eof = true);
	void Rewind(void);
	void SetPosition(const int32 position, const G42FromType from = FromBeginning);
	void Write(const byte * buffer, const int32 size);
#if defined(MACOS)
	short GetHandle(void) const
#elif defined(MSWIN)
	HMMIO GetHandle(void) const
#else
	FILE * GetHandle(void) const
#endif
		{return Handle;}
		
protected:
	// data members
#if defined(MACOS)
	short    Handle;
#elif defined(MSWIN)
	HMMIO    Handle;
#else
	FILE *   Handle;
#endif
private:
	// hidden copy constructor and operator=
	G42File(G42File &) {}
	G42File & operator = (G42File &)
			{return *this;}
};
inline bool
G42File::IsMMIO(void) const
{
#if defined(MACOS)
	return false;
#elif defined(MSWIN)
	return true;
#else
	return false;
#endif
}
#endif // GFILE_H

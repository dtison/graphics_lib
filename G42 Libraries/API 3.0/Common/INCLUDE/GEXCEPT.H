/*-----------------------------------------------------------------------------
	file:			gexcept.h
	author:		Guy Schalnat and John Pazdernik
	classes:		G42BaseException, G42Exception, G42FatalException, et. al.
	date:			10/09/96
	revision:	3.0
	These classes are the base classes for all exceptions thrown by this
	library. There are two general classifications, G42Exception and
	G42FatalException. The first is a recoverable exception, while the second
	indicates a major problem where termination is recommended.  These are base
	classes from which individualized exceptions may be created.  Also, a small
	number of specific exceptions which are used frequently in the common
	library are defined.
-----------------------------------------------------------------------------*/
// Compiler Directives
#if !defined(GEXCEPT_H)
#define GEXCEPT_H
#include "gtype.h"
#ifdef __TURBOC__
#include <except.h>
#endif
#ifdef MSWIN
#include <gexcept.rh>
#endif
#ifdef MACOS
#include <gexceptm.rh>
#endif
/*-----------------------------------------------------------------------------
-----------------------------------------------------------------------------*/
#if defined(G42_NEW_DOESNT_THROW)
	#if defined(G42_NEED_XALLOC)
		class xalloc {};		// dummy class, if necessary
	#endif
	inline void G42ThrowIfNull(void * ptr)
	{
		if (!ptr)
			throw xalloc();
	}
#else
	inline void G42ThrowIfNull(void *)
	{
	}
#endif // G42_NEW_DOESNT_THROW
/*-----------------------------------------------------------------------------
	class G42BaseException
-----------------------------------------------------------------------------*/
class G42BaseException
{
public:
	// constructors/destructors
	G42BaseException(uintf error_id, char * error_text = 0)
		: ErrorId(error_id), ErrorText(error_text)
		{}
	G42BaseException(G42BaseException & ref)
		: ErrorId(ref.ErrorId), ErrorText(0)
		{
			if (ref.ErrorText)
			{
				ErrorText = new char [strlen(ref.ErrorText) + 1];
				strcpy(ErrorText, ref.ErrorText);
			}
		}
	virtual ~G42BaseException(void)
		{
			if (ErrorText)
				delete [] ErrorText;
			ErrorText = 0;
		}
	// Access functions
	char * GetErrorText(void) const
			{return ErrorText;}
	char * TakeErrorText(void)
			{char * tmp = ErrorText; ErrorText = 0; return tmp;}
   int32 GetId(void) const
			{return ErrorId;}
private:
	// Data members
	uintf		ErrorId;		// Resource id for associated string table message.
	char *	ErrorText;  // Buffer for holding additional text.
};
/*-----------------------------------------------------------------------------
	class G42Exception
-----------------------------------------------------------------------------*/
class G42Exception : public G42BaseException
{
public:
	G42Exception(intf error_id = G42_ERROR_INTERNAL, char * error_text = 0)
			: G42BaseException(error_id, error_text) {}
};
/*-----------------------------------------------------------------------------
	class G42FatalException
-----------------------------------------------------------------------------*/
class G42FatalException : public G42BaseException
{
public:
	G42FatalException(uintf error_id = G42_ERROR_FATAL, char * error_text = 0)
			: G42BaseException(error_id, error_text) {}
};
/*-----------------------------------------------------------------------------
	other G42 exception classes
-----------------------------------------------------------------------------*/
class G42InternalException : public G42FatalException
{
public:
	G42InternalException(char * err_text = 0, uintf error_id = G42_ERROR_INTERNAL)
         : G42FatalException(error_id, err_text) {}
};
class G42InvalidParameterException : public G42Exception
{
public:
	G42InvalidParameterException(uintf error_id = G42_ERROR_INVALID_PARAMETE)
         : G42Exception(error_id) {}
};
class G42LockException : public G42Exception
{
public:
	G42LockException(uintf error_id = G42_ERROR_LOCK)
			: G42Exception(error_id) {}
};
class G42MemoryException : public G42Exception
{
public:
	G42MemoryException(uintf error_id = G42_ERROR_MEMORY)
			: G42Exception(error_id) {}
};
class G42MMSystemException : public G42Exception
{
public:
	G42MMSystemException(char * error_text = 0, uintf error_id = G42_ERROR_MMSYSTEM)
		: G42Exception(error_id, error_text) {}
};
class G42NullDereferenceException : public G42Exception
{
public:
	G42NullDereferenceException(uintf error_id = G42_ERROR_NULL_DEREF)
			: G42Exception(error_id) {}
};
class G42OutOfBoundsException : public G42Exception
{
public:
	G42OutOfBoundsException(uintf error_id = G42_ERROR_OUT_OF_BOUNDS)
			: G42Exception(error_id) {}
};
/*-----------------------------------------------------------------------------
	class G42UserCancelException
-----------------------------------------------------------------------------*/
class G42UserCancelException : public G42Exception
{
public:
	// constructor
	G42UserCancelException(uintf error_id = G42_ERROR_USER_CANCEL)
		: G42Exception(error_id) {}
};
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
#endif // GEXCEPT_H sentry

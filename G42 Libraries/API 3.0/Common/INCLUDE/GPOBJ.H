/*-----------------------------------------------------------------------------
	file:       gpobj.h
	author:		Guy Schalnat
	classes:		G42PathObject, G42PathBegin, G42PathEnd and exceptions
	date:			10/09/96
	revision:	3.0
	For general information about paths, see PATH.TXT.  For general information
	about exceptions, see GEXCEPT.CPP.
	The base path object is an empty class.  It's only purpose is to provide a
	common pointer for passing objects around.  Use RTTI to determine which
	derived type the object is.  Begin and End path objects are merely messages.
	They don't need any information.
-----------------------------------------------------------------------------*/
// Compiler directives
#if !defined(GPOBJ_H)
#define GPOBJ_H
#include "gtype.h"
/*-----------------------------------------------------------------------------
	class G42PathException
	Usage:
		A general base class for path exceptions and will probably never be used
		by itself.
-----------------------------------------------------------------------------*/
class G42PathException : public G42Exception
{
public:
	G42PathException(int32 error_id)
		: G42Exception(error_id) {}
};
/*-----------------------------------------------------------------------------
	class G42PathNoHeadException
	Usage:
		Thrown when the user asks the path to start and hasn't told the path
		about the heads.
-----------------------------------------------------------------------------*/
class G42PathNoHeadException : public G42PathException
{
public:
	G42PathNoHeadException(int32 error_id = G42_ERROR_PATH_NO_HEAD)
		: G42PathException(error_id) {}
};
/*-----------------------------------------------------------------------------
	class G42PathNotHeadException
	Usage:
		Thrown when the user asks a component to be a head when the component
		doesn't have that ability.
-----------------------------------------------------------------------------*/
class G42PathNotHeadException : public G42PathException
{
public:
	G42PathNotHeadException(int32 error_id = G42_ERROR_PATH_NOT_HEAD)
		: G42PathException(error_id) {}
};
/*-----------------------------------------------------------------------------
	class G42PathOnlyHeadException
	Usage:
		Thrown when a component is asked to be somewhere beside the head of a
		path and the component doesn't have that ability.
-----------------------------------------------------------------------------*/
class G42PathOnlyHeadException : public G42PathException
{
public:
	G42PathOnlyHeadException(int32 error_id = G42_ERROR_PATH_ONLY_HEAD)
		: G42PathException(error_id) {}
};
/*-----------------------------------------------------------------------------
	class G42PathMultiplePathsException
	Usage:
		Thrown when the user sets up two receivers for a component.
-----------------------------------------------------------------------------*/
class G42PathMultiplePathsException : public G42PathException
{
public:
	G42PathMultiplePathsException(int32 error_id = G42_ERROR_PATH_MULTI_REC)
		: G42PathException(error_id) {}
};
/*-----------------------------------------------------------------------------
	class G42PathMultipleReceiverException
	Usage:
		Thrown when the user sets up two receivers for a component.
-----------------------------------------------------------------------------*/
class G42PathMultipleReceiverException : public G42PathException
{
public:
	G42PathMultipleReceiverException(int32 error_id = G42_ERROR_PATH_MULTI_REC)
		: G42PathException(error_id) {}
};
/*-----------------------------------------------------------------------------
	class G42PathMultipleSenderException
	Usage:
		Thrown when the user sets up two senders for a component.
-----------------------------------------------------------------------------*/
class G42PathMultipleSenderException : public G42PathException
{
public:
	G42PathMultipleSenderException(int32 error_id = G42_ERROR_PATH_MULTI_SEND)
		: G42PathException(error_id) {}
};
/*-----------------------------------------------------------------------------
	class G42PathNotAReceiverException
	Usage:
		Thrown when a component is asked to be a receiver and cannot be one.
-----------------------------------------------------------------------------*/
class G42PathNotAReceiverException : public G42PathException
{
public:
	G42PathNotAReceiverException(int32 error_id = G42_ERROR_PATH_NOT_REC)
		: G42PathException(error_id) {}
};
/*-----------------------------------------------------------------------------
	class G42PathNoReceiverException
	Usage:
		Thrown when a component requires a Receiver and has not been given one.
-----------------------------------------------------------------------------*/
class G42PathNoReceiverException : public G42PathException
{
public:
	G42PathNoReceiverException(int32 error_id = G42_ERROR_PATH_NOT_REC)
		: G42PathException(error_id) {}
};
/*-----------------------------------------------------------------------------
	class G42PathNotASenderException
	Usage:
		Thrown when a component is asked to be a sender and cannot be one.
-----------------------------------------------------------------------------*/
class G42PathNotASenderException : public G42PathException
{
public:
	G42PathNotASenderException(int32 error_id = G42_ERROR_PATH_NOT_SEND)
		: G42PathException(error_id) {}
};
/*-----------------------------------------------------------------------------
	class G42PathObject
	Usage:
		The base path object.  It provides a base class for a user to build a
		pointer to.  Note the virtual destructor.
-----------------------------------------------------------------------------*/
class G42PathObject
{
public:
	// constructor/destructor
	G42PathObject(void) {}
	virtual ~G42PathObject(void) {}
protected:
	// Hidden copy constructor and operator=
	G42PathObject(G42PathObject &) {}
	G42PathObject & operator=(G42PathObject &)
		{return *this;}
};
/*-----------------------------------------------------------------------------
	class G42PathEnd
	Usage:
		The end of a path.  Components should zero out their Sender and Receiver
		pointers when they get this object (after sending it on to their Receiver
		if there is one).
-----------------------------------------------------------------------------*/
class G42PathEnd : public G42PathObject
{
public:
	// constructor/destructor
	G42PathEnd(void) {}
	virtual ~G42PathEnd(void) {}
};
/*-----------------------------------------------------------------------------
	class G42PathEnd
	Usage:
		The beginning of a path.  Components should check to be sure that they
		are set up correctly when they get this object, and throw any exceptions
		that are appropriate.  They should also do any prep necessary to receive
		objects.
-----------------------------------------------------------------------------*/
class G42PathBegin : public G42PathObject
{
public:
	// constructor/destructor
	G42PathBegin(void) {}
	virtual ~G42PathBegin(void) {}
};
#endif // GPOBJ_H

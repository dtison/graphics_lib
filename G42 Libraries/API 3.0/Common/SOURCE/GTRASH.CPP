// GTRASH.CPP - G42Trash derived from G42PathComponent
/*-----------------------------------------------------------------------------
	Modifications:
-----------------------------------------------------------------------------*/
#include "gtrash.h"
/*-----------------------------------------------------------------------------
	G42Trash member functions:
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
	Function - G42Trash constructor
	Description:
		Default constructor, no need to do anything else.
-----------------------------------------------------------------------------*/
G42Trash::G42Trash(void) throw()
{
}
/*-----------------------------------------------------------------------------
	Function - G42Trash deconstructor
	Description:
		Nothing to delete.
-----------------------------------------------------------------------------*/
G42Trash::~G42Trash(void) throw()
{
}
/*-----------------------------------------------------------------------------
	Function - Process
	Parameters:
		G42PathObject * object - the object that needs trashed
	Description:
		Delete the object and clear the Sender on G42PathEnd.  Assume the object
		is not null, that it is now owned by us, and that the G42PathEnd comes
		last.
-----------------------------------------------------------------------------*/
void
G42Trash::Process(G42PathObject * object, G42PathComponent *)
{
	if (typeid(*object) == typeid(G42PathEnd))
		Sender = 0;
	delete object;
}
/*-----------------------------------------------------------------------------
	Function - SetHead
	Exceptions:
		G42PathNotHeadException
	Description:
		This can't produce data, so it tells the user before the path starts.
-----------------------------------------------------------------------------*/
void
G42Trash::SetHead(void)
{
	throw G42PathNotHeadException();
}
/*-----------------------------------------------------------------------------
	Function - SetReceiver
	Parameters:
		G42PathComponent * receiver - ignored
	Exceptions:
		G42PathNotAReceiverException
	Description:
		This can't send data anywhere, so it tells the user before the
		path starts.
-----------------------------------------------------------------------------*/
void
G42Trash::SetReceiver(G42PathComponent *)
{
	throw G42PathNotAReceiverException();
}
/*-----------------------------------------------------------------------------
	Function - ReplaceReceiver
	Parameters:
		G42PathComponent * old_receiver - ignored
		G42PathComponent * new_receiver - ignored
	Exceptions:
		G42PathNotAReceiverException
	Description:
		This can't send data anywhere, so it throws the exception immediately
-----------------------------------------------------------------------------*/
void
G42Trash::ReplaceReceiver(G42PathComponent *, G42PathComponent *)
{
	throw G42PathNotAReceiverException();
}

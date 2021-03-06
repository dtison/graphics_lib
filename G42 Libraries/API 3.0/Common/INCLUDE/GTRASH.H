/*-----------------------------------------------------------------------------
	file:			gtrash.h
	author:		Guy Schalnat
	classes:		G42Trash
	date:			10/10/96
	revision:	3.0
	This class merely deletes any object passed to it.  It is marginally
	useful, but might become important in certain situations, and tends to be
	useful for testing.  It does correctly throw exceptions when it is
	suppossed to.
-----------------------------------------------------------------------------*/
// Compiler directives
#if !defined(GTRASH_H)
#define GTRASH_H
#include "gpcomp.h"
/*-----------------------------------------------------------------------------
	class G42Trash : public G42PathComponent
-----------------------------------------------------------------------------*/
class G42Trash : public G42PathComponent
{
public:
	// constructor/destructor
	G42Trash(void) throw();
	virtual ~G42Trash(void) throw();
	// member functions
	virtual void Process(G42PathObject * object, G42PathComponent * sender); // throw()
	virtual void SetHead(void); // throw(G42PathNotHeadException);
	virtual void SetReceiver(G42PathComponent * receiver);
			// throw(G42PathNotASenderException);
	virtual void ReplaceReceiver(G42PathComponent * old_receiver,
		G42PathComponent * new_receiver);
			// throw(G42PathNotASenderException);
};
#endif // GTRASH_H

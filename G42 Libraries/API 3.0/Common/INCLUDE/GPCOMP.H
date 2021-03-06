/*-----------------------------------------------------------------------------
	file:			gpcomp.h
	author:		Guy Schalnat
	classes:		G42PathComponent
	date:			10/09/96
	revision:	3.0
	This class is the base class for all path components.  For a more complete
	description of how to use this class, see PATH.TXT.  The base class handles
	most of these things automatically, and throws exceptions on Process() and
	BeginProcessing() if you don't override it.  Most programs will override
	whichever is approprate and ignore the rest.  If a component can't be a
	head or take a sender or receiver, it would be nice to override the
	appropriate functions and throw exceptions then.  If you don't, you must
	throw the exception when you get the begin path.  Also, if you support
	multiple Senders and Receivers, you will have to override most of the
	functions to handle them correctly.
-----------------------------------------------------------------------------*/
// Compiler directives
#if !defined(GPCOMP_H)
#define GPCOMP_H
#include "gpobj.h"
class G42Path;
/*-----------------------------------------------------------------------------
	class G42PathComponent
-----------------------------------------------------------------------------*/
class G42PathComponent
{
public:
	// constructor/destructor
	G42PathComponent(void);
	virtual ~G42PathComponent(void);
	// member functions
	virtual void BeginProcessing(void); // throw(G42PathNotAHeadException)
	virtual void ClearPath(bool path_excepted = false);
	virtual void Process(G42PathObject * object, G42PathComponent * /* sender */ = 0);
			// throw(G42PathOnlyHeadException)
	virtual void ReceiverDestructing(G42PathComponent * receiver); // throw()
	virtual void ReplaceSender(G42PathComponent * old_sender,
		G42PathComponent * new_sender);
			// throw(G42InvalidParameterException) (and check derived class)
	virtual void ReplaceReceiver(G42PathComponent * old_receiver,
		G42PathComponent * new_receiver);
			// throw(G42InvalidParameterException) (and check derived class)
	virtual void SenderDestructing(G42PathComponent * sender); // throw()
	virtual void SetHead(void); // throw() (but check derived class)
	virtual void SetReceiver(G42PathComponent * receiver);
			// throw(G42InvalidParameterException, G42PathMultipleReceiverException)
	virtual void SetSender(G42PathComponent * sender);
			// throw(G42InvalidParameterException, G42PathMultipleSenderException)
	virtual void SetPath(G42Path * path);
			// throw(G42InvalidParameterException, G42PathMultiplePathException)
protected:
	// data members
	bool						IsHead; 		// true iff head of a path
	G42PathComponent *	Receiver; 	// to whom path objects are sent
	G42PathComponent *	Sender; 		// from whom path objects arrive
	G42Path *				Path;			// path the component is linked into
	void AddComponentToPath(G42PathComponent * component);
private:
	// hidden copy constructor and operator=
	G42PathComponent(G42PathComponent &) {}
	G42PathComponent & operator = (G42PathComponent &)
			{return *this;}
};
#endif // GPCOMP_H

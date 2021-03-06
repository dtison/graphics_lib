// GPCOMP.CPP - G42PathComponent
/*-----------------------------------------------------------------------------
	Modifications:
-----------------------------------------------------------------------------*/
#include "gpcomp.h"
#include "gpath.h"
/*-----------------------------------------------------------------------------
	G42PathComponent member functions:
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
	Function - G42PathComponent constructor
	Description:
		Default constructor, simply clears everthing to an initial state.
-----------------------------------------------------------------------------*/
G42PathComponent::G42PathComponent(void)
	: Sender(0), Receiver(0), IsHead(false), Path(0)
{
}
/*-----------------------------------------------------------------------------
	Function - G42PathComponent destructor
	Description:
		Tells the sender and receiver (if any) that it is deleting.
-----------------------------------------------------------------------------*/
G42PathComponent::~G42PathComponent(void)
{
	if (Sender)
		Sender->ReceiverDestructing(this);
	Sender = 0;
	if (Receiver)
		Receiver->SenderDestructing(this);
	Receiver = 0;
	if (Path)
		Path->ComponentUnlinking(this);
	Path = 0;
	IsHead = false;
}
/*-----------------------------------------------------------------------------
	Function - BeginProcessing
	Exceptions:
		G42PathNotHeadException
	Description:
		If the derived class doesn't override this, it can't handle being a head
		of a path, so this base class throws an exception.
-----------------------------------------------------------------------------*/
void
G42PathComponent::BeginProcessing(void)
{
	throw G42PathNotHeadException();
}
/*-----------------------------------------------------------------------------
	Function - ClearPath
	Description:
		Tells the sender and receiver (if any) that it is deleting, and removes
		the flag that says it is a head of the path.
-----------------------------------------------------------------------------*/
void
G42PathComponent::ClearPath(bool)
{
	if (Sender)
		Sender->ReceiverDestructing(this);
	Sender = 0;
	if (Receiver)
		Receiver->SenderDestructing(this);
	Receiver = 0;
	if (Path)
		Path->ComponentUnlinking(this);
	Path = 0;
	IsHead = false;
}
/*-----------------------------------------------------------------------------
	Function - Process
	Parameters:
		G42PathObject * object - the path object to process
	Exceptions:
		G42PathOnlyHeadException
	Description:
		If the derived class doesn't override this, it can't handle receiving
		path objects, so this base class throws an exception.  Assume that the
		path object is valid, and that the path objects arrive in a valid order.
		Assume that the path object is now owned by this component, and it can
		be modified or deleted at will.  Assume that the first object will always
		be a G42PathBegin, and the last object will always be a G42PathEnd.
		Don't assume that all objects passed in will be of the type this object
		is made to deal with.
-----------------------------------------------------------------------------*/
#ifdef MSWIN
#pragma argsused
#endif
void
G42PathComponent::Process(G42PathObject * object, G42PathComponent * /* sender */)
{
	delete object;
	throw G42PathOnlyHeadException();
}
/*-----------------------------------------------------------------------------
	Function - ReceiverDestructing
	Parameters:
		G42PathComponent * receiver
	Description:
		Zeros out the Receiver variable.  Assume that there is a receiver that
		matches the address given.
-----------------------------------------------------------------------------*/
void
G42PathComponent::ReceiverDestructing(G42PathComponent *)
{
	Receiver = 0;
}
/*-----------------------------------------------------------------------------
	Function - ReplaceReceiver
	Parameters:
		G42PathComponent * old_receiver - the old receiver to be replaced
		G42PathComponent * new_receiver - the new receiver that will now be used
	Exceptions:
		G42InvalidParameterException
	Description:
		Checks against the old receiver to make sure it is correct, then sets
		the receiver to the new receiver.  The new receiver must not be null.
		Derived classes may override this and throw an exception if they can't
		handle receivers.  This assumes that the old and new receivers will
		be told about the change in senders.  This is really only for internal
		use, external users should use ClearPath and AddLink from G42Path.
-----------------------------------------------------------------------------*/
void
G42PathComponent::ReplaceReceiver(G42PathComponent * old_receiver,
	G42PathComponent * new_receiver)
{
	if (old_receiver != Receiver || !new_receiver)
		throw G42InvalidParameterException();
	Receiver = new_receiver;
}
/*-----------------------------------------------------------------------------
	Function - ReplaceSender
	Parameters:
		G42PathComponent * old_sender - the old sender to be replaced
		G42PathComponent * new_sender - the new sender that will now be used
	Exceptions:
		G42InvalidParameterException
	Description:
		Checks against the old sender to make sure it is correct, then sets
		the sender to the new sender.  The new sender must not be null.
		Derived classes may override this and throw an exception if they can't
		handle senders.  This assumes that the old and new receivers will
		be told about the change in senders.  This is really only for internal
		use, external users should use ClearPath and AddLink from G42Path.
-----------------------------------------------------------------------------*/
void
G42PathComponent::ReplaceSender(G42PathComponent * old_sender,
	G42PathComponent * new_sender)
{
	if (old_sender != Sender || !new_sender)
		throw G42InvalidParameterException();
	Sender = new_sender;
}
/*-----------------------------------------------------------------------------
	Function - SenderDestructing
	Parameters:
		G42PathComponent * sender
	Description:
		Zeros out the Sender variable.  Assume that there is a sender that
		matches the address given.
-----------------------------------------------------------------------------*/
void
G42PathComponent::SenderDestructing(G42PathComponent *)
{
	Sender = 0;
}
/*-----------------------------------------------------------------------------
	Function - SetHead
	Description:
		Marks the component as a head of a path.
-----------------------------------------------------------------------------*/
void
G42PathComponent::SetHead(void)
{
	IsHead = true;
}
/*-----------------------------------------------------------------------------
	Function - SetReceiver
	Parameters:
		G42PathComponent * receiver - who is receiving things from this component
	Exceptions:
		G42InvalidParameterException,
		G42PathMultipleReceiverException
	Description:
		Sets the Receiver variable, and throws exceptions if it already has a
		receiver, if it is receiving from itself, or if the receiver being
		passed in is zero.
-----------------------------------------------------------------------------*/
void
G42PathComponent::SetReceiver(G42PathComponent * receiver)
{
	if (!receiver || receiver == this)
		throw G42InvalidParameterException();
	if (Receiver)
		throw G42PathMultipleReceiverException();
	Receiver = receiver;
}
/*-----------------------------------------------------------------------------
	Function - SetSender
	Parameters:
		G42PathComponent * sender - who is sending things to this component
	Exceptions:
		G42InvalidParameterException,
		G42PathMultipleSenderException
	Description:
		Sets the Sender variable, and throws exceptions if it already has a
		sender, if it is sending to itself, or if the sender passed in is zero.
-----------------------------------------------------------------------------*/
void
G42PathComponent::SetSender(G42PathComponent * sender)
{
	if (!sender || sender == this)
		throw G42InvalidParameterException();
	if (Sender)
		throw G42PathMultipleSenderException();
	Sender = sender;
}
/*-----------------------------------------------------------------------------
	Function - SetPath
	Parameters:
		G42Path * path - path this component is linked into
	Exceptions:
		G42PathMultiplePathsException
	Description:
		Sets the Path variable, and throws exceptions if it already is in a
		different path or if the path passed in is zero
-----------------------------------------------------------------------------*/
void
G42PathComponent::SetPath(G42Path * path)
{
	if (!path)
		throw G42InvalidParameterException();
	if (Path && Path != path)
		throw G42PathMultiplePathsException();
	Path = path;
}
void
G42PathComponent::AddComponentToPath(G42PathComponent * component)
{
	Path->AddComponent(component);
}

// G42PATH.CPP
/*-----------------------------------------------------------------------------
	Modifications:
-----------------------------------------------------------------------------*/
#include "gpath.h"
/*-----------------------------------------------------------------------------
	G42Path member functions:
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
	Function - G42Path constructor
	Description:
		Default constructor, simply clears everthing to an initial state
-----------------------------------------------------------------------------*/
G42Path::G42Path(void) throw()
	: NumHead(0), NumComponents(0)
{
}
/*-----------------------------------------------------------------------------
	Function - G42Path constructor
	Parameters:
		G42PathComponent & head
	Exceptions:
		Nothing, but a components AddHead() function may throw something
		(usually G42PathNotAHeadException)
	Description:
		Clears everything then sets the head of the path.
-----------------------------------------------------------------------------*/
G42Path::G42Path(G42PathComponent & head)
	: NumHead(0), NumComponents(0)
{
	AddHead(head);
}
/*-----------------------------------------------------------------------------
	Function - G42Path destructor
	Description:
		Nothing to destruct.  If the last path has not been Start()ed, be sure
		to delete or call ClearPath() on all components.
-----------------------------------------------------------------------------*/
G42Path::~G42Path(void) throw()
{
	Clear();
}
/*-----------------------------------------------------------------------------
	Function - AddHead
	Parameters:
		G42PathComponent & head
	Exceptions:
		Nothing, but a components AddHead() function may throw something
		(usually G42PathNotAHeadException)
	Description:
		Adds this component to the list of heads.
-----------------------------------------------------------------------------*/
void
G42Path::AddHead(G42PathComponent & head)
{
	head.SetPath(this);
	Components[NumComponents++] = &head;
	Head[NumHead++] = &head;
	head.SetHead();
}
/*-----------------------------------------------------------------------------
	Function - AddLink
	Parameters:
		G42PathComponent & source - from whom the data comes
		G42PathComponent & dest - to whom the data goes
	Exceptions:
		Nothing, but components SetSender and SetReceiver may throw exceptions.
	Description:
		Tells the source and dest about each other.  Could throw an exception
		from the source or dest if the link is invalid.
-----------------------------------------------------------------------------*/
void
G42Path::AddLink(G42PathComponent & source, G42PathComponent & dest)
{
	source.SetPath(this);
	Components[NumComponents++] = &source;
	source.SetReceiver(&dest);
	dest.SetPath(this);
	Components[NumComponents++] = &dest;
	dest.SetSender(&source);
}
void
G42Path::AddComponent(G42PathComponent * component) // internal use only
{
	component->SetPath(this);
	Components[NumComponents++] = component;
}
/*-----------------------------------------------------------------------------
	Function - Clear
	Description:
		Clears the head and component array.  Each ClearPath should call
		ComponentUnlinking, but we will also just in case.
-----------------------------------------------------------------------------*/
void
G42Path::Clear(bool path_excepted)
{
	intf i;
	for (i = 0; i < NumComponents; i++)
	{
		if (Components[i])
		{
			Components[i]->ClearPath(path_excepted);
			if (Components[i])
			{
				ComponentUnlinking(Components[i]);
			}
		}
	}
	for (i = 0; i < NumHead; i++)
	{
		if (Head[i])
		{
			Head[i]->ClearPath(path_excepted);
			if (Head[i])
			{
				ComponentUnlinking(Head[i]);
			}
		}
	}
	NumComponents = 0;
	NumHead = 0;
}
/*-----------------------------------------------------------------------------
	Function - Start
	Exceptions:
		G42PathNoHeadException, plus components may throw many different
		exceptions as they handle the data flow
	Description:
		Starts the path by calling BeginProcessing() on each head component in
		the order they were added.  After the path executes, it clears the heads
		array so it can be restarted from scratch if the user desires.
-----------------------------------------------------------------------------*/
void
G42Path::Start(void)
{
	if (!NumHead)
	{
		throw G42PathNoHeadException();
	}
	try
	{
		for (intf i = 0; i < NumHead; i++)
		{
			if (Head[i])
			{
				Head[i]->BeginProcessing();
			}
		}
	}
	catch (...)
	{
		Clear(true);
		throw;
	}
}
/*-----------------------------------------------------------------------------
	Function - ComponentUnlinking
	Parameters:
		G42PathComponent * component - the component removing from the path
	Description:
		Removes any reference to the component from the Components array.
-----------------------------------------------------------------------------*/
void
G42Path::ComponentUnlinking(G42PathComponent * component)
{
	intf i;
	for (i = 0; i < NumComponents; i++)
	{
		if (Components[i] == component)
		{
			Components[i] = 0;
		}
	}
	while (NumComponents > 0 && !Components[NumComponents - 1])
	{
		NumComponents--;
	}
	for (i = 0; i < NumHead; i++)
	{
		if (Head[i] == component)
		{
			Head[i] = 0;
		}
	}
	while (NumHead > 0 && !Head[NumHead - 1])
	{
		NumHead--;
	}
}

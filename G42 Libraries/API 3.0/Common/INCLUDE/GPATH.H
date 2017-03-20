/*-----------------------------------------------------------------------------
	file:			gpath.h
	author:		Guy Schalnat
	classes:		G42Path
	date:			10/09/96
	revision:	3.0
	This classed is used to set up and execute a path.  This is just a summary
	of the class, more information can be found in PATH.TXT.  In general, you
	create all your components, then you create the path (optionally passing in
	the head of the path to the constructor). Then you call AddLink(source,
	destination) for each link between components (data flows from the source to
	the destintation).  You also give it all the heads of the path (the nodes
	that are to start the data flow).  Most of the time, a path will only have
	one head which you will pass into the constructor, but sometimes it will
	have more than one, or it will be easier to use AddHead(). When the path is
	complete, calling Start() will run the path.
-----------------------------------------------------------------------------*/
// Compiler directives
#if !defined(GPATH_H)
#define GPATH_H
#include "gpcomp.h"
#include "gtools.h"
/*-----------------------------------------------------------------------------
	class G42Path
-----------------------------------------------------------------------------*/
class G42Path
{
	friend class G42PathComponent;
public:
	// constructors/destructors
	G42Path(void) throw();
	G42Path(G42PathComponent & head);
	~G42Path(void) throw();
	// member functions
	void AddHead(G42PathComponent & head);
	void AddLink(G42PathComponent & source, G42PathComponent & dest);
	void Clear(bool path_excepted = false);
	void Start(void);
private:
	void ComponentUnlinking(G42PathComponent * component);
	void AddComponent(G42PathComponent * component); // internal use only
	// data members
	G42Vector<G42PathComponent *> Head; 	// list of heads of paths
	intf NumHead; 												// number of heads in array
	G42Vector<G42PathComponent *> Components;
	intf NumComponents;
};
#endif // GPATH_H

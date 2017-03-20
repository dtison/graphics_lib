/*-----------------------------------------------------------------------------
	file:			gdatakey.h
	author:		John Pazdernik
	classes:	 	G42DataKey, G42WritedLockedKey, G42ReadLockedKey
	date:			10/09/96
	revision:	3.0
	The G42DataKey class has a handle to a memory chunk, and is used as a base
	class for creating specific types of memory referencing objects.  These
	objects may be passed between other objects so that this memory chunk may be
	operated upon.  The G42WriteLockedKey and G42ReadLockedKey classes are used
	to lock an associated G42DataKey object.  Creating a locked object is the
	only way to obtain an actual pointer to the memory chunk, or to perform
	operations on a memory chunk that may alter the location of the chunk.
-----------------------------------------------------------------------------*/
#if !defined(GDATAKEY_H)
#define GDATAKEY_H
#include "gmem.h"
//Forward declarations
class G42WriteLockedKey;
/*-----------------------------------------------------------------------------
	class G42DataKey
-----------------------------------------------------------------------------*/
class G42DataKey
{
	friend G42WriteLockedKey;      // Provides access to protected variables
public:
	// constructors/destructors
	G42DataKey(G42HeapManager & heap, int32 handle = -1L);
	G42DataKey(G42HeapManager & heap, int32 size, bool force);
	G42DataKey(G42DataKey & ref);
	virtual ~G42DataKey(void);
	// member functions
	int32 GetHandle(void) const
			{return Handle;}
	G42HeapManager & GetHeap(void) const
			{return Heap;}
	int32 GetSize(void) const
			{return Heap.GetSize(Handle);}
	// operators
	G42DataKey & operator= (G42DataKey & ref);
protected:
	// member functions
	void Clear(void);
	void Copy(G42DataKey & ref);
	// data members
	int32					Handle;	// token for referencing a G42HeapManager entry
	G42HeapManager &	Heap;    // Reference to a G42HeapManager object
};
/*-----------------------------------------------------------------------------
	class G42WriteLockedKey
-----------------------------------------------------------------------------*/
class G42WriteLockedKey : public G42Lock
{
public:
	// constructor/destructor
	G42WriteLockedKey(G42DataKey & key);
	virtual ~G42WriteLockedKey(void);
	// operators
	operator byte * (void) const
			{return GetPtr(Key.GetHeap(), Key.GetHandle());}
protected:
	// data members
	G42DataKey & Key;		// A reference to the G42DataKey object this class locks
private:
	// Hidden Copy Contructor and operator=
	G42WriteLockedKey(G42WriteLockedKey & ref) : Key(ref.Key) {}
	G42WriteLockedKey & operator= (G42WriteLockedKey &)
			{return *this;}
};
/*-----------------------------------------------------------------------------
	class G42ReadLockedKey
-----------------------------------------------------------------------------*/
class G42ReadLockedKey : public G42Lock
{
public:
	// constructor/destructor
	G42ReadLockedKey(G42DataKey & key);
	virtual ~G42ReadLockedKey(void);
	// operators
	operator const byte * (void) const
			{return (const byte *)(GetPtr(Key.GetHeap(), Key.GetHandle()));}
protected:
	// data members
	G42DataKey & Key;		// A reference to the G42DataKey object this class locks
private:
	// Hidden Copy Constructor and operator=
	G42ReadLockedKey(G42ReadLockedKey & ref) : Key(ref.Key) {}
	G42ReadLockedKey & operator= (G42ReadLockedKey &)
			{return *this;}
};
#endif // GDATAKEY_H sentry

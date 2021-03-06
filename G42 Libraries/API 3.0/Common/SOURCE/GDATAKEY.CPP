// GDATAKEY.CPP - G42DataKey, G42WritedLockedKey, G42ReadLockedKey
/*-----------------------------------------------------------------------------
	Modifications:
-----------------------------------------------------------------------------*/
#include "gdatakey.h"
/*-----------------------------------------------------------------------------
	G42DataKey member functions:
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
	Function - Constructor for G42DataKey
	Parameters:
		G42HeapManager & heap	- A reference to the G42HeapManager object.
		int32 handle				- Defaults to -1.  This objects handle.
	Description:
		Creates a G42DataKey object.  If the handle is not -1, then a copy of
		that handle is created.  Otherwise, a new memory chunk is created, of
		the G42HeapManagers maximum size.
-----------------------------------------------------------------------------*/
G42DataKey::G42DataKey(G42HeapManager & heap, int32 handle)
	: Heap(heap), Handle(handle)
{
	if (Handle != -1L)
		Heap.Duplicate(Handle);
	else
		Handle = Heap.Allocate(Heap.GetMaxEntrySize());
}
/*-----------------------------------------------------------------------------
	Function - Constructor for G42DataKey
	Parameters:
		G42HeapManager & heap - A reference to the G42HeapManager object
		int32 size - Size for the associated memory chunk.
		bool force - true = Override G42HeapManager's MaxEntrySize.
						 false = Abide by G42HeapManager's MaxEntrySize.
	Description:
		Creates a G42DataKey object.  A new memory chunk is created, with the
		indicated size.  Note that this size may be clipped to G42HeapManager's
		MaxEntrySize unless force is true.
-----------------------------------------------------------------------------*/
G42DataKey::G42DataKey(G42HeapManager & heap, int32 size, bool force)
	: Heap(heap), Handle(-1L)
{
	Handle = Heap.Allocate(size, force);
}
/*-----------------------------------------------------------------------------
	Function - Copy Constructor for G42DataKey
	Parameters:
		G42DataKey & ref - A G42DataKey object ot copy from.
	Description:
		Creates a Copy of a G42DataKey object.  Note that memory is not copied
		at this point.  Only the reference count for the memory handle is
		incremented.
-----------------------------------------------------------------------------*/
G42DataKey::G42DataKey(G42DataKey & ref)
	: Heap(ref.Heap), Handle(ref.Handle)
{
	Heap.Duplicate(Handle);
}
/*-----------------------------------------------------------------------------
	Function - G42DataKey Destructor.
	Description:
		Destroy this object, releasing the handle.
-----------------------------------------------------------------------------*/
G42DataKey::~G42DataKey(void)
{
	try
	{
		Clear();
	}
	catch (...)
	{
		return;
	}
}
/*-----------------------------------------------------------------------------
	Function - operator=.
	Parameters:
		G42DataKey & ref - object to be made equal to.
	Returns:
		G42DataKey & - A reference to this object.
	Description:
		Make this object a copy of the reference object.
-----------------------------------------------------------------------------*/
G42DataKey &
G42DataKey::operator= (G42DataKey & ref)
{
	if (this == &ref)
		return *this;
	Copy(ref);
	return *this;
}
/*-----------------------------------------------------------------------------
	Function - Clear.
	Description:
		Clear this object and release any associated memory.
-----------------------------------------------------------------------------*/
void
G42DataKey::Clear(void)
{
	int32 handle = Handle;
	Handle = -1;
	if (handle != -1L)
		Heap.Release(handle);
}
/*-----------------------------------------------------------------------------
	Function - Copy.
	Parameters:
		G42DataKey & ref - G42DataKey reference to copy from.
	Description:
		Make this object a copy of the reference.
-----------------------------------------------------------------------------*/
void
G42DataKey::Copy(G42DataKey & ref)
{
	Clear();
	Handle = ref.Handle;
	Heap.Duplicate(Handle);
}
/*-----------------------------------------------------------------------------
	G42WriteLockedKey member functions:
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
	Function - Constructor for G42WriteLockedKey
	Parameters:
		G42DataKey & key - A reference to the G42DataKey object to lock.
	Description:
		Creates a G42WriteLockedKey object.  The associated memory is write
		locked, and then the memory is forked.  If this results in a new handle,
		the old one is unlocked and the new one is write locked.
-----------------------------------------------------------------------------*/
G42WriteLockedKey::G42WriteLockedKey(G42DataKey & key)
	: G42Lock(), Key(key)
{
	Key.GetHeap().VerifyHandle(Key.GetHandle());
	Key.GetHeap().EnsureHandle(Key.GetHandle());
	WriteLock(Key.GetHeap(), Key.GetHandle());
	int32 new_handle = Fork(Key.GetHeap(), Key.GetHandle());
	if (new_handle != Key.GetHandle())
	{
		WriteUnlock(Key.GetHeap(), Key.GetHandle());
		Key.Handle = new_handle;
		WriteLock(Key.GetHeap(), new_handle);
	}
	Key.GetHeap().InvalidateSwapHandle(new_handle);
}
/*-----------------------------------------------------------------------------
	Function - G42WriteLockedKey Destructor.
	Description:
		Destroy this object, unlocking the memory object.
-----------------------------------------------------------------------------*/
G42WriteLockedKey::~G42WriteLockedKey(void)
{
	try
	{
		WriteUnlock(Key.GetHeap(), Key.GetHandle());
	}
	catch (...)
	{
   	return;
	}
}
/*-----------------------------------------------------------------------------
	G42ReadLockedKey member functions:
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
	Function - Constructor for G42ReadLockedKey
	Parameters:
		G42DataKey & key - A reference to the G42DataKey object to lock.
	Description:
		Creates a G42ReadLockedKey object.  The associated memory is read locked.
-----------------------------------------------------------------------------*/
G42ReadLockedKey::G42ReadLockedKey(G42DataKey & key)
	: G42Lock(), Key(key)
{
	Key.GetHeap().VerifyHandle(Key.GetHandle());
	Key.GetHeap().EnsureHandle(Key.GetHandle());
	ReadLock(Key.GetHeap(), Key.GetHandle());
}
/*-----------------------------------------------------------------------------
	Function - G42ReadLockedKey Destructor.
	Description:
		Destroy this object, unlocking the memory object.
-----------------------------------------------------------------------------*/
G42ReadLockedKey::~G42ReadLockedKey(void)
{
	try
	{
		ReadUnlock(Key.GetHeap(), Key.GetHandle());
	}
	catch (...)
	{
		return;
	}
}

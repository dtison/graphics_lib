// GMEM.CPP - G42Lock, G42HeapManager, G42HeapEntry
/*-----------------------------------------------------------------------------
	Modifications:
-----------------------------------------------------------------------------*/
#include "gmem.h"
/*-----------------------------------------------------------------------------
	G42HeapManager::G42HeapEntry member functions:
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
	Function - G42HeapEntry Constructor.
	Description:
		Create a blank entry.
-----------------------------------------------------------------------------*/
G42HeapManager::G42HeapEntry::G42HeapEntry(void)
	: Lock(0L), Ptr(0), RefCount(0L), Size(0L), SwapHandle(0), SwappedOut(false)
{}
/*-----------------------------------------------------------------------------
	Function - G42HeapEntry Destructor.
	Description:
		Destroy this entry.
-----------------------------------------------------------------------------*/
G42HeapManager::G42HeapEntry::~G42HeapEntry(void)
{
	Clear();
}
/*-----------------------------------------------------------------------------
	Function - Allocate
	Parameters:
		int32 size - Size in bytes of new memory chunk.
	Exceptions:
		G42MemoryException - This entry has already been allocated, or the memory
		allocation has failed.
	Description:
		Assigns a zero initialized memory chunk to this entry.
-----------------------------------------------------------------------------*/
void
G42HeapManager::G42HeapEntry::Allocate(int32 size)
{
	if (Size)
		throw G42MemoryException();
	Ptr = 0;
	try
	{
		Ptr = new byte [size];
		G42ThrowIfNull(Ptr);
	}
	catch (xalloc)
	{
		throw G42MemoryException();
	}
	Size = size;
	memset(Ptr, 0, Size);
	RefCount = 1L;
}
/*-----------------------------------------------------------------------------
	Function - Clear
	Description:
		Reset this entry to a blank state, freeing any associated memory.
-----------------------------------------------------------------------------*/
void
G42HeapManager::G42HeapEntry::Clear(void)
{
	Size = 0L;
	Lock = 0L;
	RefCount = 0L;
	SwapHandle = 0;
	SwappedOut = false;
	if (Ptr)
		delete [] Ptr;
	Ptr = 0;
}
/*-----------------------------------------------------------------------------
	Function - Resize
	Parameters:
		int32 size - New size for this entry
	Exceptions:
		G42MemoryException - This entry has not been allocated, or the
			new allocation has failed.
	Description:
		Change the size of the memory chunk associated with this entry.
		New memory is allocated, and the current memory contents are
		moved.  Excess original memory is truncated, while excess new
		memory is zero initialized.
-----------------------------------------------------------------------------*/
void
G42HeapManager::G42HeapEntry::Resize(int32 size)
{
	if (!Ptr || !size)
		throw G42MemoryException();
	byte * ptr;
	try
	{
		ptr = new byte [size];
		G42ThrowIfNull(ptr);
	}
	catch (xalloc)
	{
		throw G42MemoryException();
	}
	if (size > Size)
	{
		memset(ptr, 0, size);
		memcpy(ptr, Ptr, Size);
	}
	else
		memcpy(ptr, Ptr, size);
	delete [] Ptr;
	Ptr = ptr;
	Size = size;
}
/*-----------------------------------------------------------------------------
	Function - SwapOut
	Exceptions:
		G42MemoryException - Memory alloation for loading the entry from disk
		has failed.
	Description:
		Changes the state of this entry to 'Swapped In'.  A new chunk of memory
		is allocated for the swapper to load data into.
-----------------------------------------------------------------------------*/
void
G42HeapManager::G42HeapEntry::SwapIn(void)
{
	try
	{
		Ptr = new byte [Size];
		G42ThrowIfNull(Ptr);
	}
	catch (xalloc)
	{
		throw G42MemoryException();
	}
	SwappedOut = false;
}
/*-----------------------------------------------------------------------------
	Function - SwapOut
	Parameters:
		G42SwapHandle handle - Handle to a swapper entry representing this entry.
	Description:
		Changes the state of this entry to 'Swapped Out'.  The associated memory
		is released.
-----------------------------------------------------------------------------*/
void
G42HeapManager::G42HeapEntry::SwapOut(G42SwapHandle handle)
{
	SwapHandle = handle;
	SwappedOut = true;
	if (Ptr)
		delete [] Ptr;
	Ptr = 0;
}
/*-----------------------------------------------------------------------------
	G42HeapManager member functions:
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
	Function - G42HeapManager Constructor.
	Parameters:
		G42SwapManager & swapper - Reference to a G42SwapManager, for use in swapping
			entries to disk.
		int32 max_entry_size - Defaults to 64512, initial value for MaxEntrySize.
		int32 max_core_size - Defaults to 4194304, initial value for MaxCoreSize.
	Description:
		Create a G42HeapManager object with no entries.
-----------------------------------------------------------------------------*/
G42HeapManager::G42HeapManager(G42SwapManager & swapper, int32 max_entry_size,
	uint32 max_core_size)
	: CurrentCoreSize(0L), MaxCoreSize(max_core_size),
	MaxEntrySize(max_entry_size), NextFreeEntry(0), NumEntries(0L),
	Swapper(swapper), Valid(true), NextSwappable(0)
{}
/*-----------------------------------------------------------------------------
	Function - G42HeapManager Destructor.
	Description:
		Destroy this object, freeing all associated memory.
-----------------------------------------------------------------------------*/
G42HeapManager::~G42HeapManager(void)
{
	Clear();
}
/*-----------------------------------------------------------------------------
	Function - Allocate
	Parameters:
		int32 size - 	Size for new entry being created.
		bool force -	true = abide by max_entry_size.
							false = Ignore max_entry_size.
	Returns:
		int32 - A handle to the memory chunk that has been allocated.
	Exceptions:
		G42InternalException -  The G42HeapManager object is invalid,
			or NextFreeEntry has become corrupted.
	Description:
		Allocate a new memory chunk of the given size.  This size will be
		truncated to max_entry_size unless force is set to true.  The value
		returned serves as an ID for the memory chunk.
-----------------------------------------------------------------------------*/
int32
G42HeapManager::Allocate(int32 size, bool force)
{
	if (!Valid)
		throw G42InternalException();
	int32 ret_index = NextFreeEntry;
	if (!force && size > MaxEntrySize)
		size = MaxEntrySize;
	while (CurrentCoreSize + size > MaxCoreSize)
		SwapOut();
	try
	{
		EntryArray[NextFreeEntry].Allocate(size);
		CurrentCoreSize += size;
		if (NextFreeEntry == NumEntries)
			NumEntries++;
		for (NextFreeEntry++; NextFreeEntry < NumEntries; NextFreeEntry++)
		{
			if (EntryArray[NextFreeEntry].GetSize() == 0)
				break;
		}
	}
	catch (G42OutOfBoundsException & err)
	{
		Clear();
		Valid = false;
		throw G42InternalException(0, err.GetId());
	}
	if (ret_index < NextSwappable)
		NextSwappable = ret_index;
	return ret_index;
}
/*-----------------------------------------------------------------------------
	Function - Duplicate
	Parameters:
		int32 index - Which entry to duplicate.
	Exceptions:
		G42InternalException -  The G42HeapManager object is invalid.
		G42OutOfBoundsException - Invalid index.
		G42MemoryException - The index references an unallocated entry.
		G42LockException - The referenced entry is write locked.
	Description:
		Make a copy of the referenced entry.  This is a "lazy" copy which only
		increments the entries RefCount.  A complete copy will only occur when
		this entry is write locked.
-----------------------------------------------------------------------------*/
void
G42HeapManager::Duplicate(int32 index)
{
	if (!Valid)
		throw G42InternalException();
	if ((index >= NumEntries) || (index < 0L))
		throw G42OutOfBoundsException();
	G42HeapEntry & entry = EntryArray[index];
	if (entry.GetSize() == 0L)
		throw G42MemoryException();
	if (entry.IsWriteLocked())
		throw G42LockException();
	entry.IncrementRefCount();
}
/*-----------------------------------------------------------------------------
	Function - EnsureHandle
	Parameters:
		int32 index - Which entry to ensure.
	Description:
		Ensures that the entry for the given index is actually present in
		physical memory, and not just swapped out to disk.  If the entry is
		not in memory, other entries will be swapped out until enough physical
		space exists.  Then the entry will be read from disk by the swapper.
-----------------------------------------------------------------------------*/
void
G42HeapManager::EnsureHandle(int32 index)
{
	G42HeapEntry & entry = EntryArray[index];
	if (!entry.IsSwappedOut())
		return;
	while (CurrentCoreSize + entry.GetSize() > MaxCoreSize)
		SwapOut();
	entry.SwapIn();
	G42SwapHandle handle = entry.GetSwapHandle();
	try
	{
		Swapper.ReadBlock(handle, (byte *)entry);
	}
	catch (...)
	{
		entry.SwapOut(handle);
		throw;
	}
	CurrentCoreSize += entry.GetSize();
	if (index < NextSwappable)
		NextSwappable = index;
}
/*-----------------------------------------------------------------------------
	Function - InvalidateSwapHandle
	Parameters:
		int32 index - Which entry to invalidate.
	Description:
		Checks the given entry to determine if it has a swap handle.  If so,
		that swap handle is freed, and removed from this entry.  Thus, this
		entry no longer has swap data associated with it.
-----------------------------------------------------------------------------*/
void
G42HeapManager::InvalidateSwapHandle(int32 index)
{
	G42HeapEntry & entry = EntryArray[index];
	G42SwapHandle handle = entry.GetSwapHandle();
	entry.InvalidateSwapHandle();
	if (handle)
		Swapper.FreeHandle(handle);
}
/*-----------------------------------------------------------------------------
	Function - Release
	Parameters:
		int32 index - Which entry to release.
	Exceptions:
		G42InternalException - The G42HeapManager object is invalid.
		G42OutOfBoundsException - Invalid index.
	Description:
		Release the referenced entry from further use.  This only causes the
		entry to free its memory chunk if it has no other active handles.
-----------------------------------------------------------------------------*/
void
G42HeapManager::Release(int32 index)
{
	if (!Valid)
		throw G42InternalException();
	if ((index >= NumEntries) || (index < 0L))
		throw G42OutOfBoundsException();
	G42HeapEntry & entry = EntryArray[index];
	entry.DecrementRefCount();
	if (entry.GetRefCount() <= 0)
	{
		G42SwapHandle handle = entry.GetSwapHandle();
		if (!entry.IsSwappedOut())
			CurrentCoreSize -= entry.GetSize();
		entry.Clear();
		if (index < NextFreeEntry)
			NextFreeEntry = index;
		if (handle)
			Swapper.FreeHandle(handle);
		if (index != NextSwappable)
			return;
		for (NextSwappable++; NextSwappable < NumEntries; NextSwappable++)
		{
			G42HeapEntry & entry = EntryArray[NextSwappable];
			if (entry.GetSize() != 0 && !entry.IsSwappedOut()
				&& !entry.IsReadLocked() && !entry.IsWriteLocked())
				break;
		}
	}
}
/*-----------------------------------------------------------------------------
	Function - VerifyHandle
	Parameters:
		int32 index - Which entry to verify.
	Exceptions:
		G42InternalException - The G42HeapManager object is invalid.
		G42OutOfBoundsException - Invalid index.
		G42MemoryException - The referenced entry if empty.
	Description:
		Checks the referenced entry to verify its validity.
-----------------------------------------------------------------------------*/
void
G42HeapManager::VerifyHandle(int32 index)
{
	if (!Valid)
		throw G42InternalException();
	if ((index >= NumEntries) || (index < 0L))
		throw G42OutOfBoundsException();
	if (EntryArray[index].GetRefCount() < 1)
		throw G42MemoryException();
}
/*-----------------------------------------------------------------------------
	Function - Clear
	Description:
		Clears out the G42HeapManager object, freeing all allocated memory.
-----------------------------------------------------------------------------*/
void
G42HeapManager::Clear(void)
{
	EntryArray.Clear();
	NumEntries = 0L;
	NextFreeEntry = 0L;
}
/*-----------------------------------------------------------------------------
	Function - Fork
	Parameters:
		int32 index - which index to fork.
	Returns:
		int32 - Handle of new entry.
	Exceptions:
		G42InternalException - The G42HeapManager object is invalid.
		G42OutOfBoundsException - index is invalid.
	Description:
		When there is only one active handle for the referenced entry, nothing
		happens.  When there is more than one, a new entry is created as an
		exact duplicate to the referenced entry.  The handle to this new entry
		is returned.
-----------------------------------------------------------------------------*/
int32
G42HeapManager::Fork(int32 index)
{
	if (!Valid)
		throw G42InternalException();
	if ((index >= NumEntries) || (index < 0))
		throw G42OutOfBoundsException();
	G42HeapEntry & entry = EntryArray[index];
	if (entry.GetRefCount() < 2L)
		return index;
	int32 size = entry.GetSize();
	int32 handle = Allocate(size, true);
	memcpy((byte *)EntryArray[handle], (byte *)entry, size);
	entry.DecrementRefCount();
	return handle;
}
/*-----------------------------------------------------------------------------
	Function - Resize
	Parameters:
		int32 index - Which entry to resize.
		int32 size - new size for this entry.
	Exceptions:
		G42InternalException	- The G42HeapManager object is invalid.
	Description:
		Resizes the referenced entry.
-----------------------------------------------------------------------------*/
void
G42HeapManager::Resize(int32 index, int32 size)
{
	if (!Valid)
		throw G42InternalException();
	G42HeapEntry & entry = EntryArray[index];
	int32 delta_core_size = size - entry.GetSize();
	while (CurrentCoreSize + delta_core_size > MaxCoreSize)
		SwapOut();
	entry.Resize(size);
	CurrentCoreSize += delta_core_size;
}
/*-----------------------------------------------------------------------------
	Function - Split
	Parameters:
		int32 index		- Which index to split.
		int32 offset 	- Where in the chunk to split.
	Returns:
		int32 - Handle of new entry.
	Exceptions:
		G42InternalException - The G42HeapManager object is invalid,
			or NextFreeEntry has become corrupted.
		G42OutOfBoundsException - invalid index, or invalid offset.
	Description:
		Splits the referenced entry into 2 entries.  It resizes the original
		entry to offset bytes, and allocates a new entry for the remaining bytes.
		The handle to the new entry is returned.
-----------------------------------------------------------------------------*/
int32
G42HeapManager::Split(int32 index, int32 offset)
{
	if (!Valid)
		throw G42InternalException();
	if ((index >= NumEntries) || (index < 0))
		throw G42OutOfBoundsException();
	if (offset <= 0L || offset >= EntryArray[index].GetSize())
		throw G42OutOfBoundsException();
	int32 size = EntryArray[index].GetSize() - offset;
	while (CurrentCoreSize + size > MaxCoreSize)
		SwapOut();
	int32 ret_index = NextFreeEntry;
	try
	{
		EntryArray[NextFreeEntry].Allocate(size);
		if (NextFreeEntry == NumEntries)
			NumEntries++;
		for (NextFreeEntry++; NextFreeEntry < NumEntries; NextFreeEntry++)
		{
			if (EntryArray[NextFreeEntry].GetSize() == 0)
				break;
		}
		memcpy((byte *)EntryArray[ret_index], (byte *)EntryArray[index] + offset,
			EntryArray[ret_index].GetSize());
		EntryArray[index].Resize(offset);
	}
	catch (G42OutOfBoundsException & err)
	{
		Clear();
		Valid = false;
		throw G42InternalException(0, err.GetId());
	}
	if (ret_index < NextSwappable)
		NextSwappable = ret_index;
	return ret_index;
}
/*-----------------------------------------------------------------------------
	Function - SwapOut
	Exceptions:
		G42InternalException - No swappable entries were found.
	Description:
		The entry list is searched for an entry that is available for swapping
		out.  Such an entry must be currently in physical memory, and cannot be
		read or write locked.  Once this entry is found, its swap handle is
		checked.  If it currently has swap data, it is now marked as 'swapped'.
		If not, its data is given to the swapper, then it is marked 'swapped'.
-----------------------------------------------------------------------------*/
void
G42HeapManager::SwapOut(void)
{
	if (NextSwappable >= NumEntries)
		throw G42InternalException();
	G42HeapEntry & entry = EntryArray[NextSwappable];
	if (entry.GetSwapHandle())
		entry.SwapOut(entry.GetSwapHandle());
	else
	{
		G42SwapHandle handle = Swapper.WriteBlock((byte *)entry, entry.GetSize());
		entry.SwapOut(handle);
	}
	CurrentCoreSize -= entry.GetSize();
	for (NextSwappable++; NextSwappable < NumEntries; NextSwappable++)
	{
		G42HeapEntry & entry = EntryArray[NextSwappable];
		if (entry.GetSize() != 0 && !entry.IsSwappedOut()
			&& !entry.IsReadLocked() && !entry.IsWriteLocked())
			break;
	}
#if 0
	for (intf n = 0; n < NumEntries; n++)
	{
		G42HeapEntry & entry = EntryArray[n];
		if (entry.GetSize() == 0 || entry.IsSwappedOut()
			|| entry.IsReadLocked()	|| entry.IsWriteLocked())
			continue;
		if (entry.GetSwapHandle())
			entry.SwapOut(entry.GetSwapHandle());
		else
		{
			G42SwapHandle handle = Swapper.WriteBlock((byte *)entry, entry.GetSize());
			entry.SwapOut(handle);
		}
		CurrentCoreSize -= entry.GetSize();
		return;
	}
	throw G42InternalException();
#endif
}
void
G42HeapManager::ReadLock(int32 index)
{
	EntryArray[index].ReadLock();
	if (index != NextSwappable)
		return;
	for (NextSwappable++; NextSwappable < NumEntries; NextSwappable++)
	{
		G42HeapEntry & entry = EntryArray[NextSwappable];
		if (entry.GetSize() != 0 && !entry.IsSwappedOut()
			&& !entry.IsReadLocked() && !entry.IsWriteLocked())
			break;
	}
}
/*-----------------------------------------------------------------------------
	Function - WriteLock
	Parameters:
		int32 index - Which entry to lock.
	Description:
		Write locks the requested entry.  Note that operator[] is utilized.
-----------------------------------------------------------------------------*/
void
G42HeapManager::WriteLock(int32 index)
{
	EntryArray[index].WriteLock();
	if (index != NextSwappable)
		return;
	for (NextSwappable++; NextSwappable < NumEntries; NextSwappable++)
	{
		G42HeapEntry & entry = EntryArray[NextSwappable];
		if (entry.GetSize() != 0 && !entry.IsSwappedOut()
			&& !entry.IsReadLocked() && !entry.IsWriteLocked())
			break;
	}
}

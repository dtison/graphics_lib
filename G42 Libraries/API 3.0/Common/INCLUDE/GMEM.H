/*-----------------------------------------------------------------------------
	file:			gmem.h
	author:		John Pazdernik
	classes:		G42Lock, G42HeapManager, G42HeapEntry
	date:			10/09/96
	revision:	3.0
	The following classes are for memory management, i.e. the allocation of,
	access to, and release of pointers to flat chunks of memory. The
	G42HeapManager class provides the majority of this functionality.  It takes
	requests for memory, gives out handles which may be "traded in" for actual
	pointers, and releases memory when it is no longer needed.  The G42Lock
	class is a base class from which a specific "LockedMemory" class may be
	derived.  Its only purpose is to provide a strict interface to the private
	functions of the G42HeapManager.
-----------------------------------------------------------------------------*/
#if !defined(GMEM_H)
#define GMEM_H
#include "gtype.h"
#include "gtools.h"
#include "gswapper.h"
// Forward declarations
class G42Lock;
/*-----------------------------------------------------------------------------
	class G42HeapManager
-----------------------------------------------------------------------------*/
class G42HeapManager
{
	friend class G42Lock;	// Allows access to private functions
public:
	// constructor/destructor
	G42HeapManager(G42SwapManager & swapper, int32 max_entry_size = 64512L,
		uint32 max_core_size = 4194304L);
	~G42HeapManager(void);
	// member functions
	int32 Allocate(int32 size, bool force = false);
	void Duplicate(int32 index);
	void EnsureHandle(int32 index);
	int32 GetMaxCoreSize(void) const
			{return MaxCoreSize;}
	int32 GetMaxEntrySize(void) const
			{return MaxEntrySize;}
	int32 GetNumEntries(void) const
			{return NumEntries;}
	int32 GetSize(int32 index) const;
	G42SwapManager & GetSwapper(void) const
			{return Swapper;}
	void InvalidateSwapHandle(int32 index);
	bool IsValid(void) const
			{return Valid;}
	void Release(int32 index);
	void SetMaxEntrySize(int32 max_entry_size);
	void SetMaxCoreSize(int32 max_core_size)
			{MaxCoreSize = max_core_size;}
	void VerifyHandle(int32 index);
private:
	/*--------------------------------------------------------------------------
		class G42HeapEntry
	--------------------------------------------------------------------------*/
	class G42HeapEntry
	{
	public:
		// constructor/destructor
		G42HeapEntry(void);
		~G42HeapEntry(void);
		// member functions
		void Allocate(int32 size);
		void Clear(void);
		void DecrementRefCount(void)
				{RefCount--;}
		intf GetRefCount(void) const
				{return RefCount;}
		int32 GetSize(void) const
				{return Size;}
		G42SwapHandle GetSwapHandle(void) const
				{return SwapHandle;}
		void IncrementRefCount(void)
				{RefCount++;}
		void InvalidateSwapHandle(void)
				{SwapHandle = 0;}
		bool IsReadLocked(void) const
				{return (Lock > 0);}
		bool IsSwappedOut(void) const
				{return SwappedOut;}
		bool IsWriteLocked(void) const
				{return (Lock == -1L);}
		void ReadLock(void);
		void ReadUnlock(void)
				{Lock--;}
		void Resize(int32 size);
		void SwapIn(void);
		void SwapOut(G42SwapHandle swap_handle);
		void WriteLock(void);
		void WriteUnlock(void)
				{Lock = 0L;}
		// operators
		operator byte * (void) const;
	private:
		// data members
		intf				Lock;			// locked status for this entry,
											// -1 = writeLocked, 1 or more = readlocked
		byte *			Ptr; 			// pointer to the actual memory
		intf				RefCount;	// count of # of active handles to this entry
		int32	 			Size;			// size of the memory chunk
		G42SwapHandle	SwapHandle; // Handle for getting data from Swapper
		bool				SwappedOut; // true if this entry has been swapped to disk
		// Hidden copy and operator=
		G42HeapEntry(G42HeapEntry &) {}
		G42HeapEntry & operator= (G42HeapEntry &)
				{return *this;}
	};
	int32 						CurrentCoreSize;	// Current amount of ram in use
	G42Vector<G42HeapEntry>	EntryArray;		// Collection of entries
	int32							MaxCoreSize;	// Maximum ram use allowed
	int32							MaxEntrySize;	// Largest allocation size allowed.
														// Can override.
	int32							NextFreeEntry; // Blank entry with smallest index
														// in EntryArray
	int32							NumEntries;		// size of EntryArray
	G42SwapManager &			Swapper;       // Reference to G42SwapManager object
	bool							Valid;			// Is this object working?
	int32							NextSwappable;
	// member functions
	void Clear(void);
	int32 Fork(int32 index);
	void ReadLock(int32 index);
	void ReadUnlock(int32 index)
			{EntryArray[index].ReadUnlock();
			if (EntryArray[index].IsReadLocked())
				return;
			if (index < NextSwappable)
				NextSwappable = index;}
	void Resize(int32 index, int32 size);
	int32 Split(int32 index, int32 offset);
	void SwapOut(void);
	void WriteLock(int32 index);
	void WriteUnlock(int32 index)
			{EntryArray[index].WriteUnlock(); if (index < NextSwappable) NextSwappable = index;}
	// operators
	byte * operator[] (int32 index) const;
	// Hidden Copy Contructor and operator=
	G42HeapManager(G42HeapManager &ref )
		: Swapper(ref.Swapper) {}
	G42HeapManager & operator= (G42HeapManager &)
			{return *this;}
};
/*-----------------------------------------------------------------------------
	class G42Lock
-----------------------------------------------------------------------------*/
class G42Lock
{
public:
	// constructor/destructor
	G42Lock(void) {}
	virtual ~G42Lock(void) {}
protected:
	int32 Fork(G42HeapManager & heap, int32 index) const
			{return heap.Fork(index);}
	byte * GetPtr(const G42HeapManager & heap, int32 index) const;
	void ReadLock(G42HeapManager & heap, int32 index) const;
	void ReadUnlock(G42HeapManager & heap, int32 index) const
			{heap.ReadUnlock(index);}
	void Resize(G42HeapManager & heap, int32 index, int32 size) const
			{heap.Resize(index, size);}
	int32 Split(G42HeapManager & heap, int32 index, int32 offset) const
			{return heap.Split(index, offset);}
	void WriteLock(G42HeapManager & heap, int32 index) const;
	void WriteUnlock(G42HeapManager & heap, int32 index) const
			{heap.WriteUnlock(index);}
private:
	// Hidden Copy Contructor and operator=
	G42Lock(G42Lock &) {}
	G42Lock & operator= (G42Lock &)
			{return *this;}
};
/*-----------------------------------------------------------------------------
	G42HeapManager::G42HeapEntry inline functions:
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
	Function - ReadLock
	Exceptions:
		G42LockException - This entry is currently write locked.
	Description:
		Read locks this entry.  Note that an entry may have more than one read
		lock at once.
-----------------------------------------------------------------------------*/
inline void
G42HeapManager::G42HeapEntry::ReadLock(void)
{
	if (Lock == -1L)
		throw G42LockException();
	Lock++;
}
/*-----------------------------------------------------------------------------
	Function - WriteLock
	Exceptions:
		G42LockException - This entry is currently write locked.
	Description:
		Write locks this entry.  An entry may have only one write lock at a time.
-----------------------------------------------------------------------------*/
inline void
G42HeapManager::G42HeapEntry::WriteLock(void)
{
	if (Lock != 0)
		throw G42LockException();
	Lock = -1L;
}
/*-----------------------------------------------------------------------------
	Function - operater byte *
	Returns:
		byte * - A pointer to the memory represented by this entry
	Exceptions:
		G42MemoryException - This entry's pointer is Null.
	Description:
		Convert this entry into a pointer to a memory chunk.
-----------------------------------------------------------------------------*/
inline
G42HeapManager::G42HeapEntry::operator byte * (void) const
{
	if (!Ptr)
		throw G42MemoryException();
	return Ptr;
}
/*-----------------------------------------------------------------------------
	G42HeapManager inline functions:
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
	Function - GetSize
	Parameters:
		int32 index - Which entry to inspect.
	Returns:
		int32 - Size in bytes of entry inspected
	Exceptions:
		G42InternalException 	- The G42HeapManager object is invalid.
		G42OutOfBoundsException	- Invalid index
	Description:
		Gets the allocated size of an entry.
-----------------------------------------------------------------------------*/
inline int32
G42HeapManager::GetSize(int32 index) const
{
	if (!IsValid())
		throw G42InternalException();
	if ((index >= NumEntries) || (index < 0))
		throw G42OutOfBoundsException();
	return EntryArray[index].GetSize();
}
/*-----------------------------------------------------------------------------
	Function - SetMaxEntrySize
	Parameters:
		int32 max_entry_size - New maximum size for entry allocation.
	Description:
		Sets the maximum size for all subsequent allocation of entries. Note that
		this size is reduced to have 4 byte granularity, and that allocation may
		override this maximum.
-----------------------------------------------------------------------------*/
inline void
G42HeapManager::SetMaxEntrySize(int32 max_entry_size)
{
	MaxEntrySize = max_entry_size;
	MaxEntrySize -= (MaxEntrySize % 4);
}
/*-----------------------------------------------------------------------------
	Function - operator []
	Parameters:
		int32 index - Which entry to inspect.
	Returns:
		byte * - A pointer to the memory chunk represented by this entry.
	Exceptions:
		G42InternalException - The G42HeapManager object is invalid.
		G42OutOfBoundsException - Invalid index.
	Description:
		Convert the referenced entry into a pointer to a memory chunk.
-----------------------------------------------------------------------------*/
inline byte *
G42HeapManager::operator[] (int32 index) const
{
	if (!IsValid())
		throw G42InternalException();
	if ((index >= NumEntries) || (index < 0))
		throw G42OutOfBoundsException();
	return (byte *)(EntryArray[index]);
}
/*-----------------------------------------------------------------------------
	Function - GetPtr
	Parameters:
		G42HeapManager & heap - Reference to a heap manager.
		int32 index - Which entry from that heap to inspect.
	Description:
		Gets the byte pointer for the requested index.  Note that operator[]
		is utilized.
-----------------------------------------------------------------------------*/
inline byte *
G42Lock::GetPtr(const G42HeapManager & heap, int32 index) const
{
	return heap[index];
}
/*-----------------------------------------------------------------------------
	Function - ReadLock
	Parameters:
		G42HeapManager & heap - Reference to a heap manager.
		int32 index - Which entry from that heap to inspect.
	Description:
		Read locks the requested index.
-----------------------------------------------------------------------------*/
inline void
G42Lock::ReadLock(G42HeapManager & heap, int32 index) const
{
	heap.ReadLock(index);
}
/*-----------------------------------------------------------------------------
	Function - WriteLock
	Parameters:
		G42HeapManager & heap - Reference to a heap manager.
		int32 index - Which entry from that heap to inspect.
	Description:
		Write locks the requested index.
-----------------------------------------------------------------------------*/
inline void
G42Lock::WriteLock(G42HeapManager & heap, int32 index) const
{
	heap.WriteLock(index);
}
#endif // GMEM_H sentry

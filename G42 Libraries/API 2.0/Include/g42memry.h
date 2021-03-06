#if !defined(G42MEMORY_H)
#define G42MEMORY_H
#include "g42itype.h"
// const long memory_block_size = 4096L; // 65528L;
class G42SwapManager
{
private:
	class SwapEntry
	{
	private:
		int32		Seek;
		bool		Free;
		SwapEntry(SwapEntry & Ref) {}		// Hidden Copy Constructor.
		SwapEntry & operator= (SwapEntry & Ref)
			{return *this;}					// Hidden Operator=
	public:
		SwapEntry(int32 seek = 0L)
			: Seek(seek), Free(true) {}
		~SwapEntry(void) {}
		int32 GetSeek(void) const
			{return Seek;}
		bool IsFree(void) const
			{return Free;}
		void SetSeek(int32 seek)
			{Seek = seek;}
		void SetFree(bool free = true)
      	{Free = free;}
	};
	char *		FileName;
	#ifdef MACOS
	short		FP;
	#else
	FILE *		FP;
	#endif
	SwapEntry *	Table;
	uint			NumEntries;
	uint			NextAvail;
	bool			Valid;
	uint			memory_block_size;
	G42SwapManager(G42SwapManager & Ref) {}	// Hidden Copy Constructor.
	G42SwapManager & operator= (G42SwapManager & Ref)
		{return *this;}							 	// Hidden Operator=
public:
	G42SwapManager(const char * name, uint block_size = 65528);
	~G42SwapManager(void);
	bool IsValid(void) const
		{return Valid;}
	bool Read(unsigned char * buf, int32 size, uint index);
	int Write(unsigned char * buf, int32 size, int index = -1);
	void FreeEntry(uint index);
	uint GetBlockSize(void) const
   		{ return memory_block_size; }
};
class G42PageEntry
{
private:
	int		SeekRef;
	int		CoreRef;
	int32		Size;
	uint		RefCount;
	bool		Modified;
	G42PageEntry(G42PageEntry & Ref) {}    // Hidden Copy Constructor.
	G42PageEntry & operator= (G42PageEntry & Ref)
		{return *this;}							// Hidden Operator=
public:
	G42PageEntry(int32 size);
	~G42PageEntry(void);
   void Flush(void);
	uint GetRefCount(void) const
   	{return RefCount;}
	int GetSeekRef(void) const
		{return SeekRef;}
	int GetCoreRef(void) const
		{return CoreRef;}
	int32 GetSize(void) const
		{return Size;}
	bool GetModified(void) const
   	{return Modified;}
	void IncRefCount(void)
		{RefCount++;}
	void DecRefCount(void)
		{RefCount--;}
	void SetSeekRef(int seek)
		{SeekRef = seek;}
	void SetCoreRef(uint ref)
		{CoreRef = ref;}
	void SetSize(int32 size)
		{Size = size;}
	void SetModified(bool mod = true)
   	{Modified = mod;}
};
class G42PageTable
{
private:
	G42PageEntry **	Table;
	uint					NumPages;
   uint					NextAvail;
	bool					Valid;
	G42PageTable(G42PageTable & Ref) {}    // Hidden Copy Constructor.
	G42PageTable & operator= (G42PageTable & Ref)
		{return *this;}							// Hidden Operator=
public:
	G42PageTable(void);
	~G42PageTable(void);
	bool IsValid(void) const
		{return Valid;}
	uint GetNumPages(void) const
		{return NumPages;}
	uint AddPage(int32 size);
	void DeletePage(uint index);
   uint ForkPage(uint index);
	void IncRefCount(uint index)
   	{Table[index]->IncRefCount();}
	G42PageEntry * operator[] (int index) const
		{return (((index < 0) || (index >= NumPages)) ? 0 : Table[index]);}
};
class G42CoreTable
{
private:
	class CoreEntry
	{
	private:
 		G42SwapManager *	Swap;
		G42PageEntry *		Page;
		int					LockFlag;
		unsigned char *	Data;
		CoreEntry *			Next;
		CoreEntry *			Prev;
		int					Index;
		uint			memory_block_size;
		CoreEntry(CoreEntry & Ref) {}    // Hidden Copy Constructor.
		CoreEntry & operator= (CoreEntry & Ref)
			{return *this;}					// Hidden Operator=
	public:
		CoreEntry(void);
		~CoreEntry(void);
		void EnsureData(void)
      	{if (!Data) Data = new unsigned char [memory_block_size];}
		void ClearData(void)
			{if (Data) memset(Data, 0, memory_block_size);}
		G42PageEntry * GetPage(void) const
			{return Page;}
		int16 GetLock(void) const
			{return LockFlag;}
		unsigned char * GetData(void) const
      	{return Data;}
		CoreEntry * GetNext(void) const
			{return Next;}
		CoreEntry * GetPrev(void) const
			{return Prev;}
		int GetIndex(void) const
      	{return Index;}
		void SetPage(G42PageEntry * page)
     	 	{Page = page;}
		void SetSwap(G42SwapManager * swap)
	      	{Swap = swap; memory_block_size = swap->GetBlockSize();}
		void SetNext(CoreEntry * next)
			{Next = next;}
		void SetPrev(CoreEntry * prev)
			{Prev = prev;}
		void SetIndex(int index)
      	{Index = index;}
		bool Load(G42PageEntry * page);
		void Unload(void);
		void Lock(void)
			{LockFlag++;}
		void Unlock(void)
			{if (LockFlag) LockFlag--;}
		operator unsigned char * (void) const
			{return Data;}
	};
	G42SwapManager *	Swap;
	CoreEntry *			Head;
	CoreEntry *			Tail;
	CoreEntry *			HeadLocked;
	CoreEntry *			TailLocked;
	CoreEntry * 		Table;
	int16					Size;
	bool					Valid;
	uint			memory_block_size;
	G42CoreTable(G42CoreTable & Ref) {}    // Hidden Copy Constructor.
	G42CoreTable & operator= (G42CoreTable & Ref)
		{return *this;}							// Hidden Operator=
public:
	G42CoreTable(G42SwapManager * swap, int32 size = 1048448L,
		bool pre_allocate = false);
	~G42CoreTable(void);
	bool IsValid(void) const
		{return Valid;}
	bool Load(G42PageEntry * page);
	void LoadAndFork(G42PageEntry * old_page, G42PageEntry * new_page);
	void Unload(int16 index = -1);
	unsigned char * GetData(int16 index)
		{return ((index >= Size) ? 0 : (unsigned char *)Table[index]);}
	void Lock(int16 index = -1);
	void Unlock(int16 index = -1);
   void ToTopOfList(uint index);
	uint GetUnlockedBlocks(void) const;
	bool IsLocked(uint index) const
		{return Table[index].GetLock();}
	G42SwapManager * GetSwap(void) const
		{return Swap;}
};
class G42Data
{
	G42CoreTable *		Core;
	G42PageTable *		Page;
   G42SwapManager *	SwapMan;
   bool					OwnsMembers;
private:
	G42Data(G42Data & Ref) {}    // Hidden Copy Constructor.
	G42Data & operator= (G42Data & Ref)
		{return *this;}			  // Hidden Operator=
public:
	G42Data(G42CoreTable * core, G42PageTable * page);
	G42Data(int32 size = 1048448L, const char * name = 0,
		bool pre_allocate = false);
	~G42Data(void);
	int AddPage(int32 size);
	void DeletePage(uint index);
	void Lock(uint index);
	void Unlock(uint index);
   int ForkPage(uint index);
	void IncRefCount(uint index)
   	{Page->IncRefCount(index);}
	void PreLoad(uint * index_array, uint size);
	void MarkModified(uint index, bool mod = true)
		{(*Page)[index]->SetModified(mod);}
	uint GetBlockSize(void)
		{ return SwapMan->GetBlockSize();}   	
	unsigned char * operator[] (int index) const;
};
#endif // G42MEMORY_H

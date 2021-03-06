/*
	g42memory.cpp - memory manager
	Author:  John Pazdernik
	Changes:
		Who	When		Key		Description
		Guy	4/11/96	ges001	Disk usage bug, new swap entries were marked used
*/
#include "g42memry.h"
#if !defined(MACOS)	// (m.3)  
#include "dir.h"
#endif
#ifdef MACOS
#include <unistd.h>
#include <Files.h>
#endif
//#include <gesdebug.h>
#define GES(f)
G42SwapManager::G42SwapManager(const char * name, uint block_size)
: 
	FileName(0), Table(0), NumEntries(0), NextAvail(0), Valid(false),
	FP(0),
	memory_block_size(block_size)
{
	if (!name)
		return;
	FileName = new char [strlen(name) + 1];
	strcpy(FileName, name);
	#ifdef MACOS
	char * buffer(new char [256]);
	if (!buffer)
		return;
	strcpy(buffer, name);
	c2pstr(buffer);
	if (Create((unsigned char *)buffer, 0, 'Gr42', 'Temp'))
	{
//		delete [] buffer;
//		return;
	}
	if (FSOpen((unsigned char *)buffer, 0, &FP))
	{
		delete [] buffer;
		return;
	}
	delete [] buffer;
	#else
	FP = fopen(FileName, "w+b");
	if (FP == (FILE *)NULL)
		return;
	#endif
	NumEntries = 16;
	Table = new SwapEntry [NumEntries];
	if (!Table)
		return;
	for (int32 n = 0; n < NumEntries; n++)
		Table[n].SetSeek(n * memory_block_size);
	Valid = true;
}
G42SwapManager::~G42SwapManager(void)
{
	if (FP)
	{
		#ifdef MACOS
		FSClose(FP);
		#else
		fclose(FP);
		#endif
		FP = 0;
	}
	if (FileName)
	{
		unlink(FileName);
		delete [] FileName;
		FileName = 0;
	}
	if (Table)
	{
		delete [] Table;
		Table = 0;
	}
}
bool
G42SwapManager::Read(unsigned char * buf, int32 size, uint index)
{
	if (!IsValid())
		return false;
	if (index >= NumEntries)
		return false;
	if (Table[index].IsFree())
		return false;
	#ifdef MACOS
	OSErr err = SetFPos(FP, fsFromStart, Table[index].GetSeek());
	if (err == eofErr)
	{
		err = SetEOF(FP, Table[index].GetSeek());
		if (err)
			return -1;
		err = SetFPos(FP, fsFromStart, Table[index].GetSeek());
	}
	if (err)
		return false;
	long nbytes(size);
	FSRead(FP, &nbytes, buf);
	if (err)
		return false;
	if (nbytes != size)
		return false;
	#else
	fseek(FP, Table[index].GetSeek(), SEEK_SET);
	int32 nbytes = fread(buf, size, 1, FP);
	if (nbytes != 1)
		return false;
	#endif
	return true;
}
int
G42SwapManager::Write(unsigned char * buf, int32 size, int index)
{
	if (!IsValid())
		return -1;
	if (index != -1)
	{
		if (index >= NumEntries)
			return -1;
		#ifdef MACOS
		OSErr err = SetFPos(FP, fsFromStart, Table[index].GetSeek());
		if (err == eofErr)
		{
			err = SetEOF(FP, Table[index].GetSeek());
			if (err)
				return -1;
			err = SetFPos(FP, fsFromStart, Table[index].GetSeek());
		}
		if (err)
			return -1;
		long nbytes(size);
		err = FSWrite(FP, &nbytes, buf);
		if (err)
			return -1;
		if (nbytes != size)
			return -1;
		#else
		fseek(FP, Table[index].GetSeek(), SEEK_SET);
		int32 nbytes = fwrite(buf, size, 1, FP);
		if (nbytes != 1)
			return -1;
		#endif
		return index;
	}
	if (NextAvail >= NumEntries)
	{
		SwapEntry * new_table = new SwapEntry [(NumEntries + 16)];
//    ges001: memset doesn't work because it sets them not free (free = 0)
//		memset(new_table, 0, (NumEntries + 16) * sizeof(SwapEntry));
		memcpy(new_table, Table, NumEntries * sizeof(SwapEntry));
		delete [] Table;
		Table = new_table;
		for (int32 n = NumEntries; n < (NumEntries + 16); n++)
			Table[n].SetSeek(n * 65528);
		NumEntries += 16;
	}
	#ifdef MACOS
	OSErr err = SetFPos(FP, fsFromStart, Table[NextAvail].GetSeek());
	if (err == eofErr)
	{
		err = SetEOF(FP, Table[NextAvail].GetSeek());
		if (err)
			return -1;
		err = SetFPos(FP, fsFromStart, Table[NextAvail].GetSeek());
	}
	if (err)
		return -1;
	long nbytes(size);
	err = FSWrite(FP, &nbytes, buf);
	if (err)
		return -1;
	if (size != nbytes)
		return -1;
	#else
	fseek(FP, Table[NextAvail].GetSeek(), SEEK_SET);
	int32 nbytes = fwrite(buf, size, 1, FP);
	if (nbytes != 1)
		return -1;
	#endif
	Table[NextAvail].SetFree(false);
	index = NextAvail;
	while (NextAvail < NumEntries)
	{
		if (Table[NextAvail].IsFree())
			break;
		NextAvail++;
	}
	return index;
}
void
G42SwapManager::FreeEntry(uint index)
{
	if (!IsValid())
		return;
	if (index >= NumEntries)
		return;
	Table[index].SetFree();
	if (index < NextAvail)
		NextAvail = index;
}
G42PageEntry::G42PageEntry(int32 size)
	: SeekRef(-1), CoreRef(-1), Size(size), RefCount(0), Modified(false)
{}
G42PageEntry::~G42PageEntry(void)
{}
void
G42PageEntry::Flush(void)
{
	SeekRef = -1;
	CoreRef = -1;
	Size = 0L;
	RefCount = 0;
   Modified = false;
}
G42PageTable::G42PageTable(void)
	: Table(0), NumPages(16), Valid(false), NextAvail(0)
{
	Table = new G42PageEntry * [NumPages];
	memset(Table, 0, NumPages * sizeof(G42PageEntry *));
	Valid = true;
}
G42PageTable::~G42PageTable(void)
{
	if (Table)
	{
		for (int n = 0; n < NumPages; n++)
			delete Table[n];
		delete [] Table;
      Table = 0;
	}
}
uint
G42PageTable::AddPage(int32 size)
{
	if (NextAvail >= NumPages)
	{
		G42PageEntry ** new_table = new G42PageEntry * [(NumPages + 16)];
		memset(new_table, 0, (NumPages+16) * sizeof(G42PageEntry *));
		memcpy(new_table, Table, NumPages * sizeof(G42PageEntry *));
		delete [] Table;
      Table = new_table;
		NumPages += 16;
	}
	if (Table[NextAvail] == 0)
		Table[NextAvail] = new G42PageEntry(size);
	else
		Table[NextAvail]->SetSize(size);
	Table[NextAvail]->IncRefCount();
   uint val = NextAvail;
	while (NextAvail < NumPages)
   {
		if ((Table[NextAvail] == 0) || (Table[NextAvail]->GetRefCount() == 0))
			break;
		NextAvail++;
	}
   return val;
}
void
G42PageTable::DeletePage(uint index)
{
	Table[index]->DecRefCount();
	if (Table[index]->GetRefCount() == 0)
	{
		Table[index]->Flush();
		if (index < NextAvail)
			NextAvail = index;
	}
}
uint
G42PageTable::ForkPage(uint index)
{
	if (NextAvail >= NumPages)
	{
		G42PageEntry ** new_table = new G42PageEntry * [(NumPages + 16)];
		memset(new_table, 0, (NumPages+16) * sizeof(G42PageEntry *));
		memcpy(new_table, Table, NumPages * sizeof(G42PageEntry *));
		delete [] Table;
      Table = new_table;
		NumPages += 16;
	}
	if (Table[NextAvail] == 0)
		Table[NextAvail] = new G42PageEntry(Table[index]->GetSize());
	else
		Table[NextAvail]->SetSize(Table[index]->GetSize());
	Table[NextAvail]->IncRefCount();
	Table[NextAvail]->SetSeekRef(Table[index]->GetSeekRef());
	Table[NextAvail]->SetCoreRef(Table[index]->GetCoreRef());
	Table[NextAvail]->SetModified(Table[index]->GetModified());
	uint val = NextAvail;
	while (NextAvail < NumPages)
	{
		if ((Table[NextAvail] == 0) || (Table[NextAvail]->GetRefCount() == 0))
			break;
		NextAvail++;
	}
	return val;
}
G42CoreTable::CoreEntry::CoreEntry(void)
: 
	Swap(0), Page(0), LockFlag(0), Data(0), Next(0), Prev(0),
	memory_block_size(0)
{
	//Data = new unsigned char [memory_block_size];
	//memset(Data, 0, memory_block_size);
}
G42CoreTable::CoreEntry::~CoreEntry(void)
{
	if (Data)
	{
		delete [] Data;
		Data = 0;
	}
}
bool
G42CoreTable::CoreEntry::Load(G42PageEntry * page)
{
	if (LockFlag)
		return false;
	if (Page != (G42PageEntry *)NULL)
	{
GES((ges, "Swapping out %d\n", Index))
		if (Page->GetSeekRef() == -1)
		{
			int new_ref = Swap->Write(Data, Page->GetSize(), -1);
			if (new_ref != -1)
			{
				Page->SetSeekRef(new_ref);
				Page->SetModified(false);
			}
		}
		else if (Page->GetModified())
		{
			Swap->Write(Data, Page->GetSize(), Page->GetSeekRef());
			Page->SetModified(false);
		}
		Page->SetCoreRef(-1);
	}
	if (!Data)
	{
		Data = new unsigned char [memory_block_size];
	}
	if (page->GetSeekRef() != -1)
	{
GES((ges, "Loading in page.\n"))
		Page = page;
		Swap->Read(Data, Page->GetSize(), Page->GetSeekRef());
	}
	else
	{
GES((ges, "Assigning page.\n"))	
		Page = page;
      memset(Data, 0, memory_block_size);
	}
	return true;
}
void
G42CoreTable::CoreEntry::Unload()
{
	if (Page->GetSeekRef() != -1)
   	Swap->FreeEntry(Page->GetSeekRef());
	Page = 0;
	LockFlag = 0;
}
G42CoreTable::G42CoreTable(G42SwapManager * swap, int32 size, bool pre_allocate)
: 
	Swap(swap), Head(0), Tail(0), HeadLocked(0), TailLocked(0), Table(0),
	Size(0), Valid(false), memory_block_size(swap->GetBlockSize())
{
	Size = (size / memory_block_size);
	if (size % memory_block_size)
		Size++;
	Table = new CoreEntry [Size];
	if (!Table)
		return;
	Head = Table;
	Tail = &(Table[(Size-1)]);
	Table[0].SetNext(&(Table[1]));
	Table[0].SetIndex(0);
	Table[0].SetSwap(Swap);
	Table[(Size-1)].SetPrev(&(Table[(Size-2)]));
	Table[(Size-1)].SetIndex(Size-1);
	Table[(Size-1)].SetSwap(Swap);
	for (int n = 1; n < (Size-1); n++)
	{
		Table[n].SetIndex(n);
		Table[n].SetPrev(&(Table[(n-1)]));
		Table[n].SetNext(&(Table[(n+1)]));
		Table[n].SetSwap(Swap);
	}
	if (pre_allocate)
	{
		for (int n = 0; n < Size; n++)
		{
			Table[n].EnsureData();
			Table[n].ClearData();
		}
	}
	Valid = true;
}
G42CoreTable::~G42CoreTable(void)
{
	if (Table)
	{
		delete [] Table;
		Table = 0;
	}
}
bool
G42CoreTable::Load(G42PageEntry * page)
{
	if (!IsValid())
		return false;
	if (Tail == 0)
		return false;
	if (Tail->Load(page))
	{
GES((ges, "Adjusting list, page core ref = %d.\n", Tail->GetIndex()))
		page->SetCoreRef(Tail->GetIndex());
		CoreEntry * entry = Tail;
		if (entry->GetPrev() != 0)
		{
GES((ges, "moving entry to head of list.\n"))		
			entry->GetPrev()->SetNext(0);
			Tail = entry->GetPrev();
			Head->SetPrev(entry);
			entry->SetNext(Head);
			entry->SetPrev(0);
			Head = entry;
		}
		return true;
	}
	return false;
}
void
G42CoreTable::LoadAndFork(G42PageEntry * old_page, G42PageEntry * new_page)
{
	if (!IsValid())
		return;
	if (old_page->GetCoreRef() != -1)
	{
		if (!Table[old_page->GetCoreRef()].GetLock())
		{
			if (old_page->GetSeekRef() == -1)
			{
				if (Tail->GetPage() == 0)
				{
					Tail->EnsureData();
					memset(Tail->GetData(), 0, old_page->GetSize());
					memcpy(Tail->GetData(), Table[old_page->GetCoreRef()].GetData(),
						old_page->GetSize());
					Tail->SetPage(old_page);
					old_page->SetCoreRef(Tail->GetIndex());
					ToTopOfList(Tail->GetIndex());
					Table[new_page->GetCoreRef()].SetPage(new_page);
					ToTopOfList(new_page->GetCoreRef());
				}
				else
				{
					int new_ref = Swap->Write(Table[old_page->GetCoreRef()].GetData(),
						old_page->GetSize(), -1);
					if (new_ref != -1)
					{
						old_page->SetSeekRef(new_ref);
						old_page->SetModified(false);
						old_page->SetCoreRef(-1);
					}
					Table[new_page->GetCoreRef()].SetPage(new_page);
					new_page->SetModified();
					ToTopOfList(new_page->GetCoreRef());
				}
			}
			else
			{
				if (old_page->GetModified())
				{
					Swap->Write(Table[old_page->GetCoreRef()].GetData(),
						old_page->GetSize(), old_page->GetSeekRef());
					old_page->SetModified(false);
				}
				old_page->SetCoreRef(-1);
				new_page->SetSeekRef(-1);
				Table[new_page->GetCoreRef()].SetPage(new_page);
				new_page->SetModified();
				ToTopOfList(new_page->GetCoreRef());
			}
   	}
		else
		{
			if (Tail == 0)
				return;
			new_page->SetSeekRef(-1);
			Load(new_page);
			memcpy(Table[new_page->GetCoreRef()].GetData(),
				Table[old_page->GetCoreRef()].GetData(), old_page->GetSize());
		}
	}
	else
	{
		Load(new_page);
		new_page->SetSeekRef(-1);
		new_page->SetModified();
	}
}
void
G42CoreTable::Unload(int16 index)
{
	if (!IsValid())
		return;
	if (index != -1)
	{
		if (Table[index].GetLock())
			return;
		Table[index].Unload();
		CoreEntry * entry = &(Table[index]);
		if (entry != Tail)
		{
			if (entry == Head)
			{
				Head = entry->GetNext();
				Head->SetPrev(0);
			}
			else
			{
				entry->GetPrev()->SetNext(entry->GetNext());
				entry->GetNext()->SetPrev(entry->GetPrev());
			}
			entry->SetPrev(Tail);
			entry->SetNext(0);
			Tail->SetNext(entry);
			Tail = entry;
		}
		return;
	}
	for (int n = 0; n < Size; n++)
	{
		if (!Table[n].GetLock())
			Table[n].Unload();
	}
}
void
G42CoreTable::Lock(int16 index)
{
	if (!IsValid())
		return;
	if (index != -1)
	{
		Table[index].Lock();
		if (Table[index].GetLock() > 1)
			return;
		CoreEntry * entry = &(Table[index]);
		if (entry == Tail)
		{
			Tail = entry->GetPrev();
			if (Tail == 0)
				Head = 0;
			else
				Tail->SetNext(0);
		}
		else if (entry == Head)
		{
			Head = entry->GetNext();
			Head->SetPrev(0);
		}
		else
		{
			entry->GetPrev()->SetNext(entry->GetNext());
			entry->GetNext()->SetPrev(entry->GetPrev());
		}
		entry->SetNext(HeadLocked);
		entry->SetPrev(0);
		if (HeadLocked)
			HeadLocked->SetPrev(entry);
		else
      	TailLocked = entry;
		HeadLocked = entry;
		return;
	}
	for (int n = 0; n < Size; n++)
	{
		Table[n].Lock();
		if (Table[n].GetLock() > 1)
			continue;
		CoreEntry * entry = &(Table[n]);
		entry->SetNext(HeadLocked);
		entry->SetPrev(0);
		if (HeadLocked)
			HeadLocked->SetPrev(entry);
		else
			TailLocked = entry;
		HeadLocked = entry;
	}
	Head = 0;
   Tail = 0;
}
void
G42CoreTable::Unlock(int16 index)
{
	if (!IsValid())
		return;
	if (!Table[index].GetLock())
   	return;
	if (index != -1)
	{
		Table[index].Unlock();
		if (Table[index].GetLock())
			return;
		CoreEntry * entry = &(Table[index]);
		if (entry == TailLocked)
		{
			TailLocked = entry->GetPrev();
			if (TailLocked == 0)
				HeadLocked = 0;
			else
				TailLocked->SetNext(0);
		}
		else if (entry == HeadLocked)
		{
			HeadLocked = entry->GetNext();
			HeadLocked->SetPrev(0);
		}
		else
		{
			entry->GetPrev()->SetNext(entry->GetNext());
			entry->GetNext()->SetPrev(entry->GetPrev());
		}
		entry->SetNext(Head);
		entry->SetPrev(0);
		if (Head)
			Head->SetPrev(entry);
		else
      	Tail = entry;
		Head = entry;
		return;
	}
	for (int16 n = 0; n < Size; n++)
	{
		Table[n].Unlock();
		if (Table[n].GetLock())
			continue;
		CoreEntry * entry = &(Table[n]);
		entry->SetNext(Head);
		entry->SetPrev(0);
		if (Head)
			Head->SetPrev(entry);
		else
      	Tail = entry;
		Head = entry;
	}
	HeadLocked = 0;
   TailLocked = 0;
}
void
G42CoreTable::ToTopOfList(uint index)
{
	CoreEntry * entry = &(Table[index]);
	if (entry == Head)
		return;
	if (entry == Tail)
	{
		Tail = entry->GetPrev();
		Tail->SetNext(0);
	}
	else
	{
		entry->GetPrev()->SetNext(entry->GetNext());
		entry->GetNext()->SetPrev(entry->GetPrev());
	}
	Head->SetPrev(entry);
	entry->SetNext(Head);
	Head = entry;
	Head->SetPrev(0);
}
uint
G42CoreTable::GetUnlockedBlocks(void) const
{
	uint count = 0;
	for (int n = 0; n < Size; n++)
	{
		if (!Table[n].GetLock())
			count++;
	}
   return count;
}
G42Data::G42Data(G42CoreTable * core, G42PageTable * page)
	: Core(core), Page(page), SwapMan(0), OwnsMembers(false)
{
	SwapMan = Core->GetSwap();
}
G42Data::G42Data(int32 size, const char * name, bool pre_allocate)
	: Core(0), Page(0), SwapMan(0), OwnsMembers(true)
{
	if (!name)
	{
#ifdef MSWIN
#ifdef WIN32
		// note: never tested
		char fname [MAX_PATH];
		char path [MAX_PATH];
		path[0] = '\0';
		fname[0] = '\0';
		GetTempPath(MAX_PATH, path);
		GetTempFileName(path, "WI", 0, fname);
#else
		char fname [144];
		GetTempFileName(0, "WI", 0, fname);
#endif
		SwapMan = new G42SwapManager(fname);
#endif
	}
   else
		SwapMan = new G42SwapManager(name);
 	Core = new G42CoreTable(SwapMan, size, pre_allocate);
   Page = new G42PageTable();
}
G42Data::~G42Data(void)
{
	if (OwnsMembers)
   {
   	delete Page;
      delete Core;
      delete SwapMan;
	// (m.3)  CodeWarrior doesn't like implicit casts
      Page = 0;
      Core = 0;
      SwapMan = 0;
   }
}
int
G42Data::AddPage(int32 size)
{
	return Page->AddPage(size);
}
void
G42Data::DeletePage(uint index)
{
	if ((*Page)[index]->GetRefCount() < 2)
	{
		if ((*Page)[index]->GetCoreRef() != -1)
		{
			Core->Unlock((*Page)[index]->GetCoreRef());
			Core->Unload((*Page)[index]->GetCoreRef());
		}
		else if ((*Page)[index]->GetSeekRef() != -1)
			Core->GetSwap()->FreeEntry((*Page)[index]->GetSeekRef());
	}
	Page->DeletePage(index);
}
int
G42Data::ForkPage(uint index)
{
	if ((*Page)[index]->GetRefCount() < 2)
		return index;
	int new_index = Page->ForkPage(index);
	if (new_index == -1)
		return -1;
	(*Page)[index]->DecRefCount();
	Core->LoadAndFork((*Page)[index], (*Page)[new_index]);
	return new_index;
}
void
G42Data::Lock(uint index)
{
	int core_ref = (*Page)[index]->GetCoreRef();
	if (core_ref != -1)
		Core->Lock(core_ref);
}
void
G42Data::Unlock(uint index)
{
	int core_ref = (*Page)[index]->GetCoreRef();
	if (core_ref != -1)
		Core->Unlock(core_ref);
}
void
G42Data::PreLoad(uint * index_array, uint size)
{
	uint num_free = Core->GetUnlockedBlocks();
	uint out [64];
	memset(out, 0, 64 * sizeof (uint));
	uint out_count = 0;
	uint n = 0;
	while (num_free && (n < size))
	{
		if ((*Page)[index_array[n]]->GetCoreRef() == -1)
		{
			out[out_count++] = index_array[n];
			num_free--;
		}
		else
		{
			if (Core->IsLocked((*Page)[index_array[n]]->GetCoreRef()))
			{
				n++;
				continue;
			}
			Core->ToTopOfList((*Page)[index_array[n]]->GetCoreRef());
         num_free--;
		}
		n++;
	}
	for (n = 0; n < out_count; n++)
	{
		Core->Load((*Page)[out[n]]);
	}
}
unsigned char *
G42Data::operator[] (int index) const
{
	int core_ref = (*Page)[index]->GetCoreRef();
	if (core_ref != -1)
		return Core->GetData(core_ref);
GES((ges, "Loading index %d\n", index))
	if (Core->Load((*Page)[index]))
		return Core->GetData((*Page)[index]->GetCoreRef());
	// Error, Could not load
	return 0;
}

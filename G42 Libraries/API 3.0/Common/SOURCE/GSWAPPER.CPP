#include "gswapper.h"
#ifdef MACOS
#include <unistd.h>
#endif
#include "gtmpmem.h"
/*  Swapper Modifications for 3-27-1997: 
There is a new class, called G42SwapManager that allows you to specify a
count for the number of files to use as swapfiles.
To the client, it looks a lot like the old swapper.  So other than construction, all
the functionality should be transparent.  Basically, the work that remains is going 
to be in G42SwapManager::WriteBlock.  ReadBlock and FreeBlock simply pass on to
whatever swapper.
The problem is to pick a swapper to send the block to on WriteBlock.
I tried to get everything else in place today to be able to begin experimenting.
Presently the constructor for G42SwapManager has a single parameter for the number 
of files.  I expect parameters to be added as different settings are found optimal.
Caveat:  I did not get quite as far as I wanted -- I was hoping to open up a lot
of the parameters to G42Swapper via Set calls, and then have these in the new class 
G42SwapManager.
One function in particular that should be useful is the new SetDeferCompaction().
When true, compaction never occurs.  This would be useful for when a whole slew of
frees are coming such as when the app shuts down.
The new class has been tested in the swap tester.
BTW I was thinking a nice range for the number of swapfiles would be 1..20
*/
/* -- Notes on Swap File Compaction --
	There are 4 conditions that will trigger a compaction:
	1.  User requests compaction via a CompactASAP call.
	2.  Swap file's efficiency falls below G42Swapper_Minimum_Efficiency
	3.  Swapped Bytes * 100 / Available space exceeds G42Swapper_Safe_Capacity 
	4.  Swapped Bytes == 0 [Special case which truncates the file]  */
/*** G42Swapper ***/
G42Swapper::G42Swapper (const G42FilePath file_path, 
	const G42FilePath file_path_alt, intf min_efficiency,
	intf safe_capacity, intf high_efficiency,
	intf safety_buffer)
:	ActiveFilePath (file_path), ActiveFilePos (0), ActiveIndex (0), 
	CompactASAP (false), Efficiency (0), FilePath (file_path), 
	FilePathAlt (file_path_alt), 
	#ifdef DEBUG
	LogFile (0),
	#endif
 	HighEfficiency (high_efficiency), NumCompactions (0), NumEntries (0), 
 	NumFreeEntries (0), SafeCapacity (safe_capacity), 
	SafetyBuffer (safety_buffer), SwapFile (0), SwapFileSize (0),
 	SwappedBytes (0), MinimumEfficiency (min_efficiency), 
	UsingAltFile (false), WriteCount (0)
{
	Init (file_path_alt, min_efficiency);
}
G42Swapper::G42Swapper (const G42FilePath file_path,
	const G42FilePath file_path_alt, G42Notifier & notifier,
	intf min_efficiency,
	intf safe_capacity, intf high_efficiency,
	intf safety_buffer)
:	ActiveFilePath (file_path), ActiveFilePos (0), ActiveIndex (0),
	CompactASAP (false), Efficiency (0), FilePath (file_path),
	FilePathAlt (file_path_alt), Notifier(notifier),
	#ifdef DEBUG
	LogFile (0),
	#endif
	HighEfficiency (high_efficiency), NumCompactions (0), NumEntries (0),
	NumFreeEntries (0), SafeCapacity (safe_capacity),
	SafetyBuffer (safety_buffer), SwapFile (0), SwapFileSize (0),
	SwappedBytes (0), MinimumEfficiency (min_efficiency),
	UsingAltFile (false), WriteCount (0)
{
	Init (file_path_alt, min_efficiency);
}
#ifdef DEBUG
/*  Debugging version for log file  */
G42Swapper::G42Swapper (const G42FilePath log_path, bool,
	const G42FilePath file_path, const G42FilePath file_path_alt,  
	intf min_efficiency, intf safe_capacity, intf high_efficiency,
	intf safety_buffer)
 :	ActiveFilePath (file_path), ActiveFilePos (0), ActiveIndex (0), 
	CompactASAP (false), Efficiency (0), FilePath (file_path), 
	FilePathAlt (file_path_alt),
	#ifdef DEBUG
	LogFile (0),
	#endif
 	HighEfficiency (high_efficiency), NumCompactions (0), NumEntries (0), 
 	NumFreeEntries (0), SafeCapacity (safe_capacity), 
 	SafetyBuffer (safety_buffer), SwapFile (0), SwapFileSize (0), 
 	SwappedBytes (0), MinimumEfficiency (min_efficiency), 
 	UsingAltFile (false), WriteCount (0)
{
	Init (file_path_alt, min_efficiency);
	if (LogFile == 0)
	{
		LogFile 	= new G42File (log_path, G42File::OpenWriteUpdate,
			G42File::OpenAlways);
	}
}
#endif
void
G42Swapper::Init (const G42FilePath file_path_alt, intf min_efficiency)
{
	/*  Create and open the swap file  */
	SwapFile = new G42File (ActiveFilePath, G42File::OpenWriteUpdate,
		G42File::OpenAlways);
	if (file_path_alt == 0)
	{
		throw G42InvalidParameterException();
	}
	TruncateOnEmpty = (min_efficiency > 0) ? true : false;
}
G42Swapper::~G42Swapper (void)
{
	if (SwapFile)
	{
		delete SwapFile; 	// This also closes the file
		SwapFile = 0;
	}
	#ifdef MSWIN // in MSWIN, both paths are probably created already
	Unlink (FilePath);
	Unlink (FilePathAlt);
	#ifdef DEBUG
	if (LogFile)
	{
		delete LogFile;
		LogFile = 0;
	}
	#endif
	#else
	Unlink (ActiveFilePath);
	#endif
	for (intf i = 0; i < NumEntries; i++)
		delete Entries [i];
}
void
G42Swapper::CompactSwapFile (void)
{
	/*  Make sure we don't overflow the hard drive, background threads notwithstanding  */
	int32 space = GetDiskSpace (ActiveFilePath) - SafetyBuffer;
	space = max ((int32) 0, space);
	/*  Don't compact if there are no entries or efficiency already high or deferred */
	if (SwappedBytes == 0 || Efficiency > HighEfficiency
		|| SwappedBytes > space || DeferCompaction) {return;}
	/*  Step 1: Create and open the new swap file  */
	G42FilePath new_file_path (FilePathAlt); 
	G42FilePath old_file_path (FilePath);
	if (UsingAltFile)
	{
		new_file_path = FilePath;
		old_file_path = FilePathAlt;
	}
	G42File * new_file = new G42File (new_file_path, 
		G42File::OpenWriteUpdate, G42File::OpenAlways);
	intf new_count = 0;								// Number of new entries
	G42Vector <G42SwapEntry *> new_entries;	// Temp. stores new entries
	int32 new_file_size = 0;						
	byte * buffer = 0;	
	try 
	{
		/*  Step 2: Copy the swapped data over to the new file  */
		SwapFile -> Rewind();
		for (intf i = 0; i < NumEntries; i++)
		{
			if (! Entries [i] -> Free)
			{
				int32 size = Entries [i] -> ActiveBlockSize;
				G42SwapEntry * entry = MakeNewEntry (size, 0);
				buffer = new byte [size];	
				G42ThrowIfNull (buffer);
				SwapFile -> SetPosition (Entries [i] -> Offset);
				SwapFile -> Read (buffer, size);
				new_file -> Write (buffer, size);
				delete [] buffer;
				buffer = 0;
				/*  Update new entries  */
				entry->ActiveBlockSize = size;
				entry->Size = size;
				entry->Index = new_count;
				entry->Offset = new_file_size;
				new_entries [new_count] = entry;
				new_file_size += size;
				new_count++;
			}
		}
	}
	catch (...)
	{
		/*  Something failed, probably disk space, kill new_entries  */
		for (intf i = 0; i < new_count; i++)
		{	
			delete new_entries [i];
		}
		if (buffer)
		{
			delete [] buffer;
		}
		/*  Kill the new temp swapfile  */
		delete new_file;
		Unlink (new_file_path);
		#ifdef DEBUG
		if (LogFile)
		{
			char buffer [256];
			sprintf (buffer, "*** Compaction Failed *** \n");
			LogFile -> Write ((const byte *) buffer, strlen (buffer));
			sprintf (buffer, "\tCompactions: %d  NumEntries: %d NumFrees: "
				"%d SwappedBytes: %d FileSize: %d  Efficiency: %d\n",			
				NumCompactions, NumEntries, NumFreeEntries, SwappedBytes, 
				SwapFileSize, Efficiency);
			LogFile -> Write ((const byte *) buffer, strlen (buffer));
		}
		#endif
		/*  Bail out  */
		return;	
	}
	/* Update the Entries List  */
	new_count = 0;	// Ok to reuse new_count
	for (intf i = 0; i < NumEntries; i++)
	{
		G42SwapEntry * entry = Entries [i];
		if (entry -> Free)
		{
			delete Entries [i];
		}
		else
		{
			entry -> ActiveBlockSize = new_entries [new_count] -> ActiveBlockSize;
			entry -> Size = new_entries [new_count] -> Size;
			entry -> Index = new_entries [new_count] -> Index;
			entry -> Offset = new_entries [new_count] -> Offset;
			Entries [new_count] = entry;
			delete new_entries [new_count];
			new_count++;
		}
	}
	/*  Step 4: Copy new elements over to Entries  */	
	NumFreeEntries = 0;
	NumEntries = new_count;
		
	/*  Step 5: Update files & stuff  */
	delete SwapFile;
	Unlink (old_file_path);
	SwapFile = new_file;
	ActiveFilePath = new_file_path;
	UsingAltFile = ! UsingAltFile;
	SwapFile -> Rewind();
	ActiveIndex = 0;
	SwappedBytes = new_file_size;
	UpdateCompaction (0);
	NumCompactions++;
	#ifdef DEBUG
	if (LogFile)
	{
		char buffer [256];
		sprintf (buffer, "*** Compaction *** \n");
		LogFile -> Write ((const byte *) buffer, strlen (buffer));
		sprintf (buffer, "\tCompactions: %d  NumEntries: %d NumFrees: "
			"%d SwappedBytes: %d FileSize: %d  Efficiency: %d\n",			
			NumCompactions, NumEntries, NumFreeEntries, SwappedBytes, 
			SwapFileSize, Efficiency);
		LogFile -> Write ((const byte *) buffer, strlen (buffer));
	}
	#endif
}
/*  Frees a previously stored [written] block. Creates a vacancy in the file  */
void
G42Swapper::FreeHandle (G42SwapHandle handle) 	
{
	if (handle -> Free) {
		throw G42SwapperHandleFreeException ();
	}
	/*  Update compaction variables  */
	UpdateCompaction (-handle -> ActiveBlockSize);
	/*  Set entry free and append to the end of the free list  */
	handle -> ActiveBlockSize = 0;
	handle -> Free = true;
	FreeEntries [NumFreeEntries] = handle;  
	NumFreeEntries++;
	
	if (TruncateOnEmpty && NumFreeEntries == NumEntries) 
	{	
		/*  If this happens we have no data on disk and want to truncate file  */
		delete SwapFile;
		Unlink (ActiveFilePath);
		SwapFile = new G42File (ActiveFilePath, G42File::OpenWriteUpdate, 
			G42File::OpenAlways);
		for (intf i = 0; i < NumEntries; i++)
			delete Entries [i];
		Entries.Clear();
		NumEntries = NumFreeEntries = SwapFileSize = 0;
	}
	#ifdef DEBUG
	if (LogFile)
	{
		char buffer [256];
		sprintf (buffer, "Free Handle: %X Index: %d Offset: %d (Active) Size %d\n", 
			handle, handle -> Index, handle -> Offset, handle -> ActiveBlockSize);
		LogFile -> Write ((const byte *) buffer, strlen (buffer));
			sprintf (buffer, "\tCompactions: %d  NumEntries: %d NumFrees: "
				"%d SwappedBytes: %d FileSize: %d  Efficiency: %d\n",			
				NumCompactions, NumEntries, NumFreeEntries, SwappedBytes, 
				SwapFileSize, Efficiency);
		LogFile -> Write ((const byte *) buffer, strlen (buffer));
	}
	#endif
	if (CompactASAP || Efficiency < MinimumEfficiency)
	{
		CompactSwapFile();
		CompactASAP = false;
	}
}
G42SwapEntry
*G42Swapper::MakeNewEntry (int32 size, intf index, int32 offset)
{
	/*  Find a place for the entry at the end of the file  */
	if (offset == -1) {
		offset =	SwapFile -> GetSize();
	}
	G42SwapEntry *entry = new G42SwapEntry (size, index, offset);
	G42ThrowIfNull (entry);
	return entry;
}
/*	Reads a stored [written] block back in from the file w/o creating a vacancy */
void
G42Swapper::ReadBlock (G42SwapHandle handle, byte * buffer) 
{
	int32 pos = handle -> Offset;
	SwapFile -> SetPosition (pos);
	SwapFile -> Read (buffer, handle -> ActiveBlockSize);	
	/*  Save the old position (don't know why right now)  */
	ActiveFilePos = pos;
	#ifdef DEBUG
	if (LogFile)
	{
		char buffer [256];
		sprintf (buffer, "Read Handle: %X Index: %d Offset: %d (Active) Size %d\n", 
			handle, handle -> Index, handle -> Offset, handle -> ActiveBlockSize);
		LogFile -> Write ((const byte *) buffer, strlen (buffer));
			sprintf (buffer, "\tCompactions: %d  NumEntries: %d NumFrees: "
				"%d SwappedBytes: %d FileSize: %d  Efficiency: %d\n",			
				NumCompactions, NumEntries, NumFreeEntries, SwappedBytes, 
				SwapFileSize, Efficiency);
		LogFile -> Write ((const byte *) buffer, strlen (buffer));
	}
	#endif
}
/*  Set the configurable settings for the swapper  */
void
G42Swapper::SetHighEfficiency (intf efficiency)
{
	if (efficiency < 0 || efficiency > 100)
		throw G42InvalidParameterException ();
	HighEfficiency = efficiency;
}
void
G42Swapper::SetSafetyBuffer (intf size)
{
	if (size < 0 || size > 100)
		throw G42InvalidParameterException ();
	SafetyBuffer = size;
}
void
G42Swapper::SetMinimumEfficiency (intf min)
{
	if (min < 0 || min > 100)
		throw G42InvalidParameterException ();
	MinimumEfficiency = min;
}
void
G42Swapper::SetSafeCapacity (intf capacity)
{
	if (capacity < 0 || capacity > 100)
		throw G42InvalidParameterException ();
	SafeCapacity = capacity;
}
void
G42Swapper::UpdateCompaction (int32 size)
{
	SwappedBytes += size;
	SwapFileSize = SwapFile -> GetSize();
	Efficiency = (SwapFileSize > 0) ? (int32)((double)SwappedBytes * 100.0 /
		(double)SwapFileSize) : 100;
	#ifdef DEBUG
	if (SwappedBytes < 0) {
		throw (G42Exception(0));
	}
	#endif
}
/*	Stores a block of data into the swap file and returns a handle to it */
#ifdef DEBUG
G42SwapHandle
G42Swapper::WriteBlock (const byte * buffer, const int32 size, const int32 id)
#else
G42SwapHandle
G42Swapper::WriteBlock (const byte * buffer, const int32 size)
#endif
{
	int32 space = GetDiskSpace (ActiveFilePath) - G42Swapper_Safety_Buffer;
	space = max ((int32) 0, space);
	/*  Bail out if space request is too large  */
	if (size > space) {
		throw G42SwapperNoDiskSpaceException ();
	}
	/*  Try to anticipate a large, fragmented swapfile  */
	if ((int)((double)SwappedBytes * 100.0 /
		(double)space) > SafeCapacity) {
		CompactSwapFile();
	}
	/*  Find a free slot, if available, otherwise this will be an append  */
	intf active_index;
	intf free_index = 0;
	bool found = false;
	while (free_index < NumFreeEntries && ! found)
	{
		if (FreeEntries [free_index] -> Size >= size) 
		{
			found = true;
			active_index = FreeEntries [free_index] -> Index;
		}	
		else 
		{
			free_index++;
		}
	}
	/*  Prepare for writing  */
	G42SwapHandle handle;	// This will become the return value
	int32	file_pos;
	if (found)
	{
		handle = Entries [active_index];
		file_pos = handle -> Offset;
	}	
	else {
		file_pos = SwapFile -> GetSize();		
	}
	/*  Write data into the file  */
	try 
	{
		SwapFile -> SetPosition (file_pos);
		SwapFile -> Write (buffer, size);		
	}
	catch (...) {
		throw G42SwapperNoDiskSpaceException ();
	}
	/*  Now update our internal elements  */
	if (found)
	{
		/*  Need to remove the entry from the free list  */	
		NumFreeEntries--;
		for (intf i = free_index; i < NumFreeEntries; i++)
			FreeEntries [i] = FreeEntries [i + 1];
		/*  Active file position to start of entry  */
		handle = Entries [active_index];
		/*  Reset ActiveBlockSize in entry  */
	 	handle -> ActiveBlockSize = size;
		handle -> Free = false;
	}	
	else
	{
		/*  Need to create a new entry  */
		NumEntries++;
		active_index = NumEntries - 1;
		Entries [active_index] = MakeNewEntry (size, active_index, file_pos);
		handle = Entries [active_index];
	}
	ActiveFilePos = file_pos;
	ActiveIndex = active_index;
	UpdateCompaction (size);
	#ifdef DEBUG
	handle -> Id = id;
	if (LogFile)
	{
		char buffer [256];
		sprintf (buffer, "Write Handle: %X Index: %d Offset: %d (Active) Size: %d  Reused: %d\n", 
			handle, handle -> Index, handle -> Offset, handle -> ActiveBlockSize, found);
		LogFile -> Write ((const byte *) buffer, strlen (buffer));
			sprintf (buffer, "\tCompactions: %d  NumEntries: %d NumFrees: "
				"%d SwappedBytes: %d FileSize: %d  Efficiency: %d\n",			
				NumCompactions, NumEntries, NumFreeEntries, SwappedBytes, 
				SwapFileSize, Efficiency);
		LogFile -> Write ((const byte *) buffer, strlen (buffer));
	}
	#endif
	WriteCount++;
	return handle;	
}
/*** G42SwapEntry ***/
G42SwapEntry::G42SwapEntry (const int32 size, const intf index, const int32 offset)
:  ActiveBlockSize (size), Free (false), Index (index), Offset (offset), 
	Size (size), SwapperIndex (0)
{}
/*** G42SwapManager ***/
/*  Need a MSWIN version of this and this should be in gfutil.h  */
#ifdef MACOS
void
G42SwapManager::MakeTempFilePath (G42FilePath & file_path, char * name,
	int16 index, const G42FilePath &)
{
	FSSpec fsSpec;
	int32 length = strlen (name);
	G42TempMem <char> buffer (length + 4);
	sprintf (buffer, "%s%d",name,index);
	short vRefNum;
	long	dirID;
	FindFolder (kOnSystemDisk, kTemporaryFolderType, kCreateFolder, &vRefNum, &dirID);
	::FSMakeFSSpec (vRefNum, dirID, c2pstr (buffer), &fsSpec);
	file_path = G42FilePath (&fsSpec);
}
#endif
#ifdef MSWIN
void
G42SwapManager::MakeTempFilePath (G42FilePath & file_path, char *,
	int16, const G42FilePath & path)
{
	G42APtr<char> temp_path(new char [260]);
	if (path)
		strcpy(temp_path, path);
	else
		GetTempPath(260, temp_path);
	G42APtr<char> temp_file_1(new char [260]);
	if (!GetTempFileName(temp_path, "g42", 0, temp_file_1))
	{
		throw G42SwapperNoTempFileException();
	}
	file_path = G42FilePath(temp_file_1);
}
#endif
G42SwapManager::G42SwapManager (intf num_files)
:
	CurrentSwapperIndex (0), NumSwappers (0),
	CurrentSizeLimit(5242880L), MaxNumSwappers(num_files)
{
	Init();
}
G42SwapManager::G42SwapManager (G42Notifier & notifier, intf num_files)
:
	CurrentSwapperIndex (0), NumSwappers (0), Notifier (notifier),
	CurrentSizeLimit(5242880L), MaxNumSwappers(num_files)
{
	Init();
}
G42SwapManager::G42SwapManager (const G42FilePath & swapper_path,
	intf num_files)
:
	CurrentSwapperIndex (0), NumSwappers (0), SwapperPath(swapper_path),
	CurrentSizeLimit(5242880L), MaxNumSwappers(num_files)
{
	Init();
}
G42SwapManager::G42SwapManager (const G42FilePath & swapper_path,
	G42Notifier & notifier, intf num_files)
:
	CurrentSwapperIndex (0), NumSwappers (0), Notifier (notifier),
	SwapperPath(swapper_path), CurrentSizeLimit(5242880L), MaxNumSwappers(num_files)
{
	Init();
}
void
G42SwapManager::Init(void)
{
}
void
G42SwapManager::InitSwapper(intf index)
{
	/*  Allocate the swappers  */
	for (intf i = NumSwappers; i <= index; i++)
	{
		/*  Looks a little strange but necessary for Mac  */
		#ifdef MACOS
		FSSpec fsSpec;
		G42FilePath  file_path (&fsSpec, 'None', 'Swap');
		G42FilePath  file_path_alt (&fsSpec, 'None', 'Swap');
		bool path_made = true;
		try
		{
			MakeTempFilePath (file_path, "SwapFile", i, SwapperPath);
		}
		catch (G42SwapperNoTempFileException &)
		{
			if (SwapperPath)
			{
				path_made = false;
			}
			else
			{
				throw;
			}
		}
		catch (...)
		{
			throw;
		}
		if (!path_made)
		{
			MakeTempFilePath (file_path, "SwapFile", i, 0);
			#ifdef MSWIN	// For Mac, something needs to be done here
			G42InvalidSwapperPath msg(SwapperPath);
			Notifier->ProcessMessage(&msg);
			#endif
			SwapperPath = 0;
		}
		MakeTempFilePath (file_path_alt, "SwapFileAlt", i, SwapperPath);
		Swappers [i] = new G42Swapper (file_path, file_path_alt, Notifier);
		#endif
		#ifdef MSWIN
		G42FilePath  file_path(0);
		G42FilePath  file_path_alt(0);
		bool path_made = true;
		try
		{
			MakeTempFilePath (file_path, "SwapFile", (int16)i, SwapperPath);
		}
		catch (G42SwapperNoTempFileException &)
		{
			if (SwapperPath)
			{
				path_made = false;
			}
			else
			{
				throw;
			}
		}
		catch (...)
		{
			throw;
		}
		if (!path_made)
		{
			MakeTempFilePath (file_path, "SwapFile", (int16)i, 0);
			G42InvalidSwapperPath msg(SwapperPath);
			Notifier->ProcessMessage(&msg);
			SwapperPath = 0;
		}
		MakeTempFilePath (file_path_alt, "SwapFileAlt", (int16)i, SwapperPath);
		Swappers [i] = new G42Swapper (file_path, file_path_alt, Notifier);
		#endif
		/*  If something fails on construction of swapper, it should throw  */
	}
	NumSwappers = index + 1;
}
G42SwapManager::~G42SwapManager (void)
{
	/*  Delete the swappers  */
	for (int16 i = 0; i < NumSwappers; i++)
	{
		delete Swappers [i];
	}
}
void
G42SwapManager::FreeHandle (G42SwapHandle handle)
{
	Swappers [handle -> SwapperIndex] -> FreeHandle (handle);
}
intf
G42SwapManager::NextRoundRobin (void)
{
#if 0
	if (NextSwapperIndex == (NumSwappers - 1))
	{
		NextSwapperIndex = 0;
	}
	return NextSwapperIndex++;
#endif
	while (true)
	{
		intf n;
		for (n = CurrentSwapperIndex; n < MaxNumSwappers; n++)
		{
			if (NumSwappers <= n)
				InitSwapper(n);
			if (Swappers[n]->GetFileSize() < CurrentSizeLimit)
				return n;
		}
		for (n = 0; n < CurrentSwapperIndex; n++)
		{
			if (Swappers[n]->GetFileSize() < CurrentSizeLimit)
				return n;
		}
		CurrentSizeLimit <<= 1;
	}
}
void
G42SwapManager::SetDeferCompaction(bool defer)
{
	for (int16 i = 0; i < NumSwappers; i++)
		Swappers[i]->SetDeferCompaction(defer);
}
void
G42SwapManager::ReadBlock (G42SwapHandle handle, byte * buffer)
{
	Swappers [handle -> SwapperIndex] -> ReadBlock (handle, buffer);
}
G42SwapHandle
#ifdef DEBUG
G42SwapManager::WriteBlock (const byte * buffer, const int32 size, const int32 id)
#else
G42SwapManager::WriteBlock (const byte * buffer, const int32 size)
#endif
{
	/*  Pick a swapper to send the block to  */
	CurrentSwapperIndex = NextRoundRobin();
	#if 0	// Some other examples
	index = FindAvailableEntry();		// Next swapper w/a big enough entry available
	index = FindLowestEfficiency();	// Swapper which is running at the lowest efficiency
	index = FindActive();				// Current swapper that will be active until swap time
												// exceeds some threshold.
	#endif
	#ifdef DEBUG
	G42SwapHandle handle = Swappers [CurrentSwapperIndex] -> WriteBlock (buffer, size, id);
	#else
	G42SwapHandle handle = Swappers [CurrentSwapperIndex] -> WriteBlock (buffer, size);
	#endif
	handle -> SwapperIndex = (int16)CurrentSwapperIndex;
	return handle;
}
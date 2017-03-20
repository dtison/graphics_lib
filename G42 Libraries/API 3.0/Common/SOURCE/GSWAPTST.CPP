#include "gswaptst.h"
G42SwapTest::G42SwapTest (G42FilePath file_path, G42FilePath file_path_alt) 
: 	ActiveHandleCount (0), BytesRead (0), BytesWritten (0),  
	EventCount (0), FreeCount (0), LogFile (0), LogCreate (false), 
	ReadCount (0), Swapper (0), TotalEfficiency (0), WriteCount (0)
{
	#ifdef DEBUG	// Stuff for swapper with a logfile
	#ifdef MACOS	
	FSSpec fsSpec;
	OSErr err = ::FSMakeFSSpec (0, 0, "\pSwapLog", &fsSpec);
	G42FilePath file_path_log (&fsSpec, 'CWIE', 'TEXT');
	// Kill file just in case
	::FSpDelete (file_path_log);
	#endif
//	Swapper = new G42Swapper (file_path_log, true, file_path, file_path_alt);
	#endif
	Swapper = new G42SwapManager;
	G42ThrowIfNull (Swapper);
}
G42SwapTest::~G42SwapTest (void)
{
	if (LogFile)
	{
		delete LogFile;
		LogFile = 0;
	}
	if (Swapper)
	{
		delete Swapper;
		Swapper = 0;
	}
}
void 
G42SwapTest::ClearBuffer (byte * buffer, int32 count)
{
	/*  Initialize buffer to 0  */
	int32 * ptr = (int32 *) buffer;
	for (int32 i = 0; i < count; i++)
		*ptr++ = 0;
}
bool 
G42SwapTest::CompareBlockValue (byte * buffer, int32 count, int32 value)
{
	int32 * ptr = (int32 *) buffer;
	for (int32 i = 0; i < count; i++)
		if (*ptr++ != value)
			return false;
		
	return true;	
}
//#define RANDOM_COMPACTION
int16
G42SwapTest::DoRandomTest (int16 count, G42FilePath * file_path, bool create)
{
	/*  If you pass in a file_path, then: 1. create = false, create log,
	    read from log 2. Else: create log  */
	if (file_path)
	{
		G42File::G42OpenType type = create ? G42File::OpenWriteUpdate :
			G42File::OpenRead;
		LogFile 		= new G42File (*file_path, type,
			G42File::OpenAlways);
		LogCreate 	= create;
	}
	do 	
	{
		#ifdef RANDOM_COMPACTION
		uint16 choice = GetRandom (4);
		#else
		uint16 choice = GetRandom (3);
		#endif
		if (LogFile && ! LogCreate)
		{
			/*  Pull choice from log file instead */
			LogFile -> Read ((byte *) &choice, sizeof (choice));	
		}
		switch (choice)
		{
			case G42Write_Block:		
				WriteBlock();
			break;
			case G42Read_Block:		
				ReadBlock();
			break;
			case G42Free_Block:		
				FreeBlock();
			break;
			case G42Compact:
#if 0
				Swapper -> SetCompactASAP (true);	
				EventCount++;
#endif
			break;
		} 
#if 0
		TotalEfficiency += Swapper -> GetEfficiency();
#endif
	} while (EventCount < count);
	return 0;
}
uint16
G42SwapTest::GetRandom (uint16 range)
{
	#ifdef MSWIN
	uint16 v = (uint16) random (range);
	#endif
	#ifdef MACOS
	uint16 r = Random();
	uint16 v = (r % range);
	#endif
	return v;
}
void 
G42SwapTest::FreeBlock (void)
{
	if (ActiveHandleCount == 0)
		return;
	/*  Need a random index to free   */
	uint16 index 	= GetRandom ((uint16) ActiveHandleCount);
	if (LogFile && ! LogCreate)
	{
		/*  Pull data from log file instead */
		LogFile -> Read ((byte *) &index, sizeof (index));	
	}
	G42SwapHandle handle = ActiveHandles [index];
	Swapper -> FreeHandle (handle);
	/*  Remove the handle from our actives list  */
	ActiveHandleCount--;
	for (int32 i = index; i < ActiveHandleCount; i++)
	{
		ActiveHandles [i] = ActiveHandles [i + 1];
	}
	FreeCount++;
	EventCount++;
	if (LogFile && LogCreate)
	{
		/*  Put data into log file */
		uint16 choice = G42Free_Block;
		LogFile -> Write ((byte *) &choice, sizeof (choice));	
		LogFile -> Write ((byte *) &index, sizeof (index));	
	}
}
byte * 
G42SwapTest::MakeNewBuffer (int32 value, int32 count)
{
	byte * buffer = new byte [count << 2];
	G42ThrowIfNull (buffer);
	/*  Initialize buffer to value  */
	int32 * ptr = (int32 *) buffer;
	for (int32 i = 0; i < count; i++)
		*ptr++ = value;
	return buffer;
}
void 
G42SwapTest::ReadBlock (void)
{
	if (ActiveHandleCount == 0)
		return;
	/*  Need a random index to read from  */
	uint16 index = GetRandom ((uint16) ActiveHandleCount);
	if (LogFile && ! LogCreate)
	{
		/*  Pull data from log file instead */
		LogFile -> Read ((byte *) &index, sizeof (index));	
	}
	G42SwapHandle handle = ActiveHandles [index];
	int32 size 		= handle -> GetActiveSize();
	byte * buffer 	= new byte [size];
	Swapper -> ReadBlock (handle, buffer);
	if (! CompareBlockValue (buffer, (size >> 2), handle -> Id)) {
		throw (G42Exception(0));
	}
	delete [] buffer;
	ReadCount++;
	EventCount++;
	BytesRead += size;
	if (LogFile && LogCreate)
	{
		/*  Put data into log file */
		uint16 choice = G42Read_Block;
		LogFile -> Write ((byte *) &choice, sizeof (choice));	
		LogFile -> Write ((byte *) &index, sizeof (index));	
	}
}
void 
G42SwapTest::WriteBlock (void)
{
	/*  Grab a random number of items to write  */
	uint32 count = (GetRandom (1024) + 1); // Normalize to 1..1024
	count <<= 4;
	int32 size  = (count << 2);
	if (LogFile && ! LogCreate)
	{
		/*  Pull data from log file instead */
		LogFile -> Read ((byte *) &count, sizeof (count));	
		LogFile -> Read ((byte *) &size, sizeof (size));	
	}
	byte * buffer = MakeNewBuffer (WriteCount, count);
	G42SwapHandle handle = Swapper -> WriteBlock (buffer, size, WriteCount);
	delete [] buffer;
	ActiveHandles [ActiveHandleCount] = handle;
	ActiveHandleCount++;
	WriteCount++;
	EventCount++;
	BytesWritten += size;
	if (LogFile && LogCreate)
	{
		/*  Put data into log file */
		uint16 choice = G42Write_Block;
		LogFile -> Write ((byte *) &choice, sizeof (choice));	
		LogFile -> Write ((byte *) &count, sizeof (count));	
		LogFile -> Write ((byte *) &size, sizeof (size));	
	}
}

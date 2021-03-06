#if ! defined (GSWAPPER_H)
#define GSWAPPER_H
#include "gtype.h"
#include "gfile.h"
#include "gfutil.h"
#include "gtools.h"
#include "gnotify.h"
//----------------------------------------------------------------------
//  file:			gswapper.h
//  author:			David Ison
//	 date:				10/11/96
//	 revision:		3.0
//
//	 Administrates the swap file(s).
//----------------------------------------------------------------------
#define DEBUG
class G42SwapperHandleFreeException : public G42Exception
{
public:
	G42SwapperHandleFreeException(uintf error_id = G42_ERROR_SWAP_HANDLE)
			: G42Exception(error_id) {}
};
class G42SwapperNoDiskSpaceException : public G42Exception
{
public:
	G42SwapperNoDiskSpaceException(uintf error_id = G42_ERROR_SWAP_NODISK)
			: G42Exception(error_id) {}
};
class G42SwapperNoTempFileException : public G42Exception
{
public:
	G42SwapperNoTempFileException(uintf error_id = G42_ERROR_SWAP_NO_TEMP_FILE)
			: G42Exception(error_id) {}
};
class G42InvalidSwapperPath : public G42NotifyMessage
{
public:
	G42InvalidSwapperPath (const char * path) : Path(path) {}
	virtual ~G42InvalidSwapperPath (void) {}
	const char * GetPath(void) const
		{ return Path; }
private:
	const char * Path;
};
/*  Entry class  */
class G42SwapEntry
{
friend class		G42Swapper;			// More efficient for Swapper and SwapManager 
friend class		G42SwapManager;	// can access members directly
public:
						G42SwapEntry (const int32 size, const intf index, const int32 offset);
						~G42SwapEntry (void) {};
	int32				GetActiveSize (void) {return ActiveBlockSize;}
	#ifdef DEBUG
	int32				Id;
	#endif
private:
	int32				ActiveBlockSize;	// Size of the currently active block
	bool				Free;					// True when the entry is free
	intf				Index;				// Position in the entry list 
	int32				Offset;				// Offset in swapfile
	int32				Size;					// Size of this entry
	int16				SwapperIndex;		// When using swap manager, index of swapper storing entry
};
typedef  G42SwapEntry * G42SwapHandle;
const intf G42Swapper_Minimum_Efficiency 	= 25;
const intf G42Swapper_Safe_Capacity 		= 50;
const intf G42Swapper_High_Efficiency 		= 90;
const intf G42Swapper_Safety_Buffer 		= 1024 * 1024;
class G42Swapper
{
public:
	G42Swapper (const G42FilePath file_path, const G42FilePath file_path_alt = 0,
		intf min_efficiency = G42Swapper_Minimum_Efficiency,
		intf safe_capacity = G42Swapper_Safe_Capacity,
		intf high_efficiency = G42Swapper_High_Efficiency,
		intf safety_buffer = G42Swapper_Safety_Buffer);
	G42Swapper (const G42FilePath file_path, const G42FilePath file_path_alt,
		G42Notifier & notifier,
		intf min_efficiency = G42Swapper_Minimum_Efficiency,
		intf safe_capacity = G42Swapper_Safe_Capacity,
		intf high_efficiency = G42Swapper_High_Efficiency,
		intf safety_buffer = G42Swapper_Safety_Buffer);
	#ifdef DEBUG
	G42Swapper (const G42FilePath log_path, bool,
		const G42FilePath file_path, const G42FilePath file_path_alt = 0,
		intf min_efficiency = G42Swapper_Minimum_Efficiency,
		intf safe_capacity = G42Swapper_Safe_Capacity,
		intf high_efficiency = G42Swapper_High_Efficiency,
		intf safety_buffer = G42Swapper_Safety_Buffer);
	#endif
	~G42Swapper (void);
	void				FreeHandle (G42SwapHandle handle);
	int32				GetNumCompactions (void) {return NumCompactions;} 			// DEBUG thing?
	void				ReadBlock (G42SwapHandle handle, byte * buffer);
	void 				SetCompactASAP (const bool flag) {CompactASAP = flag;};  // Compact ASAP
	void				SetHighEfficiency (intf efficiency);
	void				SetMinimumEfficiency (intf min);	// Threshold compaction efficiency
	void				SetSafeCapacity (intf capacity);
	void				SetSafetyBuffer (intf size);
	void 				SetDeferCompaction (bool defer) {DeferCompaction = defer;}	
	void				SetTruncateOnEmpty (bool flag) {TruncateOnEmpty = flag;}
	#ifndef DEBUG
	G42SwapHandle	WriteBlock (const byte * buffer, const int32 size);
	#endif
	
	#ifdef DEBUG	// Some of these may not end up being "DEBUG" -- they're useful
	int32				GetEfficiency (void) {return Efficiency;}	
	G42SwapHandle	WriteBlock (const byte * buffer, const int32 size, const int32 id = 0);
	int32				GetFileSize (void) {return SwapFileSize;}
	#endif
	
private:
	void				Init (const G42FilePath file_path_alt, intf min_efficiency);
	// Variables
	G42FilePath		ActiveFilePath;		// Filepath of currently active swap file
	int32				ActiveFilePos;			// Position where next read / write will take place
	intf				ActiveIndex;			// Index of most recently accessed entry
	bool				CompactASAP;			// Set true when a compact ASAP request is made
	bool				DeferCompaction;		// Set true to disable compaction until this is false
	int32				Efficiency;				// Current efficiency percentage
	G42Vector 		<G42SwapEntry *> Entries;		// Entries list
	G42Vector 		<G42SwapEntry *> FreeEntries;	// Free entries list
	G42FilePath		FilePath;				// OS-independent ref to the swap file
	G42FilePath		FilePathAlt;			// OS-independent ref to the alt swap file
	#ifdef DEBUG
	G42File			* LogFile;
	#endif
	intf				HighEfficiency;
	intf				MinimumEfficiency;  	// Compact when (SwappedBytes / SwapFileSize) * 100 falls below this
	G42NotifierVerifier
						Notifier;				// For notifier messages when compacting
	int32				NumCompactions;		// Total number of compactions since construction
	intf				NumEntries;				// Total number of swap entries
	intf				NumFreeEntries;		// Total number of free swap entries
	intf				SafetyBuffer;
	intf				SafeCapacity;			
	G42File 			* SwapFile;				// _The_ swap file
	int32				SwapFileSize;			// Total size of swap file
	int32				SwappedBytes;			// Total number of bytes swapped
	bool				TruncateOnEmpty;		// Flag to truncate swapfile when all handles freed
	// Functions
	void				CompactSwapFile (void);	// Compacts file if compact callback returns true
	G42SwapEntry	* MakeNewEntry (int32 size, intf index, int32 offset = -1);
	void 				UpdateCompaction (int32 size);
	bool				UsingAltFile;				// True when swapfile is using FilePathAlt
	int32				WriteCount;					// Total number of write req's since construction
};
/*  Administrates a cache of swappers  */
class G42SwapManager
{
public:
	G42SwapManager (intf num_files = 20);
	G42SwapManager (const G42FilePath & swapper_path, intf num_files = 20);
	G42SwapManager (G42Notifier & notifier, intf num_files = 20);
	G42SwapManager (const G42FilePath & swapper_path, G42Notifier & notifier,
		intf num_files = 20);
		
	~G42SwapManager (void);
	void				FreeHandle (G42SwapHandle handle);
	void				ReadBlock (G42SwapHandle handle, byte * buffer);
	#ifdef DEBUG
	G42SwapHandle	WriteBlock (const byte * buffer, const int32 size, const int32 id = 0);
	#else
	G42SwapHandle	WriteBlock (const byte * buffer, const int32 size);
	#endif
	void SetDeferCompaction(bool defer);
	
private:
	void				Init (void);
	intf 				NextRoundRobin (void);
	/*  Temporary thing --  Needs to move to gfutil.h  */
	static void MakeTempFilePath (G42FilePath & file_path, char * name,
		int16 index, const G42FilePath & path);
	void InitSwapper(intf index);
	// Variables
	G42Vector 		<G42Swapper *> Swappers;
	intf				CurrentSwapperIndex;
	int32				CurrentSizeLimit;
	G42NotifierVerifier
						Notifier;
	intf 				NumSwappers;
	intf				MaxNumSwappers;
	G42FilePath		SwapperPath;
};
#endif	// Sentry for gswapper.h
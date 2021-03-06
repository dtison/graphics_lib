#if ! defined (GSWAPTST_H)
#define GSWAPTST_H
#include "gswapper.h"
const int8 G42Write_Block 	= 0;
const int8 G42Read_Block 	= 1;
const int8 G42Free_Block 	= 2;
const int8 G42Compact	 	= 3;
class G42File;
class G42SwapTest
{
public:
						G42SwapTest (G42FilePath file_path, 
							G42FilePath file_path_alt = 0);
	virtual			~G42SwapTest (void);
	int16				DoRandomTest (int16 count, G42FilePath * file_path = 0, 
							bool read = false);
	int32				GetBytesRead (void) {return BytesRead;}
	int32				GetBytesWritten (void) {return BytesWritten;}
	int32				GetEfficiency (void) {return (TotalEfficiency / EventCount);} 
	int32				GetEventCount (void) {return EventCount;} 
//	int32				GetFileSize (void) {return Swapper -> GetFileSize();}
	int32				GetFileSize (void) {return 0;}
	int32				GetFreeCount (void) {return FreeCount;}
//	int32				GetNumCompactions (void) {return Swapper -> GetNumCompactions();}
	int32				GetNumCompactions (void) {return 0;}
	int32				GetReadCount (void) {return ReadCount;}
	int32				GetWriteCount (void) {return WriteCount;}
private:
	G42Vector		<G42SwapHandle> ActiveHandles;
	int32				ActiveHandleCount;
	int32				BytesRead;
	int32				BytesWritten;
	int32				EventCount;	
	int32				FreeCount;
	bool				LogCreate;
	G42File			* LogFile;
	byte 				* MakeNewBuffer (int32 value, int32 count);
	int32				ReadCount;
	G42SwapManager	* Swapper;
	int32				TotalEfficiency;		// For calculating average efficiency
	int32				WriteCount;
	bool				CompareBlockValue (byte * buffer, int32 count, int32 value);
	void 				ClearBuffer (byte * buffer, int32 count);
	uint16			GetRandom (uint16 range);
	void 				FreeBlock (void);
	void 				ReadBlock (void);
	void 				WriteBlock (void);
};
#endif
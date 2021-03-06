// g42cache.h - implement simple disk cache
#if !defined(G42CACHE_H)
#define G42CACHE_H
#include "g42type.h"
#include "g42array.h"
class G42CacheObject
{
friend class G42Cache;
public:
	G42CacheObject(void);
	uint32 GetDataSize(void) const;
	bool GetData(byte * buffer);
   bool IsValid(void) const;
protected:
	virtual uint32 GetWriteInfoSize(void) = 0;
	virtual bool WriteInfo(FILE * fp) = 0;
	virtual bool ReadInfo(FILE * fp) = 0;
	virtual int GetId(void);
	uint32 GetReadInfoSize(void);
	void Write8(FILE * fp, uint8 value);
	void Write16(FILE * fp, uint16 value);
	void Write32(FILE * fp, uint32 value);
	uint8 Read8(FILE * fp);
	uint16 Read16(FILE * fp);
	uint32 Read32(FILE * fp);
	bool Valid;
	virtual ~G42CacheObject(void); // only Cache can delete this
private:
	uint32 InfoSize;
	char * DataFile;
	uint32 DataSize;
	G42Cache * Cache;
	uint32 SeekLocation;
	uint32 TouchIncrement;
	uint32 Touch;
};
class G42Cache
{
friend class G42ReadLockedCache;
friend class G42WriteLockedCache;
friend class G42CacheObject;
public:
	G42Cache(const char * file_path, int32 size, bool create_permanent_file = false);
	~G42Cache();
	bool IsValid(void)
		{ return Valid; }
   bool IsPermanent(void) const
   	{return ((CacheMode == PermanentFile) ? true : false);}
// New Things for the Cache Dialog
   int GetCacheNumber(void) const
   	{return NumObjectArray;}
   int32 GetCacheSize(void) const
   	{return TotalSize;}
   int32 GetCacheUsed(void) const
   	{return DataSize;}
   char * GetCacheDirectory(void) const
   	{return DirPath;}
   // New stuff for the Catalog type cache.
	int GetNumModified(void) const
   	{return NumModifed;}
	int GetNumAdded(void) const
   	{return NumAdded;}
	int GetNumDeleted(void) const
   	{return (NumObjectArray - NumUsed);}
	void IncNumModifed(int inc = 1)
   	{NumModifed += inc;}
	void IncNumAdded(int inc = 1)
   	{NumAdded += inc;}
	void IncNumUsed(int inc = 1)
   	{NumUsed += inc;}
protected:
	virtual G42CacheObject * CreateObject(int id) = 0;
	virtual uint32 GetHeaderID(void) = 0;
private:
	G42Array<G42CacheObject *> ObjectArray;
	int NumObjectArray;
	char * FilePath;
	char * FileName; // pointer into file portion of FilePath
	bool Valid;
	bool Modified;
	int WriteLock;
	int ReadLock;
	int TouchReadLock;
	int TouchWriteLock;
	uint32 TouchIncrementCounter;
	uint32 TouchCounter;
	uint32 WriteCounter;
	char * DirPath;
	int DirPathSize;
	enum G42CacheMode {DiskCache = 0, PermanentFile = 1};
	G42CacheMode CacheMode;
	uint32 NextSeekLocation;
	int32 TotalSize;
	int32 DataSize;
	bool NeedRead;
   // New stuff for the Catalog type cache.
   int 	NumModifed;
   int	NumAdded;
   int	NumUsed;
	// functions for the locked cache classes below
	void CheckSize(void);
	bool AddObject(G42CacheObject * object, byte * data,
		uint32 data_size);
	bool DeleteObject(uint index);
	bool ModifyObjectData(uint index, byte * data,
		uint32 data_size);
	bool ReadObjectInfo(void);
	bool WriteObjectInfo(void);
	bool WriteNewTouchCodes(void);
	bool UpdateTouchCodes(void);
	bool SetWriteLock(void);
	bool ClearWriteLock(void);
	bool SetReadLock(void);
	bool ClearReadLock(void);
	bool SetTouchWriteLock(void);
	bool ClearTouchWriteLock(void);
	bool SetTouchReadLock(void);
	bool ClearTouchReadLock(void);
   void PurgeCache(void);
   void SetCacheSize(int32 size);
};
class G42ReadLockedCache
{
public:
	G42ReadLockedCache(G42Cache & cache);
	~G42ReadLockedCache(void)
		{ if (Locked) Unlock(); Valid = false; }
	bool IsValid(void) const
		{	return Valid && Cache.Valid; }
	bool Unlock(void);
	const G42CacheObject * GetObject(uint index)
	{
		if (!IsValid() || index >= Cache.NumObjectArray)
			return 0;
		return Cache.ObjectArray[index];
	}
	uint GetNumObjects(void) const
	{
		if (!IsValid())
			return 0;
		return Cache.NumObjectArray;
	}
	uint32 GetSizeUsed(void) const
	{
		if (!IsValid())
			return 0;
		return Cache.DataSize;
	}
	uint32 GetTotalSize(void) const
	{
		if (!IsValid())
			return 0;
		return Cache.TotalSize;
	}
private:
	bool Locked;
	G42Cache & Cache;
	bool Touched;
	bool Valid;
};
class G42WriteLockedCache
{
public:
	G42WriteLockedCache(G42Cache & cache);
	~G42WriteLockedCache(void)
	{
		if (Locked)
			Unlock();
		Valid = false;
	}
	bool IsValid(void) const
		{	return Valid && Cache.Valid; }
	bool Unlock(void);
	G42CacheObject * GetObject(uint index);
	uint GetNumObjects(void) const;
	uint32 GetSizeUsed(void) const;
	uint32 GetTotalSize(void) const;
	bool AddObject(G42CacheObject * object, byte * data, uint32 data_size);
   bool DeleteObject(uint index);
	bool ModifyObjectData(uint index, byte * data,
		uint32 data_size);
	bool WriteObjectInfo(void);
	bool SetTotalSize(uint32 size);
	void MarkModified(void);
   void SetModified(bool mod = true)
   	{Cache.Modified = mod;}
   void PurgeCache(void);
   void SetCacheSize(int32 size);
private:
	bool Locked;
	bool Modified;
	G42Cache & Cache;
	bool Valid;
};
#endif // G42CACHE_H

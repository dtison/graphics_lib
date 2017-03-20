// g42thumb.h - header file for thumbnail creation
#if !defined(G42THUMB_H)
#define G42THUMB_H
#include "g42memry.h"
#include "g42image.h"
#include "g42ifile.h"
#include "g42cache.h"
class G42ThumbCacheObject : public G42CacheObject
{
public:
	const G42ImageInfo * GetInfo(void) const;
	const G42ImageInfo * GetThumbInfo(void) const;
	G42ThumbCacheObject(void);
	G42ThumbCacheObject(const G42ImageInfo * info,
		const G42ImageInfo * thumb_info, const char * file_name);
	const char * GetFileName(void) const;
	const char * GetFileNameProper(void) const;
	time_t GetModifiedTime(void) const;
   bool TimeEqual(time_t new_time) const;
protected:
	virtual uint32 GetWriteInfoSize(void);
	virtual bool WriteInfo(FILE * fp);
	virtual bool ReadInfo(FILE * fp);
	virtual int GetId(void);
	virtual ~G42ThumbCacheObject();
private:
	G42ImageInfo * Info;
	G42ImageInfo * ThumbInfo;
	char * FileName;
	char * FilePtr;
	int Platform;
};
class G42ThumbCache : public G42Cache
{
public:
	G42ThumbCache(const char * file_path, int32 size, bool create_catalog = false);
	~G42ThumbCache();
protected:
	virtual G42CacheObject * CreateObject(int id);
	virtual uint32 GetHeaderID(void);
};
class G42Thumb
{
public:
	G42Thumb(const char * file_path, G42Data * memory_manager,
		int width = 100, int height = 100);
	G42Thumb(const G42ThumbCacheObject * thumb, G42Data * memory_manager);
	~G42Thumb();
	bool AddToCache(G42ThumbCache & cache, bool override = false);
	G42Image * GetImage(void);
	G42Image * GetFullSizeImage(void);
	G42ImageInfo * GetInfo(void);
	bool IsValid();
	const char * GetFileName(void) const;
	const char * GetFilePath(void) const;
	bool IdleAction(void);
	G42ImageType GetImageType(void) const;
private:
	bool Valid;
	char * FilePath;
	char * FileName;
	G42Image * Image;
	G42Data * MemoryManager;
	G42ImageInfo * Info;
	enum G42IdleMode {NeedAllocateMode, ReadMode, StretchMode, DoneMode};
	G42IdleMode IdleMode;
	byte * ReadBuffer;
	int ReadBufferSize;
	FILE * FP;
   G42ImageFile * Reader;
	int MaxWidth;
	int MaxHeight;
	bool InCache;
	void Clear(void);
};
class G42ThumbHandler
{
	public:
		G42ThumbHandler(G42Data * memory_manager, const char * cache_name = 0,
      	int32 size = 1048576L, int update = 2,
			int max_width = 100, int max_height = 100);
		~G42ThumbHandler(void);
		bool SetDirectory(const char * dir_path);
		bool ClearDirectory(void);
		int GetNumThumbs()
			{ return NumThumbs; }
		G42Thumb * GetThumb(uint thumb)
			{ if (thumb < NumThumbs) return Thumbs[thumb]; return 0; }
		bool AddFile(const char * file_name, bool force = false);
		HBITMAP GetBitmap(int index, int w, int h, HWND window);
		int GetBitmapDepth(void);
		G42ImageType GetImageType(uint index) const;
		bool IsValid(void)
			{ return Valid; }
		bool IdleAction(void);
		bool NeedIdleAction(void)
			{ return (Valid && IdleActionThumb < NumThumbs); }
		int GetNumBuiltThumbs(void)
			{ return IdleActionThumb; }
		bool SetTotalSize(int mbytes);
		bool SetPalette(HDC dc);
      void InitPermFile(bool remove_old = false);
      void EndPermFile(void);
      void PermAllThumbs(void);
      void PermThumb(G42Thumb * thumb);
      bool HasPermFile(void) const
      	{return (CatCache ? true : false);}
		// New Things for the Cache Dialog
      int GetCacheNumber(void) const
      	{return (Cache ? Cache->GetCacheNumber() : 0);}
      int32 GetCacheSize(void) const
      	{return (Cache ? Cache->GetCacheSize() : 0);}
      int32 GetCacheUsed(void) const
      	{return (Cache ? Cache->GetCacheUsed() : 0);}
      char * GetCacheDirectory(void) const
      	{return CachePath;}
      int GetCacheUpdate(void)
      	{return (Cache ? CacheUpdate : 0);}
      void PurgeCache(void);
      void SetCacheSize(int32 size);
      void SetCacheDirectory(char * path, bool moveit = false);
      void SetCacheUpdate(int secs)
      	{CacheUpdate = secs;}
      void EnsureCacheActive(void);
		bool CheckCatalogUpdate(int & modified, int & added, int & deleted);
      bool NeedPermUpdate(void) const
      	{return PermUpdateRequired;}
		void RequirePermUpdate(void)
      	{PermUpdateRequired = true;}
	private:
		void Clear(void);
		char * DirPath;
		int NumThumbs;
		int IdleActionThumb;
		G42Array<G42Thumb *> Thumbs;
		bool Valid;
		G42Data * MemoryManager;
		int MaxWidth;
		int MaxHeight;
		BITMAPINFOHEADER * DibHeader;
		int DibUsage;
		HBITMAP Bitmap;
		int BitmapWidth;
		int BitmapHeight;
		int BitmapDepth;
		G42ThumbCache * Cache;
		G42ThumbCache * CatCache;
      bool PermUpdateRequired;
		G42OptimizedPalette * DitherPalette;
		HPALETTE WindowsPalette;
      char *	CachePath;
      char *	CacheFile;
      int		CacheUpdate;
};
#endif // G42THUMB_H

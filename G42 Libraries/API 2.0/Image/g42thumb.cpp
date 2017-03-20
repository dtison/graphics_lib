// g42thumb.cpp - creates thumbnails for images
#include "g42thumb.h"
#include <dir.h>
#include <dos.h>
#include <sys/stat.h>
G42ThumbHandler::G42ThumbHandler(G42Data * memory_manager,
		const char * cache_name, int32 size, int update, int width, int height) :
	DirPath(0), NumThumbs(0), Valid(true), MemoryManager(memory_manager),
	DibUsage(DIB_RGB_COLORS), DibHeader(0), Bitmap(0), BitmapWidth(0),
	BitmapHeight(0), Cache(0), BitmapDepth(0), CatCache(0),
	IdleActionThumb(0), MaxWidth(width - 2), MaxHeight(height - 2),
	DitherPalette(0), WindowsPalette(0), CachePath(0), CacheFile(0),
  	PermUpdateRequired(false), CacheUpdate(update)
{
	if (cache_name)
   {
   	CachePath = new char [strlen(cache_name) + 1];
      strcpy(CachePath, cache_name);
      CacheFile = strrchr(CachePath, '\\');
      if (CacheFile)
      {
      	*CacheFile = '\0';
         CacheFile++;
      }
      if (size)
			Cache = new G42ThumbCache(cache_name, size);
   }
}
G42ThumbHandler::~G42ThumbHandler(void)
{
	Clear();
	if (Bitmap)
	{
		DeleteObject(Bitmap);
		Bitmap = 0;
	}
	if (DibHeader)
	{
		delete [] (char *)DibHeader;
		DibHeader = 0;
	}
	if (Cache)
	{
		delete Cache;
		Cache = 0;
	}
   if (CatCache)
   {
   	delete CatCache;
      CatCache = 0;
   }
   
	if (DitherPalette)
	{
		delete DitherPalette;
		DitherPalette = 0;
	}
	if (WindowsPalette)
	{
		DeleteObject(WindowsPalette);
		WindowsPalette = 0;
	}
	Valid = false;
}
void
G42ThumbHandler::Clear(void)
{
	for (uint i = 0; i < NumThumbs; i++)
	{
		if (Thumbs[i])
			delete Thumbs[i];
		Thumbs[i] = 0;
	}
	if (DirPath)
	{
		delete [] DirPath;
      DirPath = 0;
	}
	NumThumbs = 0;
   IdleActionThumb = 0;
}
bool
G42ThumbHandler::ClearDirectory(void)
{
	Clear();
   return true;
}
bool
G42ThumbHandler::SetDirectory(const char * dir_path)
{
	Clear();
	if (!dir_path || !(*dir_path))
		return true;
	DirPath = new char [strlen(dir_path) + 2]; // save room for extra '\'
	if (!DirPath)
		return false;
	strcpy(DirPath, dir_path);
	if (DirPath[strlen(DirPath) - 1] != '\\')
	{
		DirPath[strlen(DirPath) + 1] = '\0';
		DirPath[strlen(DirPath)] = '\\';
	}
  	if (CatCache)
   {
     	delete CatCache;
      CatCache = 0;
   }
   char * buf = new char [strlen(DirPath) + 11];
   strcpy(buf, DirPath);
   strcat(buf, "mwview.csh");
	struct stat st_buf;
   if (!stat(buf, &st_buf))
      CatCache = new G42ThumbCache(buf, 1L, true);
   delete [] buf;
   return true;
}
bool
G42ThumbHandler::AddFile(const char * file_name, bool force)
{
	if (!file_name || !DirPath)
   	return false;
	char * name_buf = new char [strlen(DirPath) + strlen(file_name) + 1];
	if (!name_buf)
		return false;
	char * name_ptr = name_buf + strlen(DirPath);
	strcpy(name_buf, DirPath);
	strcpy(name_ptr, file_name);
	if (CatCache)
   {
		struct stat sbuf;
		int ret = stat(name_buf, &sbuf);
		if (ret)
			return false;
		time_t mod_time = sbuf.st_mtime;
  		G42ReadLockedCache cache(*CatCache);
		for (int i = 0; i < cache.GetNumObjects(); i++)
		{
			const G42CacheObject * cache_object =
				cache.GetObject((uint)i);
			const G42ThumbCacheObject * thumb_object =
				dynamic_cast<const G42ThumbCacheObject *>(cache_object);
         if (force)
         {
				if (!strcmp(name_ptr, thumb_object->GetFileNameProper())
            	&& thumb_object->TimeEqual(mod_time))
				{
					G42Thumb * thumb = new G42Thumb(thumb_object, MemoryManager);
					if (thumb && thumb->IsValid())
					{
						Thumbs[NumThumbs] = thumb;
						NumThumbs++;
						return true;
					}
					else if (thumb)
					{
						delete thumb;
					}
            }
         }
         else
         {
				if (!strcmp(name_ptr, thumb_object->GetFileNameProper()))
				{
					G42Thumb * thumb = new G42Thumb(thumb_object, MemoryManager);
					if (thumb && thumb->IsValid())
					{
						Thumbs[NumThumbs] = thumb;
						NumThumbs++;
						if (!thumb_object->TimeEqual(mod_time))
            	      CatCache->IncNumModifed();
               	CatCache->IncNumUsed();
						return true;
					}
					else if (thumb)
					{
						delete thumb;
					}
				}
         }
		}
		G42Thumb * thumb = new G42Thumb(name_buf, MemoryManager,
			MaxWidth, MaxHeight);
      if (force)
      {
			if (thumb && thumb->IsValid())
			{
				Thumbs[NumThumbs] = thumb;
				NumThumbs++;
			}
			else if (thumb)
			{
				delete thumb;
				return false;
			}
 			return true;
      }
      else
      {
			if (thumb && thumb->IsValid())
   	   	CatCache->IncNumAdded();
			if (thumb)
	      	delete thumb;
   	   return false;
      }
   }
	if (Cache)
	{
		struct stat sbuf;
		int ret = stat(name_buf, &sbuf);
		if (ret)
			return false;
		time_t mod_time = sbuf.st_mtime;
		G42ReadLockedCache cache(*Cache);
		for (int i = 0; i < cache.GetNumObjects(); i++)
		{
			const G42CacheObject * cache_object =
				cache.GetObject((uint)i);
			const G42ThumbCacheObject * thumb_object =
				dynamic_cast<const G42ThumbCacheObject *>(cache_object);
			if (!strcmp(name_buf, thumb_object->GetFileName()) &&
				thumb_object->TimeEqual(mod_time))
			{
				G42Thumb * thumb = new G42Thumb(thumb_object, MemoryManager);
				if (thumb && thumb->IsValid())
				{
					Thumbs[NumThumbs] = thumb;
					NumThumbs++;
					return true;
				}
				else if (thumb)
				{
					delete thumb;
				}
			}
		}
	}
	G42Thumb * thumb = new G42Thumb(name_buf, MemoryManager,
		MaxWidth, MaxHeight);
	if (thumb && thumb->IsValid())
	{
		Thumbs[NumThumbs] = thumb;
		NumThumbs++;
	}
	else if (thumb)
	{
		delete thumb;
		return false;
	}
	return true;
}
void
G42ThumbHandler::InitPermFile(bool remove_old)
{
	char * tbuf = new char [strlen(DirPath) + 11];
   strcpy(tbuf, DirPath);
   strcat(tbuf, "mwview.csh");
   if (remove_old)
   {
   	delete CatCache;
      CatCache = 0;
   	DeleteFile(tbuf);
   }
   CatCache = new G42ThumbCache(tbuf, 1L, true);
   delete [] tbuf;
   G42WriteLockedCache * lcache = new G42WriteLockedCache(*CatCache);
   lcache->SetModified();
   delete lcache;
}
void
G42ThumbHandler::EndPermFile(void)
{
   G42WriteLockedCache * lcache = new G42WriteLockedCache(*CatCache);
   lcache->SetModified();
   delete lcache;
   PermUpdateRequired = false;
}
void
G42ThumbHandler::PermAllThumbs(void)
{
	for (int n = 0; n < NumThumbs; n++)
   {
   	G42Thumb * thumb = Thumbs[n];
		thumb->AddToCache(*CatCache, true);
   }
}
void
G42ThumbHandler::PermThumb(G42Thumb * thumb)
{
	thumb->AddToCache(*CatCache, true);
}
int
G42ThumbHandler::GetBitmapDepth(void)
{
	if (!BitmapDepth)
	{
		HDC sdc = GetDC(0);
		int planes = GetDeviceCaps(sdc, PLANES);
		int bits = GetDeviceCaps(sdc, BITSPIXEL);
		ReleaseDC(0, sdc);
		BitmapDepth = planes * bits;
	}
	return BitmapDepth;
}
G42ImageType
G42ThumbHandler::GetImageType(uint index) const
{
	if (index < 0 || index >= NumThumbs || !Thumbs[index])
		return Unknown;
	return Thumbs[index]->GetImageType();
}
HBITMAP
G42ThumbHandler::GetBitmap(int index, int width, int height, HWND window)
{
	if (index < 0 || index >= NumThumbs)
		return 0;
	// make room for border
	width -= 2;
	height -= 2;
	G42Image * image = Thumbs[index]->GetImage();
	bool need_delete = false;
	if (!image || !image->IsValid() || !image->IsFinished() ||
			!image->HasInfo())
		return 0;
	if (!width || !height)
		return 0;
	if (width < image->GetInfo().Width || height < image->GetInfo().Height)
	{
		image = new G42Image(*image);
		need_delete = true;
		int new_width = (int)image->GetInfo().Width;
		int new_height = (int)image->GetInfo().Height;
		if (new_width > width || new_height > height)
		{
			if (new_width > new_height)
			{
				new_height = (new_height * width) / new_width;
				new_width = width;
			}
			else
			{
				new_width = (new_width * height) / new_height;
				new_height = height;
			}
		}
		if (new_width < 1)
			new_width = 1;
		if (new_height < 1)
			new_height = 1;
		image->ResizeImage((uint)new_width, (uint)new_height);
	}
	HDC sdc = GetDC(window);
	HDC dc = CreateCompatibleDC(sdc);
	ReleaseDC(window, sdc);
	if (Bitmap && (width != BitmapWidth || height != BitmapHeight))
	{
		DeleteObject(Bitmap);
		Bitmap = 0;
	}
	if (!BitmapDepth || !Bitmap)
	{
		int planes = GetDeviceCaps(dc, PLANES);
		int bits = GetDeviceCaps(dc, BITSPIXEL);
		if (!BitmapDepth)
			BitmapDepth = planes * bits;
		if (!Bitmap)
		{
			Bitmap = CreateBitmap(width + 2, height + 2, planes, bits, 0);
			BitmapWidth = width;
			BitmapHeight = height;
			if (!Bitmap)
			{
				DeleteDC(dc);
				return 0;
			}
		}
	}
	
	if (BitmapDepth <= 8)
	{
		if (!DitherPalette)
		{
			if (BitmapDepth > 4)
			{
				DitherPalette = new G42OptimizedPalette(0, 216, false, true);
			}
			else if (BitmapDepth > 1)
			{
				DitherPalette = new G42OptimizedPalette(0, 16, false, true);
			}
			else
			{
				DitherPalette = new G42OptimizedPalette(0, 2, false, true);
			}
		}
		if (!WindowsPalette)
		{
			if (BitmapDepth <= 8)
			{
				LOGPALETTE * log_pal = (LOGPALETTE *)new byte [2048]; // just to be safe
				log_pal->palVersion = 0x300;
				log_pal->palNumEntries = (WORD)DitherPalette->GetNumPalette();
				const G42Color * pal = DitherPalette->GetPalette();
				for (int i = 0; i < log_pal->palNumEntries; i++)
				{
					log_pal->palPalEntry[i].peRed = pal[i].red;
					log_pal->palPalEntry[i].peGreen = pal[i].green;
					log_pal->palPalEntry[i].peBlue = pal[i].blue;
					log_pal->palPalEntry[i].peFlags = 0;
				}
				WindowsPalette = CreatePalette(log_pal);
				delete [] (byte *)log_pal;
			}
			else // BitmapDepth == 8
			{
				WindowsPalette = CreateHalftonePalette(dc);
			}
		}
		if (image->GetInfo().ColorType != G42ImageInfo::ColorTypeGrayscale ||
			image->GetInfo().PixelDepth > 1)
		{
			G42Image * new_image = new G42Image(*image);
			if (BitmapDepth > 1)
				new_image->ColorReduce(DitherPalette);
			else
				new_image->ColorReduce(MethodDither, Gray, 2);
			if (need_delete) // delete stretched image if necessary
				delete image;
			image = new_image;
			need_delete = true;
		}
	}
   if (!DibHeader)
	{
		DibHeader = (BITMAPINFOHEADER *)new char [sizeof (BITMAPINFOHEADER) +
			256 * sizeof (RGBQUAD)];
		if (!DibHeader)
		{
      	DeleteDC(dc);
      	return 0;
		}
		memset(DibHeader, 0, sizeof (BITMAPINFOHEADER) +
			256 * sizeof (RGBQUAD));
		DibHeader->biPlanes = 1;
		DibHeader->biSize = sizeof (BITMAPINFOHEADER);
		DibHeader->biCompression = BI_RGB;
   }
	DibHeader->biBitCount = (WORD)image->GetInfo().PixelDepth;
	if (image->GetInfo().NumPalette)
	{
		DibHeader->biClrUsed = image->GetInfo().NumPalette;
		RGBQUAD * dib_pal = (RGBQUAD *)((byte *)DibHeader +
			sizeof (BITMAPINFOHEADER));
		const G42Color * pal = image->GetInfo().ImagePalette;
		for (int i = 0; i < image->GetInfo().NumPalette; i++)
		{
			dib_pal[i].rgbRed = pal[i].red;
			dib_pal[i].rgbGreen = pal[i].green;
			dib_pal[i].rgbBlue = pal[i].blue;
			dib_pal[i].rgbReserved = 0;
		}
		DibUsage = DIB_RGB_COLORS;
	}
	else if (image->GetInfo().ColorType ==
		G42ImageInfo::ColorTypeGrayscale)
	{
		DibHeader->biClrUsed = (1 << image->GetInfo().PixelDepth);
		RGBQUAD * dib_pal = (RGBQUAD *)((byte *)DibHeader +
			sizeof (BITMAPINFOHEADER));
		for (int i = 0; i < DibHeader->biClrUsed; i++)
		{
			dib_pal[i].rgbRed = dib_pal[i].rgbGreen =
				dib_pal[i].rgbBlue = (byte)(((int32)i * (int32)255) /
				(int32)(DibHeader->biClrUsed - 1));
			dib_pal[i].rgbReserved = 0;
		}
		DibUsage = DIB_RGB_COLORS;
	}
	HBITMAP old_bmp = (HBITMAP)SelectObject(dc, Bitmap);
	if (WindowsPalette)
	{
		SelectPalette(dc, WindowsPalette, false);
		RealizePalette(dc);
	}
	HBRUSH brush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
	HPEN pen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_WINDOWTEXT));
	HBRUSH old_brush = (HBRUSH)SelectObject(dc, brush);
	HPEN old_pen = (HPEN)SelectObject(dc, pen);
	Rectangle(dc, 0, 0, width + 2, height + 2);
	G42ImageTile * itile = image->GetTile(0, 0);
	if (!itile)
	{
    	DeleteDC(dc);
     	return 0;
	}
	G42LockedImageTile tile(itile);
	int tile_width = (int)tile.GetTile()->GetTileWidth();
	int tile_start_x = (int)tile.GetTile()->GetTileOffsetX();
	int tile_height = (int)tile.GetTile()->GetTileHeight();
   int tile_start_y = (int)tile.GetTile()->GetTileOffsetY();
	byte * tile_ptr = *(tile.GetTile()->GetRow(
		tile_start_y + tile_height - 1));
	int window_width = tile_width;
	int window_height = tile_height;
	int window_start_x = 1;
	int window_start_y = 1;
	if ((int32)window_width <= (int32)width &&
		(int32)window_height <= (int32)height)
	{
		window_start_x = ((width - window_width) >> 1) + 1;
		window_start_y = ((height - window_height) >> 1) + 1;
	}
	else if ((int32)window_width * (int32)height >
		(int32)window_height * (int32)width)
	{
		window_width = width;
		window_start_x = 1;
		window_height = (int)(((int32)width * (int32)tile_height) /
			(int32)tile_width);
		window_start_y = ((height - window_height) >> 1) + 1;
	}
	else
	{
		window_height = height;
		window_start_y = 1;
		window_width = (int)(((int32)height * (int32)tile_width) /
			(int32)tile_height);
		window_start_x = ((width - window_width) >> 1) + 1;
	}
	DibHeader->biWidth = (int)image->GetTileWidth();
	DibHeader->biHeight = tile_height;
	DibHeader->biSizeImage = (DWORD)tile_height *
		(DWORD)tile.GetTile()->GetRowBytes();
	if (tile_width == window_width && tile_height == window_height)
	{
		SetDIBitsToDevice(dc,
			window_start_x, window_start_y,
			tile_width, tile_height,
			tile_start_x, 0,
			0, tile_height, tile_ptr,
				(BITMAPINFO *)DibHeader, DibUsage);
	}
	else
	{
		StretchDIBits(dc,
			window_start_x, window_start_y,
			window_width, window_height,
			tile_start_x, 0,
			tile_width, tile_height,
			tile_ptr, (BITMAPINFO *)DibHeader, DibUsage, SRCCOPY);
	}
	SelectObject(dc, old_bmp);
	SelectObject(dc, old_brush);
	SelectObject(dc, old_pen);
	DeleteDC(dc);
	DeleteObject(brush);
	DeleteObject(pen);
	if (need_delete)
		delete image;
	return Bitmap;
}
bool
G42ThumbHandler::IdleAction(void)
{
	if (IdleActionThumb >= NumThumbs)
		return false;
   G42Thumb * thumb = Thumbs[IdleActionThumb];
   bool ret = thumb->IdleAction();
	if (!ret)
	{
		if (Cache)
			thumb->AddToCache(*Cache);
		IdleActionThumb++;
	}
	return (IdleActionThumb < NumThumbs);
}
bool
G42ThumbHandler::SetTotalSize(int mbytes)
{
	if (!Cache)
		return false;
	G42WriteLockedCache cache(*Cache);
	bool ret = cache.SetTotalSize((uint32)mbytes * (uint32)(0x100000L));
	return ret;
}
bool
G42ThumbHandler::SetPalette(HDC dc)
{
	if (WindowsPalette)
	{
		SelectPalette(dc, WindowsPalette, false);
		RealizePalette(dc);
		return true;
	}
	return false;
}
void
G42ThumbHandler::PurgeCache(void)
{
	if (!Cache)
   	return;
   G42WriteLockedCache * lcache = new G42WriteLockedCache(*Cache);
   lcache->PurgeCache();
   delete lcache;
}
void
G42ThumbHandler::SetCacheSize(int32 size)
{
	if (!Cache)
   {
		char * buf = new char [MAX_PATH * 2];
		strcpy(buf, CachePath);
#ifdef MSWIN
      strcat(buf, "\\");
#else
      strcat(buf, "/");
#endif
      strcat(buf, CacheFile);
		Cache = new G42ThumbCache(buf, size);
	  	return;
   }
   if (!size)
   {
   	delete Cache;
      Cache = 0;
      return;
   }
   G42WriteLockedCache * lcache = new G42WriteLockedCache(*Cache);
   lcache->SetCacheSize(size);
   delete lcache;
}
void
G42ThumbHandler::SetCacheDirectory(char * path, bool moveit)
{
	if (moveit)
   {
		// First, delete all old cache files from the new directory.
      char * buf = new char [MAX_PATH * 2];
      strcpy(buf, path);
      strcat(buf, "\\*.csh");
   	struct ffblk block;
	  	if (!findfirst(buf, &block, FA_ARCH))
      {
      	char * fname = new char [MAX_PATH * 2];
         strcpy(fname, path);
         strcat(fname, "\\");
         char * ptr = fname + strlen(fname);
         strcpy(ptr, block.ff_name);
      	DeleteFile(fname);
         while (!findnext(&block))
         {
	         strcpy(ptr, block.ff_name);
 		     	DeleteFile(fname);
         }
         delete [] fname;
      }
      strcpy(buf, path);
      strcat(buf, "\\*.VCD");
   	struct ffblk block2;
	  	if (!findfirst(buf, &block2, FA_ARCH))
      {
      	char * fname = new char [MAX_PATH * 2];
         strcpy(fname, path);
         strcat(fname, "\\");
         char * ptr = fname + strlen(fname);
         strcpy(ptr, block2.ff_name);
      	DeleteFile(fname);
         while (!findnext(&block2))
         {
	         strcpy(ptr, block2.ff_name);
 		     	DeleteFile(fname);
         }
         delete [] fname;
      }
      // Now move all the cache files to the new directory.
      strcpy(buf, CachePath);
      strcat(buf, "\\*.csh");
   	struct ffblk block3;
	  	if (!findfirst(buf, &block3, FA_ARCH))
      {
      	char * fname = new char [MAX_PATH * 2];
      	char * fname2 = new char [MAX_PATH * 2];
         strcpy(fname, CachePath);
         strcat(fname, "\\");
         char * ptr = fname + strlen(fname);
         strcpy(ptr, block3.ff_name);
         strcpy(fname2, path);
         strcat(fname2, "\\");
         char * ptr2 = fname2 + strlen(fname2);
         strcpy(ptr2, block3.ff_name);
      	MoveFile(fname, fname2);
         while (!findnext(&block3))
         {
	         strcpy(ptr, block3.ff_name);
	         strcpy(ptr2, block3.ff_name);
	      	MoveFile(fname, fname2);
         }
         delete [] fname;
         delete [] fname2;
      }
      strcpy(buf, CachePath);
      strcat(buf, "\\*.VCD");
   	struct ffblk block4;
	  	if (!findfirst(buf, &block4, FA_ARCH))
      {
      	char * fname = new char [MAX_PATH * 2];
      	char * fname2 = new char [MAX_PATH * 2];
         strcpy(fname, CachePath);
         strcat(fname, "\\");
         char * ptr = fname + strlen(fname);
         strcpy(ptr, block4.ff_name);
         strcpy(fname2, path);
         strcat(fname2, "\\");
         char * ptr2 = fname2 + strlen(fname2);
         strcpy(ptr2, block4.ff_name);
      	MoveFile(fname, fname2);
         while (!findnext(&block4))
         {
	         strcpy(ptr, block4.ff_name);
	         strcpy(ptr2, block4.ff_name);
 		     	MoveFile(fname, fname2);
         }
         delete [] fname;
         delete [] fname2;
      }
      delete [] buf;
   }
	int old_size = strlen(CacheFile);
   char * buf = new char [MAX_PATH * 2];
   strcpy(buf, CacheFile);
   delete [] CachePath;
   CachePath = new char [strlen(path) + old_size + 2];
   strcpy(CachePath, path);
   CacheFile = CachePath + strlen(path) + 1;
   strcpy(CacheFile, buf);
   if (Cache)
   {
		strcpy(buf, CachePath);
#ifdef MSWIN
      strcat(buf, "\\");
#else
      strcat(buf, "/");
#endif
      strcat(buf, CacheFile);
		int32 cache_size = Cache->GetCacheSize();
      delete Cache;
		Cache = new G42ThumbCache(buf, cache_size);
   }
   delete [] buf;
}
void
G42ThumbHandler::EnsureCacheActive(void)
{
	if (!Cache)
   	return;
	G42ReadLockedCache * lcache = new G42ReadLockedCache(*Cache);
   delete lcache;
}
bool
G42ThumbHandler::CheckCatalogUpdate(int & modified, int & added, int & deleted)
{
	if (!CatCache)
   	return false;
   modified = CatCache->GetNumModified();
   added = CatCache->GetNumAdded();
   deleted = CatCache->GetNumDeleted();
	if (modified || added || deleted)
   	return true;
   return false;
}
G42Thumb::G42Thumb(const G42ThumbCacheObject * object,
		G42Data * memory_manager) :
	MemoryManager(memory_manager), FilePath(0), FileName(0), Image(0),
	Valid(true), ReadBuffer(0), FP(0), Reader(0), IdleMode(DoneMode),
	MaxWidth(0), MaxHeight(0), Info(0), InCache(true)
{
	const char * file_path = object->GetFileName();
	FilePath = new char [strlen(file_path) + 1];
	strcpy(FilePath, file_path);
	FileName = strrchr(FilePath, '\\');
	if (FileName)
		FileName++;
	else
		FileName = FilePath;
	Info = new G42ImageInfo(*(object->GetInfo()));
	Image = new G42Image(MemoryManager, object->GetThumbInfo());
	G42ImageTile * tile = Image->GetTile(0, 0);
	G42ModifiableImageTile itile(tile);
	byte * tile_ptr = *(itile.GetTile()->GetRow(
		itile.GetTile()->GetHeight() - 1));
	object->GetData(tile_ptr);
	itile.MarkModified();
}
G42Thumb::G42Thumb(const char * file_path, G42Data * memory_manager,
		int max_width, int max_height) :
	MemoryManager(memory_manager), FilePath(0), FileName(0), Image(0),
	Valid(true), ReadBuffer(0), FP(0), Reader(0), IdleMode(NeedAllocateMode),
	MaxWidth(max_width), MaxHeight(max_height), Info(0), InCache(false)
{
	if (!file_path || !memory_manager)
	{
		Clear();
		Valid = false;
		return;
	}
	FilePath = new char [strlen(file_path) + 1];
	if (!FilePath)
	{
		Clear();
		Valid = false;
		return;
	}
	strcpy(FilePath, file_path);
	FileName = strrchr(FilePath, '\\');
	if (FileName)
		FileName++;
	else
		FileName = FilePath;
	// create thumbnail image
	Image = new G42Image(MemoryManager);
	if (!Image)
	{
		Clear();
		Valid = false;
		return;
	}
	ReadBufferSize = 4096;
	ReadBuffer = new byte [ReadBufferSize];
	if (!ReadBuffer)
	{
		Clear();
		Valid = false;
		return;
	}
	FP = fopen(FilePath, "rb");
	if (!FP)
	{
		Clear();
		Valid = false;
		return;
	}
	Reader = new G42ImageFile(Image);
	if (!Reader->IsValid())
	{
		Clear();
		Valid = false;
		return;
	}
	uint size = fread(ReadBuffer, 1, 256, FP); // don't do all 4K first time
	if (!size) // empty file
	{
		Clear();
		Valid = false;
		return;
	}
	fclose(FP);
	FP = 0;
	Reader->ProcessData(ReadBuffer, size);
	if (!Reader->IsValid() || !Image->IsValid()) // bad file
	{
		Clear();
		Valid = false;
		return;
	}
	if (Reader->IsFinished())
	{
		IdleMode = StretchMode;
	}
	else
	{
		IdleMode = NeedAllocateMode;
	}
}
bool
G42Thumb::IdleAction(void)
{
	switch (IdleMode)
	{
		case NeedAllocateMode:
		{
			IdleMode = ReadMode;
			FP = fopen(FilePath, "rb");
			if (!FP)
			{
				Clear();
				Valid = false;
				IdleMode = DoneMode;
				return false;
			}
			fseek(FP, 256, SEEK_SET);
			break;
		}
		case ReadMode:
		{
			uint size = fread(ReadBuffer, 1, ReadBufferSize, FP);
			if (!size)
			{
				IdleMode = StretchMode;
				Reader->ForceFileEnd();
				break;
			}
			Reader->ProcessData(ReadBuffer, size);
			if (!Reader->IsValid())
			{
				Clear();
				Valid = false;
				IdleMode = DoneMode;
				return false;
			}
			if (Reader->IsFinished())
			{
				IdleMode = StretchMode;
			}
			break;
		}
		case StretchMode:
		{
			if (Image)
			{
				struct stat stat_buf;
				if (FP)
					fstat(fileno(FP), &stat_buf);
				else
					stat(FilePath, &stat_buf);
				Image->SetInfoFileSize(stat_buf.st_size);
				Image->SetInfoModifiedTime(stat_buf.st_mtime);
			}
			if (ReadBuffer)
			{
				delete [] ReadBuffer;
				ReadBuffer = 0;
			}
			if (FP)
			{
				fclose(FP);
				FP = 0;
			}
			if (Reader)
			{
				delete Reader;
				Reader = 0;
			}
			if (!Image->IsValid() || !Image->HasInfo())
			{
				Clear();
				Valid = false;
				IdleMode = DoneMode;
				return false;
			}
			Info = new G42ImageInfo(Image->GetInfo());
			uint32 width = Image->GetInfo().Width;
			uint32 height = Image->GetInfo().Height;
			if (width > MaxWidth || height > MaxHeight)
			{
				if (width > height)
				{
					height = (height * MaxWidth) / width;
					width = MaxWidth;
				}
				else
				{
					width = (width * MaxHeight) / height;
					height = MaxHeight;
				}
				if (width < 1)
					width = 1;
				if (height < 1)
					height = 1;
				Image->ResizeImage((uint)width, (uint)height);
			}
			IdleMode = DoneMode;
			return false;
		}
		default:
		{
			return false;
		}
	}
	return true;
}
void
G42Thumb::Clear(void)
{
	if (Image)
	{
		delete Image;
		Image = 0;
	}
	if (Info)
	{
		delete Info;
		Info = 0;
	}
	if (FilePath)
	{
		delete [] FilePath;
		FilePath = 0;
	}
	if (ReadBuffer)
	{
		delete [] ReadBuffer;
		ReadBuffer = 0;
	}
	if (FP)
	{
		fclose(FP);
		FP = 0;
	}
	if (Reader)
	{
		delete Reader;
		Reader = 0;
	}
}
G42Thumb::~G42Thumb()
{
	Clear();
	FileName = 0;
	MemoryManager = 0;
	Valid = false;
}
bool
G42Thumb::IsValid(void)
{
	return Valid;
}
G42Image *
G42Thumb::GetImage(void)
{
	if (!Valid)
		return 0;
	return Image;
}
G42Image *
G42Thumb::GetFullSizeImage(void)
{
	G42Image * image = new G42Image(MemoryManager);
	if (!image)
		return 0;
      
	if (ReadBuffer)
	  	delete [] ReadBuffer;
	ReadBufferSize = 4096;
	ReadBuffer = new byte [ReadBufferSize];
	if (!ReadBuffer)
   {
   	delete image;
		return 0;
   }
	FP = fopen(FilePath, "rb");
	if (!FP)
   {
   	delete image;
      delete [] ReadBuffer;
		return 0;
   }
	if (Reader)
   	delete Reader;
	Reader = new G42ImageFile(image);
	if (!Reader->IsValid())
	{
   	delete image;
      delete [] ReadBuffer;
		return 0;
   }
	uint size = fread(ReadBuffer, 1, 256, FP); // don't do all 4K first time
	if (!size) // empty file
	{
   	delete image;
      delete [] ReadBuffer;
		return 0;
   }
	fclose(FP);
	FP = 0;
	Reader->ProcessData(ReadBuffer, size);
	if (!Reader->IsValid() || !image->IsValid()) // bad file
	{
   	delete image;
      delete [] ReadBuffer;
		return 0;
   }
	FP = fopen(FilePath, "rb");
	if (!FP)
	{
   	delete image;
      delete [] ReadBuffer;
		return 0;
   }
	fseek(FP, 256, SEEK_SET);
	while (!Reader->IsFinished())
   {
		uint size = fread(ReadBuffer, 1, ReadBufferSize, FP);
		if (!size)
		{
			Reader->ForceFileEnd();
			continue;
		}
		Reader->ProcessData(ReadBuffer, size);
		if (!Reader->IsValid())
			break;
	}
	if (image)
	{
		struct stat stat_buf;
		if (FP)
			fstat(fileno(FP), &stat_buf);
		else
			stat(FilePath, &stat_buf);
		image->SetInfoFileSize(stat_buf.st_size);
		image->SetInfoModifiedTime(stat_buf.st_mtime);
	}
	if (ReadBuffer)
	{
		delete [] ReadBuffer;
		ReadBuffer = 0;
	}
   if (FP)
	{
		fclose(FP);
		FP = 0;
	}
	if (Reader && (!Reader->IsFinished() || !Reader->IsValid()))
   {
		delete Reader;
		Reader = 0;
      delete image;
		return 0;
   }
   
	if (Reader)
	{
		delete Reader;
		Reader = 0;
	}
	if (!image->IsValid() || !image->HasInfo())
   {
   	delete image;
		return 0;
   }
	return image;
}
G42ImageInfo *
G42Thumb::GetInfo(void)
{
	if (!Valid)
		return 0;
	return Info;
}
const char *
G42Thumb::GetFileName(void) const
{
	if (!Valid)
		return 0;
	return FileName;
}
const char *
G42Thumb::GetFilePath(void) const
{
	if (!Valid)
		return 0;
	return FilePath;
}
G42ImageType
G42Thumb::GetImageType(void) const
{
	if (!Valid)
		return Unknown;
	if (Image && Image->HasInfo())
		return Image->GetInfo().ImageType;
	if (Reader)
		return Reader->GetImageType();
	return Unknown;
}
bool
G42Thumb::AddToCache(G42ThumbCache & cache, bool override)
{
	if (!Valid)
		return false;
	if (!override && InCache)
		return true;
		
	if (!cache.IsValid())
		return false;
	if (!Image || !Image->HasInfo())
		return false;
	G42ImageTile * tile = Image->GetTile(0, 0);
	if (!tile)
		return false;
	G42ThumbCacheObject * object = new G42ThumbCacheObject(Info,
		&Image->GetInfo(), FilePath);
	if (!object || !object->IsValid())
		return false;
	G42LockedImageTile itile(tile);
	byte * tile_ptr = *(itile.GetTile()->GetRow(
		itile.GetTile()->GetHeight() - 1));
	G42WriteLockedCache lcache(cache);
	lcache.AddObject(object, tile_ptr, itile.GetTile()->GetHeight() *
		itile.GetTile()->GetRowBytes());
	if (override)
   	lcache.SetModified(false);
   else
		InCache = true;
	return true;
}
G42ThumbCache::G42ThumbCache(const char * file_path, int32 size, bool create_catalog) :
	G42Cache(file_path, size, create_catalog)
{
}
G42ThumbCache::~G42ThumbCache()
{
}
G42CacheObject *
G42ThumbCache::CreateObject(int id)
{
	return new G42ThumbCacheObject();
}
uint32
G42ThumbCache::GetHeaderID(void)
{
	return 0x4220; // 42 'W' 'I' 2.0
}
G42ThumbCacheObject::G42ThumbCacheObject(void) :
	Info(0), FileName(0), Platform(0), ThumbInfo(0), FilePtr(0)
{
}
G42ThumbCacheObject::G42ThumbCacheObject(const G42ImageInfo * info,
	const G42ImageInfo * thumb_info, const char * file_name) :
	Info(0), FileName(0), Platform(0), ThumbInfo(0), FilePtr(0)
{
	Info = new G42ImageInfo(*info);
	ThumbInfo = new G42ImageInfo(*thumb_info);
	FileName = new char [strlen(file_name) + 1];
	strcpy(FileName, file_name);
   FilePtr = strrchr(FileName, '\\');
   if (FilePtr)
   	FilePtr++;
   else
   	FilePtr = FileName;
}
G42ThumbCacheObject::~G42ThumbCacheObject()
{
	if (Info)
	{
		delete Info;
		Info = 0;
	}
	if (ThumbInfo)
	{
		delete ThumbInfo;
		ThumbInfo = 0;
	}
	if (FileName)
	{
		delete [] FileName;
      FileName = 0;
	}
	Valid = false;
}
const G42ImageInfo *
G42ThumbCacheObject::GetInfo(void) const
{
	return Info;
}
const G42ImageInfo *
G42ThumbCacheObject::GetThumbInfo(void) const
{
	return ThumbInfo;
}
const char *
G42ThumbCacheObject::GetFileName(void) const
{
	return FileName;
}
const char *
G42ThumbCacheObject::GetFileNameProper(void) const
{
	return FilePtr;
}
time_t
G42ThumbCacheObject::GetModifiedTime(void) const
{
	if (Info)
		return Info->ModifiedTime;
	return 0;
}
bool
G42ThumbCacheObject::TimeEqual(time_t new_time) const
{
	if (new_time == GetModifiedTime())
		return true;
	return false;
}
uint32
G42ThumbCacheObject::GetWriteInfoSize(void)
{
	return (65L + 3 * (Info->NumPalette + ThumbInfo->NumPalette) +
		strlen(FileName));
}
bool
G42ThumbCacheObject::WriteInfo(FILE * fp)
{
	Write8(fp, strlen(FileName));
	fwrite(FileName, 1, strlen(FileName), fp);
	Write32(fp, Info->Width);
	Write32(fp, Info->Height);
	Write32(fp, Info->FileSize);
	Write8(fp, Info->PixelDepth);
	Write8(fp, Info->HasAlphaChannel);
	Write8(fp, Info->IsTransparent);
	Write8(fp, Info->HasBackground);
	Write8(fp, Info->Progressive);
	Write8(fp, Info->Compressed);
	Write8(fp, Info->TransparentColor.red);
	Write8(fp, Info->TransparentColor.green);
	Write8(fp, Info->TransparentColor.blue);
	Write8(fp, Info->BackgroundColor.red);
	Write8(fp, Info->BackgroundColor.green);
	Write8(fp, Info->BackgroundColor.blue);
	Write8(fp, Info->ImageType);
	Write8(fp, Info->ColorType);
	Write32(fp, Info->ModifiedTime);
	Write16(fp, Info->NumPalette);
	if (Info->NumPalette)
	{
		const G42Color * pal = Info->ImagePalette;
		for (int i = 0; i < Info->NumPalette; i++)
		{
			Write8(fp, pal[i].red);
			Write8(fp, pal[i].green);
			Write8(fp, pal[i].blue);
		}
	}
	Write32(fp, ThumbInfo->Width);
	Write32(fp, ThumbInfo->Height);
	Write32(fp, ThumbInfo->FileSize);
	Write8(fp, ThumbInfo->PixelDepth);
	Write8(fp, ThumbInfo->HasAlphaChannel);
	Write8(fp, ThumbInfo->IsTransparent);
	Write8(fp, ThumbInfo->HasBackground);
	Write8(fp, ThumbInfo->Progressive);
	Write8(fp, ThumbInfo->Compressed);
	Write8(fp, ThumbInfo->TransparentColor.red);
	Write8(fp, ThumbInfo->TransparentColor.green);
	Write8(fp, ThumbInfo->TransparentColor.blue);
	Write8(fp, ThumbInfo->BackgroundColor.red);
	Write8(fp, ThumbInfo->BackgroundColor.green);
	Write8(fp, ThumbInfo->BackgroundColor.blue);
	Write8(fp, ThumbInfo->ImageType);
	Write8(fp, ThumbInfo->ColorType);
	Write32(fp, ThumbInfo->ModifiedTime);
	Write16(fp, ThumbInfo->NumPalette);
	if (ThumbInfo->NumPalette)
	{
		const G42Color * pal = ThumbInfo->ImagePalette;
		for (int i = 0; i < ThumbInfo->NumPalette; i++)
		{
			Write8(fp, pal[i].red);
			Write8(fp, pal[i].green);
			Write8(fp, pal[i].blue);
		}
	}
	return true;
}
bool
G42ThumbCacheObject::ReadInfo(FILE * fp)
{
	int len = Read8(fp);
	if (len <= 0)
		return false;
	FileName = new char [len + 1];
	fread(FileName, 1, len, fp);
	FileName[len] = '\0';
   FilePtr = strrchr(FileName, '\\');
   if (FilePtr)
   	FilePtr++;
   else
   	FilePtr = FileName;
	if (!Info)
		Info = new G42ImageInfo;
	Info->Width = Read32(fp);
	Info->Height = Read32(fp);
	Info->FileSize = Read32(fp);
	Info->PixelDepth = Read8(fp);
	Info->HasAlphaChannel = Read8(fp);
	Info->IsTransparent = Read8(fp);
	Info->HasBackground = Read8(fp);
	Info->Progressive = Read8(fp);
	Info->Compressed = Read8(fp);
	Info->TransparentColor.red = Read8(fp);
	Info->TransparentColor.green = Read8(fp);
	Info->TransparentColor.blue = Read8(fp);
	Info->BackgroundColor.red = Read8(fp);
	Info->BackgroundColor.green = Read8(fp);
	Info->BackgroundColor.blue = Read8(fp);
	Info->ImageType = Read8(fp);
	Info->ColorType = Read8(fp);
	Info->ModifiedTime = Read32(fp);
	Info->NumPalette = Read16(fp);
	if (Info->NumPalette)
	{
		G42Color * pal = new G42Color [Info->NumPalette];
		Info->ImagePalette.Set(pal);
		for (int i = 0; i < Info->NumPalette; i++)
		{
			pal[i].red = Read8(fp);
			pal[i].green = Read8(fp);
			pal[i].blue = Read8(fp);
		}
	}
	if (!ThumbInfo)
		ThumbInfo = new G42ImageInfo;
	ThumbInfo->Width = Read32(fp);
	ThumbInfo->Height = Read32(fp);
	ThumbInfo->FileSize = Read32(fp);
	ThumbInfo->PixelDepth = Read8(fp);
	ThumbInfo->HasAlphaChannel = Read8(fp);
	ThumbInfo->IsTransparent = Read8(fp);
	ThumbInfo->HasBackground = Read8(fp);
	ThumbInfo->Progressive = Read8(fp);
	ThumbInfo->Compressed = Read8(fp);
	ThumbInfo->TransparentColor.red = Read8(fp);
	ThumbInfo->TransparentColor.green = Read8(fp);
	ThumbInfo->TransparentColor.blue = Read8(fp);
	ThumbInfo->BackgroundColor.red = Read8(fp);
	ThumbInfo->BackgroundColor.green = Read8(fp);
	ThumbInfo->BackgroundColor.blue = Read8(fp);
	ThumbInfo->ImageType = Read8(fp);
	ThumbInfo->ColorType = Read8(fp);
	ThumbInfo->ModifiedTime = Read32(fp);
	ThumbInfo->NumPalette = Read16(fp);
	if (ThumbInfo->NumPalette)
	{
		G42Color * pal = new G42Color [ThumbInfo->NumPalette];
		ThumbInfo->ImagePalette.Set(pal);
		for (int i = 0; i < ThumbInfo->NumPalette; i++)
		{
			pal[i].red = Read8(fp);
			pal[i].green = Read8(fp);
			pal[i].blue = Read8(fp);
		}
	}
	return true;
}
int
G42ThumbCacheObject::GetId(void)
{
	return 0;
}

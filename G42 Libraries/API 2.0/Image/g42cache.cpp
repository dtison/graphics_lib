// g42cache.h - implement simple disk cache
#include "g42cache.h"
#include <fcntl.h>
#include <io.h>
#include <sys\stat.h>
G42CacheObject::G42CacheObject() :
	InfoSize(0), Cache(0), DataFile(0), DataSize(0), Valid(true),
	TouchIncrement(0), SeekLocation(0), Touch(0)
{
}
G42CacheObject::~G42CacheObject(void)
{
	if (DataFile)
	{
		remove(DataFile);
		delete [] DataFile;
		DataFile = 0;
	}
	Valid = false;
}
bool
G42CacheObject::IsValid(void) const
{
	return Valid;
}
uint32
G42CacheObject::GetDataSize(void) const
{
	if (Valid)
		return DataSize;
	return 0;
}
bool
G42CacheObject::GetData(byte * buffer)
{
	if (!Valid || !DataSize || !buffer || !Cache)
		return false;
   if (!Cache->IsPermanent() && !DataFile)
   	return false;
	FILE * fp;
	if (Cache->CacheMode == G42Cache::DiskCache)
	{
		char * buf = new char [Cache->DirPathSize + strlen(DataFile) + 1];
		if (!buf)
			return false;
		strcpy(buf, Cache->DirPath);
		strcpy(buf + Cache->DirPathSize, DataFile);
		fp = fopen(buf, "rb");
		delete [] buf;
	}
	else
	{
		fp = fopen(Cache->FilePath, "rb");
	}
	if (!fp)
		return false;
	if (Cache->CacheMode != G42Cache::DiskCache)
	{
		if (fseek(fp, SeekLocation, SEEK_SET))
		{
			fclose(fp);
			return false;
		}
	}
	int size = fread(buffer, 1, DataSize, fp);
	fclose(fp);
   TouchIncrement = ++Cache->TouchIncrementCounter;
	return (size == DataSize);
}
void
G42CacheObject::Write8(FILE * fp, uint8 value)
{
	fputc(value, fp);
}
void
G42CacheObject::Write16(FILE * fp, uint16 value)
{
	fputc(((value >> 8) & 0xff), fp);
	fputc((value & 0xff), fp);
}
void
G42CacheObject::Write32(FILE * fp, uint32 value)
{
	fputc(((value >> 24) & 0xff), fp);
	fputc(((value >> 16) & 0xff), fp);
	fputc(((value >> 8) & 0xff), fp);
	fputc((value & 0xff), fp);
}
uint8
G42CacheObject::Read8(FILE * fp)
{
	int ret = fgetc(fp);
	if (ret >= 0 && ret < 256)
		return (uint8)ret;
	else
		return 0;
}
uint16
G42CacheObject::Read16(FILE * fp)
{
	int ret = fgetc(fp);
	uint16 value;
	if (ret < 0 || ret >= 256)
		return 0;
	value = (uint16)((uint16)ret << 8);
	ret = fgetc(fp);
	if (ret < 0 || ret >= 256)
		return 0;
	value |= (uint16)ret;
	return value;
}
uint32
G42CacheObject::Read32(FILE * fp)
{
	int ret = fgetc(fp);
	uint32 value;
	if (ret < 0 || ret >= 256)
		return 0;
	value = (uint32)((uint32)ret << 24);
	ret = fgetc(fp);
	if (ret < 0 || ret >= 256)
		return 0;
	value |= (uint32)((uint32)ret << 16);
	ret = fgetc(fp);
	if (ret < 0 || ret >= 256)
		return 0;
	value |= (uint32)((uint32)ret << 8);
	ret = fgetc(fp);
	if (ret < 0 || ret >= 256)
		return 0;
	value |= (uint32)ret;
	return value;
}
int
G42CacheObject::GetId(void)
{
	return 0;
}
uint32
G42CacheObject::GetReadInfoSize(void)
{
	return InfoSize;
}
G42Cache::G42Cache(const char * file_path, int32 size, bool create_permanent_file) :
	FileName(0), NumObjectArray(0), Valid(true), DirPath(0), FilePath(0),
	CacheMode(DiskCache), NextSeekLocation(8), TotalSize(size),
	DataSize(0), NeedRead(false), TouchIncrementCounter(0), TouchCounter(0),
	WriteCounter(0), ReadLock(0), WriteLock(0), TouchReadLock(0),
	TouchWriteLock(0), NumModifed(0), NumAdded(0), NumUsed(0)
{
	if (!file_path || !(*file_path))
	{
		Valid = false;
		return;
	}
	FilePath = new char [strlen(file_path) + 1];
	strcpy(FilePath, file_path);
#ifdef MSWIN
	char * p = strrchr(FilePath, '\\');
#else
	char * p = strrchr(FilePath, '/');
#endif
	if (!p) // file name must be full path
	{
		Valid = false;
		return;
	}
	FileName = p + 1;
	DirPathSize = p - FilePath + 1;
	DirPath = new char [DirPathSize + 1];
	strncpy(DirPath, FilePath, DirPathSize);
	DirPath[DirPathSize] = '\0';
	if (create_permanent_file)
		CacheMode = PermanentFile;
	NeedRead = true;
}
G42Cache::~G42Cache()
{
	if (FilePath)
	{
		delete [] FilePath;
		FilePath = 0;
	}
	if (DirPath)
	{
		delete [] DirPath;
		DirPath = 0;
	}
	for (uint i = 0; i < NumObjectArray; i++)
   	delete ObjectArray[i];
	Valid = false;
}
bool
G42Cache::AddObject(G42CacheObject * object, byte * data,
	uint32 data_size)
{
	if (!Valid || !object || !object->IsValid() || object->Cache)
		return false;
	ObjectArray[NumObjectArray] = object;
   object->Cache = this;
	object->DataSize = data_size;
	DataSize += data_size;
	char * buf;
	if (CacheMode == DiskCache)
	{
		char * buf = new char [14 + DirPathSize];
		strcpy(buf, DirPath);
		uint32 touch = TouchCounter + TouchIncrementCounter;
		struct stat statbuf;
		do
		{
			touch++;
			sprintf(buf + DirPathSize, "%0.8lx.VCD", touch);
			if (stat(buf, &statbuf))
				break;
		} while (1);
		if (object->DataFile)
			delete [] object->DataFile;
		object->DataFile = new char [14];
		strcpy(object->DataFile, buf + DirPathSize);
		FILE * fp = fopen(buf, "wb+");
		if (fp)
		{
			fwrite(data, data_size, 1, fp);
			fclose(fp);
		}
	}
	else
	{
   	int32 loc = 0L;
		if (NumObjectArray == 0)
			loc = 46L + object->GetWriteInfoSize();
      else
      {
         loc = ObjectArray[NumObjectArray -1]->SeekLocation;
         loc += ObjectArray[NumObjectArray -1]->DataSize;
         loc += 22L + object->GetWriteInfoSize();
      }
		object->SeekLocation = loc;
		FILE * fp = fopen(FilePath, "rb+");
		if (fp)
		{
			fseek(fp, object->SeekLocation, SEEK_SET);
			fwrite(data, data_size, 1, fp);
			fclose(fp);
		}
		NextSeekLocation = loc + data_size;
	}
// Temporary value for Touch.
// To keep this object from being immediately thrown out.
	object->Touch = TouchCounter;
	object->TouchIncrement = ++TouchIncrementCounter;
	NumObjectArray++;
	Modified = true;
	CheckSize();
	return true;
}
bool
G42Cache::DeleteObject(uint index)
{
	G42CacheObject * object = ObjectArray[index];
	if (CacheMode == DiskCache)
	{
		int file_offset = strlen(DirPath);
		char * buf = new char [14 + file_offset];
		strcpy(buf, DirPath);
		strcpy(buf + file_offset, object->DataFile);
		remove(buf);
		DataSize -= object->DataSize;
		delete [] buf;
	}
	delete object;
	NumObjectArray--;
	for (uint i = index; i < NumObjectArray; i++)
		ObjectArray[i] = ObjectArray[i + 1];
	ObjectArray[NumObjectArray] = 0;
	Modified = true;
	return true;
}
bool
G42Cache::ModifyObjectData(uint index, byte * data,
	uint32 data_size)
{
	if (CacheMode == DiskCache)
	{
		int file_offset = strlen(DirPath);
		char * buf = new char [14 + file_offset];
		strcpy(buf, DirPath);
		strcat(buf, ObjectArray[index]->DataFile);
		// hmmm.  We really should write into a temp file then rename,
		// so if the file is smaller, we gain the space back
		FILE * fp = fopen(buf, "wb+");
		if (fp)
		{
			fwrite(data, data_size, 1, fp);
			fclose(fp);
		}
		delete [] buf;
	}
	else
	{
		ObjectArray[index]->SeekLocation = NextSeekLocation;
		FILE * fp = fopen(FilePath, "rb+");
		if (fp)
		{
			fseek(fp, NextSeekLocation, SEEK_SET);
			fwrite(data, data_size, 1, fp);
			fclose(fp);
		}
		NextSeekLocation += data_size;
	}
	DataSize -= ObjectArray[index]->DataSize;
	DataSize += data_size;
	Modified = true;
	ObjectArray[index]->DataSize = data_size;
	ObjectArray[index]->TouchIncrement = ++TouchIncrementCounter;
	CheckSize();
	return true;
}
bool
G42Cache::ReadObjectInfo(void)
{
	NeedRead = false;
	
	FILE * fp = fopen(FilePath, "rb");
	if (!fp)
		return false;
	char buf[4];
	if (fread(buf, 1, 4, fp) != 4)
	{
		fclose(fp);
		return false;
	}
	if (buf[0] != 0x42 || buf[1] != 0x42 || buf[2] != 0x20)
	{
		fclose(fp);
		return false;
	}
	if (buf[3] == 0)
		CacheMode = DiskCache;
	else
		CacheMode = PermanentFile;
	if (CacheMode != DiskCache)
	{
		if (fread(buf, 1, 4, fp) != 4)
		{
			fclose(fp);
			return false;
		}
		NextSeekLocation = ((uint32)(byte)(buf[0]) << 24) |
			((uint32)(byte)(buf[1]) << 16) |
			((uint32)(byte)(buf[2]) << 8) |
			((uint32)(byte)(buf[3]));
		//fseek(fp, NextSeekLocation, SEEK_SET);
	}
	if (fread(buf, 1, 4, fp) != 4)
	{
		fclose(fp);
		return false;
	}
	int id = (((int)(byte)(buf[0])) << 8) |
		((int)(byte)(buf[1]));
	if (id != GetHeaderID())
	{
		fclose(fp);
		return false;
	}
	NumObjectArray = ((uint16)(byte)(buf[2]) << 8) |
		((uint16)(byte)(buf[3]));
	if (fread(buf, 1, 4, fp) != 4)
	{
		fclose(fp);
		return false;
	}
	TouchCounter = ((uint32)(byte)(buf[0]) << 24) |
		((uint32)(byte)(buf[1]) << 16) |
		((uint32)(byte)(buf[2]) << 8) |
		((uint32)(byte)(buf[3]));
	if (fread(buf, 1, 4, fp) != 4)
	{
		fclose(fp);
		return false;
	}
	WriteCounter = ((uint32)(byte)(buf[0]) << 24) |
		((uint32)(byte)(buf[1]) << 16) |
		((uint32)(byte)(buf[2]) << 8) |
		((uint32)(byte)(buf[3]));
	if (fread(buf, 1, 4, fp) != 4)
	{
		fclose(fp);
		return false;
	}
	if (!TotalSize)
   {
		TotalSize = ((uint32)(byte)(buf[0]) << 24) |
			((uint32)(byte)(buf[1]) << 16) |
			((uint32)(byte)(buf[2]) << 8) |
			((uint32)(byte)(buf[3]));
   }
	DataSize = 0;
	for (uint i = 0; i < NumObjectArray; i++)
	{
		if (fread(buf, 1, 4, fp) != 4)
		{
			fclose(fp);
			return false;
		}
		uint32 size = ((uint32)(byte)(buf[0]) << 24) |
			((uint32)(byte)(buf[1]) << 16) |
			((uint32)(byte)(buf[2]) << 8) |
			((uint32)(byte)(buf[3]));
		if (fread(buf, 1, 2, fp) != 2)
		{
			fclose(fp);
			return false;
		}
		int id = ((uint32)(byte)(buf[2]) << 8) |
			((uint32)(byte)(buf[3]));
		G42CacheObject * object = CreateObject(id);
		if (!object)
		{
			fclose(fp);
			return false;
		}
		if (fread(buf, 1, 4, fp) != 4)
		{
			fclose(fp);
			return false;
		}
		object->Touch = ((uint32)(byte)(buf[0]) << 24) |
			((uint32)(byte)(buf[1]) << 16) |
			((uint32)(byte)(buf[2]) << 8) |
			((uint32)(byte)(buf[3]));
		if (fread(buf, 1, 4, fp) != 4)
		{
			fclose(fp);
			return false;
		}
		object->DataSize = ((uint32)(byte)(buf[0]) << 24) |
			((uint32)(byte)(buf[1]) << 16) |
			((uint32)(byte)(buf[2]) << 8) |
			((uint32)(byte)(buf[3]));
		uint32 file_size = 0;
		if (CacheMode == DiskCache)
		{
			if (fread(buf, 1, 4, fp) != 4)
			{
				fclose(fp);
				return false;
			}
			file_size = ((uint32)(byte)(buf[0]) << 24) |
				((uint32)(byte)(buf[1]) << 16) |
				((uint32)(byte)(buf[2]) << 8) |
				((uint32)(byte)(buf[3]));
			object->DataFile = new char [file_size];
			if (!object->DataFile)
			{
				fclose(fp);
				return false;
			}
		}
		else
		{
			if (fread(buf, 1, 4, fp) != 4)
			{
				fclose(fp);
				return false;
			}
			object->SeekLocation = ((uint32)(byte)(buf[0]) << 24) |
				((uint32)(byte)(buf[1]) << 16) |
				((uint32)(byte)(buf[2]) << 8) |
				((uint32)(byte)(buf[3]));
		}
		if (fread(buf, 1, 4, fp) != 4)
		{
			fclose(fp);
			return false;
		}
		object->InfoSize = ((uint32)(byte)(buf[0]) << 24) |
			((uint32)(byte)(buf[1]) << 16) |
			((uint32)(byte)(buf[2]) << 8) |
			((uint32)(byte)(buf[3]));
		if (CacheMode == DiskCache && file_size)
			fread(object->DataFile, file_size, 1, fp);
		if (!object->ReadInfo(fp))
		{
			fclose(fp);
			return false;
		}
      if (CacheMode != DiskCache)
      {
         fseek(fp, object->DataSize, SEEK_CUR);
      }
		object->Cache = this;
		ObjectArray[i] = object;
		DataSize += object->DataSize;
	}
	fclose(fp);
	Modified = false;
	return true;
}
bool
G42Cache::WriteNewTouchCodes(void)
{
	// here we update the objects that have been touched with
	// new touch codes.  This really only happens at the destructor
	// of a read or write lock, but be sure to check with read
	// locks that someone else hasn't updated the touch count
	if (CacheMode != DiskCache)
   	return true;
      
	while (!SetTouchWriteLock())
	{
	}
	FILE * fp = fopen(FilePath, "rb+");
	if (!fp)
	{
		while (!ClearTouchWriteLock())
		{
		}
		return false;
	}
	char buf[4];
   fseek(fp, 8, SEEK_SET);
	if (fread(buf, 1, 4, fp) != 4)
	{
		while (!ClearTouchWriteLock())
		{
		}
		fclose(fp);
		return false;
	}
	TouchCounter = ((uint32)(byte)(buf[0]) << 24) |
		((uint32)(byte)(buf[1]) << 16) |
		((uint32)(byte)(buf[2]) << 8) |
		((uint32)(byte)(buf[3]));
	fseek(fp, 8, SEEK_CUR);
	uint32 max_inc = 0;
	for (uint i = 0; i < NumObjectArray; i++)
	{
		if (fread(buf, 1, 4, fp) != 4)
		{
			while (!ClearTouchWriteLock())
			{
			}
			fclose(fp);
			return false;
		}
		uint32 size = ((uint32)(byte)(buf[0]) << 24) |
			((uint32)(byte)(buf[1]) << 16) |
			((uint32)(byte)(buf[2]) << 8) |
			((uint32)(byte)(buf[3]));
		if (ObjectArray[i]->TouchIncrement)
		{
			fseek(fp, 2, SEEK_CUR);
			if (ObjectArray[i]->TouchIncrement > max_inc)
				max_inc = ObjectArray[i]->TouchIncrement;
			ObjectArray[i]->Touch = TouchCounter + ObjectArray[i]->TouchIncrement;
			ObjectArray[i]->TouchIncrement = 0;
			buf[0] = (ObjectArray[i]->Touch >> 24) & 0xff;
			buf[1] = (ObjectArray[i]->Touch >> 16) & 0xff;
			buf[2] = (ObjectArray[i]->Touch >> 8) & 0xff;
			buf[3] = (ObjectArray[i]->Touch) & 0xff;
			fwrite(buf, 4, 1, fp);
			fseek(fp, size - 10, SEEK_CUR);
		}
		else // object not touched
		{
			fseek(fp, size - 4, SEEK_CUR);
		}
	}
	TouchCounter += max_inc + 1;
	TouchIncrementCounter = 0;
	fseek(fp, 8, SEEK_SET);
	buf[0] = (TouchCounter >> 24) & 0xff;
	buf[1] = (TouchCounter >> 16) & 0xff;
	buf[2] = (TouchCounter >> 8) & 0xff;
	buf[3] = (TouchCounter) & 0xff;
	fwrite(buf, 4, 1, fp);
	fclose(fp);
	while (!ClearTouchWriteLock())
	{
	}
	return true;
}
bool
G42Cache::UpdateTouchCodes(void)
{
	// gets the latest touch codes.  Be sure no one is writing them
	// at the same time.
	while (!SetTouchReadLock())
	{
	}
	FILE * fp = fopen(FilePath, "rb");
	if (!fp)
	{
		while (!ClearTouchReadLock())
		{
		}
		return false;
	}
	char buf[4];
	fseek(fp, 8, SEEK_SET);
	if (fread(buf, 1, 4, fp) != 4)
	{
		while (!ClearTouchReadLock())
		{
		}
		fclose(fp);
		return false;
	}
	uint32 new_touch_counter = ((uint32)(byte)(buf[0]) << 24) |
		((uint32)(byte)(buf[1]) << 16) |
		((uint32)(byte)(buf[2]) << 8) |
		((uint32)(byte)(buf[3]));
	if (new_touch_counter == TouchCounter)
	{
		while (!ClearTouchReadLock())
		{
		}
		fclose(fp);
		return true;
	}
	TouchCounter = new_touch_counter;
	fseek(fp, 8, SEEK_CUR);
	for (uint i = 0; i < NumObjectArray; i++)
	{
		if (fread(buf, 1, 4, fp) != 4)
		{
			while (!ClearTouchReadLock())
			{
			}
			fclose(fp);
			return false;
		}
		uint32 size = ((uint32)(byte)(buf[0]) << 24) |
			((uint32)(byte)(buf[1]) << 16) |
			((uint32)(byte)(buf[2]) << 8) |
			((uint32)(byte)(buf[3]));
		fseek(fp, 2, SEEK_CUR);
		if (fread(buf, 1, 4, fp) != 4)
		{
			while (!ClearTouchReadLock())
			{
			}
			fclose(fp);
			return false;
		}
		ObjectArray[i]->Touch = ((uint32)(byte)(buf[0]) << 24) |
			((uint32)(byte)(buf[1]) << 16) |
			((uint32)(byte)(buf[2]) << 8) |
			((uint32)(byte)(buf[3]));
		fseek(fp, size - 10, SEEK_CUR);
	}
	fclose(fp);
	while (!ClearTouchReadLock())
	{
	}
	return true;
}
bool
G42Cache::WriteObjectInfo(void)
{
	FILE * fp;
	if (CacheMode == DiskCache)
   	fp = fopen(FilePath, "wb");
   else
   {
   	struct stat st_buf;
      if (stat(FilePath, &st_buf))
      {
	   	int handle = creat(FilePath, S_IWRITE);
         close(handle);
      }
   	fp = fopen(FilePath, "r+b");
   }
	if (!fp)
		return false;
	char buf[4];
	buf[0] = 0x42;
	buf[1] = 0x42;
	buf[2] = 0x20;
	if (CacheMode == DiskCache)
		buf[3] = 0x0;
	else
		buf[3] = 0x1;
	fwrite(buf, 4, 1, fp);
	if (CacheMode != DiskCache)
	{
		buf[0] = (NextSeekLocation >> 24) & 0xff;
		buf[1] = (NextSeekLocation >> 16) & 0xff;
		buf[2] = (NextSeekLocation >> 8) & 0xff;
		buf[3] = (NextSeekLocation) & 0xff;
		fwrite(buf, 4, 1, fp);
      //fseek(fp, NextSeekLocation, SEEK_SET);
	}
	int id = GetHeaderID();
	buf[0] = (id >> 8) & 0xff;
	buf[1] = (id & 0xff);
	buf[2] = (NumObjectArray >> 8) & 0xff;
	buf[3] = (NumObjectArray & 0xff);
	fwrite(buf, 4, 1, fp);
	buf[0] = (TouchCounter >> 24) & 0xff;
	buf[1] = (TouchCounter >> 16) & 0xff;
	buf[2] = (TouchCounter >> 8) & 0xff;
	buf[3] = (TouchCounter) & 0xff;
	fwrite(buf, 4, 1, fp);
	WriteCounter++;
	buf[0] = (WriteCounter >> 24) & 0xff;
	buf[1] = (WriteCounter >> 16) & 0xff;
	buf[2] = (WriteCounter >> 8) & 0xff;
	buf[3] = (WriteCounter) & 0xff;
	fwrite(buf, 4, 1, fp);
	buf[0] = (TotalSize >> 24) & 0xff;
	buf[1] = (TotalSize >> 16) & 0xff;
	buf[2] = (TotalSize >> 8) & 0xff;
	buf[3] = (TotalSize) & 0xff;
	fwrite(buf, 4, 1, fp);
	for (uint i = 0; i < NumObjectArray; i++)
	{
		G42CacheObject * object = ObjectArray[i];
		uint32 file_size;
		if (CacheMode == DiskCache)
			file_size = strlen(object->DataFile) + 1;
		else
      	file_size = 0;
		uint32 info_size = object->GetWriteInfoSize();
		uint32 size = file_size + info_size + (uint32)20;
		buf[0] = (size >> 24) & 0xff;
		buf[1] = (size >> 16) & 0xff;
		buf[2] = (size >> 8) & 0xff;
		buf[3] = (size) & 0xff;
		fwrite(buf, 4, 1, fp);
		int id = object->GetId();
		buf[0] = (id >> 8) & 0xff;
		buf[1] = (id) & 0xff;
		fwrite(buf, 2, 1, fp);
		buf[0] = (object->Touch >> 24) & 0xff;
		buf[1] = (object->Touch >> 16) & 0xff;
		buf[2] = (object->Touch >> 8) & 0xff;
		buf[3] = (object->Touch) & 0xff;
		fwrite(buf, 4, 1, fp);
		buf[0] = (object->DataSize >> 24) & 0xff;
		buf[1] = (object->DataSize >> 16) & 0xff;
		buf[2] = (object->DataSize >> 8) & 0xff;
		buf[3] = (object->DataSize) & 0xff;
		fwrite(buf, 4, 1, fp);
		if (CacheMode == DiskCache)
		{
			buf[0] = (file_size >> 24) & 0xff;
			buf[1] = (file_size >> 16) & 0xff;
			buf[2] = (file_size >> 8) & 0xff;
			buf[3] = (file_size) & 0xff;
		}
		else
		{
      	//int32 loc = ftell(fp);
         //loc += object->GetWriteInfoSize() + 4L;
         //object->SeekLocation = loc;
			buf[0] = (object->SeekLocation >> 24) & 0xff;
			buf[1] = (object->SeekLocation >> 16) & 0xff;
			buf[2] = (object->SeekLocation >> 8) & 0xff;
			buf[3] = (object->SeekLocation) & 0xff;
		}
		fwrite(buf, 4, 1, fp);
		buf[0] = (info_size >> 24) & 0xff;
		buf[1] = (info_size >> 16) & 0xff;
		buf[2] = (info_size >> 8) & 0xff;
		buf[3] = (info_size) & 0xff;
		fwrite(buf, 4, 1, fp);
		if (CacheMode == DiskCache)
			fwrite(object->DataFile, file_size, 1, fp);
		object->WriteInfo(fp);
      if (CacheMode != DiskCache)
      {
      	fseek(fp, object->DataSize, SEEK_CUR);
      }
	}
	fclose(fp);
	Modified = false;
	return true;
}
void
G42Cache::CheckSize(void)
{
	if (CacheMode != DiskCache)
		return;
	while (DataSize > TotalSize)
	{
		uint32 min_touch = TouchCounter;
		uint min_object = 0;
		uint i = 0;
		for (; i < NumObjectArray; i++)
		{
			if (ObjectArray[i]->Touch < min_touch)
			{
				min_touch = ObjectArray[i]->Touch;
				min_object = i;
			}
		}
		if (min_object < NumObjectArray)
			DeleteObject(min_object);
		else
			break;
   }
}
bool
G42Cache::SetWriteLock(void)
{
	if (ReadLock || WriteLock)
		return false;
	WriteLock++;
	return true;
}
bool
G42Cache::ClearWriteLock(void)
{
	if (WriteLock)
		WriteLock--;
	return true;
}
bool
G42Cache::SetReadLock(void)
{
	if (WriteLock)
		return false;
	ReadLock++;
	return true;
}
bool
G42Cache::ClearReadLock(void)
{
	if (ReadLock)
		ReadLock--;
	return true;
}
bool
G42Cache::SetTouchWriteLock(void)
{
	if (TouchReadLock || TouchWriteLock)
		return false;
	TouchWriteLock++;
	return true;
}
bool
G42Cache::ClearTouchWriteLock(void)
{
	if (TouchWriteLock)
		TouchWriteLock--;
	return true;
}
bool
G42Cache::SetTouchReadLock(void)
{
	if (TouchWriteLock)
		return false;
	TouchReadLock++;
	return true;
}
bool
G42Cache::ClearTouchReadLock(void)
{
	if (TouchReadLock)
		TouchReadLock--;
	return true;
}
void
G42Cache::PurgeCache(void)
{
	for (int n = (NumObjectArray - 1); n >= 0; n--)
   	DeleteObject(n);
}
void
G42Cache::SetCacheSize(int32 size)
{
	TotalSize = size;
   CheckSize();
}
G42ReadLockedCache::G42ReadLockedCache(G42Cache & cache) :
	Cache(cache), Valid(true), Locked(false)
{
	if (Cache.NeedRead)
		Cache.ReadObjectInfo();
	while (!Locked)
		if (Cache.SetReadLock())
			Locked = true;
}
bool
G42ReadLockedCache::Unlock(void)
{
	if (Locked)
	{
		if (Cache.TouchIncrementCounter)
			Cache.WriteNewTouchCodes();
		while (!Cache.ClearReadLock())
		{
			/* empty loop */
		}
		Locked = false;
	}
	return true;
}
G42WriteLockedCache::G42WriteLockedCache(G42Cache & cache) :
	Cache(cache), Modified(false), Locked(false), Valid(true)
{
	if (Cache.NeedRead)
		Cache.ReadObjectInfo();
	while (!Locked)
		if (Cache.SetWriteLock())
			Locked = true;
}
bool
G42WriteLockedCache::Unlock(void)
{
	if (Locked)
	{
		if (Cache.TouchIncrementCounter)
			Cache.WriteNewTouchCodes();
		if (Cache.Modified)
			Cache.WriteObjectInfo();
		while (!Cache.ClearWriteLock())
		{
			/* empty loop */
		}
		Locked = false;
	}
	return true;
}
G42CacheObject *
G42WriteLockedCache::GetObject(uint index)
{
	if (!IsValid() || index >= Cache.NumObjectArray)
		return 0;
	return Cache.ObjectArray[index];
}
uint
G42WriteLockedCache::GetNumObjects(void) const
{
	if (!IsValid())
		return 0;
	return Cache.NumObjectArray;
}
uint32
G42WriteLockedCache::GetSizeUsed(void) const
{
	if (!IsValid())
		return 0;
	return Cache.DataSize;
}
uint32
G42WriteLockedCache::GetTotalSize(void) const
{
	if (!IsValid())
		return 0;
	return Cache.TotalSize;
}
bool
G42WriteLockedCache::AddObject(G42CacheObject * object, byte * data,
	uint32 data_size)
{
	return Cache.AddObject(object, data, data_size);
}
bool
G42WriteLockedCache::DeleteObject(uint index)
{
	return Cache.DeleteObject(index);
}
bool
G42WriteLockedCache::ModifyObjectData(uint index, byte * data,
	uint32 data_size)
{
	return Cache.ModifyObjectData(index, data, data_size);
}
bool
G42WriteLockedCache::WriteObjectInfo(void)
{
	return Cache.WriteObjectInfo();
}
bool
G42WriteLockedCache::SetTotalSize(uint32 size)
{
	Cache.TotalSize = size;
	Cache.CheckSize();
	return true;
}
void
G42WriteLockedCache::PurgeCache(void)
{
	Cache.PurgeCache();
}
void
G42WriteLockedCache::SetCacheSize(int32 size)
{
	Cache.SetCacheSize(size);
}

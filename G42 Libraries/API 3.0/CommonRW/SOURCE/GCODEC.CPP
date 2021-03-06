#include "gcodec.h"
#include "grwsupp.h"
#if defined(G42_READ_SOUNDS)
#include "gsdllrd.h"
#endif
#if defined(G42_WRITE_SOUNDS)
#include "gsdllwr.h"
#endif
#if defined(G42_READ_WAVE)
#include "gswaverd.h"
#endif
#if defined(G42_WRITE_WAVE)
#include "gswavewr.h"
#endif
#if defined(G42_READ_AIFF)
#include "gsaiffrd.h"
#endif
#if defined(G42_WRITE_AIFF)
#include "gsaiffwr.h"
#endif
#if defined(G42_READ_AU)
#include "gsaurd.h"
#endif
#if defined(G42_WRITE_AU)
#include "gsauwr.h"
#endif
#if defined(G42_READ_PCM)
#include "gsrawrd.h"
#endif
#if defined(G42_WRITE_PCM)
#include "gsrawwr.h"
#endif
#if defined(G42_WRITE_RA)
#include "gsrawr.h"
#endif
#if defined(G42_READ_BMP)
#include "girbmp.h"
#endif
#if defined(G42_READ_GIF)
#include "girgif.h"
#endif
#if defined(G42_READ_JPEG)
#include "girjpeg.h"
#endif
#if defined(G42_READ_PBM)
#include "girpbm.h"
#endif
#if defined(G42_READ_PCX)
#include "girpcx.h"
#endif
#if defined(G42_READ_PNG)
#include "girpng.h"
#endif
#if defined(G42_READ_RAS)
#include "girras.h"
#endif
#if defined(G42_READ_TARGA)
#include "girtarga.h"
#endif
#if defined(G42_READ_TIFF) || defined(G42_READ_EPS)
#include "girtiff.h"
#endif
#if defined(G42_READ_XWD)
#include "girxwd.h"
#endif
#if defined(G42_WRITE_BMP)
#include "giwbmp.h"
#endif
#if defined(G42_WRITE_EPS)
#include "giweps.h"
#endif
#if defined(G42_WRITE_GIF)
#include "giwgif.h"
#endif
#if defined(G42_WRITE_JPEG)
#include "giwjpeg.h"
#endif
#if defined(G42_WRITE_PBM)
#include "giwpbm.h"
#endif
#if defined(G42_WRITE_PCX)
#include "giwpcx.h"
#endif
#if defined(G42_WRITE_PNG)
#include "giwpng.h"
#endif
#if defined(G42_WRITE_RAS)
#include "giwras.h"
#endif
#if defined(G42_WRITE_TARGA)
#include "giwtarga.h"
#endif
#if defined(G42_WRITE_TIFF)
#include "giwtiff.h"
#endif
#if defined(G42_WRITE_XWD)
#include "giwxwd.h"
#endif
G42CodecManager::G42CodecManager(const G42StringTable & string_table,
	const char * dir)
:
	ReadFilter(0), WriteFilter(0), Directory(0), NumReaders(0),
	NumWriters(0), WriteTArray(0), ReadTArray(0), MaxPatternSize(0),
	StringTable(string_table)
{
	if (dir)
	{
		Directory = new char [strlen(dir) + 1];
		strcpy(Directory, dir);
	}
	QueryReaders();
	CreateReadFilter();
	QueryWriters();
	CreateWriteFilter();
}
G42CodecManager::~G42CodecManager(void)
{
	Clear();
}
void
G42CodecManager::Clear(void)
{
	intf n;
	if (ReadFilter)
		delete [] ReadFilter;
	ReadFilter = 0;
	if (WriteFilter)
		delete [] WriteFilter;
	WriteFilter = 0;
	if (Directory)
		delete [] Directory;
	Directory = 0;
	if (WriteTArray)
		delete [] WriteTArray;
	WriteTArray = 0;
	if (ReadTArray)
		delete [] ReadTArray;
	ReadTArray = 0;
	for (n = 0; n < NumReaders; n++)
		delete Readers[n];
	Readers.Clear();
	NumReaders = 0;
	for (n = 0; n < NumWriters; n++)
		delete Writers[n];
	Writers.Clear();
	NumWriters = 0;
	MaxPatternSize = 0;
}
void
G42CodecManager::QueryReaders(void)
{
#if defined(G42_READ_BMP)
	{
		G42ImageReaderBmp reader;
		Readers[NumReaders++] = new G42ReaderInfo(reader.GetInfo());
	}
#endif
#if defined(G42_READ_EPS)
	{
		G42ImageReaderEps reader;
		Readers[NumReaders++] = new G42ReaderInfo(reader.GetInfo());
	}
#endif
#if defined(G42_READ_GIF)
	{
		G42ImageReaderGif reader;
		Readers[NumReaders++] = new G42ReaderInfo(reader.GetInfo());
	}
#endif
#if defined(G42_READ_JPEG)
	{
		G42ImageReaderJpeg reader;
		Readers[NumReaders++] = new G42ReaderInfo(reader.GetInfo());
	}
#endif
#if defined(G42_READ_PBM)
	{
		G42ImageReaderPbm reader;
		Readers[NumReaders++] = new G42ReaderInfo(reader.GetInfo());
	}
#endif
#if defined(G42_READ_PCX)
	{
		G42ImageReaderPcx reader;
		Readers[NumReaders++] = new G42ReaderInfo(reader.GetInfo());
	}
#endif
#if defined(G42_READ_PNG)
	{
		G42ImageReaderPng reader;
		Readers[NumReaders++] = new G42ReaderInfo(reader.GetInfo());
	}
#endif
#if defined(G42_READ_RAS)
	{
		G42ImageReaderRas reader;
		Readers[NumReaders++] = new G42ReaderInfo(reader.GetInfo());
	}
#endif
#if defined(G42_READ_TARGA)
	{
		G42ImageReaderTarga reader;
		Readers[NumReaders++] = new G42ReaderInfo(reader.GetInfo());
	}
#endif
#if defined(G42_READ_TIFF)
	{
		G42ImageReaderTiff reader;
		Readers[NumReaders++] = new G42ReaderInfo(reader.GetInfo());
	}
#endif
#if defined(G42_READ_XWD)
	{
		G42ImageReaderXwd reader;
		Readers[NumReaders++] = new G42ReaderInfo(reader.GetInfo());
	}
#endif
#if defined(G42_READ_WAVE)
	{
		G42WaveReader reader;
		Readers[NumReaders++] = new G42ReaderInfo(reader.GetInfo());
	}
#endif
#if defined(G42_READ_AIFF)
	{
		G42AiffReader reader;
		Readers[NumReaders++] = new G42ReaderInfo(reader.GetInfo());
	}
#endif
#if defined(G42_READ_AU)
	{
		G42AuReader reader;
		Readers[NumReaders++] = new G42ReaderInfo(reader.GetInfo());
	}
#endif
	CheckReadDlls();
#if defined(G42_READ_PCM)
	G42RawPcmReader * raw_reader = new G42RawPcmReader();
	const G42ReaderInfo * info = &(raw_reader->GetInfo());
	if (info->GetVersionMajor() == 3)
	{
		intf duplicate = -1;
		for (intf n = 0; n < NumReaders; n++)
		{
			if (Readers[n]->GetTypeId() == info->GetTypeId())
			{
				duplicate = n;
				break;
			}
		}
		if (duplicate != -1)
		{
			if (info->GetVersionMinor() >= Readers[duplicate]->GetVersionMinor())
			{
				delete Readers[duplicate];
				Readers[duplicate] = new G42ReaderInfo(*info);
			}
		}
		else
			Readers[NumReaders++] = new G42ReaderInfo(*info);
	}
	delete raw_reader;
#endif
	for (intf i = 0; i < NumReaders; i++)
	{
		if (Readers[i]->GetPatternLength() > MaxPatternSize)
		{
			MaxPatternSize = Readers[i]->GetPatternLength();
		}
	}
}
void
G42CodecManager::QueryWriters(void)
{
#if defined(G42_WRITE_BMP)
	{
		G42ImageWriterBmp reader;
		Writers[NumWriters++] = new G42WriterInfo(reader.GetInfo());
	}
#endif
#if defined(G42_WRITE_EPS)
	{
		G42ImageWriterEps reader;
		Writers[NumWriters++] = new G42WriterInfo(reader.GetInfo());
	}
#endif
#if defined(G42_WRITE_GIF)
	{
		G42ImageWriterGif reader;
		Writers[NumWriters++] = new G42WriterInfo(reader.GetInfo());
	}
#endif
#if defined(G42_WRITE_JPEG)
	{
		G42ImageWriterJpeg reader;
		Writers[NumWriters++] = new G42WriterInfo(reader.GetInfo());
	}
#endif
#if defined(G42_WRITE_PBM)
	{
		G42ImageWriterPbm reader;
		Writers[NumWriters++] = new G42WriterInfo(reader.GetInfo());
	}
#endif
#if defined(G42_WRITE_PCX)
	{
		G42ImageWriterPcx reader;
		Writers[NumWriters++] = new G42WriterInfo(reader.GetInfo());
	}
#endif
#if defined(G42_WRITE_PNG)
	{
		G42ImageWriterPng reader;
		Writers[NumWriters++] = new G42WriterInfo(reader.GetInfo());
	}
#endif
#if defined(G42_WRITE_RAS)
	{
		G42ImageWriterRas reader;
		Writers[NumWriters++] = new G42WriterInfo(reader.GetInfo());
	}
#endif
#if defined(G42_WRITE_TARGA)
	{
		G42ImageWriterTarga reader;
		Writers[NumWriters++] = new G42WriterInfo(reader.GetInfo());
	}
#endif
#if defined(G42_WRITE_TIFF)
	{
		G42ImageWriterTiff reader;
		Writers[NumWriters++] = new G42WriterInfo(reader.GetInfo());
	}
#endif
#if defined(G42_WRITE_XWD)
	{
		G42ImageWriterXwd reader;
		Writers[NumWriters++] = new G42WriterInfo(reader.GetInfo());
	}
#endif
#if defined(G42_WRITE_WAVE)
	{
		G42WaveWriter writer;
		Writers[NumWriters++] = new G42WriterInfo(writer.GetInfo());
	}
#endif
#if defined(G42_WRITE_AIFF)
	{
		G42AiffWriter writer;
		Writers[NumWriters++] = new G42WriterInfo(writer.GetInfo());
	}
#endif
#if defined(G42_WRITE_AU)
	{
		G42AuWriter writer;
		Writers[NumWriters++] = new G42WriterInfo(writer.GetInfo());
	}
#endif
#if defined(G42_WRITE_RA)
	{
		G42RaWriter writer;
		Writers[NumWriters++] = new G42WriterInfo(writer.GetInfo());
	}
#endif
	CheckWriteDlls();
#if defined(G42_WRITE_PCM)
	G42RawPcmWriter * raw_writer = new G42RawPcmWriter();
	const G42WriterInfo & info = raw_writer->GetInfo();
	if (info.GetVersionMajor() == 3)
	{
		intf duplicate = -1;
		for (intf n = 0; n < NumWriters; n++)
		{
			if (Writers[n]->GetTypeId() == info.GetTypeId())
			{
				duplicate = n;
				break;
			}
		}
		if (duplicate != -1)
		{
			if (info.GetVersionMinor() >= Writers[duplicate]->GetVersionMinor())
			{
				delete Writers[duplicate];
				Writers[duplicate] = new G42WriterInfo(info);
			}
		}
		else
			Writers[NumWriters++] = new G42WriterInfo(info);
	}
	delete raw_writer;
#endif
}
#ifdef MSWIN
void
G42CodecManager::AddDllToReaderList(HMODULE mod, const char * dll_name)
{
	intf (CALLBACK * fp)(void);
	fp = (intf (CALLBACK *)(void))GetProcAddress(mod, "G42GetNumReaders");
	intf num_readers = (*fp)();
	for (intf i = 0; i < num_readers; i++)
	{
		G42BaseReader * (CALLBACK * fp2)(const char *, intf);
		fp2 = (G42BaseReader * (CALLBACK *)(const char *, intf))
			GetProcAddress(mod, "G42EnumerateReaders");
		if (fp2 == NULL)
         return;
		G42BaseReader * reader = (*fp2)(dll_name, i);
		const G42ReaderInfo & info = reader->GetInfo();
		if (info.GetVersionMajor() == 3)
		{
			intf duplicate = -1;
			for (intf n = 0; n < NumReaders; n++)
			{
				if (Readers[n]->GetTypeId() == info.GetTypeId())
				{
					duplicate = n;
					break;
				}
			}
			if (duplicate != -1)
			{
				if (info.GetVersionMinor() > Readers[duplicate]->GetVersionMinor())
				{
					delete Readers[duplicate];
					Readers[duplicate] = new G42ReaderInfo(info);
				}
			}
			else
				Readers[NumReaders++] = new G42ReaderInfo(info);
		}
		void (CALLBACK * fp3)(void);
		fp3 = (void (CALLBACK *)(void))GetProcAddress(mod, "G42ShutDownRead");
		if (fp3 == NULL)
			return;
		(*fp3)();
	}
}
#endif
#ifdef MSWIN
void
G42CodecManager::AddDllToWriterList(HMODULE mod, const char * dll_name)
{
	intf (CALLBACK * fp)(void);
	fp = (intf (CALLBACK *)(void))GetProcAddress(mod, "G42GetNumWriters");
	intf num_writers = (*fp)();
	for (intf i = 0; i < num_writers; i++)
	{
		G42BaseWriter * (CALLBACK * fp2)(const char *, intf);
		fp2 = (G42BaseWriter * (CALLBACK *)(const char *, intf))
			GetProcAddress(mod, "G42EnumerateWriters");
		if (fp2 == NULL)
         return;
		G42BaseWriter * writer = (*fp2)(dll_name, i);
		const G42WriterInfo & info = writer->GetInfo();
		if (info.GetVersionMajor() == 3)
		{
			intf duplicate = -1;
			for (intf n = 0; n < NumWriters; n++)
			{
				if (Writers[n]->GetTypeId() == info.GetTypeId())
				{
					duplicate = n;
					break;
				}
			}
			if (duplicate != -1)
			{
				if (info.GetVersionMinor() > Writers[duplicate]->GetVersionMinor())
				{
					delete Writers[duplicate];
					Writers[duplicate] = new G42WriterInfo(info);
				}
			}
			else
				Writers[NumWriters++] = new G42WriterInfo(info);
		}
		void (CALLBACK * fp3)(void);
		fp3 = (void (CALLBACK *)(void))GetProcAddress(mod, "G42ShutDownWrite");
		if (fp3 == NULL)
			return;
		(*fp3)();
	}
}
#endif
void
G42CodecManager::CheckReadDlls(void)
{
#ifdef MSWIN
	if (!Directory)
	{
		return;
	}
	char pattern[_MAX_PATH] = {0};
	strcpy(pattern, Directory);
	strcat(pattern, "\\*.dll");
	WIN32_FIND_DATA find_data;
	char * name = new char [_MAX_PATH];
	HANDLE find_handle = FindFirstFile(pattern, &find_data);
	if (find_handle != INVALID_HANDLE_VALUE)
	{
		strcpy(name, Directory);
		strcat(name, "\\");
		strcat(name, find_data.cFileName);
		HMODULE mod = LoadLibrary(name);
		if (mod != NULL)
		{
			bool (CALLBACK * fp)(void);
			fp = (bool (CALLBACK *)(void))GetProcAddress(mod, "G42IsReader");
			if (fp != NULL)
			{
				bool test = (*fp)();
				if (test)
					AddDllToReaderList(mod, name);
			}
			FreeLibrary(mod);
		}
		while (FindNextFile(find_handle, &find_data))
		{
			strcpy(name, Directory);
			strcat(name, "\\");
			strcat(name, find_data.cFileName);
			HMODULE mod = LoadLibrary(name);
			if (mod != NULL)
			{
				bool (CALLBACK * fp)(void);
				fp = (bool (CALLBACK *)(void))GetProcAddress(mod, "G42IsReader");
				if (fp != NULL)
				{
					bool test = (*fp)();
					if (test)
						AddDllToReaderList(mod, name);
				}
				FreeLibrary(mod);
			}
		}
		FindClose(find_handle);
	}
	delete [] name;
#endif
}
void
G42CodecManager::CheckWriteDlls(void)
{
#ifdef MSWIN
	if (!Directory)
	{
		return;
	}
	char pattern[_MAX_PATH] = {0};
	strcpy(pattern, Directory);
	strcat(pattern, "\\*.dll");
	WIN32_FIND_DATA find_data;
	char * name = new char [_MAX_PATH];
	HANDLE find_handle = FindFirstFile(pattern, &find_data);
	if (find_handle != INVALID_HANDLE_VALUE)
	{
		strcpy(name, Directory);
		strcat(name, "\\");
		strcat(name, find_data.cFileName);
		HMODULE mod = LoadLibrary(name);
		if (mod != NULL)
		{
			bool (CALLBACK * fp)(void);
			fp = (bool (CALLBACK *)(void))GetProcAddress(mod, "G42IsWriter");
			if (fp != NULL)
			{
				bool test = (*fp)();
				if (test)
					AddDllToWriterList(mod, name);
			}
			FreeLibrary(mod);
		}
		while (FindNextFile(find_handle, &find_data))
		{
			strcpy(name, Directory);
			strcat(name, "\\");
			strcat(name, find_data.cFileName);
			HMODULE mod = LoadLibrary(name);
			if (mod != NULL)
			{
				bool (CALLBACK * fp)(void);
				fp = (bool (CALLBACK *)(void))GetProcAddress(mod, "G42IsWriter");
				if (fp != NULL)
				{
					bool test = (*fp)();
					if (test)
						AddDllToWriterList(mod, name);
				}
				FreeLibrary(mod);
			}
		}
		FindClose(find_handle);
	}
	delete [] name;
	#endif	// MSWIN
}
void
G42CodecManager::CreateReadFilter(void)
{
	intf n;
	ReadTArray = new intf [NumReaders];
	for (n = 0; n < NumReaders; n++)
		ReadTArray[n] = n;
	bool have_swapped = true;
	while (have_swapped)
	{
		have_swapped = false;
		for (intf n = 0; n < NumReaders - 1; n++)
		{
			if (strcmp(Readers[ReadTArray[n]]->GetDefExt(),
				Readers[ReadTArray[n+1]]->GetDefExt()) > 0)
			{
				intf tmp = ReadTArray[n];
				ReadTArray[n] = ReadTArray[n+1];
				ReadTArray[n+1] = tmp;
				have_swapped = true;
			}
		}
	}
	char * buffer = new char [512];
	buffer[0] = 0;
#if defined(G42_READ_SOUNDS)
#if !defined(G42_READ_IMAGES)
	StringTable.GetString(G42_STRING_S_FILES, buffer, 512);
#else
	StringTable.GetString(G42_STRING_M_FILES, buffer, 512);
#endif
#else
	StringTable.GetString(G42_STRING_I_FILES, buffer, 512);
#endif
	if (!buffer[0])
	{
		strcpy(buffer, "*.*|");
	}
	for (n = 0; n < NumReaders; n++)
	{
		for (intf index = 0; index < Readers[ReadTArray[n]]->GetNumExtensions(); index++)
		{
			bool redundant = false;
			for(intf m = 0; m < n; m++)
			{
				for (intf i = 0; i < Readers[ReadTArray[m]]->GetNumExtensions(); i++)
				{
					if (!strcmp(Readers[ReadTArray[n]]->GetDefExt(index),
						Readers[ReadTArray[m]]->GetDefExt(i)))
					{
						redundant = true;
						break;
					}
				}
				if (redundant)
					break;
			}
			if (redundant)
				break;
			strcat(buffer, "*.");
			strcat(buffer, Readers[ReadTArray[n]]->GetDefExt(index));
			strcat(buffer, ";");
		}
	}
	strcat(buffer, "|");
	for (n = 0; n < NumReaders; n++)
	{
		strcat(buffer, Readers[ReadTArray[n]]->GetNameString());
		strcat(buffer, " (*.");
		intf index;
		for (index = 0; index < Readers[ReadTArray[n]]->GetNumExtensions() - 1; index++)
		{
			strcat(buffer, Readers[ReadTArray[n]]->GetDefExt(index));
			strcat(buffer, ";*.");
		}
		strcat(buffer, Readers[ReadTArray[n]]->
			GetDefExt(Readers[ReadTArray[n]]->GetNumExtensions() - 1));
		strcat(buffer, ")|*.");
		for (index = 0; index < Readers[ReadTArray[n]]->GetNumExtensions() - 1; index++)
		{
			strcat(buffer, Readers[ReadTArray[n]]->GetDefExt(index));
			strcat(buffer, ";*.");
		}
		strcat(buffer, Readers[ReadTArray[n]]->
			GetDefExt(Readers[ReadTArray[n]]->GetNumExtensions() - 1));
		strcat(buffer, "|");
	}
	strcat(buffer, "|");
	ReadFilter = new char [strlen(buffer) + 1];
	strcpy(ReadFilter, buffer);
	delete [] buffer;
}
void
G42CodecManager::CreateWriteFilter(void)
{
	WriteTArray = new intf [NumWriters];
	intf n;
	for (n = 0; n < NumWriters; n++)
		WriteTArray[n] = n;
	bool have_swapped = true;
	while (have_swapped)
	{
		have_swapped = false;
		for (intf n = 0; n < NumWriters - 1; n++)
		{
			if (strcmp(Writers[WriteTArray[n]]->GetDefExt(),
				Writers[WriteTArray[n+1]]->GetDefExt()) > 0)
			{
				intf tmp = WriteTArray[n];
				WriteTArray[n] = WriteTArray[n+1];
				WriteTArray[n+1] = tmp;
				have_swapped = true;
			}
		}
	}
	char * buffer = new char [512];
	buffer[0] = 0;
	for (n = 0; n < NumWriters; n++)
	{
		strcat(buffer, Writers[WriteTArray[n]]->GetNameString());
		strcat(buffer, " (*.");
		intf index;
		for (index = 0; index < Writers[WriteTArray[n]]->GetNumExtensions() - 1; index++)
		{
			strcat(buffer, Writers[WriteTArray[n]]->GetDefExt(index));
			strcat(buffer, ";*.");
		}
		strcat(buffer, Writers[WriteTArray[n]]->
			GetDefExt(Writers[WriteTArray[n]]->GetNumExtensions() - 1));
		strcat(buffer, ")|*.");
		for (index = 0; index < Writers[WriteTArray[n]]->GetNumExtensions() - 1; index++)
		{
			strcat(buffer, Writers[WriteTArray[n]]->GetDefExt(index));
			strcat(buffer, ";*.");
		}
		strcat(buffer, Writers[WriteTArray[n]]->
			GetDefExt(Writers[WriteTArray[n]]->GetNumExtensions() - 1));
		strcat(buffer, "|");
	}
	strcat(buffer, "|");
	WriteFilter = new char [strlen(buffer) + 1];
	strcpy(WriteFilter, buffer);
	delete [] buffer;
}
intf
G42CodecManager::GetReadFilterIndex(intf type)
{
	for (intf n = 0; n < NumReaders; n++)
	{
		if (Readers[n]->GetTypeId() == type)
		{
			for (intf m = 0; m < NumReaders; m++)
			{
				if (ReadTArray[m] == n)
					return m;
			}
		}
	}
	throw G42UnknownTypeException();
}
intf
G42CodecManager::GetWriteFilterIndex(intf type)
{
	for (intf n = 0; n < NumWriters; n++)
	{
		if (Writers[n]->GetTypeId() == type)
		{
			for (intf m = 0; m < NumWriters; m++)
			{
				if (WriteTArray[m] == n)
					return m;
			}
		}
	}
	throw G42UnknownTypeException();
}
G42BaseReader *
G42CodecManager::GetReader(G42File * file, const char * ext,
	G42HeapManager & heap, G42Notifier & notifier, G42BaseOptions * options)
{
	G42APtr<char> pattern(new char [MaxPatternSize]);
	file->Rewind();
	file->Read((byte *)(char *)pattern, MaxPatternSize);
	if (ext != 0)
	{
		G42APtr<char> lower_ext(new char [strlen(ext) + 1]);
		strcpy(lower_ext, ext);
		for (char * p = lower_ext; *p; p++)
		{
			*p = (char)tolower(*p);
		}
		for (intf n = 0; n < NumReaders; n++)
		{
			bool match = false;
			for (intf i = 0; i < Readers[n]->GetNumExtensions(); i++)
			{
				if (!strcmp(lower_ext, Readers[n]->GetDefExt(i)))
				{
					match = true;
					break;
				}
			}
			if (!match)
				continue;
			if (!Readers[n]->GetPatternLength())
			{
				G42BaseReader * reader = GetReader(Readers[n], heap, notifier, file,
					options);
				file->Rewind();
				if (reader && reader->TestType())
					return reader;
				delete reader;
				continue;
			}
			const char * sp = pattern;
			const char * dp = Readers[n]->GetPattern();
			match = true;
			for (intf j = 0; j < Readers[n]->GetPatternLength(); j++)
			{
				if (*sp++ != *dp++)
				{
					match = false;
					break;
				}
			}
			if (match)
			{
				G42BaseReader * reader = GetReader(Readers[n], heap, notifier, file,
					options);
				file->Rewind();
				if (reader && reader->TestType())
					return reader;
				delete reader;
			}
		}
	}
	intf Last = -1;
	for (intf n = 0; n < NumReaders; n++)
	{
		if (Readers[n]->IsLast())
		{
			Last = n;
			continue;
		}
		const char * sp = pattern;
		const char * dp = Readers[n]->GetPattern();
		bool match = true;
		for (intf j = 0; j < Readers[n]->GetPatternLength(); j++)
		{
			if (*sp++ != *dp++)
			{
				match = false;
				break;
			}
		}
		if (match)
		{
			G42BaseReader * reader = GetReader(Readers[n], heap, notifier, file,
				options);
			file->Rewind();
			if (reader && reader->TestType())
				return reader;
			delete reader;
		}
	}
	if (Last != -1)
	{
		const char * sp = pattern;
		const char * dp = Readers[Last]->GetPattern();
		bool match = true;
		for (intf j = 0; j < Readers[Last]->GetPatternLength(); j++)
		{
			if (*sp++ != *dp++)
			{
				match = false;
				break;
			}
		}
		if (match)
		{
			G42BaseReader * reader = GetReader(Readers[Last], heap, notifier, file,
				options);
			file->Rewind();
			if (reader && reader->TestType())
				return reader;
			delete reader;
		}
	}
	return 0;
}
G42BaseReader *
G42CodecManager::GetReader(const char * pattern, uint32 bytes,
	G42HeapManager & heap, G42Notifier & notifier, G42BaseOptions * options)
{
	intf Last = -1;
	for (intf n = 0; n < NumReaders; n++)
	{
		if (Readers[n]->IsLast())
		{
			Last = n;
			continue;
		}
		const char * sp = pattern;
		const char * dp = Readers[n]->GetPattern();
		bool match = true;
		for (intf j = 0; j < Readers[n]->GetPatternLength(); j++)
		{
			if (*sp++ != *dp++)
			{
				match = false;
				break;
			}
		}
		if (match)
		{
			G42BaseReader * reader = GetReader(Readers[n], heap, notifier, 0,
				options);
			if (!reader)
				continue;
			bool result;
			try
			{
				result = reader->TestType((const byte *)pattern, bytes);
			}
			catch (G42NotEnoughDataException)
			{
				delete reader;
				throw;
			}
			if (result)
				return reader;
			delete reader;
		}
	}
	if (Last != -1)
	{
		const char * sp = pattern;
		const char * dp = Readers[Last]->GetPattern();
		bool match = true;
		for (intf j = 0; j < Readers[Last]->GetPatternLength(); j++)
		{
			if (*sp++ != *dp++)
			{
				match = false;
				break;
			}
		}
		if (match)
		{
			G42BaseReader * reader = GetReader(Readers[Last], heap, notifier, 0,
				options);
			if (!reader)
				return 0;
			bool result;
			try
			{
				result = reader->TestType((const byte *)pattern, bytes);
			}
			catch (G42NotEnoughDataException)
			{
				delete reader;
				throw;
			}
			if (result)
				return reader;
			delete reader;
		}
	}
	return 0;
}
G42BaseReader *
G42CodecManager::GetReader(G42ReaderInfo * info, G42HeapManager & heap,
	G42Notifier & notifier, G42File * file, G42BaseOptions * options)
{
	G42BaseReader * reader = 0;
	if (!info->IsDll())
	{
		switch (info->GetTypeId())
		{
#if defined(G42_READ_BMP)
		case G42BmpTypeId:
			reader = new G42ImageReaderBmp(heap, notifier, file, options);
			break;
#endif
#if defined(G42_READ_EPS)
		case G42EpsTypeId:
			reader = new G42ImageReaderEps(heap, notifier, file, options);
			break;
#endif
#if defined(G42_READ_GIF)
		case G42GifTypeId:
			reader = new G42ImageReaderGif(heap, notifier, file, options);
			break;
#endif
#if defined(G42_READ_JPEG)
		case G42JpegTypeId:
			reader = new G42ImageReaderJpeg(heap, notifier, file, options);
			break;
#endif
#if defined(G42_READ_PBM)
		case G42PbmTypeId:
			reader = new G42ImageReaderPbm(heap, notifier, file, options);
			break;
#endif
#if defined(G42_READ_PCX)
		case G42PcxTypeId:
			reader = new G42ImageReaderPcx(heap, notifier, file, options);
			break;
#endif
#if defined(G42_READ_PNG)
		case G42PngTypeId:
			reader = new G42ImageReaderPng(heap, notifier, file, options);
			break;
#endif
#if defined(G42_READ_RAS)
		case G42RasTypeId:
			reader = new G42ImageReaderRas(heap, notifier, file, options);
			break;
#endif
#if defined(G42_READ_TARGA)
		case G42TargaTypeId:
			reader = new G42ImageReaderTarga(heap, notifier, file, options);
			break;
#endif
#if defined(G42_READ_TIFF)
		case G42TiffTypeId:
			reader = new G42ImageReaderTiff(heap, notifier, file, options);
			break;
#endif
#if defined(G42_READ_XWD)
		case G42XwdTypeId:
			reader = new G42ImageReaderXwd(heap, notifier, file, options);
			break;
#endif
#if defined(G42_READ_PCM)
		case RWTYPE_RAWPCM:
			reader = new G42RawPcmReader(heap, notifier, file, options);
			break;
#endif
#if defined(G42_READ_WAVE)
		case RWTYPE_WAVE:
			reader = new G42WaveReader(heap, notifier, file, options);
			break;
#endif
#if defined(G42_READ_AIFF)
		case RWTYPE_AIFF:
			reader = new G42AiffReader(heap, notifier, file, options);
			break;
#endif
#if defined(G42_READ_AU)
		case RWTYPE_AU:
			reader = new G42AuReader(heap, notifier, file, options);
			break;
#endif
		}
	}
#if defined(G42_READ_SOUNDS)
	else if (info->GetMediaType() == G42RWInfo::Sound)
		reader = new G42SoundDllReader(info->GetDllName(), info->GetTypeId(),
			heap, notifier, file, options);
#endif
	#ifdef MACOS
	if (options) {}	// Quiet warnings
	#endif
	return reader;
}
G42BaseWriter *
G42CodecManager::GetWriter(intf type_id, G42Notifier & notifier,
	G42BaseOptions * options)
{
	G42BaseWriter * writer = 0;
	intf index = -1;
	for (intf n = 0; n < NumWriters; n++)
	{
		if (Writers[n]->GetTypeId() == type_id)
		{
			index = n;
			break;
		}
	}
	if (index == -1)
		return 0;
	if (!Writers[index]->IsDll())
	{
		switch (Writers[index]->GetTypeId())
		{
#if defined(G42_WRITE_PCM)
		case RWTYPE_RAWPCM:
			writer = new G42RawPcmWriter(notifier, options);
			break;
#endif
#if defined(G42_WRITE_WAVE)
		case RWTYPE_WAVE:
			writer = new G42WaveWriter(notifier, options);
			break;
#endif
#if defined(G42_WRITE_AIFF)
		case RWTYPE_AIFF:
			writer = new G42AiffWriter(notifier, options);
			break;
#endif
#if defined(G42_WRITE_AU)
		case RWTYPE_AU:
			writer = new G42AuWriter(notifier, options);
			break;
#endif
#if defined(G42_WRITE_AU)
		case RWTYPE_RA:
			writer = new G42RaWriter(notifier, options);
			break;
#endif
#if defined(G42_WRITE_BMP)
		case G42BmpTypeId:
			writer = new G42ImageWriterBmp(notifier, options);
			break;
#endif
#if defined(G42_WRITE_EPS)
		case G42EpsTypeId:
			writer = new G42ImageWriterEps(notifier, options);
			break;
#endif
#if defined(G42_WRITE_GIF)
		case G42GifTypeId:
			writer = new G42ImageWriterGif(notifier, options);
			break;
#endif
#if defined(G42_WRITE_JPEG)
		case G42JpegTypeId:
			writer = new G42ImageWriterJpeg(notifier, options);
			break;
#endif
#if defined(G42_WRITE_PBM)
		case G42PbmTypeId:
			writer = new G42ImageWriterPbm(notifier, options);
			break;
#endif
#if defined(G42_WRITE_PCX)
		case G42PcxTypeId:
			writer = new G42ImageWriterPcx(notifier, options);
			break;
#endif
#if defined(G42_WRITE_PNG)
		case G42PngTypeId:
			writer = new G42ImageWriterPng(notifier, options);
			break;
#endif
#if defined(G42_WRITE_RAS)
		case G42RasTypeId:
			writer = new G42ImageWriterRas(notifier, options);
			break;
#endif
#if defined(G42_WRITE_TARGA)
		case G42TargaTypeId:
			writer = new G42ImageWriterTarga(notifier, options);
			break;
#endif
#if defined(G42_WRITE_TIFF)
		case G42TiffTypeId:
			writer = new G42ImageWriterTiff(notifier, options);
			break;
#endif
#if defined(G42_WRITE_XWD)
		case G42XwdTypeId:
			writer = new G42ImageWriterXwd(notifier, options);
			break;
#endif
		}
	}
#if defined(G42_WRITE_SOUNDS)
	else if (Writers[index]->GetMediaType() == G42RWInfo::Sound)
		writer = new G42SoundDllWriter(Writers[index]->GetDllName(),
			Writers[index]->GetTypeId(), notifier, options);
#endif
	#ifdef MACOS
	if (&notifier) {}	// Quiet warnings
	if (options) {}
	#endif
	return writer;
}
G42BaseOptions *
G42CodecManager::GetWriteOptions(intf index, G42BaseInfo * info)
{
	G42BaseWriter * writer = 0;
	if (!Writers[index]->IsDll())
	{
		switch (Writers[index]->GetTypeId())
		{
#if defined(G42_WRITE_PCM)
		case RWTYPE_RAWPCM:
			writer = new G42RawPcmWriter();
			break;
#endif
#if defined(G42_WRITE_WAVE)
		case RWTYPE_WAVE:
			writer = new G42WaveWriter();
			break;
#endif
#if defined(G42_WRITE_AIFF)
		case RWTYPE_AIFF:
			writer = new G42AiffWriter();
			break;
#endif
#if defined(G42_WRITE_AU)
		case RWTYPE_AU:
			writer = new G42AuWriter();
			break;
#endif
#if defined(G42_WRITE_AU)
		case RWTYPE_RA:
			writer = new G42RaWriter();
			break;
#endif
#if defined(G42_WRITE_BMP)
		case G42BmpTypeId:
			writer = new G42ImageWriterBmp();
			break;
#endif
#if defined(G42_WRITE_EPS)
		case G42EpsTypeId:
			writer = new G42ImageWriterEps();
			break;
#endif
#if defined(G42_WRITE_GIF)
		case G42GifTypeId:
			writer = new G42ImageWriterGif();
			break;
#endif
#if defined(G42_WRITE_JPEG)
		case G42JpegTypeId:
			writer = new G42ImageWriterJpeg();
			break;
#endif
#if defined(G42_WRITE_PBM)
		case G42PbmTypeId:
			writer = new G42ImageWriterPbm();
			break;
#endif
#if defined(G42_WRITE_PCX)
		case G42PcxTypeId:
			writer = new G42ImageWriterPcx();
			break;
#endif
#if defined(G42_WRITE_PNG)
		case G42PngTypeId:
			writer = new G42ImageWriterPng();
			break;
#endif
#if defined(G42_WRITE_RAS)
		case G42RasTypeId:
			writer = new G42ImageWriterRas();
			break;
#endif
#if defined(G42_WRITE_TARGA)
		case G42TargaTypeId:
			writer = new G42ImageWriterTarga();
			break;
#endif
#if defined(G42_WRITE_TIFF)
		case G42TiffTypeId:
			writer = new G42ImageWriterTiff();
			break;
#endif
#if defined(G42_WRITE_XWD)
		case G42XwdTypeId:
			writer = new G42ImageWriterXwd();
			break;
#endif
		}
	}
#if defined(G42_WRITE_SOUNDS)
	else if (Writers[index]->GetMediaType() == G42RWInfo::Sound)
		writer = new G42SoundDllWriter(Writers[index]->GetDllName(),
			Writers[index]->GetTypeId());
#endif
	G42BaseOptions * options = writer->GetDefaultOptions(info);
	delete writer;
	return options;
}

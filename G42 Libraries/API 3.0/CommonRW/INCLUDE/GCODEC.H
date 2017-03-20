#if !defined(GCODEC_H)
#define GCODEC_H
#include "gopbase.h"
#include "grbase.h"
#include "gwbase.h"
#include "gstrtab.h"
#ifdef MSWIN
#define STRICT
#include <windows.h>
#endif
class G42CodecManager
{
public:
	G42CodecManager(const G42StringTable & string_table, const char * dir = 0);
	~G42CodecManager(void);
	char * GetReadFilter(void) const
		{return ReadFilter;}
	char * GetWriteFilter(void) const
		{return WriteFilter;}
	intf GetNumReaders(void) const
		{return NumReaders;}
	intf GetNumWriters(void) const
		{return NumWriters;}
	G42ReaderInfo * GetReadInfo(intf index)
		{return Readers[index];}
	G42WriterInfo * GetWriteInfo(intf index)
		{return Writers[index];}
	G42BaseOptions * GetWriteOptions(intf type_id, G42BaseInfo * info,
		G42BaseOptions & options);
	G42BaseOptions * GetWriteOptions(intf type_id, G42BaseInfo * info);
	intf WriteTranslateIndex(intf index) const
		{return WriteTArray[index];}
	intf ReadTranslateIndex(intf index) const
		{return ReadTArray[index];}
	intf GetReadFilterIndex(intf type);
	intf GetWriteFilterIndex(intf type);
	G42BaseReader * GetReader(const char * pattern, uint32 bytes,
		G42HeapManager & heap, G42Notifier & notifier,
		G42BaseOptions * options = 0);
	G42BaseReader * GetReader(G42File * file, const char * ext,
		G42HeapManager & heap, G42Notifier & notifier,
		G42BaseOptions * options = 0);
	G42BaseReader * GetReader(G42ReaderInfo * info, G42HeapManager & heap,
		G42Notifier & notifier, G42File * file = 0, G42BaseOptions * options = 0);
	G42BaseWriter * GetWriter(intf type_id, G42Notifier & notifier,
		G42BaseOptions * options = 0);
	intf GetMaxPatternSize(void)
		{ return MaxPatternSize; }
private:
	G42StringTable			StringTable;
	char * 					Directory;
	char *					ReadFilter;
	char *					WriteFilter;
	G42Vector<G42ReaderInfo *>	Readers;
	G42Vector<G42WriterInfo *>	Writers;
	intf						NumReaders;
	intf						NumWriters;
	intf *					WriteTArray;
	intf *					ReadTArray;
	intf						MaxPatternSize;
	void Clear(void);
	void CreateReadFilter(void);
	void CreateWriteFilter(void);
	void CheckReadDlls(void);
	void CheckWriteDlls(void);
	void QueryReaders(void);
	void QueryWriters(void);
	#ifdef MSWIN
	void AddDllToReaderList(HMODULE mod, const char * dll_name);
	void AddDllToWriterList(HMODULE mod, const char * dll_name);
	#endif
	// Hidden Copy Contructor and operator=
	G42CodecManager(G42CodecManager &) {}
	G42CodecManager & operator= (G42CodecManager &)
		{return *this;}
};
#endif // GCODEC_H sentry

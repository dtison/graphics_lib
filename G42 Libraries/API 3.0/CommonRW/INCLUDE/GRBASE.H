// grbase.h - G42ReaderBase
#if !defined(GRBASE_H)
#define GRBASE_H
#include "gtype.h"
#include "gfile.h"
#include "gnotify.h"
#include "grwinfo.h"
#include "gpcomp.h"
#include "gmem.h"
#include "gopbase.h"
class G42NotEnoughDataException : public G42Exception
{
public:
	G42NotEnoughDataException(uintf error_id = G42_ERROR_SHORT_DATA)
			: G42Exception(error_id) {}
};
class G42TruncatedFileException : public G42Exception
{
public:
	G42TruncatedFileException(uintf error_id = G42_ERROR_TRUNC_FILE)
			: G42Exception(error_id) {}
};
class G42BaseReader : public G42PathComponent
{
public:
	G42BaseReader(void);
	G42BaseReader(G42HeapManager & heap, G42Notifier & notifier,
		G42File * file = 0, G42BaseOptions * options = 0);
	virtual ~G42BaseReader(void);
	const G42ReaderInfo & GetInfo(void)
		{return *Info;}
	uint32 GetBytesUsed(void) const
		{return BytesUsed;}
	void SetOptions(G42BaseOptions * options = 0);
	virtual void SetNotifier(G42Notifier & notifier)
		{Notifier = & notifier;}
	virtual void SetFile(G42File * file = 0)
		{File = file;}
	virtual void SendExtendedInfo(void) = 0;
	virtual uint32 ProcessData(const byte * buffer, uint32 buffer_size) = 0;
	virtual void TerminateStream(void) = 0;
	virtual bool TestType(void) = 0;
	virtual bool TestType(const byte * buffer, uint32 buffer_size) = 0;
protected:
	G42File *					File;
	G42HeapManager *			Heap;
	G42Ptr<G42ReaderInfo>	Info;
	G42Notifier *				Notifier;
	uint32						BytesUsed;
	G42BaseOptions *			Options;
	// Hidden Copy Contructor and operator=
	G42BaseReader(const G42BaseReader &) {}
	G42BaseReader & operator = (G42BaseReader &)
		{return *this;}
};
inline void
G42BaseReader::SetOptions(G42BaseOptions * options)
{
	if (Options)
		delete Options;
	Options = 0;
	if (options)
		Options = options->MakeCopy();
}
#endif // GRBASE_H sentry

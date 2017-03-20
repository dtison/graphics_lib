#if !defined(GREADER_H)
#define GREADER_H
#include "gcodec.h"
#include "gpcomp.h"
#include "gmem.h"
#include "grbase.h"
#include "grbuf.h"
#include "gopbase.h"
class G42Reader : public G42PathComponent
{
public:
	G42Reader(G42CodecManager & codec, G42HeapManager & heap,
		G42Notifier & notifier, const G42FilePath & file_path,
		G42BaseOptions * options = 0);
	G42Reader(G42CodecManager & codec, G42HeapManager & heap,
		G42Notifier & notifier, G42BaseOptions * options = 0);
	G42Reader(G42CodecManager & codec, G42HeapManager & heap,
		const G42FilePath & file_path, G42BaseOptions * options = 0);
	G42Reader(G42CodecManager & codec, G42HeapManager & heap,
		G42BaseOptions * options = 0);
	~G42Reader(void);
	G42FilePath GetFileName(void) const
		{return *FilePath;}
	bool IsValid(void) const
		{return Reader != 0;}
	void SetFileName(const G42FilePath & path);
	void SetNotifier(G42Notifier & notifier);
	void SetOptions(G42BaseOptions * options = 0);
	const G42ReaderInfo * GetReaderInfo(void);
	void SendExtendedInfo(void);
	virtual void BeginProcessing(void);
	void ProcessBuffer(byte * buffer, int32 bytes);
	void TerminateStream(void);
protected:
	G42BaseReader *		Reader;
	G42CodecManager &		Codec;
	G42HeapManager &		Heap;
	G42NotifierVerifier	Notifier;
	G42VirtualBuffer *  	Buffer;
	G42Ptr<G42FilePath> 	FilePath;
	bool						FileName;
	G42File * 				File;
	G42BaseOptions *		Options;
	void Clear(void);
	bool DetermineType(void);
	// Hidden Copy Contructor and operator=
	G42Reader(G42Reader & ref)
		: Heap(ref.Heap), Codec(ref.Codec) {}
	G42Reader & operator= (G42Reader &)
		{return *this;}
};
inline void
G42Reader::SetNotifier(G42Notifier & notifier)
{
	Notifier = &notifier;
	if (Reader)
		Reader->SetNotifier(*Notifier);
}
inline void
G42Reader::SetOptions(G42BaseOptions * options)
{
	if (Options)
		delete Options;
	Options = 0;
	if (options)
		Options = options->MakeCopy();
	if (Reader)
		Reader->SetOptions(Options);
}
#endif // GREADER_H sentry

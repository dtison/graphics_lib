#if !defined(GWRITER_H)
#define GWRITER_H
#include "gopbase.h"
#include "gcodec.h"
#include "gpcomp.h"
#include "gwbase.h"
#include "ginfbase.h"
class G42Writer : public G42PathComponent
{
public:
	G42Writer(G42CodecManager & codec, G42Notifier & notifier, intf type_id,
		const G42FilePath & file_path, G42BaseOptions * options = 0);
	G42Writer(G42CodecManager & codec, G42Notifier & notifier, intf type_id,
		G42BaseOptions * options = 0);
	G42Writer(G42CodecManager & codec, G42Notifier & notifier,
		const G42FilePath & file_path);
	G42Writer(G42CodecManager & codec, G42Notifier & notifier);
	G42Writer(G42CodecManager & codec, intf type_id,
		const G42FilePath & file_path, G42BaseOptions * options = 0);
	G42Writer(G42CodecManager & codec, intf type_id, G42BaseOptions * options = 0);
	G42Writer(G42CodecManager & codec, const G42FilePath & file_path);
	G42Writer(G42CodecManager & codec);
	~G42Writer(void);
	bool IsValid(void) const
		{return Writer != 0;}
	G42FilePath GetFileName(void) const
		{return *FilePath;}
	void SetOptions(G42BaseOptions * options = 0);
	void SetFileName(const G42FilePath & path, G42BaseOptions * options = 0);
	void SetNotifier(G42Notifier & notifier);
	const G42WriterInfo * GetWriterInfo(void);
	virtual void Process(G42PathObject * object, G42PathComponent * sender);
protected:
	G42BaseWriter *	Writer;
	G42CodecManager &	Codec;
	G42NotifierVerifier	Notifier;
	G42Ptr<G42FilePath> 	FilePath;
	bool						FileName;
	intf					TypeId;
	G42BaseOptions *	Options;
	G42BaseInfo *  	Info;
	void Clear(void);
	void VerifyType(void);
	void StartWriter(G42PathComponent * sender);
	// Hidden Copy Contructor and operator=
	G42Writer(G42Writer & ref)
		: Codec(ref.Codec) {}
	G42Writer & operator= (G42Writer &)
		{return *this;}
};
inline void
G42Writer::SetNotifier(G42Notifier & notifier)
{
	Notifier = &notifier;
	if (Writer)
		Writer->SetNotifier(*Notifier);
}
inline void
G42Writer::SetOptions(G42BaseOptions * options)
{
	if (Options)
		delete Options;
	Options = 0;
	if (options)
		Options = options->MakeCopy();
	if (Writer)
		Writer->SetOptions(Options);
}
#endif // GWRITER_H sentry

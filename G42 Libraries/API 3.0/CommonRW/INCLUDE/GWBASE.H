// gwbase.h - G42WriterBase
#if !defined(GWBASE_H)
#define GWBASE_H
#include "gtype.h"
#include "gtools.h"
#include "gfile.h"
#include "gnotify.h"
#include "grwinfo.h"
#include "gpcomp.h"
#include "gopbase.h"
#include "ginfbase.h"
class G42WriteNoBufferException : public G42Exception
{
public:
	G42WriteNoBufferException(uintf error_id = G42_ERROR_NO_BUFFER)
			: G42Exception(error_id) {}
};
class G42CreateBufferMsg : public G42NotifyMessage
{
public:
	G42CreateBufferMsg(void)
		: Buffer(0), BufferSize(0L) {}
	~G42CreateBufferMsg(void) {}
	char * GetBuffer(void) const
		{return Buffer;}
	uint32 GetBufferSize(void) const
		{return BufferSize;}
	void SetBuffer(char * buffer, uint32 buffer_size)
		{Buffer = buffer; BufferSize = buffer_size;}
private:
	char *	Buffer;
	uint32	BufferSize;
	// Hidden Copy Contructor and operator=
	G42CreateBufferMsg(const G42CreateBufferMsg &) {}
	G42CreateBufferMsg & operator = (G42CreateBufferMsg &)
		{return *this;}
};
class G42WriteBufferMsg : public G42NotifyMessage
{
public:
	G42WriteBufferMsg(char * buffer = 0, uint32 buffer_size = 0L)
		: Ok(true), Buffer(buffer), BufferSize(buffer_size),
		BytesLeft(BufferSize), Ptr(Buffer) {}
	~G42WriteBufferMsg(void) {}
	bool IsOk(void) const
		{return Ok;}
	void SetOk(bool ok = true)
		{Ok = ok;}
	char * GetBuffer(void) const
		{return Buffer;}
	uint32 GetBufferSize(void) const
		{return BufferSize;}
	uint32 GetBytesLeft(void) const
		{return BytesLeft;}
	uint32 GetBytesFilled(void) const
		{return Ptr - Buffer;}
	void SetBuffer(char * buffer, uint32 buffer_size);
	void FillBuffer(const byte * buffer, uint32 bytes);
private:
	char *	Buffer;
	char * 	Ptr;
	uint32	BufferSize;
	uint32	BytesLeft;
	bool		Ok;
	// Hidden Copy Contructor and operator=
	G42WriteBufferMsg(const G42WriteBufferMsg &) {}
	G42WriteBufferMsg & operator = (G42WriteBufferMsg &)
		{return *this;}
};
class G42DestroyBufferMsg : public G42NotifyMessage
{
public:
	G42DestroyBufferMsg() {}
	~G42DestroyBufferMsg(void) {}
private:
	// Hidden Copy Contructor and operator=
	G42DestroyBufferMsg(const G42DestroyBufferMsg &) {}
	G42DestroyBufferMsg & operator = (G42DestroyBufferMsg &)
		{return *this;}
};
class G42BaseWriter : public G42PathComponent
{
public:
	G42BaseWriter(void);
	G42BaseWriter(G42Notifier & notifier, const G42FilePath & file_path,
		G42BaseOptions * options = 0);
	G42BaseWriter(G42Notifier & notifier, G42BaseOptions * options = 0);
	virtual ~G42BaseWriter(void);
	const G42WriterInfo & GetInfo(void)
		{return *Info;}
	virtual void SetNotifier(G42Notifier & notifier)
		{Notifier = notifier;}
	virtual void SetFileName(const G42FilePath & file_path, bool open = true);
	void SetOptions(G42BaseOptions * options = 0);
	void Close(void);
	virtual G42BaseOptions * GetDefaultOptions(G42BaseInfo * info,
		G42BaseOptions & options);
	virtual G42BaseOptions * GetDefaultOptions(G42BaseInfo * info);
	virtual void SetHead(void)
		{throw G42PathNotHeadException();}
protected:
	G42File *					File;
	G42Ptr<G42FilePath>		FilePath;
	G42Ptr<G42WriterInfo>	Info;
	G42NotifierVerifier		Notifier;
	G42BaseOptions *			Options;
	G42CreateBufferMsg *		CreateBuffer;
	G42WriteBufferMsg *		WriteBuffer;
	void InitBuffer(void);
	void PutBytes(const byte * buffer, uint32 bytes = 1);
	void PutByte(byte value);
	void PutByteRun(byte value, uint32 bytes);
	void FlushBuffer(void);
	// Hidden Copy Contructor and operator=
	G42BaseWriter(const G42BaseWriter &) {}
	G42BaseWriter & operator = (G42BaseWriter &)
		{return *this;}
};
inline void
G42WriteBufferMsg::SetBuffer(char * buffer, uint32 buffer_size)
{
	Buffer = buffer;
	BufferSize = buffer_size;
	Ptr = Buffer;
	BytesLeft = BufferSize;
}
inline void
G42WriteBufferMsg::FillBuffer(const byte * buffer, uint32 bytes)
{
	memcpy(Ptr, buffer, bytes);
	Ptr += bytes;
	BytesLeft -= bytes;
}
inline void
G42BaseWriter::SetOptions(G42BaseOptions * options)
{
	if (Options)
		delete Options;
	Options = 0;
	if (options)
		Options = options->MakeCopy();
}
#endif // GWBASE_H sentry


#include "greader.h"
#include "gpath.h"
G42Reader::G42Reader(G42CodecManager & codec, G42HeapManager & heap,
	G42Notifier & notifier, const G42FilePath & file_path,
	G42BaseOptions * options)
:
	G42PathComponent(), Notifier(&notifier), Heap(heap), Codec(codec),
	Reader(0), FileName(true), Buffer(0), File(0),
	Options(0)
{
	SetFileName(file_path);
	if (options)
		Options = options->MakeCopy();
}
G42Reader::G42Reader(G42CodecManager & codec,
	G42HeapManager & heap, G42Notifier & notifier, G42BaseOptions * options)
:
	G42PathComponent(), Notifier(&notifier), Heap(heap), Codec(codec),
	Reader(0), FileName(false), Buffer(0), File(0), Options(0)
{
	if (options)
		Options = options->MakeCopy();
}
G42Reader::G42Reader(G42CodecManager & codec, G42HeapManager & heap,
	const G42FilePath & file_path, G42BaseOptions * options)
:
	G42PathComponent(), Heap(heap), Codec(codec),
	Reader(0), FileName(true), Buffer(0), File(0),
	Options(0)
{
	SetFileName(file_path);
	if (options)
		Options = options->MakeCopy();
}
G42Reader::G42Reader(G42CodecManager & codec, G42HeapManager & heap,
	G42BaseOptions * options)
:
	G42PathComponent(), Heap(heap), Codec(codec),
	Reader(0), FileName(false), Buffer(0), File(0), Options(0)
{
	if (options)
		Options = options->MakeCopy();
}
G42Reader::~G42Reader(void)
{
	Clear();
}
void
G42Reader::Clear(void)
{
	if (Reader)
		delete Reader;
	Reader = 0;
	if (Buffer)
		delete Buffer;
	Buffer = 0;
	if (File)
		delete File;
	File = 0;
	if (Options)
		delete Options;
	Options = 0;
}
bool
G42Reader::DetermineType(void)
{
	char * ext = 0;
	if (FileName)
	{
		File = new G42File(*FilePath);
#ifdef MSWIN
		ext = strrchr(*FilePath, '.');
		if (ext != (char *)NULL)
			ext++;
#endif			
	}
	if (File)
	{
		Reader = Codec.GetReader(File, ext, Heap, *Notifier, Options);
		if (Reader)
			Reader->SetFile();
		if (File)
			delete File;
		File = 0;
		return true;
	}
	intf length = Codec.GetMaxPatternSize();
	if (Buffer->GetNumBytes() < length)
		return false;
	try
	{
		Reader = Codec.GetReader((const char *)(byte *)*Buffer, Buffer->GetNumBytes(),
			Heap, *Notifier, Options);
	}
	catch (G42NotEnoughDataException)
	{
		Reader = 0;
		return false;
	}
	return true;
}
void
G42Reader::SetFileName(const G42FilePath & path)
{
	Clear();
	FilePath = new G42FilePath(path);
	FileName = true;
	DetermineType();
}
const G42ReaderInfo *
G42Reader::GetReaderInfo(void)
{
	if (!Reader)
		DetermineType();
	return (Reader ? &(Reader->GetInfo()) : 0);
}
void
G42Reader::SendExtendedInfo(void)
{
	if (!Reader)
		DetermineType();
	if (!Reader)
		return;
	File = new G42File(*FilePath);
	Reader->SetFile(File);
	Reader->SendExtendedInfo();
	Reader->SetFile();
	delete File;
	File = 0;
}
void
G42Reader::BeginProcessing(void)
{
	if (!Receiver)
		throw G42PathNoReceiverException();
	if (!FileName)
	{
		G42PathBegin * begin = new G42PathBegin();
		Receiver->Process(begin, this);
		Buffer = new G42VirtualBuffer();
		return;
	}
	if (!Reader)
		throw G42UnknownTypeException();
		
	File = new G42File(*FilePath);
	Reader->SetFile(File);
	Reader->SetReceiver(Receiver);
	Receiver->ReplaceSender(this, Reader);
	AddComponentToPath(Reader);
	Receiver = 0;
	ClearPath(); // remove ourselves from path;
	try
	{
		Reader->BeginProcessing();
	}
	catch (...)
	{
		Reader->SetFile();
		delete File;
		File = 0;
		throw;
	}
}
void
G42Reader::ProcessBuffer(byte * buffer, int32 bytes)
{
	if (!Buffer || !Buffer->IsValid())
		throw G42InternalException();
	G42VirtualBufferWrap vb_wrap(*Buffer);
	Buffer->AddBytes(buffer, bytes);
	if (!Reader)
	{
		if (!DetermineType())
			return;
		if (!Reader)
			throw G42UnknownTypeException();
		Reader->SetReceiver(Receiver);
		Receiver->ReplaceSender(this, Reader);
	}
	while (Buffer->GetNumBytes())
	{
		int32 bytes_used = Reader->ProcessData(*Buffer, Buffer->GetNumBytes());
		if (!bytes_used)
			break;
		Buffer->ReleaseBytes(bytes_used);
	}
}
void
G42Reader::TerminateStream(void)
{
	Reader->TerminateStream();
	Receiver = 0;
	if (Buffer)
		delete Buffer;
	Buffer = 0;
}

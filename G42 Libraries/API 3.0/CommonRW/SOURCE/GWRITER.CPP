#include "gwriter.h"
G42Writer::G42Writer(G42CodecManager & codec, G42Notifier & notifier,
	intf type_id, const G42FilePath & file_path, G42BaseOptions * options)
	: G42PathComponent(), Notifier(&notifier), Codec(codec), Info(0),
	Writer(0), Options(0), TypeId(type_id), FileName(true)
{
	SetFileName(file_path, options);
}
G42Writer::G42Writer(G42CodecManager & codec, G42Notifier & notifier,
	intf type_id, G42BaseOptions * options)
	: G42PathComponent(), Notifier(&notifier), Codec(codec), Info(0),
	Writer(0), Options(0), TypeId(type_id), FileName(false)
{
	if (options)
		Options = options->MakeCopy();
	VerifyType();
}
G42Writer::G42Writer(G42CodecManager & codec, G42Notifier & notifier,
	const G42FilePath & file_path)
	: G42PathComponent(), Notifier(&notifier), Codec(codec), Info(0),
	Writer(0), Options(0), TypeId(-1), FileName(true)
{
	SetFileName(file_path);
}
G42Writer::G42Writer(G42CodecManager & codec, G42Notifier & notifier)
	: G42PathComponent(), Notifier(&notifier), Codec(codec), Info(0),
	Writer(0), Options(0), TypeId(-1), FileName(false)
{}
G42Writer::G42Writer(G42CodecManager & codec, intf type_id,
	const G42FilePath & file_path, G42BaseOptions * options)
	: G42PathComponent(), Codec(codec), Info(0),
	Writer(0), FileName(true), Options(0), TypeId(type_id)
{
	SetFileName(file_path, options);
}
G42Writer::G42Writer(G42CodecManager & codec, intf type_id,
	G42BaseOptions * options)
	: G42PathComponent(), Codec(codec), Info(0),
	Writer(0), FileName(false), Options(0), TypeId(type_id)
{
	if (options)
		Options = options->MakeCopy();
	VerifyType();
}
G42Writer::G42Writer(G42CodecManager & codec, const G42FilePath & file_path)
	: G42PathComponent(), Codec(codec), Info(0),
	Writer(0), FileName(true), Options(0), TypeId(-1)
{
	SetFileName(file_path);
}
G42Writer::G42Writer(G42CodecManager & codec)
	: G42PathComponent(), Codec(codec), Info(0),
	Writer(0), FileName(false), Options(0), TypeId(-1)
{}
G42Writer::~G42Writer(void)
{
	Clear();
}
void
G42Writer::Clear(void)
{
	if (Writer)
		delete Writer;
	Writer = 0;
	if (Options)
		delete Options;
	Options = 0;
	if (Info)
		delete Info;
	Info = 0;
}
void
G42Writer::SetFileName(const G42FilePath & path, G42BaseOptions * options)
{
	Clear();
	SetOptions(options);
	FilePath = new G42FilePath(path);
	FileName = true;
	VerifyType();
}
const G42WriterInfo *
G42Writer::GetWriterInfo(void)
{
	if (!Writer)
		VerifyType();
	return (Writer ? &(Writer->GetInfo()) : 0);
}
void
G42Writer::VerifyType(void)
{
	if (FileName)
	{
		G42File file(*FilePath, G42File::OpenWrite);
	}
	if (TypeId != -1)
		Writer = Codec.GetWriter(TypeId, *Notifier, Options);
}
void
G42Writer::Process(G42PathObject * object, G42PathComponent * sender)
{
	G42BaseOptions * options = dynamic_cast<G42BaseOptions *>(object);
	if (options)
	{
		if (TypeId == -1)
		{
			if (options->GetTypeId() == -1)
			{
				delete options;
				throw G42InvalidParameterException();
			}
			SetOptions(options);
			delete options;
			StartWriter(sender);
		}
		else
			delete options;
		return;
	}
	G42BaseInfo * info = dynamic_cast<G42BaseInfo *>(object);
	if (info)
	{
		if (TypeId == -1)
			Info = info;
		else
			Writer->Process(info, sender);
		return;
	}
	if (typeid(*object) == typeid(G42PathBegin))
	{
		if (TypeId != -1)
		{
			if (!Writer)
				VerifyType();
			if (!Writer)
				throw G42UnknownTypeException();
			if (FileName)
				Writer->SetFileName(*FilePath);
			Writer->Process(object, sender);
		}
		else
			delete object;
	}
	else if (typeid(*object) == typeid(G42PathEnd))
	{
		Writer->Process(object, sender);
		ClearPath();
		Writer->Close();
	}
	else
		Writer->Process(object, sender);
}
void
G42Writer::StartWriter(G42PathComponent * sender)
{
	TypeId = Options->GetTypeId();
	VerifyType();
	if (!Writer)
		throw G42UnknownTypeException();
	if (FileName)
		Writer->SetFileName(*FilePath);
	G42PathBegin * begin = new G42PathBegin();
	Writer->Process(begin, sender);
	Writer->Process(Info, sender);
	Info = 0;
}

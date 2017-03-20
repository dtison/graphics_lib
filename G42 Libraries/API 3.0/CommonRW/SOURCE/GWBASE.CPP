// gwbase.cpp - G42WriterBase
#include "gwbase.h"
G42BaseWriter::G42BaseWriter(void)
	: File(0), Options(0), CreateBuffer(0), WriteBuffer(0)
{}
G42BaseWriter::G42BaseWriter(G42Notifier & notifier,
	const G42FilePath & file_path, G42BaseOptions * options)
	: File(0), Options(0), CreateBuffer(0), WriteBuffer(0)
{
	Notifier = notifier;
	SetFileName(file_path);
	SetOptions(options);
}
G42BaseWriter::G42BaseWriter(G42Notifier & notifier, G42BaseOptions * options)
	: File(0), Options(0), CreateBuffer(0), WriteBuffer(0)
{
	Notifier = notifier;
	SetOptions(options);
}
G42BaseWriter::~G42BaseWriter(void)
{
	if (CreateBuffer)
		delete CreateBuffer;
	CreateBuffer = 0;
	if (WriteBuffer)
		delete WriteBuffer;
	WriteBuffer = 0;
	if (Options)
		delete Options;
	Options = 0;
	if (File)
		delete File;
	File = 0;
}
void
G42BaseWriter::SetFileName(const G42FilePath & path, bool open)
{
	FilePath = new G42FilePath(path);
	if (File)
		delete File;
	File = 0;
	if (open)
		File = new G42File(*FilePath, G42File::OpenWrite);
}
void
G42BaseWriter::Close()
{
	if (File)
		delete File;
	File = 0;
}
void
G42BaseWriter::InitBuffer(void)
{
	if (!File)
	{
		CreateBuffer = new G42CreateBufferMsg();
		Notifier->ProcessMessage(CreateBuffer);
		if (!CreateBuffer->GetBuffer() || !CreateBuffer->GetBufferSize())
		{
			delete CreateBuffer;
			CreateBuffer = 0;
			throw G42WriteNoBufferException();
		}
		WriteBuffer = new G42WriteBufferMsg(CreateBuffer->GetBuffer(),
			CreateBuffer->GetBufferSize());
		delete CreateBuffer;
		CreateBuffer = 0;
	}
}
void
G42BaseWriter::PutBytes(const byte * buffer, uint32 bytes)
{
	if (!File)
	{
		while (WriteBuffer->IsOk() && bytes)
		{
			int32 bytes_to_send = bytes;
			if (bytes_to_send > WriteBuffer->GetBytesLeft())
				bytes_to_send = WriteBuffer->GetBytesLeft();
			WriteBuffer->FillBuffer(buffer, bytes_to_send);
			if (WriteBuffer->GetBytesLeft() <= 0)
				Notifier.ProcessMessage(WriteBuffer);
			bytes -= bytes_to_send;
			buffer += bytes_to_send;
		}
	}
	else
		File->Write(buffer, bytes);
}
void
G42BaseWriter::PutByteRun(byte value, uint32 bytes)
{
	if (!File)
	{
		while (WriteBuffer->IsOk() && bytes)
		{
			int32 bytes_to_send = bytes;
			if (bytes_to_send > WriteBuffer->GetBytesLeft())
			{
				bytes_to_send = WriteBuffer->GetBytesLeft();
			}
			byte * buffer = new byte [bytes_to_send];
			memset(buffer, value, bytes_to_send);
			WriteBuffer->FillBuffer(buffer, bytes_to_send);
			delete [] buffer;
			if (WriteBuffer->GetBytesLeft() <= 0)
			{
				Notifier.ProcessMessage(WriteBuffer);
			}
			bytes -= bytes_to_send;
		}
	}
	else
	{
		byte * buffer = new byte [bytes];
		memset(buffer, value, bytes);
		File->Write(buffer, bytes);
		delete [] buffer;
	}
}
void
G42BaseWriter::PutByte(byte value)
{
	if (!File)
	{
		if (WriteBuffer->IsOk())
		{
			WriteBuffer->FillBuffer(&value, 1);
			if (WriteBuffer->GetBytesLeft() <= 0)
			{
				Notifier.ProcessMessage(WriteBuffer);
			}
		}
	}
	else
	{
		File->Write(&value, 1);
	}
}
void
G42BaseWriter::FlushBuffer(void)
{
	if (!File)
	{
		if (WriteBuffer->GetBytesFilled())
			Notifier.ProcessMessage(WriteBuffer);
		delete WriteBuffer;
		WriteBuffer = 0;
		G42DestroyBufferMsg * msg = new G42DestroyBufferMsg();
		Notifier->ProcessMessage(msg);
		delete msg;
	}
}
G42BaseOptions *
G42BaseWriter::GetDefaultOptions(G42BaseInfo *, G42BaseOptions &)
{
	G42BaseOptions * options = new G42BaseOptions(Info->GetTypeId());
	return options;
}
G42BaseOptions *
G42BaseWriter::GetDefaultOptions(G42BaseInfo *)
{
	G42BaseOptions * options = new G42BaseOptions(Info->GetTypeId());
	return options;
}

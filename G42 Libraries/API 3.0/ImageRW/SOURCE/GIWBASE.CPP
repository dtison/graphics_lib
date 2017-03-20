// ribase.cpp - raster image base class
/*************************************************************
	File:          g42ribas.cpp
	Copyright (c) 1996, Group 42, Inc.
	Description:   Base class for reading raster files
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "giwbase.h"
G42ImageWriterBase::G42ImageWriterBase(void)
:
	G42BaseWriter(),
	Width(0), Height(0), RowBytes(0), Depth(0),
	CurrentRow(0), Valid(true), IntegerFormat(Motorola), MultiMode(false),
	SkipUntilEnd(false)
{
}
G42ImageWriterBase::G42ImageWriterBase(G42Notifier & notifier,
	G42BaseOptions * options)
:
	G42BaseWriter(notifier, options),
	Width(0), Height(0), RowBytes(0), Depth(0),
	CurrentRow(0), Valid(true), IntegerFormat(Motorola), MultiMode(false),
	SkipUntilEnd(false)
{
}
G42ImageWriterBase::~G42ImageWriterBase()
{
	Valid = false;
}
void
G42ImageWriterBase::SetInfo(intf type_id, const char * def_ext,
	const char * name_string, bool options)
{
	Info = new G42WriterInfo(type_id, def_ext, name_string,
		false, 0, 3, 0, G42RWInfo::Image, options);
	G42ThrowIfNull(Info);
}
bool
G42ImageWriterBase::WriteInfo(G42ImageDescriptor * info)
{
	Width = info->GetWidth();
	Height = info->GetHeight();
	Depth = info->GetDepth();
	RowBytes = ((Width * (uint32)Depth + 7) >> 3);
	return Valid;
}
bool
G42ImageWriterBase::WriteRow(const byte * row, const byte *)
{
	PutBytes(row, RowBytes);
	CurrentRow++;
	return Valid;
}
uint32
G42ImageWriterBase::NextRow(void)
{
	return CurrentRow;
}
bool
G42ImageWriterBase::NeedRow(void)
{
	return (CurrentRow < Height);
}
bool
G42ImageWriterBase::WriteEnd(void)
{
	PutEnd();
	return Valid;
}
void
G42ImageWriterBase::PutEnd(void)
{
	FlushBuffer();
}
void
G42ImageWriterBase::PutUInt32(uint32 number, G42IntType type)
{
	if (type == Default)
		type = IntegerFormat;
	if (type == Motorola)
	{
		PutByte((byte)((number >> 24) & 0xff));
		PutByte((byte)((number >> 16) & 0xff));
		PutByte((byte)((number >> 8) & 0xff));
		PutByte((byte)((number) & 0xff));
	}
	else if (type == Intel)
	{
		PutByte((byte)((number) & 0xff));
		PutByte((byte)((number >> 8) & 0xff));
		PutByte((byte)((number >> 16) & 0xff));
		PutByte((byte)((number >> 24) & 0xff));
	}
}
void
G42ImageWriterBase::PutUInt16(uint16 number, G42IntType type)
{
	if (type == Default)
		type = IntegerFormat;
	if (type == Motorola)
	{
		PutByte((byte)((number >> 8) & 0xff));
		PutByte((byte)((number) & 0xff));
	}
	else if (type == Intel)
	{
		PutByte((byte)((number) & 0xff));
		PutByte((byte)((number >> 8) & 0xff));
	}
}
int
G42ImageWriterBase::GetDepth(void)
{
	return Depth;
}
bool
G42ImageWriterBase::CanDoMultiple(void)
{
	return false;
}
bool
G42ImageWriterBase::WriteMultiInfo(G42MultipleImageDescriptor *)
{
	return false;
}
bool
G42ImageWriterBase::WriteMultiEnd(G42MultipleImageDescriptor *)
{
	return false;
}
bool
G42ImageWriterBase::WriteFrame(G42AnimatedImageFrame *)
{
	return false;
}
void
G42ImageWriterBase::Process(G42PathObject * object, G42PathComponent *)
{
	if (SkipUntilEnd)
	{
		if (dynamic_cast<G42PathEnd *>(object))
		{
			Sender = 0;
		}
		delete object;
		return;
	}
	if (dynamic_cast<G42PathBegin *>(object))
	{
		InitBuffer();
		delete object;
	}
	else if (dynamic_cast<G42MultipleImageDescriptor *>(object))
	{
		G42MultipleImageDescriptor * desc =
			dynamic_cast<G42MultipleImageDescriptor *>(object);
		MultiDescriptor = desc;
		if (CanDoMultiple())
		{
			WriteMultiInfo(desc);
			MultiMode = true;
		}
	}
	else if (dynamic_cast<G42ImageDescriptor *>(object))
	{
		G42ImageDescriptor * desc = dynamic_cast<G42ImageDescriptor *>(object);
		if (Descriptor && MultiMode)
		{
			WriteEnd();
		}
		else if (Descriptor && !MultiMode)
		{
			SkipUntilEnd = true;
			WriteEnd();
			delete desc;
			return;
		}
		Descriptor = desc;
		WriteInfo(desc);
		Data = new G42Ptr<G42ImageData> [Descriptor->GetNumStrips()];
	}
	else if (dynamic_cast<G42AnimatedImageFrame *>(object))
	{
		if (MultiMode)
		{
			G42AnimatedImageFrame * frame =
				dynamic_cast<G42AnimatedImageFrame *>(object);
			WriteFrame(frame);
		}
		delete object;
	}
	else if (dynamic_cast<G42ImageData *>(object))
	{
		G42ImageData * data = dynamic_cast<G42ImageData *>(object);
		Data[data->GetStripNum()] = data;
		while (NeedRow())
		{
			uint32 row_num = NextRow();
			uint32 strip_num = Descriptor->GetStripNum(row_num);
			if (!Data[strip_num])
			{
				break;
			}
			G42ImageData * data = Data[strip_num];
			G42ReadImageData lock(*data);
			uint32 row_in_strip = Descriptor->GetRowNumInStrip(row_num);
			const byte * row = lock.GetRow(row_in_strip);
			if (Descriptor->IsTransparent())
			{
				WriteRow(row, lock.GetRow(row_in_strip, true));
			}
			else
			{
				WriteRow(row, 0);
			}
		}
	}
	else if (dynamic_cast<G42PathEnd *>(object))
	{
		WriteEnd();
		if (MultiMode)
		{
			WriteMultiEnd(MultiDescriptor);
		}
		PutEnd();
		Sender = 0;
		delete object;
	}
	else
	{
		delete object;
	}
}
void
G42ImageWriterBase::AskForOptions(G42ImageReadWriteOptions * options)
{
	if (Options)
	{
		delete Options;
	}
	Options = options;
	Notifier->ProcessMessage(options);
}

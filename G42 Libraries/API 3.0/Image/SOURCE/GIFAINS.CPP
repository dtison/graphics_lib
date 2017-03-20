// gifains.cpp - animated image inserter
#include "gifains.h"
G42ImageAnimatedInserter::G42ImageAnimatedInserter(
	uint32 insert_before_frame_index, G42LinkOrder order)
:
	InsertFrame(insert_before_frame_index), InsertSender(0),
	NextSenderIsInsertedImage(order == FirstLinkIsInsertedImage),
	BeginSent(false), NumTempSenderObjects(0), NumTempInserterObjects(0),
	NumSenderObjects(0), NumInserterObjects(0), SenderFirstFrame(true),
	InserterFirstFrame(true), Mode(SendBeforeInsert), HadInserterEnd(false),
	HadSenderEnd(false), SenderFrame(0)
{
}
G42ImageAnimatedInserter::~G42ImageAnimatedInserter(void)
{
	if (InsertSender)
	{
		InsertSender->ReceiverDestructing(this);
		InsertSender = 0;
	}
	// delete any stored objects
}
G42ImageFilter *
G42ImageAnimatedInserter::MakeCopy(void) const
{
	G42ImageAnimatedInserter * inserter =
		new G42ImageAnimatedInserter(InsertFrame);
	G42ThrowIfNull(inserter);
	return inserter;
}
void
G42ImageAnimatedInserter::ClearPath(bool path_excepted)
{
	if (InsertSender)
	{
		InsertSender->ReceiverDestructing(this);
		InsertSender = 0;
	}
	// delete any stored objects
	G42ImageFilter::ClearPath(path_excepted);
}
void
G42ImageAnimatedInserter::HandleBegin(G42PathBegin * begin, G42PathComponent *)
{
	if (!BeginSent)
	{
		if (!Receiver || !Sender || !InsertSender)
		{
			throw G42Exception(0);
		}
		Receiver->Process(begin, this);
		BeginSent = true;
	}
	else
	{
		delete begin;
	}
}
void
G42ImageAnimatedInserter::HandleDescriptor(G42ImageDescriptor * descriptor,
	G42PathComponent * sender)
{
	if (sender == Sender)
	{
		if (SenderFirstFrame)
		{
			SenderFirstFrame = false;
		}
		else
		{
			if (Mode == SendInsert)
			{
				if (NumTempSenderObjects)
				{
					SenderObjects[NumSenderObjects] =
						new G42PathObject * [NumTempSenderObjects];
					G42ThrowIfNull(SenderObjects[NumSenderObjects]);
					for (uint32 i = 0; i < NumTempSenderObjects; i++)
					{
						(SenderObjects[NumSenderObjects])[i] =
							TempSenderObjects[i];
						TempSenderObjects[i] = 0;
					}
					NumSenderObjectsArray[NumSenderObjects] = NumTempSenderObjects;
					NumTempSenderObjects = 0;
					NumSenderObjects++;
				}
			}
			SenderFrame++;
		}
		if (SenderFrame == InsertFrame)
		{
			Mode = SendInsert;
			// send any insert data
			for (uint32 i = 0; i < NumInserterObjects; i++)
			{
				for (uint32 j = 0; j < NumInserterObjectsArray[i]; j++)
				{
					G42PathObject * object = (InserterObjects[i])[j];
					(InserterObjects[i])[j] = 0;
					Receiver->Process(object, this);
				}
			}
			NumInserterObjects = 0;
			InserterObjects.Clear();
			NumInserterObjectsArray.Clear();
			for (uint32 k = 0; k < NumTempInserterObjects; k++)
			{
				G42PathObject * object = TempInserterObjects[k];
				TempInserterObjects[k] = 0;
				Receiver->Process(object, this);
			}
			NumTempInserterObjects = 0;
			TempInserterObjects.Clear();
			if (HadInserterEnd)
			{
				Mode = SendAfterInsert;
			}
		}
		if (Mode == SendBeforeInsert || Mode == SendAfterInsert)
		{
			Receiver->Process(descriptor, this);
		}
		else
		{
			TempSenderObjects[NumTempSenderObjects++] = descriptor;
		}
	}
	else // inserter
	{
		if (Mode == SendInsert)
		{
			if (NumTempInserterObjects)
			{
				InserterObjects[NumInserterObjects] =
					new G42PathObject * [NumTempInserterObjects];
				G42ThrowIfNull(InserterObjects[NumInserterObjects]);
				
				for (uint32 i = 0; i < NumTempInserterObjects; i++)
				{
					(InserterObjects[NumInserterObjects])[i] =
						TempInserterObjects[i];
					TempInserterObjects[i] = 0;
				}
				NumInserterObjectsArray[NumInserterObjects] = NumTempInserterObjects;
				NumTempInserterObjects = 0;
				NumInserterObjects++;
			}
		}
		if (Mode == SendInsert)
		{
			Receiver->Process(descriptor, this);
		}
		else
		{
			TempInserterObjects[NumTempInserterObjects++] = descriptor;
		}
	}
}
void
G42ImageAnimatedInserter::HandleData(G42ImageData * data,
	G42PathComponent * sender)
{
	if (sender == Sender)
	{
		if (Mode == SendBeforeInsert || Mode == SendAfterInsert)
		{
			Receiver->Process(data, this);
		}
		else
		{
			TempSenderObjects[NumTempSenderObjects++] = data;
		}
	}
	else // inserter
	{
		if (Mode == SendInsert)
		{
			Receiver->Process(data, this);
		}
		else
		{
			TempInserterObjects[NumTempInserterObjects++] = data;
		}
	}
}
void
G42ImageAnimatedInserter::HandleEnd(G42PathEnd * end,
	G42PathComponent * sender)
{
	// depending upon which end we get, we may switch modes and write
	// out any stored data
	if (sender == Sender)
	{
		if (Mode == SendBeforeInsert)
		{
			Mode = SendInsert;
			// send any previously stored insert data
			for (uint32 i = 0; i < NumInserterObjects; i++)
			{
				for (uint32 j = 0; j < NumInserterObjectsArray[i]; j++)
				{
					G42PathObject * object = (InserterObjects[i])[j];
					(InserterObjects[i])[j] = 0;
					Receiver->Process(object, this);
				}
			}
			NumInserterObjects = 0;
			InserterObjects.Clear();
			NumInserterObjectsArray.Clear();
			for (uint32 k = 0; k < NumTempInserterObjects; k++)
			{
				G42PathObject * object = TempInserterObjects[k];
				TempInserterObjects[k] = 0;
				Receiver->Process(object, this);
			}
			NumTempInserterObjects = 0;
			TempInserterObjects.Clear();
		}
		HadSenderEnd = true;
	}
	else // inserter
	{
		if (Mode == SendInsert)
		{
			Mode = SendAfterInsert;
			// send any previously stored sender data
			for (uint32 i = 0; i < NumSenderObjects; i++)
			{
				for (uint32 j = 0; j < NumSenderObjectsArray[i]; j++)
				{
					G42PathObject * object = (SenderObjects[i])[j];
					(SenderObjects[i])[j] = 0;
					Receiver->Process(object, this);
				}
			}
			NumSenderObjects = 0;
			SenderObjects.Clear();
			NumSenderObjectsArray.Clear();
			for (uint32 k = 0; k < NumTempSenderObjects; k++)
			{
				G42PathObject * object = TempSenderObjects[k];
				TempSenderObjects[k] = 0;
				Receiver->Process(object, this);
			}
			NumTempSenderObjects = 0;
			TempSenderObjects.Clear();
		}
		HadInserterEnd = true;
	}
	if (HadInserterEnd && HadSenderEnd)
	{
		Receiver->Process(end, this);
	}
	else
	{
		delete end;
	}
}
void
G42ImageAnimatedInserter::HandleUnknown(G42PathObject * object,
	G42PathComponent * sender)
{
	if (dynamic_cast<G42MultipleImageDescriptor *>(object))
	{
		G42MultipleImageDescriptor * desc =
			dynamic_cast<G42MultipleImageDescriptor *>(object);
		if (sender == Sender)
		{
			OldMultiDescriptor = desc;
		}
		else
		{
			InsertMultiDescriptor = desc;
		}
		if (OldMultiDescriptor && !NewMultiDescriptor)
		{
			NewMultiDescriptor = OldMultiDescriptor->MakeCopy();
			if (InsertMultiDescriptor && InsertMultiDescriptor->GetNumImages() &&
				OldMultiDescriptor->GetNumImages())
			{
				NewMultiDescriptor->SetNumImages(
					OldMultiDescriptor->GetNumImages() +
					InsertMultiDescriptor->GetNumImages());
			}
			else
			{
				NewMultiDescriptor->SetNumImages(0);
			}
			G42MultipleImageDescriptor * new_desc = NewMultiDescriptor->MakeCopy();
			G42ThrowIfNull(new_desc);
			Receiver->Process(new_desc, this);
		}
		return;
	}
	if (sender == Sender)
	{
		if (Mode == SendBeforeInsert || Mode == SendAfterInsert)
		{
			Receiver->Process(object, this);
		}
		else
		{
			TempSenderObjects[NumTempSenderObjects++] = object;
		}
	}
	else // inserter
	{
		if (Mode == SendInsert)
		{
			Receiver->Process(object, this);
		}
		else
		{
			TempInserterObjects[NumTempInserterObjects++] = object;
		}
	}
}
void
G42ImageAnimatedInserter::SenderDestructing(G42PathComponent * sender)
{
	if (sender == InsertSender)
	{
		InsertSender = 0;
	}
	else if (sender == Sender)
	{
		Sender = 0;
	}
}
void
G42ImageAnimatedInserter::ReplaceSender(G42PathComponent * old_sender,
		G42PathComponent * new_sender)
{
	if (old_sender == InsertSender)
	{
		InsertSender = new_sender;
	}
	else if (old_sender == Sender)
	{
		Sender = new_sender;
	}
}
void
G42ImageAnimatedInserter::SetSender(G42PathComponent * sender)
{
	if (!sender || sender == this)
	{
		throw G42InvalidParameterException();
	}
	if (NextSenderIsInsertedImage)
	{
		if (InsertSender)
		{
			throw G42PathMultipleSenderException();
		}
		InsertSender = sender;
	}
	else
	{
		if (Sender)
		{
			throw G42PathMultipleSenderException();
		}
		Sender = sender;
	}
}

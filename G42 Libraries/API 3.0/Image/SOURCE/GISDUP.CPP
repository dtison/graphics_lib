// gisdup.cpp
#define G42_EXPAND_PTR
#include "gisdup.h"
#include "giaframe.h"
#include "giadesc.h"
#include "gidesc.h"
#include "gidata.h"
#include "girwopt.h"
G42ImageStreamDuplicator::G42ImageStreamDuplicator()
:
	NumReceivers(0)
{
}
G42ImageStreamDuplicator::~G42ImageStreamDuplicator()
{
	for (intf i = 0; i < NumReceivers; i++)
	{
		if (Receivers[i])
			Receivers[i]->SenderDestructing(this);
	}
}
void
G42ImageStreamDuplicator::ClearPath(bool path_excepted)
{
	NumReceivers = 0;
	for (intf i = 0; i < NumReceivers; i++)
	{
		if (Receivers[i])
			Receivers[i]->SenderDestructing(this);
	}
	G42PathComponent::ClearPath(path_excepted);
}
void
G42ImageStreamDuplicator::ReceiverDestructing(G42PathComponent * receiver)
{
	for (intf i = 0; i < NumReceivers; i++)
	{
		if (Receivers[i] == receiver)
			Receivers[i] = 0;
	}
}
void
G42ImageStreamDuplicator::SetReceiver(G42PathComponent * receiver)
{
	Receivers[NumReceivers++] = receiver;
}
void
G42ImageStreamDuplicator::ReplaceReceiver(G42PathComponent * old_receiver,
	G42PathComponent * new_receiver)
{
	for (intf i = 0; i < NumReceivers; i++)
	{
		if (Receivers[i] == old_receiver)
			Receivers[i] = new_receiver;
	}
}
void
G42ImageStreamDuplicator::SetHead(void)
{
	throw G42PathNotHeadException();
}
void
G42ImageStreamDuplicator::Process(G42PathObject * object, G42PathComponent *)
{
	G42ExpandableArray <G42Ptr <G42PathObject> > objects;
	intf num_objects = 0;
	bool at_end = false;
	if (typeid(*object) == typeid(G42ImageDescriptor))
	{
		G42ImageDescriptor * desc = dynamic_cast<G42ImageDescriptor *>(object);
		objects[0].Set(desc);
		for (intf i = 1; i < NumReceivers; i++)
		{
			objects[i].Set(new G42ImageDescriptor(*desc));
		}
		num_objects = NumReceivers;
	}
	if (typeid(*object) == typeid(G42AnimatedImageFrame))
	{
		G42AnimatedImageFrame * desc =
			dynamic_cast<G42AnimatedImageFrame *>(object);
		objects[0].Set(desc);
		for (intf i = 1; i < NumReceivers; i++)
		{
			objects[i].Set(new G42AnimatedImageFrame(*desc));
		}
		num_objects = NumReceivers;
	}
	else if (dynamic_cast<G42ImageData *>(object))
	{
		G42ImageData * strip = dynamic_cast<G42ImageData *>(object);
		objects[0].Set(strip);
		for (intf i = 1; i < NumReceivers; i++)
		{
			objects[i].Set(strip->MakeCopy());
		}
		num_objects = NumReceivers;
	}
	else if (dynamic_cast<G42MultipleImageDescriptor *>(object))
	{
		G42MultipleImageDescriptor * desc =
			dynamic_cast<G42MultipleImageDescriptor *>(object);
		objects[0].Set(desc);
		for (intf i = 1; i < NumReceivers; i++)
		{
			objects[i].Set(desc->MakeCopy());
		}
		num_objects = NumReceivers;
	}
	else if (dynamic_cast<G42ImageReadWriteOptions *>(object))
	{
		G42ImageReadWriteOptions * options =
			dynamic_cast<G42ImageReadWriteOptions *>(object);
		objects[0].Set(options);
		for (intf i = 1; i < NumReceivers; i++)
		{
			objects[i].Set(options->MakeCopy());
		}
		num_objects = NumReceivers;
	}
	else if (typeid(*object) == typeid(G42PathBegin))
	{
		if (NumReceivers <= 0)
			throw G42PathNoReceiverException();
			
		objects[0].Set(object);
		for (intf i = 1; i < NumReceivers; i++)
		{
			objects[i].Set(new G42PathBegin());
		}
		num_objects = NumReceivers;
	}
	else if (typeid(*object) == typeid(G42PathEnd))
	{
		objects[0].Set(object);
		for (intf i = 1; i < NumReceivers; i++)
		{
			objects[i].Set(new G42PathEnd());
		}
		num_objects = NumReceivers;
		at_end = true;
	}
	if (num_objects)
	{
		for (intf i = 0; i < num_objects; i++)
			Receivers[i]->Process(objects[i].Release(), this);
	}
	else
	{
		delete object;
	}
	if (at_end)
	{
		NumReceivers = 0;
		Sender = 0;
	}
}

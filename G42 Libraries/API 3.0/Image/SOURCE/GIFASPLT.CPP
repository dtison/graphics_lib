// gifasplt.h - animated image frame splitter
#include "gifasplt.h"
#include "gimdesc.h"
G42ImageAnimatedSplitter::G42ImageAnimatedSplitter(uint32 frame,
	G42LinkOrder order)
:
	NumFrames(1), SplitReceiver(0), CurrentFrame(0), FirstFrame(true),
	SplitFrame(false),
		NextReceiverGetsSplitFrames(order == FirstLinkGetsSplitFrames)
{
	Frames[0] = frame;
}
G42ImageAnimatedSplitter::G42ImageAnimatedSplitter(uint32 * frames,
	uint32 num_frames, G42LinkOrder order)
:
	NumFrames(num_frames), SplitReceiver(0), CurrentFrame(0), FirstFrame(true),
	SplitFrame(false),
		NextReceiverGetsSplitFrames(order == FirstLinkGetsSplitFrames)
{
	for (uint32 i = 0; i < num_frames; i++)
	{
		Frames[i] = frames[i];
	}
}
G42ImageAnimatedSplitter::G42ImageAnimatedSplitter(
	const G42Vector <uint32> & frames, uint32 num_frames, G42LinkOrder order)
:
	NumFrames(num_frames), SplitReceiver(0), CurrentFrame(0), FirstFrame(true),
	SplitFrame(false),
		NextReceiverGetsSplitFrames(order == FirstLinkGetsSplitFrames)
{
	for (uint32 i = 0; i < num_frames; i++)
	{
		Frames[i] = frames[i];
	}
}
G42ImageAnimatedSplitter::~G42ImageAnimatedSplitter(void)
{
	if (SplitReceiver)
	{
		SplitReceiver->SenderDestructing(this);
		SplitReceiver = 0;
	}
}
G42ImageFilter *
G42ImageAnimatedSplitter::MakeCopy(void) const
{
	G42ImageAnimatedSplitter * filter =
		new G42ImageAnimatedSplitter(Frames, NumFrames);
	G42ThrowIfNull(filter);
	return filter;
}
void
G42ImageAnimatedSplitter::ClearPath(bool path_excepted)
{
	if (SplitReceiver)
	{
		SplitReceiver->SenderDestructing(this);
		SplitReceiver = 0;
	}
	G42ImageFilter::ClearPath(path_excepted);
}
void
G42ImageAnimatedSplitter::HandleBegin(G42PathBegin * begin, G42PathComponent *)
{
	if (Receiver)
	{
		SplitReceiver->Process(begin, this);
	}
	else
	{
		delete begin;
	}
	if (SplitReceiver)
	{
		G42PathBegin * begin2 = new G42PathBegin;
		G42ThrowIfNull(begin2);
		SplitReceiver->Process(begin2, this);
	}
	CurrentFrame = 0;
	FirstFrame = true;
}
void
G42ImageAnimatedSplitter::HandleDescriptor(G42ImageDescriptor * descriptor,
	G42PathComponent *)
{
	if (FirstFrame)
	{
		FirstFrame = false;
	}
	else
	{
		CurrentFrame++;
	}
	SplitFrame = false;
	for (uint32 i = 0; i < NumFrames; i++)
	{
		if (Frames[i] == CurrentFrame)
		{
			SplitFrame = true;
		}
	}
	if (SplitFrame && SplitReceiver)
	{
		SplitReceiver->Process(descriptor, this);
	}
	else if (!SplitFrame && Receiver)
	{
		Receiver->Process(descriptor, this);
	}
	else
	{
		delete descriptor;
	}
}
void
G42ImageAnimatedSplitter::HandleData(G42ImageData * data, G42PathComponent *)
{
	if (SplitFrame && SplitReceiver)
	{
		SplitReceiver->Process(data, this);
	}
	else if (!SplitFrame && Receiver)
	{
		Receiver->Process(data, this);
	}
	else
	{
		delete data;
	}
}
void
G42ImageAnimatedSplitter::HandleEnd(G42PathEnd * end, G42PathComponent *)
{
	if (Receiver)
	{
		SplitReceiver->Process(end, this);
		Receiver = 0;
	}
	else
	{
		delete end;
	}
	if (SplitReceiver)
	{
		G42PathEnd * end2 = new G42PathEnd;
		G42ThrowIfNull(end2);
		SplitReceiver->Process(end2, this);
		SplitReceiver = 0;
	}
}
void
G42ImageAnimatedSplitter::HandleUnknown(G42PathObject * object,
	G42PathComponent *)
{
	if (dynamic_cast<G42MultipleImageDescriptor *>(object))
	{
		G42MultipleImageDescriptor * desc =
			dynamic_cast<G42MultipleImageDescriptor *>(object);
		if (desc->GetNumImages() >= NumFrames)
		{
			desc->SetNumImages(desc->GetNumImages() - NumFrames);
		}
		if (SplitReceiver)
		{
			G42MultipleImageDescriptor * desc2 = desc->MakeCopy();
			desc2->SetNumImages(NumFrames);
			SplitReceiver->Process(desc2, this);
		}
		if (Receiver)
		{
			Receiver->Process(desc, this);
		}
		else
		{
			delete desc;
		}
		SplitFrame = false;
		FirstFrame = true;
		CurrentFrame = 0;
		return;
	}
	if (SplitFrame && SplitReceiver)
	{
		SplitReceiver->Process(object, this);
	}
	else if (!SplitFrame && Receiver)
	{
		Receiver->Process(object, this);
	}
	else
	{
		delete object;
	}
}
void
G42ImageAnimatedSplitter::ReceiverDestructing(G42PathComponent * receiver)
{
	if (receiver == SplitReceiver)
	{
		SplitReceiver = 0;
	}
	else if (receiver == Receiver)
	{
		Receiver = 0;
	}
}
void
G42ImageAnimatedSplitter::ReplaceReceiver(G42PathComponent * old_receiver,
	G42PathComponent * new_receiver)
{
	if (old_receiver == SplitReceiver)
	{
		SplitReceiver = new_receiver;
	}
	else if (old_receiver == Receiver)
	{
		Receiver = new_receiver;
	}
}
void
G42ImageAnimatedSplitter::SetReceiver(G42PathComponent * receiver)
{
	if (!receiver || receiver == this)
	{
		throw G42InvalidParameterException();
	}
	if (NextReceiverGetsSplitFrames)
	{
		if (Receiver)
		{
			throw G42PathMultipleReceiverException();
		}
		Receiver = receiver;
		NextReceiverGetsSplitFrames = false;
	}
	else
	{
		if (SplitReceiver)
		{
			throw G42PathMultipleReceiverException();
		}
		SplitReceiver = receiver;
		NextReceiverGetsSplitFrames = true;
	}
}

#include "gifmults.h"
void
G42ImageFilterMultiSender::CleanupStrips(void)
{
// Free InputData ?
	#if 0 
	for (uint32 i = 0; i < OldInfo->GetNumStrips(); i++)
	{
		if (BottomData[i])
		{
			Receiver->Process(BottomData[i].Release(), this);
		}
	}
	if (CurData)
	{
		CurLock = 0;
		Receiver->Process(CurData.Release());
	}
	#endif
}
void
G42ImageFilterMultiSender::ClearPath (bool path_excepted)
{
	G42PathComponent::ClearPath (path_excepted);
	if (SecondSender)
	{
		SecondSender -> ReceiverDestructing (this);
	}
	SecondSender = 0;
}
void
G42ImageFilterMultiSender::HandleData (G42ImageData * strip, G42PathComponent * sender)
{
	if (sender == Sender) 
	{
		SetInputData (strip);
	}
	else if (sender == SecondSender) 
	{
		SecondImageData [strip -> GetStripNum()] = strip;
	}	
	else
	{
		throw (G42Exception(0));  // TODO:  What exception is this?
	}
}
void
G42ImageFilterMultiSender::HandleDescriptor (G42ImageDescriptor * desc, G42PathComponent * sender)
{
	if (sender == Sender) 
	{
		SetInputDescriptor (desc);
	}
	else if (sender == SecondSender) 
	{
//		SecondDescriptor = new G42ImageDescriptor (*desc);
		SecondImageDescriptor = desc;
	}	
	else
	{
		throw (G42Exception(0));  // TODO:  What exception is this?
	}
}
void
G42ImageFilterMultiSender::ReplaceSender(G42PathComponent * old_sender,
	G42PathComponent * new_sender)
{
	if (!new_sender)
	{
		throw G42InvalidParameterException();
	}
	if (old_sender == Sender)
	{
		Sender = new_sender;
	}
	else if (old_sender == SecondSender)
	{
		SecondSender = new_sender;
	}
	else
	{
		throw G42InvalidParameterException();
	}
}
void
G42ImageFilterMultiSender::SenderDestructing(G42PathComponent * sender)
{
	if (sender == Sender)
	{
		Sender = 0;
	}
	else if (sender == SecondSender)
	{
		SecondSender = 0;
	}
}
void
G42ImageFilterMultiSender::SetSender(G42PathComponent * sender)
{
	if (!sender || sender == this)
	{
		throw G42InvalidParameterException();
	}
	if (! HaveFirstSender) 
	{
		Sender = sender;
		HaveFirstSender = true;
	}
	else
	{	
		if (sender == Sender) {
			throw G42PathMultipleSenderException();
		}
		SecondSender = sender;
	}		
}

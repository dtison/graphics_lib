// giccomp.cpp - color comparer
#include "giccomp.h"
G42ImageColorComparer::G42ImageColorComparer()
:
	Sender2(0), NumEnds(0)
{
}
G42ImageColorComparer::~G42ImageColorComparer()
{
	if (Sender2)
	{
		Sender2->ReceiverDestructing(this);
		Sender2 = 0;
	}
}
void
G42ImageColorComparer::Process(G42PathObject * object, G42PathComponent * sender)
{
	if (dynamic_cast<G42PathBegin *>(object))
	{
		delete object;
	}
	else if (dynamic_cast<G42ImageDescriptor *>(object))
	{
		G42ImageDescriptor * desc = dynamic_cast<G42ImageDescriptor *>(object);
		if (sender == Sender)
		{
			Info1 = desc;
		}
		else
		{
			Info2 = desc;
		}
	}
	else if (dynamic_cast<G42ImageData *>(object))
	{
		G42ImageData * data = dynamic_cast<G42ImageData *>(object);
		if (sender == Sender)
		{
			Data1[data->GetStripNum()] = data;
		}
		else
		{
			Data2[data->GetStripNum()] = data;
		}
	}
	else if (dynamic_cast<G42PathEnd *>(object))
	{
		delete object;
		NumEnds++;
		if (NumEnds < 2)
		{
			if (sender == Sender)
			{
				Sender = 0;
			}
			else
			{
				Sender2 = 0;
			}
			return;
		}
		Sender = 0;
		Sender2 = 0;
		Info3 = new G42ImageDescriptor(Info1->GetWidth(), Info1->GetHeight(),
			G42ImageDescriptor::Gray8, Data1[0]->GetHeap());
		G42PathBegin * begin = new G42PathBegin();
		Receiver->Process(begin, this);
		G42ImageDescriptor * desc = new G42ImageDescriptor(*Info3);
		Receiver->Process(desc, this);
		G42RGBComparer comparer(Result);
		for (uint32 i = 0; i < Info3->GetNumStrips(); i++)
		{
			G42Ptr<G42ImageDataGray8> data;
			data = dynamic_cast<G42ImageDataGray8 *>(
				Info3->CreateData(i));
			G42Ptr<G42WriteImageData> lock(new G42WriteImageData(*data));
			G42CompareImages1(data->GetBegin(*lock), data->GetStripStartInImage(),
				data->GetNumRows(), (G42ImageDescriptor *)Info1, Data1,
				(G42ImageDescriptor *)Info2, Data2, comparer);
			lock = 0;
			Receiver->Process(data.Release(), this);
		}
		G42PathEnd * end = new G42PathEnd();
		Receiver->Process(end, this);
		Receiver = 0;
#ifdef MSWIN
		char buf[111];
		sprintf(buf, "Results of image compare are %f", Result);
		::MessageBox(0, buf, "giccomp.cpp", MB_OK);
#endif
	}
	else
	{
		delete object;
	}
}
void
G42ImageColorComparer::ClearPath(void)
{
	if (Sender2)
	{
		Sender2->ReceiverDestructing(this);
		Sender2 = 0;
	}
	G42PathComponent::ClearPath();
	Result = 0;
}
void
G42ImageColorComparer::ReplaceSender(G42PathComponent * old_sender,
	G42PathComponent * new_sender)
{
	if (old_sender == Sender2)
	{
		Sender2 = new_sender;
	}
	else
	{
		G42PathComponent::ReplaceSender(old_sender, new_sender);
	}
}
void
G42ImageColorComparer::SenderDestructing(G42PathComponent * sender)
{
	if (sender == Sender2)
	{
		Sender2 = 0;
	}
	else
	{
		G42PathComponent::SenderDestructing(sender);
	}
}
void
G42ImageColorComparer::SetSender(G42PathComponent * sender)
{
	if (!Sender2)
	{
		Sender2 = sender;
	}
	else
	{
		G42PathComponent::SetSender(sender);
	}
}

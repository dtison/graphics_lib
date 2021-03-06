// gifilter.cpp - base class for filters
/* TODO's: 
1. Should transparency converters be in G42ImageDataBase, not Gray1 ??
2. Sharpen doesn't have transparency yet
4. Probably don't need PC (G42Color *) 
5. On Equalize filter does gray1 need to promote to gray8?
*/
#include "gifilter.h"
#include "gidesc.h"
#include "gidata.h"
/*  Cheater declarations for template fn's (Makes calling much cleaner)  */
G42ImageDataColor8 * G42ImageFilter::DC8 = 0;
G42ImageDataColor24 * G42ImageFilter::DC24 = 0;
G42ImageDataColor32 * G42ImageFilter::DC32 = 0;
G42ImageDataGray8 * G42ImageFilter::DG8 = 0;
G42ImageDataGray1 * G42ImageFilter::DG1 = 0;
/*  Transparency  */
G42ImageDataBase * G42ImageFilter::DT1 = 0;
G42ImageDataSafeBase * G42ImageFilter::DTS1 = 0;
/* Iterators [Write-lock] */
G42ImageDataColor8::G42Iterator::G42Row::G42Iterator * G42ImageFilter::IC8 = 0;
G42ImageDataColor24::G42Iterator::G42Row::G42Iterator * G42ImageFilter::IC24 = 0;
G42ImageDataColor32::G42Iterator::G42Row::G42Iterator * G42ImageFilter::IC32 = 0;
G42ImageDataGray8::G42Iterator::G42Row::G42Iterator * G42ImageFilter::IG8 = 0;
G42ImageDataGray1::G42Iterator::G42Row::G42Iterator * G42ImageFilter::IG1 = 0;
G42ImageDataBase::G42TransIterator::G42Row::G42Iterator * G42ImageFilter::IT1 = 0;
G42ImageDataSafeBase::G42SafeTransIterator::G42Row::G42Iterator * G42ImageFilter::ITS1 = 0;
//123456789012345678901234567890123456789012345678901234567890123456789012345678
/*  Read-lock  */
G42ImageDataColor8::G42ConstIterator::G42Row::G42Iterator * G42ImageFilter::IC8R = 0;
G42ImageDataColor24::G42ConstIterator::G42Row::G42Iterator * G42ImageFilter::IC24R = 0;
G42ImageDataColor32::G42ConstIterator::G42Row::G42Iterator * G42ImageFilter::IC32R = 0;
G42ImageDataGray8::G42ConstIterator::G42Row::G42Iterator * G42ImageFilter::IG8R = 0;
G42ImageDataGray1::G42ConstIterator::G42Row::G42Iterator * G42ImageFilter::IG1R = 0;
G42ImageDataBase::G42ConstTransIterator::G42Row::G42Iterator * G42ImageFilter::IT1R = 0;
G42ImageDataSafeBase::G42ConstSafeTransIterator::G42Row::G42Iterator * G42ImageFilter::ITS1R = 0;
/*  Pixels  */
byte			* G42ImageFilter::PB8  = 0;
G42Color		* G42ImageFilter::PC   = 0;
G42Color24	* G42ImageFilter::PC24 = 0;
G42Color32	* G42ImageFilter::PC32 = 0;
G42ConstColor24	* G42ImageFilter::PC24R = 0;
G42ConstColor32	* G42ImageFilter::PC32R = 0;
G42ImageFilter::G42ImageFilter(void) : HaveFirstDescriptor (false)
{}
G42ImageFilter::~G42ImageFilter(void)
{
	CurrentOutputLock = 0; // better safe then sorry, clear lock just in case
}
void
G42ImageFilter::ClearDataAndDescriptor (bool clear_output)
{
	InputData.Clear(); 
	InputDescriptor.Clear();
	if (clear_output)
	{
		OutputData.Clear(); 
		OutputDescriptor.Clear();	
	}
}
void
G42ImageFilter::Process(G42PathObject * object, G42PathComponent * sender)
{
	if (dynamic_cast<G42PathBegin *>(object))
	{
		G42PathBegin * begin = dynamic_cast<G42PathBegin *>(object);
		HandleBegin(begin, sender);
	}
	else if (dynamic_cast<G42ImageDescriptor *>(object))
	{
		G42ImageDescriptor * desc = dynamic_cast<G42ImageDescriptor *>(object);
		HandleDescriptor(desc, sender);
	}
	else if (dynamic_cast<G42ImageData *>(object))
	{
		G42ImageData * data = dynamic_cast<G42ImageData *>(object);
		HandleData(data, sender);
	}
	else if (dynamic_cast<G42PathEnd *>(object))
	{
		G42PathEnd * end = dynamic_cast<G42PathEnd *>(object);
		HandleEnd(end, sender);
	}
	else if (dynamic_cast<G42MultipleImageDescriptor *>(object))
	{
		G42MultipleImageDescriptor * desc = 
			dynamic_cast<G42MultipleImageDescriptor *>(object);
		HandleMultiDescriptor(desc, sender);
	}
	else
	{
		HandleUnknown(object, sender);
	}
}
void
G42ImageFilter::ClearPath(bool path_excepted)
{
	G42PathComponent::ClearPath(path_excepted);
	InputDescriptor = 0;
	InputData.Clear();
	OutputDescriptor = 0;
	CurrentOutputLock = 0;
	CurrentOutputData = 0;
}
void
G42ImageFilter::CopyTransparency (void)
{
	CopyTransparency (InputDescriptor, OutputDescriptor);
}
#ifdef OLDWAY
void
G42ImageFilter::CopyTransparency (void)
{
	if (!InputDescriptor || !OutputDescriptor ||
		!InputDescriptor->IsTransparent() || !OutputDescriptor->IsTransparent())
	{
		return;
	}
	if (InputDescriptor->GetWidth() != OutputDescriptor->GetWidth() ||
		InputDescriptor->GetHeight() != OutputDescriptor->GetHeight())
	{
		return;
	}
	for (uint32 i = 0; i < InputDescriptor->GetHeight();)
	{
		uint32 input_strip = InputDescriptor->GetStripNum(i);
		uint32 input_row = InputDescriptor->GetRowNumInStrip(i);
		uint32 input_num_rows = InputDescriptor->GetRowsInStrip(i) - input_row;
		uint32 output_strip = OutputDescriptor->GetStripNum(i);
		uint32 output_row = OutputDescriptor->GetRowNumInStrip(i);
		uint32 output_num_rows = OutputDescriptor->GetRowsInStrip(i) - output_row;
		uint32 num_rows = min(input_num_rows, output_num_rows);
		G42ImageData * input_data = InputData[input_strip];
		G42ImageData * output_data = OutputData[output_strip];
		if (!input_data || !output_data)
		{
			i += num_rows;
			continue;
		}
		G42ReadImageData input_lock(*input_data);
		G42WriteImageData output_lock(*output_data);
		for (uint32 j = 0; j < num_rows; j++)
		{
			memcpy(output_lock.GetRow(output_row++, true),
				input_lock.GetRow(input_row++, true),
				InputDescriptor->GetTransRowBytes());
			i++;
		}
	}
}
#endif
void
G42ImageFilter::CopyTransparency (DescriptorRef input_descriptor, 
	DescriptorRef output_descriptor)
{
	if (!input_descriptor || !output_descriptor ||
		!input_descriptor->IsTransparent() || !output_descriptor->IsTransparent())
	{
		return;
	}
	if (input_descriptor->GetWidth() != output_descriptor->GetWidth() ||
		input_descriptor->GetHeight() != output_descriptor->GetHeight())
	{
		return;
	}
	for (uint32 i = 0; i < input_descriptor->GetHeight();)
	{
		uint32 input_strip = input_descriptor->GetStripNum(i);
		uint32 input_row = input_descriptor->GetRowNumInStrip(i);
		uint32 input_num_rows = input_descriptor->GetRowsInStrip(input_strip) - input_row;
		uint32 output_strip = output_descriptor->GetStripNum(i);
		uint32 output_row = output_descriptor->GetRowNumInStrip(i);
		uint32 output_num_rows = output_descriptor->GetRowsInStrip(output_strip) - output_row;
		uint32 num_rows = min(input_num_rows, output_num_rows);
		G42ImageData * input_data = InputData[input_strip];
		G42ImageData * output_data = OutputData[output_strip];
		if (!input_data || !output_data)
		{
			i += num_rows;
			continue;
		}
		G42ReadImageData input_lock(*input_data);
		G42WriteImageData output_lock(*output_data);
		for (uint32 j = 0; j < num_rows; j++)
		{
			memcpy(output_lock.GetRow(output_row++, true),
				input_lock.GetRow(input_row++, true),
				input_descriptor->GetTransRowBytes());
			i++;
		}
	}
}
void
G42ImageFilter::CreateOutputData (void)
{
	for (uint32 i = 0; i < OutputDescriptor -> GetNumStrips(); i++)
	{
		OutputData [i] = OutputDescriptor -> CreateData (i);
	}
}
void
G42ImageFilter::HandleDescriptor(G42ImageDescriptor * descriptor, 
	G42PathComponent *)
{
	if (HaveFirstDescriptor) 
	{
		/*  It's time to run the filter on the frame */
		ProcessFrame();
	}
	else 
	{
		HaveFirstDescriptor = true;
	}
	SetInputDescriptor (descriptor);
}
void
G42ImageFilter::HandleData(G42ImageData * data, G42PathComponent *)
{
	#ifdef OLDWAY
	Receiver->Process(data, this);
	#endif
	SetInputData (data);
}
void
G42ImageFilter::HandleBegin(G42PathBegin * begin, G42PathComponent *)
{
	if (!Receiver)
	{
		throw G42PathNoReceiverException();
	}
	Receiver->Process(begin, this);
}
void
G42ImageFilter::HandleMultiDescriptor(G42MultipleImageDescriptor * descriptor, 
	G42PathComponent *)
{
	Receiver->Process(descriptor, this);
}
void
G42ImageFilter::HandleEnd(G42PathEnd * end, G42PathComponent *)
{
	ProcessFrame();
	Receiver->Process(end, this);
	/*  Reset sender and receiver  */
	Receiver = 0;
	Sender = 0;
	ClearPath();
}
void
G42ImageFilter::HandleUnknown(G42PathObject * object, G42PathComponent *)
{
//	Receiver->Process(object, this);
}
void
G42ImageFilter::SetInputDescriptor(G42ImageDescriptor * desc, bool make_copy)
{
	if (make_copy)
	{
		InputDescriptor = new G42ImageDescriptor(*desc);
	}
	else
	{
		InputDescriptor = desc;
	}
}
void
G42ImageFilter::SetInputData(G42ImageData * data, bool make_copy)
{
	if (make_copy)
	{
		InputData[data->GetStripNum()] = data->MakeCopy();
	}
	else
	{
		InputData[data->GetStripNum()] = data;
	}
}
void
G42ImageFilter::SetOutputDescriptor(G42ImageDescriptor * desc, bool make_copy)
{
	if (make_copy)
	{
		OutputDescriptor = new G42ImageDescriptor(*desc);
	}
	else
	{
		OutputDescriptor = desc;
	}
}
void
G42ImageFilter::GetCurrOutputData(uint32 strip_index)
{
	if (CurrentOutputData && CurrentOutputData->GetStripNum() == strip_index)
	{
		return;
	}
	SendCurrOutputData();
	CurrentOutputData = OutputDescriptor->CreateData(strip_index);
	G42ThrowIfNull(CurrentOutputData);
	CurrentOutputLock = new G42WriteImageData(*CurrentOutputData);
	G42ThrowIfNull(CurrentOutputLock);
}
void
G42ImageFilter::SendCurrOutputData(void)
{
	if (CurrentOutputData)
	{
		CurrentOutputLock = 0;
		Receiver->Process(CurrentOutputData.Release());
	}
}
void
G42ImageFilter::SendOutputData (void)
{
	for (uint32 i = 0; i < OutputDescriptor -> GetNumStrips(); i++)
	{
		Receiver -> Process (OutputData [i].Release(), this);
	}	
}
/*  Plain C++ function for templates to use */
G42Color Interpolate4Colors (G42Color * colors, double row_delta, 
	double col_delta, bool single_channel)
{
	byte red = 0;
	byte blue = 0;
	/*  Interpolate green first so we can optimize for single channel */
	double part1_and2 = (col_delta * 
		((double) (colors [1].Green - colors [0].Green))) + 
		(double) colors [0].Green;
	double part3_and4 = (col_delta * 
		((double) (colors [3].Green - colors [2].Green))) + 
		(double) colors [2].Green;
	byte green = (byte) (0.5 + part1_and2 + 
		(part3_and4 - part1_and2) * row_delta);
	if (single_channel) {
		return G42Color (red, green, blue);
	}
	/*  Find interpolated red  */
	part1_and2 = (col_delta * 
		((double) (colors [1].Red - colors [0].Red))) + 
		(double) colors [0].Red;
	part3_and4 = (col_delta * 
		((double) (colors [3].Red - colors [2].Red))) + 
		(double) colors [2].Red;
	red = (byte)	(0.5 + part1_and2 + 
		(part3_and4 - part1_and2) * row_delta);
	/*  Find interpolated blue  */
	part1_and2 = (col_delta * 
		((double) (colors [1].Blue - colors [0].Blue))) + 
		(double) colors [0].Blue;
	part3_and4 = (col_delta * 
		((double) (colors [3].Blue - colors [2].Blue))) + 
		(double) colors [2].Blue;
	blue = (byte) (0.5 + part1_and2 + 
		(part3_and4 - part1_and2) * row_delta);
	return G42Color (red, green, blue);
}

// gidther.cpp
#include "gidither.h"
G42ColorReduce::G42ColorReduce(const G42OptimizedPalette * opt_pal,
	bool match)
:
	OptPal(opt_pal),
	CurrentStripNum(0), CurrentData(0), OutputRowsLeft(0), Match(match)
{
}
G42ColorReduce::~G42ColorReduce()
{
}
void
G42ColorReduce::Reset(void)
{
	RedError.Clear();
	GreenError.Clear();
	BlueError.Clear();
	OldDescriptor.Clear();
	NewDescriptor.Clear();
	OutputRowsLeft = 0;
	CurrentStripNum = 0;
	DitherGray1.Clear();
	DitherGray8.Clear();
	DitherColor8.Clear();
	DitherColor24.Clear();
	DitherColor32.Clear();
	CurrentDataLock.Clear();
	CurrentData.Clear();
}
void
G42ColorReduce::Process(G42PathObject * object, G42PathComponent *)
{
	if (typeid(*object) == typeid(G42ImageDescriptor))
	{
		OldDescriptor = dynamic_cast<G42ImageDescriptor *>(object);
		CreateConverter();
		object = 0;
	}
	else if (dynamic_cast<G42ImageData *>(object))
	{
		G42ImageData * strip = dynamic_cast<G42ImageData *>(object);
		object = 0;
		if (!NewDescriptor)
		{
			NewDescriptor = new G42ImageDescriptor(OldDescriptor->GetWidth(),
				OldDescriptor->GetHeight(), G42ImageDescriptor::Color8,
				strip->GetHeap(), OldDescriptor->IsTransparent());
			NewDescriptor->SetNumPalette((uint16)OptPal->GetNumPalette());
			G42Color * pal = NewDescriptor->GetPalette();
			const G42Color * opal = OptPal->GetPalette();
			for (intf i = 0; i < OptPal->GetNumPalette(); i++)
			{
				pal[i] = opal[i];
			}
			if (OldDescriptor->IsTransparent())
			{
				NewDescriptor->SetTransparency();
				NewDescriptor->SetTransparentColor(
					OldDescriptor->GetTransparentColor());
				// we really need to take care of this problem one day
				NewDescriptor->SetTransparentPixel(-1);
			}
			// need to take care of first and last strip if
			// old first strip is not full
			G42PathObject * desc = new G42ImageDescriptor(*NewDescriptor);
			Receiver->Process(desc, this);
		}
		G42Ptr<G42ReadImageData> rdata(new G42ReadImageData(*strip));
		uint32 input_rows_left = strip->GetNumRows();
		if (dynamic_cast<G42ImageDataColor32 *>(strip))
		{
			G42ImageDataColor32 * stripc32 =
				dynamic_cast<G42ImageDataColor32 *>(strip);
			G42ImageDataColor32::G42ConstIterator current_input_iterator =
				stripc32->GetConstBegin(rdata.GetReference());
			G42ImageDataBase::G42ConstTransIterator current_trans_iterator;
			if (NewDescriptor->IsTransparent())
			{
				G42ImageDataBase * stript =
					dynamic_cast<G42ImageDataBase *>(strip);
				current_trans_iterator = stript->GetConstBegin(*rdata);
			}
			while (input_rows_left)
			{
				if (!OutputRowsLeft)
				{
					ResetOutput(strip);
				}
				uint32 num_rows = min(input_rows_left, OutputRowsLeft);
				if (Match)
				{
					CurrentOutputIterator = G42ImageDataConvertCopy(
						current_input_iterator, current_input_iterator + num_rows,
						CurrentOutputIterator, *MatchColor32);
				}
				else
				{
					CurrentOutputIterator = G42ImageDataRowConvertCopy(
						current_input_iterator, current_input_iterator + num_rows,
						CurrentOutputIterator, *DitherColor32);
				}
				OutputRowsLeft -= num_rows;
				input_rows_left -= num_rows;
				current_input_iterator += num_rows;
				if (NewDescriptor->IsTransparent())
				{
					G42ImageDataBase::G42ConstTransIterator tend =
						current_trans_iterator + num_rows;
					while (current_trans_iterator != tend)
					{
						memcpy((*OutputTransIterator).GetPtr(),
							(*current_trans_iterator).GetPtr(),
							NewDescriptor->GetTransRowBytes());
						current_trans_iterator++;
						OutputTransIterator++;
					}
				}
			}
		} // color32
		else if (dynamic_cast<G42ImageDataColor24 *>(strip))
		{
			G42ImageDataColor24 * stripc24 =
				dynamic_cast<G42ImageDataColor24 *>(strip);
			G42ImageDataColor24::G42ConstIterator current_input_iterator =
				stripc24->GetConstBegin(rdata.GetReference());
			G42ImageDataBase::G42ConstTransIterator current_trans_iterator;
			if (NewDescriptor->IsTransparent())
			{
				G42ImageDataBase * stript =
					dynamic_cast<G42ImageDataBase *>(strip);
				current_trans_iterator = stript->GetConstBegin(*rdata);
			}
			while (input_rows_left)
			{
				if (!OutputRowsLeft)
				{
					ResetOutput(strip);
				}
				uint32 num_rows = min(input_rows_left, OutputRowsLeft);
				if (Match)
				{
					CurrentOutputIterator = G42ImageDataConvertCopy(
						current_input_iterator, current_input_iterator + num_rows,
						CurrentOutputIterator, *MatchColor24);
				}
				else
				{
					CurrentOutputIterator = G42ImageDataRowConvertCopy(
						current_input_iterator, current_input_iterator + num_rows,
						CurrentOutputIterator, *DitherColor24);
				}
				OutputRowsLeft -= num_rows;
				input_rows_left -= num_rows;
				current_input_iterator += num_rows;
				if (NewDescriptor->IsTransparent())
				{
					G42ImageDataBase::G42ConstTransIterator tend =
						current_trans_iterator + num_rows;
					while (current_trans_iterator != tend)
					{
						memcpy((*OutputTransIterator).GetPtr(),
							(*current_trans_iterator).GetPtr(),
							NewDescriptor->GetTransRowBytes());
						current_trans_iterator++;
						OutputTransIterator++;
					}
				}
			}
		} // color24
		else if (dynamic_cast<G42ImageDataColor8 *>(strip))
		{
			G42ImageDataColor8 * stripc8 =
				dynamic_cast<G42ImageDataColor8 *>(strip);
			G42ImageDataColor8::G42ConstIterator current_input_iterator =
				stripc8->GetConstBegin(rdata.GetReference());
			G42ImageDataBase::G42ConstTransIterator current_trans_iterator;
			if (NewDescriptor->IsTransparent())
			{
				G42ImageDataBase * stript =
					dynamic_cast<G42ImageDataBase *>(strip);
				current_trans_iterator = stript->GetConstBegin(*rdata);
			}
			while (input_rows_left)
			{
				if (!OutputRowsLeft)
				{
					ResetOutput(strip);
				}
				uint32 num_rows = min(input_rows_left, OutputRowsLeft);
				if (Match)
				{
					CurrentOutputIterator = G42ImageDataConvertCopy(
						current_input_iterator, current_input_iterator + num_rows,
						CurrentOutputIterator, *MatchColor8);
				}
				else
				{
					CurrentOutputIterator = G42ImageDataRowConvertCopy(
						current_input_iterator, current_input_iterator + num_rows,
						CurrentOutputIterator, *DitherColor8);
				}
				OutputRowsLeft -= num_rows;
				input_rows_left -= num_rows;
				current_input_iterator += num_rows;
				if (NewDescriptor->IsTransparent())
				{
					G42ImageDataBase::G42ConstTransIterator tend =
						current_trans_iterator + num_rows;
					while (current_trans_iterator != tend)
					{
						memcpy((*OutputTransIterator).GetPtr(),
							(*current_trans_iterator).GetPtr(),
							NewDescriptor->GetTransRowBytes());
						current_trans_iterator++;
						OutputTransIterator++;
					}
				}
			}
		} // color8
		else if (dynamic_cast<G42ImageDataGray8 *>(strip))
		{
			G42ImageDataGray8 * stripg8 =
				dynamic_cast<G42ImageDataGray8 *>(strip);
			G42ImageDataGray8::G42ConstIterator current_input_iterator =
				stripg8->GetConstBegin(rdata.GetReference());
			G42ImageDataBase::G42ConstTransIterator current_trans_iterator;
			if (NewDescriptor->IsTransparent())
			{
				G42ImageDataBase * stript =
					dynamic_cast<G42ImageDataBase *>(strip);
				current_trans_iterator = stript->GetConstBegin(*rdata);
			}
			while (input_rows_left)
			{
				if (!OutputRowsLeft)
				{
					ResetOutput(strip);
				}
				uint32 num_rows = min(input_rows_left, OutputRowsLeft);
				if (Match)
				{
					CurrentOutputIterator = G42ImageDataConvertCopy(
						current_input_iterator, current_input_iterator + num_rows,
						CurrentOutputIterator, *MatchGray8);
				}
				else
				{
					CurrentOutputIterator = G42ImageDataRowConvertCopy(
						current_input_iterator, current_input_iterator + num_rows,
						CurrentOutputIterator, *DitherGray8);
				}
				OutputRowsLeft -= num_rows;
				input_rows_left -= num_rows;
				current_input_iterator += num_rows;
				if (NewDescriptor->IsTransparent())
				{
					G42ImageDataBase::G42ConstTransIterator tend =
						current_trans_iterator + num_rows;
					while (current_trans_iterator != tend)
					{
						memcpy((*OutputTransIterator).GetPtr(),
							(*current_trans_iterator).GetPtr(),
							NewDescriptor->GetTransRowBytes());
						current_trans_iterator++;
						OutputTransIterator++;
					}
				}
			}
		} // gray8
		else if (dynamic_cast<G42ImageDataGray1 *>(strip))
		{
			G42ImageDataGray1 * stripg1 =
				dynamic_cast<G42ImageDataGray1 *>(strip);
			G42ImageDataGray1::G42ConstIterator current_input_iterator =
				stripg1->GetConstBegin(rdata.GetReference());
			G42ImageDataBase::G42ConstTransIterator current_trans_iterator;
			if (NewDescriptor->IsTransparent())
			{
				G42ImageDataBase * stript =
					dynamic_cast<G42ImageDataBase *>(strip);
				current_trans_iterator = stript->GetConstBegin(*rdata);
			}
			while (input_rows_left)
			{
				if (!OutputRowsLeft)
				{
					ResetOutput(strip);
				}
				uint32 num_rows = min(input_rows_left, OutputRowsLeft);
				if (Match)
				{
					CurrentOutputIterator = G42ImageDataConvertCopy(
						current_input_iterator, current_input_iterator + num_rows,
						CurrentOutputIterator, *MatchGray1);
				}
				else
				{
					CurrentOutputIterator = G42ImageDataRowConvertCopy(
						current_input_iterator, current_input_iterator + num_rows,
						CurrentOutputIterator, *DitherGray1);
				}
				OutputRowsLeft -= num_rows;
				input_rows_left -= num_rows;
				current_input_iterator += num_rows;
				if (NewDescriptor->IsTransparent())
				{
					G42ImageDataBase::G42ConstTransIterator tend =
						current_trans_iterator + num_rows;
					while (current_trans_iterator != tend)
					{
						memcpy((*OutputTransIterator).GetPtr(),
							(*current_trans_iterator).GetPtr(),
							NewDescriptor->GetTransRowBytes());
						current_trans_iterator++;
						OutputTransIterator++;
					}
				}
			}
		} // color32
		rdata.Clear();
		delete strip;
	} // image data
	if (object)
	{
		if (typeid(*object) == typeid(G42PathEnd))
		{
			if (CurrentData)
			{
				CurrentDataLock = 0;
				Receiver->Process(CurrentData.Release(), this);
			}
			Receiver->Process(object, this);
			Receiver = 0;
			Sender = 0;
			Reset();
		}
		else
		{
			Receiver->Process(object, this);
		}
	}
}
void
G42ColorReduce::ResetOutput(G42ImageData * strip)
{
	if (CurrentData)
	{
		CurrentDataLock = 0;
		Receiver->Process(CurrentData.Release(), this);
	}
	CurrentData = new G42ImageDataColor8(strip->GetHeap(),
		CurrentStripNum, NewDescriptor->GetRowsInStrip(CurrentStripNum),
		NewDescriptor->GetRowBytes(),
		NewDescriptor->GetStripStartInImage(CurrentStripNum),
		NewDescriptor->GetWidth(),
		NewDescriptor->IsTransparent() ? NewDescriptor->GetTransRowBytes() : 0);
	CurrentStripNum++;
	CurrentDataLock = new G42WriteImageData(*CurrentData);
	CurrentOutputIterator = CurrentData->GetBegin(*CurrentDataLock);
	OutputRowsLeft = CurrentData->GetNumRows();
	if (NewDescriptor->IsTransparent())
	{
		OutputTransIterator = CurrentData->G42ImageDataBase::GetBegin(*CurrentDataLock);
	}
}
void
G42ColorReduce::CreateConverter(void)
{
	if (Match)
	{
		switch (OldDescriptor->GetImageType())
		{
			case G42ImageDescriptor::Color32:
				MatchColor32 = new G42ColorMatchCopyConverter<
					G42ImageDataColor32::G42ConvertToG42Color, G42ConstColor32>
				(
					G42ImageDataColor32::G42ConvertToG42Color(),
					OptPal, (G42ConstColor32 *)0);
				break;
			case G42ImageDescriptor::Color24:
				MatchColor24 = new G42ColorMatchCopyConverter<
					G42ImageDataColor24::G42ConvertToG42Color, G42ConstColor24>
				(
					G42ImageDataColor24::G42ConvertToG42Color(),
					OptPal, (G42ConstColor24 *)0);
				break;
			case G42ImageDescriptor::Color8:
				MatchColor8 = new G42ColorMatchCopyConverter<
					G42ImageDataColor8::G42ConvertToG42Color, byte>
				(
					G42ImageDataColor8::G42ConvertToG42Color(
						OldDescriptor->GetPalette()),
					OptPal, (byte *)0);
				break;
			case G42ImageDescriptor::Gray8:
				MatchGray8 = new G42ColorMatchCopyConverter<
					G42ImageDataGray8::G42ConvertToG42Color, byte>
				(
					G42ImageDataGray8::G42ConvertToG42Color(),
					OptPal, (byte *)0);
				break;
			case G42ImageDescriptor::Gray1:
				MatchGray1 = new G42ColorMatchCopyConverter<
					G42ImageDataGray1::G42ConvertToG42Color, bool>
				(
					G42ImageDataGray1::G42ConvertToG42Color(),
					OptPal, (bool *)0);
				break;
		}
	}
	else
	{
		RedError = new G42OldDitherError(OldDescriptor->GetWidth());
		GreenError = new G42OldDitherError(OldDescriptor->GetWidth());
		BlueError = new G42OldDitherError(OldDescriptor->GetWidth());
		switch (OldDescriptor->GetImageType())
		{
			case G42ImageDescriptor::Color32:
				DitherColor32 = new G42ColorDitherCopyRowConverter<
					G42ImageDataColor32::G42ConvertToG42Color, G42ConstColor32>
				(
					G42ImageDataColor32::G42ConvertToG42Color(),
					OptPal, *RedError, *GreenError,
					*BlueError, (G42ConstColor32 *)0);
				break;
			case G42ImageDescriptor::Color24:
				DitherColor24 = new G42ColorDitherCopyRowConverter<
					G42ImageDataColor24::G42ConvertToG42Color, G42ConstColor24>
				(
					G42ImageDataColor24::G42ConvertToG42Color(),
					OptPal, *RedError, *GreenError,
					*BlueError, (G42ConstColor24 *)0);
				break;
			case G42ImageDescriptor::Color8:
				DitherColor8 = new G42ColorDitherCopyRowConverter<
					G42ImageDataColor8::G42ConvertToG42Color, byte>
				(
					G42ImageDataColor8::G42ConvertToG42Color(
						OldDescriptor->GetPalette()),
					OptPal, *RedError, *GreenError,
					*BlueError, (byte *)0);
				break;
			case G42ImageDescriptor::Gray8:
				DitherGray8 = new G42ColorDitherCopyRowConverter<
					G42ImageDataGray8::G42ConvertToG42Color, byte>
				(
					G42ImageDataGray8::G42ConvertToG42Color(),
					OptPal, *RedError, *GreenError,
					*BlueError, (byte *)0);
				break;
			case G42ImageDescriptor::Gray1:
				DitherGray1 = new G42ColorDitherCopyRowConverter<
					G42ImageDataGray1::G42ConvertToG42Color, bool>
				(
					G42ImageDataGray1::G42ConvertToG42Color(),
					OptPal, *RedError, *GreenError,
					*BlueError, (bool *)0);
				break;
		}
	}
}
G42ImageFilter *
G42ColorReduce::MakeCopy(void) const
{
	return new G42ColorReduce(OptPal);
}

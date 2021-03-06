// gicrop.cpp = G42ImageCrop
#include "gicrop.h"
G42ImageCrop::G42ImageCrop(const G42Rect & rect)
:
	RowBytes(0), Rect(rect)
{
	// check rect
	if (Rect.GetWidth() <= 0 || Rect.GetHeight() <= 0)
	{
		throw G42InvalidParameterException();
	}
}
G42ImageCrop::~G42ImageCrop()
{
}
void
G42ImageCrop::Process(G42PathObject * object, G42PathComponent *)
{
	if (dynamic_cast<G42ImageDescriptor *>(object))
	{
		OldInfo = dynamic_cast<G42ImageDescriptor *>(object);
		// clip cropping rect to image
		Rect &= G42Rect(0, 0, OldInfo->GetWidth() - 1, OldInfo->GetHeight() - 1);
		if (Rect.GetWidth() <= 0 || Rect.GetHeight() <= 0)
		{
			throw G42CropOutsideImageException();
		}
		object = 0;
	}
	else if (dynamic_cast<G42ImageData *>(object))
	{
		G42ImageData * strip = dynamic_cast<G42ImageData *>(object);
		if (!NewInfo)
		{
			NewInfo = new G42ImageDescriptor(*OldInfo, Rect.GetWidth(),
				Rect.GetHeight());
			G42ThrowIfNull(NewInfo);
			G42ImageDescriptor * desc = new G42ImageDescriptor(*NewInfo);
			G42ThrowIfNull(desc);
			Receiver->Process(desc, this);
		}
		G42Rect strip_rect(0, strip->GetStripStartInImage(),
			OldInfo->GetWidth() - 1,
			strip->GetNumRows() + strip->GetStripStartInImage() - 1);
		G42Rect old_strip_rect(strip_rect);
		old_strip_rect &= Rect;
		if (old_strip_rect.IsEmpty())
		{
			delete object;
			object = 0;
		}
		else if (old_strip_rect != strip_rect)
		{
			G42Rect new_strip_rect(old_strip_rect);
			new_strip_rect -= G42Size(Rect.GetLeft(), Rect.GetTop());
			uint32 start_strip = NewInfo->GetStripNum(new_strip_rect.GetTop());
			uint32 end_strip = NewInfo->GetStripNum(new_strip_rect.GetBottom());
			uint32 input_offset = 0;
			for (uint32 i_strip = start_strip; i_strip <= end_strip; i_strip++)
			{
				GetStrip(strip->GetHeap(), i_strip);
				uint32 start_offset = 0;
				if (i_strip == start_strip)
				{
					start_offset = NewInfo->GetRowNumInStrip(
						new_strip_rect.GetTop());
				}
				uint32 end_offset = CurData->GetNumRows();
				if (i_strip == end_strip)
				{
					end_offset = new_strip_rect.GetBottom() -
						CurData->GetStripStartInImage() + 1;
				}
				switch (NewInfo->GetImageType())
				{
					case G42ImageDescriptor::Gray1:
					{
						G42ImageDataGray1 * input_data =
							dynamic_cast<G42ImageDataGray1 *>(strip);
						G42ImageDataGray1 * output_data =
							dynamic_cast<G42ImageDataGray1 *>
							((G42ImageData *)CurData);
						G42ReadImageData lock(*strip);
						G42ImageDataGray1::G42ConstIterator begin =
							input_data->GetConstBegin(lock) + input_offset;
						uint32 height = end_offset - start_offset;
						G42ImageDataGray1::G42ConstIterator end = begin + height;
						G42ImageDataGray1::G42Iterator output =
							output_data->GetBegin(*CurLock) + start_offset;
						G42ImageCropStrip(begin, end, old_strip_rect.GetLeft(),
							output);
						input_offset += height;
						break;
					}
					case G42ImageDescriptor::Gray8:
					{
						G42ImageDataGray8 * input_data =
							dynamic_cast<G42ImageDataGray8 *>(strip);
						G42ImageDataGray8 * output_data =
							dynamic_cast<G42ImageDataGray8 *>
							((G42ImageData *)CurData);
						G42ReadImageData lock(*strip);
						G42ImageDataGray8::G42ConstIterator begin =
							input_data->GetConstBegin(lock) + input_offset;
						uint32 height = end_offset - start_offset;
						G42ImageDataGray8::G42ConstIterator end = begin + height;
						G42ImageDataGray8::G42Iterator output =
							output_data->GetBegin(*CurLock) + start_offset;
						G42ImageCropStrip(begin, end, old_strip_rect.GetLeft(),
							output);
						input_offset += height;
						break;
					}
					case G42ImageDescriptor::Color8:
					{
						G42ImageDataColor8 * input_data =
							dynamic_cast<G42ImageDataColor8 *>(strip);
						G42ImageDataColor8 * output_data =
							dynamic_cast<G42ImageDataColor8 *>
							((G42ImageData *)CurData);
						G42ReadImageData lock(*strip);
						G42ImageDataColor8::G42ConstIterator begin =
							input_data->GetConstBegin(lock) + input_offset;
						uint32 height = end_offset - start_offset;
						G42ImageDataColor8::G42ConstIterator end = begin + height;
						G42ImageDataColor8::G42Iterator output =
							output_data->GetBegin(*CurLock) + start_offset;
						G42ImageCropStrip(begin, end, old_strip_rect.GetLeft(),
							output);
						input_offset += height;
						break;
					}
					case G42ImageDescriptor::Color24:
					{
						G42ImageDataColor24 * input_data =
							dynamic_cast<G42ImageDataColor24 *>(strip);
						G42ImageDataColor24 * output_data =
							dynamic_cast<G42ImageDataColor24 *>
							((G42ImageData *)CurData);
						G42ReadImageData lock(*strip);
						G42ImageDataColor24::G42ConstIterator begin =
							input_data->GetConstBegin(lock) + input_offset;
						uint32 height = end_offset - start_offset;
						G42ImageDataColor24::G42ConstIterator end = begin + height;
						G42ImageDataColor24::G42Iterator output =
							output_data->GetBegin(*CurLock) + start_offset;
						G42ImageCropStrip(begin, end, old_strip_rect.GetLeft(),
							output);
						input_offset += height;
						break;
					}
					case G42ImageDescriptor::Color32:
					{
						G42ImageDataColor32 * input_data =
							dynamic_cast<G42ImageDataColor32 *>(strip);
						G42ImageDataColor32 * output_data =
							dynamic_cast<G42ImageDataColor32 *>
							((G42ImageData *)CurData);
						G42ReadImageData lock(*strip);
						G42ImageDataColor32::G42ConstIterator begin =
							input_data->GetConstBegin(lock) + input_offset;
						uint32 height = end_offset - start_offset;
						G42ImageDataColor32::G42ConstIterator end = begin + height;
						G42ImageDataColor32::G42Iterator output =
							output_data->GetBegin(*CurLock) + start_offset;
						G42ImageCropStrip(begin, end, old_strip_rect.GetLeft(),
							output);
						input_offset += height;
						break;
					}
				}
			}
			delete object;
			object = 0;
		}
		else
		{
			G42Rect new_strip_rect(old_strip_rect);
			new_strip_rect -= G42Size(Rect.GetLeft(), Rect.GetTop());
			uint32 start_strip = NewInfo->GetStripNum(new_strip_rect.GetTop());
			// copy the strip and reset the start and strip num
			G42ImageData * new_strip = strip->MakeCopy();
			G42ThrowIfNull(new_strip);
			// set data
			new_strip->SetInfo(start_strip, strip->GetRowBytes(),
				new_strip_rect.GetTop(), NewInfo->GetWidth(),
				strip->GetTransRowBytes());
		}
	}
	if (object && typeid(*object) == typeid(G42PathEnd))
	{
		CleanupStrips(); // sends out last strip if necessary
		Receiver->Process(object);
		Receiver = 0;
		Sender = 0;
	}
	else if (object)
	{
		Receiver->Process(object);
	}
}
void
G42ImageCrop::GetStrip(G42HeapManager & heap, uint32 strip_index)
{
	if (CurData && CurData->GetStripNum() == strip_index)
	{
		return;
	}
	if (CurData)
	{
		CurLock = 0;
		Receiver->Process(CurData.Release());
	}
	CurData = NewInfo->CreateData(strip_index);
	G42ThrowIfNull(CurData);
	CurLock = new G42WriteImageData(*CurData);
	G42ThrowIfNull(CurLock);
}
void
G42ImageCrop::CleanupStrips(void)
{
	if (CurData)
	{
		CurLock = 0;
		Receiver->Process(CurData.Release());
	}
}

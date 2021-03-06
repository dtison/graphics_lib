// g42image.cpp
#include "giimage.h"
G42Image::G42Image()
:
	NumFrames(0), Modified(false), MultiDescriptor(0),
	SavedMultiDescriptor(0), NumSavedFrames(0), ImageOptions(0),
	SavedImageOptions(0), NumPathFrames(0)
{
}
G42Image::G42Image(G42Image & ref)
:
	NumFrames(0), Modified(false), MultiDescriptor(0),
	SavedMultiDescriptor(0), NumSavedFrames(0), ImageOptions(0),
	SavedImageOptions(0), NumPathFrames(0)
{
	Copy(ref);
}
G42Image &
G42Image::operator = (G42Image & ref)
{
	/*  Std processing for op =   */
	if (&ref != this)
	{
		Clear();
		Copy(ref);
	}
	return *this;
}
G42Image::~G42Image()
{
	Clear();
	ClearSavedPath();
}
void
G42Image::Copy(G42Image & ref)
{
	if (&ref != this)
	{
		Clear();
		if (ref.MultiDescriptor)
		{
			MultiDescriptor = ref.MultiDescriptor->MakeCopy();
		}
		if (ref.ImageOptions)
		{
			ImageOptions = dynamic_cast<G42ImageReadWriteOptions *>
				(ref.ImageOptions->MakeCopy());
		}
		NumFrames = ref.NumFrames;
		for (uint32 i = 0; i < NumFrames; i++)
		{
			if (ref.Descriptor[i])
				Descriptor[i] = new G42ImageDescriptor(*(ref.Descriptor[i]));
			else
				Descriptor[i] = 0;
			if (ref.Frame[i])
				Frame[i] = new G42AnimatedImageFrame(*(ref.Frame[i]));
			else
				Frame[i] = 0;
			if (ref.Data[i] && Descriptor[i])
			{
				Data[i] = new G42ImageData * [Descriptor[i]->GetNumStrips()];
				memset(Data[i], 0,
					Descriptor[i]->GetNumStrips() * sizeof (G42ImageData *));
				G42ImageData ** refdata = ref.Data[i];
				G42ImageData ** data = Data[i];
				for (int j = 0; j < Descriptor[i]->GetNumStrips(); j++)
				{
					if (refdata[j])
						data[j] = refdata[j]->MakeCopy();
				}
			}
			else
			{
				Data[i] = 0;
			}
		}
		Modified = ref.Modified;
	}
}
/*  This is like a reset  */
void
G42Image::Clear()
{
	if (MultiDescriptor)
	{
		delete MultiDescriptor;
		MultiDescriptor = 0;
	}
	if (ImageOptions)
	{
		delete ImageOptions;
		ImageOptions = 0;
	}
	for (uint32 j = 0; j < NumFrames; j++)
	{
		if (Data[j])
		{
			for (int i = 0; i < Descriptor[j]->GetNumStrips(); i++)
			{
				if (Data[j][i])
					delete Data[j][i];
			}
			delete [] Data[j];
			Data[j] = 0;
		}
		if (Descriptor[j])
		{
			delete Descriptor[j];
			Descriptor[j] = 0;
		}
		if (Frame[j])
		{
			delete Frame[j];
			Frame[j] = 0;
		}
	}
	Frame.Clear();
	Data.Clear();
	Descriptor.Clear();
	
	NumFrames = 0;
	Modified = false;
}
#ifdef MACOS
#pragma RTTI on
#endif
void
G42Image::Process(G42PathObject * object, G42PathComponent *)
{
	if (typeid(*object) == typeid(G42PathBegin))
	{
		PreparePath();
		Clear(); // if prepare doesn't do it for us
		CurPathFrame = -1;
	}
	else if (dynamic_cast<G42MultipleImageDescriptor *>(object))
	{
		G42MultipleImageDescriptor * desc =
			dynamic_cast<G42MultipleImageDescriptor *>(object);
		if (!IsHead && Receiver)
		{
			MultiDescriptor = desc->MakeCopy();
		}
		else
		{
			MultiDescriptor = desc;
			object = 0;
		}
		Modified = true;
	}
	else if (dynamic_cast<G42ImageReadWriteOptions *>(object))
	{
		G42ImageReadWriteOptions * options =
			dynamic_cast<G42ImageReadWriteOptions *>(object);
		if (!IsHead && Receiver)
		{
			ImageOptions = dynamic_cast<G42ImageReadWriteOptions *>
				(options->MakeCopy());
		}
		else
		{
			ImageOptions = options;
			object = 0;
		}
	}
	else if (typeid(*object) == typeid(G42ImageDescriptor))
	{
		G42ImageDescriptor * desc = dynamic_cast<G42ImageDescriptor *>(object);
		CurPathFrame++;
		if (CurPathFrame < NumPathFrames)
		{
			NumFrames = PathFrames[CurPathFrame] + 1;
		}
		else if (NumPathFrames)
		{
			NumFrames = CurPathFrame - NumPathFrames +
				PathFrames[NumPathFrames - 1] + 1;
		}
		else
		{
			NumFrames++;
		}
		if (!IsHead && Receiver)
		{
			Descriptor[NumFrames - 1] = new G42ImageDescriptor(*desc);
		}
		else
		{
			Descriptor[NumFrames - 1] = desc;
			object = 0;
		}
		Data[NumFrames - 1] =
			new G42ImageData * [Descriptor[NumFrames - 1]->GetNumStrips()];
		memset(Data[NumFrames - 1], 0,
			Descriptor[NumFrames - 1]->GetNumStrips() * sizeof (G42ImageData *));
		Frame[NumFrames - 1] = 0;
		Modified = true;
	}
	else if (typeid(*object) == typeid(G42AnimatedImageFrame))
	{
		G42AnimatedImageFrame * frame =
			dynamic_cast<G42AnimatedImageFrame *>(object);
		if (Frame[NumFrames - 1])
		{
			delete Frame[NumFrames - 1];
		}
		if (!IsHead && Receiver)
		{
			Frame[NumFrames - 1] = new G42AnimatedImageFrame(*frame);
		}
		else
		{
			Frame[NumFrames - 1] = frame;
			object = 0;
		}
	}
	else if (dynamic_cast<G42ImageData *>(object))
	{
		G42ImageData * data = dynamic_cast<G42ImageData *>(object);
		if ((Data[NumFrames - 1])[data->GetStripNum()])
		{
			delete (Data[NumFrames - 1])[data->GetStripNum()];
		}
		if (!IsHead && Receiver)
		{
			(Data[NumFrames - 1])[data->GetStripNum()] = data->MakeCopy();
		}
		else
		{
			(Data[NumFrames - 1])[data->GetStripNum()] = data;
			object = 0;
		}
		Modified = true;
	}
	else if (typeid(*object) == typeid(G42PathEnd))
	{
		if (!IsHead && Receiver)
		{
			Receiver->Process(object, this);
			Receiver = 0;
		}
		Sender = 0;
		NeedMultiUpdate = true;
		if (!Receiver)
		{
			UpdateMultiImage();
			ClearSavedPath();
		}
	}
	if (!IsHead && Receiver)
	{
		Receiver->Process(object, this);
	}
	else if (object)
	{
		delete object;
	}
}
#ifdef MACOS
#pragma RTTI reset
#endif
void
G42Image::BeginProcessing()
{
	if (Receiver)
	{
		if (Sender) // head and tail of a loop, give the blocks away
		{
			PreparePath();
			G42PathBegin * begin = new G42PathBegin();
			Receiver->Process(begin, this);
			if (SavedImageOptions)
			{
				G42ImageReadWriteOptions * options =
					dynamic_cast<G42ImageReadWriteOptions *>
					(SavedImageOptions->MakeCopy());
				G42ThrowIfNull(options);
				Receiver->Process(options, this);
			}
			if (SavedMultiDescriptor)
			{
				G42MultipleImageDescriptor * multi_descriptor =
					SavedMultiDescriptor->MakeCopy();
				G42ThrowIfNull(multi_descriptor);
				multi_descriptor->SetNumImages(NumPathFrames);
				Receiver->Process(multi_descriptor, this);
			}
			for (uint32 k = 0; k < NumPathFrames; k++)
			{
				uint32 j = PathFrames[k];
				G42ImageDescriptor * descriptor =
					new G42ImageDescriptor(*SavedDescriptor[j]);
				G42ThrowIfNull(descriptor);
				Receiver->Process(descriptor, this);
				// send a frame if there is one
				if (SavedFrame[j])
				{
					G42AnimatedImageFrame * frame =
						new G42AnimatedImageFrame(*SavedFrame[j]);
					G42ThrowIfNull(frame);
					Receiver->Process(frame, this);
				}
				for (int i = 0; i < SavedDescriptor[j]->GetNumStrips(); i++)
				{
					if ((SavedData[j])[i])
					{
						G42ImageData * data = (SavedData[j])[i]->MakeCopy();
						Receiver->Process(data, this);
					}
				}
			}
			G42PathEnd * end = new G42PathEnd();
			Receiver->Process(end, this);
			if (!Sender)
			{
				if (NeedMultiUpdate)
				{
					UpdateMultiImage();
				}
				ClearSavedPath();
				IsHead = false;
			}
		}
		else
		{
			if (!NumPathFrames)
			{
				NumPathFrames = NumFrames;
				for (uint32 i = 0; i < NumFrames; i++)
				{
					PathFrames[i] = i;
				}
			}
			G42PathBegin * begin = new G42PathBegin();
			Receiver->Process(begin, this);
			if (ImageOptions)
			{
				G42ImageReadWriteOptions * options =
					dynamic_cast<G42ImageReadWriteOptions *>
					(ImageOptions->MakeCopy());
				G42ThrowIfNull(options);
				Receiver->Process(options, this);
			}
			if (MultiDescriptor)
			{
				G42MultipleImageDescriptor * multi_descriptor =
					MultiDescriptor->MakeCopy();
				G42ThrowIfNull(multi_descriptor);
				multi_descriptor->SetNumImages(NumPathFrames);
				Receiver->Process(multi_descriptor, this);
			}
			for (uint32 k = 0; k < NumPathFrames; k++)
			{
				uint32 j = PathFrames[k];
				G42ImageDescriptor * descriptor =
					new G42ImageDescriptor(*Descriptor[j]);
				G42ThrowIfNull(descriptor);
				Receiver->Process(descriptor, this);
				// send a frame if there is one
				if (Frame[j])
				{
					G42AnimatedImageFrame * frame =
						new G42AnimatedImageFrame(*Frame[j]);
					G42ThrowIfNull(frame);
					Receiver->Process(frame, this);
				}
				for (int i = 0; i < Descriptor[j]->GetNumStrips(); i++)
				{
					if ((Data[j])[i])
					{
						G42ImageData * data = (Data[j])[i]->MakeCopy();
						Receiver->Process(data, this);
					}
				}
			}
			G42PathEnd * end = new G42PathEnd();
			Receiver->Process(end, this);
			IsHead = false;
		}
		Receiver = 0;
//		G42PathComponent::ClearPath();
	}
	else
	{
		throw G42PathNoReceiverException();
	}
}
void
G42Image::PreparePath(void)
{
	if (!NumPathFrames)
	{
		NumPathFrames = NumFrames;
		for (uint32 i = 0; i < NumFrames; i++)
		{
			PathFrames[i] = i;
		}
	}
	if (IsHead && !NumSavedFrames)
	{
		NeedMultiUpdate = false;
		SavedMultiDescriptor = MultiDescriptor;
		SavedImageOptions = ImageOptions;
		NumSavedFrames = NumFrames;
		for (uint32 i = 0; i < NumFrames; i++)
		{
			SavedDescriptor[i] = Descriptor[i];
			SavedData[i] = Data[i];
			SavedFrame[i] = Frame[i];
			Descriptor[i] = 0;
			Data[i] = 0;
			Frame[i] = 0;
		}
		NumFrames = 0;
		MultiDescriptor = 0;
		ImageOptions = 0;
	}
	else if (!IsHead) // && Sender && !Receiver) // why?
	{
		Clear();
	}
}
void
G42Image::RestoreSavedPath(void)
{
	if (NumSavedFrames)
	{
		Clear();
		MultiDescriptor = SavedMultiDescriptor;
		ImageOptions = SavedImageOptions;
		NumFrames = NumSavedFrames;
		for (uint32 i = 0; i < NumSavedFrames; i++)
		{
			Descriptor[i] = SavedDescriptor[i];
			Data[i] = SavedData[i];
			Frame[i] = SavedFrame[i];
			SavedDescriptor[i] = 0;
			SavedData[i] = 0;
			SavedFrame[i] = 0;
		}
		NumSavedFrames = 0;
		SavedMultiDescriptor = 0;
		SavedImageOptions = 0;
		SavedData.Clear();
		SavedDescriptor.Clear();
		SavedFrame.Clear();
	}
}
void
G42Image::ClearSavedPath(void)
{
	if (SavedMultiDescriptor)
	{
		delete SavedMultiDescriptor;
		SavedMultiDescriptor = 0;
	}
	if (SavedImageOptions)
	{
		delete SavedImageOptions;
		SavedImageOptions = 0;
	}
	for (uint32 j = 0; j < NumSavedFrames; j++)
	{
		if (SavedData[j])
		{
			for (int i = 0; i < SavedDescriptor[j]->GetNumStrips(); i++)
			{
				if ((SavedData[j])[i])
				{
					delete (SavedData[j])[i];
				}
			}
			delete [] SavedData[j];
			SavedData[j] = 0;
		}
		if (SavedDescriptor[j])
		{
			delete SavedDescriptor[j];
			SavedDescriptor[j] = 0;
		}
		if (SavedFrame[j])
		{
			delete SavedFrame[j];
			SavedFrame[j] = 0;
		}
	}
	SavedData.Clear();
	SavedDescriptor.Clear();
	SavedFrame.Clear();
	NumSavedFrames = 0;
}
void
G42Image::UpdateMultiImage(void)
{
	if (!NeedMultiUpdate)
	{
		return;
	}
	if (NumPathFrames)
	{
		uintf cur_frame = 0;
		uintf cur_saved_frame = 0;
		uintf cur_path_frame = 0;
		uintf num_frames = NumFrames;
		NumFrames = 0;
		while (true)
		{
			// skip over frames that were passed in
			if (cur_path_frame < NumPathFrames &&
				cur_frame == PathFrames[cur_path_frame] && Descriptor[cur_frame])
			{
				cur_frame++;
				cur_path_frame++;
				cur_saved_frame++;
				NumFrames++;
			}
			// skip over extra frames that have been passed in
			else if (cur_path_frame >= NumPathFrames && NumFrames < num_frames &&
				Descriptor[NumFrames])
			{
				cur_frame++;
				NumFrames++;
			}
			// copy frames that were not expected to be passed in
			else if (cur_saved_frame < NumSavedFrames &&
				SavedDescriptor[cur_saved_frame])
			{
				Frame[NumFrames] = SavedFrame[cur_saved_frame];
				SavedFrame[cur_saved_frame] = 0;
				Descriptor[NumFrames] = SavedDescriptor[cur_saved_frame];
				SavedDescriptor[cur_saved_frame] = 0;
				for (uintf i = 0; i < Descriptor[cur_saved_frame]->GetNumStrips(); i++)
				{
					(Data[NumFrames])[i] = (SavedData[cur_saved_frame])[i];
					(SavedData[cur_saved_frame])[i] = 0;
				}
				cur_saved_frame++;
				if (cur_path_frame < NumPathFrames)
				{
					cur_frame++;
				}
				cur_path_frame++;
				NumFrames++;
			}
			// move any remaining passed in frames up in the list to remove holes
			// (don't know if this will ever be called)
			else if (cur_frame < num_frames)
			{
				Frame[NumFrames] = Frame[cur_frame];
				Frame[cur_frame] = 0;
				Descriptor[NumFrames] = Descriptor[cur_frame];
				Descriptor[cur_frame] = 0;
				for (uintf i = 0; i < Descriptor[cur_frame]->GetNumStrips(); i++)
				{
					(Data[NumFrames])[i] = (Data[cur_frame])[i];
					(Data[cur_frame])[i] = 0;
				}
				cur_frame++;
				NumFrames++;
			}
			// nothing else to do, break out of loop
			else
			{
				break;
			}
		}
		NumPathFrames = 0;
		PathFrames.Clear();
	}
	if (NumFrames == 1 && MultiDescriptor)
	{
		delete MultiDescriptor;
		MultiDescriptor = 0;
	}
	if (NumFrames == 1 && Frame[0])
	{
		delete Frame[0];
		Frame[0] = 0;
	}
	if (NumFrames > 1)
	{
		if (!MultiDescriptor)
		{
			MultiDescriptor = new G42MultipleImageDescriptor(NumFrames);
		}
		else
		{
			MultiDescriptor->SetNumImages(NumFrames);
		}
	}
	NeedMultiUpdate = false;
}
void
G42Image::ClearPath(bool path_excepted)
{
	G42PathComponent::ClearPath(path_excepted);
	if (path_excepted)
	{
		RestoreSavedPath();
	}
	else
	{
		UpdateMultiImage();
		ClearSavedPath();
	}
	NumPathFrames = 0;
	PathFrames.Clear();
}
void
G42Image::SetPathFrames(const uint32 * indexes, uint32 num_indexes)
{
	NumPathFrames = num_indexes;
	for (uint32 i = 0; i < NumPathFrames; i++)
	{
		PathFrames[i] = indexes[i];
	}
}
void
G42Image::SetPathFrames(const G42Vector <uint32> & indexes, uint32 num_indexes)
{
	NumPathFrames = num_indexes;
	for (uint32 i = 0; i < NumPathFrames; i++)
	{
		PathFrames[i] = indexes[i];
	}
}
void
G42Image::ClearPathFrames(void)
{
	NumPathFrames = 0;
	PathFrames.Clear();
}

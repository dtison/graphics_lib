#include "g42idata.h"
#include "g42iter.h"
#include "g42event.h"
#include "g42image.h"
#include "g42iview.h"
G42ImageData *
MirrorX(G42ImageData * data, G42EventUpdateHandler * event_handler, bool update = true)
{
	if (update)
	{
		event_handler->SetDenominator((int32)data->GetHeight());
	   event_handler->Start();
   	event_handler->Set(0L);
   }
  	int32 row_count = 0L;
	switch (data->GetDepth())
	{
   	case 32:
		case 24:
		{
			G42ImageData * new_data = new G42ImageData(data->GetData(),
				data->GetWidth(), data->GetHeight(), 8*TrueColorPixelSize, 0);
			new_data->ReferenceCount++;
			new_data->SetFinished(true);
			new_data->SetValidStartRow(0);
			new_data->SetValidNumRows(new_data->GetHeight());
			G42LockedID24BitIterator image_it1(data);
			G42ImageData24BitIterator image_it2(new_data);
			image_it2.SetRowEnd();
			do
			{
				image_it2.Set(image_it1.GetBlue(), image_it1.GetGreen(),
					image_it1.GetRed());
				image_it2.Decrement();
				if (!image_it1.Increment())
				{
					if (!image_it1.NextRow())
						break;
					image_it2.NextRow();
               image_it2.SetRowEnd();
               if (update)
               {
	               row_count++;
   	            event_handler->Set(row_count);
               }
				}
			} while (1);
         if (update)
	 	     event_handler->Stop();
			return new_data;
		}
		case 8:
		{
			G42ImageData * new_data;
			if (data->GetNumPalette() != 0)
			{
				new_data = new G42ImageData(data->GetData(),
					data->GetWidth(), data->GetHeight(), 8, data->GetNumPalette());
				new_data->SetPalette(data->GetPalette());
			}
			else
				new_data = new G42ImageData(data->GetData(),
					data->GetWidth(), data->GetHeight(), 8, 0);
			new_data->ReferenceCount++;
			new_data->SetFinished(true);
			new_data->SetValidStartRow(0);
			new_data->SetValidNumRows(new_data->GetHeight());
			G42LockedID8BitIterator image_it1(data);
			G42ImageData8BitIterator image_it2(new_data);
			image_it2.SetRowEnd();
			do
			{
				image_it2.Set((unsigned char)image_it1);
				image_it2.Decrement();
				if (!image_it1.Increment())
				{
					if (!image_it1.NextRow())
						break;
					image_it2.NextRow();
               image_it2.SetRowEnd();
               if (update)
               {
	               row_count++;
   	            event_handler->Set(row_count);
               }
				}
			} while (1);
			if (update)
	         event_handler->Stop();
			return new_data;
		}
		case 4:
		{
			G42ImageData * new_data;
			if (data->GetNumPalette() != 0)
			{
				new_data = new G42ImageData(data->GetData(),
					data->GetWidth(), data->GetHeight(), 4, data->GetNumPalette());
				new_data->SetPalette(data->GetPalette());
			}
			else
				new_data = new G42ImageData(data->GetData(),
					data->GetWidth(), data->GetHeight(), 4, 0);
			new_data->ReferenceCount++;
			new_data->SetFinished(true);
			new_data->SetValidStartRow(0);
			new_data->SetValidNumRows(new_data->GetHeight());
			G42LockedID4BitIterator image_it1(data);
			G42ImageData4BitIterator image_it2(new_data);
			image_it2.SetRowEnd();
			do
			{
				image_it2.Set4((unsigned char)image_it1);
				image_it2.Decrement();
				if (!image_it1.Increment())
				{
					if (!image_it1.NextRow())
						break;
					image_it2.NextRow();
					image_it2.SetRowEnd();
               if (update)
               {
	               row_count++;
   	            event_handler->Set(row_count);
               }
				}
			} while (1);
			if (update)
	         event_handler->Stop();
			return new_data;
		}
		case 1:
		{
			G42ImageData * new_data;
			if (data->GetNumPalette() != 0)
			{
				new_data = new G42ImageData(data->GetData(),
					data->GetWidth(), data->GetHeight(), 1, data->GetNumPalette());
				new_data->SetPalette(data->GetPalette());
			}
			else
				new_data = new G42ImageData(data->GetData(),
					data->GetWidth(), data->GetHeight(), 1, 0);
			new_data->ReferenceCount++;
			new_data->SetFinished(true);
			new_data->SetValidStartRow(0);
			new_data->SetValidNumRows(new_data->GetHeight());
			G42LockedID1BitIterator image_it1(data);
			G42ImageData1BitIterator image_it2(new_data);
			image_it2.SetRowEnd();
			do
			{
				image_it2.Set8((unsigned char)image_it1);
				image_it2.Decrement();
				if (!image_it1.Increment())
				{
					if (!image_it1.NextRow())
						break;
					image_it2.NextRow();
               image_it2.SetRowEnd();
               if (update)
               {
   	            row_count++;
	               event_handler->Set(row_count);
               }
				}
			} while (1);
			if (update)
	         event_handler->Stop();
			return new_data;
		}
	}
}
G42ImageData *
MirrorY(G42ImageData * data, G42EventUpdateHandler * event_handler, bool update = true)
{
	if (update)
   {
		event_handler->SetDenominator((int32)data->GetHeight());
   	event_handler->Start();
	   event_handler->Set(0L);
   }
   int32 row_count = 0L;
	switch (data->GetDepth())
	{
		case 32:
		case 24:
		{
			G42ImageData * new_data = new G42ImageData(data->GetData(),
				data->GetWidth(), data->GetHeight(), 8*TrueColorPixelSize, 0);
			new_data->ReferenceCount++;
			new_data->SetFinished(true);
			new_data->SetValidStartRow(0);
			new_data->SetValidNumRows(new_data->GetHeight());
			G42LockedID24BitIterator image_it1(data);
			G42ImageData24BitIterator image_it2(new_data);
			image_it2.SetLastRow();
			do
			{
				image_it2.Set(image_it1.GetBlue(), image_it1.GetGreen(),
					image_it1.GetRed());
				image_it2.Increment();
				if (!image_it1.Increment())
				{
					if (!image_it1.NextRow())
						break;
					image_it2.ResetRow();
					image_it2.DecRow();
               if (update)
               {
	               row_count++;
   	            event_handler->Set(row_count);
               }
				}
			} while (1);
      	if (update)
			   event_handler->Stop();
			return new_data;
		}
		case 8:
		{
			G42ImageData * new_data;
			if (data->GetNumPalette() != 0)
			{
				new_data = new G42ImageData(data->GetData(),
					data->GetWidth(), data->GetHeight(), 8, data->GetNumPalette());
				new_data->SetPalette(data->GetPalette());
			}
			else
				new_data = new G42ImageData(data->GetData(),
					data->GetWidth(), data->GetHeight(), 8, 0);
			new_data->ReferenceCount++;
			new_data->SetFinished(true);
			new_data->SetValidStartRow(0);
			new_data->SetValidNumRows(new_data->GetHeight());
			G42LockedID8BitIterator image_it1(data);
			G42ImageData8BitIterator image_it2(new_data);
			image_it2.SetLastRow();
			do
			{
				image_it2.Set((unsigned char)image_it1);
				image_it2.Increment();
				if (!image_it1.Increment())
				{
					if (!image_it1.NextRow())
						break;
					image_it2.ResetRow();
               image_it2.DecRow();
               if (update)
               {
	               row_count++;
   	            event_handler->Set(row_count);
               }
				}
			} while (1);
   		if (update)
  	         event_handler->Stop();
			return new_data;
		}
		case 4:
		{
			G42ImageData * new_data;
			if (data->GetNumPalette() != 0)
			{
				new_data = new G42ImageData(data->GetData(),
					data->GetWidth(), data->GetHeight(), 4, data->GetNumPalette());
				new_data->SetPalette(data->GetPalette());
			}
			else
				new_data = new G42ImageData(data->GetData(),
					data->GetWidth(), data->GetHeight(), 4, 0);
			new_data->ReferenceCount++;
			new_data->SetFinished(true);
			new_data->SetValidStartRow(0);
			new_data->SetValidNumRows(new_data->GetHeight());
			G42LockedID4BitIterator image_it1(data);
			G42ImageData4BitIterator image_it2(new_data);
			image_it2.SetLastRow();
			do
			{
				image_it2.Set4((unsigned char)image_it1);
				image_it2.Increment();
				if (!image_it1.Increment())
				{
					if (!image_it1.NextRow())
						break;
					image_it2.ResetRow();
					image_it2.DecRow();
               if (update)
               {
	               row_count++;
   	            event_handler->Set(row_count);
               }
				}
			} while (1);
			if (update)
	         event_handler->Stop();
			return new_data;
		}
		case 1:
		{
			G42ImageData * new_data;
			if (data->GetNumPalette() != 0)
			{
				new_data = new G42ImageData(data->GetData(),
					data->GetWidth(), data->GetHeight(), 1, data->GetNumPalette());
				new_data->SetPalette(data->GetPalette());
			}
			else
				new_data = new G42ImageData(data->GetData(),
					data->GetWidth(), data->GetHeight(), 1, 0);
			new_data->ReferenceCount++;
			new_data->SetFinished(true);
			new_data->SetValidStartRow(0);
			new_data->SetValidNumRows(new_data->GetHeight());
			G42LockedID1BitIterator image_it1(data);
			G42ImageData1BitIterator image_it2(new_data);
			image_it2.SetLastRow();
			do
			{
				image_it2.Set8((unsigned char)image_it1);
				image_it2.Increment();
				if (!image_it1.Increment())
				{
					if (!image_it1.NextRow())
						break;
					image_it2.ResetRow();
               image_it2.DecRow();
               if (update)
               {
	               row_count++;
   	            event_handler->Set(row_count);
               }
				}
			} while (1);
   		if (update)
	         event_handler->Stop();
			return new_data;
		}
	}
}
G42ImageData *
Rotate90(G42ImageData * data, G42EventUpdateHandler * event_handler, bool update = true)
{
	if (update)
   {
		event_handler->SetDenominator((int32)data->GetHeight());
   	event_handler->Start();
	   event_handler->Set(0L);
   }
   int32 row_count = 0L;
	switch (data->GetDepth())
	{
		case 32:
		case 24:
		{
			G42ImageData * new_data = new G42ImageData(data->GetData(),
				data->GetHeight(), data->GetWidth(), 8*TrueColorPixelSize, 0);
			new_data->ReferenceCount++;
			new_data->SetFinished(true);
			new_data->SetValidStartRow(0);
			new_data->SetValidNumRows(new_data->GetHeight());
			G42LockedID24BitIterator image_it1(data);
			G42ImageData24BitIterator image_it2(new_data);
			image_it2.SetRowEnd();
			do
			{
				image_it2.Set(image_it1.GetBlue(), image_it1.GetGreen(),
					image_it1.GetRed());
				image_it2.IncRow();
				if (!image_it1.Increment())
				{
					if (!image_it1.NextRow())
						break;
					image_it2.Decrement();
               image_it2.ToTopRow();
					if (update)
               {
	               row_count++;
   	            event_handler->Set(row_count);
               }
				}
			} while (1);
			if (update)
				event_handler->Stop();
         return new_data;
		}
		case 8:
		{
			G42ImageData * new_data;
			if (data->GetNumPalette() != 0)
			{
				new_data = new G42ImageData(data->GetData(),
					data->GetHeight(), data->GetWidth(), 8, data->GetNumPalette());
				new_data->SetPalette(data->GetPalette());
			}
			else
				new_data = new G42ImageData(data->GetData(),
					data->GetHeight(), data->GetWidth(), 8, 0);
			new_data->ReferenceCount++;
			new_data->SetFinished(true);
			new_data->SetValidStartRow(0);
			new_data->SetValidNumRows(new_data->GetHeight());
			G42LockedID8BitIterator image_it1(data);
			G42ImageData8BitIterator image_it2(new_data);
			image_it2.SetRowEnd();
			do
			{
				image_it2.Set((unsigned char)image_it1);
				image_it2.IncRow();
				if (!image_it1.Increment())
				{
					if (!image_it1.NextRow())
						break;
					image_it2.Decrement();
               image_it2.ToTopRow();
					if (update)
               {
	               row_count++;
   	            event_handler->Set(row_count);
               }
				}
			} while (1);
			if (update)
	         event_handler->Stop();
			return new_data;
		}
		case 4:
		{
			G42ImageData * new_data;
			if (data->GetNumPalette() != 0)
			{
				new_data = new G42ImageData(data->GetData(),
					data->GetHeight(), data->GetWidth(), 4, data->GetNumPalette());
				new_data->SetPalette(data->GetPalette());
			}
			else
				new_data = new G42ImageData(data->GetData(),
					data->GetHeight(), data->GetWidth(), 4, 0);
			new_data->ReferenceCount++;
			new_data->SetFinished(true);
			new_data->SetValidStartRow(0);
			new_data->SetValidNumRows(new_data->GetHeight());
			G42LockedID4BitIterator image_it1(data);
			G42ImageData4BitIterator image_it2(new_data);
			image_it2.SetRowEnd();
			do
			{
				image_it2.Set4((unsigned char)image_it1);
				image_it2.IncRow();
				if (!image_it1.Increment())
				{
					if (!image_it1.NextRow())
						break;
					image_it2.Decrement();
               image_it2.ToTopRow();
					if (update)
               {
	               row_count++;
   	            event_handler->Set(row_count);
               }
				}
			} while (1);
			if (update)
	         event_handler->Stop();
			return new_data;
		}
		case 1:
		{
			G42ImageData * new_data;
			if (data->GetNumPalette() != 0)
			{
				new_data = new G42ImageData(data->GetData(),
					data->GetHeight(), data->GetWidth(), 1, data->GetNumPalette());
				new_data->SetPalette(data->GetPalette());
			}
			else
				new_data = new G42ImageData(data->GetData(),
					data->GetHeight(), data->GetWidth(), 1, 0);
			new_data->ReferenceCount++;
			new_data->SetFinished(true);
			new_data->SetValidStartRow(0);
			new_data->SetValidNumRows(new_data->GetHeight());
			G42LockedID1BitIterator image_it1(data);
			G42ImageData1BitIterator image_it2(new_data);
			image_it2.SetRowEnd();
			do
			{
				image_it2.Set8((unsigned char)image_it1);
				image_it2.IncRow();
				if (!image_it1.Increment())
				{
					if (!image_it1.NextRow())
						break;
					image_it2.Decrement();
               image_it2.ToTopRow();
					if (update)
               {
	               row_count++;
   	            event_handler->Set(row_count);
               }
				}
			} while (1);
			if (update)
	         event_handler->Stop();
			return new_data;
		}
	}
}
G42ImageData *
Rotate180(G42ImageData * data, G42EventUpdateHandler * event_handler, bool update = true)
{
	G42ImageData * new_data = MirrorX(data, event_handler, update);
	G42ImageData * new_data2 = MirrorY(new_data, event_handler, update);
	delete new_data;
	return new_data2;
}
G42ImageData *
Rotate270(G42ImageData * data, G42EventUpdateHandler * event_handler, bool update = true)
{
	if (update)
	{
		event_handler->SetDenominator((int32)data->GetHeight());
   	event_handler->Start();
	   event_handler->Set(0L);
   }
   int32 row_count = 0L;
	switch (data->GetDepth())
	{
   	case 32:
		case 24:
		{
			G42ImageData * new_data = new G42ImageData(data->GetData(),
				data->GetHeight(), data->GetWidth(), 8*TrueColorPixelSize, 0);
			new_data->ReferenceCount++;
			new_data->SetFinished(true);
			new_data->SetValidStartRow(0);
			new_data->SetValidNumRows(new_data->GetHeight());
			G42LockedID24BitIterator image_it1(data);
			G42ImageData24BitIterator image_it2(new_data);
			image_it2.SetLastRow();
			do
			{
				image_it2.Set(image_it1.GetBlue(), image_it1.GetGreen(),
					image_it1.GetRed());
				image_it2.DecRow();
				if (!image_it1.Increment())
				{
					if (!image_it1.NextRow())
						break;
					image_it2.Increment();
					image_it2.SetLastRow();
					if (update)
               {
						row_count++;
   	            event_handler->Set(row_count);
               }
				}
			} while (1);
			if (update)
				event_handler->Stop();
         return new_data;
		}
		case 8:
		{
			G42ImageData * new_data;
			if (data->GetNumPalette() != 0)
			{
				new_data = new G42ImageData(data->GetData(),
					data->GetHeight(), data->GetWidth(), 8, data->GetNumPalette());
				new_data->SetPalette(data->GetPalette());
			}
			else
				new_data = new G42ImageData(data->GetData(),
					data->GetHeight(), data->GetWidth(), 8, 0);
			new_data->ReferenceCount++;
			new_data->SetFinished(true);
			new_data->SetValidStartRow(0);
			new_data->SetValidNumRows(new_data->GetHeight());
			G42LockedID8BitIterator image_it1(data);
			G42ImageData8BitIterator image_it2(new_data);
			image_it2.SetLastRow();
			do
			{
				image_it2.Set((unsigned char)image_it1);
				image_it2.DecRow();
				if (!image_it1.Increment())
				{
					if (!image_it1.NextRow())
						break;
					image_it2.Increment();
               image_it2.SetLastRow();
					if (update)
               {
	               row_count++;
   	            event_handler->Set(row_count);
               }
				}
			} while (1);
			if (update)
	         event_handler->Stop();
			return new_data;
		}
		case 4:
		{
			G42ImageData * new_data;
			if (data->GetNumPalette() != 0)
			{
				new_data = new G42ImageData(data->GetData(),
					data->GetHeight(), data->GetWidth(), 4, data->GetNumPalette());
				new_data->SetPalette(data->GetPalette());
			}
			else
				new_data = new G42ImageData(data->GetData(),
					data->GetHeight(), data->GetWidth(), 4, 0);
			new_data->ReferenceCount++;
			new_data->SetFinished(true);
			new_data->SetValidStartRow(0);
			new_data->SetValidNumRows(new_data->GetHeight());
			G42LockedID4BitIterator image_it1(data);
			G42ImageData4BitIterator image_it2(new_data);
			image_it2.SetLastRow();
			do
			{
				image_it2.Set4((unsigned char)image_it1);
				image_it2.DecRow();
				if (!image_it1.Increment())
				{
					if (!image_it1.NextRow())
						break;
					image_it2.Increment();
					image_it2.SetLastRow();
					if (update)
               {
	               row_count++;
   	            event_handler->Set(row_count);
               }
				}
			} while (1);
			if (update)
	         event_handler->Stop();
			return new_data;
		}
		case 1:
		{
			G42ImageData * new_data;
			if (data->GetNumPalette() != 0)
			{
				new_data = new G42ImageData(data->GetData(),
					data->GetHeight(), data->GetWidth(), 1, data->GetNumPalette());
				new_data->SetPalette(data->GetPalette());
			}
			else
				new_data = new G42ImageData(data->GetData(),
					data->GetHeight(), data->GetWidth(), 1, 0);
			new_data->ReferenceCount++;
			new_data->SetFinished(true);
			new_data->SetValidStartRow(0);
			new_data->SetValidNumRows(new_data->GetHeight());
			G42LockedID1BitIterator image_it1(data);
			G42ImageData1BitIterator image_it2(new_data);
			image_it2.SetLastRow();
			do
			{
				image_it2.Set8((unsigned char)image_it1);
				image_it2.DecRow();
				if (!image_it1.Increment())
				{
					if (!image_it1.NextRow())
						break;
					image_it2.Increment();
               image_it2.SetLastRow();
					if (update)
               {
	               row_count++;
   	            event_handler->Set(row_count);
               }
				}
			} while (1);
			if (update)
	         event_handler->Stop();
			return new_data;
		}
	}
}
G42ImageData *
Rotate(G42ImageData * data, int angle, G42EventUpdateHandler * event_handler,
	bool update = true)
{
	switch(angle)
	{
		case 90:
			return Rotate90(data, event_handler, update);
		case 180:
			return Rotate180(data, event_handler, update);
		case 270:
			return Rotate270(data, event_handler, update);
		default:
			return data;
	}
}
void
G42Image::MirrorX(void)
{
	G42ImageData * new_data = Data;
	G42ImageData * new_mask = Mask;
	if (Data)
   {
		new_data = ::MirrorX(Data, GetEventHandler());
	   if (Mask)
 			new_mask = ::MirrorX(Mask, GetEventHandler(), false);
   }
	if (new_data && Data != new_data)
	{
		delete Data;
		Data = new_data;
      if (new_mask && Mask != new_mask)
      {
	      delete Mask;
   	   Mask = new_mask;
      }
		RebuildInfo();
		for (int i = 0; i < NumViewArray; i++)
		{
			ViewArray[i]->ImageChanged();
		}
	}
   Dirty = true;
}
void
G42Image::MirrorY(void)
{
	G42ImageData * new_data = Data;
   G42ImageData * new_mask = Mask;
	if (Data)
   {
		new_data = ::MirrorY(Data, GetEventHandler());
	   if (Mask)
 			new_mask = ::MirrorY(Mask, GetEventHandler(), false);
   }
	if (new_data && Data != new_data)
	{
		delete Data;
		Data = new_data;
      if (new_mask && Mask != new_mask)
      {
	      delete Mask;
   	   Mask = new_mask;
      }
		RebuildInfo();
		for (int i = 0; i < NumViewArray; i++)
		{
			ViewArray[i]->ImageChanged();
		}
	}
   Dirty = true;
}
void
G42Image::Rotate(int angle)
{
	G42ImageData * new_data = Data;
	G42ImageData * new_mask = Mask;
	if (Data)
   {
		new_data = ::Rotate(Data, angle, GetEventHandler());
	   if (Mask)
 			new_mask = ::Rotate(Mask, angle, GetEventHandler(), false);
	}
	if (new_data && Data != new_data)
	{
		delete Data;
		Data = new_data;
      if (new_mask && Mask != new_mask)
      {
	      delete Mask;
   	   Mask = new_mask;
      }
		RebuildInfo();
		for (int i = 0; i < NumViewArray; i++)
		{
			ViewArray[i]->ImageChanged();
		}
	}
	Dirty = true;
}

// gicoladj.cpp - color adjust: gamma, brightness, contrast filters
#include "gicoladj.h"
#include "gidesc.h"
#include "gidata.h"
#include "math.h"
G42ImageColorAdjust::G42ImageColorAdjust(intf brightness, intf contrast,
	double gamma, G42Channel Channel)
:
	PaletteMode(false), Brightness(brightness), Contrast(contrast), Gamma(gamma),
	Channel(Channel), NeedNewData(false)
{
	intf n; // because it is used multiple times
	Map.Set(new byte [256]);
	for (n = 0; n < 256; n++)
		Map[n] = (byte)n;
		
	SatMap.Set(new byte [256]);
	for (n = 0; n < 256; n++)
		SatMap[n] = (byte)n;
	if (Gamma != 1.0)
	{
		double gamma = (1.0 / Gamma);
		for (intf n = 0; n < 256; n++)
		{
			if (Map[n] > 0)
				Map[n] = (byte)(pow((double)((double)(Map[n]) / 255.0),
					gamma) * 255.0 + .5);
		}
	}
	if (Contrast)
	{
		int32 contrast = Contrast + 100;
		if (Contrast < 0)
		{
			intf n;
			for (n = 0; n < 256; n++)
			{
				int32 v = Map[n] - 128;
				v *= contrast;
				if (v > 0)
					v += 50;
				else
					v -= 50;
				v /= 100;
				if (v > 127)
					v = 127;
				if (v < -128)
					v = -128;
				v += 128;
				Map[n] = (byte)v;
			}
			
			for (n = 0; n < 256; n++)
			{
				SatMap[n] = (byte)(n + ((n * Contrast) / 100));
			}
		}
		else
		{
			if (Contrast == 100)
			{
				intf n = 0;
				while (n < 128)
				{
					Map[n++] = 0;
				}
				while (n < 256)
				{
					Map[n++] = 255;
				}
			}
			else
			{
				int32 cutoff = (Contrast * 128) / 100;
				intf n = 0;
				while (n < cutoff)
				{
					Map[n++] = 0;
				}
				while (n < (255 - cutoff))
				{
					Map[n++] = (byte)(((n - cutoff) * 255) / (255 - (cutoff << 1)));
				}
				while (n < 256)
				{
					Map[n++] = 255;
				}
			}
		}
	}
	if (Brightness)
	{
		if (Brightness > 0)
		{
			int32 brightness = 100 - Brightness;
			for (intf n = 0; n < 256; n++)
			{
				int32 v = 255 - Map[n];
				v *= brightness;
				v += 50;
				v /= 100;
				if (v > 255)
					v = 255;
				v = 255 - v;
				Map[n] = (byte)v;
				SatMap[n] = (byte)(n - ((n * (int32)Brightness) / 100));
			}
		}
		else if (Brightness < 0)
		{
			int32 brightness = 100 + Brightness;
			for (intf n = 0; n < 256; n++)
			{
				int32 v = Map[n];
				v *= brightness;
				v += 50;
				v /= 100;
				if (v > 255)
					v = 255;
				Map[n] = (byte)v;
				SatMap[n] = (byte)(n + ((n * (int32)Brightness) / 100));
			}
		}
	}
}
G42ImageColorAdjust::~G42ImageColorAdjust(void)
{
}
void
G42ImageColorAdjust::HandleDescriptor(G42ImageDescriptor * desc)
{
	if (desc->IsPaletted())
	{
		PaletteMode = true;
		NeedNewData = false;
		G42Color * pal = desc->GetPalette();
		for (intf i = 0; i < desc->GetNumPalette(); i++)
		{
			pal[i] = AdjustColor(pal[i]);
		}
	}
	else if (!desc->IsColor())
	{
		PaletteMode = false;
		if (desc->GetDepth() == 1 || Channel != All)
		{
			if (Channel == All && desc->GetDepth() == 1 &&
				Map[0] == 0 || Map[255] == 255) // no change in data
			{
				NeedNewData = false;
			}
			else
			{
				NeedNewData = true;
				InputDescriptor = desc;
				desc = 0;
			}
		}
		else
		{
			NeedNewData = false;
		}
	}
	else // true color, modify in data
	{
		PaletteMode = false;
		NeedNewData = false;
	}
	if (desc)
	{
		Receiver->Process(desc, this);
	}
	OutputRowsLeft = 0;
	CurrentStripNum = 0;
}
void
G42ImageColorAdjust::HandleData(G42ImageData * strip)
{
#if 0
	if (!PaletteMode)
	{
		if (dynamic_cast<G42ImageDataGray1 *>(strip))
		{
			if (NeedNewData)
			{
				G42WriteImageData wdata(*strip); // switch to rdata
				if (!OutputDescriptor)
				{
					G42ImageDescriptor * desc =
						new G42ImageDescriptor(InputDescriptor->GetWidth(),
						InputDescriptor->GetHeight(),
						(Channel != All ?
							G42ImageDescriptor::Color8 : G42ImageDescriptor::Gray8),
						strip->GetHeap(), InputDescriptor->IsTransparent());
					G42ThrowIfNull(desc);
					if (Channel != All)
					{
						desc->SetNumPalette(2);
						G42Color * pal = desc->GetPalette();
						pal[0] = AdjustColor(G42Color(0, 0, 0));
						pal[1] = AdjustColor(G42Color(0xff, 0xff, 0xff));
						MonoMap[0] = 0;
						MonoMap[1] = 1;
					}
					else // monochrome, channel all, go to grayscale
					{
						MonoMap[0] = Map[0];
						MonoMap[1] = Map[255];
					}
					if (InputDescriptor->IsTransparent())
					{
						desc->SetTransparency();
						desc->SetTransparentColor(
							InputDescriptor->GetTransparentColor());
						// we really need to take care of this problem one day
						desc->SetTransparentPixel(-1);
					}
					// need to take care of first and last strip if
					// old first strip is not full
					SetOutputDescriptor(desc);
					Receiver->Process(desc);
				}
				uint32 input_rows_left = strip->GetNumRows();
				G42ImageDataGray1 * g1strip =
					dynamic_cast<G42ImageDataGray1 *>(strip);
				G42ImageDataGray1::G42Iterator current_input_iterator =
					g1strip->GetBegin(wdata);
				G42ImageDataBase::G42TransIterator current_trans_iterator;
				if (OutputDescriptor->IsTransparent())
				{
					G42ImageDataBase * stript =
						dynamic_cast<G42ImageDataBase *>(stript);
					current_trans_iterator = stript->GetTransBegin(wdata);
				}
				while (input_rows_left)
				{
					if (!OutputRowsLeft)
					{
						GetOutputData(CurrentStripNum);
						if (Channel == All)
						{
							CurrentOutputIteratorGray8 =
								dynamic_cast<G42ImageDataGray8 &>(*CurrentOutputData).
									GetBegin(*CurrentOutputLock);
						}
						else
						{
							CurrentOutputIteratorColor8 =
								dynamic_cast<G42ImageDataColor8 &>(*CurrentOutputData).
									GetBegin(*CurrentOutputLock);
						}
#if 0
						if (CurrentOutputData)
						{
							CurrentOutputLock = 0;
							Receiver->Process(CurrentOutputData.Release());
						}
						if (Channel == All)
						{
							G42ImageDataGray8 * data = new G42ImageDataGray8(
								strip->GetHeap(), CurrentStripNum,
								OutputDescriptor->GetRowsInStrip(CurrentStripNum),
								OutputDescriptor->GetRowBytes(),
								OutputDescriptor->GetStripStartInImage(CurrentStripNum),
								OutputDescriptor->GetWidth(),
								OutputDescriptor->IsTransparent() ?
									OutputDescriptor->GetTransRowBytes() : 0);
							CurrentOutputData = data;
							CurrentOutputLock = new G42WriteImageData(*CurrentOutputData);
							CurrentOutputIteratorGray8 =
								data->GetBegin(*CurrentOutputLock);
						}
						else
						{
							G42ImageDataColor8 * data = new G42ImageDataColor8(
								strip->GetHeap(), CurrentStripNum,
								OutputDescriptor->GetRowsInStrip(CurrentStripNum),
								OutputDescriptor->GetRowBytes(),
								OutputDescriptor->GetStripStartInImage(CurrentStripNum),
								OutputDescriptor->GetWidth(),
								OutputDescriptor->IsTransparent() ?
									OutputDescriptor->GetTransRowBytes() : 0);
							CurrentOutputData = data;
							CurrentOutputLock = new G42WriteImageData(*CurrentOutputData);
							CurrentOutputIteratorColor8 =
								data->GetBegin(*CurrentOutputLock);
						}
#endif
						CurrentStripNum++;
						OutputRowsLeft = CurrentOutputData->GetNumRows();
						if (OutputDescriptor->IsTransparent())
						{
							OutputTransIterator =
								dynamic_cast<G42ImageDataBase &>(*CurrentOutputData).
									GetTransBegin(*CurrentOutputLock);
						}
					}
					uint32 num_rows = min(input_rows_left, OutputRowsLeft);
					G42ColorAdjustConvertMono convert(MonoMap);
					if (Channel == All)
					{
						CurrentOutputIteratorGray8 = G42ImageDataConvertCopy(
							current_input_iterator, current_input_iterator + num_rows,
							CurrentOutputIteratorGray8, convert);
					}
					else
					{
						CurrentOutputIteratorColor8 = G42ImageDataConvertCopy(
							current_input_iterator, current_input_iterator + num_rows,
							CurrentOutputIteratorColor8, convert);
					}
					OutputRowsLeft -= num_rows;
					input_rows_left -= num_rows;
					current_input_iterator += num_rows;
					if (OutputDescriptor->IsTransparent())
					{
						G42ImageDataBase::G42TransIterator tend =
							current_trans_iterator + num_rows;
						while (current_trans_iterator != tend)
						{
							memcpy((*OutputTransIterator).GetPtr(),
								(*current_trans_iterator).GetPtr(),
								OutputDescriptor->GetTransRowBytes());
							current_trans_iterator++;
							OutputTransIterator++;
						}
					}
				}
				delete strip;
				strip = 0; // don't send old data
			}
		}
		else if (dynamic_cast<G42ImageDataGray8 *>(strip))
		{
			G42ImageDataGray8 * g8strip =
				dynamic_cast<G42ImageDataGray8 *>(strip);
			if (NeedNewData) // converting to color
			{
				if (!OutputDescriptor)
				{
					OutputDescriptor = new G42ImageDescriptor(
						InputDescriptor->GetWidth(),
						InputDescriptor->GetHeight(), G42ImageDescriptor::Color8,
						InputDescriptor->GetHeap());
					OutputDescriptor->SetNumPalette(256);
					G42Color * pal = OutputDescriptor->GetPalette();
					for (intf i = 0; i < OutputDescriptor->GetNumPalette(); i++)
					{
						pal[i] = AdjustColor(G42Color((byte)i, (byte)i, (byte)i));
					}
					if (InputDescriptor->IsTransparent())
					{
						OutputDescriptor->SetTransparency();
						OutputDescriptor->SetTransparentColor(
							InputDescriptor->GetTransparentColor());
						// we really need to take care of this problem one day
						OutputDescriptor->SetTransparentPixel(-1);
					}
					// need to take care of first and last strip if
					// old first strip is not full
					G42PathObject * desc = new G42ImageDescriptor(*OutputDescriptor);
					G42ThrowIfNull(desc);
					Receiver->Process(desc);
				}
				G42ImageDataGray8 * g8strip =
					dynamic_cast<G42ImageDataGray8 *>(strip);
				G42ImageDataColor8 * c8strip = new G42ImageDataColor8(*g8strip);
				delete strip;
				strip = c8strip;
			}
			else
			{
				G42WriteImageData wdata(*strip);
				G42ImageDataConvert(g8strip->GetBegin(wdata),
					g8strip->GetEnd(wdata), G42GrayAdjustFunction(Map));
			}
		}
		else if (dynamic_cast<G42ImageDataColor24 *>(strip))
		{
			G42WriteImageData wdata(*strip);
			G42ImageDataColor24 * c24strip =
				dynamic_cast<G42ImageDataColor24 *>(strip);
			switch (Channel)
			{
				case All:
				{
					G42ImageDataConvert(c24strip->GetBegin(wdata),
						c24strip->GetEnd(wdata),
						G42ColorAdjustFunction<G42Color24>(Map, SatMap, (G42Color24 *)0));
					break;
				}
				case Red:
				{
					G42ImageDataConvert(c24strip->GetBegin(wdata),
						c24strip->GetEnd(wdata),
						G42RedAdjustFunction<G42Color24>(Map));
					break;
				}
				case Green:
				{
					G42ImageDataConvert(c24strip->GetBegin(wdata),
						c24strip->GetEnd(wdata),
						G42GreenAdjustFunction<G42Color24>(Map));
					break;
				}
				case Blue:
				{
					G42ImageDataConvert(c24strip->GetBegin(wdata),
						c24strip->GetEnd(wdata),
						G42BlueAdjustFunction<G42Color24>(Map));
					break;
				}
			}
		}
		else if (dynamic_cast<G42ImageDataColor32 *>(strip))
		{
			G42WriteImageData wdata(*strip);
			G42ImageDataColor32 * c32strip =
				dynamic_cast<G42ImageDataColor32 *>(strip);
			switch (Channel)
			{
				case All:
				{
					G42ImageDataConvert(c32strip->GetBegin(wdata),
						c32strip->GetEnd(wdata),
						G42ColorAdjustFunction<G42Color32>(Map, SatMap, (G42Color32 *)0));
					break;
				}
				case Red:
				{
					G42ImageDataConvert(c32strip->GetBegin(wdata),
						c32strip->GetEnd(wdata),
						G42RedAdjustFunction<G42Color32>(Map));
					break;
				}
				case Green:
				{
					G42ImageDataConvert(c32strip->GetBegin(wdata),
						c32strip->GetEnd(wdata),
						G42GreenAdjustFunction<G42Color32>(Map));
					break;
				}
				case Blue:
				{
					G42ImageDataConvert(c32strip->GetBegin(wdata),
						c32strip->GetEnd(wdata),
						G42BlueAdjustFunction<G42Color32>(Map));
					break;
				}
			}
		}
	}
	if (strip)
	{
		Receiver->Process(strip, this);
	}
#endif	
}
void
G42ImageColorAdjust::HandleEnd(G42PathEnd * end)
{
	SendOutputData(); // be sure to clear out any remaining data
	G42ImageFilter::HandleEnd(end);
}
G42ImageFilter *
G42ImageColorAdjust::MakeCopy(void) const
{
	return new G42ImageColorAdjust(Brightness, Contrast, Gamma, Channel);
}
G42Color
G42ImageColorAdjust::AdjustColor(const G42Color & c)
{
	G42Color c_out;
	if (Channel == All)
	{
		G42ColorAdjustFunction<G42Color> adjust(Map, SatMap, (G42Color *)0);
		c_out = adjust(c);
	}
	else if (Channel == Red)
	{
		G42RedAdjustFunction<G42Color> adjust(Map);
		c_out = adjust(c);
	}
	else if (Channel == Green)
	{
		G42GreenAdjustFunction<G42Color> adjust(Map);
		c_out = adjust(c);
	}
	else if (Channel == Blue)
	{
		G42BlueAdjustFunction<G42Color> adjust(Map);
		c_out = adjust(c);
	}
	return c_out;
}

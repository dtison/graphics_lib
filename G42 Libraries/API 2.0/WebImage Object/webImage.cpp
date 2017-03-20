#include "webimage.h"
G42WebImage::G42WebImage(void) :
	TextMode(false),
	MapMode(false),
	Image(0),
   UndoImage(0),
	PreviewImage(0),
	SelectedImage(0),
	UndoSelectedImage(0),
	View(0),
	ImageMap(0),
	TextObject(0),
	InUndo(0)
{
	ImageMap = new G42ImageMap();
	TextObject = new G42Text();
}
G42WebImage::~G42WebImage(void)
{
	if (ImageMap)
	{
		delete ImageMap;
		ImageMap = 0;
	}
	if (TextObject)
	{
		delete TextObject;
		TextObject = 0;
	}
	if (UndoImage)
	{
		delete UndoImage;
		UndoImage = 0;
	}
	if (UndoSelectedImage)
	{
		delete UndoSelectedImage;
		UndoSelectedImage = 0;
	}
	if (PreviewImage)
	{
		delete PreviewImage;
		PreviewImage = 0;
	}
	//This class does not own Image or View so do not delete here
}
void
G42WebImage::DeleteUndoPreview(void)
{
	if (UndoImage)
	{
		delete UndoImage;
		UndoImage = 0;
	}
	if (UndoSelectedImage)
	{
		delete UndoSelectedImage;
		UndoSelectedImage = 0;
	}
	if (PreviewImage)
	{
		delete PreviewImage;
		PreviewImage = 0;
	}
	// clear out the view, image, and selected image.  We're going
	// down anyway, no need to save off undo info
	View = 0;
	Image = 0;
	SelectedImage = 0;
}
void
G42WebImage::SetImage(G42Image * image)
{
	if (Image)
	{
		if (UndoImage)
		{
			delete UndoImage;
			UndoImage = 0;
		}
		if (UndoSelectedImage)
		{
			delete UndoSelectedImage;
			UndoSelectedImage = 0;
		}
		SelectedImage = 0;
#if 0
		if (UndoImage)
			*UndoImage = *Image;
		else
			UndoImage = new G42Image(*Image);
		if (SelectedImage)
		{
			if (UndoSelectedImage)
				*UndoSelectedImage = *SelectedImage;
			else
				UndoSelectedImage = new G42Image(*SelectedImage);
		}
		else if (UndoSelectedImage)
		{
			delete UndoSelectedImage;
			UndoSelectedImage = 0;
		}
		SelectedImage = 0;
#endif
	}
	Image = image;
}
void
G42WebImage::SetSelectedImage(G42Image * image)
{
	if (InUndo)
	{
		SelectedImage = image;
		return;
	}
	if (!image)
	{
		if (SelectedImage)
		{
			if (UndoSelectedImage)
				*UndoSelectedImage = *SelectedImage;
			else
				UndoSelectedImage = new G42Image(*SelectedImage);
		}
		if (Image)
		{
			if (UndoImage && Image)
				*UndoImage = *Image;
			else
				UndoImage = new G42Image(*Image);
		}
	}
	else
	{
		if (SelectedImage)
		{
			if (UndoSelectedImage)
				*UndoSelectedImage = *SelectedImage;
			else
				UndoSelectedImage = new G42Image(*SelectedImage);
		}
		else if (UndoSelectedImage)
		{
			delete UndoSelectedImage;
			UndoSelectedImage = 0;
		}
		if (Image)
		{
			if (UndoImage)
				*UndoImage = *Image;
			else
				UndoImage = new G42Image(*Image);
		}
	}
	SelectedImage = image;
}
void
G42WebImage::SetView(G42SingleImageViewer * view)
{
	View = view;
	if (View)
		View->SetImageMap(ImageMap);
}
void
G42WebImage::SetTextMode(bool text_mode)
{
	TextMode = text_mode;
	if (View)
	{
		View->SetTextVisible(TextMode);
		if (TextMode)
		{
			if (!TextObject)
				TextObject = new G42Text();
			View->SetTextStyle(TextObject);
		}
	}
}
void
G42WebImage::SetMapMode(bool map_mode)
{
	MapMode = map_mode;
	if (View)
		View->SetImageMapVisible(MapMode);
}
bool
G42WebImage::GetImageChanged (void)
{
	if (Image && Image->IsDirty())
		return true;
	return false;
}
bool
G42WebImage::GetMapChanged (void)
{
	if (ImageMap && ImageMap->IsDirty())
		return true;
	return false;
}
bool
G42WebImage::OpenMap (const char * file_name)
{
	if (View)
		View->Draw();
	if (ImageMap)
	{
		delete ImageMap;
		ImageMap = 0;
	}
	ImageMap = new G42ImageMap(file_name);
	if (ImageMap && ImageMap->IsValid())
	{
		if (View)
			View->SetImageMap(ImageMap);
		return true;
	}
	if (ImageMap)
	{
		delete ImageMap;
		ImageMap = 0;
	}
   ImageMap = new G42ImageMap();
	if (View)
		View->SetImageMap(ImageMap);
	return false;
}
bool
G42WebImage::OpenMap (const char * buffer, uint size)
{
	if (View)
		View->Draw();
	if (ImageMap)
	{
		delete ImageMap;
		ImageMap = 0;
	}
	ImageMap = new G42ImageMap(buffer, size);
	if (ImageMap && ImageMap->IsValid())
	{
		if (View)
			View->SetImageMap(ImageMap);
		return true;
	}
	if (ImageMap)
	{
		delete ImageMap;
		ImageMap = 0;
	}
   ImageMap = new G42ImageMap();
	if (View)
		View->SetImageMap(ImageMap);
	return false;
}
bool
G42WebImage::SaveMap (const char * file_name, G42ImageMap::G42MapType type)
{
	if (ImageMap)
	{
		ImageMap->Save(file_name, type);
		return true;
	}
   return false;
}
bool
G42WebImage::SaveMap (char * buffer, uint size,
	G42ImageMap::G42MapType type, bool cr_lf)
{
	if (ImageMap)
	{
		ImageMap->Save(buffer, size, type);
		return true;
	}
   return false;
}
void
G42WebImage::DrawImageMapElement(uint index)
{
	if (View)
		View->Draw();
}
void
G42WebImage::ColorAdjust(int bright, int gamma, int contrast, int channel,
	bool final)
{
	G42Image * image;
	if (SelectedImage)
		image = SelectedImage;
	else
		image = Image;
	if (image)
	{
		if (final)
		{
			if (PreviewImage)
			{
				if (bright != PreviewBright || gamma != PreviewGamma ||
					contrast != PreviewContrast || channel != PreviewChannel)
				{
					ResetPreview();
					image->ColorAdjust(bright, gamma, contrast, channel);
					Refresh();
				}
				AcceptPreview();
			}
			else
			{
				ResetUndo();
				image->ColorAdjust(bright, gamma, contrast, channel);
				Refresh();
			}
		}
		else // not final
		{
			if (!PreviewImage ||
				bright != PreviewBright || gamma != PreviewGamma ||
				contrast != PreviewContrast || channel != PreviewChannel)
			{
				ResetPreview();
				image->ColorAdjust(bright, gamma, contrast, channel);
				Refresh();
				PreviewBright = bright;
				PreviewGamma = gamma;
				PreviewContrast = contrast;
				PreviewChannel = channel;
			}
		}
	}
}
void
G42WebImage::ColorReduction(int method, G42PaletteType pal_type,
	int color_cnt, bool final)
{
	G42Image * image;
	if (SelectedImage)
		image = SelectedImage;
	else
		image = Image;
	if (image)
	{
		if (final)
		{
			if (PreviewImage)
			{
				if (method != PreviewMethod || pal_type != PreviewPalType ||
					color_cnt != PreviewColorCnt)
				{
					ResetPreview();
					image->ColorReduce(method, pal_type, color_cnt);
					Refresh();
				}
				AcceptPreview();
			}
			else
			{
				ResetUndo();
				image->ColorReduce(method, pal_type, color_cnt);
				Refresh();
			}
		}
		else
		{
			if (!PreviewImage ||
				method != PreviewMethod || pal_type != PreviewPalType ||
				color_cnt != PreviewColorCnt)
			{
				ResetPreview();
				image->ColorReduce(method, pal_type, color_cnt);
				Refresh();
				PreviewMethod = (G42MethodType) method;	// MACOS MW wants explicit cast D. Ison
				PreviewPalType = pal_type;
				PreviewColorCnt = color_cnt;
			}
		}
	}
}
void
G42WebImage::Border(G42Color color, int width, bool final)
{
	if (Image)
	{
		if (final)
		{
			if (PreviewImage)
			{
				if (color.red != PreviewRed || color.green != PreviewGreen ||
            	color.blue != PreviewBlue || width != PreviewWidth)
				{
					ResetPreview();
					Image->AddBorder(color, width);
					Refresh();
				}
				AcceptPreview();
			}
			else
			{
				ResetUndo();
				Image->AddBorder(color, width);
				Refresh();
			}
		}
		else
		{
			if (!PreviewImage || color.red != PreviewRed ||
         	color.green != PreviewGreen || color.blue != PreviewBlue ||
            width != PreviewWidth)
			{
				ResetPreview();
				Image->AddBorder(color, width);
				Refresh();
				PreviewRed = color.red;
				PreviewGreen = color.green;
				PreviewBlue = color.blue;
				PreviewWidth = width;
			}
		}
	}
}
void
G42WebImage::Buttonize(G42Color top_color, G42Color bottom_color,
	G42ButtonType button_type, int width, bool has_color, bool final)
{
	if (Image)
	{
		if (final)
		{
			if (PreviewImage)
			{
				if (top_color.red != PreviewTopRed ||
            	top_color.green != PreviewTopGreen ||
            	top_color.blue != PreviewTopBlue ||
					bottom_color.red != PreviewBottomRed ||
            	bottom_color.green != PreviewBottomGreen ||
            	bottom_color.blue != PreviewBottomBlue ||
               width != PreviewWidth ||
               has_color != PreviewHasColor ||
  					button_type != PreviewButtonType)
				{
					ResetPreview();
					Image->Buttonize(top_color, bottom_color, button_type, width,
               	has_color);
					Refresh();
				}
				AcceptPreview();
			}
			else
			{
				ResetUndo();
				Image->Buttonize(top_color, bottom_color, button_type, width,
            	has_color);
				Refresh();
			}
		}
		else
		{
			if (!PreviewImage || top_color.red != PreviewTopRed ||
         	top_color.green != PreviewTopGreen ||
            top_color.blue != PreviewTopBlue ||
            bottom_color.red != PreviewBottomRed ||
            bottom_color.green != PreviewBottomGreen ||
            bottom_color.blue != PreviewBottomBlue ||
				button_type != PreviewButtonType ||
            width != PreviewWidth ||
            has_color != PreviewHasColor)
			{
				ResetPreview();
				Image->Buttonize(top_color, bottom_color, button_type, width,
            	has_color);
				Refresh();
				PreviewTopRed = top_color.red;
				PreviewTopGreen = top_color.green;
				PreviewTopBlue = top_color.blue;
				PreviewBottomRed = bottom_color.red;
				PreviewBottomGreen = bottom_color.green;
				PreviewBottomBlue = bottom_color.blue;
				PreviewButtonType = button_type;
				PreviewWidth = width;
				PreviewHasColor = has_color;
			}
		}
	}
}
void
G42WebImage::ResizeImage(int current_width, int current_height,
	bool final)
{
	G42Image * image;
	if (SelectedImage)
		image = SelectedImage;
	else
		image = Image;
	if (image)
	{
		if (final)
		{
			if (PreviewImage)
			{
				if (current_width != PreviewWidth || current_height != PreviewHeight)
				{
					ResetPreview();
					image->ResizeImage(current_width, current_height);
					Refresh();
				}
				AcceptPreview();
			}
			else
			{
				ResetUndo();
				image->ResizeImage(current_width, current_height);
				Refresh();
			}
		}
		else
		{
			if (!PreviewImage ||
				current_width != PreviewWidth || current_height != PreviewHeight)
			{
				ResetPreview();
				image->ResizeImage(current_width, current_height);
				Refresh();
				PreviewWidth = current_width;
				PreviewHeight = current_height;
			}
		}
	}
}
void
G42WebImage::DrawColorMask(byte * mask, int width, int height, G42Color color,
   	int x, int y)
{
	G42Image * image;
	if (SelectedImage)
		image = SelectedImage;
	else
		image = Image;
	if (image)
	{
		ResetUndo();
		image->DrawColorMask(mask, width, height, color, x, y);
		Refresh();
	}
}
void
G42WebImage::DrawColorMask8(byte * mask, int width, int height, G42Color color,
   	int x, int y)
{
	G42Image * image;
	if (SelectedImage)
		image = SelectedImage;
	else
		image = Image;
	if (image)
	{
		ResetUndo();
		image->DrawColorMask8(mask, width, height, color, x, y);
		Refresh();
	}
}
void
G42WebImage::Invert()
{
	G42Image * image;
	if (SelectedImage)
		image = SelectedImage;
	else
		image = Image;
	if (image)
	{
		ResetUndo();
		image->Invert();
		Refresh();
	}
}
void
G42WebImage::SwapRB()
{
	G42Image * image;
	if (SelectedImage)
		image = SelectedImage;
	else
		image = Image;
	if (image)
	{
		ResetUndo();
		image->SwapRB();
		Refresh();
	}
}
void
G42WebImage::Equalize()
{
	G42Image * image;
	if (SelectedImage)
		image = SelectedImage;
	else
		image = Image;
	if (image)
	{
		ResetUndo();
		image->Equalize();
		Refresh();
	}
}
void
G42WebImage::Refresh(void)
{
	if (View)
	{
		View->Rebuild();
		#ifndef MACOS
		View->Draw();
		#endif
	}
}
void
G42WebImage::Emboss(int angle, bool final)
{
	G42Image * image;
	if (SelectedImage)
		image = SelectedImage;
	else
		image = Image;
	if (image)
	{
		if (final)
		{
			if (PreviewImage)
			{
				if (angle != PreviewAngle)
				{
					ResetPreview();
					image->Emboss(angle);
					Refresh();
				}
				AcceptPreview();
			}
			else
			{
				ResetUndo();
				image->Emboss(angle);
				Refresh();
			}
		}
		else // not final
		{
			if (!PreviewImage || angle != PreviewAngle)
			{
				ResetPreview();
				image->Emboss(angle);
				Refresh();
				PreviewAngle = angle;
			}
		}
	}
}
void
G42WebImage::Sharpness(int sharp, bool final)
{
	G42Image * image;
	if (SelectedImage)
		image = SelectedImage;
	else
		image = Image;
	if (image)
	{
		if (final)
		{
			if (PreviewImage)
			{
				if (sharp != PreviewSharp)
				{
					ResetPreview();
					image->Sharpness(sharp);
					Refresh();
				}
				AcceptPreview();
			}
			else
			{
				ResetUndo();
				image->Sharpness(sharp);
				Refresh();
			}
		}
		else // not final
		{
			if (!PreviewImage || sharp != PreviewSharp)
			{
				ResetPreview();
				image->Sharpness(sharp);
				Refresh();
				PreviewSharp = sharp;
			}
		}
	}
}
void
G42WebImage::RemoveNoise(int noise, bool final)
{
	G42Image * image;
	if (SelectedImage)
		image = SelectedImage;
	else
		image = Image;
	if (image)
	{
		if (final)
		{
			if (PreviewImage)
			{
				if (noise != PreviewNoise)
				{
					ResetPreview();
					image->RemoveNoise(noise);
					Refresh();
				}
				AcceptPreview();
			}
			else
			{
				ResetUndo();
				image->RemoveNoise(noise);
				Refresh();
			}
		}
		else // not final
		{
			if (!PreviewImage || noise != PreviewNoise)
			{
				ResetPreview();
				image->RemoveNoise(noise);
				Refresh();
				PreviewNoise = noise;
			}
		}
	}
}
void
G42WebImage::MirrorX(void)
{
	G42Image * image;
	if (SelectedImage)
		image = SelectedImage;
	else
		image = Image;
	if (image)
	{
		ResetUndo();
		image->MirrorX();
		Refresh();
	}
}
void
G42WebImage::MirrorY(void)
{
	G42Image * image;
	if (SelectedImage)
		image = SelectedImage;
	else
		image = Image;
	if (image)
	{
		ResetUndo();
		image->MirrorY();
		Refresh();
	}
}
void
G42WebImage::Rotate(int angle)
{
	G42Image * image;
	if (SelectedImage)
		image = SelectedImage;
	else
		image = Image;
	if (image)
	{
		ResetUndo();
		image->Rotate(angle);
		Refresh();
	}
}
void
G42WebImage::EnsureColorCount(void)
{
	if (!Image)
   	return;
	Image->CheckColorCount();
}
void
G42WebImage::Crop(void)
{
	if (Image && SelectedImage)
	{
		if (UndoImage)
			*UndoImage = *Image;
		else
			UndoImage = new G42Image(*Image);
		if (UndoSelectedImage)
			*UndoSelectedImage = *SelectedImage;
		else
			UndoSelectedImage = new G42Image(*SelectedImage);
		*Image = *SelectedImage;
		InUndo++;
		if (View)
			View->DeleteSelectedImage();
		InUndo--;
		Refresh();
	}
}
void
G42WebImage::NewImage(G42Color top_color, G42Color middle_color, G42Color bottom_color,
	uint32 width, uint32 height, int size, int rows, int columns, bool final)
{
	if (((int32)width <= 0) || ((int32)height <= 0))
   	return;
	G42Image * image;
	if (SelectedImage)
		image = SelectedImage;
	else
		image = Image;
	if (final)
	{
		if (PreviewImage)
		{
			CreateNewImage(top_color, middle_color, bottom_color, width, height,
           	size, rows, columns);
			Refresh();
			AcceptPreview();
		}
		else
		{
//			ResetUndo();	Don't need this
			CreateNewImage(top_color, middle_color, bottom_color, width, height,
           	size, rows, columns);
			Refresh();
		}
	}
	else // not final
	{
		if (!PreviewImage)
			ResetPreview();
		CreateNewImage(top_color, middle_color, bottom_color, width, height,
        	size, rows, columns);
		Refresh();
	}
}
void
G42WebImage::NewImage(G42Image * image_in, bool final)
{
	G42Image * image;
	if (SelectedImage)
		image = SelectedImage;
	else
		image = Image;
	if (final)
	{
		if (PreviewImage)
		{
			PasteNewImage(image_in);
			Refresh();
			AcceptPreview();
		}
		else
		{
//			ResetUndo();	// Don't need this
			PasteNewImage(image_in);
			Refresh();
		}
	}
	else // not final
	{
		if (!PreviewImage)
			ResetPreview();
		PasteNewImage(image_in);
		Refresh();
	}
}
void
G42WebImage::CreateNewImage(G42Color top_color, G42Color middle_color, G42Color bottom_color,
	uint32 width, uint32 height, int size, int rows, int columns)
{
	G42ImageInfo info;
   info.Width = width;
   info.Height = height;
   info.PixelDepth = 4;
   info.ColorType = G42ImageInfo::ColorTypePalette;
   info.NumPalette = 1;
   G42Color * pal = new G42Color [info.NumPalette];
   pal[0].red = pal[0].green = pal[0].blue = 0;
   info.ImagePalette.Set(pal);
   info.ImageType = Unknown;
   Image->SetInfo(info);
   Image->FloodFill(top_color, middle_color, bottom_color, size, rows, columns);
}
void
G42WebImage::PasteNewImage(G42Image * image)
{
	if (!image)
   	return;
	*Image = *image;
   delete image;
   Refresh();
}
void
G42WebImage::SelectAll(void)
{
	if (View && Image && Image->IsFinished())
		View->CreateSelectedImage(0, 0, Image->GetInfo().Width,
			Image->GetInfo().Height);
}
void
G42WebImage::NewMap(void)
{
	if (View)
		View->Draw();
	if (ImageMap)
	{
		delete ImageMap;
		ImageMap = 0;
	}
	ImageMap = new G42ImageMap();
	if (ImageMap && ImageMap->IsValid())
	{
		if (View)
			View->SetImageMap(ImageMap);
	}
}
void
G42WebImage::SetJustification (G42Text::G42Justification justification)
{
	if (!TextObject)
		TextObject = new G42Text();
	TextObject->SetJustification(justification);
	if (View)
		View->TextStyleUpdated();
}
void
G42WebImage::SetDirection (G42Text::G42Direction direction)
{
	if (!TextObject)
		TextObject = new G42Text();
	TextObject->SetDirection(direction);
	if (View)
		View->TextStyleUpdated();
}
void
G42WebImage::SetColor (G42Color color)
{
	if (!TextObject)
		TextObject = new G42Text();
	TextObject->SetColor(color);
	if (View)
		View->TextStyleUpdated();
}
void
G42WebImage::SetAntiAlias (bool anti_alias)
{
	if (!TextObject)
		TextObject = new G42Text();
	TextObject->SetAntiAlias(anti_alias);
	if (View)
		View->TextStyleUpdated();
}
void
G42WebImage::SetFontName (const char * typeface_name)
{
	if (!TextObject)
		TextObject = new G42Text();
	TextObject->SetFontName(typeface_name);
	if (View)
		View->TextStyleUpdated();
}
void
G42WebImage::SetPointSize (int point_size)
{
	if (!TextObject)
		TextObject = new G42Text();
	TextObject->SetPointSize(point_size);
	if (View)
		View->TextStyleUpdated();
}
void
G42WebImage::SetBold (bool bold)
{
	if (!TextObject)
		TextObject = new G42Text();
	TextObject->SetBold(bold);
	if (View)
		View->TextStyleUpdated();
}
void
G42WebImage::SetItalic (int italic)
{
	if (!TextObject)
		TextObject = new G42Text();
	TextObject->SetItalic(italic);
	if (View)
		View->TextStyleUpdated();
}
void
G42WebImage::SetUnderline (int underline)
{
	if (!TextObject)
		TextObject = new G42Text();
	TextObject->SetUnderline(underline);
	if (View)
		View->TextStyleUpdated();
}
void
G42WebImage::SetFontAttributes (const char * typeface_name, int point_size,
	bool bold, bool italic, bool underline, G42Color color)
{
	if (!TextObject)
		TextObject = new G42Text();
	TextObject->SetFontAttributes(typeface_name, point_size, bold, italic,
		underline, color);
	if (View)
		View->TextStyleUpdated();
}
void
G42WebImage::SetTransparentMode(bool mode)
{
	View->SetTransparentMode(mode);
}
bool
G42WebImage::GetTransparentMode(void)
{
	return View->GetTransparentMode();
}
void
G42WebImage::ClearTransparency(bool final)
{
	G42Image * image;
	if (SelectedImage)
		image = SelectedImage;
	else
		image = Image;
	if (image)
	{
		if (final)
		{
			if (PreviewImage)
			{
				if (PreviewTransColorValid)
				{
					ResetPreview();
					image->ClearTransparency();
					Refresh();
				}
				AcceptPreview();
			}
			else
			{
				ResetUndo();
				image->ClearTransparency();
				Refresh();
			}
		}
		else // not final
		{
			if (!PreviewImage || PreviewTransColorValid)
			{
				ResetPreview();
				image->ClearTransparency();
				Refresh();
				PreviewTransColorValid = false;
			}
		}
	}
}
void
G42WebImage::SetTransparency(G42Color color, bool final)
{
	G42Image * image;
	if (SelectedImage)
		image = SelectedImage;
	else
		image = Image;
	if (image)
	{
		if (final)
		{
			if (PreviewImage)
			{
				if (!PreviewTransColorValid ||
					PreviewTransColor.red != color.red ||
					PreviewTransColor.green != color.green ||
					PreviewTransColor.blue != color.blue)
				{
					ResetPreview();
					image->SetTransparency(color);
					Refresh();
				}
				AcceptPreview();
			}
			else
			{
				ResetUndo();
				image->SetTransparency(color);
				Refresh();
			}
		}
		else // not final
		{
			if (!PreviewImage || !PreviewTransColorValid ||
				PreviewTransColor.red != color.red ||
				PreviewTransColor.green != color.green ||
				PreviewTransColor.blue != color.blue)
			{
				ResetPreview();
				image->SetTransparency(color);
				Refresh();
				PreviewTransColor = color;
				PreviewTransColorValid = true;
			}
		}
	}
}
G42Color
G42WebImage::GetColorAtPoint(uint32 x, uint32 y)
{
	if (Image)
		return Image->GetColorAtPoint(x, y);
}
void
G42WebImage::ZoomIn(void)
{
	if (Image && View)
	{
		View->SetZoom(View->GetZoom() * 2.0);
		#ifndef MACOS
		View->Draw();
		#endif
	}
}
void
G42WebImage::ZoomOut(void)
{
	if (Image && View)
	{
		View->SetZoom(View->GetZoom() / 2.0);
		#ifndef MACOS
		View->Draw();
		#endif
	}
}
void
G42WebImage::Unzoom(void)
{
	if (Image && View)
	{
		View->SetZoom(1.0);
		#ifndef MACOS
		View->Draw();
		#endif
	}
}
void
G42WebImage::FitWidth(void)
{
	if (View)
	{
		View->SetZoomFitWidth();
		#ifndef MACOS
		View->Draw();
		#endif
	}
}
void
G42WebImage::FitAll(void)
{
	if (View)
	{
		View->SetZoomFitAll();
		#ifndef MACOS
		View->Draw();
		#endif
	}
}
void
G42WebImage::Undo(void) // reverts back to the last ResetUndo()
{
	bool need_redraw = false;
	if (UndoImage && Image)
	{
		*Image = *UndoImage;
		delete UndoImage;
      UndoImage = 0;
		need_redraw = true;
	}
	
	if (UndoSelectedImage && SelectedImage)
	{
		*SelectedImage = *UndoSelectedImage;
		delete UndoSelectedImage;
		UndoSelectedImage = 0;
		need_redraw = true;
	}
	else if (UndoSelectedImage && !SelectedImage)
	{
		InUndo++;
		if (View)
			View->AddSelectedImage(UndoSelectedImage);
		InUndo--;
		delete UndoSelectedImage;
		UndoSelectedImage = 0;
		need_redraw = true;
	}
	else if (!UndoSelectedImage && SelectedImage)
	{
		InUndo++;
		if (View)
			View->DeleteSelectedImage();
		InUndo--;
		need_redraw = true;
	}
	
	if (need_redraw)
	{
		View->Rebuild();
		#ifndef MACOS
		View->Draw();
		#endif
	}
}
bool
G42WebImage::CanUndo(void) // true if there is an undo image
{
	if (UndoImage || UndoSelectedImage)
		return true;
	return false;
}
void
G42WebImage::ResetUndo(void) // saves off the current image for undo
{
	if (!Image)
		return;
	if (SelectedImage)
	{
		if (UndoImage)
		{
			delete UndoImage;
			UndoImage = 0;
		}
		if (UndoSelectedImage)
			*UndoSelectedImage = *SelectedImage;
		else
			UndoSelectedImage = new G42Image(*SelectedImage);
	}
	else
	{
		if (UndoSelectedImage)
		{
			delete UndoSelectedImage;
			UndoSelectedImage = 0;
		}
		if (Image)
		{
			if (UndoImage)
				*UndoImage = *Image;
			else
				UndoImage = new G42Image(*Image);
		}
	}
}
void
G42WebImage::ResetPreview(void) // sets up or resets the preview image
{
	if (!Image)
		return;
	if (SelectedImage)
	{
		if (PreviewImage)
			*SelectedImage = *PreviewImage;
		else
			PreviewImage = new G42Image(*SelectedImage);
	}
	else
	{
		if (PreviewImage)
			*Image = *PreviewImage;
		else
			PreviewImage = new G42Image(*Image);
	}
}
void
G42WebImage::AcceptPreview(void) // makes the preview image the real image
{
	if (UndoImage)
	{
		delete UndoImage;
		UndoImage = 0;
	}
	if (UndoSelectedImage)
	{
		delete UndoSelectedImage;
		UndoSelectedImage = 0;
	}
	if (SelectedImage)
		UndoSelectedImage = PreviewImage;
	else
		UndoImage = PreviewImage;
	PreviewImage = 0;
}
void
G42WebImage::CancelPreview(void) // reverts back to the original image
{
	if (PreviewImage)
	{
		if (SelectedImage)
			*SelectedImage = *PreviewImage;
		else
			*Image = *PreviewImage;
		delete PreviewImage;
		PreviewImage = 0;
		View->Rebuild();
		#ifndef MACOS
		View->Draw();
		#endif
	}
}

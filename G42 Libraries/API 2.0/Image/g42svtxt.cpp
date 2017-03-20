
#include <g42itype.h>
#include <g42sview.h>
#include <g42image.h>
#ifdef MSWIN
#include <osl\geometry.h>
#endif
void
G42SingleImageViewer::SetTextVisible(bool visible)
{
	if (TextVisible && !visible)
   	AcceptText();
	TextVisible = visible;
	if (TextVisible)
		SetCursor(TextCursor);
	else
		SetCursor(ArrowCursor);
}
void
G42SingleImageViewer::SetTextStyle(G42Text * text)
{
	TextStyle = text;
}
void
G42SingleImageViewer::TextStyleUpdated(void)
{
	if (!TextStarted)
		return;
	EraseText();
	UpdateTextPos();
	DrawText();
}
void
G42SingleImageViewer::SetTextStartPos(int x, int y)
{
	AcceptText();
	TextStarted = true;
	CursorTextPos = 0;
   NumTextArray = 0;
	StartX = x;
	StartY = y;
	TextPosX[0] = x;
	TextPosY[0] = y;
   UpdateTextPos();
	DrawCursor();
}
// NEED THIS
void
G42SingleImageViewer::Keystroke(int key)
{
	if (!TextStarted)
		return;
	if (key == KeyReturn)
		key = '\n';
	if (key < 256)
	{
		if (TextStyle->GetJustification() == G42Text::Left &&
			CursorTextPos == NumTextArray)
				DrawCursor();
		else
			EraseText();
		if (CursorTextPos < NumTextArray)
		{
			for (int i = NumTextArray; i > CursorTextPos; i--)
				TextArray[i] = TextArray[i - 1];
		}
		TextArray[CursorTextPos] = (char)key;
		CursorTextPos++;
		NumTextArray++;
		UpdateTextPos();
		DrawText();
	}
	else
	{
   	// translate from the user orientation to the text orientation
		if (TextStyle->GetDirection() == G42Text::VerticalUp)
		{
			if (key == KeyLeft)
				key = KeyUp;
			else if (key == KeyRight)
				key = KeyDown;
			else if (key == KeyDown)
				key = KeyLeft;
			else if (key == KeyUp)
				key = KeyRight;
      }
		else if (TextStyle->GetDirection() == G42Text::VerticalDown)
		{
			if (key == KeyLeft)
				key = KeyDown;
			else if (key == KeyRight)
				key = KeyUp;
			else if (key == KeyDown)
				key = KeyRight;
			else if (key == KeyUp)
				key = KeyLeft;
      }
		switch (key)
		{
			case KeyLeft:
			{
				if (CursorTextPos)
				{
					DrawCursor();
					CursorTextPos--;
					DrawCursor();
				}
				break;
			}
			case KeyRight:
			{
				if (CursorTextPos < NumTextArray)
				{
					DrawCursor();
					CursorTextPos++;
					DrawCursor();
				}
				break;
			}
			case KeyHome:
			{
				if (CursorTextPos)
				{
					DrawCursor();
					CursorTextPos = 0;
					DrawCursor();
				}
				break;
			}
			case KeyEnd:
			{
				if (CursorTextPos < NumTextArray)
				{
					DrawCursor();
					CursorTextPos = NumTextArray;
					DrawCursor();
				}
				break;
			}
			case KeyUp:
			{
				if (!CursorTextPos)
					break;
				int cur = CursorTextPos;
				if (TextArray[cur] == '\n')
            	cur--;
				while (cur && TextArray[cur] != '\n')
					cur--;
				if (TextArray[cur] != '\n') // already on first line
					break;
				if (!cur || TextArray[cur - 1] == '\n')
				{
					DrawCursor();
					CursorTextPos = cur;
					DrawCursor();
					break;
				}
				cur--;
				DrawCursor();
				switch (TextStyle->GetDirection())
				{
					case G42Text::Horizontal:
					{
						while (cur && TextArray[cur] != '\n' &&
							TextPosX[cur] > TextPosX[CursorTextPos])
								cur--;
						if (abs(TextPosX[cur + 1] - TextPosX[CursorTextPos]) <
							abs(TextPosX[CursorTextPos] - TextPosX[cur]))
								cur++;
						CursorTextPos = cur;
						break;
					}
					case G42Text::VerticalDown:
					{
						while (cur && TextArray[cur] != '\n' &&
							TextPosY[cur] > TextPosY[CursorTextPos])
								cur--;
						if (abs(TextPosY[cur + 1] - TextPosY[CursorTextPos]) <
							abs(TextPosY[CursorTextPos] - TextPosY[cur]))
								cur++;
						CursorTextPos = cur;
						break;
					}
					case G42Text::VerticalUp:
					{
						while (cur && TextArray[cur] != '\n' &&
							TextPosY[cur] < TextPosY[CursorTextPos])
								cur--;
						if (abs(TextPosY[cur] - TextPosY[CursorTextPos]) >
							abs(TextPosY[CursorTextPos] - TextPosY[cur + 1]))
								cur++;
						CursorTextPos = cur;
						break;
					}
				}
				DrawCursor();
				break;
			}
			case KeyDown:
			{
				int cur = CursorTextPos;
				while (cur < NumTextArray && TextArray[cur] != '\n')
					cur++;
				if (TextArray[cur] != '\n') // already on last line
					break;
				if (cur == NumTextArray || TextArray[cur + 1] == '\n')
				{
					DrawCursor();
					CursorTextPos = cur;
					DrawCursor();
					break;
				}
				cur++;
				DrawCursor();
				switch (TextStyle->GetDirection())
				{
					case G42Text::Horizontal:
					{
						while (cur < NumTextArray && TextArray[cur] != '\n' &&
							TextPosX[cur] < TextPosX[CursorTextPos])
								cur++;
						if (abs(TextPosX[cur] - TextPosX[CursorTextPos]) >
							abs(TextPosX[CursorTextPos] - TextPosX[cur - 1]))
								cur--;
						CursorTextPos = cur;
						break;
					}
					case G42Text::VerticalDown:
					{
						while (cur < NumTextArray && TextArray[cur] != '\n' &&
							TextPosY[cur] < TextPosY[CursorTextPos])
								cur++;
						if (abs(TextPosY[cur] - TextPosY[CursorTextPos]) >
							abs(TextPosY[CursorTextPos] - TextPosY[cur - 1]))
								cur--;
						CursorTextPos = cur;
						break;
					}
					case G42Text::VerticalUp:
					{
						while (cur < NumTextArray && TextArray[cur] != '\n' &&
							TextPosY[cur] > TextPosY[CursorTextPos])
								cur++;
						if (abs(TextPosY[cur - 1] - TextPosY[CursorTextPos]) <
							abs(TextPosY[CursorTextPos] - TextPosY[cur]))
								cur--;
						CursorTextPos = cur;
						break;
					}
				}
				DrawCursor();
				break;
			}
			case KeyBackspace:
			{
				if (!NumTextArray || !CursorTextPos)
					break;
				EraseText();
				for (int i = CursorTextPos - 1; i < NumTextArray - 1; i++)
					TextArray[i] = TextArray[i + 1];
				NumTextArray--;
				CursorTextPos--;
				UpdateTextPos();
				DrawText();
				break;
			}
			case KeyDelete:
			{
				if (!NumTextArray || CursorTextPos == NumTextArray)
					break;
				EraseText();
				for (int i = CursorTextPos; i < NumTextArray - 1; i++)
					TextArray[i] = TextArray[i + 1];
				NumTextArray--;
				UpdateTextPos();
				DrawText();
				break;
			}
		}
	}
}
void
G42SingleImageViewer::UpdateTextPos()
{
	if (!TextStarted)
		return;
	if (TextStyle && NumTextArray)
	{
		const char * line = TextArray;
//		y = text_start_y + (text_orientation ? 0 : text_cursor_descent);
//		x = text_start_x + (text_orientation * text_cursor_descent);
		int x = StartX;
		int y = StartY;
		TextPosX[0] = x;
		TextPosY[0] = y;
		int start_i = 0;
		int len = 1;
		int i = 1;
		const char * p = TextArray;
		for (; i <= NumTextArray; p++, i++, len++)
		{
			if (*p == '\n')
			{
				if (TextStyle->GetJustification() == G42Text::Right)
				{
					switch (TextStyle->GetDirection())
					{
						case G42Text::Horizontal:
						{
							int offset = TextPosX[i - 1] - TextPosX[start_i];
							for (int j = start_i; j < i; j++)
								TextPosX[j] -= offset;
							break;
						}
						case G42Text::VerticalUp:
						{
							int offset = TextPosY[start_i] - TextPosY[i - 1];
							for (int j = start_i; j < i; j++)
								TextPosY[j] += offset;
							break;
						}
						case G42Text::VerticalDown:
						{
							int offset = TextPosY[i - 1] - TextPosY[start_i];
							for (int j = start_i; j < i; j++)
								TextPosY[j] -= offset;
							break;
						}
					}
				}
				else if (TextStyle->GetJustification() == G42Text::Center)
				{
					switch (TextStyle->GetDirection())
					{
						case G42Text::Horizontal:
						{
							int offset = ((TextPosX[i - 1] - TextPosX[start_i]) >> 1);
							for (int j = start_i; j < i; j++)
								TextPosX[j] -= offset;
							break;
						}
						case G42Text::VerticalUp:
						{
							int offset = ((TextPosY[start_i] - TextPosY[i - 1]) >> 1);
							for (int j = start_i; j < i; j++)
								TextPosY[j] += offset;
							break;
						}
						case G42Text::VerticalDown:
						{
							int offset = ((TextPosY[i - 1] - TextPosY[start_i]) >> 1);
							for (int j = start_i; j < i; j++)
								TextPosY[j] -= offset;
							break;
						}
					}
				}
				line = p + 1;
				len = 0;
				switch (TextStyle->GetDirection())
				{
					case G42Text::Horizontal:
						y += TextStyle->GetFontLineSpacing();
						break;
					case G42Text::VerticalDown:
						x -= TextStyle->GetFontLineSpacing();
						break;
					case G42Text::VerticalUp:
						x += TextStyle->GetFontLineSpacing();
						break;
				}
				start_i = i;
				TextPosX[i] = x;
				TextPosY[i] = y;
			}
			else
			{
				int w, h, char_w;
				GetTextExtents(line, len, &w, &h);
//				if (text_extent_direction == TEXT_EXTENT_HORIZONTAL)
					char_w = w;
//				else
//					char_w = h;
				switch (TextStyle->GetDirection())
				{
					case G42Text::Horizontal:
						TextPosX[i] = x + char_w;
						TextPosY[i] = y;
						break;
					case G42Text::VerticalDown:
						TextPosX[i] = x;
						TextPosY[i] = y + char_w;
						break;
					case G42Text::VerticalUp:
						TextPosX[i] = x;
						TextPosY[i] = y - char_w;
						break;
				}
			}
		}
		if (TextStyle->GetJustification() == G42Text::Right)
		{
			switch (TextStyle->GetDirection())
			{
				case G42Text::Horizontal:
				{
					int offset = TextPosX[i - 1] - TextPosX[start_i];
					for (int j = start_i; j < i; j++)
						TextPosX[j] -= offset;
					break;
				}
				case G42Text::VerticalUp:
				{
					int offset = TextPosY[start_i] - TextPosY[i - 1];
					for (int j = start_i; j < i; j++)
						TextPosY[j] += offset;
					break;
				}
				case G42Text::VerticalDown:
				{
					int offset = TextPosY[i - 1] - TextPosY[start_i];
					for (int j = start_i; j < i; j++)
						TextPosY[j] -= offset;
					break;
				}
			}
		}
		else if (TextStyle->GetJustification() == G42Text::Center)
		{
			switch (TextStyle->GetDirection())
			{
				case G42Text::Horizontal:
				{
					int offset = ((TextPosX[i - 1] - TextPosX[start_i]) >> 1);
					for (int j = start_i; j < i; j++)
						TextPosX[j] -= offset;
					break;
				}
				case G42Text::VerticalUp:
				{
					int offset = ((TextPosY[start_i] - TextPosY[i - 1]) >> 1);
					for (int j = start_i; j < i; j++)
						TextPosY[j] += offset;
					break;
				}
				case G42Text::VerticalDown:
				{
					int offset = ((TextPosY[i - 1] - TextPosY[start_i]) >> 1);
					for (int j = start_i; j < i; j++)
						TextPosY[j] -= offset;
					break;
				}
			}
		}
		int min_x = TextPosX[0];
		int max_x = TextPosX[0];
		int min_y = TextPosY[0];
		int max_y = TextPosY[0];
		for (i = 1; i <= NumTextArray; i++)
		{
			if (TextPosX[i] < min_x)
				min_x = TextPosX[i];
			if (TextPosX[i] > max_x)
				max_x = TextPosX[i];
			if (TextPosY[i] < min_y)
				min_y = TextPosY[i];
			if (TextPosY[i] > max_y)
				max_y = TextPosY[i];
		}
		switch (TextStyle->GetDirection())
		{
			case G42Text::Horizontal:
				min_y -= TextStyle->GetFontAscent();
				max_y += TextStyle->GetFontDescent();
				break;
			case G42Text::VerticalDown:
				min_x -= TextStyle->GetFontDescent();
				max_x += TextStyle->GetFontAscent();
				break;
			case G42Text::VerticalUp:
				min_x -= TextStyle->GetFontAscent();
				max_x += TextStyle->GetFontDescent();
				break;
		}
		CurrentX = min_x;
		CurrentY = min_y;
		OtherX = max_x;
		OtherY = max_y;
	}
	else if (TextStyle)
	{
		TextPosX[0] = StartX;
		TextPosY[0] = StartY;
		int min_x = TextPosX[0];
		int max_x = TextPosX[0] + 1;
		int min_y = TextPosY[0];
		int max_y = TextPosY[0] + 1;
		switch (TextStyle->GetDirection())
		{
			case G42Text::Horizontal:
				min_y -= TextStyle->GetFontAscent();
				max_y += TextStyle->GetFontDescent();
				break;
			case G42Text::VerticalDown:
				min_x -= TextStyle->GetFontDescent();
				max_x += TextStyle->GetFontAscent();
				break;
			case G42Text::VerticalUp:
				min_x -= TextStyle->GetFontAscent();
				max_x += TextStyle->GetFontDescent();
				break;
		}
		CurrentX = min_x;
		CurrentY = min_y;
		OtherX = max_x;
		OtherY = max_y;
	}
}
// David: the basic idea here is that we need to make a monochrome
// mask to put the color into the image.
void
G42SingleImageViewer::AcceptText()
{
	if (TextStarted)
	{
		if (NumTextArray)
		{
#if defined(MSWIN)
			bool force = true;
			int bmp_width = OtherX - CurrentX + 1;
			int bmp_height = OtherY - CurrentY + 1;
			bmp_width = ((bmp_width + 31) & ~31); // expand to 4 byte boundries
			if (TextStyle->GetAntiAlias())
			{
            int old_point_size = TextStyle->GetPointSize();
            int scale = 0;
            if (old_point_size <= 24)
            	scale = 8;
            else if (old_point_size <= 48)
            	scale = 4;
            else
            	scale = 2;
            TextStyle->SetPointSize(old_point_size * scale);
            if (!TextStyle->CheckFont())
            {
            	TextStyle->SetPointSize(old_point_size);
            }
				else
            {
					byte * bmp_bits = (byte *)farmalloc(
						((((int32)(bmp_width) * scale) * ((int32)bmp_height * scale)) >> 3));
					memset(bmp_bits, 0,
               	((((int32)(bmp_width) * scale) * ((int32)bmp_height * scale)) >> 3));
					HBITMAP bmp = CreateBitmap(bmp_width * scale, bmp_height * scale, 1, 1, bmp_bits);
					HDC sdc = ::GetWindowDC(0);
					HDC dc = ::CreateCompatibleDC(sdc); // needs DeleteDC() when finished
					::ReleaseDC(0, sdc);
					HBITMAP old_bmp = (HBITMAP)SelectObject(dc, bmp);
					LOGPALETTE log_pal[2]; // to give us enough space
					PALETTEENTRY * text_pal = log_pal[0].palPalEntry;
					log_pal[0].palVersion = 0x300;
					log_pal[0].palNumEntries = 2;
					text_pal[0].peRed = text_pal[0].peGreen = text_pal[0].peBlue = 0;
					text_pal[0].peFlags = 0;
					text_pal[1].peRed = text_pal[1].peGreen = text_pal[1].peBlue = 255;
					text_pal[1].peFlags = 0;
					HPALETTE text_palette = CreatePalette(log_pal);
					HPALETTE old_palette = (HPALETTE)SelectObject(dc, text_palette);
					SetTextColor(dc, RGB(0xff, 0xff, 0xff));
					HFONT old_font = (HFONT)SelectObject(dc, TextStyle->GetFont());
					SetBkMode(dc, TRANSPARENT);
					SetTextAlign(dc, TA_LEFT | TA_BASELINE);
					char * ptr = TextArray;
					uint i = 0;
					while (i < NumTextArray)
					{
						int line_count = 0;
						char * lptr = ptr;
						while (i + line_count < NumTextArray && *lptr != '\n')
						{
							lptr++;
							line_count++;
						}
						if (line_count)
						{
							int start_x = TextPosX[i] - CurrentX;
							int start_y = TextPosY[i] - CurrentY;
							TextOut(dc, start_x * scale, start_y * scale, ptr, line_count);
						}
						ptr = lptr + 1;
						i += line_count + 1;
					}
      	      TextStyle->SetPointSize(old_point_size);
					BITMAPINFOHEADER * DibHeader = (BITMAPINFOHEADER *)new byte [
						256 * sizeof (RGBQUAD) + sizeof (BITMAPINFOHEADER)];
					DibHeader->biSize = sizeof (BITMAPINFOHEADER);
					DibHeader->biWidth = bmp_width * scale;
					DibHeader->biHeight = -bmp_height * scale; // turns the image right side up
					DibHeader->biPlanes = 1;
					DibHeader->biBitCount = 1;
					DibHeader->biCompression = BI_RGB;
					// it should fill the rest in for us
					DibHeader->biSizeImage = 0;
					DibHeader->biXPelsPerMeter = 0;
					DibHeader->biYPelsPerMeter = 0;
					DibHeader->biClrUsed = 0;
					DibHeader->biClrImportant = 0;
					byte * mask = (byte *)farmalloc(
               	((int32)bmp_height * scale * (int32)bmp_width * scale) >> 3);
					memset(mask, 0,
               	((int32)bmp_height * scale * (int32)bmp_width * scale) >> 3);
					byte * mask2 = (byte *)farmalloc((int32)(bmp_width) * (int32)bmp_height);
					memset(mask2, 0, (int32)(bmp_width) * (int32)bmp_height);
					GetDIBits(dc, bmp, 0, bmp_height * scale, mask, (LPBITMAPINFO)DibHeader,
						DIB_RGB_COLORS);
					FillAntiAliasTextMask(mask2, mask, bmp_width, bmp_height, scale);
					Image->DrawColorMask8(mask2, bmp_width, bmp_height,
						TextStyle->GetColor(), CurrentX, CurrentY);
					SelectObject(dc, old_font);
					SelectObject(dc, old_bmp);
					SelectObject(dc, old_palette);
					DeleteObject(bmp);
					DeleteObject(text_palette);
					farfree(mask);
					farfree(mask2);
					farfree(bmp_bits);
					DeleteDC(dc);
            	force = false;
         	}
         }
         if (force)
         {
				byte * bmp_bits = (byte *)farmalloc(
					(int32)(bmp_width >> 3) * (int32)bmp_height);
				memset(bmp_bits, 0, (bmp_width >> 3) * (int32)bmp_height);
				HBITMAP bmp = CreateBitmap(bmp_width, bmp_height, 1, 1, bmp_bits);
				HDC sdc = ::GetWindowDC(0);
				HDC dc = ::CreateCompatibleDC(sdc); // needs DeleteDC() when finished
				::ReleaseDC(0, sdc);
				HBITMAP old_bmp = (HBITMAP)SelectObject(dc, bmp);
				LOGPALETTE log_pal[2]; // to give us enough space
				PALETTEENTRY * text_pal = log_pal[0].palPalEntry;
				log_pal[0].palVersion = 0x300;
				log_pal[0].palNumEntries = 2;
				text_pal[0].peRed = text_pal[0].peGreen = text_pal[0].peBlue = 0;
				text_pal[0].peFlags = 0;
				text_pal[1].peRed = text_pal[1].peGreen = text_pal[1].peBlue = 255;
				text_pal[1].peFlags = 0;
				HPALETTE text_palette = CreatePalette(log_pal);
				HPALETTE old_palette = (HPALETTE)SelectObject(dc, text_palette);
				SelectObject(dc, GetStockObject(NULL_PEN));
				SelectObject(dc, GetStockObject(BLACK_BRUSH));
				Rectangle(dc, 0, 0, bmp_width + 1, bmp_height + 1);
				SetTextColor(dc, RGB(0xff, 0xff, 0xff));
				HFONT old_font = (HFONT)SelectObject(dc, TextStyle->GetFont());
				SetBkMode(dc, TRANSPARENT);
				SetTextAlign(dc, TA_LEFT | TA_BASELINE);
				char * ptr = TextArray;
				uint i = 0;
				while (i < NumTextArray)
				{
					int line_count = 0;
					char * lptr = ptr;
					while (i + line_count < NumTextArray && *lptr != '\n')
					{
						lptr++;
						line_count++;
					}
					if (line_count)
					{
						int start_x = TextPosX[i] - CurrentX;
						int start_y = TextPosY[i] - CurrentY;
						TextOut(dc, start_x, start_y, ptr, line_count);
					}
					ptr = lptr + 1;
					i += line_count + 1;
				}
				BITMAPINFOHEADER * DibHeader = (BITMAPINFOHEADER *)new byte [
					256 * sizeof (RGBQUAD) + sizeof (BITMAPINFOHEADER)];
				DibHeader->biSize = sizeof (BITMAPINFOHEADER);
				DibHeader->biWidth = bmp_width;
				DibHeader->biHeight = -bmp_height; // turns the image right side up
				DibHeader->biPlanes = 1;
				DibHeader->biBitCount = 1;
				DibHeader->biCompression = BI_RGB;
				// it should fill the rest in for us
				DibHeader->biSizeImage = 0;
				DibHeader->biXPelsPerMeter = 0;
				DibHeader->biYPelsPerMeter = 0;
				DibHeader->biClrUsed = 0;
				DibHeader->biClrImportant = 0;
				byte * mask = (byte *)farmalloc((int32)(bmp_width >> 3) * (int32)bmp_height);
				memset(mask, 0, (bmp_width >> 3) * (int32)bmp_height);
				GetDIBits(dc, bmp, 0, bmp_height, mask, (LPBITMAPINFO)DibHeader,
					DIB_RGB_COLORS);
				Image->DrawColorMask(mask, bmp_width, bmp_height,
					TextStyle->GetColor(), CurrentX, CurrentY);
				SelectObject(dc, old_font);
				SelectObject(dc, old_bmp);
				SelectObject(dc, old_palette);
				DeleteObject(bmp);
				DeleteObject(text_palette);
				farfree(mask);
				farfree(bmp_bits);
				DeleteDC(dc);
			}
#endif
			NumTextArray = 0; // must do this before drawing
			TextStarted = false;
			Rebuild();
			Draw();
		}
		else
		{
			DrawCursor();
		}
	}
	NumTextArray = 0;
	TextStarted = false;
}
void
G42SingleImageViewer::DrawText()
{
	if (!TextStarted)
		return;
	int32 min_x = CurrentX;
	int32 min_y = CurrentY;
	int32 max_x = OtherX;
	int32 max_y = OtherY;
	if (ErasingText)
	{
		if (ErasingTextMinX < min_x)
			min_x = ErasingTextMinX;
		if (ErasingTextMinY < min_y)
			min_y = ErasingTextMinY;
		if (ErasingTextMaxX > max_x)
			max_x = ErasingTextMaxX;
		if (ErasingTextMaxY > max_y)
			max_y = ErasingTextMaxY;
		ErasingText = false;
	}
	int x = (int)(ImageToWindow(min_x) - PanX);
	int y = (int)(ImageToWindow(min_y) - PanY);
	Draw(x, y, (int)(ImageToWindow(max_x + 1) - PanX) - x,
		(int)(ImageToWindow(max_y + 1) - PanX) - y);
#if 0
	if (!DC)
	{
		StartRubber();
		DrawText(DC);
		EndRubber();
	}
	else
	{
		DrawText(DC);
	}
#endif	
}
//TODO:  Set up drawlocation stuff
void G42SingleImageViewer::PrepareDrawText(G42DrawLocation location)
{
	#ifdef MSWIN
	G42Color c(TextStyle->GetColor());
	SetTextColor(location, RGB(c.red, c.green, c.blue));
	OldFont = (HFONT)SelectObject(location, TextStyle->GetFont());
	OldBkMode = SetBkMode(location, TRANSPARENT);
	SetTextAlign(location, TA_LEFT | TA_BASELINE);
	#endif
	#ifdef MACOS
	PenState penState;
	::GetPenState (&penState);
	penState.pnMode = srcXor;
	::SetPenState (&penState);
	#endif
}
void G42SingleImageViewer::CleanupDrawText(G42DrawLocation location)
{
	#ifdef MSWIN
	SelectObject(location, OldFont);
	SetBkMode(location, OldBkMode);
	#endif
	#ifdef MACOS
	::PenNormal();
	#endif
}
void
G42SingleImageViewer::DrawText(G42DrawLocation location)
{
	if (!TextStarted)
		return;
	if (!TextStyle)
		return;
	if (NumTextArray)
	{
		char * ptr = TextArray;
		uint i = 0;
		while (i < NumTextArray)
		{
			int line_count = 0;
			char * lptr = ptr;
			while (i + line_count < NumTextArray && *lptr != '\n')
			{
				lptr++;
				line_count++;
			}
			if (line_count)
			{
				int start_x = (int)(ImageToWindow(TextPosX[i]) - PanX);
				int start_y = (int)(ImageToWindow(TextPosY[i]) - PanY);
				#ifdef MSWIN
				TextOut(location, start_x, start_y, ptr, line_count);
				#endif
				#ifdef MACOS
				char cString [255];
				strcpy (cString, ptr);
				::MoveTo (start_x, start_y);
				::DrawString (::c2pstr (cString));			
				#endif
			}
			ptr = lptr + 1;
			i += line_count + 1;
		}
	}
	DrawCursor(location);
}
void
G42SingleImageViewer::GetTextExtents(const char * line,
	int length, int * w, int * h)
{
#ifdef MSWIN
	HDC dc = GetDC(0);
	HFONT old_font = (HFONT)SelectObject(dc, TextStyle->GetFont());
#if defined(WIN32)
	TSize size;
	GetTextExtentPoint32(dc, line, length, &size);
	*w = size.cx;
	*h = size.cy;
#else
	DWORD s = GetTextExtent(dc, line, length);
	*w = LOWORD(s);
	*h = HIWORD(s);
#endif
	SelectObject(dc, old_font);
	ReleaseDC(0, dc);
#endif
}
void
G42SingleImageViewer::DrawCursor()
{
	if (!TextStarted)
		return;
	if (DC)
	{
		DrawCursor(DC);
	}
	else
	{
		StartRubber();
		DrawCursor(DC);
		EndRubber();
	}
}
void
G42SingleImageViewer::DrawCursor(G42DrawLocation location)
{
	if (!TextStarted)
		return;
	int x1 = (int)(ImageToWindow(TextPosX[CursorTextPos]) - PanX);
	int y1 = (int)(ImageToWindow(TextPosY[CursorTextPos]) - PanY);
	int x2 = x1;
	int y2 = y1;
	switch (TextStyle->GetDirection())
	{
		case G42Text::Horizontal:
			y1 -= TextStyle->GetFontAscent();
			y2 += TextStyle->GetFontDescent();
			break;
		case G42Text::VerticalDown:
			x1 -= TextStyle->GetFontDescent();
			x2 += TextStyle->GetFontAscent();
			break;
		case G42Text::VerticalUp:
			x1 -= TextStyle->GetFontAscent();
			x2 += TextStyle->GetFontDescent();
			break;
	}
#ifdef MSWIN
	int old_rop2 = SetROP2(location, R2_NOT);
	SelectObject(location, GetStockObject(BLACK_PEN));
	MoveToEx(location, x1, y1, 0);
	LineTo(location, x2, y2);
	SetROP2(location, old_rop2);
#endif
#ifdef MACOS
	::MoveTo (x1, y1);
	::LineTo (x2, y2);
#endif
}
void
G42SingleImageViewer::EraseText()
{
	ErasingText = true;
	ErasingTextMinX = CurrentX;
	ErasingTextMinY = CurrentY;
	ErasingTextMaxX = OtherX;
	ErasingTextMaxY = OtherY;
#if 0
	if (!TextStarted)
		return;
	ErasingText++;
	int x = (int)(ImageToWindow(CurrentX) - PanX);
	int y = (int)(ImageToWindow(CurrentY) - PanY);
	Draw(x, y, (int)(ImageToWindow(OtherX + 1) - PanX) - x,
		(int)(ImageToWindow(OtherY + 1) - PanX) - y);
	ErasingText--;
#endif	
}
void
G42SingleImageViewer::FillAntiAliasTextMask(byte * to, byte * from,
	int width, int height, int scale)
{
	byte * to_ptr = to;
	byte * from_ptr = from;
	//height >>= 3;
	//width >>= 3;
	for (int i = 0; i < height; i++)
	{
		for (int x = 0; x < scale; x++)
      {
      	byte mask = 0x80;
	   	for (int n = 0; n < width; n++)
   	   {
         	for (int y = 0; y < scale; y++)
            {
	      	 	if (*from_ptr & mask)
   	      		(*to_ptr)++;
					if (mask > 0x01)
               	mask >>= 1;
               else
               {
               	mask = 0x80;
                  from_ptr++;
               }
            }
				to_ptr++;
	      }
			to_ptr -= width;
   	}
      to_ptr += width;
   }
	to_ptr = to;
   int max = 0;
   for (int n = 0; n < height; n++)
   {
   	for (int i = 0; i < width; i++, to_ptr++)
      {
      	if (*to_ptr > max)
         	max = *to_ptr;
      }
   }
	int offset = 0;
	int factor = 0;
   switch (scale)
   {
   	case 8:
      	offset = ((64 - max) << 2);
         factor = 2;
         break;
      case 4:
      	offset = ((16 - max) << 4);
         factor = 4;
         break;
      case 2:
      	offset = ((4 - max) << 6);
         factor = 6;
         break;
   }
	to_ptr = to;
   for (int n = 0; n < height; n++)
   {
   	for (int i = 0; i < width; i++, to_ptr++)
      {
      	int val = ((int)(*to_ptr) << factor);
         if (val)
         	val += offset;
         if (val > 255)
         	val = 255;
         *to_ptr = (byte)val;
      }
   }
}

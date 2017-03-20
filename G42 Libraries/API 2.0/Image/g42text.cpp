
#include "g42itype.h"
#include "g42text.h"
G42Text::G42Text(const char * typeface_name,
	int point_size, bool bold, bool italic,
	bool underline,
	G42Color color, G42Direction direction,
	G42Justification justification, bool anti_alias) :
		PointSize(point_size), Bold(bold), Italic(italic), Underline(underline),
		Color(color), Direction(direction), Justification(justification),
		AntiAlias(anti_alias),
#ifdef MSWIN
		Font(0), LogFont(0),
#endif
		TypefaceName(0)
{
	if (typeface_name)
	{
		TypefaceName = new char [strlen(typeface_name) + 1];
		strcpy(TypefaceName, typeface_name);
	}
	else
	{
#ifdef MSWIN
		TypefaceName = new char [16];
		strcpy(TypefaceName, "Times New Roman");
#endif
#ifdef MACOS
		Str255 pName;
		::GetFontName (0, pName);
		TypefaceName = new char [255];
		strcpy (TypefaceName, ::p2cstr (pName));
#else
		TypefaceName = 0;
#endif
	}
#ifdef MSWIN
   LogFont = new LOGFONT;
	LogFont->lfHeight = CalcPointsToPixels();
	LogFont->lfWidth = 0;
	LogFont->lfEscapement = Direction * 900;
	if (LogFont->lfEscapement < 0)
		LogFont->lfEscapement += 3600;
	LogFont->lfOrientation = Direction * 900;
	if (LogFont->lfOrientation < 0)
		LogFont->lfOrientation += 3600;
	LogFont->lfWeight = (Bold ? FW_BOLD : FW_NORMAL);
	LogFont->lfItalic = Italic;
	LogFont->lfUnderline = Underline;
	LogFont->lfStrikeOut = 0;
	LogFont->lfCharSet = ANSI_CHARSET;
	LogFont->lfOutPrecision = OUT_DEFAULT_PRECIS;
	LogFont->lfClipPrecision = CLIP_DEFAULT_PRECIS | CLIP_LH_ANGLES;
	LogFont->lfQuality = PROOF_QUALITY;
	LogFont->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	strcpy(LogFont->lfFaceName, TypefaceName);
#endif
	CreateFont();
}
G42Text::~G42Text (void)
{
	if (TypefaceName)
	{
		delete [] TypefaceName;
		TypefaceName = 0;
	}
#ifdef MSWIN
	if (Font)
	{
		DeleteObject(Font);
		Font = 0;
	}
   if (LogFont)
   {
   	delete LogFont;
      LogFont = 0;
   }
#endif
}
void
G42Text::CreateFont (void)
{
#ifdef MSWIN
	HFONT new_font = CreateFontIndirect(LogFont);
	if (Font && new_font)
	{
		DeleteObject(Font);
		Font = 0;
	}
	if (new_font)
	{
		Font = new_font;
		TEXTMETRIC metric;
		HDC dc = GetDC(0);
		HFONT old_font = (HFONT)SelectObject(dc, new_font);
		GetTextMetrics(dc, &metric);
		SelectObject(dc, old_font);
		ReleaseDC(0, dc);
		FontHeight = metric.tmHeight;
		FontAscent = metric.tmAscent;
		FontDescent = metric.tmDescent;
		FontLineSpacing = metric.tmHeight + metric.tmExternalLeading;
	}
#endif
#ifdef MACOS
	::GetFontInfo (&FInfo);
	FontHeight  	= FInfo.ascent + FInfo.descent;
	FontAscent  	= FInfo.ascent;
	FontDescent 	= FInfo.descent;
	FontLineSpacing = FInfo.ascent + FInfo.descent + FInfo.leading;
#endif
}
bool
G42Text::CheckFont(void)
{
	#ifdef MSWIN
	HDC dc = GetDC(0);
	HFONT old_font = (HFONT)SelectObject(dc, Font);
	int size = GetOutlineTextMetrics(dc, 0, NULL);
   if (!size)
   {
		SelectObject(dc, old_font);
		ReleaseDC(0, dc);
   	return false;
   }
   HANDLE hMem = GlobalAlloc(GMEM_ZEROINIT, (DWORD)size);
   OUTLINETEXTMETRIC * out_text_metric = (OUTLINETEXTMETRIC *)GlobalLock(hMem);
   GetOutlineTextMetrics(dc, size, out_text_metric);
	char * face_name = (char *)(out_text_metric) + (int)out_text_metric->otmpFaceName;
	bool ret_val = false;
   if (!strcmp(face_name, TypefaceName))
   	ret_val = true;
   GlobalUnlock(hMem);
	GlobalFree(hMem);
	SelectObject(dc, old_font);
	ReleaseDC(0, dc);
   return ret_val;
   #else
   return false;
   #endif
}
void
G42Text::SetDirection (G42Direction direction)
{
	#ifdef MSWIN
	if (Direction != direction)
	{
		Direction = direction;
		LogFont->lfEscapement = Direction * 900;
		if (LogFont->lfEscapement < 0)
			LogFont->lfEscapement += 3600;
		LogFont->lfOrientation = Direction * 900;
		if (LogFont->lfOrientation < 0)
			LogFont->lfOrientation += 3600;
		CreateFont();
	}
	#endif
}
void
G42Text::SetFontName (const char * typeface_name)
{
#if 0
	if (!TypefaceName || strcmp(TypefaceName, typeface_name))
	{
		if (TypefaceName)
			delete [] TypefaceName;
		TypefaceName = new char [strlen(typeface_name) + 1];
		strcpy(TypefaceName, typeface_name);
		CreateFont();
	}
#endif
}
void
G42Text::SetPointSize (int point_size)
{
	#ifdef MSWIN
	if (PointSize != point_size)
	{
		PointSize = point_size;
		LogFont->lfHeight = CalcPointsToPixels();
		CreateFont();
	}
	#endif
}
void
G42Text::SetBold (bool bold)
{
	#ifdef MSWIN
	if (Bold != bold)
	{
		Bold = bold;
		LogFont->lfWeight = (Bold ? FW_BOLD : FW_NORMAL);
		CreateFont();
	}
	#endif
}
void
G42Text::SetItalic (bool italic)
{
	#ifdef MSWIN
	if (Italic != italic)
	{
		Italic = italic;
		LogFont->lfItalic = Italic;
		CreateFont();
	}
	#endif
}
void
G42Text::SetUnderline (bool underline)
{
	#ifdef MSWIN
	if (Underline != underline)
	{
		Underline = underline;
		LogFont->lfUnderline = Underline;
		CreateFont();
	}
	#endif
}
void
G42Text::SetFontAttributes (const char * typeface_name, int point_size,
	bool bold, bool italic, bool underline, G42Color color)
{
	#ifdef MSWIN
	bool dirty = false;
	Color = color;
	if (!TypefaceName || strcmp(TypefaceName, typeface_name))
	{
		if (TypefaceName)
			delete [] TypefaceName;
		TypefaceName = new char [strlen(typeface_name) + 1];
		strcpy(TypefaceName, typeface_name);
      strcpy(LogFont->lfFaceName, TypefaceName);
		dirty = true;
	}
	if (PointSize != point_size)
	{
		PointSize = point_size;
		LogFont->lfHeight = CalcPointsToPixels();
		dirty = true;
	}
	if (Bold != bold)
	{
		Bold = bold;
		LogFont->lfWeight = (Bold ? FW_BOLD : FW_NORMAL);
		dirty = true;
	}
	if (Italic != italic)
	{
		Italic = italic;
		LogFont->lfItalic = Italic;
		dirty = true;
	}
	if (Underline != underline)
	{
		Underline = underline;
		LogFont->lfUnderline = Underline;
		dirty = true;
	}
	if (dirty)
		CreateFont();
	#endif
}
#ifdef MSWIN
void
G42Text::SetFont(LOGFONT * log_font, int point_size, G42Color color)
{
	Color = color;
	if (!TypefaceName || strcmp(TypefaceName, log_font->lfFaceName))
	{
		if (TypefaceName)
			delete [] TypefaceName;
		TypefaceName = new char [strlen(log_font->lfFaceName) + 1];
		strcpy(TypefaceName, log_font->lfFaceName);
	}
	if (PointSize != point_size)
		PointSize = point_size;
	if (log_font->lfWeight == FW_NORMAL)
   	Bold = false;
   else
   	Bold = true;
  	Italic = log_font->lfItalic;
  	Underline = log_font->lfUnderline;
	*LogFont = *log_font;
	LogFont->lfEscapement = Direction * 900;
	if (LogFont->lfEscapement < 0)
		LogFont->lfEscapement += 3600;
	LogFont->lfOrientation = Direction * 900;
	if (LogFont->lfOrientation < 0)
		LogFont->lfOrientation += 3600;
	HFONT new_font = CreateFontIndirect(LogFont);
	if (Font && new_font)
	{
		DeleteObject(Font);
		Font = 0;
	}
	if (new_font)
	{
		Font = new_font;
		TEXTMETRIC metric;
		HDC dc = GetDC(0);
		HFONT old_font = (HFONT)SelectObject(dc, new_font);
		GetTextMetrics(dc, &metric);
		SelectObject(dc, old_font);
		ReleaseDC(0, dc);
		FontHeight = metric.tmHeight;
		FontAscent = metric.tmAscent;
		FontDescent = metric.tmDescent;
		FontLineSpacing = metric.tmHeight + metric.tmExternalLeading;
	}
}
#endif
int
G42Text::CalcPointsToPixels (void)
{
#ifdef MSWIN
	HDC dc = GetDC(0);
	int pixelsY = GetDeviceCaps(dc, LOGPIXELSY);
	ReleaseDC(0, dc);
	return -(((pixelsY * PointSize + 36) / 72));
#else
	return PointSize;
#endif
}

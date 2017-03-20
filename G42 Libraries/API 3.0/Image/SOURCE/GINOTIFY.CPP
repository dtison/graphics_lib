#include "ginotify.h"
G42QueryURL::G42QueryURL (const char * url, const char * comment,
	const char * alt_text) : URL (0), Comment (0), AltText (0) 
{
	SetURL (url);
	SetComment (comment);
	SetAltText (alt_text);
}
void
G42QueryURL::SetURL (const char * url)
{ 
	if (URL)
	{
		delete [] URL;
		URL = 0;
	}
	if (url && *url)
	{
		URL = new char [strlen(url) + 1];
		strcpy (URL, url);
	}
}
	
void
G42QueryURL::SetComment (const char * comment)
{ 
	if (Comment)
	{
		delete [] Comment;
		Comment = 0;
	}
	if (comment && *comment)
	{
		Comment = new char [strlen (comment) + 1];
		strcpy (Comment, comment);
	}
}			
void
G42QueryURL::SetAltText (const char * alt_text)
{ 
	if (AltText)
	{
		delete [] AltText;
		AltText = 0;
	}
	if (alt_text && *alt_text)
	{
		AltText = new char [strlen (alt_text) + 1];
		strcpy (AltText, alt_text);
	}
}			

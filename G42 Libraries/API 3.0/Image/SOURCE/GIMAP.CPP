// gimap.cpp - image map class
#include "gimap.h"
#include "gmacfl.h"
G42ImageMap::G42ImageMap(const G42FilePath file_path) :
	NumElements(0), Valid(false), Modified(false), DefaultValid(false),
	DefaultIndex(0), ImageWidth(0), ImageHeight(0), MapType(NCSA),
	SelectedElement(0), IsClientSide(false), ClientSideState(0), ClientSideDone(0),
	ClientSideShape(G42MapElement::Rectangle), ClientSideHaveMap(false), ClientSideHaveShape(false),
	ClientSideHaveCoords(false), ClientSideToken(0), ClientSideNumPointArray(0),
	x1(0), y1(0), x2(0), y2(0), ClientSideName(0), Comment(0), AltText(0),
	SelectedElementValid(false), IsInternal(false), URL(0)
{
	#ifdef MSWIN
	char *full_path = file_path;
	#endif
	#ifdef MACOS
	FSSpec *spec = file_path;
	char *full_path = G42MacFile::GetFullPath (spec -> name, spec -> parID, spec -> vRefNum);
	#endif
	FILE* fp = fopen(full_path, "r");
	if (fp == (FILE*)NULL)
		return;
	char * buffer = new char [4096];
	if (!buffer)
		return;
	Comment = new char [256];
	if (!Comment)
		return;
	Comment[0] = '\0';
	AltText = new char [256];
	if (!AltText)
		return;
	AltText[0] = '\0';
	URL = new char [256];
	if (!URL)
		return;
	URL[0] = '\0';
	
	while(fgets(buffer, 4096, fp) != (char*)NULL)
	{
		if (ParseMapString(buffer))
			Valid = true;
	}
	fclose(fp);
	delete [] buffer;
	#ifdef MACOS
	if (full_path) {
		delete [] full_path;
	}	
	#endif
	Modified = false;
}
G42ImageMap::G42ImageMap(const char * map_buffer, uintf buffer_length) :
	NumElements(0), Valid(false), Modified(false), DefaultValid(false),
	DefaultIndex(0), ImageWidth(0), ImageHeight(0), MapType(NCSA),
	SelectedElement(0), IsClientSide(false), ClientSideState(0), ClientSideDone(0),
	ClientSideShape(G42MapElement::Rectangle), ClientSideHaveMap(false), 
	ClientSideHaveShape(false),
	ClientSideHaveCoords(false), ClientSideToken(0), ClientSideNumPointArray(0),
	x1(0), y1(0), x2(0), y2(0), ClientSideName(0), Comment(0), AltText(0),
	SelectedElementValid(false), IsInternal(false), URL(0)
{
	char * buffer = new char [4096];
	if (!buffer)
		return;
	Comment = new char [256];
	if (!Comment)
		return;
	Comment[0] = '\0';
	AltText = new char [256];
	if (!AltText)
		return;
	AltText[0] = '\0';
	URL = new char [256];
	if (!URL)
		return;
	URL[0] = '\0';
	const char * tptr = map_buffer;
	while(tptr < (map_buffer + buffer_length))
	{
		const char * ptr = strchr(tptr, '\n');
		if (ptr == 0)
      {
      	if (*tptr)
         {
				strcpy(buffer, tptr);
				if (ParseMapString(buffer))
					Valid = true;
         }
			break;
      }
		memcpy(buffer, tptr, (ptr - tptr + 1));
		buffer[(ptr - tptr + 1)] = 0;
		tptr = (ptr + 1);
		if (ParseMapString(buffer))
			Valid = true;
	}
	delete [] buffer;
	Modified = false;
}
G42ImageMap::G42ImageMap() :
	NumElements(0), Valid(true), Modified(false), DefaultValid(false),
	DefaultIndex(0), ImageWidth(0), ImageHeight(0), MapType(NCSA),
	SelectedElement(0), IsClientSide(false), ClientSideState(0), ClientSideDone(0),
	ClientSideShape(G42MapElement::Rectangle), ClientSideHaveMap(false), 
	ClientSideHaveShape(false),
	ClientSideHaveCoords(false), ClientSideToken(0), ClientSideNumPointArray(0),
	x1(0), y1(0), x2(0), y2(0), ClientSideName(0), Comment(0), AltText(0),
	SelectedElementValid(false), IsInternal(false), URL(0)
{
}
G42ImageMap::~G42ImageMap()
{
	for (int i = 0; i < NumElements; i++)
		delete Elements[i];
	if (ClientSideName)
		delete [] ClientSideName;
	if (Comment)
		delete [] Comment;
	if (AltText)
		delete [] AltText;
	if (URL)
		delete [] URL;
}
void
G42ImageMap::SetClientSideName(const char * name)
{
	if (ClientSideName)
		delete [] ClientSideName;
	ClientSideName = new char [strlen(name) + 1];
	if (ClientSideName)
		strcpy(ClientSideName, name);
}
const char * G42ImageMap::GetURLAtPoint(int x, int y)
{
	int ret = GetIndexAtPoint(x, y);
	if (ret >= 0)
		return Elements[ret]->URL;
	else
		return 0;
}
int16 G42ImageMap::GetIndexAtPoint(int x, int y)
{
	//if (x < 0 || x >= ImageWidth || y < 0 || y >= ImageHeight)
		//return -1;
	int16 hit = -1;
	bool HasPoints = false;
	long Dist = 0x7FFFFFFF;
	int16 WhichPoint = -1;
	for (int16 i = 0; i < NumElements; i++)
	{
		switch(Elements[i]->Type)
		{
		case G42MapElement::Point:
		{
			long TDist = (long)(x - Elements[i]->Points[0].X)*
				(long)(x - Elements[i]->Points[0].X) +
				(long)(y - Elements[i]->Points[0].Y)*
				(long)(y - Elements[i]->Points[0].Y);
			if (TDist < Dist)
			{
				Dist = TDist;
				WhichPoint = i;
			}
			HasPoints = true;		// MACOS: TRUE changed to true  D. Ison
		}
			break;
		case G42MapElement::Rectangle:
			if ((x >= Elements[i]->Points[0].X) &&
				(x <= Elements[i]->Points[1].X)
				&& (y >= Elements[i]->Points[0].Y) &&
				(y <= Elements[i]->Points[1].Y))
			{
				hit = i;
				i = NumElements;
			}
			break;
		case G42MapElement::Circle:
		{
			long CenterX = (Elements[i]->Points[0].X +
				Elements[i]->Points[1].X)>>1;
			long CenterY = (Elements[i]->Points[0].Y +
				Elements[i]->Points[1].Y)>>1;
			long Rad2 = (y - CenterY)*(y - CenterY) + (x - CenterX)*(x - CenterX);
			long Rad1 = (Elements[i]->Points[1].X - Elements[i]->Points[0].X)>>1;
			Rad1 = Rad1*Rad1;
			if (Rad2 <= Rad1)
			{
				hit = i;
				i = NumElements;
			}
			break;
		}
		case G42MapElement::Polygon:
			if (PointInPoly(x, y, i))
			{
				hit = i;
				i = NumElements;
			}
			break;
		default:
			break;
		}
	}
	if (hit == -1)
	{
		if (HasPoints)
			hit = WhichPoint;
		else if (DefaultValid && Elements[DefaultIndex]->Type == G42MapElement::Default)
			hit = DefaultIndex;
	}
   return hit;
}
void
G42ImageMap::Save(const G42FilePath file_path, G42MapType type)
{
	if (NumElements == 0)
		return;
	#ifdef MSWIN
	char *full_path = file_path;
	#endif
	#ifdef MACOS
	FSSpec *spec = file_path;
	char *full_path = G42MacFile::GetFullPath (spec -> name, spec -> parID, spec -> vRefNum);
	#endif
	FILE* fp = fopen(full_path, "w+");
	if (type == NCSA)
		SaveNCSA(fp);
	else if (type == CERN)
		SaveCERN(fp);
	else if (type == ClientSide)
		SaveClientSide(full_path, fp);
	else if (type == Internal)
		SaveInternal(fp);
	fclose(fp);
	#ifdef MACOS
	if (full_path) {
		delete [] full_path;
	}	
	#endif
	Modified = false;
}
void 
G42ImageMap::SaveNCSA(FILE *fp)
{
	for(int i = 0; i < NumElements; i++)
	{
		switch(Elements[i]->Type)
		{
		case G42MapElement::Point:
			fprintf(fp, "#%s\n", Elements[i]->Comment);
			fprintf(fp, "point %s %d,%d\n", Elements[i]->URL,
				Elements[i]->Points[0].X, Elements[i]->Points[0].Y);
			break;
		case G42MapElement::Rectangle:
			fprintf(fp, "#%s\n", Elements[i]->Comment);
			fprintf(fp, "rect %s %d,%d %d,%d\n", Elements[i]->URL,
				Elements[i]->Points[0].X, Elements[i]->Points[0].Y,
				Elements[i]->Points[1].X, Elements[i]->Points[1].Y);
			break;
		case G42MapElement::Circle:
			fprintf(fp, "#%s\n", Elements[i]->Comment);
			fprintf(fp, "circle %s %d,%d %d,%d\n", Elements[i]->URL,
				(Elements[i]->Points[0].X+Elements[i]->Points[1].X)/2,
				(Elements[i]->Points[0].Y+Elements[i]->Points[1].Y)/2,
				(Elements[i]->Points[0].X+Elements[i]->Points[1].X)/2,
				Elements[i]->Points[0].Y);
			break;
		case G42MapElement::Polygon:
			{
			fprintf(fp, "#%s\n", Elements[i]->Comment);
			fprintf(fp, "poly %s", Elements[i]->URL);
			for(int j = 0; j < Elements[i]->NumPoints; j++)
				fprintf(fp, " %d,%d", Elements[i]->Points[j].X,
					Elements[i]->Points[j].Y);
			fprintf(fp, "\n");
			}
			break;
		case G42MapElement::Default:
			fprintf(fp, "#%s\n", Elements[i]->Comment);
			fprintf(fp, "default %s\n", Elements[i]->URL);
			break;
		default:
			break;
		}
	}
}
void
G42ImageMap::SaveCERN (FILE *fp)
{
	for(int i=0; i<NumElements; i++)
	{
		switch(Elements[i]->Type)
		{
		case G42MapElement::Rectangle:
			fprintf(fp, "#%s\n", Elements[i]->Comment);
			fprintf(fp, "rectangle (%d,%d) (%d,%d) %s\n",
				Elements[i]->Points[0].X, Elements[i]->Points[0].Y,
				Elements[i]->Points[1].X, Elements[i]->Points[1].Y,
				Elements[i]->URL);
			break;
		case G42MapElement::Circle:
			fprintf(fp, "#%s\n", Elements[i]->Comment);
			fprintf(fp, "circle (%d,%d) %d %s\n",
				(Elements[i]->Points[0].X+Elements[i]->Points[1].X)/2,
				(Elements[i]->Points[0].Y+Elements[i]->Points[1].Y)/2,
				(abs(Elements[i]->Points[1].X-Elements[i]->Points[0].X))/2,
				Elements[i]->URL);
			break;
		case G42MapElement::Polygon:
			{
			fprintf(fp, "#%s\n", Elements[i]->Comment);
			fprintf(fp, "polygon");
			for(int j = 0; j < Elements[i]->NumPoints; j++)
				fprintf(fp, " (%d,%d)", Elements[i]->Points[j].X,
					Elements[i]->Points[j].Y);
			fprintf(fp, " %s\n", Elements[i]->URL);
			}
			break;
		case G42MapElement::Default:
			fprintf(fp, "#%s\n", Elements[i]->Comment);
			fprintf(fp, "default %s\n", Elements[i]->URL);
			break;
		default:
			break;
		}
	}
}
void
G42ImageMap::SaveClientSide(const char * full_path, FILE *fp)
{
	if (!ClientSideName)
	{
		char tfname [256];
		strcpy(tfname, full_path);
		char * start = tfname + strlen(tfname) - 1;
		while (start > tfname && *start != '\\')
			start--;
		if (*start == '\\')
			start++;
		char * end = start;
		while (*end && *end != '.')
			end++;
		ClientSideName = new char [(int)(end - start) + 1];
		strncpy(ClientSideName, start, (int)(end - start));
		ClientSideName[(int)(end - start)] = '\0';
	}
	fprintf(fp, "<MAP NAME=\"%s\">\n", ClientSideName);
	for (int i = 0; i < NumElements; i++)
	{
		switch(Elements[i]->Type)
		{
			case G42MapElement::Rectangle:
				if (Elements[i]->Comment && Elements[i]->Comment[0])
				{
					fprintf(fp, "<!-- %s -->\n", Elements[i]->Comment);
				}
				fprintf(fp, "<AREA SHAPE=RECT COORDS=\"%d,%d,%d,%d\"",
					Elements[i]->Points[0].X, Elements[i]->Points[0].Y,
					Elements[i]->Points[1].X, Elements[i]->Points[1].Y);
				if (Elements[i]->URL && Elements[i]->URL[0])
				{
					fprintf(fp, " HREF=\"%s\"", Elements[i]->URL);
				}
				else
					fprintf(fp, " NOHREF");
				if (Elements[i]->AltText && Elements[i]->AltText[0])
				{
					fprintf(fp, " ALT=\"%s\"", Elements[i]->AltText);
				}
				fprintf(fp, ">\n");
				break;
			case G42MapElement::Circle:
				if (Elements[i]->Comment && Elements[i]->Comment[0])
				{
					fprintf(fp, "<!-- %s -->\n", Elements[i]->Comment);
				}
				fprintf(fp, "<AREA SHAPE=CIRCLE COORDS=\"%d,%d,%d\"",
					(Elements[i]->Points[0].X+Elements[i]->Points[1].X)/2,
					(Elements[i]->Points[0].Y+Elements[i]->Points[1].Y)/2,
					(abs(Elements[i]->Points[1].X-Elements[i]->Points[0].X))/2);
				if (Elements[i]->URL && Elements[i]->URL[0])
				{
					fprintf(fp, " HREF=\"%s\"", Elements[i]->URL);
				}
				else
					fprintf(fp, " NOHREF");
				if (Elements[i]->AltText && Elements[i]->AltText[0])
				{
					fprintf(fp, " ALT=\"%s\"", Elements[i]->AltText);
				}
				fprintf(fp, ">\n");
				break;
			case G42MapElement::Polygon:
			{
				if (Elements[i]->Comment && Elements[i]->Comment[0])
				{
					fprintf(fp, "<!-- %s -->\n", Elements[i]->Comment);
				}
				fprintf(fp, "<AREA SHAPE=POLYGON COORDS=\"%d,%d",
					Elements[i]->Points[0].X, Elements[i]->Points[0].Y);
				for(int j = 1; j < Elements[i]->NumPoints; j++)
				{
					fprintf(fp, ",%d,%d", Elements[i]->Points[j].X,
						Elements[i]->Points[j].Y);
				}
				fprintf(fp, "\"");
				if (Elements[i]->URL && Elements[i]->URL[0])
				{
					fprintf(fp, " HREF=\"%s\"", Elements[i]->URL);
				}
				else
					fprintf(fp, " NOHREF");
				if (Elements[i]->AltText && Elements[i]->AltText[0])
				{
					fprintf(fp, " ALT=\"%s\"", Elements[i]->AltText);
				}
				fprintf(fp, ">\n");
				break;
			}
			case G42MapElement::Default:
				// save default until end
				break;
			default:
				break;
		}
	}
	if (HasDefault())
	{
		if (Elements[DefaultIndex]->Comment
			&& Elements[DefaultIndex]->Comment[0])
		{
			fprintf(fp, "<!-- %s -->\n", Elements[DefaultIndex]->Comment);
		}
		fprintf(fp, "<AREA SHAPE=RECT COORDS=\"%d,%d,%d,%d\"",
			0, 0, ImageWidth, ImageHeight);
		if (Elements[DefaultIndex]->URL && Elements[DefaultIndex]->URL[0])
		{
			fprintf(fp, " HREF=\"%s\"", Elements[DefaultIndex]->URL);
		}
		else
			fprintf(fp, " NOHREF");
		if (Elements[DefaultIndex]->AltText && Elements[DefaultIndex]->AltText[0])
		{
			fprintf(fp, " ALT=\"%s\"", Elements[DefaultIndex]->AltText);
		}
		fprintf(fp, ">\n");
	}
	fprintf(fp, "</MAP>\n");
}
void
G42ImageMap::SaveInternal(FILE *fp)
{
	fprintf(fp, "g42imap\n"); // mark as our internal format
	if (ClientSideName && *ClientSideName)
	{
		fprintf(fp, "n%s\n", ClientSideName);
	}
	
	if (ImageWidth && ImageHeight)
	{
		fprintf(fp, "w%d\n", ImageWidth);
		fprintf(fp, "h%d\n", ImageHeight);
	}
	for (int i = 0; i < NumElements; i++)
	{
		switch (Elements[i]->Type)
		{
		case G42MapElement::Point:
			if (Elements[i]->Comment && Elements[i]->Comment[0])
			{
				fprintf(fp, "c%s\n", Elements[i]->Comment);
			}
			if (Elements[i]->URL && Elements[i]->URL[0])
			{
				fprintf(fp, "u%s\n", Elements[i]->URL);
			}
			if (Elements[i]->AltText && Elements[i]->AltText[0])
			{
				fprintf(fp, "a%s\n", Elements[i]->AltText);
			}
			fprintf(fp, "t%d,%d\n",
				Elements[i]->Points[0].X, Elements[i]->Points[0].Y);
			break;
		case G42MapElement::Rectangle:
			if (Elements[i]->Comment && Elements[i]->Comment[0])
			{
				fprintf(fp, "c%s\n", Elements[i]->Comment);
			}
			if (Elements[i]->URL && Elements[i]->URL[0])
			{
				fprintf(fp, "u%s\n", Elements[i]->URL);
			}
			if (Elements[i]->AltText && Elements[i]->AltText[0])
			{
				fprintf(fp, "a%s\n", Elements[i]->AltText);
			}
			fprintf(fp, "r%d,%d,%d,%d\n",
				Elements[i]->Points[0].X, Elements[i]->Points[0].Y,
				Elements[i]->Points[1].X, Elements[i]->Points[1].Y);
			break;
		case G42MapElement::Circle:
			if (Elements[i]->Comment && Elements[i]->Comment[0])
			{
				fprintf(fp, "c%s\n", Elements[i]->Comment);
			}
			if (Elements[i]->URL && Elements[i]->URL[0])
			{
				fprintf(fp, "u%s\n", Elements[i]->URL);
			}
			if (Elements[i]->AltText && Elements[i]->AltText[0])
			{
				fprintf(fp, "a%s\n", Elements[i]->AltText);
			}
			fprintf(fp, "o%d,%d,%d,%d\n",
				Elements[i]->Points[0].X, Elements[i]->Points[0].Y,
				Elements[i]->Points[1].X, Elements[i]->Points[1].Y);
			break;
		case G42MapElement::Polygon:
		{
			if (Elements[i]->Comment && Elements[i]->Comment[0])
			{
				fprintf(fp, "c%s\n", Elements[i]->Comment);
			}
			if (Elements[i]->URL && Elements[i]->URL[0])
			{
				fprintf(fp, "u%s\n", Elements[i]->URL);
			}
			if (Elements[i]->AltText && Elements[i]->AltText[0])
			{
				fprintf(fp, "a%s\n", Elements[i]->AltText);
			}
			fprintf(fp, "p");
			for(int j = 0; j < Elements[i]->NumPoints; j++)
			{
				if (j)
					fprintf(fp, ",");
				fprintf(fp, "%d,%d", Elements[i]->Points[j].X,
					Elements[i]->Points[j].Y);
			}
			fprintf(fp, "\n");
			break;
		}
		case G42MapElement::Default:
			if (Elements[i]->Comment && Elements[i]->Comment[0])
			{
				fprintf(fp, "c%s\n", Elements[i]->Comment);
			}
			if (Elements[i]->URL && Elements[i]->URL[0])
			{
				fprintf(fp, "u%s\n", Elements[i]->URL);
			}
			if (Elements[i]->AltText && Elements[i]->AltText[0])
			{
				fprintf(fp, "a%s\n", Elements[i]->AltText);
			}
			fprintf(fp, "d\n", Elements[i]->URL);
			break;
		default:
			break;
		}
	}
}
void G42ImageMap::Save(char * map_buffer, uintf max_buffer_length,
	G42MapType type)
{
	/*  Quiets warnings  */
	if (max_buffer_length) {}
	if (NumElements == 0)
		return;
	char * ptr = map_buffer;
	if (type == NCSA)
		SaveNCSA(ptr);
	else if (type == CERN)
		SaveCERN(ptr);
	else if (type == ClientSide)
		SaveClientSide(ptr);
	else if (type == Internal)
		SaveInternal(ptr);
	Modified = false;
}
void
G42ImageMap::SaveNCSA (char *ptr)
{
	for(int i = 0; i < NumElements; i++)
	{
		switch(Elements[i]->Type)
		{
		case G42MapElement::Point:
			sprintf(ptr, "#%s\n", Elements[i]->Comment);
			ptr += strlen(ptr);
			sprintf(ptr, "point %s %d,%d\n", Elements[i]->URL,
				Elements[i]->Points[0].X, Elements[i]->Points[0].Y);
			ptr += strlen(ptr);
			break;
		case G42MapElement::Rectangle:
			sprintf(ptr, "#%s\n", Elements[i]->Comment);
			ptr += strlen(ptr);
			sprintf(ptr, "rect %s %d,%d %d,%d\n", Elements[i]->URL,
				Elements[i]->Points[0].X, Elements[i]->Points[0].Y,
				Elements[i]->Points[1].X, Elements[i]->Points[1].Y);
			ptr += strlen(ptr);
			break;
		case G42MapElement::Circle:
			sprintf(ptr, "#%s\n", Elements[i]->Comment);
			ptr += strlen(ptr);
			sprintf(ptr, "circle %s %d,%d %d,%d\n", Elements[i]->URL,
				(Elements[i]->Points[0].X+Elements[i]->Points[1].X)/2,
				(Elements[i]->Points[0].Y+Elements[i]->Points[1].Y)/2,
				(Elements[i]->Points[0].X+Elements[i]->Points[1].X)/2,
				Elements[i]->Points[0].Y);
			ptr += strlen(ptr);
			break;
		case G42MapElement::Polygon:
			{
			sprintf(ptr, "#%s\n", Elements[i]->Comment);
			ptr += strlen(ptr);
			sprintf(ptr, "poly %s", Elements[i]->URL);
			ptr += strlen(ptr);
			for(int j = 0; j < Elements[i]->NumPoints; j++)
			{
				sprintf(ptr, " %d,%d", Elements[i]->Points[j].X,
					Elements[i]->Points[j].Y);
				ptr += strlen(ptr);
			}
			sprintf(ptr, "\n");
			}
			break;
		case G42MapElement::Default:
			sprintf(ptr, "#%s\n", Elements[i]->Comment);
			ptr += strlen(ptr);
			sprintf(ptr, "default %s\n", Elements[i]->URL);
			ptr += strlen(ptr);
			break;
		default:
			break;
		}
	}
}
void
G42ImageMap::SaveCERN (char *ptr)
{
	for(int i=0; i<NumElements; i++)
	{
		switch(Elements[i]->Type)
		{
		case G42MapElement::Rectangle:
			sprintf(ptr, "#%s\n", Elements[i]->Comment);
			ptr += strlen(ptr);
			sprintf(ptr, "rectangle (%d,%d) (%d,%d) %s\n",
				Elements[i]->Points[0].X, Elements[i]->Points[0].Y,
				Elements[i]->Points[1].X, Elements[i]->Points[1].Y,
				Elements[i]->URL);
			ptr += strlen(ptr);
			break;
		case G42MapElement::Circle:
			sprintf(ptr, "#%s\n", Elements[i]->Comment);
			ptr += strlen(ptr);
			sprintf(ptr, "circle (%d,%d) %d %s\n",
				(Elements[i]->Points[0].X+Elements[i]->Points[1].X)/2,
				(Elements[i]->Points[0].Y+Elements[i]->Points[1].Y)/2,
				(abs(Elements[i]->Points[1].X-Elements[i]->Points[0].X))/2,
				Elements[i]->URL);
			ptr += strlen(ptr);
			break;
		case G42MapElement::Polygon:
			{
			sprintf(ptr, "#%s\n", Elements[i]->Comment);
			ptr += strlen(ptr);
			sprintf(ptr, "polygon");
			ptr += strlen(ptr);
			for(int j = 0; j < Elements[i]->NumPoints; j++)
			{
				sprintf(ptr, " (%d,%d)", Elements[i]->Points[j].X,
					Elements[i]->Points[j].Y);
			}
			sprintf(ptr, " %s\n", Elements[i]->URL);
			ptr += strlen(ptr);
			}
			break;
		case G42MapElement::Default:
			sprintf(ptr, "#%s\n", Elements[i]->Comment);
			ptr += strlen(ptr);
			sprintf(ptr, "default %s\n", Elements[i]->URL);
			ptr += strlen(ptr);
			break;
		default:
			break;
		}
	}
}
void
G42ImageMap::SaveClientSide (char *ptr)
{
	if (!ClientSideName)
	{
		char tfname [256];
		strcpy(tfname, "MapName");
		char * start = tfname + strlen(tfname) - 1;
		while (start > tfname && *start != '\\')
			start--;
		if (*start == '\\')
			start++;
		char * end = start;
		while (*end && *end != '.')
			end++;
		ClientSideName = new char [(int)(end - start) + 1];
		strncpy(ClientSideName, start, (int)(end - start));
		ClientSideName[(int)(end - start)] = '\0';
	}
	sprintf(ptr, "<MAP NAME=\"%s\">\n", ClientSideName);
	ptr += strlen(ptr);
	for (int i = 0; i < NumElements; i++)
	{
		switch(Elements[i]->Type)
		{
			case G42MapElement::Rectangle:
				if (Elements[i]->Comment && Elements[i]->Comment[0])
				{
					sprintf(ptr, "<!-- %s -->\n", Elements[i]->Comment);
					ptr += strlen(ptr);
				}
				sprintf(ptr, "<AREA SHAPE=RECT COORDS=\"%d,%d,%d,%d\"",
					Elements[i]->Points[0].X, Elements[i]->Points[0].Y,
					Elements[i]->Points[1].X, Elements[i]->Points[1].Y);
				ptr += strlen(ptr);
				if (Elements[i]->URL && Elements[i]->URL[0])
				{
					sprintf(ptr, " HREF=\"%s\"", Elements[i]->URL);
					ptr += strlen(ptr);
				}
				else
				{
					sprintf(ptr, " NOHREF");
					ptr += strlen(ptr);
				}
				if (Elements[i]->AltText && Elements[i]->AltText[0])
				{
					sprintf(ptr, " ALT=\"%s\"", Elements[i]->AltText);
					ptr += strlen(ptr);
				}
				sprintf(ptr, ">\n");
				ptr += strlen(ptr);
				break;
			case G42MapElement::Circle:
				if (Elements[i]->Comment && Elements[i]->Comment[0])
				{
					sprintf(ptr, "<!-- %s -->\n", Elements[i]->Comment);
					ptr += strlen(ptr);
				}
				sprintf(ptr, "<AREA SHAPE=CIRCLE COORDS=\"%d,%d,%d\"",
					(Elements[i]->Points[0].X+Elements[i]->Points[1].X)/2,
					(Elements[i]->Points[0].Y+Elements[i]->Points[1].Y)/2,
					(abs(Elements[i]->Points[1].X-Elements[i]->Points[0].X))/2);
				ptr += strlen(ptr);
				if (Elements[i]->URL && Elements[i]->URL[0])
				{
					sprintf(ptr, " HREF=\"%s\"", Elements[i]->URL);
					ptr += strlen(ptr);
				}
				else
				{
					sprintf(ptr, " NOHREF");
					ptr += strlen(ptr);
				}
				if (Elements[i]->AltText && Elements[i]->AltText[0])
				{
					sprintf(ptr, " ALT=\"%s\"", Elements[i]->AltText);
					ptr += strlen(ptr);
				}
				sprintf(ptr, ">\n");
				ptr += strlen(ptr);
				break;
			case G42MapElement::Polygon:
			{
				if (Elements[i]->Comment && Elements[i]->Comment[0])
				{
					sprintf(ptr, "<!-- %s -->\n", Elements[i]->Comment);
					ptr += strlen(ptr);
				}
				sprintf(ptr, "<AREA SHAPE=POLYGON COORDS=\"%d,%d",
					Elements[i]->Points[0].X, Elements[i]->Points[0].Y);
				ptr += strlen(ptr);
				for(int j = 1; j < Elements[i]->NumPoints; j++)
				{
					sprintf(ptr, ",%d,%d", Elements[i]->Points[j].X,
						Elements[i]->Points[j].Y);
					ptr += strlen(ptr);
				}
				sprintf(ptr, "\"");
				ptr += strlen(ptr);
				if (Elements[i]->URL && Elements[i]->URL[0])
				{
					sprintf(ptr, " HREF=\"%s\"", Elements[i]->URL);
					ptr += strlen(ptr);
				}
				else
				{
					sprintf(ptr, " NOHREF");
					ptr += strlen(ptr);
				}
				if (Elements[i]->AltText && Elements[i]->AltText[0])
				{
					sprintf(ptr, " ALT=\"%s\"", Elements[i]->AltText);
					ptr += strlen(ptr);
				}
				sprintf(ptr, ">\n");
				ptr += strlen(ptr);
				break;
			}
			case G42MapElement::Default:
				// save default until end
				break;
			default:
				break;
		}
	}
	if (HasDefault())
	{
		if (Elements[DefaultIndex]->Comment
			&& Elements[DefaultIndex]->Comment[0])
		{
			sprintf(ptr, "<!-- %s -->\n", Elements[DefaultIndex]->Comment);
			ptr += strlen(ptr);
		}
		sprintf(ptr, "<AREA SHAPE=RECT COORDS=\"%d,%d,%d,%d\"",
			0, 0, ImageWidth, ImageHeight);
		ptr += strlen(ptr);
		if (Elements[DefaultIndex]->URL && Elements[DefaultIndex]->URL[0])
		{
			sprintf(ptr, " HREF=\"%s\"", Elements[DefaultIndex]->URL);
			ptr += strlen(ptr);
		}
		else
		{
			sprintf(ptr, " NOHREF");
			ptr += strlen(ptr);
		}
		if (Elements[DefaultIndex]->AltText && Elements[DefaultIndex]->AltText[0])
		{
			sprintf(ptr, " ALT=\"%s\"", Elements[DefaultIndex]->AltText);
			ptr += strlen(ptr);
		}
		sprintf(ptr, ">\n");
		ptr += strlen(ptr);
	}
	sprintf(ptr, "</MAP>\n");
//	ptr += strlen(ptr);
}
/****  G42MapElement Stuff  ***/
G42MapElement::G42MapElement (G42ElementType type, const char * url,
	const char * comment, const char * alt_text)
	: URL(0), Comment(0), AltText(0), Type(type), NumPoints(0)
{
	if (url && *url)
	{
		URL = new char[strlen(url) + 1];
		strcpy(URL, url);
	}
	if (comment && *comment)
	{
		Comment = new char[strlen(comment) + 1];
		strcpy(Comment, comment);
	}
	if (alt_text && *alt_text)
	{
		AltText = new char[strlen(alt_text) + 1];
		strcpy(AltText, alt_text);
	}
}
//G42ImageMap::G42MapElement::G42MapElement(const G42MapElement & ref) : URL(0), Comment(0), AltText(0), Type(Rectangle),
G42MapElement::G42MapElement(const G42MapElement & ref)
	: URL(0), Comment(0), AltText(0), Type(Rectangle),
	NumPoints(0)
{
	Type = ref.Type;
	NumPoints = ref.NumPoints;
	for (int i = 0; i < NumPoints; i++)
		Points[i] = ref.Points [i];
	
	if (ref.URL)
	{
		URL = new char [strlen (ref.URL)];
		strcpy (URL, ref.URL);
	}
	if (ref.Comment)
	{
		Comment = new char [strlen (ref.Comment)];
		strcpy (Comment, ref.Comment);
	}
	if (ref.AltText)
	{
		AltText = new char [strlen (ref.AltText)];
		strcpy (AltText, ref.AltText);
	}
}

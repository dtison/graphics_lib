// g42imap.cpp - image map class
#include <g42imap.h>
G42ImageMap::G42ImageMap(const char * file_name) :
	NumElements(0), Valid(false), Modified(false), DefaultValid(false),
	DefaultIndex(0), ImageWidth(0), ImageHeight(0), MapType(NCSA),
	SelectedElement(0), IsClientSide(false), ClientSideState(0), ClientSideDone(0),
	ClientSideShape(Rectangle), ClientSideHaveMap(false), ClientSideHaveShape(false),
	ClientSideHaveCoords(false), ClientSideToken(0), ClientSideNumPointArray(0),
	x1(0), y1(0), x2(0), y2(0), ClientSideName(0), Comment(0), AltText(0),
	SelectedElementValid(false), IsInternal(false), URL(0)
{
	FILE* fp = fopen(file_name, "r");
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
	Modified = false;
}
G42ImageMap::G42ImageMap(const char * map_buffer, uint buffer_length) :
	NumElements(0), Valid(false), Modified(false), DefaultValid(false),
	DefaultIndex(0), ImageWidth(0), ImageHeight(0), MapType(NCSA),
	SelectedElement(0), IsClientSide(false), ClientSideState(0), ClientSideDone(0),
	ClientSideShape(Rectangle), ClientSideHaveMap(false), ClientSideHaveShape(false),
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
	ClientSideShape(Rectangle), ClientSideHaveMap(false), ClientSideHaveShape(false),
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
int G42ImageMap::GetIndexAtPoint(int x, int y)
{
	//if (x < 0 || x >= ImageWidth || y < 0 || y >= ImageHeight)
		//return -1;
	int hit = -1;
	bool HasPoints = false;
	long Dist = 0x7FFFFFFF;
	int WhichPoint = -1;
	for (int i = 0; i < NumElements; i++)
	{
		switch(Elements[i]->Type)
		{
		case Point:
		{
			long TDist = (long)(x - Elements[i]->Coords[0].x)*
				(long)(x - Elements[i]->Coords[0].x) +
				(long)(y - Elements[i]->Coords[0].y)*
				(long)(y - Elements[i]->Coords[0].y);
			if (TDist < Dist)
			{
				Dist = TDist;
				WhichPoint = i;
			}
			HasPoints = true;		// MACOS: TRUE changed to true  D. Ison
		}
			break;
		case Rectangle:
			if ((x >= Elements[i]->Coords[0].x) &&
				(x <= Elements[i]->Coords[1].x)
				&& (y >= Elements[i]->Coords[0].y) &&
				(y <= Elements[i]->Coords[1].y))
			{
				hit = i;
				i = NumElements;
			}
			break;
		case Circle:
		{
			long CenterX = (Elements[i]->Coords[0].x +
				Elements[i]->Coords[1].x)>>1;
			long CenterY = (Elements[i]->Coords[0].y +
				Elements[i]->Coords[1].y)>>1;
			long Rad2 = (y - CenterY)*(y - CenterY) + (x - CenterX)*(x - CenterX);
			long Rad1 = (Elements[i]->Coords[1].x - Elements[i]->Coords[0].x)>>1;
			Rad1 = Rad1*Rad1;
			if (Rad2 <= Rad1)
			{
				hit = i;
				i = NumElements;
			}
			break;
		}
		case Polygon:
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
		else if (DefaultValid && Elements[DefaultIndex]->Type == Default)
			hit = DefaultIndex;
	}
   return hit;
#if 0
	if (x < 0 || x >= ImageWidth || y < 0 || y >= ImageHeight)
		return -1;
	int ret = -1;
	for (int i = 0; i < NumElements && ret < 0; i++)
	{
		switch (Elements[i]->Type)
		{
			case Rectangle:
				if (((x <= Elements[i]->Coords[0].x &&
					x >= Elements[i]->Coords[1].x) ||
					(x >= Elements[i]->Coords[0].x &&
					x <= Elements[i]->Coords[1].x)) &&
					((y <= Elements[i]->Coords[0].y &&
					y >= Elements[i]->Coords[1].y) ||
					(y >= Elements[i]->Coords[0].y &&
					y <= Elements[i]->Coords[1].y)))
						ret = i;
				break;
			case Circle:
				if (((x <= Elements[i]->Coords[0].x &&
					x >= Elements[i]->Coords[1].x) ||
					(x >= Elements[i]->Coords[0].x &&
					x <= Elements[i]->Coords[1].x)) &&
					((y <= Elements[i]->Coords[0].y &&
					y >= Elements[i]->Coords[1].y) ||
					(y >= Elements[i]->Coords[0].y &&
					y <= Elements[i]->Coords[1].y)))
						ret = i;
				break;
			case Point:
				if ((x <= Elements[i]->Coords[0].x + 5 &&
					x >= Elements[i]->Coords[0].x - 5) &&
					(y >= Elements[i]->Coords[0].y - 5 &&
					y <= Elements[i]->Coords[0].y + 5))
						ret = i;
				break;
			case Polygon:
				for (int j = 0; j < Elements[i]->NumCoords; j++)
				{
					if ((x <= Elements[i]->Coords[j].x + 5 &&
						x >= Elements[i]->Coords[j].x - 5) &&
						(y >= Elements[i]->Coords[j].y - 5 &&
						y <= Elements[i]->Coords[j].y + 5))
					{
						ret = i;
						break;
					}
				}
				break;
		}
	}
	if (ret < 0 && DefaultValid)
		ret = DefaultIndex;
	return ret;
#endif
}
void
G42ImageMap::Save(const char * file_name, G42MapType type)
{
	if (NumElements == 0)
		return;
	FILE* fp = fopen(file_name, "w+");
	if (type == NCSA)
		SaveNCSA(fp);
	else if (type == CERN)
		SaveCERN(fp);
	else if (type == ClientSide)
		SaveClientSide(file_name, fp);
	else if (type == Internal)
		SaveInternal(fp);
	fclose(fp);
	Modified = false;
}
void 
G42ImageMap::SaveNCSA(FILE *fp)
{
	for(int i = 0; i < NumElements; i++)
	{
		switch(Elements[i]->Type)
		{
		case Point:
			fprintf(fp, "#%s\n", Elements[i]->Comment);
			fprintf(fp, "point %s %d,%d\n", Elements[i]->URL,
				Elements[i]->Coords[0].x, Elements[i]->Coords[0].y);
			break;
		case Rectangle:
			fprintf(fp, "#%s\n", Elements[i]->Comment);
			fprintf(fp, "rect %s %d,%d %d,%d\n", Elements[i]->URL,
				Elements[i]->Coords[0].x, Elements[i]->Coords[0].y,
				Elements[i]->Coords[1].x, Elements[i]->Coords[1].y);
			break;
		case Circle:
			fprintf(fp, "#%s\n", Elements[i]->Comment);
			fprintf(fp, "circle %s %d,%d %d,%d\n", Elements[i]->URL,
				(Elements[i]->Coords[0].x+Elements[i]->Coords[1].x)/2,
				(Elements[i]->Coords[0].y+Elements[i]->Coords[1].y)/2,
				(Elements[i]->Coords[0].x+Elements[i]->Coords[1].x)/2,
				Elements[i]->Coords[0].y);
			break;
		case Polygon:
			{
			fprintf(fp, "#%s\n", Elements[i]->Comment);
			fprintf(fp, "poly %s", Elements[i]->URL);
			for(int j = 0; j < Elements[i]->NumCoords; j++)
				fprintf(fp, " %d,%d", Elements[i]->Coords[j].x,
					Elements[i]->Coords[j].y);
			fprintf(fp, "\n");
			}
			break;
		case Default:
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
		case Rectangle:
			fprintf(fp, "#%s\n", Elements[i]->Comment);
			fprintf(fp, "rectangle (%d,%d) (%d,%d) %s\n",
				Elements[i]->Coords[0].x, Elements[i]->Coords[0].y,
				Elements[i]->Coords[1].x, Elements[i]->Coords[1].y,
				Elements[i]->URL);
			break;
		case Circle:
			fprintf(fp, "#%s\n", Elements[i]->Comment);
			fprintf(fp, "circle (%d,%d) %d %s\n",
				(Elements[i]->Coords[0].x+Elements[i]->Coords[1].x)/2,
				(Elements[i]->Coords[0].y+Elements[i]->Coords[1].y)/2,
				(abs(Elements[i]->Coords[1].x-Elements[i]->Coords[0].x))/2,
				Elements[i]->URL);
			break;
		case Polygon:
			{
			fprintf(fp, "#%s\n", Elements[i]->Comment);
			fprintf(fp, "polygon");
			for(int j = 0; j < Elements[i]->NumCoords; j++)
				fprintf(fp, " (%d,%d)", Elements[i]->Coords[j].x,
					Elements[i]->Coords[j].y);
			fprintf(fp, " %s\n", Elements[i]->URL);
			}
			break;
		case Default:
			fprintf(fp, "#%s\n", Elements[i]->Comment);
			fprintf(fp, "default %s\n", Elements[i]->URL);
			break;
		default:
			break;
		}
	}
}
void
G42ImageMap::SaveClientSide(const char * file_name, FILE *fp)
{
	if (!ClientSideName)
	{
		char tfname [256];
		strcpy(tfname, file_name);
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
			case Rectangle:
				if (Elements[i]->Comment && Elements[i]->Comment[0])
				{
					fprintf(fp, "<!-- %s -->\n", Elements[i]->Comment);
				}
				fprintf(fp, "<AREA SHAPE=RECT COORDS=\"%d,%d,%d,%d\"",
					Elements[i]->Coords[0].x, Elements[i]->Coords[0].y,
					Elements[i]->Coords[1].x, Elements[i]->Coords[1].y);
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
			case Circle:
				if (Elements[i]->Comment && Elements[i]->Comment[0])
				{
					fprintf(fp, "<!-- %s -->\n", Elements[i]->Comment);
				}
				fprintf(fp, "<AREA SHAPE=CIRCLE COORDS=\"%d,%d,%d\"",
					(Elements[i]->Coords[0].x+Elements[i]->Coords[1].x)/2,
					(Elements[i]->Coords[0].y+Elements[i]->Coords[1].y)/2,
					(abs(Elements[i]->Coords[1].x-Elements[i]->Coords[0].x))/2);
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
			case Polygon:
			{
				if (Elements[i]->Comment && Elements[i]->Comment[0])
				{
					fprintf(fp, "<!-- %s -->\n", Elements[i]->Comment);
				}
				fprintf(fp, "<AREA SHAPE=POLYGON COORDS=\"%d,%d",
					Elements[i]->Coords[0].x, Elements[i]->Coords[0].y);
				for(int j = 1; j < Elements[i]->NumCoords; j++)
				{
					fprintf(fp, ",%d,%d", Elements[i]->Coords[j].x,
						Elements[i]->Coords[j].y);
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
			case Default:
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
		case Point:
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
				Elements[i]->Coords[0].x, Elements[i]->Coords[0].y);
			break;
		case Rectangle:
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
				Elements[i]->Coords[0].x, Elements[i]->Coords[0].y,
				Elements[i]->Coords[1].x, Elements[i]->Coords[1].y);
			break;
		case Circle:
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
				Elements[i]->Coords[0].x, Elements[i]->Coords[0].y,
				Elements[i]->Coords[1].x, Elements[i]->Coords[1].y);
			break;
		case Polygon:
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
			for(int j = 0; j < Elements[i]->NumCoords; j++)
			{
				if (j)
					fprintf(fp, ",");
				fprintf(fp, "%d,%d", Elements[i]->Coords[j].x,
					Elements[i]->Coords[j].y);
			}
			fprintf(fp, "\n");
			break;
		}
		case Default:
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
void G42ImageMap::Save(char * map_buffer, uint max_buffer_length,
	G42MapType type)
{
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
		case Point:
			sprintf(ptr, "#%s\n", Elements[i]->Comment);
			ptr += strlen(ptr);
			sprintf(ptr, "point %s %d,%d\n", Elements[i]->URL,
				Elements[i]->Coords[0].x, Elements[i]->Coords[0].y);
			ptr += strlen(ptr);
			break;
		case Rectangle:
			sprintf(ptr, "#%s\n", Elements[i]->Comment);
			ptr += strlen(ptr);
			sprintf(ptr, "rect %s %d,%d %d,%d\n", Elements[i]->URL,
				Elements[i]->Coords[0].x, Elements[i]->Coords[0].y,
				Elements[i]->Coords[1].x, Elements[i]->Coords[1].y);
			ptr += strlen(ptr);
			break;
		case Circle:
			sprintf(ptr, "#%s\n", Elements[i]->Comment);
			ptr += strlen(ptr);
			sprintf(ptr, "circle %s %d,%d %d,%d\n", Elements[i]->URL,
				(Elements[i]->Coords[0].x+Elements[i]->Coords[1].x)/2,
				(Elements[i]->Coords[0].y+Elements[i]->Coords[1].y)/2,
				(Elements[i]->Coords[0].x+Elements[i]->Coords[1].x)/2,
				Elements[i]->Coords[0].y);
			ptr += strlen(ptr);
			break;
		case Polygon:
			{
			sprintf(ptr, "#%s\n", Elements[i]->Comment);
			ptr += strlen(ptr);
			sprintf(ptr, "poly %s", Elements[i]->URL);
			ptr += strlen(ptr);
			for(int j = 0; j < Elements[i]->NumCoords; j++)
			{
				sprintf(ptr, " %d,%d", Elements[i]->Coords[j].x,
					Elements[i]->Coords[j].y);
				ptr += strlen(ptr);
			}
			sprintf(ptr, "\n");
			}
			break;
		case Default:
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
		case Rectangle:
			sprintf(ptr, "#%s\n", Elements[i]->Comment);
			ptr += strlen(ptr);
			sprintf(ptr, "rectangle (%d,%d) (%d,%d) %s\n",
				Elements[i]->Coords[0].x, Elements[i]->Coords[0].y,
				Elements[i]->Coords[1].x, Elements[i]->Coords[1].y,
				Elements[i]->URL);
			ptr += strlen(ptr);
			break;
		case Circle:
			sprintf(ptr, "#%s\n", Elements[i]->Comment);
			ptr += strlen(ptr);
			sprintf(ptr, "circle (%d,%d) %d %s\n",
				(Elements[i]->Coords[0].x+Elements[i]->Coords[1].x)/2,
				(Elements[i]->Coords[0].y+Elements[i]->Coords[1].y)/2,
				(abs(Elements[i]->Coords[1].x-Elements[i]->Coords[0].x))/2,
				Elements[i]->URL);
			ptr += strlen(ptr);
			break;
		case Polygon:
			{
			sprintf(ptr, "#%s\n", Elements[i]->Comment);
			ptr += strlen(ptr);
			sprintf(ptr, "polygon");
			ptr += strlen(ptr);
			for(int j = 0; j < Elements[i]->NumCoords; j++)
			{
				sprintf(ptr, " (%d,%d)", Elements[i]->Coords[j].x,
					Elements[i]->Coords[j].y);
			}
			sprintf(ptr, " %s\n", Elements[i]->URL);
			ptr += strlen(ptr);
			}
			break;
		case Default:
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
			case Rectangle:
				if (Elements[i]->Comment && Elements[i]->Comment[0])
				{
					sprintf(ptr, "<!-- %s -->\n", Elements[i]->Comment);
					ptr += strlen(ptr);
				}
				sprintf(ptr, "<AREA SHAPE=RECT COORDS=\"%d,%d,%d,%d\"",
					Elements[i]->Coords[0].x, Elements[i]->Coords[0].y,
					Elements[i]->Coords[1].x, Elements[i]->Coords[1].y);
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
			case Circle:
				if (Elements[i]->Comment && Elements[i]->Comment[0])
				{
					sprintf(ptr, "<!-- %s -->\n", Elements[i]->Comment);
					ptr += strlen(ptr);
				}
				sprintf(ptr, "<AREA SHAPE=CIRCLE COORDS=\"%d,%d,%d\"",
					(Elements[i]->Coords[0].x+Elements[i]->Coords[1].x)/2,
					(Elements[i]->Coords[0].y+Elements[i]->Coords[1].y)/2,
					(abs(Elements[i]->Coords[1].x-Elements[i]->Coords[0].x))/2);
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
			case Polygon:
			{
				if (Elements[i]->Comment && Elements[i]->Comment[0])
				{
					sprintf(ptr, "<!-- %s -->\n", Elements[i]->Comment);
					ptr += strlen(ptr);
				}
				sprintf(ptr, "<AREA SHAPE=POLYGON COORDS=\"%d,%d",
					Elements[i]->Coords[0].x, Elements[i]->Coords[0].y);
				ptr += strlen(ptr);
				for(int j = 1; j < Elements[i]->NumCoords; j++)
				{
					sprintf(ptr, ",%d,%d", Elements[i]->Coords[j].x,
						Elements[i]->Coords[j].y);
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
			case Default:
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

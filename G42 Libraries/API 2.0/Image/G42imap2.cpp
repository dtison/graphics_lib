// g42imap2.cpp - image map class
#include <g42imap.h>
void
G42ImageMap::SaveInternal(char *ptr)
{
	strcpy(ptr, "g42imap\n"); // mark as our internal format
	ptr += strlen(ptr);
	if (ClientSideName && *ClientSideName)
	{
		sprintf(ptr, "n%s\n", ClientSideName);
		ptr += strlen(ptr);
	}
	if (ImageWidth && ImageHeight)
	{
		sprintf(ptr, "w%d\n", ImageWidth);
		ptr += strlen(ptr);
		sprintf(ptr, "h%d\n", ImageHeight);
		ptr += strlen(ptr);
	}
	for (int i = 0; i < NumElements; i++)
	{
		switch (Elements[i]->Type)
		{
		case Point:
			if (Elements[i]->Comment && Elements[i]->Comment[0])
			{
				sprintf(ptr, "c%s\n", Elements[i]->Comment);
				ptr += strlen(ptr);
			}
			if (Elements[i]->URL && Elements[i]->URL[0])
			{
				sprintf(ptr, "u%s\n", Elements[i]->URL);
				ptr += strlen(ptr);
			}
			if (Elements[i]->AltText && Elements[i]->AltText[0])
			{
				sprintf(ptr, "a%s\n", Elements[i]->AltText);
				ptr += strlen(ptr);
			}
			sprintf(ptr, "t%d,%d\n", Elements[i]->Coords[0].x, Elements[i]->Coords[0].y);
			ptr += strlen(ptr);
			break;
		case Rectangle:
			if (Elements[i]->Comment && Elements[i]->Comment[0])
			{
				sprintf(ptr, "c%s\n", Elements[i]->Comment);
				ptr += strlen(ptr);
			}
			if (Elements[i]->URL && Elements[i]->URL[0])
			{
				sprintf(ptr, "u%s\n", Elements[i]->URL);
				ptr += strlen(ptr);
			}
			if (Elements[i]->AltText && Elements[i]->AltText[0])
			{
				sprintf(ptr, "a%s\n", Elements[i]->AltText);
				ptr += strlen(ptr);
			}
			sprintf(ptr, "r%d,%d,%d,%d\n",
				Elements[i]->Coords[0].x, Elements[i]->Coords[0].y,
				Elements[i]->Coords[1].x, Elements[i]->Coords[1].y);
			ptr += strlen(ptr);
			break;
		case Circle:
			if (Elements[i]->Comment && Elements[i]->Comment[0])
			{
				sprintf(ptr, "c%s\n", Elements[i]->Comment);
				ptr += strlen(ptr);
			}
			if (Elements[i]->URL && Elements[i]->URL[0])
			{
				sprintf(ptr, "u%s\n", Elements[i]->URL);
				ptr += strlen(ptr);
			}
			if (Elements[i]->AltText && Elements[i]->AltText[0])
			{
				sprintf(ptr, "a%s\n", Elements[i]->AltText);
				ptr += strlen(ptr);
			}
			sprintf(ptr, "o%d,%d,%d,%d\n",
				Elements[i]->Coords[0].x, Elements[i]->Coords[0].y,
				Elements[i]->Coords[1].x, Elements[i]->Coords[1].y);
			ptr += strlen(ptr);
			break;
		case Polygon:
		{
			if (Elements[i]->Comment && Elements[i]->Comment[0])
			{
				sprintf(ptr, "c%s\n", Elements[i]->Comment);
				ptr += strlen(ptr);
			}
			if (Elements[i]->URL && Elements[i]->URL[0])
			{
				sprintf(ptr, "u%s\n", Elements[i]->URL);
				ptr += strlen(ptr);
			}
			if (Elements[i]->AltText && Elements[i]->AltText[0])
			{
				sprintf(ptr, "a%s\n", Elements[i]->AltText);
				ptr += strlen(ptr);
			}
			sprintf(ptr, "p");
			ptr += strlen(ptr);
			for(int j = 0; j < Elements[i]->NumCoords; j++)
			{
				if (j)
					*ptr++ = ',';
				sprintf(ptr, "%d,%d", Elements[i]->Coords[j].x,
					Elements[i]->Coords[j].y);
				ptr += strlen(ptr);
			}
			sprintf(ptr, "\n");
			ptr += strlen(ptr);
			break;
		}
		case Default:
			if (Elements[i]->Comment && Elements[i]->Comment[0])
			{
				sprintf(ptr, "c%s\n", Elements[i]->Comment);
				ptr += strlen(ptr);
			}
			if (Elements[i]->URL && Elements[i]->URL[0])
			{
				sprintf(ptr, "u%s\n", Elements[i]->URL);
				ptr += strlen(ptr);
			}
			if (Elements[i]->AltText && Elements[i]->AltText[0])
			{
				sprintf(ptr, "a%s\n", Elements[i]->AltText);
				ptr += strlen(ptr);
			}
			sprintf(ptr, "d\n", Elements[i]->URL);
			ptr += strlen(ptr);
			break;
		default:
			break;
		}
	}
}
void G42ImageMap::SetImageSize(uint width, uint height)
{
	ImageWidth = width;
	ImageHeight = height;
}
void G42ImageMap::SetElement(uint /* index */, G42MapElement & element)
{
	AddElement(element.Type, element.Coords, element.NumCoords,
		element.URL, element.Comment, element.AltText);
}
void G42ImageMap::SetElementURL(uint index, const char * url)
{
	if (index >= NumElements)
		return;
	if (Elements[index]->URL)
		delete [] Elements[index]->URL;
	if (url)
	{
		Elements[index]->URL = new char [strlen(url) + 1];
		strcpy(Elements[index]->URL, url);
	}
	else
	{
		Elements[index]->URL = new char [1];
		Elements[index]->URL[0] = '\0';
	}
	Modified = true;
}
void G42ImageMap::SetElementComment(uint index, const char * comment)
{
	if (index >= NumElements)
		return;
	if (Elements[index]->Comment)
		delete [] Elements[index]->Comment;
	if (comment)
	{
		Elements[index]->Comment = new char [strlen(comment) + 1];
		strcpy(Elements[index]->Comment, comment);
	}
	else
	{
		Elements[index]->Comment = new char [1];
		Elements[index]->Comment[0] = '\0';
	}
	Modified = true;
}
void G42ImageMap::SetElementAltText(uint index, const char * alt_text)
{
	if (index >= NumElements)
		return;
	if (Elements[index]->AltText)
		delete [] Elements[index]->AltText;
	if (alt_text)
	{
		Elements[index]->AltText = new char [strlen(alt_text) + 1];
		strcpy(Elements[index]->AltText, alt_text);
	}
	else
	{
		Elements[index]->AltText = new char [1];
		Elements[index]->AltText[0] = '\0';
	}
	Modified = true;
}
void
G42ImageMap::MoveElement(uint index, int dx, int dy)
{
	if (index >= NumElements || !Elements[index] ||
		Elements[index]->Type == Default)
			return;
	for (uint i = 0; i < Elements[index]->NumCoords; i++)
	{
		Elements[index]->Coords[i].x += dx;
		Elements[index]->Coords[i].y += dy;
	}
	Modified = true;
}
void
G42ImageMap::StretchElement(uint index, uint new_width, uint new_height,
	int dx, int dy)
{
	if (index >= NumElements || !Elements[index])
		return;
	switch (Elements[index]->Type)
	{
		case Circle:
		{
			if (new_width != new_height)
			{
				if (new_width < new_height)
					new_height = new_width;
				else
					new_width = new_height;
			}
		}
		// fall through to rectangle
		case Rectangle:
		case Polygon:
		{
			int old_width = Elements[index]->Coords[0].x;
			int old_height = Elements[index]->Coords[0].y;
			int old_x = Elements[index]->Coords[0].x;
			int old_y = Elements[index]->Coords[0].y;
			uint i;	// MACOS: "i not declared" error below  D. Ison
			for (i = 1; i < Elements[index]->NumCoords; i++)
			{
				if (Elements[index]->Coords[i].x < old_x)
					old_x = Elements[index]->Coords[i].x;
				if (Elements[index]->Coords[i].x > old_width)
					old_width = Elements[index]->Coords[i].x;
				if (Elements[index]->Coords[i].y < old_y)
					old_y = Elements[index]->Coords[i].y;
				if (Elements[index]->Coords[i].y > old_height)
					old_height = Elements[index]->Coords[i].y;
			}
			old_width -= old_x - 1;
			old_height -= old_y - 1;
			for (i = 0; i < Elements[index]->NumCoords; i++)
			{
				Elements[index]->Coords[i].x =
					(uint)((int)(((int32)(Elements[index]->Coords[i].x - old_x) *
					(int32)(new_width - 1)) / (int32)(old_width - 1)) +
					(int)old_x + dx);
				Elements[index]->Coords[i].y =
					(uint)((int)(((int32)(Elements[index]->Coords[i].y - old_y) *
					(int32)(new_height - 1)) / (int32)(old_height - 1)) +
					(int)old_y + dy);
			}
			break;
		}
		case Point:
		{
			Elements[index]->Coords[0].x += dx;
			Elements[index]->Coords[0].y += dy;
		}
		default:
			break;
	}
	Modified = true;
}
void
G42ImageMap::SetPolygonPoint(uint index, uint point, int x, int y)
{
	if (index >= NumElements || !Elements[index] ||
			Elements[index]->Type != Polygon ||
			point >= Elements[index]->NumCoords)
		return;
	Elements[index]->Coords[point].x = x;
	Elements[index]->Coords[point].y = y;
	Modified = true;
}
void
G42ImageMap::DeleteSelectedElement(void)
{
	if (!SelectedElementValid || SelectedElement >= NumElements)
		return;
	DeleteElement(SelectedElement);
	if (SelectedElement >= NumElements)
		SelectedElement = (NumElements - 1);
}
void
G42ImageMap::DeleteElement(uint index)
{
	if (index >= NumElements)
		return;
	if (Elements[index]->Type == Default)
		DefaultValid = false;
	if (SelectedElement > index)
		SelectedElement--;
	delete Elements[index];
	for(int i = index; i< (NumElements-1); i++)
		Elements[i] = Elements[i+1];
	NumElements--;
	Elements[NumElements] = 0;
	Modified = true;
	if (SelectedElement >= NumElements)
   	SelectedElementValid = false;
}
bool
G42ImageMap::GetTwoDigits(char** ptr, int* x, int* y)
{
	char* tmp = *ptr;
	while (isspace(*tmp)) tmp++;
	*x = atoi(*ptr);
	while (isdigit(*tmp) || *tmp == '-') tmp++;
	while (isspace(*tmp)) tmp++;
	if (*tmp != ',')
	{
		*ptr = tmp;
		return false;
	}
	tmp++;
	while (isspace(*tmp)) tmp++;
	*y = atoi(tmp);
	while (isdigit(*tmp) || *tmp == '-') tmp++;
	*ptr = tmp;
	return true;
}
void
G42ImageMap::AddElement(G42ElementType T, G42Array<G42Coord> & PtIn, int nPts,
	char* URL, char* Comment, char * alt_text, int N)
{
	int i;	// For MWerks compiler D.I.
	if (N == -1)
	{
		if (SelectedElement == NumElements)
			SelectedElement++;
		Elements[NumElements] = new G42MapElement;
		Elements[NumElements]->Type = T;
		if (URL)
		{
			Elements[NumElements]->URL = new char[strlen(URL) + 1];
			strcpy(Elements[NumElements]->URL, URL);
		}
		else
		{
			Elements[NumElements]->URL = new char[1];
			Elements[NumElements]->URL[0] = '\0';
		}
		if (Comment)
		{
			Elements[NumElements]->Comment = new char[strlen(Comment) + 1];
			strcpy(Elements[NumElements]->Comment, Comment);
		}
		else
		{
			Elements[NumElements]->Comment = new char[1];
			Elements[NumElements]->Comment[0] = '\0';
		}
		if (alt_text)
		{
			Elements[NumElements]->AltText = new char[strlen(alt_text) + 1];
			strcpy(Elements[NumElements]->AltText, alt_text);
		}
		else
		{
			Elements[NumElements]->AltText = new char[1];
			Elements[NumElements]->AltText[0] = '\0';
		}
      if (T == Rectangle)
      {
      	int x_min = PtIn[0].x;
         int y_min = PtIn[0].y;
         int x_max = PtIn[1].x;
         int y_max = PtIn[1].y;
         if (x_max < x_min)
         {
         	x_min = x_max;
            x_max = PtIn[0].x;
         }
         if (y_max < y_min)
         {
         	y_min = y_max;
            y_max = PtIn[0].y;
         }
         Elements[NumElements]->Coords[0].x = x_min;
         Elements[NumElements]->Coords[0].y = y_min;
         Elements[NumElements]->Coords[1].x = x_max;
         Elements[NumElements]->Coords[1].y = y_max;
      }
      else
		{
			for (int i=0; i<nPts; i++)
			{
				Elements[NumElements]->Coords[i].x = PtIn[i].x;
				Elements[NumElements]->Coords[i].y = PtIn[i].y;
			}
      }
		Elements[NumElements]->NumCoords = nPts;
		if (T == Default)
		{
			DefaultValid = true;
			DefaultIndex = NumElements;
		}
		NumElements++;
	}
	else
	{
		if (SelectedElement >= N)
			SelectedElement++;
		for(int i=NumElements-1; i>=N; i--)
			Elements[i+1] = Elements[i];
		Elements[N] = new G42MapElement;
		Elements[N]->Type = T;
		if (URL)
		{
			Elements[N]->URL = new char[strlen(URL) + 1];
			strcpy(Elements[N]->URL, URL);
		}
		else
		{
			Elements[N]->URL = new char[1];
			Elements[N]->URL[0] = '\0';
		}
		if (Comment)
		{
			Elements[N]->Comment = new char[strlen(Comment) + 1];
			strcpy(Elements[N]->Comment, Comment);
		}
		else
		{
			Elements[N]->Comment = new char[1];
			Elements[N]->Comment[0] = '\0';
		}
		if (alt_text)
		{
			Elements[N]->AltText = new char[strlen(alt_text) + 1];
			strcpy(Elements[N]->AltText, alt_text);
		}
		else
		{
			Elements[N]->AltText = new char[1];
			Elements[N]->AltText[0] = '\0';
		}
		for (i=0; i<nPts; i++)
		{
			Elements[N]->Coords[i].x = PtIn[i].x;
			Elements[N]->Coords[i].y = PtIn[i].y;
		}
		Elements[N]->NumCoords = nPts;
		if (HasDefault() && DefaultIndex >= N)
			DefaultIndex++;
		if (T == Default)
		{
			DefaultValid = true;
			DefaultIndex = N;
		}
		NumElements++;
	}
	Modified = true;
}
void
G42ImageMap::AddElement(G42ElementType T, int x, int y, int x2, int y2,
	char* URL, char* Comment, char * alt_text, int N)
{
	if (N == -1)
	{
		if (SelectedElement == NumElements)
			SelectedElement++;
		Elements[NumElements] = new G42MapElement;
		Elements[NumElements]->Type = T;
		if (URL)
		{
			Elements[NumElements]->URL = new char[strlen(URL) + 1];
			strcpy(Elements[NumElements]->URL, URL);
		}
		else
		{
			Elements[NumElements]->URL = new char[1];
			Elements[NumElements]->URL[0] = '\0';
		}
		if (Comment)
		{
			Elements[NumElements]->Comment = new char[strlen(Comment) + 1];
			strcpy(Elements[NumElements]->Comment, Comment);
		}
		else
		{
			Elements[NumElements]->Comment = new char[1];
			Elements[NumElements]->Comment[0] = '\0';
		}
		if (alt_text)
		{
			Elements[NumElements]->AltText = new char[strlen(alt_text) + 1];
			strcpy(Elements[NumElements]->AltText, alt_text);
		}
		else
		{
			Elements[NumElements]->AltText = new char[1];
			Elements[NumElements]->AltText[0] = '\0';
		}
		Elements[NumElements]->Coords[0].x = x;
		Elements[NumElements]->Coords[0].y = y;
		Elements[NumElements]->Coords[1].x = x2;
		Elements[NumElements]->Coords[1].y = y2;
		Elements[NumElements]->NumCoords = 2;
		if (T == Default)
		{
			DefaultValid = true;
			DefaultIndex = NumElements;
		}
		NumElements++;
	}
	else
	{
		if (SelectedElement >= N)
			SelectedElement++;
		for(int i=NumElements-1; i>=N; i--)
			Elements[i+1] = Elements[i];
		Elements[N] = new G42MapElement;
		Elements[N]->Type = T;
		if (URL)
		{
			Elements[N]->URL = new char[strlen(URL) + 1];
			strcpy(Elements[N]->URL, URL);
		}
		else
		{
			Elements[N]->URL = new char[1];
			Elements[N]->URL[0] = '\0';
		}
		if (Comment)
		{
			Elements[N]->Comment = new char[strlen(Comment) + 1];
			strcpy(Elements[N]->Comment, Comment);
		}
		else
		{
			Elements[N]->Comment = new char[1];
			Elements[N]->Comment[0] = '\0';
		}
		if (alt_text)
		{
			Elements[N]->AltText = new char[strlen(alt_text) + 1];
			strcpy(Elements[N]->AltText, alt_text);
		}
		else
		{
			Elements[N]->AltText = new char[1];
			Elements[N]->AltText[0] = '\0';
		}
		Elements[N]->Coords[0].x = x;
		Elements[N]->Coords[0].y = y;
		Elements[N]->Coords[1].x = x2;
		Elements[N]->Coords[1].y = y2;
		Elements[N]->NumCoords = 2;
		if (HasDefault() && DefaultIndex >= N)
			DefaultIndex++;
		if (T == Default)
		{
			DefaultValid = true;
			DefaultIndex = N;
		}
		NumElements++;
	}
	Modified = true;
}
void
G42ImageMap::MoveElementPos(uint new_pos, uint old_pos)
{
	if (new_pos >= NumElements || old_pos >= NumElements ||
		old_pos == new_pos)
			return;
	G42MapElement * element = Elements[old_pos];
	if (old_pos > new_pos)
	{
		for (uint i = old_pos; i > new_pos; i--)
			Elements[i] = Elements[i - 1];
		if (SelectedElementValid && SelectedElement == old_pos)
			SelectedElement = new_pos;
		else if (SelectedElementValid && SelectedElement >= new_pos &&
			SelectedElement < old_pos)
				SelectedElement++;
		if (DefaultValid && DefaultIndex == old_pos)
			DefaultIndex = new_pos;
		else if (DefaultValid && DefaultIndex >= new_pos &&
			DefaultIndex < old_pos)
				DefaultIndex++;
	}
	else
	{
		for (uint i = old_pos; i < new_pos; i++)
			Elements[i] = Elements[i + 1];
		if (SelectedElementValid && SelectedElement == old_pos)
			SelectedElement = new_pos;
		else if (SelectedElementValid && SelectedElement > old_pos &&
			SelectedElement <= new_pos)
				SelectedElement--;
		if (DefaultValid && DefaultIndex == old_pos)
			DefaultIndex = new_pos;
		else if (DefaultValid && DefaultIndex > old_pos &&
			DefaultIndex <= new_pos)
				DefaultIndex--;
	}
	Elements[new_pos] = element;
}
void
G42ImageMap::SelectElement(uint index)
{
	if (index >= NumElements)
	{
		SelectedElement = NumElements;
		SelectedElementValid = false;
	}
	else
	{
		SelectedElement = index;
		SelectedElementValid = true;
	}
}
void
G42ImageMap::ClearSelectedElement(void)
{
	SelectedElement = NumElements;
   SelectedElementValid = false;
}
bool
G42ImageMap::PointInPoly(int X, int Y, int Index)
{
		  int i, numverts, inside_flag, xflag0;
		  int crossings;
		  double *p, *stop;
		  double tx, ty, y;
	double pgon[100][2];
		  for (i = 0; i<Elements[Index]->NumCoords; i++)
		  {
				pgon[i][0] = (double)Elements[Index]->Coords[i].x;
				pgon[i][1] = (double)Elements[Index]->Coords[i].y;
		  }
					 ;
		  numverts = Elements[Index]->NumCoords;
		  crossings = 0;
		  tx = X;
		  ty = Y;
		  y = pgon[numverts - 1][1];
		  p = (double *) pgon + 1;
		  if ((y >= ty) != (*p >= ty)) {
					 if ((xflag0 = (pgon[numverts - 1][0] >= tx)) ==
					 (*(double *) pgon >= tx)) {
								if (xflag0)
										  crossings++;
					 }
					 else {
								crossings += (pgon[numverts - 1][0] - (y - ty) *
                        (*(double *) pgon - pgon[numverts - 1][0]) /
                        (*p - y)) >= tx;
                }
        }
		  stop = pgon[numverts];
        for (y = *p, p += 2; p < stop; y = *p, p += 2) {
                if (y >= ty) {
                        while ((p < stop) && (*p >= ty))
                                p += 2;
                        if (p >= stop)
                                break;
								if ((xflag0 = (*(p - 3) >= tx)) == (*(p - 1) >= tx)) {
                                if (xflag0)
                                        crossings++;
                        }
								else {
                                crossings += (*(p - 3) - (*(p - 2) - ty) *
                                (*(p - 1) - *(p - 3)) / (*p - *(p - 2))) >= tx;
                        }
                }
                else {
								while ((p < stop) && (*p < ty))
                                p += 2;
                        if (p >= stop)
                                break;
                        if ((xflag0 = (*(p - 3) >= tx)) == (*(p - 1) >= tx)) {
                                if (xflag0)
													 crossings++;
                        }
                        else {
                                crossings += (*(p - 3) - (*(p - 2) - ty) *
                                (*(p - 1) - *(p - 3)) / (*p - *(p - 2))) >= tx;
                        }
					 }
        }
        inside_flag = crossings & 0x01;
        return (inside_flag);
}

// gimap2.cpp - image map class
#include "gimap.h"
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
		case G42MapElement::Point:
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
			sprintf(ptr, "t%d,%d\n", Elements[i]->Points[0].X, Elements[i]->Points[0].Y);
			ptr += strlen(ptr);
			break;
		case G42MapElement::Rectangle:
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
				Elements[i]->Points[0].X, Elements[i]->Points[0].Y,
				Elements[i]->Points[1].X, Elements[i]->Points[1].Y);
			ptr += strlen(ptr);
			break;
		case G42MapElement::Circle:
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
				Elements[i]->Points[0].X, Elements[i]->Points[0].Y,
				Elements[i]->Points[1].X, Elements[i]->Points[1].Y);
			ptr += strlen(ptr);
			break;
		case G42MapElement::Polygon:
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
			for(int j = 0; j < Elements[i]->NumPoints; j++)
			{
				if (j)
					*ptr++ = ',';
				sprintf(ptr, "%d,%d", Elements[i]->Points[j].X,
					Elements[i]->Points[j].Y);
				ptr += strlen(ptr);
			}
			sprintf(ptr, "\n");
			ptr += strlen(ptr);
			break;
		}
		case G42MapElement::Default:
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
void G42ImageMap::SetImageSize(uintf width, uintf height)
{
	ImageWidth = width;
	ImageHeight = height;
}
void G42ImageMap::AddElement(const G42MapElement & element, int16 N)
{
	AddElement(element.Type, element.Points, element.NumPoints,
		element.URL, element.Comment, element.AltText, N);
}
void G42ImageMap::SetElement(int16 index, const G42MapElement & element)
{
	DeleteElement(index);
	AddElement(element.Type, element.Points, element.NumPoints,
		element.URL, element.Comment, element.AltText, index);
}
void G42ImageMap::SetElementURL(int16 index, const char * url)
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
void G42ImageMap::SetElementComment(int16 index, const char * comment)
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
void G42ImageMap::SetElementAltText(int16 index, const char * alt_text)
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
G42ImageMap::MoveElement(int16 index, int dx, int dy)
{
	if (index >= NumElements || !Elements[index] ||
		Elements[index]->Type == G42MapElement::Default)
			return;
	for (uintf i = 0; i < Elements[index]->NumPoints; i++)
	{
		Elements[index]->Points[i].X += dx;
		Elements[index]->Points[i].Y += dy;
	}
	Modified = true;
}
void
G42ImageMap::StretchElement(int16 index, uintf new_width, uintf new_height,
	int dx, int dy)
{
	if (index >= NumElements || !Elements[index])
		return;
	switch (Elements[index]->Type)
	{
		case G42MapElement::Circle:
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
		case G42MapElement::Rectangle:
		case G42MapElement::Polygon:
		{
			int old_width = Elements[index]->Points[0].X;
			int old_height = Elements[index]->Points[0].Y;
			int old_x = Elements[index]->Points[0].X;
			int old_y = Elements[index]->Points[0].Y;
			uintf i;	// MACOS: "i not declared" error below  D. Ison
			for (i = 1; i < Elements[index]->NumPoints; i++)
			{
				if (Elements[index]->Points[i].X < old_x)
					old_x = Elements[index]->Points[i].X;
				if (Elements[index]->Points[i].X > old_width)
					old_width = Elements[index]->Points[i].X;
				if (Elements[index]->Points[i].Y < old_y)
					old_y = Elements[index]->Points[i].Y;
				if (Elements[index]->Points[i].Y > old_height)
					old_height = Elements[index]->Points[i].Y;
			}
			old_width -= old_x - 1;
			old_height -= old_y - 1;
			for (i = 0; i < Elements[index]->NumPoints; i++)
			{
				Elements[index]->Points[i].X =
					(uintf)((int)(((int32)(Elements[index]->Points[i].X - old_x) *
					(int32)(new_width - 1)) / (int32)(old_width - 1)) +
					(int)old_x + dx);
				Elements[index]->Points[i].Y =
					(uintf)((int)(((int32)(Elements[index]->Points[i].Y - old_y) *
					(int32)(new_height - 1)) / (int32)(old_height - 1)) +
					(int)old_y + dy);
			}
			break;
		}
		case G42MapElement::Point:
		{
			Elements[index]->Points[0].X += dx;
			Elements[index]->Points[0].Y += dy;
		}
		default:
			break;
	}
	Modified = true;
}
void
G42ImageMap::SetPolygonPoint(int16 index, uintf point, int x, int y)
{
	if (index >= NumElements || !Elements[index] ||
			Elements[index]->Type != G42MapElement::Polygon ||
			point >= Elements[index]->NumPoints)
		return;
	Elements[index]->Points[point].X = x;
	Elements[index]->Points[point].Y = y;
	Modified = true;
}
void
G42ImageMap::DeleteSelectedElement(void)
{
	if (!SelectedElementValid || SelectedElement >= NumElements)
		return;
	DeleteElement(SelectedElement);
	if (SelectedElement >= NumElements)
		SelectedElement = (int16) (NumElements - 1);
}
void
G42ImageMap::DeleteElement(int16 index)
{
	if (index >= NumElements)
		return;
	if (Elements[index]->Type == G42MapElement::Default)
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
G42ImageMap::AddElement(G42MapElement::G42ElementType T, 
	const G42Vector<G42Point> & PtIn, 
	int16 nPts, const char * URL, const char * Comment, const char * alt_text, 
	int16 N)
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
      if (T == G42MapElement::Rectangle)
      {
      	int x_min = PtIn[0].X;
         int y_min = PtIn[0].Y;
         int x_max = PtIn[1].X;
         int y_max = PtIn[1].Y;
         if (x_max < x_min)
         {
         	x_min = x_max;
            x_max = PtIn[0].X;
         }
         if (y_max < y_min)
         {
         	y_min = y_max;
            y_max = PtIn[0].Y;
         }
         Elements[NumElements]->Points[0].X = x_min;
         Elements[NumElements]->Points[0].Y = y_min;
         Elements[NumElements]->Points[1].X = x_max;
         Elements[NumElements]->Points[1].Y = y_max;
      }
      else
		{
			for (int i=0; i<nPts; i++)
			{
				Elements[NumElements]->Points[i].X = PtIn[i].X;
				Elements[NumElements]->Points[i].Y = PtIn[i].Y;
			}
      }
		Elements[NumElements]->NumPoints = nPts;
		if (T == G42MapElement::Default)
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
			Elements[N]->Points[i].X = PtIn[i].X;
			Elements[N]->Points[i].Y = PtIn[i].Y;
		}
		Elements[N]->NumPoints = nPts;
		if (HasDefault() && DefaultIndex >= N)
			DefaultIndex++;
		if (T == G42MapElement::Default)
		{
			DefaultValid = true;
			DefaultIndex = N;
		}
		NumElements++;
	}
	Modified = true;
}
void
G42ImageMap::AddElement(G42MapElement::G42ElementType T, int x, int y, int x2, int y2,
	char* URL, char* Comment, char * alt_text, int16 N)
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
		Elements[NumElements]->Points[0].X = x;
		Elements[NumElements]->Points[0].Y = y;
		Elements[NumElements]->Points[1].X = x2;
		Elements[NumElements]->Points[1].Y = y2;
		Elements[NumElements]->NumPoints = 2;
		if (T == G42MapElement::Default)
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
		Elements[N]->Points[0].X = x;
		Elements[N]->Points[0].Y = y;
		Elements[N]->Points[1].X = x2;
		Elements[N]->Points[1].Y = y2;
		Elements[N]->NumPoints = 2;
		if (HasDefault() && DefaultIndex >= N)
			DefaultIndex++;
		if (T == G42MapElement::Default)
		{
			DefaultValid = true;
			DefaultIndex = N;
		}
		NumElements++;
	}
	Modified = true;
}
void
G42ImageMap::MoveElementPos(int16 new_pos, int16 old_pos)
{
	if (new_pos >= NumElements || old_pos >= NumElements ||
		old_pos == new_pos)
			return;
	G42MapElement * element = Elements[old_pos];
	if (old_pos > new_pos)
	{
		for (uintf i = old_pos; i > new_pos; i--)
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
		for (uintf i = old_pos; i < new_pos; i++)
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
G42ImageMap::SelectElement(int16 index)
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
		  for (i = 0; i<Elements[Index]->NumPoints; i++)
		  {
				pgon[i][0] = (double)Elements[Index]->Points[i].X;
				pgon[i][1] = (double)Elements[Index]->Points[i].Y;
		  }
					 ;
		  numverts = Elements[Index]->NumPoints;
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
/*******************************/
// Image map parser
bool
G42ImageMap::ParseMapString(char * buffer)
{
	bool ValidLoad = false;
	char* ptr = buffer, *ptr2 = 0;
	if (!strncmp(buffer, "g42imap", 7))
	{
		IsInternal = true;
		MapType = Internal;
	}
	if (!IsClientSide && !IsInternal)
	{
		URL[0] = 0;
		x1 = 0;
		y1 = 0;
		x2 = 0;
		y2 = 0;
	}
	while (isspace(*ptr)) ptr++;
	if (*ptr == '<')
	{
		IsClientSide = true;
		MapType = ClientSide;
	}
	if (IsInternal)
	{
   	if (buffer[strlen(buffer) - 1] == '\n')
      	buffer[strlen(buffer) - 1] = '\0';
         
		switch (buffer[0])
		{
			case 'g': // header
			{
				break; // header has already been checked
			}
			case 'n': // client side name
			{
				if (ClientSideName)
					delete [] ClientSideName;
				ClientSideName = new char [strlen(buffer)];
				if (ClientSideName)
					strcpy(ClientSideName, buffer + 1);
				break;
			}
			case 'c': // comment
			{
				strcpy(Comment, buffer + 1);
				break;
			}
			case 'u': // URL
			{
				strcpy(URL, buffer + 1);
				break;
			}
			case 'a': // alt text
			{
				strcpy(AltText, buffer + 1);
				break;
			}
			case 'w': // image width
			{
				if (!ImageWidth)
				{
					ptr = buffer + 1;
					ImageWidth = atoi(ptr);
				}
				break;
			}
			case 'h': // image height
			{
				if (!ImageHeight)
				{
					ptr = buffer + 1;
					ImageHeight = atoi(ptr);
				}
				break;
			}
			case 't': // point
			{
				ptr = buffer + 1;
				int x1 = atoi(ptr);
				while (isdigit(*ptr) || *ptr == '-')
					ptr++;
				ptr++; // skip comma
				int y1 = atoi(ptr);
				AddElement(G42MapElement::Point, x1, y1, 0, 0, URL, Comment, AltText);
				Comment[0] = 0;
				URL[0] = 0;
				AltText[0] = 0;
				ValidLoad = true;
				break;
			}
			case 'r': // rectangle
			{
				ptr = buffer + 1;
				int x1 = atoi(ptr);
				while (isdigit(*ptr) || *ptr == '-')
					ptr++;
				ptr++; // skip comma
				int y1 = atoi(ptr);
				while (isdigit(*ptr) || *ptr == '-')
					ptr++;
				ptr++; // skip comma
				int x2 = atoi(ptr);
				while (isdigit(*ptr) || *ptr == '-')
					ptr++;
				ptr++; // skip comma
				int y2 = atoi(ptr);
				AddElement(G42MapElement::Rectangle, x1, y1, x2, y2, URL, Comment, AltText);
				Comment[0] = 0;
				URL[0] = 0;
				AltText[0] = 0;
				ValidLoad = true;
				break;
			}
			case 'o': // circle (c is comment, and an 'o' is round, so...)
			{
				ptr = buffer + 1;
				int x1 = atoi(ptr);
				while (isdigit(*ptr) || *ptr == '-')
					ptr++;
				ptr++; // skip comma
				int y1 = atoi(ptr);
				while (isdigit(*ptr) || *ptr == '-')
					ptr++;
				ptr++; // skip comma
				int x2 = atoi(ptr);
				while (isdigit(*ptr) || *ptr == '-')
					ptr++;
				ptr++; // skip comma
				int y2 = atoi(ptr);
				AddElement(G42MapElement::Circle, x1, y1, x2, y2, URL, Comment, AltText);
				Comment[0] = 0;
				URL[0] = 0;
				AltText[0] = 0;
				ValidLoad = true;
				break;
			}
			case 'p': // polygon
			{
				G42Vector<G42Point> points;
				int16 num_points = 0;
				ptr = buffer + 1;
				while (1)
				{
					points[num_points].X = atoi(ptr);
					while (isdigit(*ptr) || *ptr == '-')
						ptr++;
					if (*ptr != ',')
						break;
					ptr++; // skip comma
					points[num_points].Y = atoi(ptr);
					while (isdigit(*ptr) || *ptr == '-')
						ptr++;
					num_points++;
					if (*ptr != ',')
						break;
					ptr++; // skip comma
				}
				AddElement(G42MapElement::Polygon, points, num_points, URL, Comment, AltText);
				Comment[0] = 0;
				URL[0] = 0;
				AltText[0] = 0;
				ValidLoad = true;
				break;
			}
			case 'd': // default
			{
				AddElement(G42MapElement::Default, 0, 0, 0, 0, URL, Comment, AltText);
				Comment[0] = 0;
				URL[0] = 0;
				AltText[0] = 0;
				ValidLoad = true;
				break;
			}
		}
	}
	else if (IsClientSide)
	{
		while (*ptr && !ClientSideDone) // repeat until line is used up
		{
			switch (ClientSideState)
			{
				case 0: // outside of tag
				{
					// find tag start
					while (*ptr && *ptr != '<')
						ptr++;
						if (*ptr == '<') // start of tag
					{
						ClientSideState = 1; // unknown tag
						ptr++;
						if (!ClientSideHaveMap &&
							tolower(*ptr) == 'm') // possible map tag
						{
							ptr++;
							if (tolower(*ptr) == 'a')
							{
								ptr++;
								if (tolower(*ptr) == 'p')
								{
									ptr++;
									ClientSideToken = 1; // map tag
									ClientSideState = 11; // get token
									ClientSideHaveMap = true;
								}
							}
						}
						else if (tolower(*ptr) == 'a') // possible area tag
						{
							ptr++;
							if (tolower(*ptr) == 'r')
							{
								ptr++;
								if (tolower(*ptr) == 'e')
								{
									ptr++;
									if (tolower(*ptr) == 'a')
									{
										ptr++;
										ClientSideState = 10; // reset area
										ClientSideToken = 2; // area tag
									}
								}
							}
						}
						else if (*ptr == '!') // possible area tag
						{
							ptr++;
							if (*ptr == '-')
							{
								ptr++;
								if (*ptr == '-')
								{
									ptr++;
									ClientSideState = 2; // reset area
								}
							}
						}
						else if (ClientSideHaveMap &&
							tolower(*ptr) == '/') // possible end map tag
						{
							ptr++;
							if (tolower(*ptr) == 'm')
							{
								ptr++;
								if (tolower(*ptr) == 'a')
								{
									ptr++;
									if (tolower(*ptr) == 'p')
									{
										ptr++;
										ClientSideHaveMap = false;
										ClientSideDone = true;
										ClientSideState = 1; // skip tag
									}
								}
							}
						}
					}
					break;
				}
				case 1: // skip unknown tag
				{
					while (*ptr && *ptr != '>')
						ptr++;
					if (*ptr == '>')
					{
						ptr++;
						ClientSideState = 0; // outside of tag
					}
					break;
				}
				case 2: // comment
				{
					char *cp = Comment;
					while (*ptr && isspace(*ptr))
						ptr++;
					while (*ptr && cp - Comment < 256 &&
						!(*ptr == '-' &&
						*(ptr + 1) == '-' && *(ptr + 2) == '>'))
						*cp++ = *ptr++;
					while (cp > Comment && isspace(*(cp - 1)))
							cp--;
					*cp = '\0';
					if (*ptr == '-' && *(ptr + 1) == '-' &&
						*(ptr + 2) == '>')
					{
						ptr += 3;
						ClientSideState = 0; // outside of tag
					}
					else
					{
						ClientSideState = 3; // skip rest of tag
					}
					break;
				}
				case 3: // comment
				{
					while (*ptr &&
						!(*ptr == '-' &&
						*(ptr + 1) == '-' && *(ptr + 2) == '>'))
						ptr++;
					if (*ptr == '-' && *(ptr + 1) == '-' &&
						*(ptr + 2) == '>')
					{
						ptr += 3;
						ClientSideState = 0; // outside of tag
					}
				}
				case 10: // start of area tag
				{
					ClientSideShape = G42MapElement::Rectangle;
					URL[0] = '\0';
               AltText[0] = '\0';
					ClientSideState = 11;
					ClientSideHaveShape = true;
					ClientSideHaveCoords = false;
					// fall through
				}
				case 11: // need new token
				{
					while (*ptr && isspace(*ptr))
						ptr++;
					if (!*ptr)
						break;
						if (*ptr == '>')
					{
						if (ClientSideToken == 1) // if map tag
						{
							ClientSideState = 0; // outside tag
							break;
						}
						if (ClientSideHaveShape && ClientSideHaveCoords)
						{
							switch (ClientSideShape)
							{
								case G42MapElement::Polygon:
									AddElement(G42MapElement::Polygon, ClientSidePointArray,
										ClientSideNumPointArray, URL, Comment, AltText);
									//delete[] ClientSidePointArray;
									//ClientSidePointArray = 0;
									ClientSideNumPointArray = 0;
									ValidLoad = true;
									//DrawShape(nShapes-1, false);
									break;
								case G42MapElement::Circle:
								case G42MapElement::Rectangle:
									AddElement(ClientSideShape, x1, y1, x2, y2,
										URL, Comment, AltText);
									ValidLoad = true;
									//DrawShape(nShapes-1, false);
									break;
								case G42MapElement::Default:
									AddElement(G42MapElement::Default, 0, 0, 0, 0, URL, Comment, AltText, 0);
									ValidLoad = true;
									break;
							}
						}
						ClientSideState = 0; // outside tag
						Comment[0] = '\0';
						AltText[0] = '\0';
						break;
					}
					if (*ptr == '=')
					{
						ptr++;
						break;
					}
					if (*ptr == '"')
					{
						ptr++;
						ClientSideState = 12; // eat string
						break;
					}
					char token[10];
					int token_length = 0;
					while (token_length < 10 && *ptr && isalpha(*ptr))
					{
						token[token_length++] = (char)tolower(*ptr++);
					}
					if (token_length == 10)
					{
						while (*ptr && isalpha(*ptr))
							ptr++;
						break;
					}
					if (!token_length)
					{
						ptr++;
						break;
					}
					token[token_length] = '\0';
					if (ClientSideToken == 1 && !strcmp(token, "name"))
					{
						ClientSideToken = 22;
						ClientSideState = 13;
					}
					else if (!ClientSideHaveCoords && !strcmp(token, "shape"))
					{
						ClientSideToken = 23;
						ClientSideState = 13;
					}
					else if (ClientSideHaveShape && !strcmp(token, "coords"))
					{
						ClientSideToken = 24;
						ClientSideState = 13;
					}
					else if (ClientSideHaveShape && !strcmp(token, "href"))
					{
						ClientSideToken = 25;
						ClientSideState = 13;
					}
					else if (ClientSideHaveShape && !strcmp(token, "nohref"))
					{
						ClientSideState = 26;
					}
					else if (ClientSideHaveShape && !strcmp(token, "alt"))
					{
						ClientSideToken = 27;
						ClientSideState = 13;
					}
					break;
				}
				case 12: // eat string
				{
					while (*ptr && *ptr != '"')
						ptr++;
					if (!*ptr)
						break;
					ClientSideState = 11; // new token
					break;
				}
				case 13: // = before string
				{
					while (*ptr && isspace(*ptr))
						ptr++;
					if (!*ptr)
						break;
					if (*ptr == '=')
					{
						ClientSideState = 14;
						ptr++;
					}
					else
					{
						ClientSideState = 11;
						if (ClientSideToken == 22)
							ClientSideToken = 1;
					}
					break;
				}
				case 14: // get string
				{
					bool need_close = false;
					while (*ptr && isspace(*ptr))
					ptr++;
					if (!*ptr)
						break;
					if (*ptr == '"')
					{
						need_close = true;
						ptr++;
					}
					char * name = ptr;
					if (need_close)
					{
						while (*ptr && *ptr != '"')
							ptr++;
					}
					else
					{
						while (*ptr && !isspace(*ptr) && *ptr != '>')
							ptr++;
					}
					ClientSideString = new char [(int)(ptr - name) + 1];
					strncpy(ClientSideString, name, (int)(ptr - name));
					ClientSideString[(int)(ptr - name)] = '\0';
						if (*ptr && need_close)
						ptr++;
					ClientSideState = ClientSideToken;
					break;
				}
				case 22: // name
				{
					ClientSideName = ClientSideString;
					ClientSideString = 0;
					ClientSideState = 11;
					ClientSideToken = 1;
					break;
				}
				case 23: // shape
				{
					if (!ClientSideHaveCoords)
					{
						char * p = ClientSideString;
						while (*p)
						{
							*p = (char)toupper(*p);
								 p++;
						}
						if (!strcmp(ClientSideString, "RECT"))
						{
							ClientSideShape = G42MapElement::Rectangle;
							ClientSideHaveShape = true;
						}
						else if (!strcmp(ClientSideString, "CIRCLE"))
						{
							ClientSideShape = G42MapElement::Circle;
							ClientSideHaveShape = true;
						}
						else if (!strcmp(ClientSideString, "POLYGON"))
						{
							ClientSideShape = G42MapElement::Polygon;
							ClientSideHaveShape = true;
						}
						else if (!strcmp(ClientSideString, "POLY"))
						{
							ClientSideShape = G42MapElement::Polygon;
							ClientSideHaveShape = true;
						}
						else if (!strcmp(ClientSideString, "DEFAULT"))
						{
							ClientSideShape = G42MapElement::Default;
							ClientSideHaveShape = true;
							ClientSideHaveCoords = true;
						}
						else
						{
							ClientSideHaveShape = false;
						}
					}
					delete[] ClientSideString;
					ClientSideString = 0;
					ClientSideState = 11;
					break;
				}
				case 24: // coords
				{
					switch (ClientSideShape)
					{
						case G42MapElement::Rectangle:
						{
							char * ptr = ClientSideString;
							while (*ptr && isspace(*ptr))
								ptr++;
							if (!*ptr)
								break;
							x1 = atoi(ptr);
							while (*ptr && (isdigit(*ptr) || *ptr == '-'))
								ptr++;
							while (*ptr && isspace(*ptr))
								ptr++;
							if (*ptr == ',')
									ptr++;
							while (*ptr && isspace(*ptr))
								ptr++;
							if (!*ptr)
									break;
							y1 = atoi(ptr);
							while (*ptr && (isdigit(*ptr) || *ptr == '-'))
								ptr++;
							while (*ptr && isspace(*ptr))
								ptr++;
							if (*ptr == ',')
								ptr++;
							while (*ptr && isspace(*ptr))
								ptr++;
							if (!*ptr)
								break;
							x2 = atoi(ptr);
							while (*ptr && (isdigit(*ptr) || *ptr == '-'))
								ptr++;
							while (*ptr && isspace(*ptr))
								ptr++;
							if (*ptr == ',')
								ptr++;
							while (*ptr && isspace(*ptr))
								ptr++;
							if (!*ptr)
								break;
							y2 = atoi(ptr);
							ClientSideHaveCoords = true;
							break;
						}
						case G42MapElement::Circle:
						{
							char * ptr = ClientSideString;
							while (*ptr && isspace(*ptr))
								ptr++;
							if (!*ptr)
								break;
							x1 = atoi(ptr);
							while (*ptr && (isdigit(*ptr) || *ptr == '-'))
								ptr++;
							while (*ptr && isspace(*ptr))
								ptr++;
							if (*ptr == ',')
								ptr++;
							while (*ptr && isspace(*ptr))
								ptr++;
							if (!*ptr)
								break;
							y1 = atoi(ptr);
							while (*ptr && (isdigit(*ptr) || *ptr == '-'))
								ptr++;
							while (*ptr && isspace(*ptr))
								ptr++;
							if (*ptr == ',')
								ptr++;
							while (*ptr && isspace(*ptr))
								ptr++;
							if (!*ptr)
								break;
							int radius = atoi(ptr);
							ClientSideHaveCoords = true;
							y2 = y1 + radius;
							x2 = x1 + radius;
							x1 = x1 - radius;
							y1 = y1 - radius;
							break;
						}
						case G42MapElement::Polygon:
						{
							//ClientSidePointArray = new TPoint [1000];
							ClientSideNumPointArray = 0;
							char * ptr = ClientSideString;
							while (*ptr && ClientSideNumPointArray < 1000)
							{
								while (*ptr && isspace(*ptr))
									ptr++;
								if (!*ptr)
									break;
								ClientSidePointArray[
									ClientSideNumPointArray].X = atoi(ptr);
								while (*ptr && (isdigit(*ptr) || *ptr == '-'))
									ptr++;
								while (*ptr && isspace(*ptr))
									ptr++;
								if (*ptr == ',')
									ptr++;
								while (*ptr && isspace(*ptr))
									ptr++;
								if (!*ptr)
									break;
								ClientSidePointArray[
									ClientSideNumPointArray].Y = atoi(ptr);
								while (*ptr && (isdigit(*ptr) || *ptr == '-'))
									ptr++;
								while (*ptr && isspace(*ptr))
									ptr++;
								if (*ptr == ',')
									ptr++;
								ClientSideNumPointArray++;
							}
							ClientSideHaveCoords = true;
							break;
						}
					}
					delete[] ClientSideString;
					ClientSideString = 0;
					ClientSideState = 11;
					break;
				}
				case 25: // href
				{
					strncpy(URL, ClientSideString, 256);
					delete[] ClientSideString;
					ClientSideString = 0;
					ClientSideState = 11;
					break;
				}
				case 26: // nohref
				{
					URL[0] = '\0';
					ClientSideState = 11;
					break;
				}
				case 27: // alt
				{
					strncpy(AltText, ClientSideString, 256);
					delete[] ClientSideString;
					ClientSideString = 0;
					ClientSideState = 11;
					break;
				}
			}
		}
		if (ClientSideDone)
		{
			return true;
		}
	}
	else
	{
		char* ptr = buffer, *ptr2 = 0;
		int x1=0, y1=0, x2=0, y2=0;
		while (isspace(*ptr))
			ptr++;
		switch(*ptr)
		{
		case 'r':
			if (!strncmp(ptr, "rectangle", 9))
			{
				ptr += 9;
				while (isspace(*ptr))
					ptr++;
				if (*ptr != '(')
					break;
				ptr++;
				if (!GetTwoDigits(&ptr, &x1, &y1))
					break;
				while (isspace(*ptr))
					ptr++;
				if (*ptr != ')')
					break;
				ptr++;
				while (isspace(*ptr))
					ptr++;
				if (*ptr != '(')
					break;
				ptr++;
				if (!GetTwoDigits(&ptr, &x2, &y2))
					break;
				while (isspace(*ptr))
					ptr++;
				if (*ptr != ')')
					break;
				ptr++;
				while (isspace(*ptr))
					ptr++;
				strcpy(URL, ptr);
				URL[strlen(URL)-1] = 0;
				MapType = CERN;
			}
			else if (!strncmp(ptr, "rect ", 5))
			{
				ptr += 5;
				while (isspace(*ptr))
					ptr++;
				ptr2 = ptr;
				while (!isspace(*ptr2))
					ptr2++;
				strncpy(URL, ptr, ptr2 - ptr);
				URL[ptr2-ptr] = 0;
				ptr = ptr2;
				if (!GetTwoDigits(&ptr, &x1, &y1))
					break;
				if (!GetTwoDigits(&ptr, &x2, &y2))
					break;
				MapType = NCSA;
			}
			else
				break;
			AddElement(G42MapElement::Rectangle, x1, y1, x2, y2, URL, Comment, 0);
			//DrawShape(nShapes-1, false);
			Comment[0] = 0;
			ValidLoad = true;
			break;
		case 'c':
			if (strncmp(ptr, "circle", 6))
				break;
			ptr += 6;
			while (isspace(*ptr))
				ptr++;
			if (*ptr == '(')
			{
				ptr++;
				if (!GetTwoDigits(&ptr, &x1, &y1))
					break;
				while (isspace(*ptr))
					ptr++;
				if (*ptr != ')')
					break;
				ptr++;
				while (isspace(*ptr))
					ptr++;
				x2 = atoi(ptr);
				while (!isspace(*ptr))
					ptr++;
				while (isspace(*ptr))
					ptr++;
				strcpy(URL, ptr);
				URL[strlen(URL)-1] = 0;
				MapType = CERN;
				AddElement(G42MapElement::Circle, (x1-x2), (y1-x2), (x1+x2), (y1+x2), URL, Comment, 0);
			}
			else
			{
				ptr2 = ptr;
				while (!isspace(*ptr2))
					ptr2++;
				strncpy(URL, ptr, ptr2 - ptr);
				URL[ptr2-ptr] = 0;
				ptr = ptr2;
				if (!GetTwoDigits(&ptr, &x1, &y1))
					break;
				if (!GetTwoDigits(&ptr, &x2, &y2))
					break;
				int Rad = (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1);
				Rad = sqrt(Rad);
				x2 = x1 + Rad;
				y2 = y1 + Rad;
				x1 = x1 - Rad;
				y1 = y1 - Rad;
				MapType = NCSA;
				AddElement(G42MapElement::Circle, x1, y1, x2, y2, URL, Comment, 0);
			}
			//DrawShape(nShapes-1, false);
			Comment[0] = 0;
			ValidLoad = true;
			break;
		case 'd':
			if (strncmp(ptr, "default", 7))
				break;
			ptr += 7;
			while (isspace(*ptr))
				ptr++;
			strcpy(URL, ptr);
			URL[strlen(URL)-1] = 0;
			AddElement(G42MapElement::Default, 0, 0, 0, 0, URL, Comment, 0, 0);
			Comment[0] = 0;
			ValidLoad = true;
			break;
		case 'p':
			if (!strncmp(ptr, "polygon", 7))
			{
				ptr += 7;
				G42Vector<G42Point> tmpCoord;
				//TPoint* TmpPtArray = new TPoint[100];
				int16 TmpPt = 0;
				while (TmpPt <= 100)
				{
					while (isspace(*ptr))
						ptr++;
					if (*ptr != '(')
						break;
					ptr++;
					if (!GetTwoDigits(&ptr, (int *)&(tmpCoord[TmpPt].X),
						(int *)&(tmpCoord[TmpPt].Y)))
						break;
					while (isspace(*ptr))
						ptr++;
					if (*ptr != ')')
						break;
					ptr++;
					TmpPt++;
				}
				strcpy(URL, ptr);
				URL[strlen(URL)-1] = 0;
				AddElement(G42MapElement::Polygon, tmpCoord, TmpPt, URL, Comment, 0);
				//DrawShape(nShapes-1, false);
				//delete[] TmpPtArray;
				//TmpPtArray=0;
				ValidLoad = true;
				MapType = CERN;
			}
			else if (!strncmp(ptr, "poly", 4))
			{
				ptr += 4;
				while (isspace(*ptr))
					ptr++;
				ptr2 = ptr;
				while (!isspace(*ptr2))
					ptr2++;
				strncpy(URL, ptr, ptr2 - ptr);
				URL[ptr2-ptr] = 0;
				ptr = ptr2;
				G42Vector<G42Point> tmpCoord;
				//TPoint* TmpPtArray = new TPoint[100];
				int16 TmpPt = 0;
				while (TmpPt <= 100)
				{
					if (!GetTwoDigits(&ptr, (int *)&(tmpCoord[TmpPt].X),
						(int *)&(tmpCoord[TmpPt].Y)))
						break;
					TmpPt++;
				}
				AddElement(G42MapElement::Polygon, tmpCoord, TmpPt, URL, Comment, (int16) 0);
				//DrawShape(nShapes-1, false);
				//delete[] TmpPtArray;
				//TmpPtArray=0;
				MapType = NCSA;
				ValidLoad = true;
			}
			else if (!strncmp(ptr, "point", 5))
			{
				if (MapType != NCSA)
					break;
				ptr += 5;
				while (isspace(*ptr))
					ptr++;
				ptr2 = ptr;
				while (!isspace(*ptr2))
					ptr2++;
				strncpy(URL, ptr, ptr2 - ptr);
				URL[ptr2-ptr] = 0;
				ptr = ptr2;
				if (!GetTwoDigits(&ptr, &x1, &y1))
					break;
				AddElement(G42MapElement::Point, x1, y1, 0, 0, URL, Comment, 0);
				//DrawShape(nShapes-1, false);
				ValidLoad = true;
			  }
			  Comment[0] = 0;
			break;
		case '#':
			Comment[0] = 0;
			strncpy(Comment, buffer+1, 255);
			Comment[255] = 0;
			ptr = strchr(Comment, '\n');
			if (ptr != (char*)NULL)
				*ptr = 0;
			break;
		default:
			Comment[0] = 0;
			break;
		}
	}
	return ValidLoad;
}

// g42impar.cpp - image map parser
#include <g42imap.h>
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
				AddElement(Point, x1, y1, 0, 0, URL, Comment, AltText);
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
				AddElement(Rectangle, x1, y1, x2, y2, URL, Comment, AltText);
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
				AddElement(Circle, x1, y1, x2, y2, URL, Comment, AltText);
				Comment[0] = 0;
				URL[0] = 0;
				AltText[0] = 0;
				ValidLoad = true;
				break;
			}
			case 'p': // polygon
			{
				G42Array<G42Coord> points;
				int num_points = 0;
				ptr = buffer + 1;
				while (1)
				{
					points[num_points].x = atoi(ptr);
					while (isdigit(*ptr) || *ptr == '-')
						ptr++;
					if (*ptr != ',')
						break;
					ptr++; // skip comma
					points[num_points].y = atoi(ptr);
					while (isdigit(*ptr) || *ptr == '-')
						ptr++;
					num_points++;
					if (*ptr != ',')
						break;
					ptr++; // skip comma
				}
				AddElement(Polygon, points, num_points, URL, Comment, AltText);
				Comment[0] = 0;
				URL[0] = 0;
				AltText[0] = 0;
				ValidLoad = true;
				break;
			}
			case 'd': // default
			{
				AddElement(Default, 0, 0, 0, 0, URL, Comment, AltText);
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
					ClientSideShape = Rectangle;
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
								case Polygon:
									AddElement(Polygon, ClientSidePointArray,
										ClientSideNumPointArray, URL, Comment, AltText);
									//delete[] ClientSidePointArray;
									//ClientSidePointArray = 0;
									ClientSideNumPointArray = 0;
									ValidLoad = true;
									//DrawShape(nShapes-1, false);
									break;
								case Circle:
								case Rectangle:
									AddElement(ClientSideShape, x1, y1, x2, y2,
										URL, Comment, AltText);
									ValidLoad = true;
									//DrawShape(nShapes-1, false);
									break;
								case Default:
									AddElement(Default, 0, 0, 0, 0, URL, Comment, AltText, 0);
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
							ClientSideShape = Rectangle;
							ClientSideHaveShape = true;
						}
						else if (!strcmp(ClientSideString, "CIRCLE"))
						{
							ClientSideShape = Circle;
							ClientSideHaveShape = true;
						}
						else if (!strcmp(ClientSideString, "POLYGON"))
						{
							ClientSideShape = Polygon;
							ClientSideHaveShape = true;
						}
						else if (!strcmp(ClientSideString, "POLY"))
						{
							ClientSideShape = Polygon;
							ClientSideHaveShape = true;
						}
						else if (!strcmp(ClientSideString, "DEFAULT"))
						{
							ClientSideShape = Default;
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
						case Rectangle:
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
						case Circle:
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
						case Polygon:
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
									ClientSideNumPointArray].x = atoi(ptr);
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
									ClientSideNumPointArray].y = atoi(ptr);
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
			AddElement(Rectangle, x1, y1, x2, y2, URL, Comment, 0);
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
				AddElement(Circle, (x1-x2), (y1-x2), (x1+x2), (y1+x2), URL, Comment, 0);
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
				AddElement(Circle, x1, y1, x2, y2, URL, Comment, 0);
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
			AddElement(Default, 0, 0, 0, 0, URL, Comment, 0, 0);
			Comment[0] = 0;
			ValidLoad = true;
			break;
		case 'p':
			if (!strncmp(ptr, "polygon", 7))
			{
				ptr += 7;
				G42Array<G42Coord> tmpCoord;
				//TPoint* TmpPtArray = new TPoint[100];
				int TmpPt = 0;
				while (TmpPt <= 100)
				{
					while (isspace(*ptr))
						ptr++;
					if (*ptr != '(')
						break;
					ptr++;
					if (!GetTwoDigits(&ptr, (int *)&(tmpCoord[TmpPt].x),
						(int *)&(tmpCoord[TmpPt].y)))
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
				AddElement(Polygon, tmpCoord, TmpPt, URL, Comment, 0);
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
				G42Array<G42Coord> tmpCoord;
				//TPoint* TmpPtArray = new TPoint[100];
				int TmpPt = 0;
				while (TmpPt <= 100)
				{
					if (!GetTwoDigits(&ptr, (int *)&(tmpCoord[TmpPt].x),
						(int *)&(tmpCoord[TmpPt].y)))
						break;
					TmpPt++;
				}
				AddElement(Polygon, tmpCoord, TmpPt, URL, Comment, 0);
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
				AddElement(Point, x1, y1, 0, 0, URL, Comment, 0);
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

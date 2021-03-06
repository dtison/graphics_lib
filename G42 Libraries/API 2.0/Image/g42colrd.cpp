#include "g42idata.h"
#include "g42iter.h"
#include "g42iterf.h"
#include "g42opal.h"
#include "g42event.h"
#include "g42image.h"
#include "g42iview.h"
#define CLIP_ERROR(ERROR)\
	if (ERROR > 48)\
		ERROR = 32;\
	else if (ERROR > 16)\
		ERROR = ((ERROR - 16) >> 1) + 16;\
#define CLIP_ERROR2(ERROR)\
	if (ERROR > 64)\
		ERROR = 48;\
	else if (ERROR > 32)\
		ERROR = ((ERROR - 32) >> 1) + 32;\
#define DIFFUSE_ERROR(ERROR)\
	if (ERROR < 0)\
	{\
		ERROR = -ERROR;\
		int error1 = (ERROR >> 4);\
		int error2 = (ERROR >> 3) + error1;\
		int error3 = (ERROR >> 2) + error1;\
		*erow2_ptr += -error2;\
		erow2_ptr += 3;\
		*erow2_ptr += -error3;\
		erow2_ptr += 3;\
		*erow2_ptr += -error1;\
		ERROR -= (error1 + error2 + error3);\
		ERROR = -ERROR;\
	}\
	else\
	{\
		int error1 = (ERROR >> 4);\
		int error2 = (ERROR >> 3) + error1;\
		int error3 = (ERROR >> 2) + error1;\
		*erow2_ptr += error2;\
		erow2_ptr += 3;\
		*erow2_ptr += error3;\
		erow2_ptr += 3;\
		*erow2_ptr += error1;\
		ERROR -= (error1 + error2 + error3);\
	}
#define DIFFUSE_ERROR2(ERROR)\
	if (ERROR < 0)\
	{\
		ERROR = -ERROR;\
		int error1 = (ERROR >> 4);\
		int error2 = (ERROR >> 3) + error1;\
		int error3 = (ERROR >> 2) + error1;\
		*erow2_ptr += -error2;\
		erow2_ptr++;\
		*erow2_ptr += -error3;\
		erow2_ptr++;\
		*erow2_ptr += -error1;\
		ERROR -= (error1 + error2 + error3);\
		ERROR = -ERROR;\
	}\
	else\
	{\
		int error1 = (ERROR >> 4);\
		int error2 = (ERROR >> 3) + error1;\
		int error3 = (ERROR >> 2) + error1;\
		*erow2_ptr += error2;\
		erow2_ptr++;\
		*erow2_ptr += error3;\
		erow2_ptr++;\
		*erow2_ptr += error1;\
		ERROR -= (error1 + error2 + error3);\
	}
#define LIMIT(VAL)\
	if (VAL > 255)\
   	VAL = 255;\
   if (VAL < 0)\
   	VAL = 0;
G42ImageData * ColorReduce4Bit(G42ImageData *, int, G42PaletteType, int, G42EventUpdateHandler *);
G42ImageData * ColorReduce1Bit(G42ImageData *, int, G42PaletteType, int, G42EventUpdateHandler *);
void Match(G42ImageData *, G42ImageData *, int, int, int, int, G42OptimizedPalette *);
/*  I Added these  D.I. */
void Reduce24ColorTo8Fast (G42ImageData * data, G42ImageData * new_data,
	int left, int top, int right, int bottom, G42OptimizedPalette * opt_pal);
void Reduce8ColorTo8Fast (G42ImageData * data, G42ImageData * new_data,
	int left, int top, int right, int bottom, G42OptimizedPalette * opt_pal);
void Reduce8GrayTo8Fast (G42ImageData * data, G42ImageData * new_data,
	int left, int top, int right, int bottom, G42OptimizedPalette * opt_pal);
G42ImageData *
ColorReduce(G42ImageData * data, int method, G42PaletteType pal_type,
	int num_colors, G42EventUpdateHandler * event_handler)
{
	event_handler->SetDenominator((int32)data->GetHeight());
   int32 row_count = 0L;
	switch (data->GetDepth())
	{
   	case 32:
		case 24:
		{
			if (pal_type == Color)
			{
				if (num_colors > 16)
				{
					G42OptimizedPalette * new_opt_pal = new G42OptimizedPalette(data, num_colors);
					G42ImageData * new_data = new G42ImageData(data->GetData(),
						data->GetWidth(), data->GetHeight(), 8, new_opt_pal->GetNumPalette());
					new_data->ReferenceCount++;
					new_data->SetFinished(true);
					new_data->SetValidStartRow(0);
					new_data->SetValidNumRows(new_data->GetHeight());
					G42Color * new_pal = new_opt_pal->GetPalette();
					new_data->SetPalette(new_pal);
					G42LockedID24BitIterator image_it1(data);
					G42ImageData8BitIterator image_it2(new_data);
					if (method == MethodDither)
					{
						int32 row_width = 0L;
						for (uint x = 0; x < data->GetNumTilesAcross(); x++)
						{
							G42ImageTile * tile = data->GetTile(0, x);
							row_width += tile->GetTileWidth();
						}
						row_width *= image_it1.GetCellSize();
						int * error_row1 = new int [row_width + 6];
						memset(error_row1, 0, (row_width + 6) * sizeof(int));
						int * error_row2 = new int [row_width + 6];
						memset(error_row2, 0, (row_width + 6) * sizeof(int));
						int * erow1_ptr = error_row1 + 3;
						int * erow2_ptr = error_row2;
						int red, green, blue;
                  event_handler->Start();
                  event_handler->Set(0L);
						do
						{
							int e1 = *erow1_ptr;
                     CLIP_ERROR(e1)
                     erow1_ptr++;
							int e2 = *erow1_ptr;
                     CLIP_ERROR(e2)
                     erow1_ptr++;
							int e3 = *erow1_ptr;
                     CLIP_ERROR(e2)
                     erow1_ptr++;
							red = image_it1.GetRed() + e1;
							green = image_it1.GetGreen() + e2;
							blue = image_it1.GetBlue() + e3;
                     LIMIT(red)
                     LIMIT(green)
                     LIMIT(blue)
							int val = new_opt_pal->GetPixel(G42Color(red, green, blue));
							image_it2.Set(val);
							int error = red - new_pal[val].red;
							DIFFUSE_ERROR(error)
							*erow1_ptr += error;
							erow1_ptr++;
							erow2_ptr -= 5;
							error = green - new_pal[val].green;
							DIFFUSE_ERROR(error)
							*erow1_ptr += error;
							erow1_ptr++;
							erow2_ptr -= 5;
							error = blue - new_pal[val].blue;
							DIFFUSE_ERROR(error)
							*erow1_ptr += error;
							erow2_ptr -= 5;
							erow1_ptr -= 2;
							image_it2.Increment();
							if (!image_it1.Increment())
							{
								if (!image_it1.NextRow())
									break;
								image_it2.NextRow();
                        row_count++;
                        event_handler->Set(row_count);
								int * swap = error_row1;
								error_row1 = error_row2;
								error_row2 = swap;
								memset(error_row2, 0, (row_width + 6) * sizeof(int));
								erow1_ptr = error_row1 + 3;
								erow2_ptr = error_row2;
							}
						} while (1);
						delete new_opt_pal;
						delete [] error_row1;
						delete [] error_row2;
                  event_handler->Stop();
						return new_data;
					}
					else
					{
               	event_handler->Start();
                  event_handler->Set(0L);
						do
						{
							image_it2.Set(new_opt_pal->GetPixel(G42Color(image_it1.GetRed(), image_it1.GetGreen(),
								image_it1.GetBlue())));
							image_it2.Increment();
							if (!image_it1.Increment())
							{
								if (!image_it1.NextRow())
									break;
								image_it2.NextRow();
                        row_count++;
                        event_handler->Set(row_count);
							}
						} while (1);
						delete new_opt_pal;
                  event_handler->Stop();
						return new_data;
					}
				}
				if (num_colors > 2)
				{
					G42OptimizedPalette * new_opt_pal = new G42OptimizedPalette(data, num_colors);
					G42ImageData * new_data = new G42ImageData(data->GetData(),
						data->GetWidth(), data->GetHeight(), 4, new_opt_pal->GetNumPalette());
					new_data->ReferenceCount++;
					new_data->SetFinished(true);
					new_data->SetValidStartRow(0);
					new_data->SetValidNumRows(new_data->GetHeight());
					G42Color * new_pal = new_opt_pal->GetPalette();
					new_data->SetPalette(new_pal);
					G42LockedID24BitIterator image_it1(data);
					G42ImageData4BitIterator image_it2(new_data);
					if (method == MethodDither)
					{
						int32 row_width = 0L;
						for (uint x = 0; x < data->GetNumTilesAcross(); x++)
						{
							G42ImageTile * tile = data->GetTile(0, x);
							row_width += tile->GetTileWidth();
						}
						row_width *= image_it1.GetCellSize();
						int * error_row1 = new int [row_width + 6];
						memset(error_row1, 0, (row_width + 6) * sizeof(int));
						int * error_row2 = new int [row_width + 6];
						memset(error_row2, 0, (row_width + 6) * sizeof(int));
						int * erow1_ptr = error_row1 + 3;
						int * erow2_ptr = error_row2;
						int red, green, blue;
                  event_handler->Start();
                  event_handler->Set(0L);
						do
						{
                  	int e1 = *erow1_ptr;
                     CLIP_ERROR2(e1);
                     erow1_ptr++;
                  	int e2 = *erow1_ptr;
                     CLIP_ERROR2(e2);
                     erow1_ptr++;
                  	int e3 = *erow1_ptr;
                     CLIP_ERROR2(e3);
                     erow1_ptr++;
							red = image_it1.GetRed() + e1;
							green = image_it1.GetGreen() + e2;
							blue = image_it1.GetBlue() + e3;
                     LIMIT(red)
                     LIMIT(green)
                     LIMIT(blue)
							int val = new_opt_pal->GetPixel(G42Color(red, green, blue));
							image_it2.Set4(val);
							int error = red - new_pal[val].red;
							DIFFUSE_ERROR(error)
							*erow1_ptr += error;
							erow1_ptr++;
							erow2_ptr -= 5;
							error = green - new_pal[val].green;
							DIFFUSE_ERROR(error)
							*erow1_ptr += error;
							erow1_ptr++;
							erow2_ptr -= 5;
							error = blue - new_pal[val].blue;
							DIFFUSE_ERROR(error)
							*erow1_ptr += error;
							erow2_ptr -= 5;
							erow1_ptr -= 2;
							image_it2.Increment();
							if (!image_it1.Increment())
							{
								if (!image_it1.NextRow())
									break;
								image_it2.NextRow();
                        row_count++;
                        event_handler->Set(row_count);
								int * swap = error_row1;
								error_row1 = error_row2;
								error_row2 = swap;
								memset(error_row2, 0, (row_width + 6) * sizeof(int));
								erow1_ptr = error_row1 + 3;
								erow2_ptr = error_row2;
							}
						} while (1);
						delete new_opt_pal;
						delete [] error_row1;
						delete [] error_row2;
                  event_handler->Stop();
						return new_data;
					}
					else
					{
               	event_handler->Start();
                  event_handler->Set(0L);
						do
						{
							image_it2.Set4(new_opt_pal->GetPixel(G42Color(image_it1.GetRed(), image_it1.GetGreen(),
								image_it1.GetBlue())));
							image_it2.Increment();
							if (!image_it1.Increment())
							{
								if (!image_it1.NextRow())
									break;
								image_it2.NextRow();
								row_count++;
                        event_handler->Set(row_count);
							}
						} while (1);
						delete new_opt_pal;
						event_handler->Stop();
						return new_data;
					}
				}
				G42OptimizedPalette * new_opt_pal = new G42OptimizedPalette(data, 2);
				G42ImageData * new_data = new G42ImageData(data->GetData(),
					data->GetWidth(), data->GetHeight(), 1, new_opt_pal->GetNumPalette());
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
				G42Color * new_pal = new_opt_pal->GetPalette();
				new_data->SetPalette(new_pal);
				G42LockedID24BitIterator image_it1(data);
				G42ImageData1BitIterator image_it2(new_data);
				if (method == MethodDither)
				{
					int32 row_width = 0L;
					for (uint x = 0; x < data->GetNumTilesAcross(); x++)
					{
						G42ImageTile * tile = data->GetTile(0, x);
						row_width += tile->GetTileWidth();
					}
					row_width *= image_it1.GetCellSize();
					int * error_row1 = new int [row_width + 6];
					memset(error_row1, 0, (row_width + 6) * sizeof(int));
					int * error_row2 = new int [row_width + 6];
					memset(error_row2, 0, (row_width + 6) * sizeof(int));
					int * erow1_ptr = error_row1 + 3;
					int * erow2_ptr = error_row2;
					int red, green, blue;
               event_handler->Start();
               event_handler->Set(0L);
					do
					{
						red = image_it1.GetRed() + *erow1_ptr;
						erow1_ptr++;
						green = image_it1.GetGreen() + *erow1_ptr;
						erow1_ptr++;
						blue = image_it1.GetBlue() + *erow1_ptr;
						erow1_ptr++;
                  LIMIT(red)
                  LIMIT(green)
                  LIMIT(blue)
						int val = new_opt_pal->GetPixel(G42Color(red, green, blue));
						image_it2.Set8(val);
						int error = red - new_pal[val].red;
						DIFFUSE_ERROR(error)
						*erow1_ptr += error;
						erow1_ptr++;
						erow2_ptr -= 5;
						error = green - new_pal[val].green;
						DIFFUSE_ERROR(error)
						*erow1_ptr += error;
						erow1_ptr++;
						erow2_ptr -= 5;
						error = blue - new_pal[val].blue;
						DIFFUSE_ERROR(error)
						*erow1_ptr += error;
						erow2_ptr -= 5;
						erow1_ptr -= 2;
						image_it2.Increment();
						if (!image_it1.Increment())
						{
							if (!image_it1.NextRow())
								break;
							image_it2.NextRow();
                     row_count++;
                     event_handler->Set(row_count);
							int * swap = error_row1;
							error_row1 = error_row2;
							error_row2 = swap;
							memset(error_row2, 0, (row_width + 6) * sizeof(int));
							erow1_ptr = error_row1 + 3;
							erow2_ptr = error_row2;
						}
					} while (1);
					delete new_opt_pal;
					delete [] error_row1;
					delete [] error_row2;
               event_handler->Stop();
					return new_data;
				}
				else
				{
      			event_handler->Start();
               event_handler->Set(0L);
					do
					{
						image_it2.Set8(new_opt_pal->GetPixel(G42Color(image_it1.GetRed(), image_it1.GetGreen(),
							image_it1.GetBlue())));
						image_it2.Increment();
						if (!image_it1.Increment())
						{
							if (!image_it1.NextRow())
								break;
							image_it2.NextRow();
                     row_count++;
                     event_handler->Set(row_count);
						}
					} while (1);
					delete new_opt_pal;
					event_handler->Stop();
					return new_data;
				}
			}
			if (pal_type == Gray)
			{
				if (num_colors > 16)
				{
					G42ImageData * new_data = new G42ImageData(data->GetData(),
						data->GetWidth(), data->GetHeight(), 8, 0);
					new_data->ReferenceCount++;
					new_data->SetFinished(true);
					new_data->SetValidStartRow(0);
					new_data->SetValidNumRows(new_data->GetHeight());
					unsigned char * map = new unsigned char [256];
					int index = 0;
					for (int32 n = 0; n < num_colors; n++)
					{
						int val = (n * 255L)/(int32)(num_colors - 1);
						int u_limit = ((n * 255L) + 127L)/(int32)(num_colors - 1);
						if (u_limit > 255)
							u_limit = 255;
						for (; index <= u_limit; index++)
							map[index] = val;
					}
					for (; index < 256; index++)
						map[index] = 255;
					G42LockedID24BitIterator image_it1(data);
					G42ImageData8BitIterator image_it2(new_data);
					if (method == MethodDither)
					{
						int32 row_width = 0L;
						for (uint x = 0; x < data->GetNumTilesAcross(); x++)
						{
							G42ImageTile * tile = data->GetTile(0, x);
							row_width += tile->GetTileWidth();
						}
						int * error_row1 = new int [row_width + 2];
						memset(error_row1, 0, (row_width + 2) * sizeof(int));
						int * error_row2 = new int [row_width + 2];
						memset(error_row2, 0, (row_width + 2) * sizeof(int));
						int * erow1_ptr = error_row1 + 1;
						int * erow2_ptr = error_row2;
						int red, green, blue;
                  event_handler->Start();
                  event_handler->Set(0L);
						do
						{
							red = image_it1.GetRed();
							green = image_it1.GetGreen();
							blue = image_it1.GetBlue();
							int val = (int)((((int32)red * (int32)306) +
								((int32)green * (int32)601) +
								((int32)blue * (int32)117) + (int32)512) >> 10);
                     int e1 = *erow1_ptr;
                     CLIP_ERROR(e1)
							val += e1;
                     LIMIT(val)
							erow1_ptr++;
							image_it2.Set(map[val]);
							int error = val - map[val];
							DIFFUSE_ERROR2(error)
							*erow1_ptr += error;
							erow2_ptr--;
							image_it2.Increment();
							if (!image_it1.Increment())
							{
								if (!image_it1.NextRow())
									break;
								image_it2.NextRow();
                        row_count++;
                        event_handler->Set(row_count);
								int * swap = error_row1;
								error_row1 = error_row2;
								error_row2 = swap;
								memset(error_row2, 0, (row_width + 2) * sizeof(int));
								erow1_ptr = error_row1 + 1;
								erow2_ptr = error_row2;
							}
						} while (1);
						delete [] error_row1;
						delete [] error_row2;
						delete [] map;
                  event_handler->Stop();
						return new_data;
					}
					else
					{
               	event_handler->Start();
                  event_handler->Set(0L);
						do
						{
							int val = (int)(((image_it1.GetRed32() * (int32)306) +
								(image_it1.GetGreen32() * (int32)601) +
								(image_it1.GetBlue32() * (int32)117) + (int32)512) >> 10);
							if (val > 255)
								val = 255;
							image_it2.Set(map[val]);
							image_it2.Increment();
							if (!image_it1.Increment())
							{
								if (!image_it1.NextRow())
									break;
								image_it2.NextRow();
                        row_count++;
                        event_handler->Set(row_count);
							}
						} while (1);
						delete [] map;
                  event_handler->Stop();
						return new_data;
					}
				}
				if (num_colors > 2)
				{
					G42ImageData * new_data = new G42ImageData(data->GetData(),
						data->GetWidth(), data->GetHeight(), 4, 0);
					new_data->ReferenceCount++;
					new_data->SetFinished(true);
					new_data->SetValidStartRow(0);
					new_data->SetValidNumRows(new_data->GetHeight());
					unsigned char * map = new unsigned char [256];
					int index = 0;
					for (int32 n = 0; n < num_colors; n++)
					{
						int val = (n * 15L)/(int32)(num_colors - 1);
						int u_limit = ((n * 255L) + 127L)/(int32)(num_colors - 1);
						if (u_limit > 255)
							u_limit = 255;
						for (; index <= u_limit; index++)
							map[index] = val;
					}
					for (; index < 256; index++)
						map[index] = 15;
					G42LockedID24BitIterator image_it1(data);
					G42ImageData4BitIterator image_it2(new_data);
					if (method == MethodDither)
					{
						int32 row_width = 0L;
						for (uint x = 0; x < data->GetNumTilesAcross(); x++)
						{
							G42ImageTile * tile = data->GetTile(0, x);
							row_width += tile->GetTileWidth();
						}
						int * error_row1 = new int [row_width + 2];
						memset(error_row1, 0, (row_width + 2) * sizeof(int));
						int * error_row2 = new int [row_width + 2];
						memset(error_row2, 0, (row_width + 2) * sizeof(int));
						int * erow1_ptr = error_row1 + 1;
						int * erow2_ptr = error_row2;
						int red, green, blue;
                  event_handler->Start();
                  event_handler->Set(0L);
						do
						{
							red = image_it1.GetRed();
							green = image_it1.GetGreen();
							blue = image_it1.GetBlue();
							int val = (int)((((int32)red * (int32)306) +
								((int32)green * (int32)601) +
								((int32)blue * (int32)117) + (int32)512) >> 10);
                     int e1 = *erow1_ptr;
                     CLIP_ERROR2(e1)
							val += e1;
                  	LIMIT(val)
							erow1_ptr++;
							image_it2.Set4(map[val]);
							int error = val - ((map[val] << 4) + map[val]);
							DIFFUSE_ERROR2(error)
							*erow1_ptr += error;
							erow2_ptr--;
							image_it2.Increment();
							if (!image_it1.Increment())
							{
								if (!image_it1.NextRow())
									break;
								image_it2.NextRow();
                        row_count++;
                        event_handler->Set(row_count);
								int * swap = error_row1;
								error_row1 = error_row2;
								error_row2 = swap;
								memset(error_row2, 0, (row_width + 2) * sizeof(int));
								erow1_ptr = error_row1 + 1;
								erow2_ptr = error_row2;
							}
						} while (1);
						delete [] error_row1;
						delete [] error_row2;
						delete [] map;
                  event_handler->Stop();
						return new_data;
					}
					else
					{
               	event_handler->Start();
                  event_handler->Set(0L);
						do
						{
							int val = (int)(((image_it1.GetRed32() * (int32)306) +
								(image_it1.GetGreen32() * (int32)601) +
								(image_it1.GetBlue32() * (int32)117) + (int32)512) >> 10);
							if (val > 255)
								val = 255;
							if (val < 0)
								val = 0;
							image_it2.Set4(map[val]);
							image_it2.Increment();
							if (!image_it1.Increment())
							{
								if (!image_it1.NextRow())
									break;
								image_it2.NextRow();
                        row_count++;
                        event_handler->Set(row_count);
							}
						} while (1);
						delete [] map;
                  event_handler->Stop();
						return new_data;
					}
				}
				G42ImageData * new_data = new G42ImageData(data->GetData(),
					data->GetWidth(), data->GetHeight(), 1, 0);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
				G42LockedID24BitIterator image_it1(data);
				G42ImageData1BitIterator image_it2(new_data);
				if (method == MethodDither)
				{
					int32 row_width = 0L;
					for (uint x = 0; x < data->GetNumTilesAcross(); x++)
					{
						G42ImageTile * tile = data->GetTile(0, x);
						row_width += tile->GetTileWidth();
					}
					int * error_row1 = new int [row_width + 2];
					memset(error_row1, 0, (row_width + 2) * sizeof(int));
					int * error_row2 = new int [row_width + 2];
					memset(error_row2, 0, (row_width + 2) * sizeof(int));
					int * erow1_ptr = error_row1 + 1;
					int * erow2_ptr = error_row2;
					int red, green, blue;
               event_handler->Start();
               event_handler->Set(0L);
					do
					{
						red = image_it1.GetRed();
						green = image_it1.GetGreen();
						blue = image_it1.GetBlue();
						int val = (int)((((int32)red * (int32)306) +
							((int32)green * (int32)601) +
							((int32)blue * (int32)117) + (int32)512) >> 10);
						val += *erow1_ptr;
						erow1_ptr++;
                  LIMIT(val)
						int cval = 0;
						if (val > 127)
							cval = 255;
						image_it2.Set8(cval);
						int error = val - cval;
						DIFFUSE_ERROR2(error)
						*erow1_ptr += error;
						erow2_ptr--;
						image_it2.Increment();
						if (!image_it1.Increment())
						{
							if (!image_it1.NextRow())
								break;
							image_it2.NextRow();
                     row_count++;
                     event_handler->Set(row_count);
							int * swap = error_row1;
							error_row1 = error_row2;
							error_row2 = swap;
							memset(error_row2, 0, (row_width + 2) * sizeof(int));
							erow1_ptr = error_row1 + 1;
							erow2_ptr = error_row2;
						}
					} while (1);
					delete [] error_row1;
					delete [] error_row2;
               event_handler->Stop();
					return new_data;
				}
				else
				{
            	event_handler->Start();
               event_handler->Set(0L);
					do
					{
						int val = (int)(((image_it1.GetRed32() * (int32)306) +
							(image_it1.GetGreen32() * (int32)601) +
							(image_it1.GetBlue32() * (int32)117) + (int32)512) >> 10);
						if (val > 127)
							image_it2.Set8(true);
						else
							image_it2.Set8(false);
						image_it2.Increment();
						if (!image_it1.Increment())
						{
							if (!image_it1.NextRow())
								break;
							image_it2.NextRow();
                     row_count++;
                     event_handler->Set(row_count);
						}
					} while (1);
					event_handler->Stop();
					return new_data;
				}
			}
			if (pal_type == WinColor)
			{
				if (num_colors > 16)
            	return data;
				G42OptimizedPalette * new_opt_pal = new G42OptimizedPalette(data, num_colors,
					false, true);
				G42ImageData * new_data = new G42ImageData(data->GetData(),
					data->GetWidth(), data->GetHeight(), 4, new_opt_pal->GetNumPalette());
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
				G42Color * new_pal = new_opt_pal->GetPalette();
				new_data->SetPalette(new_pal);
				G42LockedID24BitIterator image_it1(data);
				G42ImageData4BitIterator image_it2(new_data);
				if (method == MethodDither)
				{
					int32 row_width = 0L;
					for (uint x = 0; x < data->GetNumTilesAcross(); x++)
					{
						G42ImageTile * tile = data->GetTile(0, x);
						row_width += tile->GetTileWidth();
					}
					row_width *= 3;
					int * error_row1 = new int [row_width + 6];
					memset(error_row1, 0, (row_width + 6) * sizeof(int));
					int * error_row2 = new int [row_width + 6];
					memset(error_row2, 0, (row_width + 6) * sizeof(int));
					int * erow1_ptr = error_row1 + 3;
					int * erow2_ptr = error_row2;
					int red, green, blue;
               event_handler->Start();
               event_handler->Set(0L);
					do
					{
						int e1 = *erow1_ptr;
                  CLIP_ERROR(e1)
         			erow1_ptr++;
						int e2 = *erow1_ptr;
                  CLIP_ERROR(e2)
         			erow1_ptr++;
						int e3 = *erow1_ptr;
                  CLIP_ERROR(e3)
         			erow1_ptr++;
						red = image_it1.GetRed() + e1;
						green = image_it1.GetGreen() + e2;
						blue = image_it1.GetBlue() + e3;
                  LIMIT(red)
                  LIMIT(green)
                  LIMIT(blue)
						int val = new_opt_pal->GetPixel(G42Color(red, green, blue));
						image_it2.Set4(val);
						int error = red - new_pal[val].red;
						DIFFUSE_ERROR(error)
						*erow1_ptr += error;
						erow1_ptr++;
						erow2_ptr -= 5;
						error = green - new_pal[val].green;
						DIFFUSE_ERROR(error)
						*erow1_ptr += error;
						erow1_ptr++;
						erow2_ptr -= 5;
						error = blue - new_pal[val].blue;
						DIFFUSE_ERROR(error)
						*erow1_ptr += error;
						erow2_ptr -= 5;
						erow1_ptr -= 2;
						image_it2.Increment();
						if (!image_it1.Increment())
						{
							if (!image_it1.NextRow())
								break;
							image_it2.NextRow();
                     row_count++;
                     event_handler->Set(row_count);
							int * swap = error_row1;
							error_row1 = error_row2;
							error_row2 = swap;
							memset(error_row2, 0, (row_width + 6) * sizeof(int));
							erow1_ptr = error_row1 + 3;
							erow2_ptr = error_row2;
						}
					} while (1);
					delete new_opt_pal;
					delete [] error_row1;
					delete [] error_row2;
               event_handler->Stop();
					return new_data;
				}
				else
				{
					event_handler->Start();
               event_handler->Set(0L);
					do
					{
						image_it2.Set4(new_opt_pal->GetPixel(G42Color(image_it1.GetRed(), image_it1.GetGreen(),
							image_it1.GetBlue())));
						image_it2.Increment();
						if (!image_it1.Increment())
						{
							if (!image_it1.NextRow())
								break;
							image_it2.NextRow();
                     row_count++;
                     event_handler->Set(row_count);
						}
					} while (1);
					delete new_opt_pal;
               event_handler->Stop();
					return new_data;
				}
			}
		}
		case 8:
		{
			if (data->GetNumPalette() != 0)
			{
				if (pal_type == Color)
				{
					if (num_colors >= data->GetNumPalette())
						return data;
					if (num_colors > 16)
					{
						G42OptimizedPalette * new_opt_pal = new G42OptimizedPalette(data, num_colors);
						G42ImageData * new_data = new G42ImageData(data->GetData(),
							data->GetWidth(), data->GetHeight(), 8, new_opt_pal->GetNumPalette());
						new_data->ReferenceCount++;
						new_data->SetFinished(true);
						new_data->SetValidStartRow(0);
						new_data->SetValidNumRows(new_data->GetHeight());
						G42Color * new_pal = new_opt_pal->GetPalette();
						new_data->SetPalette(new_pal);
                  G42Color * old_pal = data->GetPalette();
						G42LockedID8BitIterator image_it1(data);
						G42ImageData8BitIterator image_it2(new_data);
						if (method == MethodDither)
						{
							int32 row_width = 0L;
							for (uint x = 0; x < data->GetNumTilesAcross(); x++)
							{
								G42ImageTile * tile = data->GetTile(0, x);
								row_width += tile->GetTileWidth();
							}
							row_width *= 3;
							int * error_row1 = new int [row_width + 6];
							memset(error_row1, 0, (row_width + 6) * sizeof(int));
							int * error_row2 = new int [row_width + 6];
							memset(error_row2, 0, (row_width + 6) * sizeof(int));
							int * erow1_ptr = error_row1 + 3;
							int * erow2_ptr = error_row2;
							int red, green, blue;
                     event_handler->Start();
                     event_handler->Set(0L);
							do
							{
                     	int e1 = *erow1_ptr;
                        CLIP_ERROR(e1);
                        erow1_ptr++;
                     	int e2 = *erow1_ptr;
                        CLIP_ERROR(e2);
                        erow1_ptr++;
                     	int e3 = *erow1_ptr;
                        CLIP_ERROR(e3);
                        erow1_ptr++;
								red = old_pal[(unsigned char)image_it1].red + e1;
								green = old_pal[(unsigned char)image_it1].green + e2;
								blue = old_pal[(unsigned char)image_it1].blue + e3;
                        LIMIT(red)
                        LIMIT(green)
                        LIMIT(blue)
								int val = new_opt_pal->GetPixel(G42Color(red, green, blue));
								image_it2.Set(val);
								int error = red - new_pal[val].red;
								DIFFUSE_ERROR(error)
								*erow1_ptr += error;
								erow1_ptr++;
								erow2_ptr -= 5;
								error = green - new_pal[val].green;
								DIFFUSE_ERROR(error)
								*erow1_ptr += error;
								erow1_ptr++;
								erow2_ptr -= 5;
								error = blue - new_pal[val].blue;
								DIFFUSE_ERROR(error)
								*erow1_ptr += error;
								erow2_ptr -= 5;
								erow1_ptr -= 2;
								image_it2.Increment();
								if (!image_it1.Increment())
								{
									if (!image_it1.NextRow())
										break;
									image_it2.NextRow();
                           row_count++;
                           event_handler->Set(row_count);
									int * swap = error_row1;
									error_row1 = error_row2;
									error_row2 = swap;
									memset(error_row2, 0, (row_width + 6) * sizeof(int));
									erow1_ptr = error_row1 + 3;
									erow2_ptr = error_row2;
								}
							} while (1);
							delete new_opt_pal;
							delete [] error_row1;
							delete [] error_row2;
                     event_handler->Stop();
							return new_data;
						}
						event_handler->Start();
                  event_handler->Set(0L);
						do
						{
							image_it2.Set(new_opt_pal->GetPixel(G42Color(
								old_pal[(unsigned char)image_it1].red,
								old_pal[(unsigned char)image_it1].green,
								old_pal[(unsigned char)image_it1].blue)));
							image_it2.Increment();
							if (!image_it1.Increment())
							{
								if (!image_it1.NextRow())
									break;
								image_it2.NextRow();
                        row_count++;
                        event_handler->Set(row_count);
							}
						} while (1);
						delete new_opt_pal;
                  event_handler->Stop();
						return new_data;
					}
					if (num_colors > 2)
					{
						G42OptimizedPalette * new_opt_pal = new G42OptimizedPalette(data, num_colors);
						G42ImageData * new_data = new G42ImageData(data->GetData(),
							data->GetWidth(), data->GetHeight(), 4, new_opt_pal->GetNumPalette());
						new_data->ReferenceCount++;
						new_data->SetFinished(true);
						new_data->SetValidStartRow(0);
						new_data->SetValidNumRows(new_data->GetHeight());
						G42Color * new_pal = new_opt_pal->GetPalette();
						new_data->SetPalette(new_pal);
						G42Color * old_pal = data->GetPalette();
						G42LockedID8BitIterator image_it1(data);
						G42ImageData4BitIterator image_it2(new_data);
						if (method == MethodDither)
						{
							int32 row_width = 0L;
							for (uint x = 0; x < data->GetNumTilesAcross(); x++)
							{
								G42ImageTile * tile = data->GetTile(0, x);
								row_width += tile->GetTileWidth();
							}
							row_width *= 3;
							int * error_row1 = new int [row_width + 6];
							memset(error_row1, 0, (row_width + 6) * sizeof(int));
							int * error_row2 = new int [row_width + 6];
							memset(error_row2, 0, (row_width + 6) * sizeof(int));
							int * erow1_ptr = error_row1 + 3;
							int * erow2_ptr = error_row2;
							int red, green, blue;
                     event_handler->Start();
                     event_handler->Set(0L);
							do
							{
                     	int e1 = *erow1_ptr;
                        CLIP_ERROR2(e1)
                        erow1_ptr++;
                     	int e2 = *erow1_ptr;
                        CLIP_ERROR2(e2)
                        erow1_ptr++;
                     	int e3 = *erow1_ptr;
                        CLIP_ERROR2(e3)
                        erow1_ptr++;
								red = old_pal[(unsigned char)image_it1].red + e1;
								green = old_pal[(unsigned char)image_it1].green + e2;
								blue = old_pal[(unsigned char)image_it1].blue + e3;
                        LIMIT(red)
                        LIMIT(green)
                        LIMIT(blue)
								int val = new_opt_pal->GetPixel(G42Color(red, green, blue));
								image_it2.Set4(val);
								int error = red - new_pal[val].red;
								DIFFUSE_ERROR(error)
								*erow1_ptr += error;
								erow1_ptr++;
								erow2_ptr -= 5;
								error = green - new_pal[val].green;
								DIFFUSE_ERROR(error)
								*erow1_ptr += error;
								erow1_ptr++;
								erow2_ptr -= 5;
								error = blue - new_pal[val].blue;
								DIFFUSE_ERROR(error)
								*erow1_ptr += error;
								erow2_ptr -= 5;
								erow1_ptr -= 2;
								image_it2.Increment();
								if (!image_it1.Increment())
								{
									if (!image_it1.NextRow())
										break;
									image_it2.NextRow();
                           row_count++;
                           event_handler->Set(row_count);
									int * swap = error_row1;
									error_row1 = error_row2;
									error_row2 = swap;
									memset(error_row2, 0, (row_width + 6) * sizeof(int));
									erow1_ptr = error_row1 + 3;
									erow2_ptr = error_row2;
								}
							} while (1);
							delete new_opt_pal;
							delete [] error_row1;
							delete [] error_row2;
                     event_handler->Stop();
							return new_data;
						}
                  event_handler->Start();
                  event_handler->Set(0L);
						do
						{
							image_it2.Set4(new_opt_pal->GetPixel(G42Color(
								old_pal[(unsigned char)image_it1].red,
								old_pal[(unsigned char)image_it1].green,
								old_pal[(unsigned char)image_it1].blue)));
							image_it2.Increment();
							if (!image_it1.Increment())
							{
								if (!image_it1.NextRow())
									break;
								image_it2.NextRow();
                        row_count++;
                        event_handler->Set(row_count);
							}
						} while (1);
						delete new_opt_pal;
                  event_handler->Stop();
						return new_data;
					}
					G42OptimizedPalette * new_opt_pal = new G42OptimizedPalette(data, num_colors);
					G42ImageData * new_data = new G42ImageData(data->GetData(),
						data->GetWidth(), data->GetHeight(), 1, new_opt_pal->GetNumPalette());
					new_data->ReferenceCount++;
					new_data->SetFinished(true);
					new_data->SetValidStartRow(0);
					new_data->SetValidNumRows(new_data->GetHeight());
					G42Color * new_pal = new_opt_pal->GetPalette();
					new_data->SetPalette(new_pal);
					G42Color * old_pal = data->GetPalette();
					G42LockedID8BitIterator image_it1(data);
					G42ImageData1BitIterator image_it2(new_data);
					if (method == MethodDither)
					{
						int32 row_width = 0L;
						for (uint x = 0; x < data->GetNumTilesAcross(); x++)
						{
							G42ImageTile * tile = data->GetTile(0, x);
							row_width += tile->GetTileWidth();
						}
						row_width *= 3;
						int * error_row1 = new int [row_width + 6];
						memset(error_row1, 0, (row_width + 6) * sizeof(int));
						int * error_row2 = new int [row_width + 6];
						memset(error_row2, 0, (row_width + 6) * sizeof(int));
						int * erow1_ptr = error_row1 + 3;
						int * erow2_ptr = error_row2;
						int red, green, blue;
                  event_handler->Start();
                  event_handler->Set(0L);
						do
						{
							red = old_pal[(unsigned char)image_it1].red + *erow1_ptr;
							erow1_ptr++;
							green = old_pal[(unsigned char)image_it1].green + *erow1_ptr;
							erow1_ptr++;
							blue = old_pal[(unsigned char)image_it1].blue + *erow1_ptr;
							erow1_ptr++;
                     LIMIT(red)
                     LIMIT(green)
                     LIMIT(blue)
							int val = new_opt_pal->GetPixel(G42Color(red, green, blue));
							image_it2.Set8(val);
							int error = red - new_pal[val].red;
							DIFFUSE_ERROR(error)
							*erow1_ptr += error;
							erow1_ptr++;
							erow2_ptr -= 5;
							error = green - new_pal[val].green;
							DIFFUSE_ERROR(error)
							*erow1_ptr += error;
							erow1_ptr++;
							erow2_ptr -= 5;
							error = blue - new_pal[val].blue;
							DIFFUSE_ERROR(error)
							*erow1_ptr += error;
							erow2_ptr -= 5;
							erow1_ptr -= 2;
							image_it2.Increment();
							if (!image_it1.Increment())
							{
								if (!image_it1.NextRow())
									break;
								image_it2.NextRow();
                        row_count++;
                        event_handler->Set(row_count);
								int * swap = error_row1;
								error_row1 = error_row2;
								error_row2 = swap;
								memset(error_row2, 0, (row_width + 6) * sizeof(int));
								erow1_ptr = error_row1 + 3;
								erow2_ptr = error_row2;
							}
						} while (1);
						delete new_opt_pal;
						delete [] error_row1;
						delete [] error_row2;
                  event_handler->Stop();
						return new_data;
					}
					event_handler->Start();
               event_handler->Set(0L);
					do
					{
						image_it2.Set8(new_opt_pal->GetPixel(G42Color(
							old_pal[(unsigned char)image_it1].red,
							old_pal[(unsigned char)image_it1].green,
							old_pal[(unsigned char)image_it1].blue)));
						image_it2.Increment();
						if (!image_it1.Increment())
						{
							if (!image_it1.NextRow())
								break;
							image_it2.NextRow();
                     row_count++;
                     event_handler->Set(row_count);
						}
					} while (1);
					delete new_opt_pal;
               event_handler->Stop();
					return new_data;
				}
				if (pal_type == Gray)
				{
					if (num_colors > 16)
					{
						G42ImageData * new_data = new G42ImageData(data->GetData(),
							data->GetWidth(), data->GetHeight(), 8, 0);
						new_data->ReferenceCount++;
						new_data->SetFinished(true);
						new_data->SetValidStartRow(0);
						new_data->SetValidNumRows(new_data->GetHeight());
						G42Color * old_pal = data->GetPalette();
						unsigned char * map = new unsigned char [256];
						int index = 0;
						for (int32 n = 0; n < num_colors; n++)
						{
							int val = (n * 255L)/(int32)(num_colors - 1);
							int u_limit = ((n * 255L) + 127L)/(int32)(num_colors - 1);
							if (u_limit > 255)
								u_limit = 255;
							for (; index <= u_limit; index++)
								map[index] = val;
						}
						for (; index < 256; index++)
							map[index] = 255;
						G42LockedID8BitIterator image_it1(data);
						G42ImageData8BitIterator image_it2(new_data);
						if (method == MethodDither)
						{
							int32 row_width = 0L;
							for (uint x = 0; x < data->GetNumTilesAcross(); x++)
							{
								G42ImageTile * tile = data->GetTile(0, x);
								row_width += tile->GetTileWidth();
							}
							int * error_row1 = new int [row_width + 2];
							memset(error_row1, 0, (row_width + 2) * sizeof(int));
							int * error_row2 = new int [row_width + 2];
							memset(error_row2, 0, (row_width + 2) * sizeof(int));
							int * erow1_ptr = error_row1 + 1;
							int * erow2_ptr = error_row2;
							int red, green, blue;
                     event_handler->Start();
                     event_handler->Set(0L);
							do
							{
								red = old_pal[(unsigned char)image_it1].red;
								green = old_pal[(unsigned char)image_it1].green;
								blue = old_pal[(unsigned char)image_it1].blue;
								int val = (int)((((int32)red * (int32)306) +
									((int32)green * (int32)601) +
									((int32)blue * (int32)117) + (int32)512) >> 10);
                        int e1 = *erow1_ptr;
                        CLIP_ERROR(e1)
								val += e1;
								LIMIT(val)
                        erow1_ptr++;
								image_it2.Set(map[val]);
								int error = val - map[val];
								DIFFUSE_ERROR2(error)
								*erow1_ptr += error;
								erow2_ptr--;
								image_it2.Increment();
								if (!image_it1.Increment())
								{
									if (!image_it1.NextRow())
										break;
									image_it2.NextRow();
                           row_count++;
                           event_handler->Set(row_count);
									int * swap = error_row1;
									error_row1 = error_row2;
									error_row2 = swap;
									memset(error_row2, 0, (row_width + 2) * sizeof(int));
									erow1_ptr = error_row1 + 1;
									erow2_ptr = error_row2;
								}
							} while (1);
							delete [] error_row1;
							delete [] error_row2;
							delete [] map;
                     event_handler->Stop();
							return new_data;
						}
						else
						{
							event_handler->Start();
                     event_handler->Set(0L);
							do
							{
								int32 red = (int32)old_pal[(unsigned char)image_it1].red;
								int32 green = (int32)old_pal[(unsigned char)image_it1].green;
								int32 blue = (int32)old_pal[(unsigned char)image_it1].blue;
								int val = (int)(((red * (int32)306) +
									(green * (int32)601) +
									(blue * (int32)117) + (int32)512) >> 10);
								if (val > 255)
									val = 255;
								image_it2.Set(map[val]);
								image_it2.Increment();
								if (!image_it1.Increment())
								{
									if (!image_it1.NextRow())
										break;
									image_it2.NextRow();
                           row_count++;
                           event_handler->Set(row_count);
								}
							} while (1);
							delete [] map;
                     event_handler->Stop();
							return new_data;
						}
					}
					if (num_colors > 2)
					{
						G42ImageData * new_data = new G42ImageData(data->GetData(),
							data->GetWidth(), data->GetHeight(), 4, 0);
						new_data->ReferenceCount++;
						new_data->SetFinished(true);
						new_data->SetValidStartRow(0);
						new_data->SetValidNumRows(new_data->GetHeight());
						G42Color * old_pal = data->GetPalette();
						unsigned char * map = new unsigned char [256];
						int index = 0;
						for (int32 n = 0; n < num_colors; n++)
						{
							int val = (n * 15L)/(int32)(num_colors - 1);
							int u_limit = ((n * 255L) + 127L)/(int32)(num_colors - 1);
							if (u_limit > 255)
								u_limit = 255;
							for (; index <= u_limit; index++)
								map[index] = val;
						}
						for (; index < 256; index++)
							map[index] = 15;
						G42LockedID8BitIterator image_it1(data);
						G42ImageData4BitIterator image_it2(new_data);
						if (method == MethodDither)
						{
							int32 row_width = 0L;
							for (uint x = 0; x < data->GetNumTilesAcross(); x++)
							{
								G42ImageTile * tile = data->GetTile(0, x);
								row_width += tile->GetTileWidth();
							}
							int * error_row1 = new int [row_width + 2];
							memset(error_row1, 0, (row_width + 2) * sizeof(int));
							int * error_row2 = new int [row_width + 2];
							memset(error_row2, 0, (row_width + 2) * sizeof(int));
							int * erow1_ptr = error_row1 + 1;
							int * erow2_ptr = error_row2;
							int red, green, blue;
                     event_handler->Start();
                     event_handler->Set(0L);
							do
							{
								red = old_pal[(unsigned char)image_it1].red;
								green = old_pal[(unsigned char)image_it1].green;
								blue = old_pal[(unsigned char)image_it1].blue;
								int val = (int)((((int32)red * (int32)306) +
									((int32)green * (int32)601) +
									((int32)blue * (int32)117) + (int32)512) >> 10);
                        int e1 = *erow1_ptr;
                        CLIP_ERROR2(e1)
								val += e1;
                        LIMIT(val)
								erow1_ptr++;
								image_it2.Set4(map[val]);
								int error = val - ((map[val] << 4) + map[val]);
								DIFFUSE_ERROR2(error)
								*erow1_ptr += error;
								erow2_ptr--;
								image_it2.Increment();
								if (!image_it1.Increment())
								{
									if (!image_it1.NextRow())
										break;
									image_it2.NextRow();
                           row_count++;
                           event_handler->Set(row_count);
									int * swap = error_row1;
									error_row1 = error_row2;
									error_row2 = swap;
									memset(error_row2, 0, (row_width + 2) * sizeof(int));
									erow1_ptr = error_row1 + 1;
									erow2_ptr = error_row2;
								}
							} while (1);
							delete [] error_row1;
							delete [] error_row2;
							delete [] map;
                     event_handler->Stop();
							return new_data;
						}
						else
						{
                  	event_handler->Start();
                     event_handler->Set(0L);
							do
							{
								int red = old_pal[(unsigned char)image_it1].red;
								int green = old_pal[(unsigned char)image_it1].green;
								int blue = old_pal[(unsigned char)image_it1].blue;
								int val = (int)((((int32)red * (int32)306) +
									((int32)green * (int32)601) +
									((int32)blue * (int32)117) + (int32)512) >> 10);
								if (val > 255)
									val = 255;
								if (val < 0)
									val = 0;
								image_it2.Set4(map[val]);
								image_it2.Increment();
								if (!image_it1.Increment())
								{
									if (!image_it1.NextRow())
										break;
									image_it2.NextRow();
                           row_count++;
                           event_handler->Set(row_count);
								}
							} while (1);
							delete [] map;
                     event_handler->Stop();
							return new_data;
						}
					}
					G42ImageData * new_data = new G42ImageData(data->GetData(),
						data->GetWidth(), data->GetHeight(), 1, 0);
					new_data->ReferenceCount++;
					new_data->SetFinished(true);
					new_data->SetValidStartRow(0);
					new_data->SetValidNumRows(new_data->GetHeight());
					G42Color * old_pal = data->GetPalette();
					G42LockedID8BitIterator image_it1(data);
					G42ImageData1BitIterator image_it2(new_data);
					if (method == MethodDither)
					{
						int32 row_width = 0L;
						for (uint x = 0; x < data->GetNumTilesAcross(); x++)
						{
							G42ImageTile * tile = data->GetTile(0, x);
							row_width += tile->GetTileWidth();
						}
						int * error_row1 = new int [row_width + 2];
						memset(error_row1, 0, (row_width + 2) * sizeof(int));
						int * error_row2 = new int [row_width + 2];
						memset(error_row2, 0, (row_width + 2) * sizeof(int));
						int * erow1_ptr = error_row1 + 1;
						int * erow2_ptr = error_row2;
						int red, green, blue;
                  event_handler->Start();
                  event_handler->Set(0L);
						do
						{
							red = old_pal[(unsigned char)image_it1].red;
							green = old_pal[(unsigned char)image_it1].green;
							blue = old_pal[(unsigned char)image_it1].blue;
							int val = (int)((((int32)red * (int32)306) +
								((int32)green * (int32)601) +
								((int32)blue * (int32)117) + (int32)512) >> 10);
							val += *erow1_ptr;
							erow1_ptr++;
                     LIMIT(val)
							int cval = 0;
							if (val > 127)
								cval = 255;
							image_it2.Set8(cval);
							int error = val - cval;
							DIFFUSE_ERROR2(error)
							*erow1_ptr += error;
							erow2_ptr--;
							image_it2.Increment();
							if (!image_it1.Increment())
							{
								if (!image_it1.NextRow())
									break;
								image_it2.NextRow();
                        row_count++;
                        event_handler->Set(row_count);
								int * swap = error_row1;
								error_row1 = error_row2;
								error_row2 = swap;
								memset(error_row2, 0, (row_width + 2) * sizeof(int));
								erow1_ptr = error_row1 + 1;
								erow2_ptr = error_row2;
							}
						} while (1);
						delete [] error_row1;
						delete [] error_row2;
                  event_handler->Stop();
						return new_data;
					}
					else
					{
               	event_handler->Start();
                  event_handler->Set(0L);
						do
						{
							int32 red = (int32)old_pal[(unsigned char)image_it1].red;
							int32 green = (int32)old_pal[(unsigned char)image_it1].green;
							int32 blue = (int32)old_pal[(unsigned char)image_it1].blue;
							int val = (int)(((red * (int32)306) +
								(green * (int32)601) +
								(blue * (int32)117) + (int32)512) >> 10);
							if (val > 127)
								image_it2.Set8(true);
							else
								image_it2.Set8(false);
							image_it2.Increment();
							if (!image_it1.Increment())
							{
								if (!image_it1.NextRow())
									break;
								image_it2.NextRow();
                        row_count++;
                        event_handler->Set(row_count);
							}
						} while (1);
                  event_handler->Stop();
						return new_data;
					}
				}
				if (pal_type == WinColor)
				{
					if (num_colors > 16)
               	return data;
					G42OptimizedPalette * new_opt_pal = new G42OptimizedPalette(data, num_colors,
               	false, true);
					G42ImageData * new_data = new G42ImageData(data->GetData(),
						data->GetWidth(), data->GetHeight(), 4, new_opt_pal->GetNumPalette());
					new_data->ReferenceCount++;
					new_data->SetFinished(true);
					new_data->SetValidStartRow(0);
					new_data->SetValidNumRows(new_data->GetHeight());
					G42Color * new_pal = new_opt_pal->GetPalette();
					new_data->SetPalette(new_pal);
					G42Color * old_pal = data->GetPalette();
					G42LockedID8BitIterator image_it1(data);
					G42ImageData4BitIterator image_it2(new_data);
					if (method == MethodDither)
					{
						int32 row_width = 0L;
						for (uint x = 0; x < data->GetNumTilesAcross(); x++)
						{
							G42ImageTile * tile = data->GetTile(0, x);
							row_width += tile->GetTileWidth();
						}
						row_width *= 3;
						int * error_row1 = new int [row_width + 6];
						memset(error_row1, 0, (row_width + 6) * sizeof(int));
						int * error_row2 = new int [row_width + 6];
						memset(error_row2, 0, (row_width + 6) * sizeof(int));
						int * erow1_ptr = error_row1 + 3;
						int * erow2_ptr = error_row2;
						int red, green, blue;
                  event_handler->Start();
                  event_handler->Set(0L);
						do
						{
                  	int e1 = *erow1_ptr;
                     CLIP_ERROR2(e1);
                     erow1_ptr++;
                  	int e2 = *erow1_ptr;
                     CLIP_ERROR2(e2);
                     erow1_ptr++;
                  	int e3 = *erow1_ptr;
                     CLIP_ERROR2(e3);
                     erow1_ptr++;
							red = old_pal[(unsigned char)image_it1].red + e1;
							green = old_pal[(unsigned char)image_it1].green + e2;
							blue = old_pal[(unsigned char)image_it1].blue + e3;
                     LIMIT(red)
                     LIMIT(green)
                     LIMIT(blue)
							int val = new_opt_pal->GetPixel(G42Color(red, green, blue));
							image_it2.Set4(val);
							int error = red - new_pal[val].red;
							DIFFUSE_ERROR(error)
							*erow1_ptr += error;
							erow1_ptr++;
							erow2_ptr -= 5;
							error = green - new_pal[val].green;
							DIFFUSE_ERROR(error)
							*erow1_ptr += error;
							erow1_ptr++;
							erow2_ptr -= 5;
							error = blue - new_pal[val].blue;
							DIFFUSE_ERROR(error)
							*erow1_ptr += error;
							erow2_ptr -= 5;
							erow1_ptr -= 2;
							image_it2.Increment();
							if (!image_it1.Increment())
							{
								if (!image_it1.NextRow())
									break;
								image_it2.NextRow();
                        row_count++;
                        event_handler->Set(row_count);
								int * swap = error_row1;
								error_row1 = error_row2;
								error_row2 = swap;
								memset(error_row2, 0, (row_width + 6) * sizeof(int));
								erow1_ptr = error_row1 + 3;
								erow2_ptr = error_row2;
							}
						} while (1);
						delete new_opt_pal;
						delete [] error_row1;
						delete [] error_row2;
                  event_handler->Stop();
						return new_data;
					}
	            event_handler->Start();
               event_handler->Set(0L);
					do
					{
						image_it2.Set4(new_opt_pal->GetPixel(G42Color(
							old_pal[(unsigned char)image_it1].red,
							old_pal[(unsigned char)image_it1].green,
							old_pal[(unsigned char)image_it1].blue)));
						image_it2.Increment();
						if (!image_it1.Increment())
						{
							if (!image_it1.NextRow())
								break;
							image_it2.NextRow();
                     row_count++;
                     event_handler->Set(row_count);
						}
					} while (1);
					delete new_opt_pal;
               event_handler->Stop();
					return new_data;
				}
            return data;
			}
			else // 8 bit non paletted
			{
				if ((pal_type == Color) || (pal_type == Gray))
				{
					if (num_colors > 16)
					{
						G42ImageData * new_data = new G42ImageData(data->GetData(),
							data->GetWidth(), data->GetHeight(), 8, 0);
						new_data->ReferenceCount++;
						new_data->SetFinished(true);
						new_data->SetValidStartRow(0);
						new_data->SetValidNumRows(new_data->GetHeight());
						unsigned char * map = new unsigned char [256];
						int index = 0;
						for (int32 n = 0; n < num_colors; n++)
						{
							int val = (n * 255L)/(int32)(num_colors - 1);
							int u_limit = ((n * 255L) + 127L)/(int32)(num_colors - 1);
							if (u_limit > 255)
								u_limit = 255;
							for (; index <= u_limit; index++)
								map[index] = val;
						}
						for (; index < 256; index++)
							map[index] = 255;
						G42LockedID8BitIterator image_it1(data);
						G42ImageData8BitIterator image_it2(new_data);
						if (method == MethodDither)
						{
							int32 row_width = 0L;
							for (uint x = 0; x < data->GetNumTilesAcross(); x++)
							{
								G42ImageTile * tile = data->GetTile(0, x);
								row_width += tile->GetTileWidth();
							}
							int * error_row1 = new int [row_width + 2];
							memset(error_row1, 0, (row_width + 2) * sizeof(int));
							int * error_row2 = new int [row_width + 2];
							memset(error_row2, 0, (row_width + 2) * sizeof(int));
							int * erow1_ptr = error_row1 + 1;
							int * erow2_ptr = error_row2;
                     event_handler->Start();
                     event_handler->Set(0L);
							do
							{
                     	int e1 = *erow1_ptr;
                        CLIP_ERROR(e1)
								int val = (int)(unsigned char)image_it1 + e1;
								erow1_ptr++;
                        LIMIT(val)
								image_it2.Set(map[val]);
								int error = val - map[val];
								DIFFUSE_ERROR2(error)
								*erow1_ptr += error;
								erow2_ptr--;
								image_it2.Increment();
								if (!image_it1.Increment())
								{
									if (!image_it1.NextRow())
										break;
									image_it2.NextRow();
                           row_count++;
                           event_handler->Set(row_count);
									int * swap = error_row1;
									error_row1 = error_row2;
									error_row2 = swap;
									memset(error_row2, 0, (row_width + 2) * sizeof(int));
									erow1_ptr = error_row1 + 1;
									erow2_ptr = error_row2;
								}
							} while (1);
							delete [] map;
							delete [] error_row1;
							delete [] error_row2;
                     event_handler->Stop();
							return new_data;
						}
						event_handler->Start();
                  event_handler->Set(0L);
						do
						{
							image_it2.Set(map[(unsigned char)image_it1]);
							image_it2.Increment();
							if (!image_it1.Increment())
							{
								if (!image_it1.NextRow())
									break;
								image_it2.NextRow();
                        row_count++;
                        event_handler->Set(row_count);
							}
						} while (1);
						delete [] map;
                  event_handler->Stop();
						return new_data;
					}
					if (num_colors > 2)
					{
						G42ImageData * new_data = new G42ImageData(data->GetData(),
							data->GetWidth(), data->GetHeight(), 4, 0);
						new_data->ReferenceCount++;
						new_data->SetFinished(true);
						new_data->SetValidStartRow(0);
						new_data->SetValidNumRows(new_data->GetHeight());
						unsigned char * map = new unsigned char [256];
						int index = 0;
						for (int32 n = 0; n < num_colors; n++)
						{
							int val = (n * 15L)/(int32)(num_colors - 1);
							int u_limit = ((n * 255L) + 127L)/(int32)(num_colors - 1);
							if (u_limit > 255)
								u_limit = 255;
							for (; index <= u_limit; index++)
								map[index] = val;
						}
						for (; index < 256; index++)
							map[index] = 15;
						G42LockedID8BitIterator image_it1(data);
						G42ImageData4BitIterator image_it2(new_data);
						if (method == MethodDither)
						{
							int32 row_width = 0L;
							for (uint x = 0; x < data->GetNumTilesAcross(); x++)
							{
								G42ImageTile * tile = data->GetTile(0, x);
								row_width += tile->GetTileWidth();
							}
							int * error_row1 = new int [row_width + 2];
							memset(error_row1, 0, (row_width + 2) * sizeof(int));
							int * error_row2 = new int [row_width + 2];
							memset(error_row2, 0, (row_width + 2) * sizeof(int));
							int * erow1_ptr = error_row1 + 1;
							int * erow2_ptr = error_row2;
                     event_handler->Start();
                     event_handler->Set(0L);
							do
							{
                     	int e1 = *erow1_ptr;
                        CLIP_ERROR2(e1)
								int val = (int)(unsigned char)image_it1 + e1;
								erow1_ptr++;
                        LIMIT(val)
								image_it2.Set4(map[val]);
								int error = val - ((map[val] << 4) + map[val]);
								DIFFUSE_ERROR2(error)
								*erow1_ptr += error;
								erow2_ptr--;
								image_it2.Increment();
								if (!image_it1.Increment())
								{
									if (!image_it1.NextRow())
										break;
									image_it2.NextRow();
                           row_count++;
                           event_handler->Set(row_count);
									int * swap = error_row1;
									error_row1 = error_row2;
									error_row2 = swap;
									memset(error_row2, 0, (row_width + 2) * sizeof(int));
									erow1_ptr = error_row1 + 1;
									erow2_ptr = error_row2;
								}
							} while (1);
							delete [] map;
							delete [] error_row1;
							delete [] error_row2;
                     event_handler->Stop();
							return new_data;
						}
						event_handler->Start();
                  event_handler->Set(0L);
						do
						{
							image_it2.Set4(map[(unsigned char)image_it1]);
							image_it2.Increment();
							if (!image_it1.Increment())
							{
								if (!image_it1.NextRow())
									break;
								image_it2.NextRow();
                        row_count++;
                        event_handler->Set(row_count);
							}
						} while (1);
						delete [] map;
                  event_handler->Stop();
						return new_data;
					}
					G42ImageData * new_data = new G42ImageData(data->GetData(),
						data->GetWidth(), data->GetHeight(), 1, 0);
					new_data->ReferenceCount++;
					new_data->SetFinished(true);
					new_data->SetValidStartRow(0);
					new_data->SetValidNumRows(new_data->GetHeight());
					G42LockedID8BitIterator image_it1(data);
					G42ImageData1BitIterator image_it2(new_data);
					if (method == MethodDither)
					{
						int32 row_width = 0L;
						for (uint x = 0; x < data->GetNumTilesAcross(); x++)
						{
							G42ImageTile * tile = data->GetTile(0, x);
							row_width += tile->GetTileWidth();
						}
						int * error_row1 = new int [row_width + 2];
						memset(error_row1, 0, (row_width + 2) * sizeof(int));
						int * error_row2 = new int [row_width + 2];
						memset(error_row2, 0, (row_width + 2) * sizeof(int));
						int * erow1_ptr = error_row1 + 1;
						int * erow2_ptr = error_row2;
                  event_handler->Start();
                  event_handler->Set(0L);
						do
						{
							int val = (int)(unsigned char)image_it1;
							val += *erow1_ptr;
							erow1_ptr++;
                     LIMIT(val)
							int cval = 0;
							if (val > 127)
								cval = 255;
							image_it2.Set8(cval);
							int error = val - cval;
							DIFFUSE_ERROR2(error)
							*erow1_ptr += error;
							erow2_ptr--;
							image_it2.Increment();
							if (!image_it1.Increment())
							{
								if (!image_it1.NextRow())
									break;
								image_it2.NextRow();
                        row_count++;
                        event_handler->Set(row_count);
								int * swap = error_row1;
								error_row1 = error_row2;
								error_row2 = swap;
								memset(error_row2, 0, (row_width + 2) * sizeof(int));
								erow1_ptr = error_row1 + 1;
								erow2_ptr = error_row2;
							}
						} while (1);
						delete [] error_row1;
						delete [] error_row2;
                  event_handler->Stop();
						return new_data;
					}
					else
					{
               	event_handler->Start();
                  event_handler->Set(0L);
						do
						{
							int val = (int)(unsigned char)image_it1;;
							if (val > 127)
								image_it2.Set8(true);
							else
								image_it2.Set8(false);
							image_it2.Increment();
							if (!image_it1.Increment())
							{
								if (!image_it1.NextRow())
									break;
								image_it2.NextRow();
                        row_count++;
                        event_handler->Set(row_count);
							}
						} while (1);
                  event_handler->Stop();
						return new_data;
					}
				}
				if (pal_type == WinColor)
				{
					if (num_colors > 16)
						return data;
					G42OptimizedPalette * new_opt_pal = new G42OptimizedPalette(data, num_colors,
               	false, true);
					G42ImageData * new_data = new G42ImageData(data->GetData(),
						data->GetWidth(), data->GetHeight(), 4, new_opt_pal->GetNumPalette());
					new_data->ReferenceCount++;
					new_data->SetFinished(true);
					new_data->SetValidStartRow(0);
					new_data->SetValidNumRows(new_data->GetHeight());
					G42Color * new_pal = new_opt_pal->GetPalette();
					new_data->SetPalette(new_pal);
					G42LockedID8BitIterator image_it1(data);
					G42ImageData4BitIterator image_it2(new_data);
					if (method == MethodDither)
					{
						int32 row_width = 0L;
						for (uint x = 0; x < data->GetNumTilesAcross(); x++)
						{
							G42ImageTile * tile = data->GetTile(0, x);
							row_width += tile->GetTileWidth();
						}
						row_width *= 3;
						int * error_row1 = new int [row_width + 6];
						memset(error_row1, 0, (row_width + 6) * sizeof(int));
						int * error_row2 = new int [row_width + 6];
						memset(error_row2, 0, (row_width + 6) * sizeof(int));
						int * erow1_ptr = error_row1 + 3;
						int * erow2_ptr = error_row2;
						int red, green, blue;
                  event_handler->Start();
                  event_handler->Set(0L);
						do
						{
                  	int e1 = *erow1_ptr;
                     CLIP_ERROR2(e1);
                     erow1_ptr++;
                  	int e2 = *erow1_ptr;
                     CLIP_ERROR2(e2);
                     erow1_ptr++;
                  	int e3 = *erow1_ptr;
                     CLIP_ERROR2(e3);
                     erow1_ptr++;
							red = green = blue = (int)(unsigned char)image_it1;
							red += e1;
							green += e2;
							blue += e3;
                     LIMIT(red)
                     LIMIT(green)
                     LIMIT(blue)
							int val = new_opt_pal->GetPixel(G42Color(red, green, blue));
							image_it2.Set4(val);
							int error = red - new_pal[val].red;
							DIFFUSE_ERROR(error)
							*erow1_ptr += error;
							erow1_ptr++;
							erow2_ptr -= 5;
							error = green - new_pal[val].green;
							DIFFUSE_ERROR(error)
							*erow1_ptr += error;
							erow1_ptr++;
							erow2_ptr -= 5;
							error = blue - new_pal[val].blue;
							DIFFUSE_ERROR(error)
							*erow1_ptr += error;
							erow1_ptr -= 2;
							erow2_ptr -= 5;
							image_it2.Increment();
							if (!image_it1.Increment())
							{
								if (!image_it1.NextRow())
									break;
								image_it2.NextRow();
                        row_count++;
                        event_handler->Set(row_count);
								int * swap = error_row1;
								error_row1 = error_row2;
								error_row2 = swap;
								memset(error_row2, 0, (row_width + 6) * sizeof(int));
								erow1_ptr = error_row1 + 3;
								erow2_ptr = error_row2;
							}
						} while (1);
						delete new_opt_pal;
						delete [] error_row1;
						delete [] error_row2;
                  event_handler->Stop();
						return new_data;
					}
					event_handler->Start();
               event_handler->Set(0L);
					do
					{
						int red, green, blue;
						red = green = blue = (int)(unsigned char)image_it1;
						image_it2.Set4(new_opt_pal->GetPixel(G42Color(red, green, blue)));
						image_it2.Increment();
						if (!image_it1.Increment())
						{
							if (!image_it1.NextRow())
								break;
							image_it2.NextRow();
                     row_count++;
                     event_handler->Set(row_count);
						}
					} while (1);
					delete new_opt_pal;
               event_handler->Stop();
					return new_data;
				}
			}
         return data;
		}
		case 4:
		{
			return ColorReduce4Bit(data, method, pal_type, num_colors, event_handler);
		}
		case 1:
		{
			return ColorReduce1Bit(data, method, pal_type, num_colors, event_handler);
      }
		default:
			return data;
	}
}
void
Dither(G42ImageData * data, G42ImageData * new_data, int left, int top,
	int right, int bottom, G42OptimizedPalette * opt_pal, bool match = false)
{
	int MinRow = new_data->GetTileRowIndex(top);
	int MaxRow = new_data->GetTileRowIndex(bottom);
	int MinColumn = new_data->GetTileColumnIndex(left);
	int MaxColumn = new_data->GetTileColumnIndex(right);
	for (int n = MinRow; n <= MaxRow; n++)
   {
   	for (int i = MinColumn; i <= MaxColumn; i++)
      {
    		G42ImageTile * tile = new_data->GetTile(n, i);
         if (!tile)
         	return;
      }
   }
   MinRow = data->GetTileRowIndex(top);
	MaxRow = data->GetTileRowIndex(bottom);
	MinColumn = data->GetTileColumnIndex(left);
	MaxColumn = data->GetTileColumnIndex(right);
	for (int n = MinRow; n <= MaxRow; n++)
   {
   	for (int i = MinColumn; i <= MaxColumn; i++)
      {
    		G42ImageTile * tile = data->GetTile(n, i);
         if (!tile)
         	return;
      }
   }
	if (match)
   {
   	Match(data, new_data, left, top, right, bottom, opt_pal);
   	return;
   }
	switch (data->GetDepth())
   {
   	case 32:
   	case 24:
	   {
#if 1
			Reduce24ColorTo8Fast (data, new_data, left, top, right, bottom, opt_pal);
#else
			G42LockedID24BitIterator image_it1(data, false);
			image_it1.SetArea(left, top, right, bottom);
			G42ImageData8BitIterator image_it2(new_data, false);
			image_it2.SetArea(left, top, right, bottom);
			int32 row_width = right - left + 1;
			row_width *= image_it1.GetCellSize();
			int * error_row1 = new int [row_width + 6];
			memset(error_row1, 0, (row_width + 6) * sizeof(int));
			int * error_row2 = new int [row_width + 6];
			memset(error_row2, 0, (row_width + 6) * sizeof(int));
			int * erow1_ptr = error_row1 + 3;
			int * erow2_ptr = error_row2;
			G42Color * palette = opt_pal->GetPalette();
			int red, green, blue;
			do
			{
		   	int e1 = *erow1_ptr;
      		CLIP_ERROR(e1)
		      erow1_ptr++;
   			int e2 = *erow1_ptr;
		      CLIP_ERROR(e2)
      		erow1_ptr++;
		   	int e3 = *erow1_ptr;
      		CLIP_ERROR(e3)
		      erow1_ptr++;
				red = image_it1.GetRed() + e1;
				green = image_it1.GetGreen() + e2;
				blue = image_it1.GetBlue() + e3;
            LIMIT(red)
            LIMIT(green)
            LIMIT(blue)
				int val = opt_pal->GetPixel(G42Color(red, green, blue));
				image_it2.Set(val);
				int error = red - palette[val].red;
				DIFFUSE_ERROR(error)
				*erow1_ptr += error;
				erow1_ptr++;
				erow2_ptr -= 5;
				error = green - palette[val].green;
				DIFFUSE_ERROR(error)
				*erow1_ptr += error;
				erow1_ptr++;
				erow2_ptr -= 5;
				error = blue - palette[val].blue;
				DIFFUSE_ERROR(error)
				*erow1_ptr += error;
				erow2_ptr -= 5;
				erow1_ptr -= 2;
				image_it2.Increment();
				if (!image_it1.Increment())
				{
					if (!image_it1.NextRow())
						break;
					image_it2.NextRow();
					int * swap = error_row1;
					error_row1 = error_row2;
					error_row2 = swap;
					memset(error_row2, 0, (row_width + 6) * sizeof(int));
					erow1_ptr = error_row1 + 3;
					erow2_ptr = error_row2;
				}
			} while (1);
			delete [] error_row1;
			delete [] error_row2;
#endif         
			break;
		}
		case 8:
      {
         if (data->GetNumPalette() != 0)
         {
#if 1
				Reduce8ColorTo8Fast (data, new_data, left, top, right, bottom, opt_pal);
#else
				G42LockedID8BitIterator image_it1(data, false);
				image_it1.SetArea(left, top, right, bottom);
				G42ImageData8BitIterator image_it2(new_data, false);
				image_it2.SetArea(left, top, right, bottom);
				int32 row_width = right - left + 1;
            row_width *= 3;
				int * error_row1 = new int [row_width + 6];
				memset(error_row1, 0, (row_width + 6) * sizeof(int));
				int * error_row2 = new int [row_width + 6];
				memset(error_row2, 0, (row_width + 6) * sizeof(int));
				int * erow1_ptr = error_row1 + 3;
				int * erow2_ptr = error_row2;
				G42Color * palette = opt_pal->GetPalette();
			   G42Color * old_pal = data->GetPalette();
				int red, green, blue;
				do
				{
			   	int e1 = *erow1_ptr;
			      CLIP_ERROR(e1)
			      erow1_ptr++;
			   	int e2 = *erow1_ptr;
					CLIP_ERROR(e2)
					erow1_ptr++;
			   	int e3 = *erow1_ptr;
			      CLIP_ERROR(e3)
			      erow1_ptr++;
					red = old_pal[(unsigned char)image_it1].red + e1;
					green = old_pal[(unsigned char)image_it1].green + e2;
					blue = old_pal[(unsigned char)image_it1].blue + e3;
               LIMIT(red)
               LIMIT(green)
               LIMIT(blue)
					int val = opt_pal->GetPixel(G42Color(red, green, blue));
					image_it2.Set(val);
					int error = red - palette[val].red;
					DIFFUSE_ERROR(error)
					*erow1_ptr += error;
					erow1_ptr++;
					erow2_ptr -= 5;
					error = green - palette[val].green;
					DIFFUSE_ERROR(error)
					*erow1_ptr += error;
					erow1_ptr++;
					erow2_ptr -= 5;
					error = blue - palette[val].blue;
					DIFFUSE_ERROR(error)
					*erow1_ptr += error;
					erow2_ptr -= 5;
					erow1_ptr -= 2;
					image_it2.Increment();
					if (!image_it1.Increment())
					{
						if (!image_it1.NextRow())
							break;
						image_it2.NextRow();
						int * swap = error_row1;
						error_row1 = error_row2;
						error_row2 = swap;
						memset(error_row2, 0, (row_width + 6) * sizeof(int));
						erow1_ptr = error_row1 + 3;
						erow2_ptr = error_row2;
					}
				} while (1);
				delete [] error_row1;
				delete [] error_row2;
#endif
			}
			else
			{
#if 1
				Reduce8GrayTo8Fast (data, new_data, left, top, right, bottom, opt_pal);
#else
				G42LockedID8BitIterator image_it1(data, false);
				image_it1.SetArea(left, top, right, bottom);
				G42ImageData8BitIterator image_it2(new_data, false);
				image_it2.SetArea(left, top, right, bottom);
				int32 row_width = right - left + 1;
            row_width *= 3;
				int * error_row1 = new int [row_width + 6];
				memset(error_row1, 0, (row_width + 6) * sizeof(int));
				int * error_row2 = new int [row_width + 6];
				memset(error_row2, 0, (row_width + 6) * sizeof(int));
				int * erow1_ptr = error_row1 + 3;
				int * erow2_ptr = error_row2;
				G42Color * palette = opt_pal->GetPalette();
				int red, green, blue;
				do
				{
					int e1 = *erow1_ptr;
			      CLIP_ERROR(e1)
			      erow1_ptr++;
			   	int e2 = *erow1_ptr;
			      CLIP_ERROR(e2)
			      erow1_ptr++;
			   	int e3 = *erow1_ptr;
					CLIP_ERROR(e3)
			      erow1_ptr++;
               int val = (int)(unsigned char)image_it1;
					red = val + e1;
					green = val + e2;
					blue = val + e3;
               LIMIT(red)
               LIMIT(green)
               LIMIT(blue)
					val = opt_pal->GetPixel(G42Color(red, green, blue));
					image_it2.Set(val);
					int error = red - palette[val].red;
					DIFFUSE_ERROR(error)
					*erow1_ptr += error;
					erow1_ptr++;
					erow2_ptr -= 5;
					error = green - palette[val].green;
					DIFFUSE_ERROR(error)
					*erow1_ptr += error;
					erow1_ptr++;
					erow2_ptr -= 5;
					error = blue - palette[val].blue;
					DIFFUSE_ERROR(error)
					*erow1_ptr += error;
					erow2_ptr -= 5;
					erow1_ptr -= 2;
					image_it2.Increment();
					if (!image_it1.Increment())
					{
						if (!image_it1.NextRow())
							break;
						image_it2.NextRow();
						int * swap = error_row1;
						error_row1 = error_row2;
						error_row2 = swap;
						memset(error_row2, 0, (row_width + 6) * sizeof(int));
						erow1_ptr = error_row1 + 3;
						erow2_ptr = error_row2;
					}
				} while (1);
				delete [] error_row1;
				delete [] error_row2;
#endif
			}
			break;
		}
		case 4:
		{
			if (data->GetNumPalette() != 0)
			{
				G42LockedID4BitIterator image_it1(data, false);
				image_it1.SetArea(left, top, right, bottom);
				G42ImageData8BitIterator image_it2(new_data, false);
				image_it2.SetArea(left, top, right, bottom);
				int32 row_width = right - left + 1;
            row_width *= 3;
				int * error_row1 = new int [row_width + 6];
				memset(error_row1, 0, (row_width + 6) * sizeof(int));
				int * error_row2 = new int [row_width + 6];
				memset(error_row2, 0, (row_width + 6) * sizeof(int));
				int * erow1_ptr = error_row1 + 3;
				int * erow2_ptr = error_row2;
				G42Color * palette = opt_pal->GetPalette();
			   G42Color * old_pal = data->GetPalette();
				int red, green, blue;
				do
				{
			   	int e1 = *erow1_ptr;
			      CLIP_ERROR(e1)
			      erow1_ptr++;
			   	int e2 = *erow1_ptr;
			      CLIP_ERROR(e2)
			      erow1_ptr++;
			   	int e3 = *erow1_ptr;
			      CLIP_ERROR(e3)
			      erow1_ptr++;
					red = old_pal[(unsigned char)image_it1].red + e1;
					green = old_pal[(unsigned char)image_it1].green + e2;
					blue = old_pal[(unsigned char)image_it1].blue + e3;
					LIMIT(red)
               LIMIT(green)
               LIMIT(blue)
					int val = opt_pal->GetPixel(G42Color(red, green, blue));
					image_it2.Set(val);
					int error = red - palette[val].red;
					DIFFUSE_ERROR(error)
					*erow1_ptr += error;
					erow1_ptr++;
					erow2_ptr -= 5;
					error = green - palette[val].green;
					DIFFUSE_ERROR(error)
					*erow1_ptr += error;
					erow1_ptr++;
					erow2_ptr -= 5;
					error = blue - palette[val].blue;
					DIFFUSE_ERROR(error)
					*erow1_ptr += error;
					erow2_ptr -= 5;
					erow1_ptr -= 2;
					image_it2.Increment();
					if (!image_it1.Increment())
					{
						if (!image_it1.NextRow())
							break;
						image_it2.NextRow();
						int * swap = error_row1;
						error_row1 = error_row2;
						error_row2 = swap;
						memset(error_row2, 0, (row_width + 6) * sizeof(int));
						erow1_ptr = error_row1 + 3;
						erow2_ptr = error_row2;
					}
				} while (1);
				delete [] error_row1;
				delete [] error_row2;
         }
         else
         {
				G42LockedID4BitIterator image_it1(data, false);
				image_it1.SetArea(left, top, right, bottom);
				G42ImageData8BitIterator image_it2(new_data, false);
				image_it2.SetArea(left, top, right, bottom);
				int32 row_width = right - left + 1;
            row_width *= 3;
				int * error_row1 = new int [row_width + 6];
				memset(error_row1, 0, (row_width + 6) * sizeof(int));
				int * error_row2 = new int [row_width + 6];
				memset(error_row2, 0, (row_width + 6) * sizeof(int));
				int * erow1_ptr = error_row1 + 3;
				int * erow2_ptr = error_row2;
				G42Color * palette = opt_pal->GetPalette();
				int red, green, blue;
				do
				{
			   	int e1 = *erow1_ptr;
					CLIP_ERROR(e1)
			      erow1_ptr++;
			   	int e2 = *erow1_ptr;
			      CLIP_ERROR(e2)
			      erow1_ptr++;
			   	int e3 = *erow1_ptr;
			      CLIP_ERROR(e3)
			      erow1_ptr++;
               int val = (int)(unsigned char)image_it1;
               val = ((val << 4) + val);
					red = val + e1;
					green = val + e2;
					blue = val + e3;
               LIMIT(red)
               LIMIT(green)
               LIMIT(blue)
					val = opt_pal->GetPixel(G42Color(red, green, blue));
					image_it2.Set(val);
					int error = red - palette[val].red;
					DIFFUSE_ERROR(error)
					*erow1_ptr += error;
					erow1_ptr++;
					erow2_ptr -= 5;
					error = green - palette[val].green;
					DIFFUSE_ERROR(error)
					*erow1_ptr += error;
					erow1_ptr++;
					erow2_ptr -= 5;
					error = blue - palette[val].blue;
					DIFFUSE_ERROR(error)
					*erow1_ptr += error;
					erow2_ptr -= 5;
					erow1_ptr -= 2;
					image_it2.Increment();
					if (!image_it1.Increment())
					{
						if (!image_it1.NextRow())
							break;
						image_it2.NextRow();
						int * swap = error_row1;
						error_row1 = error_row2;
						error_row2 = swap;
						memset(error_row2, 0, (row_width + 6) * sizeof(int));
						erow1_ptr = error_row1 + 3;
						erow2_ptr = error_row2;
					}
				} while (1);
				delete [] error_row1;
				delete [] error_row2;
			}
			break;
		}
      case 1:
      {
       	break;
      }
   }
}
void
Dither8to7Gray(G42ImageData * data, G42ImageData * new_data, int left, int top,
	int right, int bottom)
{
	G42LockedID8BitIterator image_it1(data, false);
	image_it1.SetArea(left, top, right, bottom);
	G42ImageData8BitIterator image_it2(new_data, false);
	image_it2.SetArea(left, top, right, bottom);
	do
	{
		unsigned char val = (unsigned char)image_it1;
		image_it2.Set(val >> 1);
		image_it2.Increment();
		if (!image_it1.Increment())
		{
			if (!image_it1.NextRow())
				break;
			image_it2.NextRow();
		}
	} while (1);
}
void
Match(G42ImageData * data, G42ImageData * new_data, int left, int top,
	int right, int bottom, G42OptimizedPalette * opt_pal)
{
	switch (data->GetDepth())
   {
		case 32:
   	case 24:
	   {
			G42LockedID24BitIterator image_it1(data, false);
			image_it1.SetArea(left, top, right, bottom);
			G42ImageData8BitIterator image_it2(new_data, false);
			image_it2.SetArea(left, top, right, bottom);
			do
			{
				image_it2.Set(opt_pal->GetPixel(G42Color(image_it1.GetRed(), image_it1.GetGreen(),
					image_it1.GetBlue())));
				image_it2.Increment();
				if (!image_it1.Increment())
				{
					if (!image_it1.NextRow())
						break;
					image_it2.NextRow();
				}
			} while (1);
			break;
		}
		case 8:
      {
         if (data->GetNumPalette() != 0)
         {
				G42LockedID8BitIterator image_it1(data, false);
				image_it1.SetArea(left, top, right, bottom);
				G42ImageData8BitIterator image_it2(new_data, false);
				image_it2.SetArea(left, top, right, bottom);
			   G42Color * old_pal = data->GetPalette();
				int red, green, blue;
				do
				{
					unsigned char val = (unsigned char)image_it1;
               red = old_pal[val].red;
               green = old_pal[val].green;
               blue = old_pal[val].blue;
					val = opt_pal->GetPixel(G42Color(red, green, blue));
					image_it2.Set(val);
					image_it2.Increment();
					if (!image_it1.Increment())
					{
						if (!image_it1.NextRow())
							break;
						image_it2.NextRow();
					}
				} while (1);
			}
			else
			{
				G42LockedID8BitIterator image_it1(data, false);
				image_it1.SetArea(left, top, right, bottom);
				G42ImageData8BitIterator image_it2(new_data, false);
				image_it2.SetArea(left, top, right, bottom);
				int red, green, blue;
				do
				{
               unsigned char val = (unsigned char)image_it1;
					red = val;
					green = val;
					blue = val;
					val = opt_pal->GetPixel(G42Color(red, green, blue));
					image_it2.Set(val);
					image_it2.Increment();
					if (!image_it1.Increment())
					{
						if (!image_it1.NextRow())
							break;
						image_it2.NextRow();
					}
				} while (1);
			}
			break;
		}
		case 4:
		{
			if (data->GetNumPalette() != 0)
			{
				G42LockedID4BitIterator image_it1(data, false);
				image_it1.SetArea(left, top, right, bottom);
				G42ImageData8BitIterator image_it2(new_data, false);
				image_it2.SetArea(left, top, right, bottom);
			   G42Color * old_pal = data->GetPalette();
				int red, green, blue;
				do
				{
            	unsigned char val = (unsigned char)image_it1;
					red = old_pal[val].red;
					green = old_pal[val].green;
					blue = old_pal[val].blue;
					val = opt_pal->GetPixel(G42Color(red, green, blue));
					image_it2.Set(val);
					image_it2.Increment();
					if (!image_it1.Increment())
					{
						if (!image_it1.NextRow())
							break;
						image_it2.NextRow();
					}
				} while (1);
         }
         else
         {
				G42LockedID4BitIterator image_it1(data, false);
				image_it1.SetArea(left, top, right, bottom);
				G42ImageData8BitIterator image_it2(new_data, false);
				image_it2.SetArea(left, top, right, bottom);
				int red, green, blue;
				do
				{
               unsigned char val = (unsigned char)image_it1;
               val = ((val << 4) + val);
					red = val;
					green = val;
					blue = val;
					val = opt_pal->GetPixel(G42Color(red, green, blue));
					image_it2.Set(val);
					image_it2.Increment();
					if (!image_it1.Increment())
					{
						if (!image_it1.NextRow())
							break;
						image_it2.NextRow();
					}
				} while (1);
			}
			break;
		}
      case 1:
      {
       	break;
      }
   }
}
void
G42Image::ColorReduce(G42OptimizedPalette * pal, bool match)
{
	if (!Data)
		return;
	G42ImageData * new_data = new G42ImageData(Data->GetData(), Data->GetWidth(),
		Data->GetHeight(), 8, pal->GetNumPalette());
	new_data->ReferenceCount++;
	new_data->SetFinished(true);
	new_data->SetValidStartRow(0);
	new_data->SetValidNumRows(new_data->GetHeight());
	G42Color * new_pal = pal->GetPalette();
	new_data->SetPalette(new_pal);
	::Dither(Data, new_data, 0, 0, Data->GetWidth() - 1, Data->GetHeight() - 1,
		pal, match);
	delete Data;
	Data = new_data;
	RebuildInfo();
	for (int i = 0; i < NumViewArray; i++)
	{
		ViewArray[i]->ImageChanged();
	}
	Dirty = true;
}
void
G42Image::ColorReduce(int method, G42PaletteType pal_type,
	int color_cnt)
{
	if (!Data)
		return;
	if (color_cnt <= 1 || color_cnt > 256)
   	return;
      
	G42ImageData * new_data = Data;
	if (Mask)
	{
		if (pal_type == WinColor)
		{
			delete Mask;
			Mask = 0;
		}
		else if (pal_type == Color)
		{
			color_cnt--;
		}
	}
	new_data = ::ColorReduce(Data, method, pal_type, color_cnt,
		GetEventHandler());
	if (!new_data)
		new_data = Data;
	if (new_data && Data != new_data)
	{
		delete Data;
		Data = new_data;
		RebuildInfo();
		for (int i = 0; i < NumViewArray; i++)
		{
			ViewArray[i]->ImageChanged();
		}
	}
	Dirty = true;
}
/*  TODO:  Make a template that will deal with the iterator types.
    NO:  This says reduce-to-8, but it's really for 8 bit and 4 bit targets  */
void
Reduce24ColorTo8Fast (G42ImageData * data, G42ImageData * new_data,
	int left, int top, int right, int bottom, G42OptimizedPalette * opt_pal)
{
	int32 row_count = 0L;
	G42Color * new_pal = opt_pal->GetPalette();
	G42FastLockedID24BitIterator fast_it1 (data, false);
	G42FastImageData8BitIterator fast_it2 (new_data, false);
	fast_it1.FastSetArea(left, top, right, bottom);
	fast_it2.FastSetArea(left, top, right, bottom);
	//int imageWidth	= data->GetWidth();
	//int imageHeight = data->GetHeight();
	int imageWidth	= right - left + 1;
	int imageHeight = bottom - top + 1;
	int rowBytes = (imageWidth * TrueColorPixelSize);
	// Same code as above using fast iterators
		int32 row_width = imageWidth;
		row_width *= fast_it1.GetCellSize();
		int * error_row1 = new int [row_width + 6];
		memset(error_row1, 0, (row_width + 6) * sizeof(int));
		int * error_row2 = new int [row_width + 6];
		memset(error_row2, 0, (row_width + 6) * sizeof(int));
		int * erow1_ptr = error_row1 + 3;
		int * erow2_ptr = error_row2;
		int red, green, blue;
		int pixelCount = 1;
		/*  TODO:  Rearrange this more like the match example above.. */
		do
		{
			int e1 = *erow1_ptr++;
	 		CLIP_ERROR(e1)
			int e2 = *erow1_ptr++;
			CLIP_ERROR(e2)
			int e3 = *erow1_ptr++;
			CLIP_ERROR(e3)
			G42Color color = fast_it1++;
			red 	= color.red + e1;
			green 	= color.green + e2; 
			blue 	= color.blue + e3;				
			
			LIMIT(red)
			LIMIT(green)
			LIMIT(blue)
			int val = opt_pal->GetPixel(G42Color(red, green, blue));
			fast_it2.Set(val);
			int error = red - new_pal[val].red;
			DIFFUSE_ERROR(error)
			*erow1_ptr += error;
			erow1_ptr++;
			erow2_ptr -= 5;
			error = green - new_pal[val].green;
			DIFFUSE_ERROR(error)
			*erow1_ptr += error;
			erow1_ptr++;
			erow2_ptr -= 5;
			error = blue - new_pal[val].blue;
			DIFFUSE_ERROR(error)
			*erow1_ptr += error;
			erow2_ptr -= 5;
			erow1_ptr -= 2;
			fast_it2++;
			if (pixelCount == imageWidth)
			{
				fast_it2.FastNextRow();
            row_count++;
         	if (row_count == imageHeight)
            	break;
				fast_it1.FastNextRow();
				int * swap = error_row1;
				error_row1 = error_row2;
				error_row2 = swap;
				memset(error_row2, 0, (row_width + 6) * sizeof(int));
				erow1_ptr = error_row1 + 3;
				erow2_ptr = error_row2;
				pixelCount = 0;
			}
			pixelCount++;
		} while (1);
		delete [] error_row1;
		delete [] error_row2;
}
void
Reduce8ColorTo8Fast (G42ImageData * data, G42ImageData * new_data,
	int left, int top, int right, int bottom, G42OptimizedPalette * opt_pal)
{
	int32 row_count = 0L;
	G42Color * new_pal = opt_pal->GetPalette();
	G42Color * old_pal = data->GetPalette();
	G42FastLockedID8BitIterator fast_it1 (data, false);
	G42FastImageData8BitIterator fast_it2 (new_data, false);
	fast_it1.FastSetArea(left, top, right, bottom);
	fast_it2.FastSetArea(left, top, right, bottom);
	//int imageWidth	= data->GetWidth();
	//int imageHeight = data->GetHeight();
	int imageWidth	= right - left + 1;
	int imageHeight = bottom - top + 1;
	int rowBytes = (imageWidth * TrueColorPixelSize);
	// Same code as above using fast iterators
		int32 row_width = imageWidth;
		row_width *= 3;
		int * error_row1 = new int [row_width + 6];
		memset(error_row1, 0, (row_width + 6) * sizeof(int));
		int * error_row2 = new int [row_width + 6];
		memset(error_row2, 0, (row_width + 6) * sizeof(int));
		int * erow1_ptr = error_row1 + 3;
		int * erow2_ptr = error_row2;
		int red, green, blue;
		int pixelCount = 1;
		/*  TODO:  Rearrange this more like the match example above.. */
		do
		{
			int e1 = *erow1_ptr++;
	 		CLIP_ERROR(e1)
			int e2 = *erow1_ptr++;
			CLIP_ERROR(e2)
			int e3 = *erow1_ptr++;
			CLIP_ERROR(e3)
			int color = fast_it1++;
			red 	= old_pal[color].red + e1;
			green 	= old_pal[color].green + e2;
			blue 	= old_pal[color].blue + e3;				
			
			LIMIT(red)
			LIMIT(green)
			LIMIT(blue)
			int val = opt_pal->GetPixel(G42Color(red, green, blue));
			fast_it2.Set(val);
			int error = red - new_pal[val].red;
			DIFFUSE_ERROR(error)
			*erow1_ptr += error;
			erow1_ptr++;
			erow2_ptr -= 5;
			error = green - new_pal[val].green;
			DIFFUSE_ERROR(error)
			*erow1_ptr += error;
			erow1_ptr++;
			erow2_ptr -= 5;
			error = blue - new_pal[val].blue;
			DIFFUSE_ERROR(error)
			*erow1_ptr += error;
			erow2_ptr -= 5;
			erow1_ptr -= 2;
			fast_it2++;
			if (pixelCount == imageWidth)
			{
				fast_it2.FastNextRow();
            row_count++;
         	if (row_count == imageHeight)
            	break;
				fast_it1.FastNextRow();
				int * swap = error_row1;
				error_row1 = error_row2;
				error_row2 = swap;
				memset(error_row2, 0, (row_width + 6) * sizeof(int));
				erow1_ptr = error_row1 + 3;
				erow2_ptr = error_row2;
				pixelCount = 0;
			}
			pixelCount++;
		} while (1);
		delete [] error_row1;
		delete [] error_row2;
}
// John: I don't need this anymore, but thanks anyway
void
Reduce8GrayTo8Fast (G42ImageData * data, G42ImageData * new_data,
	int left, int top, int right, int bottom, G42OptimizedPalette * opt_pal)
{
	int32 row_count = 0L;
	G42Color * new_pal = opt_pal->GetPalette();
   
	G42FastLockedID8BitIterator fast_it1 (data, false);
	G42FastImageData8BitIterator fast_it2 (new_data, false);
	fast_it1.FastSetArea(left, top, right, bottom);
	fast_it2.FastSetArea(left, top, right, bottom);
	//int imageWidth	= data->GetWidth();
	//int imageHeight = data->GetHeight();
	int imageWidth	= right - left + 1;
	int imageHeight = bottom - top + 1;
	int rowBytes = (imageWidth * TrueColorPixelSize);
	// Same code as above using fast iterators
		int32 row_width = imageWidth;
		row_width *= 3;
		int * error_row1 = new int [row_width + 6];
		memset(error_row1, 0, (row_width + 6) * sizeof(int));
		int * error_row2 = new int [row_width + 6];
		memset(error_row2, 0, (row_width + 6) * sizeof(int));
		int * erow1_ptr = error_row1 + 3;
		int * erow2_ptr = error_row2;
		int red, green, blue;
		int pixelCount = 1;
		/*  TODO:  Rearrange this more like the match example above.. */
		do
		{
			int e1 = *erow1_ptr++;
	 		CLIP_ERROR(e1)
			int e2 = *erow1_ptr++;
			CLIP_ERROR(e2)
			int e3 = *erow1_ptr++;
			CLIP_ERROR(e3)
			int color = fast_it1++;
			red 	= color + e1;
			green 	= color + e2;
			blue 	= color + e3;				
			
			LIMIT(red)
			LIMIT(green)
			LIMIT(blue)
			int val = opt_pal->GetPixel(G42Color(red, green, blue));
			fast_it2.Set(val);
			int error = red - new_pal[val].red;
			DIFFUSE_ERROR(error)
			*erow1_ptr += error;
			erow1_ptr++;
			erow2_ptr -= 5;
			error = green - new_pal[val].green;
			DIFFUSE_ERROR(error)
			*erow1_ptr += error;
			erow1_ptr++;
			erow2_ptr -= 5;
			error = blue - new_pal[val].blue;
			DIFFUSE_ERROR(error)
			*erow1_ptr += error;
			erow2_ptr -= 5;
			erow1_ptr -= 2;
			fast_it2++;
			if (pixelCount == imageWidth)
			{
				if (! fast_it1.FastNextRow())
					break;
				fast_it2.FastNextRow();
                row_count++;
				int * swap = error_row1;
				error_row1 = error_row2;
				error_row2 = swap;
				memset(error_row2, 0, (row_width + 6) * sizeof(int));
				erow1_ptr = error_row1 + 3;
				erow2_ptr = error_row2;
				pixelCount = 0;
			}
			pixelCount++;
		} while (1);
		delete [] error_row1;
		delete [] error_row2;
}

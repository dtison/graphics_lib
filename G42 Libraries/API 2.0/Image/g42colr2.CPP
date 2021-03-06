#include "g42idata.h"
#include "g42iter.h"
#include "g42opal.h"
#include "g42event.h"
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
G42ImageData *
ColorReduce1Bit(G42ImageData * data, int method, G42PaletteType pal_type,
	int num_colors, G42EventUpdateHandler * event_handler)
{
	event_handler->SetDenominator((int32)data->GetHeight());
   int32 row_count = 0L;
	if (data->GetNumPalette() != 0) // can only change to monochrome
	{
		if ((pal_type == Color) || (pal_type == Gray))
		{
			if (num_colors > 2)
				return data;
			G42ImageData * new_data = new G42ImageData(data->GetData(),
				data->GetWidth(), data->GetHeight(), 1, 0);
			new_data->ReferenceCount++;
			new_data->SetFinished(true);
			new_data->SetValidStartRow(0);
			new_data->SetValidNumRows(new_data->GetHeight());
			G42LockedID1BitIterator image_it1(data);
			G42ImageData1BitIterator image_it2(new_data);
			if (method == MethodDither)
			{
				int32 row_width = 0L;
				for (uint x = 0; x < data->GetNumTilesAcross(); x++)
				{
					G42ImageTile * tile = data->GetTile(0, x);
					row_width += tile->GetTileWidth();
				}
				int * error_row1 = new int [row_width + 9];
				memset(error_row1, 0, (row_width + 9) * sizeof(int));
				int * error_row2 = new int [row_width + 9];
				memset(error_row2, 0, (row_width + 9) * sizeof(int));
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
						memset(error_row2, 0, (row_width + 9) * sizeof(int));
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
      else
      {
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
			G42LockedID1BitIterator image_it1(data);
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
					int val = (int)(unsigned char)image_it1;
               if (val == 255)
              	val = 1;
               red = old_pal[val].red + e1;
               green = old_pal[val].green + e2;
               blue = old_pal[val].blue + e3;
					LIMIT(red)
               LIMIT(green)
               LIMIT(blue)
               val = new_opt_pal->GetPixel(G42Color(red, green, blue));
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
					int val = (int)(unsigned char)image_it1;;
               if (val > 127)
                 	val = 1;
					image_it2.Set4(new_opt_pal->GetPixel(G42Color(
					old_pal[val].red, old_pal[val].green, old_pal[val].blue)));
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
	else // can't do anything with this
	{
     	return data;
   }
	return data;
}
G42ImageData *
ColorReduce4Bit(G42ImageData * data, int method, G42PaletteType pal_type,
	int num_colors, G42EventUpdateHandler * event_handler)
{
	event_handler->SetDenominator((int32)data->GetHeight());
   int32 row_count = 0L;
	if (data->GetNumPalette() != 0)
	{
		if (pal_type == Color)
		{
			if (num_colors >= data->GetNumPalette())
				return data;
			if (num_colors > 2)
			{
				G42OptimizedPalette * new_opt_pal = new G42OptimizedPalette(data, num_colors);
				G42ImageData * new_data = new G42ImageData(data->GetData(),
					data->GetWidth(), data->GetHeight(), 4, 16);
				new_data->ReferenceCount++;
				new_data->SetFinished(true);
				new_data->SetValidStartRow(0);
				new_data->SetValidNumRows(new_data->GetHeight());
				G42Color * new_pal = new_opt_pal->GetPalette();
				new_data->SetPalette(new_pal);
				G42Color * old_pal = data->GetPalette();
				G42LockedID4BitIterator image_it1(data);
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
					int * error_row1 = new int [row_width + 9];
					memset(error_row1, 0, (row_width + 9) * sizeof(int));
					int * error_row2 = new int [row_width + 9];
					memset(error_row2, 0, (row_width + 9) * sizeof(int));
					int * erow1_ptr = error_row1 + 3;
					int * erow2_ptr = error_row2;
					int red, green, blue;
               event_handler->Start();
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
							memset(error_row2, 0, (row_width + 9) * sizeof(int));
							erow1_ptr = error_row1 + 3;
							erow2_ptr = error_row2;
						}
					} while (1);
					delete [] error_row1;
					delete [] error_row2;
					delete new_opt_pal;
               event_handler->Stop();
					return new_data;
				}
				else
				{
					event_handler->Start();
					do
					{
						int red = old_pal[(unsigned char)image_it1].red;
						int green = old_pal[(unsigned char)image_it1].green;
						int blue = old_pal[(unsigned char)image_it1].blue;
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
			G42OptimizedPalette * new_opt_pal = new G42OptimizedPalette(data, 2);
			G42ImageData * new_data = new G42ImageData(data->GetData(),
				data->GetWidth(), data->GetHeight(), 1, new_opt_pal->GetNumPalette());
			new_data->ReferenceCount++;
			new_data->SetFinished(true);
			new_data->SetValidStartRow(0);
			new_data->SetValidNumRows(new_data->GetHeight());
			G42Color * new_pal = new_opt_pal->GetPalette();
			new_data->SetPalette(new_pal);
			G42Color * old_pal = data->GetPalette();
			G42LockedID4BitIterator image_it1(data);
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
				int * error_row1 = new int [row_width + 9];
				memset(error_row1, 0, (row_width + 9) * sizeof(int));
				int * error_row2 = new int [row_width + 9];
				memset(error_row2, 0, (row_width + 9) * sizeof(int));
				int * erow1_ptr = error_row1 + 3;
				int * erow2_ptr = error_row2;
				int red, green, blue;
            event_handler->Start();
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
						memset(error_row2, 0, (row_width + 9) * sizeof(int));
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
				do
				{
					int red = old_pal[(unsigned char)image_it1].red;
					int green = old_pal[(unsigned char)image_it1].green;
					int blue = old_pal[(unsigned char)image_it1].blue;
					image_it2.Set8(new_opt_pal->GetPixel(G42Color(red, green, blue)));
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
			if (num_colors > data->GetNumPalette())
				return data;
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
				G42LockedID4BitIterator image_it1(data);
				G42ImageData4BitIterator image_it2(new_data);
				if (method == MethodDither)
				{
					int32 row_width = 0L;
					for (uint x = 0; x < data->GetNumTilesAcross(); x++)
					{
						G42ImageTile * tile = data->GetTile(0, x);
						row_width += tile->GetTileWidth();
					}
					int * error_row1 = new int [row_width + 3];
					memset(error_row1, 0, (row_width + 3) * sizeof(int));
					int * error_row2 = new int [row_width + 3];
					memset(error_row2, 0, (row_width + 3) * sizeof(int));
					int * erow1_ptr = error_row1 + 1;
					int * erow2_ptr = error_row2;
					int red, green, blue;
               event_handler->Start();
					do
					{
						red = old_pal[(unsigned char)image_it1].red;
						green = old_pal[(unsigned char)image_it1].red;
						blue = old_pal[(unsigned char)image_it1].red;
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
							memset(error_row2, 0, (row_width + 3) * sizeof(int));
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
					do
					{
						int red = old_pal[(unsigned char)image_it1].red;
						int green = old_pal[(unsigned char)image_it1].green;
						int blue = old_pal[(unsigned char)image_it1].blue;
						int val = (int)((((int32)red * (int32)306) +
							((int32)green * (int32)601) +
							((int32)blue * (int32)117) + (int32)512) >> 10);
                  LIMIT(val)
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
			G42LockedID4BitIterator image_it1(data);
			G42ImageData1BitIterator image_it2(new_data);
			if (method == MethodDither)
			{
				int32 row_width = 0L;
				for (uint x = 0; x < data->GetNumTilesAcross(); x++)
				{
					G42ImageTile * tile = data->GetTile(0, x);
					row_width += tile->GetTileWidth();
				}
				int * error_row1 = new int [row_width + 3];
				memset(error_row1, 0, (row_width + 3) * sizeof(int));
				int * error_row2 = new int [row_width + 3];
				memset(error_row2, 0, (row_width + 3) * sizeof(int));
				int * erow1_ptr = error_row1 + 1;
				int * erow2_ptr = error_row2;
				int red, green, blue;
				event_handler->Start();
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
						memset(error_row2, 0, (row_width + 3) * sizeof(int));
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
			G42LockedID4BitIterator image_it1(data);
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
				int * error_row1 = new int [row_width + 12];
				memset(error_row1, 0, (row_width + 12) * sizeof(int));
				int * error_row2 = new int [row_width + 12];
				memset(error_row2, 0, (row_width + 12) * sizeof(int));
				int * erow1_ptr = error_row1 + 3;
				int * erow2_ptr = error_row2;
				int red, green, blue;
            event_handler->Start();
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
						memset(error_row2, 0, (row_width + 12) * sizeof(int));
						erow1_ptr = error_row1 + 3;
						erow2_ptr = error_row2;
					}
				} while (1);
				delete [] error_row1;
				delete [] error_row2;
				delete new_opt_pal;
            event_handler->Stop();
				return new_data;
			}
			else
			{
				event_handler->Start();
				do
				{
					int red = old_pal[(unsigned char)image_it1].red;
					int green = old_pal[(unsigned char)image_it1].green;
					int blue = old_pal[(unsigned char)image_it1].blue;
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
	}
	else // non paletted image
	{
		if ((pal_type == Color) || (pal_type == Gray))
		{
			if (num_colors >= 16)
				return data;
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
				G42LockedID4BitIterator image_it1(data);
				G42ImageData4BitIterator image_it2(new_data);
				if (method == MethodDither)
				{
					int32 row_width = 0L;
					for (uint x = 0; x < data->GetNumTilesAcross(); x++)
					{
						G42ImageTile * tile = data->GetTile(0, x);
						row_width += tile->GetTileWidth();
					}
					int * error_row1 = new int [row_width + 3];
					memset(error_row1, 0, (row_width + 3) * sizeof(int));
					int * error_row2 = new int [row_width + 3];
					memset(error_row2, 0, (row_width + 3) * sizeof(int));
					int * erow1_ptr = error_row1 + 1;
					int * erow2_ptr = error_row2;
               event_handler->Start();
					do
					{
						int val = (int)(unsigned char)image_it1;
                  int e1 = *erow1_ptr;
                  CLIP_ERROR2(e1)
						val = ((val << 4) + val) + e1;
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
							memset(error_row2, 0, (row_width + 3) * sizeof(int));
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
				do
				{
					int val = (int)(unsigned char)image_it1;
					val = ((val << 4) + val);
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
			G42ImageData * new_data = new G42ImageData(data->GetData(),
				data->GetWidth(), data->GetHeight(), 1, 0);
			new_data->ReferenceCount++;
			new_data->SetFinished(true);
			new_data->SetValidStartRow(0);
			new_data->SetValidNumRows(new_data->GetHeight());
			G42LockedID4BitIterator image_it1(data);
			G42ImageData1BitIterator image_it2(new_data);
			if (method == MethodDither)
			{
				int32 row_width = 0L;
				for (uint x = 0; x < data->GetNumTilesAcross(); x++)
				{
					G42ImageTile * tile = data->GetTile(0, x);
					row_width += tile->GetTileWidth();
				}
				int * error_row1 = new int [row_width + 4];
				memset(error_row1, 0, (row_width + 4) * sizeof(int));
				int * error_row2 = new int [row_width + 4];
				memset(error_row2, 0, (row_width + 4) * sizeof(int));
				int * erow1_ptr = error_row1 + 1;
				int * erow2_ptr = error_row2;
            event_handler->Start();
				do
				{
					int val = (int)(unsigned char)image_it1;
					val = ((val << 4) + val) + *erow1_ptr;
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
						memset(error_row2, 0, (row_width + 4) * sizeof(int));
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
				do
				{
					int val = (int)(unsigned char)image_it1;;
						 val = ((val << 4) + val);
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
			G42LockedID4BitIterator image_it1(data);
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
				int * error_row1 = new int [row_width + 9];
				memset(error_row1, 0, (row_width + 9) * sizeof(int));
				int * error_row2 = new int [row_width + 9];
				memset(error_row2, 0, (row_width + 9) * sizeof(int));
				int * erow1_ptr = error_row1 + 3;
				int * erow2_ptr = error_row2;
				int red, green, blue;
            event_handler->Start();
				do
				{
					int val = (int)(unsigned char)image_it1;
					red = green = blue = ((val << 4) + val);
               int e1 = *erow1_ptr;
               CLIP_ERROR2(e1)
               erow1_ptr++;
               int e2 = *erow1_ptr;
               CLIP_ERROR2(e2)
               erow1_ptr++;
               int e3 = *erow1_ptr;
               CLIP_ERROR2(e3)
               erow1_ptr++;
					red += e1;
					green += e2;
					blue += e3;
               LIMIT(red)
               LIMIT(green)
               LIMIT(blue)
					val = new_opt_pal->GetPixel(G42Color(red, green, blue));
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
						memset(error_row2, 0, (row_width + 9) * sizeof(int));
						erow1_ptr = error_row1 + 3;
						erow2_ptr = error_row2;
					}
				} while (1);
				delete [] error_row1;
				delete [] error_row2;
				delete new_opt_pal;
            event_handler->Stop();
				return new_data;
			}
			else
			{
				event_handler->Start();
				do
				{
					int red, green, blue;
					int val = (int)(unsigned char)image_it1;
					red = green = blue = ((val << 4) + val);
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
	}
}

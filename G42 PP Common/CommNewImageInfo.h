#pragma once
#include "g42itype.h"
class CommNewImageInfo
{
public:
	CommNewImageInfo (void) : ImageSource (OpenDialog), FileSpec (0) {};
	enum {OpenDialog, SourceColor, Clipboard, ButtonBar} SourceType;
	short		ImageSource;	
	FSSpec 		*FileSpec;	// For Images via open dialog
	G42Color	Color;
	int			Width;
	int			Height;
	/*  Button Bar Stuff  */
	int			Rows;
	int			Columns;
	short		Depth;
};

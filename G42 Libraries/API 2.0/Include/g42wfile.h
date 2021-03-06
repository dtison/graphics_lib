// g42ifile.h - image file class header
#ifndef G42WFILE_H
#define G42WFILE_H
/*************************************************************
	File:          g42ifile.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   To read an image file and present the information
						contained within in a common format.
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
	Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "g42wimag.h" // raster file reader
#include "g42image.h" // class that deals with the information
class G42ImageWriter : public G42RasterImageWriter
{
	protected:
		char * FilePath;
		FILE * FP;
//		bool Valid;
		bool Finished;
		G42ImageType Type;
		bool Progressive;
		G42ImageInfo * Info;
		G42Image * Image;
		int Compression;
		int JpegQuality;
		int JpegStyle;
	public:
		G42ImageWriter(G42Image * image, const char * file_path = 0);
		~G42ImageWriter();
//		bool IsValid(void) const
//			{ return Valid; }
		bool IsFinished(void) const
			{ return (Valid && Finished); }
		bool WriteImage(void);
		virtual bool Output(const byte * buffer, uint size);
      virtual void OutputFinished(void);
};
#endif // G42IFILE_H

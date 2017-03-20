// g42ribas.h - raster image base class
#ifndef GIWBASE_H
#define GIWBASE_H
/*************************************************************
	File:          giwbase.h
	Copyright (c) 1996, Group 42, Inc.
	Description:   Base class for reading raster files
	Author:        Guy Eric Schalnat
	Creation Date: 26 Jan. 1996
   Modification History:
		Code   Date   Name and Description
***************************************************************/
#include "gpcomp.h"
#include "gidata.h"
#include "gidesc.h"
#include "gimdesc.h"
#include "gwbase.h"
#include "girwopt.h"
#include "giaframe.h"
class G42ImageWriterBase : public G42BaseWriter
{
public:
	G42ImageWriterBase(void);
	G42ImageWriterBase(G42Notifier & notifier,
		G42BaseOptions * options = 0);
	~G42ImageWriterBase(void);
	void SetInfo(intf type_id, const char * def_ext,
		const char * name_string, bool options = false);
	virtual void Process(G42PathObject * object, G42PathComponent * sender);
	virtual int GetDepth(void);
	enum G42IntType {Motorola, Intel, Default}; // type of integers to read
	void PutUInt32(uint32 number, G42IntType type = Default); // write a 32 bit integer
	void PutUInt16(uint16 number, G42IntType type = Default); // write a 16 bit integer
protected:
	uint32 Width;
	uint32 Height;
	uint32 RowBytes;
	uint32 Depth;
	uint32 CurrentRow;
	bool Valid;
	bool MultiMode;
	bool SkipUntilEnd;
	void PutEnd(void);
	virtual bool WriteInfo(G42ImageDescriptor * info) = 0;
	virtual bool WriteFrame(G42AnimatedImageFrame * frame);
	virtual bool WriteRow(const byte * row, const byte * mask);
	virtual uint32 NextRow(void);
	virtual bool NeedRow(void);
	virtual bool WriteEnd(void);
	virtual bool CanDoMultiple(void);
	virtual bool WriteMultiInfo(G42MultipleImageDescriptor * info);
	virtual bool WriteMultiEnd(G42MultipleImageDescriptor * info);
	void AskForOptions(G42ImageReadWriteOptions * options = 0);
	G42IntType IntegerFormat; // this variable holds what integer type to
									  // use if Default
	byte * OutputBuffer;
	uintf OutputBufferSize;
	uintf OutputBufferLeft;
	byte * NextOutput;
	G42Ptr<G42MultipleImageDescriptor> MultiDescriptor;
	G42Ptr<G42ImageDescriptor> Descriptor;
	G42Ptr<G42AnimatedImageFrame> Frame;
	G42APtr<G42Ptr<G42ImageData> > Data;
};
#endif // GIWBASE_H

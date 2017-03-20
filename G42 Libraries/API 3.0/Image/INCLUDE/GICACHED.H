#if ! defined (G42CACHED_H)
#define G42CACHED_H
#include "gtools.h"
#include "gidata.h"
#include "gidesc.h"
extern G42File * LogFile;
/*  Some useful typedef's  */
typedef G42Ptr<G42ImageDescriptor> & DescriptorRef;
typedef G42Vector<G42Ptr<G42ImageData> > & DataRef;
template <class Data, class Iterator, class DescriptorPtr, class DataArray>
Iterator
GetConstIteratorForRowBase (uint32 i, G42Lock * & lock,
	int32 & LastStripNum, DescriptorPtr InputDescriptor,
	DataArray InputData, Data *, Iterator *)
{
	/*  Get the strip  */
	int32 strip_num 	  = (int32) InputDescriptor -> GetStripNum (i);
	int32 strip_row_num = (int32) InputDescriptor -> GetRowNumInStrip (i);
	G42ImageData * image_data = InputData [strip_num];
	/*  Cast into my current type  */
	Data * data =
		dynamic_cast <Data *> (image_data);
	/*  Only if strip number is different do we need a new lock  */
	if (strip_num != LastStripNum)
	{
		if (LastStripNum != -1) {
			delete lock;
		}
		#ifdef USE_LOCKLOG
		{
			char buffer [80];
			sprintf (buffer, "Creating read lock in cached image\n");
			LogFile -> Write ((const byte *) buffer, strlen (buffer));
		}
		#endif
		lock = new G42ReadImageData (*data);
		G42ThrowIfNull (lock);
	}
	LastStripNum = strip_num;
	return ((*(data->GetConstBegin(*((G42ReadImageData *)lock)) + strip_row_num)).GetBegin());
}
template <class Data, class Iterator>
Iterator
GetConstIteratorForRow (uint32 i, G42Lock * & lock,
	int32 & LastStripNum, DescriptorRef InputDescriptor,
	DataRef InputData, Data *, Iterator *)
{
	/*  Get the strip  */
	int32 strip_num 	  = (int32) InputDescriptor -> GetStripNum (i);
	int32 strip_row_num = (int32) InputDescriptor -> GetRowNumInStrip (i);
	G42ImageData * image_data = InputData [strip_num];
	/*  Cast into my current type  */
	Data * data =
		dynamic_cast <Data *> (image_data);
	/*  Only if strip number is different do we need a new lock  */
	if (strip_num != LastStripNum)
	{
		if (LastStripNum != -1) {
			delete lock;
		}
		#ifdef USE_LOCKLOG
		{
			char buffer [80];
			sprintf (buffer, "Creating read lock in cached image\n");
			LogFile -> Write ((const byte *) buffer, strlen (buffer));
		}
		#endif
		lock = new G42ReadImageData (*data);
		G42ThrowIfNull (lock);
	}
	LastStripNum = strip_num;
	return ((*(data->GetConstBegin(*((G42ReadImageData *)lock)) + strip_row_num)).GetBegin());
}
// general class that handles both G42Ptr and non-G42Ptr cases for
// descriptor and data
template <class Data, class Iterator, class DescriptorPtr, class DataArray>
Iterator GetIteratorForRowBase (uint32 i, G42Lock * & lock,
	int32 & LastStripNum, DescriptorPtr InputDescriptor,
	DataArray InputData, Data *, Iterator *)
{
	/*  Get the strip  */	
	int32 strip_num 		= (int32) InputDescriptor -> GetStripNum (i);
	int32 strip_row_num  = (int32) InputDescriptor -> GetRowNumInStrip (i);
	G42ImageData * image_data = InputData [strip_num];
	/*  Cast into the current type  */
	Data * data = dynamic_cast <Data *> (image_data);
	/*  Only if strip number is different do we need a new lock  */
	if (strip_num != LastStripNum) 
	{
		if (LastStripNum != -1) {
			delete lock;
		}
		lock = new G42WriteImageData (*data);
		#ifdef USE_LOCKLOG
		{
			char buffer [80];
			sprintf (buffer, "Creating write lock in cached image\n");
			LogFile -> Write ((const byte *) buffer, strlen (buffer));
		}
		#endif
		G42ThrowIfNull (lock);
	}
	LastStripNum = strip_num;
	#if 0	
	Data::G42Iterator row_iterator = 
		data -> GetBegin (*((G42WriteImageData *)lock);
	/*  Advance n rows  */
	row_iterator += strip_row_num;
	Data::G42Iterator::G42Row row (*row_iterator);		
	return row.GetBegin();
	#endif
	/*  This return replaces the stuff above  */
	return ((*(data->GetBegin(*((G42WriteImageData *)lock)) + strip_row_num)).GetBegin());
}
template <class Data, class Iterator>
Iterator GetIteratorForRow (uint32 i, G42Lock * & lock,
	int32 & LastStripNum, DescriptorRef InputDescriptor,
	DataRef InputData, Data *, Iterator *)
{
	/*  Get the strip  */	
	int32 strip_num 		= (int32) InputDescriptor -> GetStripNum (i);
	int32 strip_row_num  = (int32) InputDescriptor -> GetRowNumInStrip (i);
	G42ImageData * image_data = InputData [strip_num];
	/*  Cast into the current type  */
	Data * data = dynamic_cast <Data *> (image_data);
	/*  Only if strip number is different do we need a new lock  */
	if (strip_num != LastStripNum) 
	{
		if (LastStripNum != -1) {
			delete lock;
		}
		lock = new G42WriteImageData (*data);
		#ifdef USE_LOCKLOG
		{
			char buffer [80];
			sprintf (buffer, "Creating write lock in cached image\n");
			LogFile -> Write ((const byte *) buffer, strlen (buffer));
		}
		#endif
		G42ThrowIfNull (lock);
	}
	LastStripNum = strip_num;
	#if 0	
	Data::G42Iterator row_iterator = 
		data -> GetBegin (*((G42WriteImageData *)lock);
	/*  Advance n rows  */
	row_iterator += strip_row_num;
	Data::G42Iterator::G42Row row (*row_iterator);		
	return row.GetBegin();
	#endif
	/*  This return replaces the stuff above  */
	return ((*(data->GetBegin(*((G42WriteImageData *)lock)) + strip_row_num)).GetBegin());
}
/*  A Template for navigating around thru image data.  Operator [] returns a row iterator  */
template <class DescriptorPtr, class DataPtr>
class G42CachedImageParentBase
{
public:
	G42CachedImageParentBase (DescriptorPtr desc, DataPtr image_data)
	:  Lock (0), ImageData (image_data), Descriptor (desc), LastStripNum (-1) {}
	virtual ~G42CachedImageParentBase (void)
	{
		if (Lock)
		{
			delete Lock;
			Lock = 0;
		}
	};
protected:
	G42Lock						* Lock;
	DataPtr 						ImageData;
	DescriptorPtr				Descriptor;
	int32							LastStripNum;
};
/*  For Read-locked images.  Operator [] returns a row iterator  */
template <class Data, class Iterator, class DescriptorPtr, class DataPtr>
class G42ReadCachedImageBase :
	public G42CachedImageParentBase<DescriptorPtr, DataPtr>
{
public:
	G42ReadCachedImageBase (DescriptorPtr desc, DataPtr image_data) :
		G42CachedImageParentBase<DescriptorPtr, DataPtr> (desc, image_data) {}
	Iterator operator [] (uint32 index)
	{
		return ::GetConstIteratorForRowBase
			<Data, Iterator, DescriptorPtr, DataPtr>
			(index, Lock, LastStripNum, Descriptor,
			ImageData, (Data *) 0, (Iterator *) 0);
	}		
};
/*  A Template for navigating around thru image data.  Operator [] returns a row iterator  */
class G42CachedImageParent
{
public:
	G42CachedImageParent (DescriptorRef desc, DataRef image_data)
	:  Lock (0), ImageData (image_data), Descriptor (desc), LastStripNum (-1) {}
	virtual ~G42CachedImageParent (void) 
	{
		if (Lock)
		{
			delete Lock;
			Lock = 0;
		}
	};
protected:
	G42Lock						* Lock;
	DataRef 						ImageData;
	DescriptorRef				Descriptor;
	int32							LastStripNum;
};
/*  For Read-locked images.  Operator [] returns a row iterator  */
template <class Data, class Iterator>
class G42ReadCachedImage: public G42CachedImageParent
{
public:
	G42ReadCachedImage (DescriptorRef desc, DataRef image_data) : 
		G42CachedImageParent (desc, image_data) {}
	Iterator operator [] (uint32 index)  
	{
		return ::GetConstIteratorForRow (index, Lock, LastStripNum, Descriptor, 
						ImageData, (Data *) 0, (Iterator *) 0);
	}		
};
/*  For Read-locked images w/transparency support.  GetTransIterator() 
	 returns a (safe) transparancy row iterator  */
template <class Data, class Iterator, class TransData, class TransIterator>
class G42TransReadCachedImage: public G42ReadCachedImage <Data, Iterator>
{
public:
	G42TransReadCachedImage (DescriptorRef desc, DataRef image_data) : 
		G42ReadCachedImage <Data, Iterator> (desc, image_data) {}
	TransIterator GetTransIterator (uint32 index) 
	{
		return ::GetConstIteratorForRow (index, Lock, LastStripNum, Descriptor, 
						ImageData, (TransData *) 0, (TransIterator *) 0);
	}	
};
/*  For Write-locked images.  Operator [] returns a row iterator  */
template <class Data, class Iterator>
class G42WriteCachedImage: public G42CachedImageParent
{
public:
	G42WriteCachedImage (DescriptorRef desc, DataRef image_data) : 
		G42CachedImageParent (desc, image_data) {}
	Iterator operator [] (uint32 index)  
	{
		return ::GetIteratorForRow (index, Lock, LastStripNum, Descriptor, 
						ImageData, (Data *) 0, (Iterator *) 0);
	}	
};
/*  For Write-locked images w/transparency support.  GetTransIterator() 
	 returns a (safe) transparancy row iterator  */
template <class Data, class Iterator, class TransData, class TransIterator>
class G42TransWriteCachedImage: public G42WriteCachedImage <Data, Iterator>
{
public:
	G42TransWriteCachedImage (DescriptorRef desc, DataRef image_data) : 
		G42WriteCachedImage <Data, Iterator> (desc, image_data) {}
	TransIterator GetTransIterator (uint32 index) 
	{
		return ::GetIteratorForRow (index, Lock, LastStripNum, Descriptor, 
						ImageData, (TransData *) 0, (TransIterator *) 0);
	}	
};
#define G42CachedImage G42WriteCachedImage
#endif

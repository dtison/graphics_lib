// g42image.h
#if !defined(GICROP_H)
#define GICROP_H
#include "gpcomp.h"
#include "gidesc.h"
#include "gidata.h"	// Chunks alias blocks alias rows
#include "ggeomtry.h"
class G42CropOutsideImageException : public G42Exception
{
public:
	G42CropOutsideImageException(int32 error_id = 0)
		: G42Exception(error_id) {}
};
class G42ImageCrop : public G42PathComponent
{
public:
	G42ImageCrop(const G42Rect & rect);
	virtual ~G42ImageCrop();
	virtual void Process(G42PathObject * object, G42PathComponent * sender);
protected:
	int RowBytes;
	G42Rect Rect;
	G42Ptr<G42ImageDescriptor> OldInfo;
	G42Ptr<G42ImageDescriptor> NewInfo;
	G42Ptr<G42ImageData> CurData;
	G42Ptr<G42WriteImageData> CurLock;
	void GetStrip(G42HeapManager & heap, uint32 strip_index);
	void CleanupStrips(void);
};
template <class Iterator, class OutputIterator>
OutputIterator
G42ImageCropStrip(Iterator GetBegin, Iterator GetEnd, int32 input_offset,
	OutputIterator output)
{
	for (Iterator i = GetBegin; i != GetEnd; i++)
	{
		OutputIterator::G42Row output_row(*output);
		Iterator::G42Row row(*i);
		OutputIterator::G42Row::G42Iterator out_it(output_row.GetBegin());
		Iterator::G42Row::G42Iterator j = row.GetBegin();
		j += input_offset;
		while (out_it != output_row.GetEnd())
		{
			*out_it = (*j);
			out_it++;
			j++;
		}
		output++;
	}
	return output;
}
#endif // GICROP_H

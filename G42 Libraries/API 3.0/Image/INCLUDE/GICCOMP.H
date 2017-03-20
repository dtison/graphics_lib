// giccomp.h - color comparer - for internal use only
#if !defined(GICCOMP_H)
#define GICCOMP_H
#include "gpcomp.h"
#include "gidesc.h"
#include "gidata.h"
#include "ggeomtry.h"
#ifdef MSWIN
#include <math.h>
#endif
// this is a simple comparer that is for demonstration purposes only
class G42RGBComparer
{
public:
	G42RGBComparer(double & value) : Value(value), Count(0) {}
	G42RGBComparer(const G42RGBComparer & ref) :
		Value(ref.Value), Count(ref.Count) {}
	~G42RGBComparer(void)
		{ Value /= (double)Count; }
	byte operator () (G42Color c1, G42Color c2)
		{
			Count++;
			int32 dr = (int32)((intf)(c1.Red) - (intf)(c2.Red));
			int32 dg = (int32)((intf)(c1.Green) - (intf)(c2.Green));
			int32 db = (int32)((intf)(c1.Blue) - (intf)(c2.Blue));
			int32 d2 = dr * dr + dg * dg + db * db;
			double d = sqrt((double)d2);
			Value += d;
			intf i;
			if (d >= 0.0)
			{
				i = (intf)(d * 10.0 + 0.5) + 128;
			}
			else
			{
				i = 128 - (intf)(-d * 10.0 + 0.5);
			}
			if (i > 255)
			{
				i = 255;
			}
			else if (i < 0)
			{
				i = 0;
			}
			return (byte)i;
		}
private:
	double & Value;
	uint32 Count;
};
class G42ImageColorComparer : public G42PathComponent
{
public:
	G42ImageColorComparer();
	virtual ~G42ImageColorComparer();
	virtual void Process(G42PathObject * object, G42PathComponent * sender);
	virtual void ClearPath(void);
	virtual void ReplaceSender(G42PathComponent * old_sender,
		G42PathComponent * new_sender);
	virtual void SenderDestructing(G42PathComponent * sender); // throw()
	virtual void SetSender(G42PathComponent * sender);
	double GetResult(void)
		{ return Result; }
protected:
	double Result;
	G42Ptr<G42ImageDescriptor> Info1;
	G42Vector<G42Ptr<G42ImageData> > Data1;
	G42Ptr<G42ImageDescriptor> Info2;
	G42Vector<G42Ptr<G42ImageData> > Data2;
	G42Ptr<G42ReadImageData> Lock1;
	G42Ptr<G42ReadImageData> Lock2;
	G42PathComponent * Sender2;
	G42Ptr<G42ImageDescriptor> Info3;
	intf NumEnds;
};
template <class Iterator1, class Iterator2, class Converter1, class Converter2,
	class Comparer, class OutputIterator>
OutputIterator
G42ImageComparer(Iterator1 GetBegin, Iterator1 GetEnd, Converter1 converter1,
	Iterator2 i2, Converter2 converter2,
	Comparer comparer, OutputIterator output)
{
	for (Iterator1 i = GetBegin; i != GetEnd; i++, i2++, output++)
	{
		Iterator1::G42Row row1(*i);
		Iterator2::G42Row row2(*i2);
		OutputIterator::G42Row output_row(*output);
		Iterator2::G42Row::G42Iterator j2(row2.GetBegin());
		OutputIterator::G42Row::G42Iterator out_it(output_row.GetBegin());
		for (Iterator1::G42Row::G42Iterator j = row1.GetBegin();
			j != row1.GetEnd(); j++, j2++, out_it++)
		{
			*out_it = comparer(converter1(*j), converter2(*j2));
		}
	}
	return output;
}
template <class OutputIterator, class Iterator, class Converter, class Comparer>
void
G42CompareImages2(OutputIterator output_begin, Iterator begin,
	Converter convert, uint32 start, uint32 height,
	G42ImageDescriptor * desc1, G42Vector<G42Ptr<G42ImageData> > & data1,
	Comparer comparer)
{
	uint32 start_strip = desc1->GetStripNum(start);
	uint32 stop_strip = desc1->GetStripNum(start + height - 1);
	uint32 start_row = desc1->GetRowNumInStrip(start);
	uint32 rows_left = height;
	uint32 cur_row = start;
	for (uint32 i = start_strip; i <= stop_strip; i++)
	{
		uint32 num_rows = min(rows_left, data1[i]->GetNumRows() - start_row);
		switch (desc1->GetImageType())
		{
			case G42ImageDescriptor::Color24:
			{
				G42ImageDataColor24 * data =
					dynamic_cast<G42ImageDataColor24 *>((G42ImageData *)data1[i]);
				G42ReadImageData lock(*data);
				G42ImageDataColor24::G42ConstIterator begin1 =
					data->GetConstBegin(lock);
				begin1 += start_row;
				G42ImageDataColor24::G42ConstIterator end1 = begin1;
				end1 += num_rows;
				G42ImageDataColor24::G42ConvertToG42Color convert1;
				G42ImageComparer(begin1, end1, convert1, begin, convert,
					comparer, output_begin);
				break;
			}
			case G42ImageDescriptor::Color8:
			{
				G42ImageDataColor8 * data =
					dynamic_cast<G42ImageDataColor8 *>((G42ImageData *)data1[i]);
				G42ReadImageData lock(*data);
				G42ImageDataColor8::G42ConstIterator begin1 =
					data->GetConstBegin(lock);
				begin1 += start_row;
				G42ImageDataColor8::G42ConstIterator end1 = begin1;
				end1 += num_rows;
				G42ImageDataColor8::G42ConvertToG42Color convert1(
					desc1->GetPalette());
				G42ImageComparer(begin1, end1, convert1, begin, convert,
					comparer, output_begin);
				break;
			}
		}
		rows_left -= num_rows;
		cur_row += num_rows;
		output_begin += num_rows;
		begin += num_rows;
		start_row = 0;
	}
}
template <class Iterator, class Comparer>
void
G42CompareImages1(Iterator begin, uint32 start, uint32 height,
	G42ImageDescriptor * desc1, G42Vector<G42Ptr<G42ImageData> > & data1,
	G42ImageDescriptor * desc2, G42Vector<G42Ptr<G42ImageData> > & data2,
	Comparer comparer)
{
	uint32 start_strip = desc1->GetStripNum(start);
	uint32 stop_strip = desc1->GetStripNum(start + height - 1);
	uint32 start_row = desc1->GetRowNumInStrip(start);
	uint32 rows_left = height;
	uint32 cur_row = start;
	for (uint32 i = start_strip; i <= stop_strip; i++)
	{
		uint32 num_rows = min(rows_left, data1[i]->GetNumRows() - start_row);
		switch (desc1->GetImageType())
		{
			case G42ImageDescriptor::Color24:
			{
				G42ImageDataColor24 * data =
					dynamic_cast<G42ImageDataColor24 *>((G42ImageData *)data1[i]);
				G42ReadImageData lock(*data);
				G42ImageDataColor24::G42ConstIterator begin1 =
					data->GetConstBegin(lock);
				begin1 += start_row;
				G42ImageDataColor24::G42ConvertToG42Color convert;
				G42CompareImages2(begin, begin1, convert, cur_row, num_rows,
					desc2, data2, comparer);
				break;
			}
			case G42ImageDescriptor::Color8:
			{
				G42ImageDataColor8 * data =
					dynamic_cast<G42ImageDataColor8 *>((G42ImageData *)data1[i]);
				G42ReadImageData lock(*data);
				G42ImageDataColor8::G42ConstIterator begin1 =
					data->GetConstBegin(lock);
				begin1 += start_row;
				G42ImageDataColor8::G42ConvertToG42Color convert(
					desc1->GetPalette());
				G42CompareImages2(begin, begin1, convert, cur_row, num_rows,
					desc2, data2, comparer);
				break;
			}
		}
		rows_left -= num_rows;
		cur_row += num_rows;
		begin += num_rows;
		start_row = 0;
	}
}
#endif // GICCOMP_H

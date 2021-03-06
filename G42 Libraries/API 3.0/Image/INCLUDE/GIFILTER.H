// gifilter.h - image filters base class
#if !defined(GIFILTER_H)
#define GIFILTER_H
/*  (Parent) class to handle filters and make copies of them.  */
#include "gidata.h"
#include "gpcomp.h"
#include "gidesc.h"
#include "gimdesc.h"
#include "gicached.h"	// (All filters will use this class)
#include "girgn.h"		// Temporary, until it's in G42Image..
class G42ImageFilter : public G42PathComponent
{
public:
	G42ImageFilter(void);
	virtual ~G42ImageFilter(void);
	virtual void Process(G42PathObject * object, G42PathComponent * sender);
	virtual G42ImageFilter * MakeCopy(void) const = 0;
	virtual void ClearPath(bool path_excepted = false);
	virtual void HandleBegin(G42PathBegin * begin, 
		G42PathComponent * sender);
	virtual void HandleData(G42ImageData * data, 
		G42PathComponent * sender);
	virtual void HandleDescriptor(G42ImageDescriptor * descriptor, 
		G42PathComponent * sender);
	virtual void HandleEnd(G42PathEnd * end, 
		G42PathComponent * sender);
	virtual void HandleMultiDescriptor(G42MultipleImageDescriptor * descriptor, 
		G42PathComponent * sender);
	virtual void HandleUnknown(G42PathObject * object, 
		G42PathComponent * sender);
protected:
	void 				SetInputDescriptor(G42ImageDescriptor * desc, bool make_copy = false);
	void 				SetInputData(G42ImageData * data, bool make_copy = false);
	void 				ClearDataAndDescriptor (bool clear_output = false);
	void 				CopyTransparency (void);
	void 				CopyTransparency (DescriptorRef input_descriptor, 
							DescriptorRef output_descriptor);
	void 				CreateOutputData (void);
	void 				SetOutputDescriptor(G42ImageDescriptor * desc, 
							bool make_copy = true);
	void 				GetCurrOutputData (uint32 strip_index);
	virtual void 	ProcessFrame (void) {}
	void 				SendCurrOutputData(void); 	
	void 				SendOutputData (void);		
	G42Ptr<G42ImageData>					CurrentOutputData;
	G42Ptr<G42WriteImageData>			CurrentOutputLock;
	bool										HaveFirstDescriptor;
	G42Ptr<G42ImageDescriptor>			InputDescriptor;
	G42Vector<G42Ptr<G42ImageData> >	InputData;
	G42Vector<G42Ptr<G42ImageData> >	OutputData;
	G42Ptr<G42ImageDescriptor>			OutputDescriptor;
	/*  All filters are much cleaner by using these values  */
	static G42ImageDataColor8 											* DC8;
	static G42ImageDataColor24 										* DC24;
	static G42ImageDataColor32	 										* DC32;
	static G42ImageDataGray8 											* DG8;
	static G42ImageDataGray1											* DG1;
	static G42ImageDataBase												* DT1;
	static G42ImageDataSafeBase										* DTS1;
	static G42ImageDataColor8::
				G42Iterator::G42Row::G42Iterator 					* IC8;
	static G42ImageDataColor24::
				G42Iterator::G42Row::G42Iterator 					* IC24;
	static G42ImageDataColor32::
				G42Iterator::G42Row::G42Iterator 					* IC32;
	static G42ImageDataGray8::
				G42Iterator::G42Row::G42Iterator 					* IG8;
	static G42ImageDataGray1::
				G42Iterator::G42Row::G42Iterator 					* IG1;
	static G42ImageDataBase::
				G42TransIterator::G42Row::G42Iterator 				* IT1;
	static G42ImageDataSafeBase::
				G42SafeTransIterator::G42Row::G42Iterator 		* ITS1;
	static G42ImageDataColor8::
				G42ConstIterator::G42Row::G42Iterator 				* IC8R;
	static G42ImageDataColor24::
				G42ConstIterator::G42Row::G42Iterator 				* IC24R;
	static G42ImageDataColor32::
				G42ConstIterator::G42Row::G42Iterator 				* IC32R;
	static G42ImageDataGray8::
				G42ConstIterator::G42Row::G42Iterator 				* IG8R;
	static G42ImageDataGray1::
				G42ConstIterator::G42Row::G42Iterator 				* IG1R;
	static G42ImageDataBase::
				G42ConstTransIterator::G42Row::G42Iterator 		* IT1R;
	static G42ImageDataSafeBase::
				G42ConstSafeTransIterator::G42Row::G42Iterator	* ITS1R;
	static byte																* PB8;
	static G42Color														* PC;
	static G42Color24														* PC24;
	static G42Color32														* PC32;
	static G42ConstColor24												* PC24R;
	static G42ConstColor32												* PC32R;
};
/* Copy a row of pixels  */
template <class SourceIterator, class DestIterator,
	class SourcePixelConv, class DestPixelConv> 
void CopyPixels (SourceIterator source_ptr, DestIterator dest_ptr, 
	SourcePixelConv source_converter, DestPixelConv dest_converter, uint32 width)
{
	for (uint32 i = 0; i < width; i++) 
	{
		*dest_ptr++ = dest_converter (source_converter (*source_ptr++));
	}
}
/*  Plain C++ function for templates to use */
G42Color Interpolate4Colors (G42Color * colors, double row_delta, 
	double col_delta,  bool single_channel = false);
#endif // GIFILTER_H

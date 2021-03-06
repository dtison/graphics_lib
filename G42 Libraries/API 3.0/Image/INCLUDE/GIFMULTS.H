#if ! defined (GIFMULTS_H)
#define GIFMULTS_H
#include "gifilter.h"
class G42ImageFilterMultiSender : public G42ImageFilter
{
public:
							G42ImageFilterMultiSender (void) : 
								HaveFirstSender (0), SecondSender (0) {}
	virtual 				~G42ImageFilterMultiSender (void) {}
protected:
	void			 		CleanupStrips (void);
	virtual void 		ClearPath(bool path_excepted = false);
	virtual void 		HandleData (G42ImageData * data, 
								G42PathComponent * sender);
	virtual void 		HandleDescriptor (G42ImageDescriptor * descriptor, 
								G42PathComponent * sender);
	virtual void 		ReplaceSender(G42PathComponent * old_sender,
								G42PathComponent * new_sender);
	virtual void 		SenderDestructing(G42PathComponent * sender);
	virtual void 		SetSender(G42PathComponent * sender);
	/*  Variables  */
	G42Vector			<G42Ptr<G42ImageData> > SecondImageData;
	G42Ptr				<G42ImageDescriptor> SecondImageDescriptor;
	bool										HaveFirstSender;	// For filters with 2 senders
	G42PathComponent 						* SecondSender;	// For filters with 2 senders
};
#endif

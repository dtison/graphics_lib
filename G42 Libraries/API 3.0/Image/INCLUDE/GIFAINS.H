// gifains.h - animated image frame inserter
#if !defined(GIFAINS_H)
#define GIFAINS_H
#include "gifilter.h"
#include "gimdesc.h"
class G42ImageAnimatedInserter : public G42ImageFilter
{
public:
	// pass in the frame or frames you want split out
	enum G42LinkOrder { FirstLinkIsInsertedImage, FirstLinkIsNotInsertedImage };
	G42ImageAnimatedInserter(uint32 insert_before_frame_index,
		G42LinkOrder order = FirstLinkIsNotInsertedImage);
	virtual ~G42ImageAnimatedInserter(void);
	// defaults not inserted image first, inserted image second, both must be
	// present
	void MakeNextLinkInsertedImage(void)
		{ NextSenderIsInsertedImage = true; }
	void MakeNextLinkNotInsertedImage(void)
		{ NextSenderIsInsertedImage = false; }
	virtual G42ImageFilter * MakeCopy(void) const;
	virtual void ClearPath(bool path_excepted = false);
	virtual void HandleBegin(G42PathBegin * begin, G42PathComponent * sender);
	virtual void HandleDescriptor(G42ImageDescriptor * descriptor,
		G42PathComponent * sender);
	virtual void HandleData(G42ImageData * data, G42PathComponent * sender);
	virtual void HandleEnd(G42PathEnd * end, G42PathComponent * sender);
	virtual void HandleUnknown(G42PathObject * object,
		G42PathComponent * sender);
	virtual void SenderDestructing(G42PathComponent * sender); // throw()
	virtual void ReplaceSender(G42PathComponent * old_sender,
		G42PathComponent * new_sender);
	virtual void SetSender(G42PathComponent * sender);
private:
	bool						BeginSent;
	G42PathComponent *	InsertSender;
	bool						NextSenderIsInsertedImage;
	G42Vector <G42PathObject *>	TempSenderObjects;
	uint32	NumTempSenderObjects;
	G42Vector <G42PathObject *>	TempInserterObjects;
	uint32	NumTempInserterObjects;
	G42Vector <G42APtr <G42PathObject *> >	SenderObjects;
	uint32	NumSenderObjects;
	G42Vector <G42APtr <G42PathObject *> >	InserterObjects;
	uint32	NumInserterObjects;
	G42Vector <uint32> NumSenderObjectsArray;
	G42Vector <uint32> NumInserterObjectsArray;
	bool SenderFirstFrame;
	bool InserterFirstFrame;
	enum G42Mode { SendBeforeInsert, SendInsert, SendAfterInsert };
	G42Mode Mode;
	bool HadInserterEnd;
	bool HadSenderEnd;
	uint32 InsertFrame;
	uint32 SenderFrame;
	G42Ptr <G42MultipleImageDescriptor> OldMultiDescriptor;
	G42Ptr <G42MultipleImageDescriptor> InsertMultiDescriptor;
	G42Ptr <G42MultipleImageDescriptor> NewMultiDescriptor;
};
#endif // GIFAINS_H

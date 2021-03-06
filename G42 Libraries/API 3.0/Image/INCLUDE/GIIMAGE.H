// giimage.h - G42Image
#if !defined(GIIMAGE_H)
#define GIIMAGE_H
#include "gpcomp.h"
#include "gidesc.h"
#include "gidata.h"
#include "giaframe.h"
#include "giadesc.h"
#include "girwopt.h"
class G42Image : public G42PathComponent
{
public:
	G42Image();
	G42Image(G42Image & ref);
	G42Image & operator = (G42Image & ref);
	virtual ~G42Image();
	virtual void Process(G42PathObject * object, G42PathComponent * sender);
	virtual void BeginProcessing();
	virtual void ClearPath(bool path_excepted = false);
	void Copy(G42Image & ref);
	void Clear();
	void ClearPathFrames(void);
	void ClearRegion (void)
		{ Descriptor[0]->ClearRegion(); }
	bool IsEmpty(void)
		{ return !NumFrames || !Descriptor[0]; }
	const G42ImageDescriptor & GetInfo(uint32 index = 0)
		{ return *Descriptor[index]; }
	const bool HasInfo(uint32 index = 0)
		{ return NumFrames && Descriptor[index]; }
	const G42MultipleImageDescriptor & GetMultiInfo(void)
		{ return *MultiDescriptor; }
	const bool HasMultiInfo(void)
		{ return MultiDescriptor; }
	const G42ImageReadWriteOptions & GetImageOptions(void)
		{ return *ImageOptions; }
	const bool HasImageOptions(void)
		{ return ImageOptions; }
	bool HasFrame(uint32 index = 0)
		{ return Frame[index]; }
	G42AnimatedImageFrame & GetFrame(uint32 index = 0)
		{ return *Frame[index]; }
	G42ImageData ** GetDataArray(uint32 index = 0)
		{ return Data[index]; }
	bool HasHeap(void) const
		{ return NumFrames && Descriptor[0]; }
	G42HeapManager & GetHeap(void) const
		{ return Descriptor[0]->GetHeap(); }
	bool IsModified(void)
		{ return Modified; }
	void SetModified(bool mod = true)
		{ Modified = mod; }
	void SetPathFrames(const uint32 * indexes, uint32 num_indexes);
	void SetPathFrames(const G42Vector <uint32> & indexes, uint32 num_indexes);
	void SetRegion (const G42Region & region)
		{Descriptor[0] -> SetRegion (region);}
protected:
	G42Vector<G42ImageDescriptor *> Descriptor;
	G42Vector<G42ImageData **> Data;
	G42Vector<G42AnimatedImageFrame *> Frame;
	G42MultipleImageDescriptor * MultiDescriptor;
	G42ImageReadWriteOptions * ImageOptions;
	uint32 NumFrames;
	bool Modified;
	void PreparePath(void);
	void ClearSavedPath(void);
	void RestoreSavedPath(void);
	void UpdateMultiImage(void);
	G42Vector<G42ImageDescriptor *> SavedDescriptor;
	G42Vector<G42ImageData **> SavedData;
	G42Vector<G42AnimatedImageFrame *> SavedFrame;
	G42MultipleImageDescriptor * SavedMultiDescriptor;
	G42ImageReadWriteOptions * SavedImageOptions;
	uint32 NumSavedFrames;
	G42Vector <uint32> PathFrames;
	uint32 NumPathFrames;
	int32 CurPathFrame;
	bool NeedMultiUpdate;
};
#endif // GIIMAGE_H

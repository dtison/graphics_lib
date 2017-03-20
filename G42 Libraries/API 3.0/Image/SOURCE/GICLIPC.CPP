#include "giclipc.h"
#include "gmacfl.h"
#include "gtmpmem.h"
//PICTClipboardType = 'PICT'
/*  Put the image into clipboard  */
void
G42ClipboardCopy::ProcessFrame (void)
{
	#ifdef MACOS
	/*  Make Pict from image data */
	G42Ptr <G42ImageDescriptor> nt_descriptor = 
		new G42ImageDescriptor (*InputDescriptor);
	G42ThrowIfNull (nt_descriptor);
	nt_descriptor -> ClearTransparency();
	PicHandle pict;
	pict = G42MacFile::PICTFromImageData (nt_descriptor, InputData, false); 
	bool error = false;
	if (pict)
	{
		/*  Tell OS to put it up  */
		::ZeroScrap();
		::HLock ((Handle) pict);
		Size handle_size = ::GetHandleSize ((Handle) pict);
		::CompactMem (handle_size);
		long err = ::PutScrap (handle_size, PICTClipboardType, *pict);
		if (err == memFullErr) {
			error = true;
		}
		::HUnlock ((Handle) pict);
		::KillPicture (pict);
	}
	else {
		error = true;
	}
	/*  HOW TO HANDLE THIS ERROR  */
	#if 0
	if (error)
	{
		Str255 msg;
		::GetIndString (msg, STRx_Miscellaneous, str_NomemoryClipboard);
		G42MacFile::Msg (msg);
	}		
	#endif	
	
	/*  Put up Region  */
	if (InputDescriptor -> HasRegion())
	{
		G42Ptr <G42Region> region = new G42Region (InputDescriptor -> GetRegion());
		G42ThrowIfNull (region);
		long err = ::PutScrap (sizeof (G42Region), RegionClipboardType, 
			(Ptr) (G42Region *) region);
		if (err == memFullErr) {
			error = true;
		}
	}
	/*  Put up Tranparency  */
	if (InputDescriptor -> IsTransparent())
	{
		int32 width  = InputDescriptor -> GetWidth();
		int32 height = InputDescriptor -> GetHeight();
		int32 row_bytes = ((((width + 7) >> 3) + 3) & ~3);
		G42TempMem <char> buffer ((row_bytes * height), true);
		G42ReadCachedImage <G42ImageDataBase, 
			G42ImageDataBase::G42ConstTransIterator::G42Row::G42Iterator> 
			trans_image (InputDescriptor, InputData);
		for (int32 i = 0; i < height; i++)
		{
			G42ImageDataBase::G42ConstTransIterator::G42Row::G42Iterator 
				trans_ptr = trans_image [i];
			for (int32 j = 0; j < width; j++)
			{
				if ((bool) trans_ptr++)
				{
					*(buffer + row_bytes * i + (j >> 3)) |= 
						(1 << (7 - (j & 7)));
				}
			}
		}
		long err = ::PutScrap (sizeof (G42Region), TransClipboardType, buffer);
		if (err == memFullErr) {
			error = true;
		}
		
	}
	/*  Clear everything  */
	InputData.Clear(); 
	InputDescriptor.Clear();
	#endif
}
void
G42ClipboardCopy::HandleEnd(G42PathEnd * end, G42PathComponent *)
{
	ProcessFrame();
#if 0
	Receiver->Process(end, this);
#endif
	/*  Reset sender and receiver  */
#if 0
	Receiver = 0;
#endif
	Sender = 0;
	ClearPath();
}
G42ImageFilter *
G42ClipboardCopy::MakeCopy (void) const
{
	return new G42ClipboardCopy;
}

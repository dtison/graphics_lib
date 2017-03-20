// g42iinfo.h - basic image information.
#if !defined(G42IINFO_H)
#define G42IINFO_H
#include "g42mptr.h"
#include "g42array.h"
#include "g42com.h"
#include <time.h>
class G42ImageInfo
{
	public:
		uint32 Width;
		uint32 Height;
      uint32 FileSize;
		uint PixelDepth;
		enum G42ColorType
			{ ColorTypeTrueColor, ColorTypeGrayscale, ColorTypePalette };
		bool HasAlphaChannel;
		bool IsTransparent;
      bool HasBackground;
		bool Progressive;
      int Compressed; // 0 if none, 1 or more if compressed (see g42itype.h)
		G42Color TransparentColor;
		G42Color BackgroundColor;
		G42ImageType ImageType;
		G42ColorType ColorType;
		G42MultiAPtr<G42Color> ImagePalette;
		uint NumPalette;
		G42Array <G42MultiPtr <G42Comment> > Comments;
		uint NumComments;
		time_t ModifiedTime;
		byte TargaDepth; // 16, 24, 32 - used to save original targa file depth
		byte JpegQuality;
		bool JpegSubSampling; // true if jpeg image is "photograph" mode 
		G42ImageInfo();
		~G42ImageInfo();
		G42ImageInfo(const G42ImageInfo & info);
      G42ImageInfo & operator = (const G42ImageInfo & info);
};
#endif // G42IINFO_H

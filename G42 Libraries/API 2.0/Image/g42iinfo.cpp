
#include <g42itype.h>
#include <g42com.h>
#include <g42iinfo.h>
G42ImageInfo::G42ImageInfo() :
	NumPalette(0), NumComments(0), Width(0),
	Height(0), PixelDepth(0), HasAlphaChannel(false),
	IsTransparent(false), Progressive(false),
	HasBackground(false), ImageType(Unknown),
	ColorType(ColorTypeTrueColor), FileSize(0),
	Compressed(0), ModifiedTime(0),
	TargaDepth(24), JpegQuality(90), JpegSubSampling(true),
	TransparentColor(0, 0, 0), BackgroundColor(0, 0, 0)
{
}
G42ImageInfo::~G42ImageInfo()
{
}
G42ImageInfo::G42ImageInfo(const G42ImageInfo & info) :
	NumPalette(info.NumPalette), NumComments(info.NumComments),
	Width(info.Width), Height(info.Height), PixelDepth(info.PixelDepth),
	HasAlphaChannel(info.HasAlphaChannel),
	IsTransparent(info.IsTransparent), Progressive(info.Progressive),
	HasBackground(info.HasBackground), ImageType(info.ImageType),
	ColorType(info.ColorType),
	FileSize(info.FileSize), Compressed(info.Compressed),
	ModifiedTime(info.ModifiedTime), TargaDepth(info.TargaDepth),
	JpegQuality(info.JpegQuality), JpegSubSampling(info.JpegSubSampling),
	TransparentColor(info.TransparentColor),
	BackgroundColor(info.BackgroundColor)
{
	if (NumPalette)
	{
		G42Color * pal = new G42Color [256];
		if (pal)
		{
			const G42Color * old_pal = info.ImagePalette;
			ImagePalette.Set(pal);
			for (uint i = 0; i < NumPalette; i++)
			{
				pal[i] = old_pal[i];
			}
		}
	}
	for (int i = 0; i < NumComments; i++)
	{
		Comments[i] = info.Comments[i];
	}
}
G42ImageInfo & G42ImageInfo::operator = (const G42ImageInfo & info)
{
	if (&info != this)
	{
		Width = info.Width;
		Height = info.Height;
		PixelDepth = info.PixelDepth;
		HasAlphaChannel = info.HasAlphaChannel;
		IsTransparent = info.IsTransparent;
		Progressive = info.Progressive;
		HasBackground = info.HasBackground;
		ImageType = info.ImageType;
		ColorType = info.ColorType;
		TransparentColor = info.TransparentColor;
		BackgroundColor = info.BackgroundColor;
		if (NumPalette)
		{
			G42Color * pal = new G42Color [256];
			if (pal)
			{
				const G42Color * old_pal = info.ImagePalette;
				ImagePalette.Set(pal);
				for (uint i = 0; i < NumPalette; i++)
				{
					pal[i] = old_pal[i];
				}
			}
		}
		NumPalette = info.NumPalette;
		NumComments = info.NumComments;
		FileSize = info.FileSize;
		Compressed = info.Compressed;
		ModifiedTime = info.ModifiedTime;
		TargaDepth = info.TargaDepth;
		JpegQuality = info.JpegQuality;
		JpegSubSampling = info.JpegSubSampling;
		for (int i = 0; i < NumComments; i++)
		{
			Comments[i] = info.Comments[i];
		}
	}
   return *this;
}

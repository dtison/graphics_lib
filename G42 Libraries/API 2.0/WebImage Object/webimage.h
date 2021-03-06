// webimage.h - webimage include file
#if !defined(WEBIMAGE_H)
#define WEBIMAGE_H
#include "g42text.h"
#include "g42type.h"
#include "g42itype.h"
#include <g42image.h>
#include <g42imap.h>
#include <g42sview.h>
class G42WebImage
{
protected:
	bool							TextMode;
	bool							MapMode;
	G42ImageMap 			* 	ImageMap;
	G42Image 				*	Image;
   G42Image					*	UndoImage;
	G42Image					*	PreviewImage;
	G42Image					*	SelectedImage;
	G42Image					*	UndoSelectedImage;
	G42SingleImageViewer *	View;
	G42Text					*	TextObject;
	int							PreviewNoise;
	int							PreviewSharp;
   int							PreviewAngle;
	int							PreviewBright;
	int							PreviewGamma;
	int							PreviewContrast;
   int							PreviewChannel;
	G42MethodType				PreviewMethod;
	G42PaletteType          PreviewPalType;
	int							PreviewColorCnt;
	int							PreviewWidth;
	int							PreviewHeight;
	int							PreviewRed;
	int							PreviewGreen;
	int							PreviewBlue;
	int							PreviewTopRed;
	int							PreviewTopGreen;
	int							PreviewTopBlue;
	int							PreviewBottomRed;
	int							PreviewBottomGreen;
	int							PreviewBottomBlue;
   G42ButtonType				PreviewButtonType;
	bool							PreviewHasColor;
	bool							PreviewTransColorValid;
	G42Color						PreviewTransColor;
	int							InUndo;
public:
					G42WebImage(void);
					~G42WebImage(void);
	G42Image *	GetImage(void)
						{return Image;}
	G42Image *	GetSelectedImage(void)
						{return SelectedImage;}
	G42Image *	GetCurrentImage(void)
						{if (SelectedImage) return SelectedImage; return Image;}
	bool			GetTextMode(void)
   					{return TextMode;}
	bool			GetMapMode(void)
						{return MapMode;}
	G42SingleImageViewer * GetView(void)
						{return View;}
	bool			GetMapChanged(void);
	bool			GetImageChanged(void);
	void			SetTextMode(bool text_mode);
	void			SetJustification (G42Text::G42Justification justification);
	void			SetDirection (G42Text::G42Direction direction);
	void			SetColor (G42Color color);
	void			SetAntiAlias (bool anti_alias);
	void			SetFontName (const char * typeface_name);
	void			SetPointSize (int point_size);
	void			SetBold (bool bold);
	void			SetItalic (int italic);
	void			SetUnderline (int underline);
	void			SetFontAttributes (const char * typeface_name, int point_size,
						bool bold, bool italic, bool underline, G42Color color);
	G42Text	*	GetG42Text (void)
						{return TextObject;}
	void			SetMapMode(bool map_mode);
	void			NewMap(void);
	bool			OpenMap(const char * file_name);
	bool			OpenMap(const char * buffer, uint size);
	bool			SaveMap(const char * file_name, G42ImageMap::G42MapType type);
	bool			SaveMap(char * buffer, uint size,
						G42ImageMap::G42MapType type, bool cr_lf = false);
	G42ImageMap * GetImageMap(void)
						{return ImageMap;}
	void			SetImage(G42Image * image);
	void			SetSelectedImage(G42Image * image);
	void			SetView(G42SingleImageViewer * view);
	void			DrawImageMapElement(uint index); // for delete
   void			NewImage(G42Color top_color, G42Color middle_color,
   					G42Color bottom_color, uint32 width, uint32 height, int size,
                  int rows, int columns, bool final = true);
	void			NewImage(G42Image * image_in, bool final = true);
   void			CreateNewImage(G42Color top_color, G42Color middle_color,
   					G42Color bottom_color, uint32 width, uint32 height, int size,
                  int rows, int columns);
   void			PasteNewImage(G42Image * image);
	void			ColorAdjust(int bright, int gamma, int contrast, int channel,
						bool final = true);
	void			ColorReduction(int method, G42PaletteType pal_type,
						int color_cnt,	bool final = true);
	void			Border(G42Color color, int width, bool final = true);
	void			Buttonize(G42Color top_color, G42Color bottom_color,
						G42ButtonType button_type, int width, bool has_color,
                  bool final = true);
	void			ResizeImage(int current_width, int current_height,
						bool final = true);
	void			Invert(void);
	void			SwapRB(void);
	void			Equalize(void);
	void			Emboss(int angle, bool final = true);
	void			Sharpness(int sharp, bool final = true);
	void			RemoveNoise(int noise, bool final = true);
   void			MirrorX(void);
	void			MirrorY(void);
	void			Rotate(int angle);
	void			Crop(void);
	void			SelectAll(void);
   void			EnsureColorCount(void);
   void			DrawColorMask(byte * mask, int width, int height, G42Color color,
				   	int x, int y);
   void			DrawColorMask8(byte * mask, int width, int height, G42Color color,
				   	int x, int y);
	void			SetTransparentMode(bool mode);
	bool			GetTransparentMode(void);
	void			ClearTransparency(bool final = true);
	void			SetTransparency(G42Color color, bool final = true);
	G42Color		GetColorAtPoint(uint32 x, uint32 y);
	void 			Refresh(void);
	void			ZoomIn(void);
	void			ZoomOut(void);
	void			Unzoom(void);
	void			FitWidth(void);
	void			FitAll(void);
	void			Undo(void); // reverts back to the last ResetUndo()
	bool			CanUndo(void); // true if there is an undo image
	void			ResetUndo(void); // saves off the current image for undo
	void			ResetPreview(void); // sets up or resets the preview image
	void			AcceptPreview(void); // makes the preview image the real image
	void			CancelPreview(void); // reverts back to the original image
	void			DeleteUndoPreview(void); // clears before deleting swap
   bool			HasPreview(void) const
   	{return PreviewImage ? true : false;}
};
#endif //WEBIMAGE_H

#if !defined(G42TEXT_H)
#define G42TEXT_H
#include "g42itype.h"
#ifdef MACOS
#include <Fonts.h>
#endif
class G42Text
{
public:
	enum	G42Direction {Horizontal = 0, VerticalDown = -1, VerticalUp = 1};
	enum	G42Justification {Left = 0, Center = 1, Right = 2};
	G42Text (const char * typeface_name = 0,
		int point_size = 10, bool bold = false, bool italic = false,
		bool underline = false,
		G42Color color = G42Color(0, 0, 0), G42Direction direction = Horizontal,
		G42Justification justification = Left, bool anti_alias = true);
	~G42Text (void);
	G42Direction GetDirection (void)
		{return Direction;}
	G42Justification	GetJustification (void)
		{return Justification;}
	int	GetPointSize (void)
		{return PointSize;}
	const char * GetTypefaceName (void)
		{return TypefaceName;}
	const G42Color & GetColor (void)
		{return Color;}
	bool	GetAntiAlias (void)
		{return AntiAlias;}
	bool	GetBold (void)
		{return Bold;}
	bool	GetItalic (void)
		{return Italic;}
	bool	GetUnderline (void)
		{return Underline;}
	int	GetFontHeight(void)
		{return FontHeight;}
	int	GetFontAscent(void)
		{return FontAscent;}
	int	GetFontDescent(void)
		{return FontDescent;}
	int	GetFontLineSpacing(void)
		{return FontLineSpacing;}
	void	SetJustification (G42Justification justification)
		{Justification = justification;}
	void	SetDirection (G42Direction direction);
	void	SetColor (G42Color & color)
		{Color = color;}
	void	SetAntiAlias (bool anti_alias)
		{AntiAlias = anti_alias;}
	void	SetFontName (const char * typeface_name);
	void	SetPointSize (int point_size);
	void	SetBold (bool bold);
	void	SetItalic (bool italic);
	void	SetUnderline(bool underline);
	void	SetFontAttributes (const char * typeface_name, int point_size,
		bool bold, bool italic, bool underline, G42Color color);
#ifdef MSWIN
// Trying something new - JRP
   void  SetFont(LOGFONT * log_font, int point_size, G42Color color);
	HFONT GetFont(void)
		{return Font;}
#endif
	bool CheckFont(void);
   
private:
	bool		AntiAlias;
	char   	* TypefaceName;  // Name of this font
	bool		Italic;
	bool		Underline;
	bool		Bold;
	int		PointSize;
	int		FontHeight; // height of the font in pixels, used for drawing
	int		FontAscent; // ascent of the font in pixels, used for drawing
	int		FontDescent; // descent of the font in pixels, used for drawing
	int		FontLineSpacing; // spacing of the font in pixels, used for drawing
	G42Direction		Direction;
	G42Justification		Justification;
	G42Color	Color;
#ifdef MSWIN
	LOGFONT *	LogFont;
	HFONT			Font;
#endif
#ifdef MACOS
	FontInfo		FInfo;	
#endif
	void		CreateFont (void);
	int		CalcPointsToPixels (void);
};
#endif															//G42Text_H

//  giobtext.h - G42TextObject
#if ! defined (GIOBTEXT_H)
#define GIOBTEXT_H
/*-----------------------------------------------------------------------------
	file:			giobtext.h
	author:			Guy Eric Schalnat
	date:			04/28/97
	revision:		3.0
	Onscreen graphics object:  Text
 ----------------------------------------------------------------------------*/
/*
	Note:  this is really a few classes rolled up into one.  I almost made
	it two classes, but to do so correctly would take into account a large
	and ever growing list of uses, the least of which is being a vector
	path object.  Also, the other graph objects should be split similiarly.
	As this is a design job that requires time, thought, and discussion
	(none of which are available to me at this time), I keep it all in one
	big class.  However, I left some extra functionality which will be used
	when the class does get split up.
	Much of the lower level of text code was from java.
*/
#include "giobquad.h"
#include "gtools.h"
class G42TextObject : public G42QuadObject
{
public:
								G42TextObject (G42AreaController *acontroller,
								G42MouseKeyController *mcontroller);
	virtual 					~G42TextObject (void);
	virtual G42Region 	* CreateObjectRegion (void);
	virtual void			KeyDown (G42KeyDown *message);
	enum G42AlignHoriz { AlignLeft, AlignCenterH, AlignRight };
	enum G42AlignVert { AlignTop, AlignCenterV, AlignBottom };
	enum G42LineSpacing {LineSpacingProportional, LineSpacingIncremental,
		LineSpacingAbsolute};
	void AddText(const char * text, intf num);
	void NextParagraph();
	void SetStart(const G42Point & start);
	void SetFontName(const char * name);
	void SetFontSize(intf size);
	void SetBold(bool bold);
	void SetItalic(bool bold);
	void SetColor(const G42Color & color);
	intf GetWrapWidth()
		{ return WrapWidth; }
	void SetWrapWidth(intf width);
	bool IsWrapped (void) const
		{ return (WrapWidth > 0); }
	void SetNoWrap();
	void SetUnderline(bool state);
	void SetWrapHeight(intf pixels);
	void SetLineSpacing(intf pixels, G42LineSpacing mode);
	void SetParagraphSpacing(intf pixels);
	void SetTabSpacing(intf pixels);
	void SetFirstIndent(intf pixels);
	void SetLeftIndent(intf pixels);
	void SetRightIndent(intf pixels);
	void SetHorizontalAlignment(G42AlignHoriz state);
	void SetVerticalAlignment(G42AlignVert state);
	void AddBullet();
	void AddBullet(intf bullet);
	void AddIndentBullet();
	void AddIndentBullet(intf bullet);
#if 0 // new stuff that needs to be added
	void SetInsertionPoint(???);
	void SetSelectionRange(???, ???);
	void HandleMovementKeys(...);
#endif
protected:
	// java
	struct G42Attributes
	{
		const char * Name; // font name
		bool Bold;
		bool Italic;
		bool Underline;
		G42AlignHoriz HorizontalAlignment;
		G42AlignVert VerticalAlignment;
		intf Size; // font size
		G42Color Color; // font color
		intf TabSpacing; // pixels for each tab
		intf LeftIndent;
		intf RightIndent;
		intf FirstIndent;
		intf ParagraphSpacing;
		intf LineSpacing;
		G42LineSpacing LineSpacingMode;
		intf BulletCharacter;
		bool BulletHanging;
		bool BulletDingbats;
		bool NeedFont; // recalculates font if true
		bool NeedColor; // gets color if true
	};
	struct G42Line
	{
		intf Width;
		intf Ascent;
		intf Descent;
		intf SectionStart;
		intf SectionEnd;
	};
	struct G42Section
	{
		bool NewParagraph;
		intf TextStart;
		intf TextNum;
		intf X;
		intf Y;
		intf Attribute;
		bool Bullet;
		intf BulletChar;
		G42Color Color;
	};
	class G42BaseAttribute
	{
	public:
		G42BaseAttribute () : Position(0) {}
		G42BaseAttribute (uintf position) : Position(position) {}
		virtual ~G42BaseAttribute() {}
		virtual G42BaseAttribute * MakeCopy() = 0;
		intf GetPosition() const
			{ return Position; }
		virtual void ChangeText(G42Attributes & attributes) = 0;
	private:
		intf Position;
	};
	class G42FontNameAttribute : public G42BaseAttribute
	{
	public:
		G42FontNameAttribute (const char * name, uintf position) : Name(0),
			G42BaseAttribute(position)
		{
			if (name && *name)
			{
				Name = new char [strlen(name) + 1];
				G42ThrowIfNull(Name);
				strcpy(Name, name);
			}
		}
		virtual ~G42FontNameAttribute() {}
		virtual G42BaseAttribute * MakeCopy()
			{ return new G42FontNameAttribute(Name, GetPosition()); }
		const char * GetName() const
			{ return Name; }
		virtual void ChangeText(G42Attributes & attributes)
			{ attributes.Name = Name; attributes.NeedFont = true; }
	private:
		G42APtr <char> Name;
	};
	class G42FontSizeAttribute : public G42BaseAttribute
	{
	public:
		G42FontSizeAttribute (intf size, uintf position) : Size(size),
			G42BaseAttribute(position) {}
		virtual ~G42FontSizeAttribute() {}
		virtual G42BaseAttribute * MakeCopy()
			{ return new G42FontSizeAttribute(Size, GetPosition()); }
		intf GetSize() const
			{ return Size; }
		virtual void ChangeText(G42Attributes & attributes)
			{ attributes.Size = Size; attributes.NeedFont = true; }
	private:
		intf Size;
	};
	class G42BoldAttribute : public G42BaseAttribute
	{
	public:
		G42BoldAttribute (bool state, uintf position) : Bold(state),
			G42BaseAttribute(position) {}
		virtual ~G42BoldAttribute() {}
		virtual G42BaseAttribute * MakeCopy()
			{ return new G42BoldAttribute(Bold, GetPosition()); }
		bool IsBold() const
			{ return Bold; }
		virtual void ChangeText(G42Attributes & attributes)
			{ attributes.Bold = Bold; attributes.NeedFont = true; }
	private:
		bool Bold;
	};
	class G42ItalicAttribute : public G42BaseAttribute
	{
	public:
		G42ItalicAttribute (bool state, uintf position) : Italic(state),
			G42BaseAttribute(position) {}
		virtual ~G42ItalicAttribute() {}
		virtual G42BaseAttribute * MakeCopy()
			{ return new G42ItalicAttribute(Italic, GetPosition()); }
		bool IsItalic() const
			{ return Italic; }
		virtual void ChangeText(G42Attributes & attributes)
			{ attributes.Italic = Italic; attributes.NeedFont = true; }
	private:
		bool Italic;
	};
	class G42UnderlineAttribute : public G42BaseAttribute
	{
	public:
		G42UnderlineAttribute (bool state, uintf position) : Underline(state),
			G42BaseAttribute(position) {}
		virtual ~G42UnderlineAttribute() {}
		virtual G42BaseAttribute * MakeCopy()
			{ return new G42UnderlineAttribute(Underline, GetPosition()); }
		bool IsUnderline() const
			{ return Underline; }
		virtual void ChangeText(G42Attributes & attributes)
			{ attributes.Underline = Underline; attributes.NeedFont = true; }
	private:
		bool Underline;
	};
	class G42ColorAttribute : public G42BaseAttribute
	{
	public:
		G42ColorAttribute (const G42Color & size, uintf position) : Color(size),
			G42BaseAttribute(position) {}
		virtual ~G42ColorAttribute() {}
		virtual G42BaseAttribute * MakeCopy()
			{ return new G42ColorAttribute(Color, GetPosition()); }
		const G42Color & GetColor() const
			{ return Color; }
		virtual void ChangeText(G42Attributes & attributes)
			{ attributes.Color = Color; }
	private:
		G42Color Color;
	};
	class G42LineSpacingAttribute : public G42BaseAttribute
	{
	public:
		G42LineSpacingAttribute (intf pixels, uintf position) :
			LineSpacing(pixels), G42BaseAttribute(position) {}
		virtual ~G42LineSpacingAttribute() {}
		virtual G42BaseAttribute * MakeCopy()
			{ return new G42LineSpacingAttribute(LineSpacing, GetPosition()); }
		intf GetLineSpacing() const
			{ return LineSpacing; }
		virtual void ChangeText(G42Attributes & attributes)
			{ attributes.LineSpacing = LineSpacing; }
	private:
		intf LineSpacing;
	};
	class G42ParagraphSpacingAttribute : public G42BaseAttribute
	{
	public:
		G42ParagraphSpacingAttribute (intf pixels, uintf position) :
			ParagraphSpacing(pixels), G42BaseAttribute(position) {}
		virtual ~G42ParagraphSpacingAttribute() {}
		virtual G42BaseAttribute * MakeCopy()
			{ return new G42ParagraphSpacingAttribute(ParagraphSpacing, GetPosition()); }
		intf GetParagraphSpacing() const
			{ return ParagraphSpacing; }
		virtual void ChangeText(G42Attributes & attributes)
			{ attributes.ParagraphSpacing = ParagraphSpacing; }
	private:
		intf ParagraphSpacing;
	};
	class G42TabSpacingAttribute : public G42BaseAttribute
	{
	public:
		G42TabSpacingAttribute (intf pixels, uintf position) :
			TabSpacing(pixels), G42BaseAttribute(position) {}
		virtual ~G42TabSpacingAttribute() {}
		virtual G42BaseAttribute * MakeCopy()
			{ return new G42TabSpacingAttribute(TabSpacing, GetPosition()); }
		intf GetTabSpacing() const
			{ return TabSpacing; }
		virtual void ChangeText(G42Attributes & attributes)
			{ attributes.TabSpacing = TabSpacing; }
	private:
		intf TabSpacing;
	};
	class G42FirstIndentAttribute : public G42BaseAttribute
	{
	public:
		G42FirstIndentAttribute (intf pixels, uintf position) :
			FirstIndent(pixels), G42BaseAttribute(position) {}
		virtual ~G42FirstIndentAttribute() {}
		virtual G42BaseAttribute * MakeCopy()
			{ return new G42FirstIndentAttribute(FirstIndent, GetPosition()); }
		intf GetFirstIndent() const
			{ return FirstIndent; }
		virtual void ChangeText(G42Attributes & attributes)
			{ attributes.FirstIndent = FirstIndent; }
	private:
		intf FirstIndent;
	};
	class G42LeftIndentAttribute : public G42BaseAttribute
	{
	public:
		G42LeftIndentAttribute (intf pixels, uintf position) :
			LeftIndent(pixels), G42BaseAttribute(position) {}
		virtual ~G42LeftIndentAttribute() {}
		virtual G42BaseAttribute * MakeCopy()
			{ return new G42LeftIndentAttribute(LeftIndent, GetPosition()); }
		intf GetLeftIndent() const
			{ return LeftIndent; }
		virtual void ChangeText(G42Attributes & attributes)
			{ attributes.LeftIndent = LeftIndent; }
	private:
		intf LeftIndent;
	};
	class G42RightIndentAttribute : public G42BaseAttribute
	{
	public:
		G42RightIndentAttribute (intf pixels, uintf position) :
			RightIndent(pixels), G42BaseAttribute(position) {}
		virtual ~G42RightIndentAttribute() {}
		virtual G42BaseAttribute * MakeCopy()
			{ return new G42RightIndentAttribute(RightIndent, GetPosition()); }
		intf GetRightIndent() const
			{ return RightIndent; }
		virtual void ChangeText(G42Attributes & attributes)
			{ attributes.RightIndent = RightIndent; }
	private:
		intf RightIndent;
	};
	class G42HorizontalAlignmentAttribute : public G42BaseAttribute
	{
	public:
		G42HorizontalAlignmentAttribute (G42AlignHoriz alignment, uintf position)
			: HorizontalAlignment(alignment), G42BaseAttribute(position) {}
		virtual ~G42HorizontalAlignmentAttribute() {}
		virtual G42BaseAttribute * MakeCopy()
			{ return new G42HorizontalAlignmentAttribute(HorizontalAlignment,
				GetPosition()); }
		G42AlignHoriz GetHorizontalAlignment() const
			{ return HorizontalAlignment; }
		virtual void ChangeText(G42Attributes & attributes)
			{ attributes.HorizontalAlignment = HorizontalAlignment; }
	private:
		G42AlignHoriz HorizontalAlignment;
	};
	class G42VerticalAlignmentAttribute : public G42BaseAttribute
	{
	public:
		G42VerticalAlignmentAttribute (G42AlignVert alignment, uintf position)
			: VerticalAlignment(alignment), G42BaseAttribute(position) {}
		virtual ~G42VerticalAlignmentAttribute() {}
		virtual G42BaseAttribute * MakeCopy()
			{ return new G42VerticalAlignmentAttribute(VerticalAlignment,
				GetPosition()); }
		G42AlignVert GetVerticalAlignment() const
			{ return VerticalAlignment; }
		virtual void ChangeText(G42Attributes & attributes)
			{ attributes.VerticalAlignment = VerticalAlignment; }
	private:
		G42AlignVert VerticalAlignment;
	};
	class G42BulletAttribute : public G42BaseAttribute
	{
	public:
		G42BulletAttribute (intf bullet, bool use_default,
			bool force_indent, intf position)
		:
			G42BaseAttribute(position),
			Bullet(bullet), UseDefault(use_default), ForceIndent(force_indent)
		{}
		virtual ~G42BulletAttribute() {}
		virtual G42BaseAttribute * MakeCopy()
			{ return new G42BulletAttribute(Bullet, UseDefault, ForceIndent,
				GetPosition()); }
		intf GetBullet() const
			{ return Bullet; }
		bool GetUseDefault() const
			{ return UseDefault; }
		bool GetForceIndent() const
			{ return ForceIndent; }
		virtual void ChangeText(G42Attributes & attributes)
			{ attributes.BulletCharacter = Bullet;
				attributes.BulletHanging = ForceIndent;
				attributes.BulletDingbats = UseDefault; }
	private:
		intf Bullet;
		bool UseDefault;
		bool ForceIndent;
	};
	G42Point TextStart; // where the text starts
	G42APtr <char> TextArray; // have to be careful here
	G42Vector <G42Point> CursorLoc; // one per character
	G42Vector <intf> PropLine;
	intf InsertIndex;
	intf NumProp;
	intf NumText;
	intf MaxText;
	G42Point Start; // where on the screen the text starts (mouse click point)
	#ifdef MSWIN
	HFONT Font; // current font to use (changes as we go through attribute array
	#endif
	G42Vector <G42Ptr <G42BaseAttribute> > AttributeArray; // array of attributes
	intf NumAttributes;
	G42Vector <intf> ParagraphArray; // character where paragraph starts
	G42Vector <intf> ParagraphAttribute; // attribute where paragraph starts
	intf NumParagraph;   // in textArray (characters)
	intf ParagraphStart; // where in the attributes the first starts
	intf WrapWidth; // width of wrap (0 is no wrap)
	intf WrapHeight; // max height (0 is no max)
	G42Attributes Current;
	G42Attributes InsertAttributes;
	G42Attributes Saved;
	G42Vector <G42Line> Line;
	intf NumLines;
	G42Vector <G42Section> Section;
	intf NumSections;
	void 						CheckTextArray(intf length);
	virtual void			DrawGrafObject (G42DrawLocation location = 0);
	void						UpdateBounds (bool final = true);
	void						ResetState();
	void						SaveState();
	void						RestoreState();
	void						HandleAttribute(G42BaseAttribute & attribute);
	void						SetFont();
	void						Calculate(G42DrawLocation location);
	void						DrawText (G42DrawLocation location);
	void DrawCursor(G42DrawLocation location = 0);
	intf GetAscent(G42DrawLocation location);
	intf GetDescent(G42DrawLocation location);
	intf GetLeading(G42DrawLocation location);
	intf GetCharsWidth(const char * c, intf num_c, G42DrawLocation location);
	intf GetCharWidth(char c, G42DrawLocation location);
};
#if 0 // original java code
// g42text.java - implements g42text class
/*
	 g42text.java - stores and paints advanced text
	 formating.
	 Copyright (c) 1996 Group 42, Inc.
	 This class implements text that can change attributes
	 between characters, and can handle paragraphs.  To use,
	 you create it, set up your initial conditions, then
	 start adding text with addText().  When you need to
	 change an attribute, you just change it, and the class
	 will only change the attribute for new text added.  To
	 start a new paragraph, call nextParagraph().  nextParagraph()
	 can be called for the first paragraph if you want to set
	 up some attributes on a global bases, but others are inside
	 the paragraph.  This class will word wrap if you setWrapWidth()
    which is the width of the box to wrap words.
	 I eventually plan to add the ability to move the insertion
    point for addText(), and to give the ability to select a
    range of text to change an attribute over.
Know problems or bugs:
    No known problems.
Fixed problems
    23 Sep 96 - added word wrapping, fixed other bugs, added comments
    21 Nov 96 - added bullets, fixed word wrapping
    22 Nov 96 - added vertical alignment
    13 Jan 97 - fixed indent problems
    23 Jan 97 - changed bullets to drawing a circle
     4 Feb 97 - removed it from the package, added alignment without wrap
Constructor Index
	 public Text()
		  creates an empty Text object
Method Index
    public void addText(char [] text, int offset, int num)
    public void addText(String string)
    public void addText(String string, int offset, int num)
        Adds the text using the current attributes
    public void nextParagraph()
        Starts a new paragraph.  You don't have to call this
        for the first paragraph (although you can, especially if
        you want to have some attributes tied to the paragraph
        instead of the whole thing).
    public void setStart(Point start)
        Sets the starting point of the text string.
    public void setFontName(String name)
		  Sets the font name.  Must be one of
        Toolkit.getDefaultToolkit().getFontList()
	 public void setFontSize(int pixels)
		  Sets the size of the font in pixels
    public void setBold(boolean state)
		  Turns bold on or off
    public void setItalic(boolean state)
        Turns italic on or off
    public void setColor(Color color)
        Sets the color of the text
    public int getWrapWidth()
        gets the width of word wrapping
    public void setWrapWidth(int width)
        turns on word wrapping and sets the width
    public boolean isWrapped()
        returns true if word wrapping is enabled
    public void setNoWrap()
        turns off word wrapping
	 public void setUnderline(boolean state)
        turns on or off underlining
	 public void setWrapHeight(int pixels)
        sets word wrapping height
    public void setLineSpacing(int pixels)
        sets the line spacing in pixels
    public void setParagraphSpacing(int pixels)
        sets the paragraph spacing in pixels
    public void setTabSpacing(int pixels)
        sets the tab spacing in pixels
    public void setFirstIndent(int pixels)
        sets the indent of the first line of the paragraph in pixels
    public void setLeftIndent(int pixels)
		  sets the left indent of the paragraph in pixels
    public void setRightIndent(int pixels)
		  sets the right indent of the paragraph in pixels
    These next variables are constants for the alignment functions below
	 public static int AlignLeft = 0;
    public static int AlignTop = 0;
    public static int AlignCenter = 1;
    public static int AlignRight = 2;
    public static int AlignBottom = 2;
    public void setHorizontalAlignment(int state)
        sets the horizontal alignment (left, center, or right)
    public void setVerticalAlignment(int state)
        sets the vertical alignment (top, center, or bottom)
    public void addBullet()
        adds a bullet into the text
    public void addBullet(int bullet)
		  adds a bullet from the dingbat font into the text
    public void addIndentBullet()
		  adds a bullet into the text and indents all following lines
		  of the paragraph to one tab stop to the right of the bullet
    public void addIndentBullet(int bullet)
		  adds a bullet into the text and indents all following lines
        of the paragraph to one tab stop to the right of the bullet.
        The bullet is from the dingbats font.
    public void draw(Graphics g)
        draws the text
*/
public class Text extends Object
{
	 public static int AlignLeft = 0;
	 public static int AlignTop = 0;
	 public static int AlignCenter = 1;
	 public static int AlignRight = 2;
	 public static int AlignBottom = 2;
	 public static int LineSpacingProportional = 1;
	 public static int LineSpacingIncremental = 2;
	 public static int LineSpacingAbsolute = 3;
	 private char [] textArray = null;
    private int numText = 0;
    private Point start = null;
	 private Font font = null;
    private BaseTextAttribute [] attributeArray = null;
    private int numAttributes = 0;
    private int paragraphArray [] = null; // where paragraph starts
    private int paragraphAttribute [] = null; // attribute where paragraph starts
    private int numParagraph = 0;   // in textArray (characters)
    private int paragraphStart = 0; // where in the attributes the first starts
    private int wrapWidth = 0;
    private int wrapHeight = 0;
    boolean needFont = true;
    boolean needColor = true;
    String name = null;
    boolean bold = false;
    boolean italic = false;
    boolean underline = false;
    int horizontalAlignment = AlignLeft;
	 int verticalAlignment = AlignTop;
	 int size = 12;
	 Color color = null;
	 int tabSpacing = 5;
	 int leftIndent = 0;
	 int rightIndent = 0;
	 int firstIndent = 0;
	 int paragraphSpacing = 0;
	 int lineSpacing = 0;
	 int lineSpacingMode = LineSpacingIncremental;
	 int bulletCharacter;
	 boolean bulletHanging;
	 boolean bulletDingbats;
	 String savedName;
	 boolean savedBold;
	 boolean savedItalic;
	 boolean savedUnderline;
	 int savedHorizontalAlignment;
	 int savedVerticalAlignment;
	 int savedSize;
	 Color savedColor;
	 int savedTabSpacing;
	 int savedFirstIndent;
	 int savedLeftIndent;
	 int savedRightIndent;
	 int savedParagraphSpacing;
	 int savedLineSpacing;
	 int savedLineSpacingMode;
	 private int lineWidth [] = null;
	 private int lineAscent [] = null;
	 private int lineDescent [] = null;
	 private int lineSectionStart [] = null;
	 private int lineSectionEnd [] = null;
	 private int numLines = 0;
	 private boolean sectionNewParagraph [];
	 private int sectionTextStart [];
	 private int sectionTextNum [];
	 private int sectionX [];
	 private int sectionY [];
	 private int sectionAttribute [];
	 private boolean sectionBullet [];
	 private int sectionBulletChar [];
	 private int numSections;
	public Text()
	private void checkTextArray(int length)
	private void checkAttributeArray(int length)
	private void checkParagraphArray(int length)
	public void addText(char [] text, int offset, int num)
	public void addText(String string)
	public void nextParagraph()
	public void addText(String string, int offset, int num)
	public void setStart(Point start)
	public void setFontName(String name)
	public void setFontSize(int size)
	public void setBold(boolean bold)
	public void setItalic(boolean bold)
	public void setColor(Color color)
	public int getWrapWidth()
	public void setWrapWidth(int width)
	public boolean isWrapped()
	public void setNoWrap()
	public void setUnderline(boolean state)
	public void setWrapHeight(int pixels)
	public void setLineSpacing(int pixels, int mode)
	public void setParagraphSpacing(int pixels)
	public void setTabSpacing(int pixels)
	public void setFirstIndent(int pixels)
	public void setLeftIndent(int pixels)
	public void setRightIndent(int pixels)
	public void setHorizontalAlignment(int state)
	public void setVerticalAlignment(int state)
	public void addBullet()
	public void addBullet(int bullet)
	public void addIndentBullet()
	public void addIndentBullet(int bullet)
	private void resetState()
	private void saveState()
	private void restoreState()
	private void handleAttribute(BaseTextAttribute attribute)
	private void setFont(Graphics g)
	private void checkLineArrays(int length)
	private void checkSectionArrays(int length)
	private int propLine [];
	private int numProp;
	private void checkProp(int length)
	private void calculate(Graphics g)
	public void draw(Graphics g)
#endif
#endif // giobtext.h

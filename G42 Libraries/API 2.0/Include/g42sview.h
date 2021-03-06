// G42SView - Single Image Viewer
#if !defined(G42SVIEW_H)
#define G42SVIEW_H
/*
	G42SingleImageViewer class - displays a single image in a window,
		and handles selected areas, text, and mapping modes.
	The mapping code is in g42svmap.cpp, the selected area code is in
	that and g42sview.cpp, the text code is in g42svtxt.cpp.  The mouse
	is handled in g42svmap.cpp, since most of the mouse code deals with
	maps.
	Drawing is handled in g42sview.cpp mostly, with the mapping and
	text stuff handled in g42svmap.cpp and g42svtxt.cpp.  When the
	draw function is called, PrepareShadow is called with the size,
	then PrepareDrawImage is called, then DrawImage is called, then
	CleanupDrawImage is called, followed by any other element depending
	upon what mode we are in.  For example, if we are in mapping mode,
	PrepareDrawMap is called, followed by DrawMap, followed by CleanupDrawMap.
	After all elements have been drawn, PutShadow is called to put the shadow
	area on the screen, and clean up any memory associated with the shadow.
	Any rubber shapes are drawn directly on the screen, and are sandwitched
	between PrepareDrawRubber and CleanupDrawRubber.  There is also a
	StartRubber and EndRubber that corresponds with the mouse down and up,
	if that will make things easier.  Because of scrolling, be aware that
	you may get Draw calls between the StartRubber and EndRubber, but
	never between a PrepareDrawRubber and CleanupDrawRubber.
	Note that g42zview has a PrepareDrawImage and CleanupDrawImage, but that
	these must be overridden, because zview doesn't have a shadow.  However,
	for speed purposes, if nothing is being drawn besides the image, you
	can let zview do this, if you don't mind the tiles flashing on the screen
	one at a time.
	Primary author:
		Guy Eric Schalnat
	Mac author:
		David Ison
*/
#include <g42zview.h>
#include <g42imap.h>
#include <g42sbar.h>
#include <g42text.h>
#include <g42opal.h>
const MapPointSize = 10; // size of a point (really half) unzoomed
#ifdef MACOS
const SelectMargin  		= 5;
const GrabMargin 			= 3;
const GrabInternalMargin 	= 2;
#else
const SelectMargin = 7;
const GrabMargin 	= 7;
#endif
class G42SingleImageViewer : public G42ZoomImageViewer
{
public:
#ifdef MSWIN
	G42SingleImageViewer(HWND window, HWND top_window);
#endif
#ifdef MACOS
	G42SingleImageViewer (GWorldPtr windowPtr, short xOrigin, short yOrigin);
#endif
	~G42SingleImageViewer(void);
	virtual void Draw(void);
	virtual void Draw(int x, int y, int width, int height, bool expand = false);
	virtual void Rebuild(void);
	virtual void SetZoom(double zoom);
	virtual void SetWindowSize(int width, int height);
	void SetImageMap(G42ImageMap * image_map);
	void SetImageMapVisible(bool visible);
	bool GetImageMapVisible(void)
		{ return ImageMapVisible; }
	void MouseMove(int x, int y, bool shift_down, bool control_down);
	void MouseUp(int x, int y, int button, bool shift_down, bool control_down);
	void DrawImageMapElement(G42DrawLocation location,
		const G42ImageMap::G42MapElement * element);
	#ifdef MACOS
	void DrawImageMapElementPolygon(G42DrawLocation location,
	const G42ImageMap::G42MapElement * element, bool is_new,
	bool is_current, bool is_selected);
	#endif
	void PrepareImageMapDC(G42DrawLocation location);
#ifdef MSWIN
	void SetCursor(); // for MSWindows WM_SETCURSOR message
	void SetTransCursor(HCURSOR cursor)
		{ TransCursor = cursor; }
	void PrintImage(G42DrawLocation location,
		int32 start_x, int32 start_y,
		int32 width, int32 height);
	virtual void WindowSizeChanged(void);
#endif
// new map stuff - GES
	enum G42MapTool { MapRectangleTool, MapCircleTool, MapPolygonTool,
		MapPointTool, MapEditTool };
	void SetMapTool(G42MapTool map_tool);
	G42MapTool GetMapTool(void)
		{ return MapTool; }
	void GetCircleRect(int & x1, int & y1, int & x2, int & y2);
	virtual bool AddMapElement(G42ImageMap::G42MapElement & element);
	virtual void SetStatusString(const char * string);
	virtual void EditMapElement(uint index);
	virtual void MapSelChanged(void);
	virtual void SetURLString(const char * string);
	void DrawSelectedArea(G42DrawLocation location);
	void EraseSelectedArea(G42DrawLocation location);
	void DrawSelectedRectangle(G42DrawLocation location);
	void DrawImageMapElement(G42DrawLocation location, uint index);
	void DrawImageMapElement(G42DrawLocation location,
		const G42ImageMap::G42MapElement * element, bool is_new = false,
		bool is_current = false, bool is_selected = false);
	bool MouseDown(int x, int y, int button, bool shift_down, bool control_down,
		uint count); // count is 0 first time, 1 for double clicks, etc.
	void StartStretchElement(void);
	void StretchNewElement(bool shift_down);
	void MoveNewElement(int dx, int dy);
	void AboutToMoveElement(void);
// end new map stuff - GES
// new text stuff - GES
	void SetTextVisible(bool visible);
	bool GetTextVisible(void)
		{ return TextVisible; }
	void SetTextStyle(G42Text * text);
	void Keystroke(int key);
	void TextStyleUpdated(void);
	enum G42Key
		{KeyReturn = 256, KeyBackspace, KeyDelete, KeyHome, KeyEnd,
			KeyLeft, KeyRight, KeyUp, KeyDown};
	virtual bool IdleAction(void);
// end new text stuff - GES
	void SetTransparentMode(bool mode);
	bool GetTransparentMode(void);
	virtual void TransparentPointClick(uint32 x, uint32 y, bool final);
	virtual void SelectedImageChanged(G42Image * image);
	void DeleteSelectedImage(bool erase = false);
	void AddSelectedImage(G42Image * image, bool reset_location = false);
	void CreateSelectedImage(int32 x, int32 y, int32 width, int32 height);
	virtual void Clear(void);
	G42Image * GetSelectedImage(void)
		{ return SelectedImage; }
	#ifdef MACOS
	CursPtr		GetCurrentCursor (void) {return CurrentCursor;};
	#endif
	void		DrawIdle (G42DrawLocation location = 0);	// Draws marchings ants
	void 		DrawSelectedHandles (G42DrawLocation location = 0);
	bool 		IsAnElementSelected (void);
	void		DrawImageOnly (G42DrawLocation location, G42Image *image);
	void		DrawImageOnly (G42DrawLocation location, G42Image *image, int x,
					int y, int width, int height, bool use_zoom = true);
	void		SetUseShadow (Boolean useShadow);
	Boolean		UseShadow;
	void 		PutAreaDown(void);
	
private:
// new text stuff - GES
	bool TextVisible;
	bool TextStarted;
	G42Array<char> TextArray;
	G42Array<int> TextPosX;
	G42Array<int> TextPosY;
	int NumTextArray;
	int CursorTextPos;
	G42Text * TextStyle;
	int ErasingText;
	int ErasingTextMinX;
	int ErasingTextMinY;
	int ErasingTextMaxX;
	int ErasingTextMaxY;
	#ifdef MSWIN
	HFONT OldFont;
	int OldBkMode;
	#endif
	bool NeedMainIdle;
	bool NeedSelectedIdle;
	bool CheckMainPartial;
	bool CheckSelectedPartial;
	virtual void Draw(G42DrawLocation location, int x, int y,
		int width, int height, bool expand = false);
	void DrawText();
	void DrawText(G42DrawLocation location);
	void EraseText();
	void DrawCursor();
	void DrawCursor(G42DrawLocation location);
	void AcceptText();
	void SetTextStartPos(int x, int y);
	void UpdateTextPos();
	void GetTextExtents(const char * line, int length, int * w, int * h);
	void SetMouseString(int32 x, int32 y);
	void FillAntiAliasTextMask(byte * to, byte * from, int width, int height, int scale);   
	bool EditingPolygon;
	uint EditingPolygonIndex;
// end new text stuff - GES
// new map stuff - GES
	bool MoveFromSelect;
	G42MapTool MapTool;
	bool NewMapElementStarted;
	bool NewPolygonStarted;
	bool NewPolygonClosed;
	bool MovingElement;
	bool StretchingElement;
	int CurrentElement;
	bool ReportingTransColors;
	G42ImageMap::G42MapElement NewElement;
	int SelX;
	int SelY;
	int SelWidth;
	int SelHeight;
	bool SelStretch;
	int32 OtherX;
	int32 OtherY;
	int StretchMinX;
	int StretchMinY;
	int StretchMaxX;
	int StretchMaxY;
	int CursorXFraction;
	int CursorYFraction;
	int OldCursorX;
	int OldCursorY;
	int OldControlDown;
	int SettingCursor;
	enum G42StretchMode {
		StretchUpper = 0x1,
		StretchLower = 0x2,
		StretchLeft = 0x4,
		StretchRight = 0x8,
		StretchUpperLeft = 0x5,
		StretchUpperRight = 0x9,
		StretchLowerLeft = 0x6,
		StretchLowerRight = 0xa,
		StretchTowardsRight = 0x10,
		StretchTowardsLower = 0x20,
		StretchProportional = 0x40
		};
	uint StretchMode;
	bool ShiftInverted;
	void DrawRubberCircle(int x1, int y1, int x2, int y2);
	void MakeProportional(void);
	// end new map stuff - GES
	enum G42CursorType {ArrowCursor, CrossCursor, TextCursor, MoveCursor,
		StretchHorizontalCursor, StretchVerticalCursor,
		StretchTopLeftCursor, StretchTopRightCursor,
		TransparencyCursor };
	void SetCursor(G42CursorType type);
	G42Image * SelectedStretchImage;
	G42Image * SelectedDitherImage;
	G42Image * SelectedDitherBaseImage;
#ifdef MSWIN
	G42DrawLocation DC;
	HCURSOR WindowsCursor;
	HCURSOR TransCursor;
	HPEN AreaPen;
	HPEN OldAreaPen;
	HPEN MapNormalPen;
	HPEN MapSelectedPen;
	HPEN OldMapPen;
	HBITMAP Shadow;
	G42DrawLocation ShadowDC;
#endif
#ifdef MACOS
	void 			SetRectMac (Rect *rect, short left, short top, short right, 
						short bottom);	// kill
	void 			CheckRect (Rect *rect); // Debugging thing
	CursPtr			CurrentCursor;
	static 			Pattern AntsPattern;	
	static 			Pattern AntsPattern2;
	CGrafPtr		ActivePort;
	G42DrawLocation	OldPrepDrawMapPort;
	G42DrawLocation	OldPrepDrawRubberPort;
	GDHandle		OldPrepDrawMapDevice;
	GDHandle		OldPrepDrawRubberDevice;
	GWorldPtr		ShadowPort;
	short			XOrigin;
	short			YOrigin;
	//#define 		SETORIGIN
#endif
	G42ImageMap * ImageMap;
	bool ImageMapVisible;
	void DrawRubberRect(int x1, int y1, int x2, int y2);
	void DrawRubberImageRect(int32 x1, int32 y1, int32 x2, int32 y2);
	void DrawRubberRect(G42DrawLocation location,
		int x1, int y1, int x2, int y2);
	void DrawRubberImageRect(G42DrawLocation location,
		int32 x1, int32 y1, int32 x2, int32 y2);
	void SelectArea(int32 x1, int32 y1, int32 x2, int32 y2);
	void StartRubber(void);
	void EndRubber(void);
	void StretchSelectedArea(void);
	void StretchSelectedRubberArea(bool shift_down = false);
	void AbsStretchingRubberArea(bool shift_down = false);
	bool SelectingArea;
	bool AreaSelected;
	bool MovingArea;
	bool StretchingArea;
	int32 SelectedAreaX;
	int32 SelectedAreaY;
	int32 SelectedAreaWidth;
	int32 SelectedAreaHeight;
	G42Image * SelectedImage;
	int32 StartX;
	int32 StartY;
	int32 CurrentX;
	int32 CurrentY;
	int32 MouseX;
	int32 MouseY;
	bool TransparentMode;
	virtual void PreparePalette(void);
	virtual void PrepareDrawImage(G42DrawLocation location,
		int draw_x, int draw_y, int draw_width, int draw_height);
	virtual void CleanupDrawImage(G42DrawLocation location);
	virtual void PrepareShadow(int draw_x, int draw_y,
		int draw_width, int draw_height);
	virtual void PrepareDrawMap(G42DrawLocation location);
	virtual void DrawMap(G42DrawLocation location);
	virtual void CleanupDrawMap(G42DrawLocation location);
	virtual void PrepareDrawArea(G42DrawLocation location);
	virtual void DrawArea(G42DrawLocation location);
	virtual void CleanupDrawArea(G42DrawLocation location);
	virtual void PrepareDrawText(G42DrawLocation location);
	virtual void CleanupDrawText(G42DrawLocation location);
	virtual void PutShadow(int draw_x, int draw_y,
		int draw_width, int draw_height);
	virtual void PrepareDrawRubber(G42DrawLocation location);
	virtual void CleanupDrawRubber(G42DrawLocation location);
};
#endif // G42SVIEW_H

#pragma once
#ifndef DLGSTATUS
enum  DlgStatus {Ok, Cancel, PreviewOk, PreviewCancel, View};
#define DLGSTATUS
#endif
// Some resources
/*  Edit URL Dialog  */
const ResIDT	dialog_EditURLDlg			= 7000;
const ResIDT	pane_EditURLEdit1			= 7001;
const ResIDT	pane_EditURLEdit2			= 7002;
const ResIDT	pane_EditURLEdit3			= 7003;
/*  View URL's (modeless) Dialog  */
const ResIDT	dialog_ViewURLDlg			= 7050;
const ResIDT	pane_ViewURLListBox			= 7051;
const ResIDT	pane_ViewURLMoveUp 			= 7052;
const ResIDT	pane_ViewURLMoveDn 			= 7053;
const ResIDT	pane_ViewURLEdit 			= 7054;
const ResIDT	pane_ViewURLDelete			= 7055;
const ResIDT	pane_ViewURLOk				= 7056;
const ResIDT	pane_ViewURLAddDefault 		= 7057;
const ResIDT	value_ViewURLListBoxDblClk 	= 7058;
const ResIDT	value_ViewURLMoveUp 		= 7052;
const ResIDT	value_ViewURLMoveDn 		= 7053;
const ResIDT	value_ViewURLEdit 			= 7054;
const ResIDT	value_ViewURLDelete			= 7055;
const ResIDT	value_ViewURLOk				= 7056;
const ResIDT	value_ViewURLAddDefault 	= 7057;
/*  Save Options Dialog  */	
const ResIDT	dialog_SaveOptions			= 7100;
const ResIDT	pane_ColorsPopupMenu		= 7101;
const ResIDT	pane_DitherRadio			= 7102;
const ResIDT	pane_MatchingRadio			= 7103;
const ResIDT	pane_ComprPopupMenu			= 7104;
const ResIDT	pane_JPEGSlider				= 7105;
const ResIDT	pane_JPEGCaption			= 7106;
const ResIDT	pane_PhotographicRadio		= 7107;
const ResIDT	pane_LineartRadio			= 7108;
const ResIDT	pane_ProgressiveCheckbox	= 7109;
const ResIDT	value_ColorsPopupMenu		= 7101;
const ResIDT	value_ComprPopupMenu		= 7104;
const ResIDT	value_JPEGSlider			= 7105;
/*  String Tables  */	// The string table id's should eventually not be 210 & 211
const ResIDT	STRx_SAOptionsColors		= 210;	// SA = Save As
const ResIDT	STRx_SAOptionsCompressions	= 211;	// SA = Save As
/*  STRx_ImageFormats indexes  */
const ResIDT	str_1BitMonochrome			= 1;
const ResIDT	str_2BitColor				= 2;
const ResIDT	str_2BitGray				= 3;
const ResIDT	str_4BitColor				= 4;
const ResIDT	str_4BitGray				= 5;
const ResIDT	str_6BitColor				= 6;
const ResIDT	str_6BitGray				= 7;
const ResIDT	str_8BitColor_236			= 8;
const ResIDT	str_8BitGray_236			= 9;
const ResIDT	str_8BitColor				= 10;
const ResIDT	str_8BitGray				= 11;
const ResIDT	str_24BitColor				= 12;
const ResIDT	str_16BitGray				= 13;
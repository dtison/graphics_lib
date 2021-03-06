/*------------------------------------------------------------------
	File:          Example.cp  Copyright (c) 1996, Group 42, Inc.
	Description:   Example program for implementing the Image SDK 2.0
	Author:        David Ison
	Creation Date: 26 June 1996
	Modification History:
     Code   Date   Name and Description
------------------------------------------------------------------*/
#include "Example.h"
#include <LGrowZone.h>
#include <LWindow.h>
#include <PP_Messages.h>
#include <PP_Resources.h>
#include <PPobClasses.h>
#include <UDrawingState.h>
#include <UMemoryMgr.h>
#include <URegistrar.h>
#include <LEditField.h>
#include "g42ifile.h"	// G42ImageFile class
#include "g42iview.h"	// G42SimpleImageViewer class
#include "g42macfl.h"	// Mac Filename class
const ResIDT	window_Sample		= 1;
const ResIDT	window_Example		= 128;	// The window where we put up some G42 images
void main(void)
{
	SetDebugThrow_(debugAction_Alert);
	SetDebugSignal_(debugAction_Alert);
	InitializeHeap(3);	
	UQDGlobals::InitializeToolbox(&qd);
	new LGrowZone(20000);	
	CPPStarterApp	theApp;		
	theApp.Run();
}
CPPStarterApp::CPPStarterApp() : DataObject (0), Swap (0), Core (0), Page (0)
{	
	RegisterAllPPClasses();
	/*  Initialize sundry image things  */
	for (short i = 0; i < Num_Images; i++)
		Images [i] = 0;
	/*  (These are the points at which the images will be drawn) */
	DrawOrigins [0].h = 50;
	DrawOrigins [0].v = 50;
	DrawOrigins [1].h = 250;
	DrawOrigins [1].v = 50;	
	DrawOrigins [2].h = 50;
	DrawOrigins [2].v = 250;		
	DrawOrigins [3].h = 250;
	DrawOrigins [3].v = 250;		
}
CPPStarterApp::~CPPStarterApp()
{
	/*  Delete the G42 Memory Manager  */
	if (DataObject)	
	{
		delete DataObject; delete Page; delete Core; delete Swap;		
		DataObject = 0; Page = 0; Core = 0; Swap = 0;		
	}
	/* Delete the images  */
	for (short i = 0; i < Num_Images; i++)
		if (Images [i])
		{
			delete Images [i];
			Images [i] = 0;
		}
}
void
CPPStarterApp::StartUp()
{
	/*  Set up the temporary files path for swap file */
	//  (All this code from here..)
    short 	vRefNum;
    long 	dirID;
	char 	*swapFilePath;
	FSSpec 	dirFSSpec;
	FindFolder (kOnSystemDisk, kTemporaryFolderType, kCreateFolder, &vRefNum, &dirID); 
	FSMakeFSSpec (vRefNum, dirID, nil, &dirFSSpec);
	swapFilePath = G42MacFile::GetFullPath (dirFSSpec.name, dirFSSpec.parID, dirFSSpec.vRefNum);
	strcat (swapFilePath, ":G42ExampleSwapFile");
	Swap = new G42SwapManager (swapFilePath);
	delete [] swapFilePath;
	// (..To here is for getting a file path for the swap file.)
	
	/*  Initialize the memory manager 					    */
	long memBytes 		= ::MaxBlock ();
	long minBytes 		= 512 * 1024;	
	/* In this example we trust there is at least 512k free */
	memBytes 			= (((memBytes - minBytes) / 65528) * 65528);
	Core 				= new G42CoreTable (Swap, memBytes);
	Page 				= new G42PageTable ();
	DataObject  		= new G42Data (Core, Page);	
	
	/*  (DrawSomeImages() in a real program would not be done in StartUp())	*/
	DrawSomeImages();
	while (! Button());	// Wait for a mouse click
	DoQuit(false);		// Kill this program
}
/*  DrawSomeImages() does a little more than just drawing. But, all the 
	necessary steps for drawing are here */
void
CPPStarterApp::DrawSomeImages (void)
{
	/*  First put a window up  */
	LWindow		*window;
	window = LWindow::CreateWindow (window_Example, this);	
	char buffer [80];
	
	/*  Get images named image1..image4	 */
	for (short i = 0; i < Num_Images; i++)
	{
		::sprintf (buffer, "image%d", (i + 1));		
		Images [i] = ReadImage (c2pstr (buffer));			
	}
	
	/* Create a viewer   */
	GrafPtr port = window -> GetMacPort();
	G42SimpleImageViewer viewer ((GWorldPtr) port);
	
	/*  We have to tell the viewer the window size  */
	Rect frame;
	window -> CalcLocalFrameRect (frame);
	int windowWidth 	= frame.right - frame.left;
	int windowHeight 	= frame.bottom - frame.top;
	viewer.SetWindowSize (windowWidth, windowHeight);
	
	/*  Draw the images  */
	for (short i = 0; i < Num_Images; i++)
	{
		G42Image *image = Images [i];
		if (image)
		{
			/*  (This is an example use of the GetInfo() function) */
			int width 	= image -> GetInfo().Width;
			int height 	= image -> GetInfo().Height;
			viewer.DrawImageAt ((GWorldPtr) port, image, 
				DrawOrigins [i].h, (int) DrawOrigins [i].v);
		}
	}	
}
/*  ReadImage() is a basic example of reading in an image from a file.  
	It contains all the necessary steps for getting an image created  */
G42Image
*CPPStarterApp::ReadImage (StringPtr filename)
{
	G42Image *image = 0;		// This will be the pointer returned
	long bufferSize = 4096L;	// By convention, we read 4k chunks
	short fileRef;
	OSErr err = ::FSOpen (filename, 0, &fileRef);
	if (! err)
	{
		/*  See how many bytes to read  */	
		long bytesToRead, bytesRead = 0;
		::GetEOF (fileRef, &bytesToRead);
		char *buffer = new char [bufferSize];
		/*  Create a new, "blank" image  */
		image = new G42Image (DataObject);
		/*  Now make a reader for that image  */
		G42ImageFile reader (image);
		while (reader.IsValid() && ! reader.IsFinished() && bytesRead < bytesToRead)
		{
			::FSRead (fileRef, &bufferSize, buffer);		
			bytesRead += bufferSize;
			reader.ProcessData ((byte *) buffer, bufferSize);
		}
		/*  The ForceFileEnd() is necessary to clean up some readers */
		reader.ForceFileEnd();	
		/*  Check IsValid() and IsFinished() as needed  */
		if (! image -> IsValid())
		{
			delete image;
			image = 0;
		}
		if (! image -> IsFinished())
		{
			delete image;
			image = 0;
		}
		::FSClose (fileRef);
		
		/*  This is an optional call to set the file size  */
		image -> SetInfoFileSize (bytesRead);	
		delete [] buffer;
	}
	return image;
}

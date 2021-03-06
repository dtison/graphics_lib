// gicamult.cpp - G42AreaMultiController
#include "gicaanim.h"
#include "gicsur.h"
#include "gpath.h"
#include "gtools.h"
#include "gpobj.h"
G42AreaAnimatedController::G42AreaAnimatedController(G42Notifier * notifier)
:
	G42MultiAreaBaseController(notifier), FrameImage(0), CurrentFrameIndex(0),
	UseSelectedInPath(false)
{
	TextSpace = 25; // no need to have 50 pixels
}
G42AreaAnimatedController::~G42AreaAnimatedController(void)
{
}
// save off stuff?, send out frames (only selected if UseSelectedInPath)
// be sure to send a good AnimatedDescriptor
void
G42AreaAnimatedController::BeginProcessing(void)
{
}
// if UseSelectedInPath, only replace frames that are selected.  Update
// AnimatedDescriptor and watch (a la G42Display) that the multi-palette is
// valid.  Be careful of extra or lacking frames.
void
G42AreaAnimatedController::Process(G42PathObject * object, G42PathComponent *
	sender)
{
	if (dynamic_cast<G42PathBegin *>(object))
	{
		G42AreaSingleController::Process(object, sender);
		NumImages = 0;
		CurrentFrameIndex = 0;
		ImageControllers.Clear();
		StringArray.Clear();
	}
	else if (dynamic_cast<G42AnimatedImageDescriptor *>(object))
	{
		G42AreaSingleController::Process(object, sender);
	}
	else if (dynamic_cast<G42MultipleImageDescriptor *>(object))
	{
		G42AreaSingleController::Process(object, sender);
	}
	else if (dynamic_cast<G42ImageDescriptor *>(object))
	{
		G42ImageDescriptor * desc =
			dynamic_cast<G42ImageDescriptor *>(object);
		desc = new G42ImageDescriptor(*desc);
		G42AreaSingleController::Process(object, sender);
		if (FrameImage) // clean out old path image
		{
			FinishFrame();
#if 0
			G42PathEnd * end = new G42PathEnd();
			FrameImage->Process(end, this);
			FrameImage = 0;
			ResetGrowth();
			ResetThumbLocation(CurrentFrameIndex);
			ImageControllers[CurrentFrameIndex]->GetDisplay()->SetAutoDraw(false);
			G42Path path(*(ImageControllers[CurrentFrameIndex]->GetImage()));
			path.AddLink(*(ImageControllers[CurrentFrameIndex]->GetImage()),
				*(ImageControllers[CurrentFrameIndex]->GetDisplay()));
			path.Start();
			path.Clear();
			ImageControllers[CurrentFrameIndex]->GetDisplay()->SetAutoDraw(true);
			char * buf = new char [10];
			sprintf(buf, "%d", CurrentFrameIndex + 1);
			StringArray[CurrentFrameIndex] = buf;
			Selected[CurrentFrameIndex] = false;
			G42Point translate(ScaleTranslate.GetTranslate());
			CumlativeScaleTranslate.Convert(translate);
			G42ScaleTranslate image_trans(
				CumlativeScaleTranslate.GetScaleX() * ScaleTranslate.GetScaleX(),
				CumlativeScaleTranslate.GetScaleY() * ScaleTranslate.GetScaleY(),
				translate);
			ImageControllers[CurrentFrameIndex]->SetCumlativeScaleTranslate(
				image_trans);
#endif
		}
		CurrentFrameIndex = NumImages++;
		ImageControllers[CurrentFrameIndex] = new G42ImageController();
		ImageControllers[CurrentFrameIndex]->SetAreaController(this);
		FrameImage = ImageControllers[CurrentFrameIndex]->GetImage();
		G42PathBegin * begin = new G42PathBegin();
		FrameImage->Process(begin, this);
		FrameImage->Process(desc, this);
	}
	else if (dynamic_cast<G42AnimatedImageFrame *>(object))
	{
		if (FrameImage)
		{
			G42AnimatedImageFrame * frame =
				dynamic_cast<G42AnimatedImageFrame *>(object);
			frame = new G42AnimatedImageFrame(*frame);
			FrameImage->Process(frame, this);
		}
		G42AreaSingleController::Process(object, sender);
	}
	else if (dynamic_cast<G42ImageData *>(object))
	{
		if (FrameImage)
		{
			G42ImageData * data =
				dynamic_cast<G42ImageData *>(object);
			data = data->MakeCopy();
			FrameImage->Process(data, this);
		}
		G42AreaSingleController::Process(object, sender);
	}
	else if (dynamic_cast<G42PathEnd *>(object))
	{
		G42AreaSingleController::Process(object, sender);
		if (FrameImage) // clean out old path image
		{
			FinishFrame();
#if 0
			G42PathEnd * end = new G42PathEnd();
			FrameImage->Process(end, this);
			ResetGrowth();
			ResetThumbLocation(CurrentFrameIndex);
			ImageControllers[CurrentFrameIndex]->GetDisplay()->SetAutoDraw(false);
			G42Path path(*(ImageControllers[CurrentFrameIndex]->GetImage()));
			path.AddLink(*(ImageControllers[CurrentFrameIndex]->GetImage()),
				*(ImageControllers[CurrentFrameIndex]->GetDisplay()));
			path.Start();
			path.Clear();
			ImageControllers[CurrentFrameIndex]->GetDisplay()->SetAutoDraw(true);
			char * buf = new char [10];
			sprintf(buf, "%d", CurrentFrameIndex + 1);
			StringArray[CurrentFrameIndex] = buf;
			Selected[CurrentFrameIndex] = false;
			G42Point translate(ScaleTranslate.GetTranslate());
			CumlativeScaleTranslate.Convert(translate);
			G42ScaleTranslate image_trans(
				CumlativeScaleTranslate.GetScaleX() * ScaleTranslate.GetScaleX(),
				CumlativeScaleTranslate.GetScaleY() * ScaleTranslate.GetScaleY(),
				translate);
			ImageControllers[CurrentFrameIndex]->SetCumlativeScaleTranslate(
				image_trans);
			FrameImage = 0;
#endif
		}
		// setup info needed to run animation
		ResetThumbsAcross();
		Invalidate();
	}
	else
	{
		G42AreaSingleController::Process(object, sender);
	}
}
bool
G42AreaAnimatedController::GetDrawAnimated(void)
{
	if (DrawThumbs)
	{
		return false;
	}
	return G42AreaSingleController::GetDrawAnimated();
}
void
G42AreaAnimatedController::SetDrawAnimated(bool state)
{
	DrawThumbs = false;
	G42AreaSingleController::SetDrawAnimated(state);
}
void
G42AreaAnimatedController::SetThumbnailMode(bool thumbnail_mode)
{
	G42AreaSingleController::SetDrawAnimated(false);
	G42MultiAreaBaseController::SetThumbnailMode(thumbnail_mode);
}
void
G42AreaAnimatedController::Play(void)
{
	DrawThumbs = false;
	G42AreaSingleController::Play();
}
void
G42AreaAnimatedController::FinishFrame(void)
{
	G42PathEnd * end = new G42PathEnd();
	FrameImage->Process(end, this);
	FrameImage->ClearPath();
	FrameImage = 0;
	ResetGrowth();
	ResetThumbLocation(CurrentFrameIndex);
	ImageControllers[CurrentFrameIndex]->GetDisplay()->SetAutoDraw(false);
	G42Path path(*(ImageControllers[CurrentFrameIndex]->GetImage()));
	path.AddLink(*(ImageControllers[CurrentFrameIndex]->GetImage()),
		*(ImageControllers[CurrentFrameIndex]->GetDisplay()));
	path.Start();
	path.Clear();
	ImageControllers[CurrentFrameIndex]->GetDisplay()->SetAutoDraw(true);
	char * buf = new char [10];
	sprintf(buf, "%d", CurrentFrameIndex + 1);
	StringArray[CurrentFrameIndex] = buf;
	Selected[CurrentFrameIndex] = false;
	G42Point translate(ScaleTranslate.GetTranslate());
	CumlativeScaleTranslate.Convert(translate);
	G42ScaleTranslate image_trans(
		CumlativeScaleTranslate.GetScaleX() * ScaleTranslate.GetScaleX(),
		CumlativeScaleTranslate.GetScaleY() * ScaleTranslate.GetScaleY(),
		translate);
	ImageControllers[CurrentFrameIndex]->SetCumlativeScaleTranslate(
		image_trans);
}

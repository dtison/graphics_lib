// gicasing.cpp - G42AreaSingleController
#include "gicasing.h"
#include "gicsur.h"
#include "gpath.h"
#include "gtools.h"
G42AreaSingleController::G42AreaSingleController(G42Notifier * notifier)
:
	G42AreaController(notifier), PathImage(0), ClearModified(false),
	BlockDrawImage(false)
{
	ImageController.Set(new G42ImageController);
	ImageController->SetAreaController(this);
}
G42AreaSingleController::~G42AreaSingleController(void)
{
	Duplicator = 0;
}
G42Image *
G42AreaSingleController::GetImage(void)
{
	return ImageController->GetImage();
}
G42Display *
G42AreaSingleController::GetDisplay(void)
{
	return ImageController->GetDisplay();
}
void
G42AreaSingleController::Draw(G42Rect & rect, G42DrawLocation location,
	bool image_only)
{
	if (ImageController->GetImage()->IsEmpty())
	{
		return;
	}
	if (ImageController->GetNeedRebuild())
	{
		Rebuild(rect, location, image_only);
		return;
	}
	DrawBackground(rect, location);
	if (BlockDrawImage)
	{
		return;
	}
	ImageController->GetDisplay()->Draw(rect, location);
	if (ControlManager && !image_only)
	{
		ControlManager->Draw(location);
	}
}
void
G42AreaSingleController::Rebuild(G42Rect & rect, G42DrawLocation location,
	bool image_only)
{
	if (ImageController->GetImage()->IsEmpty() || PathImage)
	{
		return;
	}
	DrawBackground(rect, location);
	if (BlockDrawImage)
	{
		return;
	}
	ImageController->GetDisplay()->SetAutoDraw(false);
	G42Path path(*(ImageController->GetImage()));
	G42PathComponent * source = ImageController->GetImage();
	for (uintf i = 0; i < NumFilters; i++)
	{
		path.AddLink(*source, *Filters[i]);
		source = Filters[i];
	}
	path.AddLink(*source,
		*(ImageController->GetDisplay()));
	path.Start();
	ImageController->GetDisplay()->SetAutoDraw(true);
	ImageController->GetDisplay()->Draw(rect, location);
	if (ControlManager && !image_only)
	{
		ControlManager->Draw(location);
	}
	ImageController->SetNeedRebuild(false);
}
void
G42AreaSingleController::SetImageScaleTranslate(void)
{
	G42Point translate(ScaleTranslate.GetTranslate());
	CumlativeScaleTranslate.Convert(translate);
	translate += G42Point(Margin.Left, Margin.Top);
	if (ShouldDrawBorder)
	{
		translate += G42Point(1, 1);
	}
	G42ScaleTranslate image_trans(
		CumlativeScaleTranslate.GetScaleX() * ScaleTranslate.GetScaleX(),
		CumlativeScaleTranslate.GetScaleY() * ScaleTranslate.GetScaleY(),
		translate);
	if (ImageController)
		ImageController->SetCumlativeScaleTranslate(image_trans);
}
void
G42AreaSingleController::SetArea(void)
{
	if (ImageController)
	{
		Area = ImageController->GetArea();
	}
	else
	{
		Area.Left = 0;
		Area.Right = 0;
		Area.Top = 0;
		Area.Bottom = 0;
	}
	G42AreaController::SetArea();
}
void
G42AreaSingleController::SetSurfaceController(G42SurfaceController * surface)
{
	G42AreaController::SetSurfaceController(surface);
	if (SurfaceController)
	{
		ImageController->SetSurface(SurfaceController->GetSurface());
		ImageController->SetSurfaceController(SurfaceController);
	}
}
void
G42AreaSingleController::SetViewport(const G42Rect & viewport)
{
	G42AreaController::SetViewport(viewport);
	if (ImageController)
	{
		ImageController->GetDisplay()->SetViewport(viewport);
	}
}
void
G42AreaSingleController::ResetImageFilters(void)
{
	if (ImageController)
	{
		ImageController->ClearFilters();
		for (uintf i = 0; i < NumFilters; i++)
		{
			ImageController->AppendFilter(*Filters[i]);
		}
	}
}
void
G42AreaSingleController::BeginProcessing(void)
{
	SetupPath();
	PathImage->BeginProcessing();
	if (!Sender) // don't clear until loop finishes
	{
//		PathImage = 0;
		ClearPath();
	}
	IsHead = false; // don't clear until now for Process
}
void
G42AreaSingleController::Process(G42PathObject * object, G42PathComponent *)
{
	SetupPath();
	bool is_end = (typeid(*object) == typeid(G42PathEnd));
	bool need_init = PathImage->IsEmpty();
	if (Duplicator)
	{
		Duplicator->Process(object, this);
	}
	else
	{
		PathImage->Process(object, this);
	}
	if (need_init && !PathImage->IsEmpty())
	{
		NeedImageInfoChangedMessage = true;
		SetImageScaleTranslate();
		if (AutoDraw)
		{
//			DrawBackground(Area, 0);
			BlockDrawImage = true;
			Invalidate(Area);
			BlockDrawImage = false;
		}
	}
	else if (!need_init && PathImage->IsEmpty() && AutoDraw)
	{
		Invalidate(Area);
	}
	if (is_end)
	{
		if (PathImage->IsEmpty())
		{
			SetImageScaleTranslate();
		}
		if (IsHead) // don't clear out in case BeginProcessing hasn't been called yet
		{
			Sender = 0;
		}
		else
		{
			ClearPath();
//			PathImage = 0;
		}
		if (!AutoDraw)
		{
			G42CanvasNeedsRedrawn msg(SurfaceController);
			Notifier->ProcessMessage(&msg);
		}
	}
}
void
G42AreaSingleController::ClearPath(bool path_excepted)
{
	if (PathImage)
	{
		if (ClearModified)
		{
			PathImage->SetModified(false);
			ClearModified = false;
		}
		PathImage->ClearPath(path_excepted);
		PathImage = 0;
	}
	if (Duplicator)
	{
		Duplicator->ClearPath(path_excepted);
		Duplicator = 0;
	}
	G42PathComponent::ClearPath(path_excepted);
	if (NeedImageInfoChangedMessage)
	{
		G42ImageInfoChanged msg(this);
		Notifier->ProcessMessage(&msg);
		NeedImageInfoChangedMessage = false;
	}
}
void
G42AreaSingleController::SetupPath(void)
{
	if (!PathImage)
	{
		NeedImageInfoChangedMessage = false;
		PathImage = ImageController->GetImage();
		AddComponentToPath(PathImage);
		ClearModified = false;
		if (Sender && (!Receiver || !IsHead)) // only happens on open or new
		{
			if (ControlManager)
			{
				ControlManager->ClearController();
			}
			ClearModified = true;
		}
		if (Receiver)
		{
			PathImage->SetReceiver(Receiver);
			Receiver->ReplaceSender(this, PathImage);
			Receiver = 0;
		}
		if (IsHead)
		{
			PathImage->SetHead();
		}
		if (Sender)
		{
#if !defined(MACOS)
			if (AutoDraw)
			{
				Duplicator.Set(new G42ImageStreamDuplicator());
				AddComponentToPath(Duplicator);
				Duplicator->SetSender(this);
//				Sender->ReplaceReceiver(this, Duplicator);
//				Sender = 0;
				Duplicator->SetReceiver(PathImage);
				PathImage->SetSender(Duplicator);
				if (NumFilters == 0)
				{
					Duplicator->SetReceiver(ImageController->GetDisplay());
					ImageController->GetDisplay()->SetSender(Duplicator);
				}
				else
				{
					G42PathComponent * current = Duplicator;
					for (uintf i = 0; i < NumFilters; i++)
					{
						AddComponentToPath(Filters[i]);
						current->SetReceiver(Filters[i]);
						Filters[i]->SetSender(current);
						current = Filters[i];
					}
					ImageController->GetDisplay()->SetSender(current);
					current->SetReceiver(ImageController->GetDisplay());
				}
			}
			else
#endif
			{
				PathImage->SetSender(this);
//				Sender->ReplaceReceiver(this, PathImage);
//				Sender = 0;
				ImageController->SetNeedRebuild();
			}
		}
	}
}
void
G42AreaSingleController::Play(void)
{
	ImageController->GetDisplay()->SetAnimatedDraw();
	SetArea();
	Invalidate();
	ImageController->GetDisplay()->Play();
}
void
G42AreaSingleController::Pause(void)
{
	ImageController->GetDisplay()->Pause();
}
void
G42AreaSingleController::Stop(void)
{
	ImageController->GetDisplay()->Stop();
	if (ControlManager)
	{
		ControlManager->ResetDrawState();
	}
}
void
G42AreaSingleController::Forward(void)
{
	ImageController->GetDisplay()->Forward();
}
void
G42AreaSingleController::Backward(void)
{
	ImageController->GetDisplay()->Backward();
}
void
G42AreaSingleController::Rewind(void)
{
	ImageController->GetDisplay()->Rewind();
}
void
G42AreaSingleController::GoToEnd(void)
{
	ImageController->GetDisplay()->GoToEnd();
}
bool
G42AreaSingleController::IsPaused(void)
{
	return ImageController->GetDisplay()->IsPaused();
}
bool
G42AreaSingleController::IsPlaying(void)
{
	return ImageController->GetDisplay()->IsPlaying();
}
bool
G42AreaSingleController::GetDrawAnimated(void)
{
	return ImageController->GetDisplay()->GetAnimatedDraw();
}
void
G42AreaSingleController::SetDrawAnimated(bool state)
{
	ImageController->GetDisplay()->SetAnimatedDraw(state);
}

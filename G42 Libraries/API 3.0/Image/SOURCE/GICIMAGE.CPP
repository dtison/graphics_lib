// gicimage.cpp - G42ImageController
#include "gicimage.h"
#include "gicarea.h"
G42ImageController::G42ImageController()
:
	AreaController(0), Area(0, 0, 0, 0), NeedRebuild(false), NumFilters(0)
{
	try
	{
		Image.Set(new G42Image());
		Display.Set(new G42Display());
	}
	catch (...)
	{
		Image.Clear();
		Display.Clear();
		throw;
	}
}
G42ImageController::~G42ImageController()
{
}
void
G42ImageController::SetAreaController(G42AreaController * area_controller)
{
	AreaController = area_controller;
	Display->SetSurface(AreaController->GetSurface());
}
void
G42ImageController::SetSurface(G42Surface surface)
{
	Display->SetSurface(surface);
}
void
G42ImageController::SetSurfaceController(G42SurfaceController * surface)
{
	Display->SetSurfaceController(surface);
}
void
G42ImageController::SetScaleTranslate(const G42ScaleTranslate & trans)
{
	ScaleTranslate = trans;
	SetDisplayScaleTranslate();
}
void
G42ImageController::SetCumlativeScaleTranslate(G42ScaleTranslate & trans)
{
	CumlativeScaleTranslate = trans;
	SetDisplayScaleTranslate();
}
void
G42ImageController::SetDisplayScaleTranslate(void)
{
	if (Image->IsEmpty())
		return;
		
	G42Point translate(ScaleTranslate.GetTranslate());
	CumlativeScaleTranslate.Convert(translate);
	translate += G42Point(Margin.Left, Margin.Top);
	G42ScaleTranslate display_trans(
		CumlativeScaleTranslate.GetScaleX() * ScaleTranslate.GetScaleX(),
		CumlativeScaleTranslate.GetScaleY() * ScaleTranslate.GetScaleY(),
		translate);
	if (Display.NotNull())
		Display->SetScaleTranslate(display_trans);
	G42Point upper_left(0, 0);
	display_trans.Convert(upper_left);
	G42Point lower_right(Image->GetInfo().GetWidth(),
		Image->GetInfo().GetHeight());
	if (Image->HasMultiInfo())
	{
		const G42MultipleImageDescriptor * mdesc = &(Image->GetMultiInfo());
		if (dynamic_cast<const G42AnimatedImageDescriptor *>(mdesc))
		{
			const G42AnimatedImageDescriptor * adesc =
				dynamic_cast<const G42AnimatedImageDescriptor *>(mdesc);
			lower_right.X = adesc->GetWidth();
			lower_right.Y = adesc->GetHeight();
		}
	}
	display_trans.Convert(lower_right);
	Area.Set(upper_left.X, upper_left.Y, lower_right.X - 1, lower_right.Y - 1);
	AreaController->SetArea();
}
void
G42ImageController::InsertFilter(const G42ImageFilter & filter, uintf index)
{
	for (uintf i = NumFilters; i > index; i--)
	{
		Filters[i].Set(Filters[i - 1].Release());
	}
	NumFilters++;
	Filters[index].Set(filter.MakeCopy());
}
void
G42ImageController::RemoveFilter(uintf index)
{
	Filters[index].Clear();
	NumFilters--;
	for (uintf i = index; i < NumFilters; i++)
	{
		Filters[i].Set(Filters[i + 1].Release());
	}
}
void
G42ImageController::ClearFilters(void)
{
	Filters.Clear();
	NumFilters = 0;
}

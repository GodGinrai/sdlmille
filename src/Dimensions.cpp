#include "Dimensions.h"

double	Dimensions::ScaleFactor;
int		Dimensions::EffectiveTableauHeight,
		Dimensions::FirstRowY,
		Dimensions::PadLeft,
		Dimensions::SecondRowY,
		Dimensions::ScreenHeight,
		Dimensions::ScreenWidth,
		Dimensions::TableauHeight;
bool	Dimensions::MultiRowSafeties;

void	Dimensions::SetDimensions	(int Width, int Height)
{
	ScreenHeight = Height;
	ScreenWidth = Width;

	if (ScreenWidth < 320)
		exit(-1);
	else
	{
		if (ScreenWidth < 480)
			ScaleFactor = 1;
		else
			ScaleFactor = 1.5;
	}
		
	PadLeft = (ScreenWidth - (320 * ScaleFactor)) / 2;

	TableauHeight = (ScreenHeight - 130) / (ScaleFactor * 2);
	EffectiveTableauHeight = TableauHeight * ScaleFactor;

	FirstRowY = (TableauHeight * 2) + 8;
	SecondRowY = FirstRowY + 62;

	if (TableauHeight >= 175)
		MultiRowSafeties = true;
	else
		MultiRowSafeties = false;
}
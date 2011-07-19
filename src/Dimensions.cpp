/*
This file is part of SDL Mille.

SDL Mille is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

SDL Mille is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SDL Mille.  If not, see <http://www.gnu.org/licenses/>.

(See file LICENSE for full text of license)
*/

#include "Dimensions.h"

namespace _SDLMille
{

double	Dimensions::ScaleFactor;
int		Dimensions::EffectiveTableauHeight,
		Dimensions::FirstRowY,
		Dimensions::GamePlayCardHeight,
		Dimensions::GamePlayCardSpacingX,
		Dimensions::GamePlayCardSpacingY,
		Dimensions::GamePlayCardWidth,
		Dimensions::GamePlayHandLeftX,
		Dimensions::MenuBorderPadding,
		Dimensions::MenuColumn1X,
		Dimensions::MenuColumn2X,
		Dimensions::MenuItemSpacing,
		Dimensions::MenuItemsTopY,
		Dimensions::MenuX,
		Dimensions::MenuY,
		Dimensions::PadLeft,
		Dimensions::SecondRowY,
		Dimensions::ScreenHeight,
		Dimensions::ScreenWidth,
		Dimensions::TableauHeight;
bool	Dimensions::MultiRowSafeties,
		Dimensions::GamePlayMultiRowTray;

void	Dimensions::SetDimensions	(int Width, int Height, int CardWidth, int CardHeight)
{
	ScreenHeight = Height;
	ScreenWidth = Width;

	//if (ScreenWidth < 320)
	//	exit(-1);
	//else
	//{
	//	if (ScreenWidth < 480)
			ScaleFactor = 1;
	//	else
	//		ScaleFactor = 1.5;
	//}
		
	PadLeft = (ScreenWidth - (320 * ScaleFactor)) / 2;

	TableauHeight = (ScreenHeight - (130 * ScaleFactor)) / (ScaleFactor * 2);
	EffectiveTableauHeight = TableauHeight * ScaleFactor;

	FirstRowY = (TableauHeight * 2) + 8;
	SecondRowY = FirstRowY + 62;

	if (TableauHeight >= 175)
		MultiRowSafeties = true;
	else
		MultiRowSafeties = false;

	GamePlayCardHeight = CardHeight;
	GamePlayCardWidth = CardWidth;
	
	SetGamePlayMetrics(CardWidth, CardHeight);
}

void	Dimensions::SetGamePlayMetrics	(int CardWidth, int CardHeight)
{
	//TODO: Remove parameters?

	int	CardsPerRow;

	GamePlayCardSpacingX = CardWidth * .586;
	GamePlayCardSpacingY = CardWidth * .088;

	if ((CardWidth > 0) && ((CardWidth * 13) <= ScreenWidth))
	{
		GamePlayMultiRowTray = false;
		CardsPerRow = 7;	//TODO: No literal
		SecondRowY = FirstRowY = ScreenHeight - TRAY_TOP_BOTTOM_PADDING - CardHeight;
	}
	else
	{
		GamePlayMultiRowTray = true;
		CardsPerRow = 4;	//TODO: No literal
		SecondRowY = ScreenHeight - TRAY_TOP_BOTTOM_PADDING - CardHeight;
		FirstRowY = SecondRowY - GamePlayCardSpacingY - CardHeight;
	}

	GamePlayHandLeftX = ScreenWidth - SCREEN_EDGE_PADDING - (CardWidth * CardsPerRow) - (GamePlayCardSpacingX * (CardsPerRow - 1));
}

void	Dimensions::SetMenuMetrics	(Surface &MenuSurface)
{
	MenuX = (ScreenWidth - MenuSurface.GetWidth()) / 2;
	MenuY = (ScreenHeight - MenuSurface.GetHeight()) / 2;

	MenuBorderPadding = MenuSurface.GetWidth() / 30;

	MenuColumn1X = MenuX + MenuBorderPadding;
	MenuColumn2X = MenuX + MenuSurface.GetWidth() * 0.8;
	
	MenuItemSpacing = MenuSurface.GetHeight() * 0.12;
	MenuItemsTopY = MenuY + (MenuSurface.GetHeight() * 0.18);
}

}
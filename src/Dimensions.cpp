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
		Dimensions::GamePlayCardsPerRow,
		Dimensions::GamePlayCardWidth,
		Dimensions::GamePlayHandLeftX,
		Dimensions::GamePlayTableauWidth,
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
		Dimensions::TableauBattleX,
		Dimensions::TableauHeight,
		Dimensions::TableauLimitX,
		Dimensions::TableauPileSpacing,
		Dimensions::TableauSpacingX,
		Dimensions::TableauSpacingY,
		Dimensions::TrayDiscardX,
		Dimensions::TrayDiscardY,
		Dimensions::TrayDrawX,
		Dimensions::TrayDrawY;
bool	Dimensions::LandscapeMode,
		Dimensions::MultiRowSafeties,
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
		
	//PadLeft = (ScreenWidth - (320 * ScaleFactor)) / 2;

	//TableauHeight = (ScreenHeight - (130 * ScaleFactor)) / (ScaleFactor * 2);
	//EffectiveTableauHeight = TableauHeight * ScaleFactor;

	//FirstRowY = (TableauHeight * 2) + 8;
	//SecondRowY = FirstRowY + 62;

	GamePlayCardHeight = CardHeight;
	GamePlayCardWidth = CardWidth;
	
	SetGamePlayMetrics(CardWidth, CardHeight);

	if (LandscapeMode)
		TableauHeight = ScreenHeight / 2;
	else
		TableauHeight = (FirstRowY - TRAY_TOP_BOTTOM_PADDING) / 2;

	EffectiveTableauHeight = TableauHeight;

	if (TableauHeight >= 175)
		MultiRowSafeties = true;
	else
		MultiRowSafeties = false;

	SetTableauMetrics();
}

void	Dimensions::SetGamePlayMetrics	(int CardWidth, int CardHeight)
{
	//TODO: Remove parameters?

	int	CardsPerRow;

	GamePlayCardHeight = CardHeight;
	GamePlayCardWidth = CardWidth;

	GamePlayCardSpacingX = CardWidth * .586;
	GamePlayCardSpacingY = CardHeight * .088;

	//if (ScreenWidth >= ScreenHeight)
	//	LandscapeMode = true;
	//else
		LandscapeMode = false;

	if ((CardWidth > 0) && ((CardWidth * 11) <= ScreenWidth))
	{
		if (LandscapeMode)
		{
			if (((CardHeight + GamePlayCardSpacingY) * 9) + (TRAY_TOP_BOTTOM_PADDING << 1) + CardHeight <= ScreenHeight)
			{
				GamePlayMultiRowTray = false;
				FirstRowY = ScreenHeight - TRAY_TOP_BOTTOM_PADDING - (CardHeight * 7) - (GamePlayCardSpacingY * 6); //TODO: literals
			}
			else if (((CardHeight + GamePlayCardSpacingY) << 2) + TRAY_TOP_BOTTOM_PADDING + CardHeight <= ScreenHeight)
			{
				GamePlayMultiRowTray = true;
				FirstRowY = ScreenHeight - TRAY_TOP_BOTTOM_PADDING - (CardHeight << 2) - (GamePlayCardSpacingY * 3);
			}
			else
				LandscapeMode = false;

			printf("Needed height: %u\n", ((CardHeight + GamePlayCardSpacingY) << 2) + (TRAY_TOP_BOTTOM_PADDING << 1) + CardHeight);
		}
		if (!LandscapeMode)
		{			
			GamePlayMultiRowTray = false;
			SecondRowY = FirstRowY = ScreenHeight - TRAY_TOP_BOTTOM_PADDING - CardHeight;
		}
	}
	else
	{
		LandscapeMode = false;
		GamePlayMultiRowTray = true;
		SecondRowY = ScreenHeight - TRAY_TOP_BOTTOM_PADDING - CardHeight;
		FirstRowY = SecondRowY - GamePlayCardSpacingY - CardHeight;
	}

	if (GamePlayMultiRowTray)
	{
		if (LandscapeMode)
			CardsPerRow = 2;
		else
			CardsPerRow = 4;	//TODO: No literal
	}
	else if (LandscapeMode)
		CardsPerRow = 1;
	else
		CardsPerRow = 7;	//TODO: No literal

	GamePlayCardsPerRow = CardsPerRow;

	GamePlayHandLeftX = ScreenWidth - SCREEN_EDGE_PADDING - (CardWidth * CardsPerRow) - (GamePlayCardSpacingX * (CardsPerRow - 1));

	if (LandscapeMode)
	{
		GamePlayTableauWidth = GamePlayHandLeftX - SCREEN_EDGE_PADDING;

		TrayDiscardX = GamePlayHandLeftX;

		if (GamePlayMultiRowTray)
		{
			TrayDrawX = TrayDiscardX + CardWidth + GamePlayCardSpacingX;
			TrayDrawY = std::max(SCREEN_EDGE_PADDING, FirstRowY - ((CardHeight + GamePlayCardSpacingY) << 1));
			TrayDiscardY = TrayDrawY;
		}
		else
		{
			TrayDrawX = TrayDiscardX;
			TrayDrawY = FirstRowY - ((CardHeight + GamePlayCardSpacingY) * 3);
			TrayDiscardY = TrayDrawY + CardHeight + GamePlayCardSpacingY;
		}
	}
	else
	{
		GamePlayTableauWidth = ScreenWidth;

		TrayDrawX = SCREEN_EDGE_PADDING;
		TrayDrawY = ScreenHeight - TRAY_TOP_BOTTOM_PADDING - CardHeight;

		if (GamePlayMultiRowTray)
		{
			TrayDiscardX = TrayDrawX;
			TrayDiscardY = TrayDrawY - CardHeight - GamePlayCardSpacingY;
		}
		else
		{
			TrayDiscardX = TrayDrawX + CardWidth + GamePlayCardSpacingX;
			TrayDiscardY = TrayDrawY;
		}
	}
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

void	Dimensions::SetTableauMetrics	(void)
{
	TableauSpacingX = (GamePlayTableauWidth - (GamePlayCardWidth * 7)) / 10;

	TableauLimitX = GamePlayTableauWidth - GamePlayCardWidth - (TableauSpacingX << 1);
	TableauBattleX = TableauLimitX - GamePlayCardWidth - (TableauSpacingX << 1);
}

}
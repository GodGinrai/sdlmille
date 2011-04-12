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

	TableauHeight = (ScreenHeight - (130 * ScaleFactor)) / (ScaleFactor * 2);
	EffectiveTableauHeight = TableauHeight * ScaleFactor;

	FirstRowY = (TableauHeight * 2) + 8;
	SecondRowY = FirstRowY + 62;

	if (TableauHeight >= 175)
		MultiRowSafeties = true;
	else
		MultiRowSafeties = false;
}

}
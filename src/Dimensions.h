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

#ifndef	_SDLMILLE_DIMENSIONS_H
#define	_SDLMILLE_DIMENSIONS_H

#include <stdlib.h>

namespace _SDLMille
{

class	Dimensions
{
public:
	static	void	SetDimensions	(int Width, int Height);

	static	double	ScaleFactor;
	static	int		EffectiveTableauHeight,
					FirstRowY,
					PadLeft,
					SecondRowY,
					ScreenWidth,
					ScreenHeight,
					TableauHeight;
	static	bool	MultiRowSafeties;
};

}

#endif

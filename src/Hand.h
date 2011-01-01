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

#ifndef _SDLMILLE_HAND_H
#define _SDLMILLE_HAND_H

#include "Card.h"

namespace	_SDLMille
{

const Uint8 HAND_SIZE = 7;

class Hand
{
public:
				Hand		(void);
				~Hand		(void)							{}
	void		Detach		(Uint8 Index);
	bool		Discard		(Uint8 Index);
	bool		Draw		(Deck * Source, Uint8 Index);
	Uint8		GetType		(Uint8 Index)								const;
	Uint8		GetValue	(Uint8 Index)								const;
	bool		IsDirty		(void)										const;
	bool		IsPopped	(Uint8 Index)								const;
	void		OnInit		(void);
	bool		OnRender	(SDL_Surface * Target, bool Force = false);
	void		Pop			(Uint8 Index);
	void		Reset		(void);
	bool		Restore		(std::ifstream &SaveFile);
	bool		Save		(std::ofstream &SaveFile);
	void		UnPop		(Uint8 Index);
private:
	Card		ThisHand[HAND_SIZE];
	bool		Detached[HAND_SIZE],
				Dirty,
				Popped[HAND_SIZE];
	Surface		CancelSurface,
				CardSurfaces[HAND_SIZE],				
				OrbSurface,
				Overlay;
};

inline	bool	Hand::IsDirty	(void)	const
{
	return Dirty;
}

}

#endif

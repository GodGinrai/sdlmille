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

#ifndef _SDLMILLE_TABLEAU_H
#define	_SDLMILLE_TABLEAU_H

#include "Card.h"

namespace _SDLMille
{

const Uint8	SAFETY_COUNT = 4,
			MILEAGE_PILES =	5,	// Number of mileage stacks
			MAX_PILE_SIZE =	12, // Max size of any pile
			MAX_CARD_COUNT[MILEAGE_PILES] = {10, 10, 10, 12, 2};	/* Max size of each pile, broken down
				individually. This represents the total number of each card in the entire deck, with the
				exception of 200's. Each player is limited to playing a maximum of two 200-mile cards	*/

const int	TABLEAU_TOO_MANY_CARDS = 5000;

class Tableau
{
public:
				Tableau			(void);
				~Tableau		(void);
	Uint8		Get200Count		(void)															const;
	int			GetMileage		(void)															const;
	Uint8		GetTopCard		(bool SpeedPile = false)										const;
	bool		HasCoupFourre	(Uint8 Value)													const;
	bool		HasSafety		(Uint8 Value)													const;
	bool		HasSpeedLimit	(void)															const;
	bool		IsRolling		(void)															const;
	void		OnInit			(void);
	bool		OnRender		(SDL_Surface * Target, Uint8 PlayerIndex, bool Force = false);
	void		OnPlay			(Uint8 Value, bool CoupFourre, bool SpeedLimit);
	void		Reset			(void);
private:
	/* Methods */
	void		SetTopCard		(Uint8 Value);

	/* Properties */
	Surface		BattleSurface,
				LimitSurface,
				MileageTextSurface,
				PileSurfaces[MILEAGE_PILES][MAX_PILE_SIZE],
				SafetySurfaces[SAFETY_COUNT];
	Uint8		CardCount[MILEAGE_PILES],
				LimitCard, OldLimitCard,
				TopCard, OldTopCard;
	bool		CoupFourres[SAFETY_COUNT],
				Dirty,
				Safeties[SAFETY_COUNT];
	Uint32		Mileage;
	TTF_Font	*MyFont;
};

inline	Uint8	Tableau::Get200Count	(void)	const
{
	return CardCount[MILEAGE_PILES - 1];
}

inline int		Tableau::GetMileage		(void)	const
{
	return Mileage;
}

}

#endif

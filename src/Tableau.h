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
			MAX_PILE_SIZE =	10, // Max size of any pile
			MAX_CARD_COUNT[MILEAGE_PILES] = {10, 10, 10, 10, 2};	/* Max size of each pile, broken down
				individually. This represents the total number of each card in the entire deck, with the
				exception of 100's and 200's. Each player is limited to playing a maximum of two 200-mile cards.
				There are 12 cards with a value of 100, but it is impossible to play more than 10 (1,000 miles) */

const int	BattleX = 220,
			LimitX = 265,
			TABLEAU_TOO_MANY_CARDS = 5000;

enum		{STATUS_ROLLING, STATUS_LIMITED, STATUS_STOPPED};

class Tableau
{
public:
				Tableau			(void);
				~Tableau		(void);
	void		BlitWithShadow	(Surface &CardSurface, int X, int Y, SDL_Surface *Target, bool CoupFourre = false);
	void		FadeIn			(Uint8 PlayerIndex, SDL_Surface *Target);
	Uint8		Get200Count		(void)															const;
	int			GetMileage		(void)															const;
	Uint8		GetTopCard		(bool SpeedPile = false)										const;
	bool		HasCoupFourre	(Uint8 Value)													const;
	bool		HasSafety		(Uint8 Value)													const;
	bool		HasSpeedLimit	(void)															const;
	bool		IsDirty			(void)															const;
	bool		IsRolling		(void)															const;
	void		OnInit			(void);
	bool		OnRender		(SDL_Surface * Target, Uint8 PlayerIndex, bool Force = false);
	void		OnPlay			(Uint8 Value, bool CoupFourre, bool SpeedLimit);
	void		Reset			(void);
	bool		Restore			(std::ifstream &SaveFile);
	bool		Save			(std::ofstream &SaveFile);
private:
	/* Methods */
	void		SetTopCard		(Uint8 Value);

	/* Properties */
	Surface		BattleSurface,
				LimitSurface,
				MileageTextSurface,
				PileSurfaces[MILEAGE_PILES][MAX_PILE_SIZE],
				SafetySurfaces[SAFETY_COUNT],

				ShadowSurface,
				ShadowSurfaceCF;
	Uint8		CardCount[MILEAGE_PILES],
				LimitCard, OldLimitCard,
				TopCard, OldTopCard;
	bool		CoupFourres[SAFETY_COUNT],
				Dirty,
				FadeRunning,
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

#ifndef _SDLMILLE_TABLEAU_H
#define	_SDLMILLE_TABLEAU_H

#include "Card.h"

namespace _SDLMille
{

const Uint8	SAFETY_COUNT = 4,
			MILEAGE_PILES =	5,	// Number of mileage stacks
			MAX_PILE_SIZE =	12, // Max size of any pile
			MAX_CARD_COUNT[MILEAGE_PILES] = {10, 10, 10, 12, 2}; // Max size of each
				// pile, broken down individually. This represents the total number
				// of each card in the entire deck, with the exception of 200's. Each
				// player is limited to playing a maximum of two 200-mile cards

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
	bool		OnRender		(SDL_Surface * Surface, Uint8 PlayerIndex, bool Force = false);
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
				TopCard, OldTopCard,
				LimitCard, OldLimitCard;
	bool		Safeties[SAFETY_COUNT],
				CoupFourres[SAFETY_COUNT],
				Dirty;
	Uint32		Mileage;
	TTF_Font	*MyFont;
};

inline int	Tableau::GetMileage		(void)			const
{
	return Mileage;
}

inline bool	Tableau::HasSpeedLimit	(void)			const
{
	if (Safeties[CARD_SAFETY_RIGHT_OF_WAY - SAFETY_OFFSET])
		return false;

	return (LimitCard == CARD_HAZARD_SPEED_LIMIT);
}

}

#endif

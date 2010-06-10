#ifndef _SDLMILLE_CARD_H
#define	_SDLMILLE_CARD_H

#include "Deck.h"

namespace _SDLMille
{

const int	CARD_VALUE_INVALID	= 1000;

const Uint8	SAFETY_OFFSET = 10, // Used to convert from an index to a value
			MILEAGE_OFFSET = 14;

class Card
{
public:
							Card				(void);
							Card				(const Card & Source);
							Card				(Uint8 V);
	const	Card &			Copy				(const Card & Source);
			bool			Discard				(void);
			bool			Draw				(Deck * Source);
	static	SDL_Surface	*	GetImageFromValue	(Uint8 ArgValue, bool CoupFourre = false);
	static	Uint8			GetMatchingSafety	(Uint8 HazardValue);
	static	Uint8			GetMileValue		(Uint8 ArgValue);
			Uint8			GetType				(void);
	static	Uint8			GetTypeFromValue	(Uint8 ArgValue);
			Uint8			GetValue			(void);
private:
			void			Set					(Uint8 V);
			Uint8			Value;
			Uint8			Type;
};

}

#endif
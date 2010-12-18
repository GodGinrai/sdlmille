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
							Card				(Uint8 ArgValue);
	const	Card &			Copy				(const Card & Source);
			bool			Discard				(void);
			bool			Draw				(Deck * Source);
	static	const char *	GetFileFromValue	(Uint8 ArgValue, bool CoupFourre = false);
	static	Uint8			GetMatchingSafety	(Uint8 HazardValue);
	static	Uint8			GetMileValue		(Uint8 ArgValue);
			Uint8			GetType				(void)									const;
	static	Uint8			GetTypeFromValue	(Uint8 ArgValue);
			Uint8			GetValue			(void)									const;
			bool			Restore				(std::ifstream &SaveFile);
			bool			Save				(std::ofstream &SaveFile);
private:
			void			Set					(Uint8 ArgValue);

			Uint8			Value;
			Uint8			Type;
};

inline	Uint8	Card::GetType	(void)	const
{
	return Type;
}

inline	Uint8	Card::GetValue	(void)	const
{
	return Value;
}

}

#endif

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

#include "Deck.h"

namespace _SDLMille
{

		Deck::Deck		(void)
{
	Shuffle();
}

int		Deck::CardsLeft	(void)	const
{
	return (DECK_SIZE - Marker);
}

Uint8	Deck::Draw		(void)
{
	Uint8	RetVal = CARD_NULL_NULL;
	if (Marker < DECK_SIZE)
	{
		RetVal = STD_DECK[Order[Marker]];
		++Marker;
	}
	return RetVal;
}

bool	Deck::Empty		(void)	const
{
	return !(Marker < DECK_SIZE);
}

void		Deck::Shuffle	(void)
{
	//TODO: Benchmark this vs. using a vector with its built-in shuffling method.

	//Reset marker
	Marker = 0;

	//Clear sort order (by making each value invalid)
	for (int i = 0; i < DECK_SIZE; ++i)
		Order[i] = DECK_SIZE;

	srand(time(NULL));

	//Generate dealing order

	for (int i = 0; i < DECK_SIZE; ++i)
	{
		Uint8	RandIndex;
		bool	Assigned =	false;

		/*	The for loop gives us each value stored in STD_DECK. We then assign that
			value to a random index in the Order array, if that index is free. */
		do
		{
			RandIndex = rand() % DECK_SIZE;
			Assigned = true;
			if (Order[RandIndex] == DECK_SIZE)
				Order[RandIndex] = i;
			else if (Order[RandIndex] > DECK_SIZE)
				exit(DECK_ERROR);
			else
				Assigned = false;
		} while (Assigned == false);
	}
}

}

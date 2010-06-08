#include "Deck.h"

namespace _SDLMille
{

		Deck::Deck		(void)
{
	Shuffle();
}

int		Deck::CardsLeft	(void)
{
	return (DECK_SIZE - Marker);
}

Uint8	Deck::Draw		(void)
{
	Uint8	RetVal = CARD_NULL_NULL;
	//std::cout << static_cast<int>(Marker) << '\n';
	if (Marker < DECK_SIZE)
	{
		RetVal = STD_DECK[Order[Marker]];
		//Available[Marker] = false;
		++Marker;
	}
	return RetVal;
}

bool	Deck::Empty		(void)
{
	return !(Marker < DECK_SIZE);
}

void		Deck::Shuffle	(void)
{
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
				throw DECK_ERROR;
			else
				Assigned = false;
		} while (Assigned == false);
	}
}

}
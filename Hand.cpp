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

#include "Hand.h"

namespace _SDLMille
{

		Hand::Hand		(void)
{
	Dirty = false;

	for (int i = 0; i < HAND_SIZE; ++i)
	{
		CardSurfaces[i] = 0;
		Popped[i] = false;
	}

	Overlay = 0;
}

		Hand::~Hand(void)
{
	if (Overlay)
		SDL_FreeSurface(Overlay);

	for (int i = 0; i < HAND_SIZE; ++i)
	{
		if (CardSurfaces[i])
			SDL_FreeSurface(CardSurfaces[i]);
	}

}

bool	Hand::Discard	(Uint8 Index)
{
	if (Index < HAND_SIZE)
	{
		Dirty = true;
		return ThisHand[Index].Discard();
	}

	return false;
}

bool	Hand::Draw		(Deck * Source, Uint8 Index)
{
	if (Index < HAND_SIZE)
	{
		if (ThisHand[Index].Draw(Source))
		{
			Dirty = true;
			return true;
		}
	}

	return false;
}

Uint8	Hand::GetType	(Uint8 Index)
{
	if (Index < HAND_SIZE)
		return ThisHand[Index].GetType();

	return CARD_NULL;
}

Uint8	Hand::GetValue	(Uint8 Index)
{
	if (Index < HAND_SIZE)
		return ThisHand[Index].GetValue();

	return CARD_NULL_NULL;
}

bool	Hand::IsPopped	(Uint8 Index)
{
	if (Index < HAND_SIZE)
		return Popped[Index];

	return false;
}

void	Hand::OnInit	(void)
{
	if (!Overlay)
		Overlay = Surface::Load("orb.png");

	for (int i = 0; i < HAND_SIZE; ++i)
	{
		if (CardSurfaces[i])
		{
			SDL_FreeSurface(CardSurfaces[i]);
			CardSurfaces[i] = 0;
		}
		CardSurfaces[i] = Card::GetImageFromValue(ThisHand[i].GetValue());
	}
}

bool	Hand::OnRender	(SDL_Surface * Surface, bool Force)
{
	if (Dirty || Force)
	{
		OnInit();
		Dirty = false;

		for (int i = 0; i < HAND_SIZE; ++i)
		{
			/*	The next line is a fancy (and more efficient) way of multiplying i times 65 and then
				adding 81. 65 is the horizontal distance between two cards, and 81 is the left edge
				of our render. Because this uses the bit shift operator to do multiplication, it will
				only work on little-endian architectures. Reverse the shift operator for big-endian. */
			int	X = ((i << 6) + i + 81),
				Y =	0;

			if (i < 3)
			{
				// The first three cards get a right-shift one space. Y is set for the top row.
				X += 65;
				Y = 358;
			}

			else
			{
				// The remaining cards get a left-shift three spaces (to account for the three cards
				// above). Y is set for the second row.
				X -= (65 * 3);
				Y = 420;
			}

			if (CardSurfaces[i])
			{
				// Draw the cards
				Surface::Draw(Surface, CardSurfaces[i], X, Y);

				if (Popped[i] && Overlay)
					// If this card is popped, render the orb over it
					Surface::Draw(Surface, Overlay, X, Y + 8);
			}
		}

		return true;
	}

	return false;
}

bool	Hand::Pop		(Uint8 Index)
{
	if (Index < HAND_SIZE)
	{
		if (!Popped[Index]) // If it's not already popped
		{
			// Unpop any other cards
			for (int i = 0; i < HAND_SIZE; ++i)
			{
				if (Popped[i] == true)
				{
					Popped[i] = false;
					Dirty = true;
				}
			}

			if (ThisHand[Index].GetType() != CARD_NULL) // This isn't an empty slot
			{
				// Pop it
				Popped[Index] = true;
				Dirty = true;
			}
		}
	}

	// TODO: We are always returning true, and we really don't need to return anything.
	// Unfortunately, it's bools all the way up the call chain, so I'm going to wait until
	// after the audit to change these all to voids.
	return true;
}

void	Hand::UnPop		(Uint8 Index)
{
	if (Index < HAND_SIZE)
	{
		Popped[Index] = false;
		Dirty = true;
	}
}

}
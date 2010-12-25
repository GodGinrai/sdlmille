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
		Popped[i] = false;
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

Uint8	Hand::GetType	(Uint8 Index)						const
{
	if (Index < HAND_SIZE)
		return ThisHand[Index].GetType();

	return CARD_NULL;
}

Uint8	Hand::GetValue	(Uint8 Index)						const
{
	if (Index < HAND_SIZE)
		return ThisHand[Index].GetValue();

	return CARD_NULL_NULL;
}

bool	Hand::IsPopped	(Uint8 Index)						const
{
	if (Index < HAND_SIZE)
		return Popped[Index];

	return false;
}

void	Hand::OnInit	(void)
{
	OrbSurface.SetImage("gfx/orb.png");
	CancelSurface.SetImage("gfx/cancel.png");
	Overlay.SetImage("gfx/overlays/game_play_2.png");

	for (int i = 0; i < HAND_SIZE; ++i)
		CardSurfaces[i].SetImage(Card::GetFileFromValue(ThisHand[i].GetValue()));
}

bool	Hand::OnRender	(SDL_Surface * Target, bool Force)
{
	bool	WasDirty = Dirty;
	
	if (Target != 0)
	{
		if (Dirty || Force)
		{
			if (Dirty)
			{
				OnInit();
				Dirty = false;
			}

			Overlay.Render(0, Dimensions::TableauHeight * 2, Target);

			for (int i = 0; i < (HAND_SIZE + 1); ++i)
			{
				/*	The next line is a fancy (and more efficient) way of multiplying i times 65 and then
					adding 81. 65 is the horizontal distance between two cards, and 81 is the left edge
					of our render. */
				int	X = ((i << 6) + i + 81),
					Y =	0;

				if (i < 4)
					// The first three cards get a right-shift one space. Y is set for the top row.
					//X += 65;
					Y = Dimensions::FirstRowY;
				else
				{
					// The remaining cards get a left-shift four spaces (to account for the four spaces
					// above). Y is set for the second row.
					X -= (65 << 2);
					Y = Dimensions::SecondRowY;
				}

				if (i == 0)
					CancelSurface.Render(X, Y, Target);
				else
				{
					int	Index = i - 1;
					if (CardSurfaces[Index])
					{
						CardSurfaces[Index].Render(X, Y, Target);	//Draw the cards

						if (Popped[Index] && OrbSurface)	//If this card is popped, render the orb over it
							OrbSurface.Render(X, Y + 8, Target);
					}
				}
			}
		}
	}

	return WasDirty;
}

void	Hand::Pop		(Uint8 Index)
{
	if (Index < HAND_SIZE)
	{
		if (!Popped[Index]) // If it's not already popped
		{
			for (int i = 0; i < HAND_SIZE; ++i)	//Unpop any other cards
			{
				if (Popped[i] == true)
				{
					Popped[i] = false;
					Dirty = true;
				}
			}

			if (ThisHand[Index].GetType() != CARD_NULL) // This isn't an empty slot
			{
				Popped[Index] = true;	//Pop it
				Dirty = true;
			}
		}
	}
}

void	Hand::Reset		(void)
{
	for (int i = 0; i < HAND_SIZE; ++i)
	{
		ThisHand[i].Discard();
		Popped[i] = false;
	}

	Dirty = true;
}

bool	Hand::Restore	(std::ifstream &SaveFile)
{
	bool	Success = true;
	for (int i = 0; i < HAND_SIZE; ++i)
		Success &= ThisHand[i].Restore(SaveFile);

	return Success;
}

bool	Hand::Save		(std::ofstream &SaveFile)
{
	bool	Success = true;
	for (int i = 0; i < HAND_SIZE; ++i)
		Success &= ThisHand[i].Save(SaveFile);

	return Success;
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

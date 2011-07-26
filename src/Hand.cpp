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
		Detached[i] = false;
		Popped[i] = false;
	}
}

void	Hand::Detach	(Uint8 Index)
{
	if (Index < HAND_SIZE)
	{
		if (Detached[Index])
			return;
		
		for (int i = 0; i < HAND_SIZE; ++i)
			Detached[i] = false;

		Detached[Index] = true;

		Dirty = true;
	}
}

bool	Hand::Discard	(Uint8 Index)
{
	if (Index < HAND_SIZE)
	{
		Dirty = true;

		Detached[Index] = false;
		Popped[Index] = false;

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

Uint8	Hand::GetIndex			(int X, int Y)
{
	Uint8	Invalid = 0xFF;

	int ColCount = Dimensions::GamePlayCardsPerRow,
		RowCount = ceil((double) HAND_SIZE / Dimensions::GamePlayCardsPerRow),
		MinX = Dimensions::GamePlayHandLeftX,
		MaxX = MinX + (ColCount * Dimensions::GamePlayCardWidth) + ((ColCount - 1) * Dimensions::GamePlayCardSpacingX),
		MinY = Dimensions::FirstRowY,
		MaxY = MinY + (RowCount * Dimensions::GamePlayCardHeight) + ((RowCount - 1) * Dimensions::GamePlayCardSpacingY),
		XIncrement = Dimensions::GamePlayCardWidth + Dimensions::GamePlayCardSpacingX,
		YIncrement = Dimensions::GamePlayCardHeight + Dimensions::GamePlayCardSpacingY;

	if ((Y >= MinY) && (Y <= MaxY))
	{
		Uint8	RowIndex = 0,
				Index = 0;

		//if (Y >= Dimensions::SecondRowY)	// Clicked the bottom row. Add 4 to the index
		//	Add = 4;
		//else if (Y >= (Dimensions::FirstRowY + 57))	//Clicked in the dead zone
		//	return Invalid;

		if ((Y - MinY) % YIncrement < Dimensions::GamePlayCardHeight)
			RowIndex = (Y - MinY) / YIncrement;
		else
			return Invalid;

		if ((X >= MinX) && (X <= MaxX))	//Clicked within hand
		{
			if (((X - MinX) % XIncrement) < Dimensions::GamePlayCardWidth)
			{
				int Index = (((X - MinX) / XIncrement) + (RowIndex * Dimensions::GamePlayCardsPerRow));

				if (Dimensions::GamePlayMultiRowTray)
					--Index;

				if (Index < HAND_SIZE)
					return Index;
			}
		}
	}

	return Invalid;
}

void	Hand::GetIndexCoords	(Uint8 Index, int &X, int &Y)
{
	if (Index < 3)
	{
		X = 81 + (65 * (Index + 1));
		Y = Dimensions::FirstRowY;
	}
	else
	{
		X = 81 + (65 * (Index - 3));
		Y = Dimensions::SecondRowY;
	}
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
	{
		if (Detached[i])
			CardSurfaces[i].SetImage(Card::GetFileFromValue(CARD_NULL_NULL));
		else
			CardSurfaces[i].SetImage(Card::GetFileFromValue(ThisHand[i].GetValue()));
	}
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

			//Overlay.Render(0, Dimensions::EffectiveTableauHeight * 2, Target, SCALE_NONE);

			int	XIncrement = Dimensions::GamePlayCardWidth + Dimensions::GamePlayCardSpacingX,
				YIncrement = Dimensions::GamePlayCardHeight + Dimensions::GamePlayCardSpacingY;

			for (int i = 0; i < HAND_SIZE; ++i)
			{
				/*	The next line is a fancy (and more efficient) way of multiplying i times 65 and then
					adding 81. 65 is the horizontal distance between two cards, and 81 is the left edge
					of our render. */

				int	IndexVal = (Dimensions::GamePlayMultiRowTray) ? i + 1 : i;

				int RowNum = IndexVal / Dimensions::GamePlayCardsPerRow,
					ColNum = IndexVal % Dimensions::GamePlayCardsPerRow;

				int X = Dimensions::GamePlayHandLeftX + (ColNum * XIncrement),
					Y = Dimensions::FirstRowY + (RowNum * YIncrement);

				//int	X = Dimensions::GamePlayHandLeftX + (i * (Dimensions::GamePlayCardWidth + Dimensions::GamePlayCardSpacingX)),
				//	Y =	Dimensions::FirstRowY;

				//if ((i >= 4) && Dimensions::GamePlayMultiRowTray)
				//{
				//	// The remaining cards get a left-shift four spaces (to account for the four spaces
				//	// above). Y is set for the second row.
				//	X -= ((Dimensions::GamePlayCardWidth + Dimensions::GamePlayCardSpacingX) << 2);
				//	Y = Dimensions::SecondRowY;
				//}
				//else
				//{
				//	//TODO: Update comments
				//	// The first three cards get a right-shift one space. Y is set for the top row.
				//	//X += 65;
				//	Y = Dimensions::FirstRowY;
				//}

				//if (i == 0)
				//	CancelSurface.Render(X, Y, Target);
				//else
				//{
					//int	Index = i - 1;
				if (CardSurfaces[i])
				{
					CardSurfaces[i].Render(X, Y, Target);	//Draw the cards

					if (Popped[i] && OrbSurface && !Detached[i])	//If this card is popped, render the orb over it
						OrbSurface.Render(X, Y + (Dimensions::GamePlayCardHeight >> 1) - (OrbSurface.GetHeight() >> 1), Target);
				}
				//}
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
					Detached[i] = false;
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

bool	Hand::Restore	(FILE *SaveFile)
{
	bool	Success = true;

	if (SaveFile != 0)
	{
		for (int i = 0; i < HAND_SIZE; ++i)
			Success &= ThisHand[i].Restore(SaveFile);
	}
	else
		Success = false;

	return Success;
}

bool	Hand::Save		(FILE *SaveFile)
{
	bool	Success = true;

	if (SaveFile != 0)
	{
		for (int i = 0; i < HAND_SIZE; ++i)
			Success &= ThisHand[i].Save(SaveFile);

		return Success;
	}

	return false;
}

void	Hand::UnPop		(Uint8 Index)
{
	if (Index < HAND_SIZE)
	{
		Detached[Index] = false;
		Popped[Index] = false;
		Dirty = true;
	}
}

}

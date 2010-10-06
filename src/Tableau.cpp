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

#include "Tableau.h"

namespace _SDLMille
{

		Tableau::Tableau		(void)
{
	MyFont = 0;

	for (int i = 0; i < MILEAGE_PILES; ++i)
		CardCount[i] = 0;

	for (int i = 0; i < SAFETY_COUNT; ++i)
	{
		Safeties[i] = false;
		CoupFourres[i] = false;
	}
	
	Dirty = true;
	Mileage = 0;
	OldTopCard = TopCard = CARD_NULL_NULL;
	OldLimitCard = LimitCard = CARD_NULL_NULL;

	// Set up font
	if (!TTF_WasInit())
		TTF_Init();
	MyFont = TTF_OpenFont("LiberationMono-Regular.ttf", 24);
}

		Tableau::~Tableau		(void)
{
	if (MyFont)
		/*	TODO: Find a way to fix the following. SDL_ttf reuses pointers, so this will
			end up closing an invalid pointer (we have two tableaus using the same font).
			I think SDL_ttf handles this gracefully, but it does cause an access violation
			when run on Visual Studio in debug mode	*/
		TTF_CloseFont(MyFont);
}


Uint8	Tableau::GetTopCard		(bool SpeedPile)								const
{
	if (SpeedPile)
		return LimitCard;
	else
		return TopCard;
}

bool	Tableau::HasCoupFourre	(Uint8 Value)									const
{
	if (HasSafety(Value))
		return CoupFourres[Value - SAFETY_OFFSET];

	return false;
}

bool	Tableau::HasSafety		(Uint8 Value)									const
{

	if (Card::GetTypeFromValue(Value) == CARD_SAFETY)
		return Safeties[Value - SAFETY_OFFSET];

	return false;
}

bool	Tableau::HasSpeedLimit	(void)											const
{
	if (Safeties[CARD_SAFETY_RIGHT_OF_WAY - SAFETY_OFFSET])
		return false;

	return (LimitCard == CARD_HAZARD_SPEED_LIMIT);
}

bool	Tableau::IsRolling		(void)											const
{
	if (TopCard == CARD_REMEDY_ROLL)
		return true;

	if (HasSafety(CARD_SAFETY_RIGHT_OF_WAY))
	{
		if ((Card::GetTypeFromValue(TopCard) == CARD_REMEDY) || (TopCard == CARD_HAZARD_STOP) || (TopCard == CARD_NULL_NULL) || HasSafety(Card::GetMatchingSafety(TopCard)))
			return true;
	}

	return false;
}

void	Tableau::OnInit			(void)
{
	// Refresh our surfaces
	BattleSurface.SetImage(Card::GetFileFromValue(TopCard));
	LimitSurface.SetImage(Card::GetFileFromValue(LimitCard));

	for (int i = 0; i < MILEAGE_PILES; ++i)
	{
		if (CardCount[i])
		{
			for (int j = 0; j < CardCount[i]; ++j)
			{
				if (!PileSurfaces[i][j])
					PileSurfaces[i][j].SetImage(Card::GetFileFromValue(i + MILEAGE_OFFSET));
			}
		}
	}

	for (int i = 0; i < SAFETY_COUNT; ++i)
	{
		if (Safeties[i])
		{
			if (!SafetySurfaces[i])
				SafetySurfaces[i].SetImage(Card::GetFileFromValue(i + 10, CoupFourres[i]));
		}
	}

	if ((Mileage <= 1000) && MyFont)
		MileageTextSurface.SetInteger(Mileage, MyFont);	// Font is valid, mileage is sane
}

void	Tableau::OnPlay			(Uint8 Value, bool CoupFourre, bool SpeedLimit)
{
	Uint8	Type =	Card::GetTypeFromValue(Value),
			Index =	0xFF;

	Dirty = true;

	switch (Type)
	{
	case CARD_MILEAGE:
		Index = Value - MILEAGE_OFFSET;
		if ((Index >= 0) && (Index < MILEAGE_PILES))
		{
			if (CardCount[Index] < MAX_CARD_COUNT[Index])
			{
				++CardCount[Index];
				Mileage += Card::GetMileValue(Value);
			}
			else
				exit(TABLEAU_TOO_MANY_CARDS);
		}
		break;
	case CARD_HAZARD:
		if (Value == CARD_HAZARD_SPEED_LIMIT)
		{
			if (!HasSpeedLimit())
			{
				OldLimitCard = LimitCard;
				LimitCard = Value;
			}
		}
		else
		{
			if (IsRolling())
				SetTopCard(Value);
		}
		break;
	case CARD_REMEDY:
		if (Value == CARD_REMEDY_END_LIMIT)
		{
			if (HasSpeedLimit())
				LimitCard = Value;
		}
		else if (Value == CARD_REMEDY_ROLL)
		{
			if (!IsRolling())
				SetTopCard(Value);
		}
		else
		{
			if (Card::GetTypeFromValue(TopCard) == CARD_HAZARD)
				SetTopCard(Value);
		}
		break;
	case CARD_SAFETY:
		Index = Value - SAFETY_OFFSET;
		if (Index < SAFETY_COUNT)
		{
			Safeties[Index] = true;

			if (CoupFourre)
			{
				CoupFourres[Index] = true;

				// Throw away the hazard
				if (SpeedLimit)
					LimitCard = OldLimitCard;
				else
					TopCard = OldTopCard;
			}
		}
		break;
	}
}

bool	Tableau::OnRender		(SDL_Surface * Target, Uint8 PlayerIndex, bool Force)
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

			int	R = 0, G = 0, B = 0,
				Y = 1, RectY = 0, W = 320, H = 174;;

			if (PlayerIndex == 0)
			{
				Y += 175;
				RectY += 175;
			}

			#ifdef	ANDROID_DEVICE
				RectY *= 1.5;
				RectY += 40;
				W = 480;
				H = 261;
			#endif

			SDL_Rect	PlayerRect = {0, RectY, W, H}; // Tableau background

			// Color coding
			//TODO: Pre-fill rectangles because FillRect is slow.
			if (IsRolling())
			{
				if ((LimitCard == CARD_HAZARD_SPEED_LIMIT) && !HasSafety(CARD_SAFETY_RIGHT_OF_WAY))
					R = G = 191;
				else
				{
					R = 120; G = 192; B = 86;
				}
			}
			else
				R = 191;

			SDL_FillRect(Target, &PlayerRect, SDL_MapRGB(Target->format, R, G, B));

			// Draw our stuff
			for (int i = 0; i < MILEAGE_PILES; ++i)
			{
				for (int j = 0; j < MAX_PILE_SIZE; ++ j)
					PileSurfaces[i][j].Render((i * 42) + 2, Y + (j * 8), Target);
			}

			BattleSurface.Render(220, Y, Target);

			LimitSurface.Render(263, Y, Target);

			for (int i = 0; i < SAFETY_COUNT; ++i)
			{
				if (SafetySurfaces[i])
				{
					int YOffset = 58 + ((i < 2) ? 0 : 58);
					int X = 213 + ((i % 2) ? 0 : 58);

					if (CoupFourres[i])
					{
						YOffset += 8;
						X -= 8;
					}

					SafetySurfaces[i].Render(X, Y + YOffset, Target);
				}
			}

			MileageTextSurface.Render(65 - MileageTextSurface.GetWidth(), Y + 150, Target);
		}
	}

	return WasDirty;
}

void	Tableau::Reset			(void)
{
	for (int i = 0; i < MILEAGE_PILES; ++i)
	{
		for (int j = 0; j < MAX_PILE_SIZE; ++j)
			PileSurfaces[i][j].Clear();

		CardCount[i] = 0;
	}

	for (int i = 0; i < SAFETY_COUNT; ++i)
	{
		SafetySurfaces[i].Clear();
		CoupFourres[i] = Safeties[i] = false;
	}

	OldLimitCard = LimitCard = OldTopCard = TopCard = CARD_NULL_NULL;

	Dirty = true;

	Mileage = 0;
}

/* Private Methods */

void	Tableau::SetTopCard		(Uint8 Value)
{
	OldTopCard = TopCard;
	TopCard = Value;
}

}

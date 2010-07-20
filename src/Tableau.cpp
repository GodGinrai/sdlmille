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
	// Pointers
	BattleSurface = 0;
	LimitSurface = 0;
	MileageTextSurface = 0;
	MyFont = 0;

	for (int i = 0; i < MILEAGE_PILES; ++i)
	{
		CardCount[i] = 0;
		for (int j = 0; j < MAX_PILE_SIZE; ++j)
		{
			PileSurfaces[i][j] = 0;
		}
	}

	for (int i = 0; i < SAFETY_COUNT; ++i)
	{
		SafetySurfaces[i] = 0;
		Safeties[i] = false;
		CoupFourres[i] = false;
	}

	// Vars
	Dirty = false;
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
		// TODO: Find a way to fix the following. SDL_ttf reuses pointers, so this will
		// end up closing an invalid pointer (we have two tableaus using the same font).
		// I think SDL_ttf handles this gracefully, but it does cause an access violation
		// when run on Visual Studio in debug mode
		TTF_CloseFont(MyFont);


	if (BattleSurface)
		SDL_FreeSurface(BattleSurface);
	if (LimitSurface)
		SDL_FreeSurface(LimitSurface);
	if (MileageTextSurface)
		SDL_FreeSurface(MileageTextSurface);

	for (int i = 0; i < MILEAGE_PILES; ++i)
	{
		for (int j = 0; j < MAX_PILE_SIZE; ++j)
		{
			if(PileSurfaces[i][j])
				SDL_FreeSurface(PileSurfaces[i][j]);
		}
	}


	for (int i = 0; i < SAFETY_COUNT; ++i)
	{
		if (SafetySurfaces[i])
			SDL_FreeSurface(SafetySurfaces[i]);
	}

}

Uint8	Tableau::Get200Count	(void)
{
	return CardCount[MILEAGE_PILES - 1];
}

Uint8	Tableau::GetTopCard		(bool SpeedPile)
{
	if (SpeedPile)
		return LimitCard;
	else
		return TopCard;
}

bool	Tableau::HasCoupFourre	(Uint8 Value)
{
	int Index = -1;

	if (HasSafety(Value))
		Index = Value - SAFETY_OFFSET;

	if ((Index >= 0) && (Index < SAFETY_COUNT))
		return CoupFourres[Index];

	return false;
}

bool	Tableau::HasSafety		(Uint8 Value)
{
	int Index = -1;

	if (Card::GetTypeFromValue(Value) == CARD_SAFETY)
		Index = Value - SAFETY_OFFSET;

	if ((Index >= 0) && (Index < SAFETY_COUNT))
		return Safeties[Index];

	return false;
}

bool	Tableau::IsRolling		(void)
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
	// Free our surfaces

	if (BattleSurface)
	{
		SDL_FreeSurface(BattleSurface);
		BattleSurface = 0;
	}

	if (LimitSurface)
	{
		SDL_FreeSurface(LimitSurface);
		LimitSurface = 0;
	}

	// Refresh our surfaces

	BattleSurface = Card::GetImageFromValue(TopCard);
	LimitSurface = Card::GetImageFromValue(LimitCard);

	for (int i = 0; i < MILEAGE_PILES; ++i)
	{
		if (CardCount[i])
		{
			for (int j = 0; j < CardCount[i]; ++j)
			{
				if (!PileSurfaces[i][j])
					PileSurfaces[i][j] = Card::GetImageFromValue(i + MILEAGE_OFFSET);
			}
		}
	}

	for (int i = 0; i < SAFETY_COUNT; ++i)
	{
		if (Safeties[i])
		{
			if (!SafetySurfaces[i])
				SafetySurfaces[i] = Card::GetImageFromValue(i + 10, CoupFourres[i]);
		}
	}

	if (MileageTextSurface)
	{
		SDL_FreeSurface(MileageTextSurface);
		MileageTextSurface = 0;
	}

	char	MileageText[5];

	if ((Mileage <= 1000) && MyFont)
	{
		// We have a font, and the mileage is not going to over-run our buffer
		sprintf(MileageText, "%4u", Mileage);
		MileageTextSurface = Surface::RenderText(MileageText, MyFont);
	}

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
				#ifndef	ANDROID_DEVICE
					throw TABLEAU_TOO_MANY_CARDS;
				#else
					exit(-1);
				#endif
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

bool	Tableau::OnRender		(SDL_Surface * Surface, Uint8 PlayerIndex, bool Force)
{
	if (Surface)
	{
		if (Dirty || Force)
		{
			OnInit();
			Dirty = false;

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

			SDL_FillRect(Surface, &PlayerRect, SDL_MapRGB(Surface->format, R, G, B));

			// Draw our stuff
			for (int i = 0; i < MILEAGE_PILES; ++i)
			{
				for (int j = 0; j < MAX_PILE_SIZE; ++ j)
				{
					if (PileSurfaces[i][j])
					{
						Surface::Draw(Surface, PileSurfaces[i][j], (i * 42) + 2, Y + (j * 8));
					}
				}
			}

			if (BattleSurface)
				Surface::Draw(Surface, BattleSurface, 220, Y);

			if (LimitSurface)
				Surface::Draw(Surface, LimitSurface, 263, Y);

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

					Surface::Draw(Surface, SafetySurfaces[i], X, Y + YOffset);
				}
			}

			if (MileageTextSurface)
				Surface::Draw(Surface, MileageTextSurface, 10, Y + 150);

			return true;
		}
	}

	return false;
}

void	Tableau::Reset			(void)
{
	for (int i = 0; i < MILEAGE_PILES; ++i)
	{
		for (int j = 0; j < MAX_PILE_SIZE; ++j)
		{
			if (PileSurfaces[i][j])
			{
				SDL_FreeSurface(PileSurfaces[i][j]);
				PileSurfaces[i][j] = 0;
			}
		}

		CardCount[i] = 0;
	}

	for (int i = 0; i < SAFETY_COUNT; ++i)
	{
		if (SafetySurfaces[i])
		{
			SDL_FreeSurface(SafetySurfaces[i]);
			SafetySurfaces[i] = 0;
		}

		CoupFourres[i] = Safeties[i] = false;
	}

	OldLimitCard = LimitCard = OldTopCard = TopCard = CARD_NULL_NULL;

	Dirty = true;

	Mileage = 0;
}

/* Private */

void	Tableau::SetTopCard		(Uint8 Value)
{
	OldTopCard = TopCard;
	TopCard = Value;
}

}

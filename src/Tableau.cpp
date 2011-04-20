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

Surface		Tableau::MileageSurfaces[MILEAGE_PILES],
			Tableau::ShadowSurface,
			Tableau::ShadowSurfaceCF;
TTF_Font	*Tableau::MyFont;
Uint32		Tableau::LastAnimationBlit;
bool		Tableau::EnableAnimation;

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
	
	//Animating = false;
	Dirty = true;
	EnableAnimation = true;
	Mileage = 0;
	OldTopCard = TopCard = CARD_NULL_NULL;
	OldLimitCard = LimitCard = CARD_NULL_NULL;
}

		Tableau::~Tableau		(void)
{
	if (MyFont != 0)
	{
		TTF_CloseFont(MyFont);
		MyFont = 0;
	}
}

/*
void	Tableau::Animate		(Uint8 PlayerIndex, SDL_Surface *Target)
{
	return;
	EnableAnimation = false;

	static	bool	BattleArea = false,
					LimitArea = false;

	static	Surface	RollCard,
					EndLimit;

			int		AreaTop = 1;

	static	int		i = 0;
	static	int		SafetyX,
					SafetyY,
					RollX,
					RollY,
					EndLimitX,
					EndLimitY;

	#ifdef	SOFTWARE_MODE
			int		Divisor = 35;
	#else
			int		Divisor = 70;
	#endif

	if (PlayerIndex == 0)
		AreaTop += Dimensions::TableauHeight;

	if (!Animating)
	{
		Animating = true;
		i = 0;
		LastAnimationBlit = 0;

		if (IsRolling() && HasSafety(CARD_SAFETY_RIGHT_OF_WAY) && (TopCard != CARD_REMEDY_ROLL))
			BattleArea = true;
		if (LimitCard == CARD_HAZARD_SPEED_LIMIT)
			LimitArea = true;

		RollCard.SetImage("gfx/remedy_roll.png");
		EndLimit.SetImage("gfx/remedy_end_limit.png");

		SafetyX = (Dimensions::MultiRowSafeties) ? 213 : 271;
		SafetyY = (Dimensions::MultiRowSafeties) ? 117 : 76;
		if (PlayerIndex == 0)
			SafetyY += Dimensions::TableauHeight;

		EndLimitX = RollX = SafetyX;
		EndLimitY = RollY = SafetyY;
	}
	else
	{
		if ((Card::GetTypeFromValue(TopCard) == CARD_HAZARD) && (TopCard != CARD_HAZARD_STOP))
		{
			BattleArea = false;
			RollX = BattleX;
		}
	}

	if (!EnableAnimation)
		RollY = AreaTop - 1;

	if (RollY >= AreaTop)
	{
		int	Y = 1;

		if (PlayerIndex == 0)
			Y += Dimensions::TableauHeight;

		if (BattleArea)
			RollCard.Render(RollX, RollY, Target);
		if (LimitArea)
			EndLimit.Render(EndLimitX, EndLimitY, Target);

		if ((LastAnimationBlit + 15) <= SDL_GetTicks())
		{
			LastAnimationBlit = SDL_GetTicks();
			RollX = SafetyX - (((SafetyX - BattleX) * i) / Divisor);
			RollY = SafetyY - (((SafetyY - AreaTop) * i) / Divisor);
			EndLimitX = SafetyX - (((SafetyX - LimitX) * i) / Divisor);
			EndLimitY = RollY;

			++i;
		}
	}
	else
	{
		if (BattleArea)
		{
			BattleArea = false;
			SetTopCard(CARD_REMEDY_ROLL);
			RollCard.Render(BattleX, AreaTop, Target);
		}
		if (LimitArea)
		{
			LimitArea = false;
			LimitCard = CARD_REMEDY_END_LIMIT;
			EndLimit.Render(LimitX, AreaTop, Target);
		}


		Animating = false;
	}

	Dirty = true;
}
*/

void		Tableau::BlitWithShadow	(Surface &CardSurface, int X, int Y, SDL_Surface *Target, bool CoupFourre)
{
	if (CardSurface)
	{
		if (CoupFourre)
			ShadowSurfaceCF.Render(X, Y, Target);
		else
			ShadowSurface.Render(X, Y, Target);

		CardSurface.Render(X, Y, Target);
	}
}

Uint8	Tableau::GetPileCount		(Uint8 Value)														const
{
	if ((Value >= CARD_MILEAGE_25) && (Value <= CARD_MILEAGE_200))
		return CardCount[Value - MILEAGE_OFFSET];

	return 0;
}

void	Tableau::GetTargetCoords	(Uint8 Value, Uint8 PlayerIndex, int &X, int &Y, bool CoupFourre, Uint8 PileCount)
{
	Uint8 Type = Card::GetTypeFromValue(Value);

	if (Type < CARD_NULL)
	{
		Y = 1;
		if (PlayerIndex == 0)
			Y += Dimensions::TableauHeight;

		if ((Type == CARD_HAZARD) || (Type == CARD_REMEDY))
		{
			if ((Value == CARD_HAZARD_SPEED_LIMIT) || (Value == CARD_REMEDY_END_LIMIT))
				X = LimitX;
			else
				X = BattleX;
		}
		else if (Type == CARD_MILEAGE)
		{
			X = (Value - MILEAGE_OFFSET) * 42 + 2;
			if (PileCount > 0)
				Y += PileCount * 8;
		}
		else if (Type == CARD_SAFETY)
		{
			if (Dimensions::MultiRowSafeties)
			{
				X = 213;
				Y += 58;
				if ((Value - SAFETY_OFFSET) > 1)
					Y += 58;
				if ((Value - SAFETY_OFFSET) % 2 == 0)
				X += 58;
			}
			else
			{
				Y += 75;
				X = 97 + ((Value - SAFETY_OFFSET) * 58);
			}

			if (CoupFourre)
			{
				Y += 8;
				X -= 8;
			}
		}
	}
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

bool	Tableau::IsDirty		(void)											const
{
	return Dirty;
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
	if (MyFont == 0)
	{
		// Set up font
		if (!TTF_WasInit())
			TTF_Init();

		MyFont = TTF_OpenFont("LiberationMono-Regular.ttf", 24);
	}

	// Refresh our surfaces
	Backdrop.SetImage("gfx/overlays/tableau.png");
	BattleSurface.SetImage(Card::GetFileFromValue(TopCard));
	LimitSurface.SetImage(Card::GetFileFromValue(LimitCard));

	ShadowSurface.SetImage("gfx/card_shadow.png");
	ShadowSurfaceCF.SetImage("gfx/card_shadow_cf.png");

	if (IsRolling())
	{
		if (HasSpeedLimit())
			Backdrop.SetImage("gfx/overlays/tableau_yellow.png");
		else
			Backdrop.SetImage("gfx/overlays/tableau_green.png");
	}
	else
		Backdrop.SetImage("gfx/overlays/tableau_red.png");

	for (int i = 0; i < MILEAGE_PILES; ++i)
		MileageSurfaces[i].SetImage(Card::GetFileFromValue(i + MILEAGE_OFFSET));

	for (int i = 0; i < SAFETY_COUNT; ++i)
	{
		if (Safeties[i])
		{
			if (!SafetySurfaces[i])
				SafetySurfaces[i].SetImage(Card::GetFileFromValue(i + SAFETY_OFFSET, CoupFourres[i]));
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
			SDL_Rect	PlayerRect =	{0, 0, Dimensions::ScreenWidth, Dimensions::EffectiveTableauHeight - 1};
	static	bool		RectSetUp =		false;
	static	Uint8		LastStatus =	0xFF;
			bool		WasDirty =		Dirty;

	//if ((Card::GetTypeFromValue(TopCard) == CARD_HAZARD) && (TopCard != CARD_HAZARD_STOP) && HasSafety(Card::GetMatchingSafety(TopCard)))
	//	SetTopCard(TopCard + 5);

	if (Target != 0)
	{
		if (Dirty || Force)
		{
			if (Dirty)
			{
				OnInit();
				Dirty = false;
			}

			int	R, G = 0, B = 0,
				X, Y = 1, SafetyY;

			if (PlayerIndex == 0)
			{
				Y += Dimensions::TableauHeight;
				PlayerRect.y += Dimensions::EffectiveTableauHeight;
			}
			
			///* Color-coding */
			//Uint8	Status = STATUS_STOPPED;

			//if (IsRolling())
			//{
			//	if (HasSpeedLimit())
			//		Status = STATUS_LIMITED;
			//	else
			//		Status = STATUS_ROLLING;
			//}

			//if (Status != LastStatus)
			//{
			//	switch(Status)
			//	{
			//	case	STATUS_ROLLING:
			//		R = 120; G = 192; B = 86; break;
			//	case	STATUS_LIMITED:
			//		R = G = 191; break;
			//	default:
			//		R = 191;
			//	}

			//	Backdrop.SetRGBALoss(0, 255, 255, 0);

			//	SDL_FillRect(Target, &PlayerRect, SDL_MapRGB(Target->format, R, G, B));
			//}

			Backdrop.Render(0, Y - 1, Target);

			// Draw our stuff
			for (int i = 0; i < MILEAGE_PILES; ++i)
			{
				if (CardCount[i])
				{
					for (int j = 0; j < CardCount[i]; ++j)
						BlitWithShadow(MileageSurfaces[i], (i * 42) + 2, Y + (j * 8), Target);
				}
				//for (int j = 0; j < MAX_PILE_SIZE; ++ j)
				//	BlitWithShadow(PileSurfaces[i][j], (i * 42) + 2, Y + (j * 8), Target);
			}

			BlitWithShadow(BattleSurface, BattleX, Y, Target);

			BlitWithShadow(LimitSurface, LimitX, Y, Target);

			for (int i = 0; i < SAFETY_COUNT; ++i)
			{
				if (SafetySurfaces[i])
				{
					bool CoupFourre = CoupFourres[i];
					GetTargetCoords(i + SAFETY_OFFSET, PlayerIndex, X, SafetyY, CoupFourre);
					BlitWithShadow(SafetySurfaces[i], X, SafetyY, Target, CoupFourre);
				}
			}

			MileageTextSurface.Render(65 - MileageTextSurface.GetWidth(), Y + (Dimensions::TableauHeight - 25), Target, SCALE_Y);
		}
	}

	//if (Animating)
	//	Animate(PlayerIndex, Target);
	//else if (IsRolling() && HasSafety(CARD_SAFETY_RIGHT_OF_WAY) && (TopCard != CARD_REMEDY_ROLL))
	//	Animate(PlayerIndex, Target);
	//else if ((LimitCard == CARD_HAZARD_SPEED_LIMIT) && (HasSafety(CARD_SAFETY_RIGHT_OF_WAY)))
	//	Animate(PlayerIndex, Target);

	return WasDirty;
}

void	Tableau::Reset			(void)
{
	for (int i = 0; i < MILEAGE_PILES; ++i)
		CardCount[i] = 0;

	for (int i = 0; i < SAFETY_COUNT; ++i)
	{
		SafetySurfaces[i].Clear();
		CoupFourres[i] = Safeties[i] = false;
	}

	OldLimitCard = LimitCard = OldTopCard = TopCard = CARD_NULL_NULL;

	Mileage = 0;

	//Animating = false;

	Dirty = true;
}

bool	Tableau::Restore		(FILE *SaveFile)
{
	if (SaveFile != 0)
	{
		fread(CardCount, sizeof(Uint8), MILEAGE_PILES, SaveFile);
		fread(&LimitCard, sizeof(Uint8), 1, SaveFile);
		fread(&OldLimitCard, sizeof(Uint8), 1, SaveFile);
		fread(&TopCard, sizeof(Uint8), 1, SaveFile);
		fread(&OldTopCard, sizeof(Uint8), 1, SaveFile);

		fread(CoupFourres, sizeof(bool), SAFETY_COUNT, SaveFile);
		fread(Safeties, sizeof(bool), SAFETY_COUNT, SaveFile);

		fread(&Mileage, sizeof(Uint32), 1, SaveFile);

		return true;
	}

	return false;
}

bool	Tableau::Save			(FILE *SaveFile)
{
	if (SaveFile != 0)
	{
		fwrite(CardCount, sizeof(Uint8), MILEAGE_PILES, SaveFile);
		fwrite(&LimitCard, sizeof(Uint8), 1, SaveFile);
		fwrite(&OldLimitCard, sizeof(Uint8), 1, SaveFile);
		fwrite(&TopCard, sizeof(Uint8), 1, SaveFile);
		fwrite(&OldTopCard, sizeof(Uint8), 1, SaveFile);

		fwrite(CoupFourres, sizeof(bool), SAFETY_COUNT, SaveFile);
		fwrite(Safeties, sizeof(bool), SAFETY_COUNT, SaveFile);

		fwrite(&Mileage, sizeof(Uint32), 1, SaveFile);

		return true;
	}

	return false;
}

void	Tableau::UpdateTopCard	(bool IncludeRollCard, bool IncludeEndLimit)
{
	Dirty = true;

	if ((Card::GetTypeFromValue(TopCard) == CARD_HAZARD) && (HasSafety(Card::GetMatchingSafety(TopCard))) && (TopCard != CARD_HAZARD_STOP))
		TopCard += 5;

	if (IncludeRollCard && IsRolling() && (TopCard != CARD_REMEDY_ROLL))
		TopCard = CARD_REMEDY_ROLL;

	if (IncludeEndLimit && (LimitCard == CARD_HAZARD_SPEED_LIMIT) && HasSafety(CARD_SAFETY_RIGHT_OF_WAY))
		LimitCard = CARD_REMEDY_END_LIMIT;
}

/* Private Methods */

void	Tableau::SetTopCard		(Uint8 Value)
{
	OldTopCard = TopCard;
	TopCard = Value;
}

}

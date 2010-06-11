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

#include "Card.h"

namespace _SDLMille
{


/* Public methods */


				Card::Card				(void)
{
	Set(CARD_NULL_NULL);
}

				Card::Card				(const Card & Source)
{
	Copy(Source);
}

				Card::Card				(Uint8 V)
{
	if (V <= CARD_NULL_NULL)
		Set(V);
	else
		throw CARD_VALUE_INVALID;
}

const Card &	Card::Copy				(const Card & Source)
{
	if (&Source != this)
	{
		Value =		Source.Value;
		Type =		Source.Type;
	}
	return *this;
}

bool			Card::Discard			(void)
{
	if (Value != CARD_NULL_NULL)
	{
		Set(CARD_NULL_NULL);
		return true;
	}
	return false;
}

bool			Card::Draw				(Deck * Source)
{
	if (Source)
	{
		Set(Source->Draw());
		return true;
	}
	else
		return false;
}

SDL_Surface *	Card::GetImageFromValue	(Uint8 ArgValue, bool CoupFourre)
{
	switch (ArgValue)
	{
	case CARD_HAZARD_ACCIDENT:
		return Surface::Load("gfx/hazard_accident.png");
	case CARD_HAZARD_GAS:
		return Surface::Load("gfx/hazard_gas.png");
	case CARD_HAZARD_FLAT:
		return Surface::Load("gfx/hazard_flat.png");
	case CARD_HAZARD_SPEED_LIMIT:
		return Surface::Load("gfx/hazard_speed_limit.png");
	case CARD_HAZARD_STOP:
		return Surface::Load("gfx/hazard_stop.png");
	case CARD_REMEDY_REPAIR:
		return Surface::Load("gfx/remedy_repair.png");
	case CARD_REMEDY_FUEL:
		return Surface::Load("gfx/remedy_fuel.png");
	case CARD_REMEDY_SPARE:
		return Surface::Load("gfx/remedy_spare.png");
	case CARD_REMEDY_END_LIMIT:
		return Surface::Load("gfx/remedy_end_limit.png");
	case CARD_REMEDY_ROLL:
		return Surface::Load("gfx/remedy_roll.png");
	case CARD_MILEAGE_25:
		return Surface::Load("gfx/mileage_25.png");
	case CARD_MILEAGE_50:
		return Surface::Load("gfx/mileage_50.png");
	case CARD_MILEAGE_75:
		return Surface::Load("gfx/mileage_75.png");
	case CARD_MILEAGE_100:
		return Surface::Load("gfx/mileage_100.png");
	case CARD_MILEAGE_200:
		return Surface::Load("gfx/mileage_200.png");
	case CARD_SAFETY_DRIVING_ACE:
		if (CoupFourre)
			return Surface::Load("gfx/safety_driving_ace_cf.png");
		else
			return Surface::Load("gfx/safety_driving_ace.png");
	case CARD_SAFETY_EXTRA_TANK:
		if (CoupFourre)
			return Surface::Load("gfx/safety_extra_tank_cf.png");
		else
			return Surface::Load("gfx/safety_extra_tank.png");
	case CARD_SAFETY_PUNCTURE_PROOF:
		if (CoupFourre)
			return Surface::Load("gfx/safety_puncture_proof_cf.png");
		else
			return Surface::Load("gfx/safety_puncture_proof.png");
	case CARD_SAFETY_RIGHT_OF_WAY:
		if (CoupFourre)
			return Surface::Load("gfx/safety_right_of_way_cf.png");
		else
			return Surface::Load("gfx/safety_right_of_way.png");
	case CARD_NULL_NULL:
	default:
		return Surface::Load("gfx/null_null.png");
	}
}

Uint8			Card::GetMatchingSafety	(Uint8 HazardValue)
{
	if (GetTypeFromValue(HazardValue) == CARD_HAZARD)
	{
		if (HazardValue == CARD_HAZARD_STOP)
			return HazardValue + SAFETY_OFFSET - 1;
		else
			return HazardValue + SAFETY_OFFSET;
	}
	else
		return 0xFF;
}

Uint8			Card::GetMileValue		(Uint8 ArgValue)
{
	if (GetTypeFromValue(ArgValue) == CARD_MILEAGE)
	{
		switch (ArgValue)
		{
		case	CARD_MILEAGE_25:
			return 25;
		case	CARD_MILEAGE_50:
			return 50;
		case	CARD_MILEAGE_75:
			return 75;
		case	CARD_MILEAGE_100:
			return 100;
		case	CARD_MILEAGE_200:
			return 200;
		}
	}
	return 0;
}

Uint8			Card::GetType			(void)
{
	return Type;
}

Uint8			Card::GetTypeFromValue	(Uint8 V)
{
	if (V <= CARD_HAZARD_STOP)
		return CARD_HAZARD;
	else if (V <= CARD_REMEDY_ROLL)
		return CARD_REMEDY;
	else if (V <= CARD_SAFETY_RIGHT_OF_WAY)
		return CARD_SAFETY;
	else if (V < CARD_NULL_NULL)
		return CARD_MILEAGE;
	else
		return CARD_NULL;
}

Uint8			Card::GetValue			(void)
{
	return Value;
}


/* Private methods */


void			Card::Set				(Uint8 V)
{
	if (V <= CARD_NULL_NULL)
	{
		Value = V;
		Type = GetTypeFromValue(Value);
	}
	else
		throw CARD_VALUE_INVALID;
}

}

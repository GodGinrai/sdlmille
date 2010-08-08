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
		exit(CARD_VALUE_INVALID);
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

const char *	Card::GetFileFromValue		(Uint8 ArgValue, bool CoupFourre)
{
	switch (ArgValue)
	{
	case CARD_HAZARD_ACCIDENT:
		return "gfx/hazard_accident.png";
	case CARD_HAZARD_GAS:
		return "gfx/hazard_gas.png";
	case CARD_HAZARD_FLAT:
		return "gfx/hazard_flat.png";
	case CARD_HAZARD_SPEED_LIMIT:
		return "gfx/hazard_speed_limit.png";
	case CARD_HAZARD_STOP:
		return "gfx/hazard_stop.png";
	case CARD_REMEDY_REPAIR:
		return "gfx/remedy_repair.png";
	case CARD_REMEDY_FUEL:
		return "gfx/remedy_fuel.png";
	case CARD_REMEDY_SPARE:
		return "gfx/remedy_spare.png";
	case CARD_REMEDY_END_LIMIT:
		return "gfx/remedy_end_limit.png";
	case CARD_REMEDY_ROLL:
		return "gfx/remedy_roll.png";
	case CARD_MILEAGE_25:
		return "gfx/mileage_25.png";
	case CARD_MILEAGE_50:
		return "gfx/mileage_50.png";
	case CARD_MILEAGE_75:
		return "gfx/mileage_75.png";
	case CARD_MILEAGE_100:
		return "gfx/mileage_100.png";
	case CARD_MILEAGE_200:
		return "gfx/mileage_200.png";
	case CARD_SAFETY_DRIVING_ACE:
		if (CoupFourre)
			return "gfx/safety_driving_ace_cf.png";
		else
			return "gfx/safety_driving_ace.png";
	case CARD_SAFETY_EXTRA_TANK:
		if (CoupFourre)
			return "gfx/safety_extra_tank_cf.png";
		else
			return "gfx/safety_extra_tank.png";
	case CARD_SAFETY_PUNCTURE_PROOF:
		if (CoupFourre)
			return "gfx/safety_puncture_proof_cf.png";
		else
			return "gfx/safety_puncture_proof.png";
	case CARD_SAFETY_RIGHT_OF_WAY:
		if (CoupFourre)
			return "gfx/safety_right_of_way_cf.png";
		else
			return "gfx/safety_right_of_way.png";
	case CARD_NULL_NULL:
	default:
		return "gfx/null_null.png";
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

Uint8			Card::GetType			(void)									const
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

Uint8			Card::GetValue			(void)									const
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
		exit(CARD_VALUE_INVALID);
}

}

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

#include "Player.h"

namespace _SDLMille
{

		Player::Player			(void)
{
	QualifiedCoupFourre = 0xFF;	//Player does not qualify for a Coup Fourre
	SourceDeck = 0;
}

bool	Player::Discard			(Uint8 Index)
{
	if (PlayerHand.Discard(Index))
	{
		QualifiedCoupFourre = 0xFF;	//Discarding disqualifies the player from any Coup Fourre
		return true;
	}

	return false;
}

void	Player::Draw			(void)
{
	if (SourceDeck != 0)
	{
		for (int i = 0; i < HAND_SIZE; ++i)
		{
			// If we find an empty slot, and we've got a draw pile to draw from,
			// draw one card and then return
			if (GetValue(i) == CARD_NULL_NULL && !SourceDeck->Empty())
			{
				PlayerHand.Draw(SourceDeck, i);
				return;
			}
		}
	}
}

bool	Player::Draw			(Uint8 Index)
{
	if (SourceDeck != 0)
		return PlayerHand.Draw(SourceDeck, Index);

	return false;
}

bool	Player::IsOutOfCards	(void)												const
{
	// If we find a populated card slot, immediately return false.
	for (int i = 0; i < HAND_SIZE; ++i)
	{
		if (GetType(i) != CARD_NULL)
			return false;
	}

	return true;	//We didn't find a populated slot
}

Uint8	Player::OnPlay			(Uint8 Index)
{
	Uint8	ReturnValue =	CARD_NULL_NULL;
	bool	CoupFourre =	false,
			SpeedLimit =	false;

	if ((GetType(Index) == CARD_SAFETY) && (GetValue(Index) == Card::GetMatchingSafety(QualifiedCoupFourre)))
	{
		// We're playing a safety, and its value is the matching safety to the
		// hazard that we just suffered. Therefore, this is a Coup Fourre.
		CoupFourre = true;

		/*	Speed limit is a control variable that determines whether we fetch the top
			of the battle pile or the speed pile. We need this so we can place the
			appropriate card on top of the discard pile. This is accomplished by returning
			the value of top card. */
		if (QualifiedCoupFourre == CARD_HAZARD_SPEED_LIMIT)
			SpeedLimit = true;

		ReturnValue = GetTopCard(SpeedLimit);
	}

	MyTableau.OnPlay(GetValue(Index), CoupFourre, SpeedLimit);
	QualifiedCoupFourre = 0xFF;	//Playing disqualifies any Coup Fourre

	return ReturnValue;
}

bool	Player::OnRender		(SDL_Surface * Target, Uint8 PlayerIndex, bool Force)
{
	/*	DidSomething is a control variable that tells our calling function whether or not
		we actually rendered anything. If nobody in the call chain "does something" then
		the top-level function will not flip the screen. */
	bool DidSomething =		false;

	if (PlayerIndex == 0)		//Render the human player's hand
		DidSomething = PlayerHand.OnRender(Target, Force);

	DidSomething |= MyTableau.OnRender(Target, PlayerIndex, Force);

	return DidSomething;
}


bool	Player::ReceiveHazard	(Uint8 Value)
{
	// Sanity check: we don't have immunity to the hazard, and it is, in fact, a hazard
	if (!HasSafety(Card::GetMatchingSafety(Value)) && (Card::GetTypeFromValue(Value) == CARD_HAZARD))
	{
		// Send the card down to our tableau
		MyTableau.OnPlay(Value, false, false);

		// We now qualify for a Coup Fourre on this hazard
		QualifiedCoupFourre = Value;

		return true;
	}

	return false;
}

void	Player::Reset			(void)
{
	SourceDeck = 0;
	QualifiedCoupFourre = 0xFF;

	PlayerHand.Reset();
	MyTableau.Reset();
}

void	Player::SetSource		(Deck * ArgSource)
{
	if ((SourceDeck == 0) && (ArgSource != 0))	//SourceDeck isn't set, and ArgSource is set
		SourceDeck = ArgSource;
}

}

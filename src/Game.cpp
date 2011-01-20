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

#include "Game.h"

namespace _SDLMille
{
/*Public methods */

		Game::Game				(void)
{
	Current = 0;	// Set current player to Player 1

	// Set our deck pointer to zero, then try to allocate
	SourceDeck = 0;
	SourceDeck = new Deck;

	// Initialize remaining pointers to zero
	Window = 0;
	GameOverSmall = GameOverBig = DrawFont = 0;

	Modal = MODAL_NONE;
	Scene = SCENE_MAIN;
	LastScene = SCENE_INVALID;
	DownIndex = 0xFF;
	Outcome = OUTCOME_NOT_OVER;

	Animating = false;
	Dirty = true;
	Dragging = false;
	Extended = false;
	ExtensionDeclined = false;
	Frozen = false;
	MouseDown = false;
	Running = true;

	OldDiscardTop = DiscardTop = CARD_NULL_NULL;

	FrozenAt = 0;
	MessagedAt = 0;

	// Initialize SDL_ttf
	if (!TTF_WasInit())
		TTF_Init();

	// Tell all of our players to use our deck
	for (int i = 0; i < PLAYER_COUNT; ++i)
	{
		if (SourceDeck)
			Players[i].SetSource(SourceDeck);

		// And initialize their scores to zero
		Scores[i] = 0;
		RunningScores[i] = 0;

		for (int j = 0; j < SCORE_CATEGORY_COUNT; ++j)
			ScoreBreakdown[i][j] = 0;
	}

	//Staggered deal
	for (int i = 0; i < 13; ++i)
		Players[i % 2].Draw();

	if (SourceDeck)
		OldDeckCount = DeckCount = SourceDeck->CardsLeft();

	GameOptions.ReadOpts();
	Tableau::EnableAnimation = GameOptions.GetOpt(OPTION_ANIMATIONS);

	Message[0] = '\0';

	DrawFont = TTF_OpenFont("LiberationMono-Regular.ttf", 16);
	GameOverBig = TTF_OpenFont("LiberationMono-Regular.ttf", 18);
	GameOverSmall = TTF_OpenFont("LiberationMono-Regular.ttf", 14);

	Black.r = 0; Black.g = 0; Black.b = 0;
	White.r = 255; White.g = 255; White.b = 255;
}

		Game::~Game				(void)
{
	GameOptions.SaveOpts();

	// Clean up all of our pointers
	if (SourceDeck)
		delete SourceDeck;
	//if (Window)
	//	SDL_FreeSurface(Window);
	if (DrawFont)
		TTF_CloseFont(DrawFont);
	if (GameOverBig)
		TTF_CloseFont(GameOverBig);
	if (GameOverSmall)
		TTF_CloseFont(GameOverSmall);

	// SDL_ttf cleanup
	if (TTF_WasInit())
		TTF_Quit();
}

bool	Game::OnExecute			(void)
{
	if (!OnInit())
		return false;

	SDL_Event	Event;

	// Main loop
	while (Running)
	{
		while (SDL_PollEvent(&Event))
			OnEvent(&Event);

		// We have to render twice because the computer moves during OnLoop() and we
		// want to render the human's move before the 500ms delay. Hopefully this will
		// be fixed soon.

		OnRender(Window, false, true);
		OnLoop();
		OnRender(Window, false, true);

		if (!AnimationRunning())
			SDL_Delay(25);
	}
	
	return true;

}

/* Private methods */

void	Game::Animate			(Uint8 Index, Uint8 AnimationType, Uint8 Value)
{
	bool	CoupFourre	= false;
	Uint8	PileCount	= 0;
	char	DurationMsg[41];
	Uint32	StartTicks	= SDL_GetTicks(),
			LastLocalBlit = 0,
			Duration;
	int		TargetFrames = 
			#ifdef WEBOS_DEVICE
			12;
			#else
			50;
			#endif

	if (!GameOptions.GetOpt(OPTION_ANIMATIONS))
		return;

	if (AnimationType >= ANIMATION_INVALID)
		return;

	if ((AnimationType == ANIMATION_COUP_FOURRE_BOUNCE) || ((Index < HAND_SIZE) && (Index == FindPopped()) && (IsValidPlay(Index) || (AnimationType != ANIMATION_PLAY))))
	{
		Animating = true;

		int StartX, StartY,
			DeltaX, DeltaY,
			DestX, DestY,
			i = 0;

		double	IncX, IncY,
				X, Y;

		Uint8	Target = Current,
				Type = CARD_NULL;

		SDL_Surface	*Backdrop = 0;

		if (AnimationType != ANIMATION_COUP_FOURRE_BOUNCE)
		{
			Value = Players[Current].GetValue(Index);
			Players[Current].Detach(Index);
		}

		Type = Card::GetTypeFromValue(Value);

		if (Type < CARD_NULL)
		{
			if (Dragging && (AnimationType != ANIMATION_COUP_FOURRE_BOUNCE))
			{
				StartX = (DragX - 20) / Dimensions::ScaleFactor;
				StartY = (DragY - 67) / Dimensions::ScaleFactor;
			}
			else
			{
				if (AnimationType == ANIMATION_COUP_FOURRE_BOUNCE)
					Tableau::GetTargetCoords(Value, Current, StartX, StartY, false);
				else
				{
					if (Current == 0)
						Hand::GetIndexCoords(Index, StartX, StartY);
					else
					{
						StartX = 140;
						StartY = -61;
					}
				}
			}

			if (Type == CARD_HAZARD)
				Target = 1 - Current;
			else if (Type == CARD_MILEAGE)
				PileCount = Players[Current].GetPileCount(Value);
			else if ((Type == CARD_SAFETY) && (Value == Card::GetMatchingSafety(Players[Current].GetQualifiedCoupFourre())))
				CoupFourre = true;

			if ((AnimationType == ANIMATION_DISCARD) || (AnimationType == ANIMATION_COUP_FOURRE_BOUNCE))
			{
				DestX = 3;
				DestY = Dimensions::FirstRowY;
			}
			else if (AnimationType == ANIMATION_PLAY)
				Tableau::GetTargetCoords(Value, Target, DestX, DestY, CoupFourre, PileCount);
			else
				Hand::GetIndexCoords(Index, DestX, DestY);


			if (Current == 0)
				FloatSurface.SetImage(Card::GetFileFromValue(Value, CoupFourre));
			else
				FloatSurface.SetImage("gfx/card_bg.png");

			X = StartX;
			Y = StartY;

			DeltaX = abs(DestX - X);
			DeltaY = abs(DestY - Y);

			IncX = DeltaX / (double) TargetFrames;
			IncY = DeltaY / (double) TargetFrames;

			#ifdef WEBOS_DEVICE
			Backdrop = SDL_CreateRGBSurface(SDL_SWSURFACE, Dimensions::ScreenWidth, Dimensions::ScreenHeight, Window->format->BitsPerPixel, 0, 0, 0, 0);
			#else
			Backdrop = SDL_CreateRGBSurface(SDL_HWSURFACE, Dimensions::ScreenWidth, Dimensions::ScreenHeight, Window->format->BitsPerPixel, 0, 0, 0, 0);
			#endif

			if (Backdrop != 0)
				OnRender(Backdrop, true, false);

			while ((X != DestX) || (Y != DestY))
			{
				CheckTableau(Backdrop);

				IgnoreEvents();

				#ifndef	WEBOS_DEVICE
				if (SDL_GetTicks() < (LastLocalBlit + (250 / TargetFrames)))
					continue;
				#endif

				LastLocalBlit = SDL_GetTicks();

				if ((i == TargetFrames / 2) && (Current != 0))
					FloatSurface.SetImage(Card::GetFileFromValue(Value, CoupFourre));

				if (abs(DestX - X) < IncX)
					X = DestX;
				else
				{
					if (X > DestX)
						X -= IncX;
					else
						X += IncX;
				}

				if (abs(DestY - Y) < IncY)
					Y = DestY;
				else
				{
					if (Y > DestY)
						Y -= IncY;
					else
						Y += IncY;
				}

				SDL_BlitSurface(Backdrop, 0, Window, 0);
				
				if (CoupFourre)
					Tableau::ShadowSurfaceCF.Render(X, Y, Window);
				else
					Tableau::ShadowSurface.Render(X, Y, Window);

				FloatSurface.Render(X, Y, Window);
				SDL_Flip(Window);

				++i;
			}

			Duration = SDL_GetTicks() - StartTicks;

			if (Backdrop != 0)
			{
				SDL_FreeSurface(Backdrop);
				Backdrop = 0;
			}

			#ifdef	DEBUG
			sprintf(DurationMsg, "%u , %u ms", i, Duration);
			ShowMessage(DurationMsg);
			#endif
		}

		Animating = false;
	}			
}

bool	Game::AnimationRunning	(void)						const
{
	bool	ReturnValue = false;

	for (int i = 0; i < PLAYER_COUNT; ++i)
		ReturnValue |= Players[i].AnimationRunning();

	return ReturnValue;
}

void	Game::ChangePlayer		(void)
{
	if (!EndOfGame())
	{
		Current = 1 - Current;
		Players[Current].Draw();
	}
}

bool	Game::CheckForChange	(Uint8 &Old, Uint8 &New)
{
	if (Old != New)
	{
		Old = New;
		return true;
	}

	return false;
}

void	Game::CheckTableau		(SDL_Surface *Target)
{
	if (AnimationRunning())
	{
		if (Tableau::LastAnimationBlit + 15 <= SDL_GetTicks())
			OnRender(Target);
	}
}

void	Game::ClearMessage		(void)
{
	Message[0] = '\0';
	MessagedAt = 0;
	MessageSurface.Clear();
	Dirty = true;
}

bool	Game::ComputerDecideExtension	(void)			const
{
	bool	ReturnValue = true;
	int		MileageInHand = Players[1].MileageInHand(),
			MileageNeeded;
	Uint8	CardsRemaining = 0;

	if (SourceDeck != 0)
		CardsRemaining = SourceDeck->CardsLeft();

	if (CardsRemaining < 5)
		MileageNeeded = 300;
	else if (CardsRemaining < 10)
		MileageNeeded = 200;
	else
		MileageNeeded = 100;

	if (Players[0].GetMileage() == 0)
		ReturnValue = false;
	else if (MileageInHand < MileageNeeded)
		ReturnValue = false;

	return ReturnValue;
}

void	Game::ComputerMove		(void)
{
	bool	Played		= false;
	Uint32	StartTicks	= SDL_GetTicks(),
			EndTicks = StartTicks + ((GameOptions.GetOpt(OPTION_FAST_GAME)) ? 200 : 500);

	for (int i = 0; i < HAND_SIZE; ++i)
	{	
		CheckTableau(Window);

		if (IsValidPlay(i))	// Play the first valid move we find (the computer is currently stupid)
		{
			DelayUntil(EndTicks);

			Pop(i);
			Pop(i);
			Played = true;
			break;
		}
	}

	if (Played == false)
	{
		CheckTableau(Window);

		for (int i = 0; i < HAND_SIZE; ++i)	//No valid moves, discard
		{
			if (Players[Current].GetValue(i) < CARD_NULL_NULL)
			{
				DelayUntil(EndTicks);

				Pop(i);
				if (Discard())
					break;
			}
		}
	}
}

void	Game::ComputerSmartMove	(void)
{
	Uint8	ArrayIndex = 0,
			MatchingCard = 0,
			Opponent = 1 - Current,
			Processed[CARD_NULL_NULL];
	int		TripLength = (Extended) ? 1000 : 700,
			MyMileage = Players[Current].GetMileage(),
			MyRemaining = TripLength - MyMileage,
			OpponentMileage = Players[Opponent].GetMileage(),
			OpponentLead = OpponentMileage - MyMileage,
			OpponentRemaining = TripLength - OpponentMileage,
			Weight[HAND_SIZE][2];
	bool	MyselfLimited = Players[Current].IsLimited(),
			MyselfOneMoveAway = false,
			MyselfRolling = Players[Current].IsRolling(),
			OpponentLimited = Players[Opponent].IsLimited(),
			OpponentOneMoveAway = false,
			OpponentRolling = Players[Opponent].IsRolling();

	for (int i = 0; i < CARD_NULL_NULL; ++i)
		Processed[i] = 0;

	for (int i = 0; i < HAND_SIZE; ++i)
	{
		Uint8	Type = Players[Current].GetType(i),
				Value = Players[Current].GetValue(i);

		Weight[i][0] = i;
		Weight[i][1] = 0;

		if (Type == CARD_SAFETY)
		{
			MatchingCard = Value - SAFETY_OFFSET;

			if (Card::GetMatchingSafety(Players[Current].GetQualifiedCoupFourre()) == Value)
			{
				// This is a coup fourre
				Weight[i][1] += 100;
				printf("A0: Coup Fourre\n");
			}
			else if (IsOneCardAway(Opponent))
			{
				// Opponent could end the game. Play safeties now
				Weight[i][1] += 90;
				printf("A10: Game almost over\n");
			}
			else if (KnownCards(MatchingCard) == EXISTING_CARDS[MatchingCard])
			{
				if ((Value != CARD_SAFETY_RIGHT_OF_WAY) || (KnownCards(CARD_HAZARD_STOP) == EXISTING_CARDS[CARD_HAZARD_STOP]))
				{
					// No more hazards to go with it. No use in saving it
					Weight[i][1] += 50;
					printf("A20: No more hazards\n");
				}
			}
		}
		else if (Type == CARD_REMEDY)
		{
			Uint8	TopCard = CARD_NULL_NULL;

			// Get corresponding hazard
			MatchingCard = Value - 5;
			TopCard = Players[Current].GetTopCard((MatchingCard == CARD_HAZARD_SPEED_LIMIT));

			if (Players[Current].HasSafety(Card::GetMatchingSafety(MatchingCard)))
			{
				// We have the safety, our remedy is useless
				Weight[i][1] -= 100;
				printf("B0: We have the safety\n");
			}
			else if ((TopCard == MatchingCard) || ((Value == CARD_REMEDY_ROLL) && ((Card::GetTypeFromValue(TopCard) == CARD_REMEDY) || TopCard == CARD_NULL_NULL)))
			{
				// A remedy for our current hazard
				Weight[i][1] += 50;
				printf("B10: Remedy for current hazard\n");
			}
			else if (KnownCards(MatchingCard) == EXISTING_CARDS[MatchingCard])
			{
				printf("B20: Exhausted hazard\n");
				// Matching hazard has been exhausted. No use in keeping the remedy
				if (Value != CARD_REMEDY_ROLL)
					//We need to keep roll cards on hand even if there are no more stop cards
					Weight[i][1] -= 100;
				else
					Weight[i][1] += 10;
			}
			else if (InHand(Value) >= (EXISTING_CARDS[MatchingCard] - ExposedCards[MatchingCard]))
			{
				printf("B30: More remedies than remaining hazards\n");
				if (Value != CARD_REMEDY_ROLL)
					// We have more than we need.
					Weight[i][1] -= 100;
				else
					Weight[i][1] += 10;
			}
			else if (Value == CARD_REMEDY_ROLL)
			{
				if (!Players[Current].HasSafety(CARD_SAFETY_RIGHT_OF_WAY) && !InHand(CARD_SAFETY_RIGHT_OF_WAY))
				{
					// Roll cards are more valuable than other remedies.
					Weight[i][1] += 40;
					printf("B40: Roll card without RoW safety\n");
				}
			}
			else
			{
				printf("B50: General remedy\n");
				// Doesn't help us right now, but we might need it later
				Weight[i][1] += 30;
			}
		}
		else if (Type == CARD_HAZARD)
		{
			MatchingCard = Value + 5;

			if (Players[Opponent].HasSafety(Card::GetMatchingSafety(Value)))
			{
				printf("C0: Opponent has safety\n");
				// Hazard is useless
				Weight[i][1] -= 100;
			}
			else
			{
				if (IsOneCardAway(Opponent) || (OpponentMileage == 0) || (OpponentRemaining <= 200) || (OpponentLead >= 200))
				{
					printf("C10: Must stop opponent\n");
					// We need to stop the opponent if possible
					Weight[i][1] += 60;
				}
				else
				{
					printf("C20: General hazard\n");
					// No pressing need right now
					Weight[i][1] += 40;
				}

				if (!CouldHoldCard(Opponent, MatchingCard))
				{
					printf("C30: Opponent could not hold remedy\n");
					// Opponent could not hold the remedy
					Weight[i][1] += 10;
				}

				if (!CouldHoldCard(Opponent, Card::GetMatchingSafety(Value)))
				{
					printf("C40: Opponent could not hold safety\n");
					// Opponent could not hold the safety
					Weight[i][1] += 15;
				}
			}
		}
		else if (Type == CARD_MILEAGE)
		{
			Uint8	MileValue	= Card::GetMileValue(Value),
					My200Count	= Players[Current].GetPileCount(CARD_MILEAGE_200);

			if (MileValue > MyRemaining)
			{
				printf("D0: Goes past end of trip\n");
				// Would take us past end of trip
				if (Extended)
					// Card is totally useless
					Weight[i][1] -= 100;
				else
					// Card is mostly useless
					Weight[i][1] -= 50;
			}
			else if (MileValue == MyRemaining)
			{
				printf("D10: Takes us exactly to end of hand\n");
				// Card could win us the hand
				Weight[i][1] += 50;

				if ((Value != CARD_MILEAGE_200) || (My200Count > 0))
					// Prefer not to play 200 if we haven't already
					Weight[i][1] += 40;
			}
			else if (MyRemaining <= 200)
			{
				// We have to be careful in the last part of the trip
				int	MileBalance = MyRemaining - MileValue;

				if (MileBalance <= 100)
				{
					// Hand could be one in two plays, including this play
					if (InHand(Card::GetCardFromMileage(MileBalance)) > 0)
					{
						printf("D20: Complimentary mileage to finish trip\n");
						// We have the mileage necessary to do so
						Weight[i][1] += 50;
					}
					else if (MileBalance == 25)
					{
						printf("D30: Leaves us with only 25 miles left\n");
						// Prefer not to end up with only 25 miles remaining
						Weight[i][1] -= 10;
					}
					else
					{
						printf("D40: Gets us within 100 miles\n");
						Weight[i][1] += 20;
					}
				}
				else
				{
					printf("D50: Nearing end; balance greater than 100\n");
					// Give some weight to it, but not much. Would require at least two more plays after this one
					Weight[i][1] += 10;
				}
			}
			else
			{
				printf("D60: Weighted on mileage\n");
				// Weight based on mileage
				Weight[i][1] += (MileValue / 25) * 6;

				if (Value == CARD_MILEAGE_200)
				{
					if (My200Count < 1)
						// Prefer not to play
						Weight[i][1] /= 4;
				}
			}

			if ((Value == CARD_MILEAGE_200) && (My200Count > 1))
			{
				printf("Can't play any more 200's\n");
				// Useless card
				Weight[i][1] = -100;
			}
		}

		if (Type == CARD_MILEAGE)
			printf("%u %u %i %u\n", i, Value, Weight[i][1], Card::GetMileValue(Value));
		else
			printf("%u %u %i %s\n", i, Value, Weight[i][1], CARD_CAPTIONS[Value]); 
	}

	printf("\n");

	for (int i = 0; i < (HAND_SIZE - 1); ++i)
	{
		bool Sorted = false;

		while (!Sorted)
		{
			Sorted = true;

			for (int j = i; j < (HAND_SIZE - 1); ++j)
			{
				if (Weight[j][1] < Weight[j+1][1])
				{
					int	TempIndex = Weight[j][0],
						TempWeight = Weight[j][1];

					Weight[j][0] = Weight[j+1][0];
					Weight[j][1] = Weight[j+1][1];

					Weight[j+1][0] = TempIndex;
					Weight[j+1][1] = TempWeight;

					Sorted = false;
				}
			}
		}
	}

	bool Played = false;

	for (int i = 0; i < HAND_SIZE; ++i)
	{
		Uint8 Index = Weight[i][0];

		if (IsValidPlay(Index) && (Weight[i][1] != 0))
		{
			Pop(Index);
			Pop(Index);

			Played = true;
			break;
		}
	}
	
	if (!Played)
	{
		for (int i = HAND_SIZE - 1; i >= 0; --i)
		{
			Uint8 Index = Weight[i][0];

			if ((Players[Current].GetValue(Index) < CARD_NULL_NULL) && (Weight[i][1] != 0))
			{
				Pop(Index);

				if (Discard())
					break;
			}
		}
	}
}

bool	Game::CouldHoldCard		(Uint8 PlayerIndex, Uint8 Value)			const
{
	Uint8	NumberExisting	= 0,
			NumberExposed	= 0,
			NumberInHand	= 0;
	bool	IsCurrentPlayer	= (PlayerIndex == Current),
			ReturnValue		= false;

	if (Value < CARD_NULL_NULL)
	{
		NumberExisting = EXISTING_CARDS[Value];
		NumberExposed = ExposedCards[Value];
		NumberInHand = InHand(Value);

		if (NumberExposed < NumberExisting)
		{
			// Someone could hold the card

			if (NumberInHand > 0)
			{
				if (IsCurrentPlayer)
					// We have the card
					ReturnValue = true;
				else if (NumberExposed + NumberInHand == NumberExisting)
					// We have the rest of the cards; opponent cannot hold one
					ReturnValue = false;
				else
					ReturnValue = true;
			}
			else
				// We don't have the card, opponent may
				ReturnValue = true;
		}			
	}

	return ReturnValue;
}

void	Game::DelayUntil		(Uint32 Ticks)
{
	while (SDL_GetTicks() < Ticks)
	{
		CheckTableau(Window);
		IgnoreEvents();
		SDL_Delay(5);
	}
}

bool	Game::Discard			(void)
{
	Uint8	Value =	CARD_NULL_NULL,
			Index =	FindPopped(); // Find out which card is popped

	if (Index < HAND_SIZE)
	{
		Value	= Players[Current].GetValue(Index);

		if (Value < CARD_NULL_NULL)	// Sanity check
		{
			Players[Current].Detach(Index);
			Animate(Index, ANIMATION_DISCARD);
			DiscardTop = Value;	// Put the card on top of the discard pile
			Players[Current].Discard(Index);

			++ExposedCards[Value];
		}

		Players[Current].UnPop(Index);
		ChangePlayer();

		//Save after each discard
		Save();

		return true;
	}

	return false;
}

bool	Game::EndOfGame			(void)								const
{
	if (Players[0].IsOutOfCards() && Players[1].IsOutOfCards())	//Both players are out of cards
		return true;

	for (int i = 0; i < PLAYER_COUNT; ++i)
	{
		if (Players[i].GetMileage() == ((Extended) ? 1000 : 700))	//A player has completed the trip
			return true;
	}

	return false;
}

Uint8	Game::FindPopped		(void)								const
{
	// Find which card in the hand is "popped"
	for (int i = 0; i < HAND_SIZE; ++i)
	{
		if (Players[Current].IsPopped(i))
			return i;
	}

	return 0xFF;
}

void	Game::GetScores			(void)
{
	if (EndOfGame())
	{
		for (int i = 0; i < PLAYER_COUNT; ++i)
		{
			int Score = 0,
				PlayerSafetyCount = 0, PlayerCoupFourreCount = 0,
				CategoryIndex = 0;

			/* Points scored by everyone */

			// Distance (1pt per mile travelled)
			Score += Players[i].GetMileage();
			ScoreBreakdown[i][CategoryIndex] = Score;
			++CategoryIndex;
			
			for (int j = 0; j < SAFETY_COUNT; ++j) // Get info about safeties and Coup Fourres
			{
				if (Players[i].HasSafety(j + SAFETY_OFFSET))
				{
					++PlayerSafetyCount;
					if (Players[i].HasCoupFourre(j + SAFETY_OFFSET))
						++PlayerCoupFourreCount;
				}
			}

			// 100pt for each safety, no matter how played
			if (PlayerSafetyCount)
			{
				int SafetyScore = PlayerSafetyCount * 100;
				Score += SafetyScore;
				ScoreBreakdown[i][CategoryIndex] = SafetyScore;
			}
			++CategoryIndex;

			// 300pt bonus for all four safeties
			if (PlayerSafetyCount == 4)
			{
				Score += 300;
				ScoreBreakdown[i][CategoryIndex] = 300;
			}
			++CategoryIndex;

			// 300pt bonus for each Coup Fourre
			if (PlayerCoupFourreCount)
			{
				int CoupFourreScore = PlayerCoupFourreCount * 300;
				Score += CoupFourreScore;
				ScoreBreakdown[i][CategoryIndex] += CoupFourreScore;
			}
			++CategoryIndex;

			/* Points scored only by the player which completed the trip */

			if (Players[i].GetMileage() == ((Extended) ? 1000 : 700))
			{
				// 400pt for completing trip
				Score += 400;
				ScoreBreakdown[i][CategoryIndex] = 400;
				++CategoryIndex;

				// 300pt bonus for delayed action (draw pile exhausted before trip completion)
				if (SourceDeck)
				{
					if (SourceDeck->Empty())
					{
						Score += 300;
						ScoreBreakdown[i][CategoryIndex] = 300;
					}
				}
				++CategoryIndex;

				// 300pt bonus for safe trip (no 200-mile cards)
				if (Players[i].GetPileCount(CARD_MILEAGE_200) == 0)
				{
					Score += 300;
					ScoreBreakdown[i][CategoryIndex] = 300;
				}
				++CategoryIndex;

				// 200pt bonus for completing an extended trip
				if (Extended)
				{
					Score += 200;
					ScoreBreakdown[i][CategoryIndex] = 200;
				}
				++CategoryIndex;

				// 500pt shutout bonus (opponent did not play any mileage cards during the hand)
				if (Players[1 - i].GetMileage() == 0)
				{
					Score += 500;
					ScoreBreakdown[i][CategoryIndex] = 500;
				}
			}
			else
				CategoryIndex += 4;

			++CategoryIndex;

			Scores[i] = Score;
			ScoreBreakdown[i][CategoryIndex] = Score;	//Set subtotal

			++CategoryIndex;

			ScoreBreakdown[i][CategoryIndex] = RunningScores[i];	//Set previous score

			++CategoryIndex;

			ScoreBreakdown[i][CategoryIndex] = RunningScores[i] + Score;	//Set total score
		}
	}
}

void	Game::IgnoreEvents		(void)
{
	SDL_Event	DummyEvent;

	while (SDL_PollEvent(&DummyEvent))
		;
}

bool	Game::InDiscardPile		(int X, int Y)						const
{
	int MaxX = 48;
	int MaxY = Dimensions::FirstRowY + 61;

	if (Dragging)
	{
		MaxX += 12;
		MaxY += 3;
	}

	return ((X >= 3) && (X <= MaxX) && (Y >= Dimensions::FirstRowY) && (Y <= MaxY));
}

bool	Game::IsOneCardAway		(Uint8 PlayerIndex)					const
{
	int		Mileage = Players[PlayerIndex].GetMileage(),
			RemainingMileage = ((Extended) ? 1000 : 700) - Mileage;

	Uint8	CardCount = Players[PlayerIndex].CardsInHand(),
			TopCard = Players[PlayerIndex].GetTopCard(),
			MatchingSafety = Card::GetMatchingSafety(TopCard),
			ValueNeeded = CARD_NULL_NULL;

	bool	CouldRoll = false,
			IsCurrentPlayer = (PlayerIndex == Current),
			IsLimited = Players[PlayerIndex].IsLimited(),
			IsRolling = Players[PlayerIndex].IsRolling(),
			MayHaveROW = false;

	if (CardCount < 1)
		// Can't with without any cards
		return false;
	
	if (Players[PlayerIndex].HasSafety(CARD_SAFETY_RIGHT_OF_WAY))
		MayHaveROW = true;
	else if (CouldHoldCard(PlayerIndex, CARD_SAFETY_RIGHT_OF_WAY))
		MayHaveROW = true;

	if (!IsRolling)
	{
		if (CardCount < 2)
			// Not enough cards to start rolling and play a mileage
			return false;
		else if (!MayHaveROW)
			// Can't have ROW card, no way to roll and play mileage in one turn
			return false;
	}

	if ((Card::GetTypeFromValue(TopCard) == CARD_HAZARD) && (!CouldHoldCard(PlayerIndex, MatchingSafety)))
		//Player is afflicted by a hazard and can't hold the safety
		return false;

	if (IsLimited && !MayHaveROW && (RemainingMileage > 50))
		// Player is limited, could not hold ROW, and has more than 50 miles to go
		return false;
	else if (RemainingMileage > 200)
		// Too far to travel in one turn
		return false;
	else if ((RemainingMileage < 200) && (RemainingMileage > 100))
		// No existing mileage card to complete trip in one turn
		return false;

	ValueNeeded = Card::GetCardFromMileage(RemainingMileage);

	if (!CouldHoldCard(PlayerIndex, ValueNeeded))
		// Could not hold required mileage card
		return false;

	if ((ValueNeeded == CARD_MILEAGE_200) && (Players[PlayerIndex].GetPileCount(CARD_MILEAGE_200) > 1))
		// Needs a 200 and has already exhausted limit
		return false;

	return true;
}

Uint8	Game::InHand			(Uint8 Value)						const
{
	Uint8 ReturnValue = 0;

	if (Value < CARD_NULL_NULL)
	{
		for (int i = 0; i < HAND_SIZE; ++i)
		{
			if (Players[Current].GetValue(i) == Value)
				++ReturnValue;
		}
	}

	return ReturnValue;
}

bool	Game::IsValidPlay		(Uint8 Index)						const
{
	if (Index >= HAND_SIZE)
		return false;

	Uint8	Type =	Players[Current].GetType(Index),
			Value =	Players[Current].GetValue(Index);

	if (Type == CARD_NULL)
		// The player tapped an empty slot
		return false;

	if (Type == CARD_MILEAGE)
	{
		Uint8 MileageValue = Card::GetMileValue(Value);

		if (!Players[Current].IsRolling())
			// Must be rolling to play mileage cards
			return false;

		if (Players[Current].IsLimited())
		{
			// Enforce speed limit
			if (MileageValue > 50)
				return false;
		}

		if ((Value == CARD_MILEAGE_200) && (Players[Current].GetPileCount(CARD_MILEAGE_200) > 1))
			// Cannot play more than two 200-mile cards
			return false;

		//Cannot go past trip end, 1000 miles if the trip was extended, otherwise 700
		if ((MileageValue + Players[Current].GetMileage()) > ((Extended) ? 1000 : 700))
			// Cannot go past end of trip
			return false;
		
		return true;
	}
	
	if (Type == CARD_HAZARD)
	{
		if (Players[1 - Current].HasSafety(Card::GetMatchingSafety(Value)))
			// If our opponent has the matching safety, the hazard can't be played
			return false;

		if (Value == CARD_HAZARD_SPEED_LIMIT)
		{
			if (Players[1 - Current].IsLimited())
				// Opponent is already limited
				return false;
		}
		else
		{
			if (!Players[1 - Current].IsRolling())
				// Cannot play other hazards unless opponent is rolling
				return false;
		}

		return true;
	}
	
	if (Type == CARD_REMEDY)
	{
		Uint8 TopCard =		Players[Current].GetTopCard();
		Uint8 TopCardType = Card::GetTypeFromValue(TopCard);
		
		if (Value == CARD_REMEDY_ROLL)
		{
			if ((TopCardType != CARD_REMEDY) && (TopCard != CARD_HAZARD_STOP) && (TopCard != CARD_NULL_NULL) && !Players[Current].HasSafety(Card::GetMatchingSafety(TopCard)))
				/* Are we:	1. Playing on top of a remedy
							2. Playing on top of a "stop" card
							3. Playing on an empty battle pile
							4. Playing on a hazard to which we have the matching safety */
				return false;

			if (Players[Current].IsRolling())
				// We're already rolling
				return false;
		}
		else
		{
			if (TopCardType == CARD_HAZARD)
			{
				if (Players[Current].HasSafety(Card::GetMatchingSafety(TopCard)))
					// Remedy is superfluous; we already have the safety
					return false;
			}

			if (Value == CARD_REMEDY_END_LIMIT)
			{
				if (!Players[Current].IsLimited())
					// We're not limited, so we can't end the limit
					return false;
			}
			else
			{
				if (TopCardType != CARD_HAZARD)
					// Other remedies can only be played on top of hazards
					return false;

				if (TopCard != (Value - 5))
					// The remedy does not match the hazard
					return false;
			}
		}

		return true;
	}

	if (Type == CARD_SAFETY)
		// There are no restrictions on playing safeties.
		return true;

	// Default to false. We should never get here unless something went horribly wrong.
	return false;
}

Uint8	Game::KnownCards		(Uint8 Value)						const
{
	Uint8 ReturnValue = ExposedCards[Value];

	for (int i = 0; i < HAND_SIZE; ++i)
	{
		if (Players[Current].GetValue(i) == Value)
			++ReturnValue;
	}

	return ReturnValue;
}

void	Game::OnClick			(int X, int Y)
{
	static	Uint32	LastClick = 0;

	if (Frozen)		//Don't register clicks when frozen
	{
		if ((SDL_GetTicks() - 1000) > FrozenAt)
		{
			Frozen = false;
			FrozenAt = 0;
		}
		else
			return;
	}

	if (Modal < MODAL_NONE)		// Handle clicks on a modal window
	{
		if (Modal == MODAL_EXTENSION)
		{
			if ((Y >= 251) && (Y <= 276))
			{
				if ((X >= 81) && (X < 151))
				{
					Extended = true;
					Modal = MODAL_NONE;
					Dirty = true;
					ChangePlayer();
				}
				else if ((X > 169) && (X <= 239))
				{
					Extended = false;
					ExtensionDeclined = true;
					Modal = MODAL_NONE;
					Dirty = true;
				}

				Save();
			}
		}
		else if (Modal == MODAL_GAME_MENU)
		{
			if ((X >= 50) && (X <= 270))
			{
				if ((Y >= 120) && (Y <= 400))
				{
					int Index = (Y - 120) / 40;

					if (Index < OPTION_COUNT)	//Clicked an option toggle
					{
						GameOptions.SwitchOpt(Index);
						ShowModal(Modal);
						return;
					}
					else if (Index < (OPTION_COUNT + MENU_ITEM_COUNT)) //Clicked a menu item
					{
						switch (Index - OPTION_COUNT)
						{
						case 0:
							ShowModal(MODAL_NEW_GAME);
							break;
						case 1:
							LastScene = Scene;
							Scene = SCENE_MAIN;
							Modal = MODAL_NONE;
							Dirty = true;
							break;
						}
					}
				}
			}						
			if ((X >= 251) && (X <= 277))
			{
				if ((Y >= 83) && (Y <= 109))	// Clicked the X button
				{
					GameOptions.SaveOpts();
					Modal = MODAL_NONE;
					Dirty = true;
					Tableau::EnableAnimation = GameOptions.GetOpt(OPTION_ANIMATIONS);
				}
			}
		}
		else if (Modal == MODAL_NEW_GAME)
		{
			if ((X >= 65) && (X <= 255) && (Y >= 220) && (Y <= 265))	// Clicked Cancel
				ShowModal(MODAL_GAME_MENU);
			else if ((X >= 130) && (X <= 190) && (Y >= 281) && (Y <= 311))	//Clicked Confirm
			{
				Reset(false);
				for (int i = 0; i < PLAYER_COUNT; ++i)
					RunningScores[i] = 0;

				Save();

				Modal = MODAL_NONE;
			}
		}	
		return;
	}

	if (Scene == SCENE_MAIN)	//Main menu
	{
		if ((X >= 45) && (X <= 275))
		{
			if ((Y >= 300) && (Y <= 355))	//Clicked Play
			{
				ShowLoading();
				Reset(false);
				LastScene = SCENE_MAIN;
				Scene =		SCENE_GAME_PLAY;
				Restore();
			}
			if ((Y >= 370) && (Y <= 415))	//Clicked Learn
			{
				ShowLoading();
				Reset(false);
				LastScene = SCENE_MAIN;
				Scene =		SCENE_LEARN_1;
			}
		}
		if ((X >= (Dimensions::ScreenWidth - LogoSurface.GetWidth())) && (Y >= (Dimensions::ScreenHeight - LogoSurface.GetHeight())))		//Clicked GPL logo
		{
			LastScene = Scene;
			Scene = SCENE_LEGAL;
			return;
		}
	}

	else if (Scene == SCENE_GAME_PLAY)		//In game play
	{
		if (Current == 0) // Don't respond to clicks unless it's the human's turn
		{
			if (Y < Dimensions::TableauHeight)	//Clicked within opponent's tableau
			{
				if ((Y < 45) && (X < 80))		//Clicked Menu button
					ShowModal(MODAL_GAME_MENU);
				else
				{
					// Play selected card, if it's a hazard
					if (Players[Current].GetType(FindPopped()) == CARD_HAZARD)
						Pop(FindPopped());
				}
			}
			else if ((Y > Dimensions::TableauHeight) && (Y < (Dimensions::TableauHeight * 2)))	//Clicked within own tableau
				Pop(FindPopped());	//Play selected card, if any
			else if ((Y >= Dimensions::FirstRowY) && (Y <= (Dimensions::SecondRowY + 57)))	//Clicked within the two rows at the bottom of the screen
			{
				Uint8 Index = Hand::GetIndex(X, Y);
				
				if (Index == 0)
				{
					Players[0].UnPop(FindPopped());
					return;
				}
				else
				{
					Index -= 1;

					if (Index < HAND_SIZE)
						Pop(Index);	//Clicked a card, so pop it
					else
					{
						if (InDiscardPile(X, Y))	//Clicked the discard pile
							Discard();
					}
				}
			}
		}
	}

	else if (Scene == SCENE_GAME_OVER)	//Score screen, start new game on click
	{
		Reset(true);
		LastScene = Scene;
		Scene = SCENE_GAME_PLAY;
		Save();
	}

	else if (IN_TUTORIAL)	//Tutorial scenes
	{
		if ((Y >= 5) && (Y <= 80))
		{
			if ((X >= 5) && (X <= 80))	//Clicked back arrow
			{
				LastScene = Scene;
				if (Scene == SCENE_LEARN_2)
					ClearMessage();
				if (Scene != SCENE_LEARN_1)
					--Scene;
				else
					Scene = SCENE_MAIN;
			}
			else if ((X >= 245) && (X <= 315))	//Clicked forward arrow
			{
				LastScene = Scene;
				if (Scene != SCENE_LEARN_7)
					++Scene;
				else
				{
					ClearMessage();
					Scene = SCENE_MAIN;
				}
			}
		}
	}

	else if (Scene == SCENE_LEGAL)	//Legal scene, return to main menu on click
	{
		if (LastClick + 500 >= SDL_GetTicks())
		{
			LastScene = Scene;
			Scene = SCENE_MAIN;
		}
		else
		{
			LastClick = SDL_GetTicks();
			Overlay[2].SetText("Double-click to return.", GameOverBig, &White, &Black);
			Dirty = true;
		}
	}
}

void	Game::OnEvent			(SDL_Event * Event)
{
	int	X = 0, Y = 0;

	if (Event != 0)
	{
		if (Event->type == SDL_QUIT)
			Running = false;
		else if (Event->type == SDL_MOUSEBUTTONDOWN)
		{
			if (Event->button.which == 0)
			{
				MouseDown = true;

				DownX = Event->button.x;
				DownY = Event->button.y;

				if (Scene == SCENE_GAME_PLAY)
					DownIndex = Hand::GetIndex(DownX / Dimensions::ScaleFactor, DownY / Dimensions::ScaleFactor) - 1;
			}
		}
		else if (Event->type == SDL_MOUSEBUTTONUP)	//Mouse click
		{
			if (Event->button.which == 0)
			{
				double	Scale = Dimensions::ScaleFactor;
				MouseDown = false;

				X = Event->button.x;
				Y = Event->button.y;

				if ((Scene == SCENE_GAME_PLAY) && (Current == 0) && Dragging)
				{

					if (DownIndex < HAND_SIZE)
					{
						if (Y < (Dimensions::EffectiveTableauHeight * 2))
						{
							if (IsValidPlay(DownIndex))
								Pop(DownIndex);
							else
								Animate(DownIndex, ANIMATION_RETURN);
						}
						else if (InDiscardPile(X / Scale, Y / Scale))
							Discard();
						else
							Animate(DownIndex, ANIMATION_RETURN);

						Players[0].UnPop(DownIndex);
						DownIndex = 0xFF;

						FloatSurface.Clear();
					}

					Dragging = false;
				}

				if ((abs(DownX - X) > 5) || (abs(DownY - Y) > 5))
				{
					if ((Scene != SCENE_GAME_PLAY) || (Hand::GetIndex(X / Scale, Y / Scale) != 0))
						return;
				}

				if (Scale != 1)
				{
					X /= Scale;
					Y /= Scale;
				}

				OnClick(X, Y);
			}
		}
		else if (Event->type == SDL_MOUSEMOTION)
		{
			if (MouseDown)
			{
				int MotionX = Event->motion.xrel;
				int MotionY = Event->motion.yrel;

				if ((Scene == SCENE_LEGAL) || (Scene == SCENE_LEARN_1))
				{
					int CurX = Portal.x;
					int CurY = Portal.y;

					int NewX = CurX - MotionX;
					int NewY = CurY - MotionY;

					int MaxX = Overlay[0].GetWidth() - Dimensions::ScreenWidth;
					int MaxY = Overlay[0].GetHeight() - Dimensions::ScreenHeight;

					if (NewX > MaxX)
						NewX = MaxX;
					if (NewX < 0)
						NewX = 0;

					if (NewY > MaxY)
						NewY = MaxY;
					if (NewY < 0)
						NewY = 0;

					if ((NewX != CurX) || (NewY != CurY))
					{
						Portal.x = NewX;
						Portal.y = NewY;
						Overlay[2].Clear();

						Dirty = true;
					}
				}
				else if ((Scene == SCENE_GAME_PLAY) && (Current == 0))
				{
					if (DownIndex < HAND_SIZE)
					{
						Uint8	Value	= Players[Current].GetValue(DownIndex);
						if (Value < CARD_NULL_NULL)
						{
							DragX = Event->motion.x;
							DragY = Event->motion.y;

							if (!Dragging)
							{
								if ((abs(DownX - DragX) > 5) || (abs(DownY - DragY) > 5))
								{
									Dragging = true;

									Uint8 Value = Players[Current].GetValue(DownIndex);
									Pop(DownIndex);
									Players[0].Detach(DownIndex);
									FloatSurface.SetImage(Card::GetFileFromValue(Value, (Value == Card::GetMatchingSafety(Players[Current].GetQualifiedCoupFourre()))));
								}
							}
							
							Dirty = true;
						}
					}
				}
			}
		}
		else if (Event->type == SDL_KEYUP)	//Debugging purposes
		{
			//return;

			OnRender(Window, true, false);
			SDL_SaveBMP(Window, "screenshot.bmp");
			/*
			for (int i = 0; i < CARD_NULL_NULL; ++i)
				printf("%2u %2u\n", i, ExposedCards[i]);

			for (int i = 0; i < PLAYER_COUNT; ++i)
			{
				if (IsOneCardAway(i))
					printf("%u could win in one turn.\n", i);
				else
					printf("%u could not win.\n", i);
			}

			printf("\n");

			Uint8	OldPlayer = Current;

			Current = 1;

			Players[Current].OnRender(Window, 0, true);
			SDL_Flip(Window);

			ComputerSmartMove();

			Current = OldPlayer;
			*/
		}
	}
}

bool	Game::OnInit			(void)
{
	Dirty = false;

	if (!Window)
	{
		// Set up our display if we haven't already
		if(SDL_Init(SDL_INIT_VIDEO) < 0)
			return false;

		#if defined WEBOS_DEVICE
		if(!(Window = SDL_SetVideoMode(0, 0, 0, SDL_SWSURFACE)))
		#else
		if(!(Window = SDL_SetVideoMode(320, 480, 32, SDL_HWSURFACE | SDL_DOUBLEBUF)))
		#endif
			return false;

		Dimensions::SetDimensions(Window->w, Window->h);

		ResetPortal();

		SDL_WM_SetCaption("SDL Mille", "SDL Mille");
	}

	if (IN_TUTORIAL)
	{
		ArrowSurfaces[0].SetImage("gfx/arrowl.png");
		ArrowSurfaces[1].SetImage("gfx/arrowr.png");
		if (Scene >= SCENE_LEARN_2)
		{
			HandSurface.SetImage("gfx/hand.png");
			ShowMessage(TUTORIAL_TEXT[Scene - SCENE_LEARN_2]);
		}
	}

	if (Message[0] != '\0')
		MessageSurface.SetText(Message, GameOverBig, &Black);

	if (Scene == SCENE_MAIN)
	{
		Background.SetImage("gfx/scenes/main.png");
		if (!Background)
			return false;

		if (Dimensions::ScreenHeight > 440)
			LogoSurface.SetImage("gfx/gpl.png");
		else
			LogoSurface.SetImage("gfx/gpl_sideways.png");

		Overlay[0].SetImage("gfx/loading.png");

		return true;
	}

	else if ((Scene == SCENE_GAME_PLAY) || (Scene == SCENE_LEARN_2))
	{
		Background.Clear();

		Overlay[0].SetImage("gfx/overlays/game_play_1.png");

		DiscardSurface.SetImage(Card::GetFileFromValue(DiscardTop));
		TargetSurface.SetImage("gfx/drop_target.png");

		if ((SourceDeck != 0) && (SourceDeck->Empty()))
			DrawCardSurface.SetImage(Card::GetFileFromValue(CARD_NULL_NULL));
		else
			DrawCardSurface.SetImage("gfx/card_bg.png");

		if (DrawFont)
		{
			DrawTextSurface.SetInteger(DeckCount, DrawFont, true, &White);
			if ((Current == 0) && (FindPopped() < HAND_SIZE))
			{
				Uint8	Value = Players[Current].GetValue(FindPopped());
				if (Value < CARD_MILEAGE_25)
					CaptionSurface.SetText(CARD_CAPTIONS[Players[Current].GetValue(FindPopped())], DrawFont);
				else
					CaptionSurface.SetText("MILEAGE", DrawFont);
			}
			else
				CaptionSurface.Clear();
		}

		if (Scene == SCENE_GAME_PLAY)
			MenuSurface.SetImage("gfx/menu.png");
		else
			OrbSurface.SetImage("gfx/orb.png");

		return true;
	}

	else if (Scene == SCENE_LEARN_1)
	{
		ResetPortal();

		Background.SetImage("gfx/scenes/green_bg.png");
		Overlay[0].SetImage("gfx/overlays/learn_1.png");

		if (!Background)
			return false;

		if (GameOverBig)
			Overlay[1].SetText("Drag to scroll.", GameOverBig, &White, &Black);


		return true;
	}

	else if (Scene == SCENE_GAME_OVER)
	{
		ClearMessage();

		Background.SetImage("gfx/scenes/green_bg.png");

		if (Outcome == OUTCOME_WON)
			Overlay[0].SetImage("gfx/overlays/game_over_won.jpg");
		else
			Overlay[0].SetImage("gfx/overlays/game_over.png");

		switch (Outcome)
		{
		case	OUTCOME_WON:
			Overlay[1].SetText("Congrats! Click to start next game.", GameOverSmall, &White, &Black);
			break;
		case	OUTCOME_DRAW:
			Overlay[1].SetText("It's a draw! Click to start next game.", GameOverSmall, &White);
			break;
		case	OUTCOME_LOST:
			Overlay[1].SetText("Aw, shucks! Click to start next game.", GameOverSmall, &White);
			break;
		default:
			Overlay[1].SetText("Click to start next hand!", GameOverSmall, &White);
		}

		if (GameOverBig)
		{
			ScoreSurfaces[0][1].Clear();
			ScoreSurfaces[0][2].Clear();

			if (Outcome == OUTCOME_WON)
			{
				ScoreSurfaces[0][1].SetText("Human", GameOverBig, &White, &Black);
				ScoreSurfaces[0][2].SetText("CPU", GameOverBig, &White, &Black);
			}
			else
			{
				ScoreSurfaces[0][1].SetText("Human", GameOverBig);
				ScoreSurfaces[0][2].SetText("CPU", GameOverBig);
			}

			for (int i = 1; i < (SCORE_CATEGORY_COUNT + 1); ++i)
			{
				bool ShowRow = false;

				for (int j = 0; j < SCORE_COLUMN_COUNT; ++j)
				{
					ScoreSurfaces[i][j].Clear();

					if (j == 0)
					{
						if (Outcome == OUTCOME_WON)
						{
							for (int k = 0; k < PLAYER_COUNT; ++k)
							{
								if (ScoreBreakdown[k][i - 1] != 0)
								{
									ShowRow = true;
									break;
								}
							}

							if (ShowRow)
								ScoreSurfaces[i][j].SetText(SCORE_CAT_NAMES[i - 1], GameOverBig, &White, &Black);
						}
						else
						{
							ShowRow = true;
							ScoreSurfaces[i][j].SetText(SCORE_CAT_NAMES[i - 1], GameOverBig);
						}
					}
					else
					{
						int Score = ScoreBreakdown[j - 1][i - 1];

						if (Outcome == OUTCOME_WON)
						{
							if ((Score != 0) && ShowRow)
								ScoreSurfaces[i][j].SetInteger(Score, GameOverBig, (i >= (SCORE_CATEGORY_COUNT - 2)), &White, &Black);
						}
						else
							ScoreSurfaces[i][j].SetInteger(Score, GameOverBig, (i >= (SCORE_CATEGORY_COUNT - 2)));
					}
				}
			}
		}

		return true;
	}

	else if (Scene == SCENE_LEGAL)
	{
		SDL_Color	VersionRed = {230, 0, 11, 0};

		ResetPortal();

		Background.Clear();

		Overlay[0].SetImage("gfx/overlays/legal.png");

		if (GameOverBig)
			Overlay[1].SetText("Drag to scroll.", GameOverBig, &White, &Black);

		if (GameOverSmall)
			VersionSurface.SetText(VERSION_TEXT, GameOverSmall, &VersionRed);

		return true;
	}

	return false;
}

void	Game::OnLoop			(void)
{
	if (Message[0] != '\0')	//Clear message if necessary
	{
		if (((SDL_GetTicks() - 4000) > MessagedAt) && !IN_DEMO && (Scene != SCENE_GAME_OVER))
		{
			ClearMessage();
			Dirty = true;
		}
	}

	if (Scene == SCENE_GAME_PLAY)
	{
		if (SourceDeck)
			DeckCount = SourceDeck->CardsLeft();

		if (EndOfGame())
		{
			if (!Extended && !ExtensionDeclined)
			{
				if (Players[0].GetMileage() == 700) //Give player option to extend
				{
					if (Modal != MODAL_EXTENSION)
						ShowModal(MODAL_EXTENSION);
				}
				else if (Players[1].GetMileage() == 700) //Computer randomly decides whether to extend
				{
					Extended = ComputerDecideExtension();
					if (Extended)
					{
						ShowMessage("Computer extends trip");
						ChangePlayer();
					}
					else
						ExtensionDeclined = true;
				}
				else	//Hand ended with trip uncompleted
					ExtensionDeclined = true;

				Save();

				return;
			}

			Frozen = true;	//Freeze to prevent premature closing of score screen
			FrozenAt = SDL_GetTicks();

			GetScores();

			/* Determine outcome of hand */
			if (ScoreBreakdown[0][SCORE_CATEGORY_COUNT - 1] >= 5000)
			{
				Outcome = OUTCOME_WON;

				for (int i = 1; i < PLAYER_COUNT; ++i)
				{
					if (ScoreBreakdown[i][SCORE_CATEGORY_COUNT - 1] >= ScoreBreakdown[0][SCORE_CATEGORY_COUNT - 1])
					{
						if (ScoreBreakdown[i][SCORE_CATEGORY_COUNT - 1] > ScoreBreakdown[0][SCORE_CATEGORY_COUNT - 1])
						{
							Outcome = OUTCOME_LOST;
							break;
						}
						else
							Outcome = OUTCOME_DRAW;
					}
				}
			}
			else
			{
				for (int i = 1; i < PLAYER_COUNT; ++i)
				{
					if (ScoreBreakdown[i][SCORE_CATEGORY_COUNT - 1] >= 5000)
					{
						Outcome = OUTCOME_LOST;
						break;
					}
				}
			}

			LastScene = Scene;
			Scene = SCENE_GAME_OVER;	//Switch to score screen

			return;
		}

		if (Players[Current].IsOutOfCards())
			ChangePlayer();

		if (Current == 1)
			ComputerSmartMove();
	}
}

void	Game::OnPlay			(Uint8 Index)
{
	// DiscardedCard places the correct card on top of the discard pile after a Coup Fourre.
	Uint8 DiscardedCard = CARD_NULL_NULL;

	if (IsValidPlay(Index))
	{
		Uint8	Type = Players[Current].GetType(Index),
				Value = Players[Current].GetValue(Index);

		if ((Type == CARD_MILEAGE) || (Type == CARD_REMEDY) || (Type == CARD_SAFETY))
			DiscardedCard = Players[Current].OnPlay(Index);
		else
			Players[1 - Current].ReceiveHazard(Value);

		// We "discard" after playing, but the card doesn't actually go to the discard pile.
		Players[Current].Discard(Index);

		Dirty = true;	// Graphics will need to be re-drawn now

		if (Type != CARD_SAFETY)
			ChangePlayer();
		else	//Playing a safety gives us another turn.
		{
			if (DiscardedCard != CARD_NULL_NULL)
			{
				// A Coup Fourre bounced a card off the player's tableau. Put it on
				// the discard pile
				Animate(0, ANIMATION_COUP_FOURRE_BOUNCE, DiscardedCard);
				DiscardTop = DiscardedCard;
			}

			if (SourceDeck)
				// We immediately draw another card after playing a safety.
				Players[Current].Draw();
		}

		if (Value < CARD_NULL_NULL)
			++ExposedCards[Value];

		//Save game after every card played
		Save();
	}
}

void	Game::OnRender			(SDL_Surface *Target, bool Force, bool Flip)
{
	bool	RefreshedSomething =	false, // We only flip the display if something changed
			SceneChanged =			false; // Control variable. Do we need to call OnInit()?

	#ifdef DEBUG
	static	Uint32	LastReset = 0;
	static	Uint32	FrameCount = 0;

	++FrameCount;

	Uint32 TickCount = SDL_GetTicks();

	if ((LastReset + 333) < TickCount)
	{
		LastReset = TickCount;
		DebugSurface.SetInteger(FrameCount * 3, GameOverBig, true, &Black, &White);
		FrameCount = 0;
		RefreshedSomething = true;
	}
	#endif

	if ((Modal == MODAL_NONE) || Force)	//Don't re-render during modal, unless forced
	{
		// If the scene, discard pile, or deck count have changed, we need to do a refresh
		SceneChanged |= CheckForChange(OldDeckCount, DeckCount);
		SceneChanged |= CheckForChange(OldDiscardTop, DiscardTop);
		SceneChanged |= CheckForChange(LastScene, Scene);

		// Also if we're otherwise dirty
		Force |= Dirty;

		//Re-render the background if the hand has changed
		if ((Scene == SCENE_GAME_PLAY) && Players[0].IsDirty())
		{
			if (GameOptions.GetOpt(OPTION_CARD_CAPTIONS)) //With captions, we need to re-init
				SceneChanged = true;
			else	//Without captions, we just need to re-render
				Force = true;
		}

		if (SceneChanged)
			Force = true;

		if ((Scene == SCENE_GAME_PLAY) || IN_DEMO)
		{
			for (int i = (PLAYER_COUNT - 1); i >= 0; --i)
				RefreshedSomething |= Players[i].OnRender(Target, i, Force);
		}


		if (Force)
		{
			if (SceneChanged)
				OnInit(); //Refresh our surfaces

			Force = true;
			RefreshedSomething = true;

		
			// Render the appropriate surfaces
			Background.Render(0, 0, Target);

			if (Scene == SCENE_MAIN)
				LogoSurface.Render(Dimensions::ScreenWidth - LogoSurface.GetWidth(), Dimensions::ScreenHeight - LogoSurface.GetHeight(), Target);
			else if ((Scene == SCENE_GAME_PLAY) || IN_DEMO)
			{
				Overlay[0].Render(0, Dimensions::EffectiveTableauHeight - 1, Target, SCALE_NONE);
				Overlay[0].Render(0, (Dimensions::EffectiveTableauHeight * 2) - 1, Target, SCALE_NONE);

				DiscardSurface.Render(3, Dimensions::FirstRowY, Target);
				DrawCardSurface.Render(3, Dimensions::SecondRowY, Target);
				DrawTextSurface.Render(23 - (DrawTextSurface.GetWidth() / 2), Dimensions::SecondRowY + 33, Target);
			}
			else if (Scene == SCENE_GAME_OVER)
			{
				int X = 0, Y = 0;

				Overlay[0].Render(0, (Dimensions::ScreenHeight - Overlay[0].GetHeight()) / 2, Target);
				Overlay[1].Render((Dimensions::ScreenWidth - Overlay[1].GetWidth()) / 2, Dimensions::ScreenHeight - Overlay[1].GetHeight() - 12, Target);

				for (int i = 0; i < (SCORE_CATEGORY_COUNT + 1); ++i)
				{
					for (int j = 0; j < SCORE_COLUMN_COUNT; ++j)
					{
						if (ScoreSurfaces[i][j])
						{
							int Padding = 25;

							if (Dimensions::ScreenHeight < 480)
								Padding = 10;

							X = 12 + ((j > 0) ? 175 : 0) + ((j > 1) ? 75 : 0);
							Y = Padding + (i * 26) + ((i > 0) ? Padding : 0) + ((i > (SCORE_CATEGORY_COUNT - 3)) ? Padding : 0) + ((i > (SCORE_CATEGORY_COUNT - 1)) ? Padding : 0);
							ScoreSurfaces[i][j].Render(X, Y, Target);
						}
					}
				}
			}
			
			else if ((Scene == SCENE_LEARN_1) || (Scene == SCENE_LEGAL))
			{
				Overlay[0].DrawPart(Portal, Target);

				if ((Portal.x == 0) && (Portal.y == 0) && ((Overlay[0].GetWidth() > Dimensions::ScreenWidth) || (Overlay[0].GetHeight() > Dimensions::ScreenHeight)))
					Overlay[1].Render((Dimensions::ScreenWidth - Overlay[1].GetWidth()) / 2, Dimensions::ScreenHeight - Overlay[1].GetHeight() - 5, Target, SCALE_NONE);

				if (Scene == SCENE_LEGAL)
				{
					if (Portal.y < (VersionSurface.GetHeight() + 1))
						VersionSurface.Render(Dimensions::ScreenWidth - VersionSurface.GetWidth() - 1, 1 - Portal.y, Target, SCALE_NONE);

					Overlay[2].Render((Dimensions::ScreenWidth - Overlay[2].GetWidth()) / 2, 10, Target, SCALE_NONE);
				}
			}
		}

		if ((Scene == SCENE_GAME_PLAY) || IN_DEMO)
		{
			//Render card caption
			if (GameOptions.GetOpt(OPTION_CARD_CAPTIONS))
				CaptionSurface.Render((320 - CaptionSurface.GetWidth()) / 2, ((Dimensions::TableauHeight * 2) - CaptionSurface.GetHeight()) - 10, Target);

			if (Scene == SCENE_GAME_PLAY)
				MenuSurface.Render(2, 5, Target);
		}

		if (IN_TUTORIAL)
		{
			ArrowSurfaces[0].Render(5, 5, Target);	//Render back and forward arrows
			ArrowSurfaces[1].Render(240, 5, Target);

			if (Scene >= SCENE_LEARN_2)
			{
				Uint8 Index = Scene - SCENE_LEARN_2;

				if (Scene < SCENE_LEARN_6)	//Render orb
				{
					if (Index < 2)
						OrbSurface.Render((Dimensions::ScreenWidth - 41) / 2, (Dimensions::TableauHeight * Index) + ((Dimensions::TableauHeight - 41) / 2), Target, SCALE_Y);
					else
						OrbSurface.Render(146, Dimensions::FirstRowY + 8, Target);
				}

				if ((Scene >= SCENE_LEARN_4) && (Scene < SCENE_LEARN_7)) //Render hand icon
				{
					Index -= 2;
					HandSurface.Render(HAND_COORDS[Index], Dimensions::FirstRowY + 32, Target);
				}
			}
		}

		if (MessageSurface)
			MessageSurface.Render((Dimensions::ScreenWidth - MessageSurface.GetWidth()) / 2, Dimensions::TableauHeight - 50, Target, SCALE_Y); //Render the message last.

		if (Dragging && !Animating)
		{
			if (Players[Current].GetValue(DownIndex) == Card::GetMatchingSafety(Players[Current].GetQualifiedCoupFourre()))
				Tableau::ShadowSurfaceCF.Render(DragX - 20, DragY - 67, Target, SCALE_NONE);
			else
				Tableau::ShadowSurface.Render(DragX - 20, DragY - 67, Target, SCALE_NONE);

			FloatSurface.Render(DragX - 20, DragY - 67, Target, SCALE_NONE);
		}
	}

	#ifdef DEBUG
	DebugSurface.Render(0, 0, Target);
	#endif

	if (RefreshedSomething && Flip)
		SDL_Flip(Target);
}

void	Game::Pop				(Uint8 Index)
{
	if (Players[Current].IsPopped(Index) && IsValidPlay(Index))
	{
		// If the card is already popped, then play it (if it's a valid play)
		Animate(Index, ANIMATION_PLAY);
		Players[Current].UnPop(Index);
		OnPlay(Index);
	}
	else
		Players[Current].Pop(Index);	//If it's not already popped, pop it
}

void	Game::Reset				(bool SaveStats)
{
	if (SaveStats)
		PlayerStats.ProcessHand(Outcome, ScoreBreakdown[0][9], ScoreBreakdown[0][11]);

	//Reset my stuff
	if (SourceDeck)
		SourceDeck->Shuffle();
	else
		SourceDeck = new Deck;

	if (SourceDeck)
		OldDeckCount = DeckCount = SourceDeck->CardsLeft();

	for (int i = 0; i < PLAYER_COUNT; ++i)
	{
		RunningScores[i] += Scores[i]; //Roll up score
		Scores[i] = 0;
		
		for (int j = 0; j < SCORE_CATEGORY_COUNT; ++j)
			ScoreBreakdown[i][j] = 0;
	}

	//Reset running scores if the round is over
	for (int i = 0; i < PLAYER_COUNT; ++i)
	{
		if (RunningScores[i] >= 5000)
		{
			for (int j = 0; j < PLAYER_COUNT; ++j)
				RunningScores[j] = 0;

			break;
		}
	}

	for (int i = 0; i < CARD_NULL_NULL; ++i)
		ExposedCards[i] = 0;

	Dirty = true;
	Extended = false;
	ExtensionDeclined = false;

	Current = 0;

	OldDiscardTop = DiscardTop = CARD_NULL_NULL;

	//Reset down the chain
	for (int i = 0; i < PLAYER_COUNT; ++i)
	{
		Players[i].Reset();
		if (SourceDeck)
			Players[i].SetSource(SourceDeck);
	}

	for (int i = 0; i < 13; ++i) //Staggered deal
		Players[i % 2].Draw();

	//Odds and ends
	if (SourceDeck)
		OldDeckCount = DeckCount = SourceDeck->CardsLeft();

	Outcome = OUTCOME_NOT_OVER;
}

void	Game::ResetPortal		(void)
{
	Portal.x = 0;
	Portal.y = 0;
	Portal.w = Dimensions::ScreenWidth;
	Portal.h = Dimensions::ScreenHeight;
}

bool	Game::Restore			(void)
{
	using namespace std;

	bool Success = false;
	ifstream SaveFile ("game.sav", ios::in | ios::binary);

	if (SaveFile.is_open())
	{
		SaveFile.seekg(0);

		int SaveVersion = 0;
		SaveFile.read((char *) &SaveVersion, sizeof(int));

		if (SaveVersion == SAVE_FORMAT_VER)
		{
			SaveFile.read((char *) &Current, sizeof(Uint8));
			SaveFile.read((char *) &RunningScores, sizeof(int) * 2);
			SaveFile.read((char *) &DiscardTop, sizeof(Uint8));
			SaveFile.read((char *) &Extended, sizeof(bool));
			SaveFile.read((char *) &ExtensionDeclined, sizeof(bool));

			SourceDeck->Restore(SaveFile);

			for (int i = 0; i < PLAYER_COUNT; ++i)
				Players[i].Restore(SaveFile);
			
			Success = SaveFile.good();
		}
		else
			Success = false;

		SaveFile.close();
	}

	return Success;	
}

bool	Game::Save				(void)
{
	using namespace std;

	bool Success = false;
	ofstream SaveFile ("game.sav", ios::out | ios::binary);

	if (SaveFile.is_open())
	{
		SaveFile.seekp(0);
		SaveFile.write((char *) &SAVE_FORMAT_VER, sizeof(int));
		SaveFile.write((char *) &Current, sizeof(Uint8));
		SaveFile.write((char *) &RunningScores, sizeof(int) * PLAYER_COUNT);
		SaveFile.write((char *) &DiscardTop, sizeof(Uint8));
		SaveFile.write((char *) &Extended, sizeof(bool));
		SaveFile.write((char *) &ExtensionDeclined, sizeof(bool));
		
		SourceDeck->Save(SaveFile);

		for (int i = 0; i < PLAYER_COUNT; ++i)
			Players[i].Save(SaveFile);

		Success = SaveFile.good();
		SaveFile.close();
	}

	return Success;	

}

void	Game::ShowLoading		(void)
{
	Overlay[0].Render((Dimensions::ScreenWidth - Overlay[0].GetWidth()) / 2, (Dimensions::ScreenHeight - Overlay[0].GetHeight()) / 2, Window, SCALE_NONE);
	SDL_Flip(Window);
}

void	Game::ShowMessage		(const char * Msg, bool SetDirty)
{
	if (strlen(Msg) < MESSAGE_SIZE)
	{
		strcpy(Message, Msg);
		MessagedAt = SDL_GetTicks();
		Dirty |= SetDirty;
	}
}

bool	Game::ShowModal			(Uint8 ModalName)
{
	if (ModalName < MODAL_NONE)
	{
		Modal = ModalName;

		Surface::Draw(Window, Surface::Load("gfx/modals/shadow.png"), 0, 0, true);	//Render shadow

		switch(Modal)
		{
		case MODAL_EXTENSION:
			ModalSurface.SetImage("gfx/modals/extension.png");
			ModalSurface.Render(74, 193, Window);
			break;
		case MODAL_GAME_MENU:
			OnRender(Window, true, false); //Re-render the background, but don't flip it.

			ModalSurface.SetImage("gfx/modals/menu_top.png");
			ModalSurface.Render(40, 80, Window);
			for (int i = 0; i < (OPTION_COUNT + MENU_ITEM_COUNT); ++i)	//Render options and other menu items
			{
				if (i < OPTION_COUNT)
				{
					MenuSurfaces[i][0].SetText(OPTION_NAMES[i], GameOverBig, &White);
					MenuSurfaces[i][1].SetText((GameOptions.GetOpt(i)) ? "ON" : "OFF", GameOverBig, &White);
					MenuSurfaces[i][1].Render(240, 120 + (i * 40), Window);
				}
				else
					MenuSurfaces[i][0].SetText(MENU_ITEM_NAMES[i - OPTION_COUNT], GameOverBig, &White);

				MenuSurfaces[i][0].Render(50, 120 + (i * 40), Window);
			}
			break;
		case MODAL_NEW_GAME:
			ModalSurface.SetImage("gfx/modals/quit.png");
			ModalSurface.Render(60, 165, Window);
			break;
		}		

		SDL_Flip(Window);

		return true;
	}

	return false;
}

}

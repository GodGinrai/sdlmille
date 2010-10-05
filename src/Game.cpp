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
	GameOverFont = DrawFont = 0;

	Modal = MODAL_NONE;
	Scene = SCENE_MAIN;
	LastScene = SCENE_INVALID;

	Dirty = true;
	Extended = false;
	ExtensionDeclined = false;
	Frozen = false;
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

	Message[0] = '\0';

	DrawFont = TTF_OpenFont("LiberationMono-Regular.ttf", 16);
	GameOverFont = TTF_OpenFont("LiberationMono-Regular.ttf", 18);
}

		Game::~Game				(void)
{
	GameOptions.SaveOpts();

	// Clean up all of our pointers
	if (SourceDeck)
		delete SourceDeck;
	if (Window)
		SDL_FreeSurface(Window);
	if (DrawFont)
		TTF_CloseFont(DrawFont);
	if (GameOverFont)
		TTF_CloseFont(GameOverFont);

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
		{
			OnEvent(&Event);
		}

		// We have to render twice because the computer moves during OnLoop() and we
		// want to render the human's move before the 500ms delay. Hopefully this will
		// be fixed soon.
		OnRender(); 
		OnLoop();
		OnRender();
		SDL_Delay(25);
	}
	
	return true;

}

/* Private methods */

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

void	Game::ClearMessage		(void)
{
	Message[0] = '\0';
	MessagedAt = 0;
	MessageSurface.Clear();
	Dirty = true;
}

void	Game::ComputerMove		(void)
{
	bool Played = false;

	for (int i = 0; i < HAND_SIZE; ++i)
	{	
		if (IsValidPlay(i))	// Play the first valid move we find (the computer is currently stupid)
		{
			OnPlay(i);
			Played = true;
			break;
		}
	}

	if (Played == false)
	{
		for (int i = 0; i < HAND_SIZE; ++i)	//No valid moves, discard
		{
			Pop(i);
			if (Discard())
				break;
		}
	}
}

bool	Game::Discard			(void)
{
	bool Success =	false;

	Uint8	Value =	CARD_NULL_NULL,
			Index =	FindPopped(); // Find out which card is popped

	if (Index < HAND_SIZE)
	{
		Value	= Players[Current].GetValue(Index);
		Success = Players[Current].Discard(Index);

		if (Success)
		{
			if (Value != CARD_NULL_NULL)	// Sanity check
				DiscardTop = Value;	// Put the card on top of the discard pile

			Players[Current].UnPop(Index);
			ChangePlayer();
		}
	}

	return Success;
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
				if (Players[i].Get200Count() == 0)
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

bool	Game::IsValidPlay		(Uint8 Index)						const
{
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

		if ((Value == CARD_MILEAGE_200) && (Players[Current].Get200Count() > 1))
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

void	Game::OnClick			(int X, int Y)
{
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
							ShowModal(MODAL_MAIN_MENU);
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
				}
			}
		}
		else if ((Modal == MODAL_NEW_GAME) || (Modal == MODAL_MAIN_MENU))
		{
			if ((X >= 65) && (X <= 255) && (Y >= 220) && (Y <= 265))	// Clicked Cancel
				ShowModal(MODAL_GAME_MENU);
			else if ((X >= 130) && (X <= 190) && (Y >= 281) && (Y <= 311))	//Clicked Confirm
			{
				Reset();
				for (int i = 0; i < PLAYER_COUNT; ++i)
					RunningScores[i] = 0;

				if (Modal == MODAL_MAIN_MENU)
				{
					LastScene = Scene;
					Scene = SCENE_MAIN;
				}

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
				Reset();
				LastScene = SCENE_MAIN;
				Scene =		SCENE_GAME_PLAY;
			}
			if ((Y >= 370) && (Y <= 415))	//Clicked Learn
			{
				LastScene = SCENE_MAIN;
				Scene =		SCENE_LEARN_1;
			}
		}
		if ((X >= 232) && (Y >= 449))		//Clicked GPL logo
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
			if (Y < 175)	//Clicked within opponent's tableau
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
			else if ((Y > 175) && (Y < 350))	//Clicked within own tableau
				Pop(FindPopped());	//Play selected card, if any
			else if ((Y >= 358) && (Y <= 476))	//Clicked within the two rows at the bottom of the screen
			{
				Uint8	Add = 0,
						Index = 0;

				if (Y >= 425)	// Clicked the bottom row. Add 4 to the index
					Add = 4;
				else if (Y > 410)	//Clicked in the dead zone
					return; 

				if (X >= 86)	//Clicked within hand
				{
					if (((X - 84) % 65) <= 35)	//Click was not in horizontal dead zone
					{
						Index = ((X - 84) / 65) + Add;

						if (Index > 0)	//Clicked a card, so pop it
							Pop(Index - 1);
						else //Clicked the X, so cancel selection/click
						{
							Players[0].UnPop(FindPopped());
							return;
						}
					}
				}
				else if ((X >= 3) && (X <= 43) && (Y <= 410))	//Clicked the discard pile
					Discard();
			}
		}
	}

	else if (Scene == SCENE_GAME_OVER)	//Score screen, start new game on click
	{
		Reset();
		LastScene = Scene;
		Scene = SCENE_GAME_PLAY;
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
		LastScene = Scene;
		Scene = SCENE_MAIN;
	}
}

void	Game::OnEvent			(SDL_Event * Event)
{
	int	X = 0, Y = 0;

	if (Event != 0)
	{
		if (Event->type == SDL_QUIT)
			Running = false;
		else if (Event->type == SDL_MOUSEBUTTONUP)	//Mouse click
		{
			X = Event->button.x;
			Y = Event->button.y;
			#ifdef	ANDROID_DEVICE		//Fix X and Y for WVGA
			if ((Y < 40) || (Y > 760))
				return;
			else
			{
				X = (X / 1.5);
				Y = ((Y - 40) / 1.5);
			}
			#endif
			OnClick(X, Y);
		}
		else if (Event->type == SDL_KEYUP)	//Debugging purposes
		{
			//Players[1].ReceiveHazard(rand() % 5);
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
		#elif defined ANDROID_DEVICE
		if(!(Window = SDL_SetVideoMode(480, 800, 16, SDL_SWSURFACE)))
		#else
		if(!(Window = SDL_SetVideoMode(320, 480, 32, SDL_HWSURFACE | SDL_DOUBLEBUF)))
		#endif
			return false;

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
	{
		if (GameOverFont)
			MessageSurface.SetText(Message, GameOverFont);
	}

	if (Scene == SCENE_MAIN)
	{
		Background.SetImage("gfx/scenes/main.png");
		if (!Background)
			return false;

		LogoSurface.SetImage("gfx/gpl.png");

		return true;
	}

	else if ((Scene == SCENE_GAME_PLAY) || (Scene == SCENE_LEARN_2))
	{
		Background.SetImage("gfx/scenes/game-play.png");

		DiscardSurface.SetImage(Card::GetFileFromValue(DiscardTop));

		DrawCardSurface.SetImage(Card::GetFileFromValue(CARD_NULL_NULL));

		if (DrawFont)
		{
			DrawTextSurface.SetInteger(DeckCount, DrawFont);
			if (FindPopped() < HAND_SIZE)
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
		Background.SetImage("gfx/scenes/the-cards.png");

		if (!Background)
			return false;

		return true;
	}

	else if (Scene == SCENE_GAME_OVER)
	{
		Background.SetImage("gfx/scenes/game_over.png");

		if (GameOverFont)
		{
			ScoreSurfaces[0][1].SetText("Human", GameOverFont);
			ScoreSurfaces[0][2].SetText("CPU", GameOverFont);

			for (int i = 1; i < (SCORE_CATEGORY_COUNT + 1); ++i)
			{
				for (int j = 0; j < SCORE_COLUMN_COUNT; ++j)
				{
					if (j == 0)
						ScoreSurfaces[i][j].SetText(SCORE_CAT_NAMES[i - 1], GameOverFont);
					else
						ScoreSurfaces[i][j].SetInteger(ScoreBreakdown[j - 1][i - 1], GameOverFont, (i >= (SCORE_CATEGORY_COUNT - 2)));
				}
			}
		}

		return true;
	}

	else if (Scene == SCENE_LEGAL)
	{
		Background.SetImage("gfx/scenes/legal.png");
		if (!Background)
			return false;

		VersionSurface.SetText(VERSION_TEXT, DrawFont, 230, 0, 11);

		return true;
	}

	return false;
}

void	Game::OnLoop			(void)
{
	if (Message[0] != '\0')	//Clear message if necessary
	{
		if (((SDL_GetTicks() - 2500) > MessagedAt) && !IN_DEMO)
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
					srand(time(0));
					if (rand() % 2)
					{
						Extended = true;
						ShowMessage("Computer extends trip");
						ChangePlayer();
					}
					else
						ExtensionDeclined = true;
				}
				else	//Hand ended with trip uncompleted
					ExtensionDeclined = true;

				return;
			}

			Frozen = true;	//Freeze to prevent premature closing of score screen
			FrozenAt = SDL_GetTicks();

			GetScores();

			LastScene = Scene;
			Scene = SCENE_GAME_OVER;	//Switch to score screen

			return;
		}

		if (Players[Current].IsOutOfCards())
			ChangePlayer();

		if (Current == 1)
		{
			// Delay for a moment, then the computer makes a move
			SDL_Delay((GameOptions.GetOpt(OPTION_FAST_GAME)) ? 200 : 500);

			ComputerMove();
		}
	}
}

void	Game::OnPlay			(Uint8 Index)
{
	// DiscardedCard places the correct card on top of the discard pile after a Coup Fourre.
	Uint8 DiscardedCard = CARD_NULL_NULL;

	if (IsValidPlay(Index))
	{
		Uint8 Type = Players[Current].GetType(Index);

		if ((Type == CARD_MILEAGE) || (Type == CARD_REMEDY) || (Type == CARD_SAFETY))
			DiscardedCard = Players[Current].OnPlay(Index);
		else
			Players[1 - Current].ReceiveHazard(Players[Current].GetValue(Index));

		// We "discard" after playing, but the card doesn't actually go to the discard pile.
		Players[Current].Discard(Index);

		Dirty = true;	// Graphics will need to be re-drawn now

		if (Type != CARD_SAFETY)
			ChangePlayer();
		else	//Playing a safety gives us another turn.
		{
			if (DiscardedCard != CARD_NULL_NULL)
				// A Coup Fourre bounced a card off the player's tableau. Put it on
				// the discard pile
				DiscardTop = DiscardedCard;

			if (SourceDeck)
				// We immediately draw another card after playing a safety.
				Players[Current].Draw();
		}
	}
}

void	Game::OnRender			(bool Force, bool Flip)
{
	if ((Modal == MODAL_NONE) || Force)	//Don't re-render during modal, unless forced
	{
		bool	RefreshedSomething =	false, // We only flip the display if something changed
				SceneChanged =			false; // Control variable. Do we need to call OnInit()?

		#ifdef	ANDROID_DEVICE
		static	SDL_Rect	SceneRect = {0, 40, 480, 720};
		#endif

		// If the scene, discard pile, or deck count have changed, we need to do a refresh
		SceneChanged |= CheckForChange(OldDeckCount, DeckCount);
		SceneChanged |= CheckForChange(OldDiscardTop, DiscardTop);
		SceneChanged |= CheckForChange(LastScene, Scene);

		// Also if we're otherwise dirty
		SceneChanged |= Dirty;

		//Re-render the background if the hand has changed
		if ((Scene == SCENE_GAME_PLAY) && Players[0].IsDirty())
		{
			if (GameOptions.GetOpt(OPTION_CARD_CAPTIONS)) //With captions, we need to re-init
				SceneChanged = true;
			else	//Without captions, we just need to re-render
				Force = true;
		}

		if (SceneChanged || Force)
		{
			if (SceneChanged)
				OnInit(); //Refresh our surfaces

			Force = true;
			RefreshedSomething = true;

			// TODO: Remove the following hack, necessitated by transparency present in the enlarged graphics
			#ifdef	ANDROID_DEVICE
			SDL_FillRect(Window, &SceneRect, SDL_MapRGB(Window->format, 120, 192, 86));
			#endif
			
			// Render the appropriate surfaces
			Background.Render(0, 0, Window);

			if (Scene == SCENE_MAIN)
				LogoSurface.Render(232, 449, Window);
			else if ((Scene == SCENE_GAME_PLAY) || IN_DEMO)
			{
				DiscardSurface.Render(3, 358, Window);
				DrawCardSurface.Render(3, 420, Window);
				DrawTextSurface.Render(23 - (DrawTextSurface.GetWidth() / 2), 440, Window);
			}
			else if (Scene == SCENE_GAME_OVER)
			{
				int X = 0, Y = 0;

				for (int i = 0; i < (SCORE_CATEGORY_COUNT + 1); ++i)
				{
					for (int j = 0; j < SCORE_COLUMN_COUNT; ++j)
					{
						if (ScoreSurfaces[i][j])
						{
							X = 12 + ((j > 0) ? 175 : 0) + ((j > 1) ? 75 : 0);
							Y = 20 + (i * 26) + ((i > 0) ? 20 : 0) + ((i > (SCORE_CATEGORY_COUNT - 3)) ? 20 : 0) + ((i > (SCORE_CATEGORY_COUNT - 1)) ? 20 : 0);
							ScoreSurfaces[i][j].Render(X, Y, Window);
						}
					}
				}
			}
			else if (Scene == SCENE_LEARN_1)
			{
				printf("Drew cards\n");
				for (int i = 0; i < CARD_MILEAGE_25; ++i)
					Surface::Draw(Window, Surface::Load(Card::GetFileFromValue(i)), 135 + ((i / 5) * 64), 113 + ((i % 5) * 64) + ((i == CARD_SAFETY_RIGHT_OF_WAY) ? 32 : 0), true);
			}
			else if (Scene == SCENE_LEGAL)
				VersionSurface.Render(315 - VersionSurface.GetWidth(), 1, Window);
		}

		if ((Scene == SCENE_GAME_PLAY) || IN_DEMO) // During play, we also need to render our players
		{
			//Force compels players to re-render if this function re-rendered
			RefreshedSomething |= Players[0].OnRender(Window, 0, Force);
			RefreshedSomething |= Players[1].OnRender(Window, 1, Force);

			//Render caption over hand
			if (GameOptions.GetOpt(OPTION_CARD_CAPTIONS))
				CaptionSurface.Render((320 - CaptionSurface.GetWidth()) / 2, (350 - CaptionSurface.GetHeight()) - 10, Window);
			if (Scene == SCENE_GAME_PLAY)
				MenuSurface.Render(2, 5, Window);
		}

		if (IN_TUTORIAL)
		{
			ArrowSurfaces[0].Render(5, 5, Window);	//Render back and forward arrows
			ArrowSurfaces[1].Render(240, 5, Window);

			if (Scene >= SCENE_LEARN_2)
			{
				Uint8 Index = Scene - SCENE_LEARN_2;

				if (Scene < SCENE_LEARN_6)	//Render orb
					OrbSurface.Render(ORB_COORDS[Index][0], ORB_COORDS[Index][1], Window);

				if ((Scene >= SCENE_LEARN_4) && (Scene < SCENE_LEARN_7)) //Render hand icon
				{
					Index -= 2;
					HandSurface.Render(HAND_COORDS[Index][0], HAND_COORDS[Index][1], Window);
				}
			}
		}

		MessageSurface.Render(((320 - MessageSurface.GetWidth()) / 2), 125, Window); //Render the message last.

		if (RefreshedSomething && Flip)
			SDL_Flip(Window);
	}
}

void	Game::Pop				(Uint8 Index)
{
	if (Players[Current].IsPopped(Index) && IsValidPlay(Index))
	{
		// If the card is already popped, then play it (if it's a valid play)
		Players[Current].UnPop(Index);
		OnPlay(Index);
	}
	else
		Players[Current].Pop(Index);	//If it's not already popped, pop it
}

void	Game::Reset				(void)
{
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
		int R, G, B;
		R = G = B = 255;	//Set text color

		Modal = ModalName;

		Surface::Draw(Window, Surface::Load("gfx/modals/shadow.png"), 0, 0, true);	//Render shadow

		switch(Modal)
		{
		case MODAL_EXTENSION:
			ModalSurface.SetImage("gfx/modals/extension.png");
			ModalSurface.Render(74, 193, Window);
			break;
		case MODAL_GAME_MENU:
			OnRender(true, false); //Re-render the background, but don't flip it.

			ModalSurface.SetImage("gfx/modals/menu_top.png");
			ModalSurface.Render(40, 80, Window);
			for (int i = 0; i < (OPTION_COUNT + MENU_ITEM_COUNT); ++i)	//Render options and other menu items
			{
				if (i < OPTION_COUNT)
				{
					MenuSurfaces[i][0].SetText(OPTION_NAMES[i], GameOverFont, R, G, B);
					MenuSurfaces[i][1].SetText((GameOptions.GetOpt(i)) ? "ON" : "OFF", GameOverFont, R, G, B);
					MenuSurfaces[i][1].Render(240, 120 + (i * 40), Window);
				}
				else
					MenuSurfaces[i][0].SetText(MENU_ITEM_NAMES[i - OPTION_COUNT], GameOverFont, R, G, B);

				MenuSurfaces[i][0].Render(50, 120 + (i * 40), Window);
			}
			break;
		case MODAL_NEW_GAME:
		case MODAL_MAIN_MENU:
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

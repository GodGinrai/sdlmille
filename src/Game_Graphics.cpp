#include "Game.h"

namespace	_SDLMille
{

void	Game::Animate			(Uint8 Index, Uint8 AnimationType, Uint8 Value)
{
	bool	CoupFourre	= false;
	Uint8	PileCount	= 0;
	char	DurationMsg[41];
	Uint32	StartTicks	= SDL_GetTicks(),
			LastLocalBlit = 0,
			Duration;
	int		TargetFrames = 
			#ifdef SOFTWARE_MODE
			12;
			#else
			50;
			#endif

	if (!GameOptions.GetOpt(OPTION_ANIMATIONS))
		return;

	if (AnimationType >= ANIMATION_INVALID)
		return;

	if ((AnimationType >= ANIMATION_COUP_FOURRE_BOUNCE) || ((Index < HAND_SIZE) && (Index == FindPopped()) && (IsValidPlay(Index) || (AnimationType != ANIMATION_PLAY))))
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

		if (AnimationType < ANIMATION_COUP_FOURRE_BOUNCE)
		{
			Value = Players[Current].GetValue(Index);
			Players[Current].Detach(Index);
		}

		Type = Card::GetTypeFromValue(Value);

		if (Type < CARD_NULL)
		{
			if (Dragging && (AnimationType < ANIMATION_COUP_FOURRE_BOUNCE))
			{
				StartX = (DragX - 20) / Dimensions::ScaleFactor;
				StartY = (DragY - 67) / Dimensions::ScaleFactor;
			}
			else
			{
				if (AnimationType == ANIMATION_COUP_FOURRE_BOUNCE)
					Tableau::GetTargetCoords(Value, Current, StartX, StartY, false);
				else if (AnimationType == ANIMATION_SAFETY_SPAWN)
				{
					Uint8	SafetyValue = 0;

					if (Value == CARD_REMEDY_ROLL)
						SafetyValue = CARD_SAFETY_RIGHT_OF_WAY;
					else
						SafetyValue = Value + 5;

					Tableau::GetTargetCoords(SafetyValue, Current, StartX, StartY, false);
				}
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
			else if ((AnimationType == ANIMATION_PLAY) || (AnimationType == ANIMATION_SAFETY_SPAWN))
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

			#ifdef SOFTWARE_MODE
			Backdrop = SDL_CreateRGBSurface(SDL_SWSURFACE, Dimensions::ScreenWidth, Dimensions::ScreenHeight, Window->format->BitsPerPixel, 0, 0, 0, 0);
			#else
			Backdrop = SDL_CreateRGBSurface(SDL_HWSURFACE, Dimensions::ScreenWidth, Dimensions::ScreenHeight, Window->format->BitsPerPixel, 0, 0, 0, 0);
			#endif

			if (Backdrop != 0)
				OnRender(Backdrop, true, false);

			while ((X != DestX) || (Y != DestY))
			{
				//CheckTableau(Backdrop);

				IgnoreEvents();

				#ifndef	SOFTWARE_MODE
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

bool	Game::OnInit			(void)
{
	char	DebugStr	[101];
	Dirty = false;

	if (!Window)
	{
		// Set up our display if we haven't already
		if(SDL_Init(SDL_INIT_VIDEO) < 0)
			return false;

		#ifdef SOFTWARE_MODE
		if(!(Window = SDL_SetVideoMode(0, 0, 0, SDL_SWSURFACE)))
		#else
		if(!(Window = SDL_SetVideoMode(1024, 768, 32, SDL_HWSURFACE | SDL_DOUBLEBUF)))
		#endif
			return false;

		DiscardSurface.SetImage("gfx/null_null.png");

		UpdateMetrics();

		ResetPortal();

		SDL_WM_SetCaption("SDL Mille", "SDL Mille");

		sprintf(DebugStr, "0: %i | 1: %i\n", SDL_BUTTON(0), SDL_BUTTON(1));
		DEBUG_PRINT(DebugStr);
	}

	int ScreenHeight = Dimensions::ScreenHeight,
		ScreenWidth = Dimensions::ScreenWidth;

	if (Modal < MODAL_NONE)
	{
		ShadowSurface.SetImage("gfx/modals/shadow.png");	//Render shadow

		if (Modal == MODAL_EXTENSION)
		{
			ModalSurface.SetImage("gfx/modals/question_box.png");
			Overlay[3].SetText("EXTEND TRIP?", GameOverBig, &White);
			Overlay[4].SetImage("gfx/modals/menu_check.png");
			Overlay[5].SetImage("gfx/modals/menu_x.png");

			int BoxHeight = ModalSurface.GetHeight(),
				BoxWidth = ModalSurface.GetWidth(),
				BoxTop = (Dimensions::ScreenHeight - BoxHeight) / 2,
				BoxLeft = (Dimensions::ScreenWidth - BoxWidth) / 2,
				BoxCenterX = BoxLeft + (BoxWidth / 2),
				BoxCenterY = BoxTop + (BoxHeight / 2),
				CircleRadius = Overlay[4].GetWidth() / 2;

			ModalSurface.SetCoords(BoxLeft, BoxTop);

			Overlay[3].SetCoords(BoxCenterX - (Overlay[3].GetWidth() / 2), BoxTop + ((BoxCenterY - BoxTop - Overlay[3].GetHeight()) / 2));
			Overlay[4].SetCoords(BoxLeft + (((BoxWidth / 2) - Overlay[4].GetWidth()) / 2), BoxCenterY + (((BoxHeight / 2) - Overlay[4].GetHeight()) / 2));
			Overlay[5].SetCoords(BoxCenterX + (((BoxWidth / 2) - Overlay[5].GetWidth()) / 2), BoxCenterY + (((BoxHeight / 2) - Overlay[5].GetHeight()) / 2));

			return true;
		}
		else if (Modal <= MODAL_OPTIONS)
		{
			int	TextRowCount = 0;

			// Menu modals
			ModalSurface.SetImage("gfx/modals/menu_bg.png");
			//ModalSurface.SetCoords((ScreenWidth - ModalSurface.GetWidth()) / 2, (ScreenHeight - ModalSurface.GetHeight()) / 2 );
			
			//Dimensions::Menu2ndColumnX = ModalSurface.GetX() + (ModalSurface.GetWidth() * 0.8);
			Dimensions::SetMenuMetrics(ModalSurface.GetWidth(), ModalSurface.GetHeight());

			ModalSurface.SetCoords(Dimensions::MenuX, Dimensions::MenuY);

			//const	int	BORDER_PADDING = ModalSurface.GetWidth() / 30,
			//const	int	LEFT_RENDER_X = ModalSurface.GetX() + BORDER_PADDING;

			Overlay[3].SetY(Dimensions::MenuY + Dimensions::MenuBorderPadding);

			/*
			Overlay[1].SetText("Current game's difficulty:", GameOverSmall, &White, &Black);

			if (Difficulty < DIFFICULTY_LEVEL_COUNT)
				Overlay[2].SetText(DIFFICULTY_TEXT[Difficulty], GameOverBig, &White, &Black);
			else
				Overlay[2].SetText("Error", GameOverBig, &White);
			*/

			if (Modal == MODAL_GAME_MENU)
			{
				char	CurrScoreText[30];

				bool	BoundsSafe = true;

				TextRowCount = MENU_SURFACE_COUNT;

				for (int i = 0; i < PLAYER_COUNT; ++i)
				{
					if ((RunningScores[i] < 0) || (RunningScores[i] >= 10000))
						BoundsSafe = false;
				}

				if (strlen(DIFFICULTY_TEXT[Difficulty]) > 6)
					BoundsSafe = false;

				if (BoundsSafe && (PLAYER_COUNT > 1))
					sprintf(CurrScoreText, "H:%u/%u (%s)", RunningScores[0], RunningScores[1], DIFFICULTY_TEXT[Difficulty]); // TODO: Add game difficulty here
				else
					CurrScoreText[0] = '\0';

				//printf("%s\n", CurrScoreText);

				Overlay[3].SetImage("gfx/modals/menu_x.png");
				Overlay[3].SetX(Dimensions::MenuX + ModalSurface.GetWidth() - Overlay[3].GetWidth() - Dimensions::MenuBorderPadding);

				for (int i = 0; i < MENU_SURFACE_COUNT; ++i)	//Render options and other menu items
				{
					if (i < MENU_ITEM_COUNT)
						MenuSurfaces[i][0].SetText(MENU_ITEM_NAMES[i], GameOverBig, &White);
					else if (i == (MENU_SURFACE_COUNT - 1))
						MenuSurfaces[i][0].SetText(CurrScoreText, GameOverBig, &White);
					else
						MenuSurfaces[i][0].Clear();

					MenuSurfaces[i][1].Clear();
				}
			}
			else
			{
				Overlay[3].SetImage("gfx/modals/menu_back.png");
				Overlay[3].SetX(Dimensions::MenuColumn1X);

				if (Modal == MODAL_STATS)
				{
					Uint32	Statistics[7];

					TextRowCount = STAT_CAPTIONS_SIZE;

					PlayerStats.GetStats(Statistics[0], Statistics[1], Statistics[2], Statistics[3], Statistics[4], Statistics[5], Statistics[6]);

					for (int i = 0; i < STAT_CAPTIONS_SIZE; ++i)
					{
						MenuSurfaces[i][0].SetText(STAT_CAPTIONS[i], GameOverBig, &White);
						MenuSurfaces[i][1].SetInteger(Statistics[i], GameOverBig, true, &White);
					}
				}

				else
				{
					// Options
					TextRowCount = OPTION_COUNT;
					Dimensions::MenuColumn2X = ModalSurface.GetX() + (ModalSurface.GetWidth() * 0.825); //TODO: Smart column placement

					//Overlay[1].SetText("Current game's difficulty:", GameOverSmall, &White, &Black);

					//if (Difficulty < DIFFICULTY_LEVEL_COUNT)
					//	Overlay[2].SetText(DIFFICULTY_TEXT[Difficulty], GameOverBig, &White, &Black);
					//else
					//	Overlay[2].SetText("Error", GameOverBig, &White);

					for (int i = 0; i < OPTION_COUNT; ++i)
					{
						MenuSurfaces[i][0].SetText(OPTION_NAMES[i], GameOverBig, &White);
						MenuSurfaces[i][1].SetText((GameOptions.GetOpt(i)) ? "ON" : "OFF", GameOverBig, &White);
					}
				}
			}

			//NEW
			//const	int	MENU_ITEMS_TOP = ModalSurface.GetY() + (ModalSurface.GetHeight() * 0.18),
			//			MENU_ITEMS_SPACING = ModalSurface.GetHeight() * 0.12;

			for (int i = 0; i < TextRowCount; ++i)	//Render options and other menu items
			{
				MenuSurfaces[i][0].SetCoords(Dimensions::MenuColumn1X, Dimensions::MenuItemsTopY + (i * Dimensions::MenuItemSpacing));
				MenuSurfaces[i][1].SetCoords(Dimensions::MenuColumn2X, Dimensions::MenuItemsTopY + (i * Dimensions::MenuItemSpacing));
			}

			//Overlay[1].Render((Dimensions::ScreenWidth - Overlay[1].GetWidth()) / 2, 10, Target, SCALE_Y);
			//Overlay[2].Render((Dimensions::ScreenWidth - Overlay[2].GetWidth()) / 2, 20 + Overlay[1].GetHeight(), Target, SCALE_Y);

			//END NEW

			return true;
		}
		else if (Modal <= MODAL_CLEAR_STATS)
		{
			ModalSurface.SetImage("gfx/modals/warn_confirm.png");

			if (Modal == MODAL_NEW_GAME)
			{
				Overlay[3].SetText("Current game will", GameOverBig, &White);
				Overlay[4].SetText("be lost!", GameOverBig, &White);
			}
			else
			{
				Overlay[3].SetText("All statistics", GameOverBig, &White);
				Overlay[4].SetText("will be erased!", GameOverBig, &White);
			}

			ModalSurface.Center(CENTER_X_Y);

			Overlay[3].Center(CENTER_X);
			Overlay[3].SetY(ModalSurface.GetY() + 5);

			Overlay[4].Center(CENTER_X);
			Overlay[4].SetY(Overlay[3].GetY() + 5 + Overlay[3].GetHeight());

			return true;
		}

		return false;
	}
	else
	{
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
			SDL_Color MessageBg;

			if ((Scene == SCENE_GAME_PLAY) && Players[1].IsRolling())
			{
				if (Players[1].IsLimited())
					MessageBg = Yellow;
				else
					MessageBg = Green;
			}
			else
				MessageBg = Red;

			MessageSurface.SetText(Message, GameOverBig, &Black, &MessageBg);
		}

		if (Scene == SCENE_MAIN)
		{
			Background.SetImage("gfx/scenes/green_bg.png");
			if (!Background)
				return false;

			Overlay[0].SetImage("gfx/overlays/main.png");
			Overlay[1].SetImage("gfx/loading.png");
			Overlay[2].SetImage("gfx/overlays/main_play.png");
			Overlay[3].SetImage("gfx/overlays/main_learn.png");

			LogoSurface.SetImage("gfx/gpl.png");
			
			int	BannerHeight = Overlay[0].GetHeight(),
				ButtonsHeight = Overlay[2].GetHeight() + Overlay[3].GetHeight(),
				LogoHeight = LogoSurface.GetHeight();

			if ((BannerHeight + ButtonsHeight + LogoHeight + 30) > ScreenHeight)
			{
				LogoSurface.SetImage("gfx/gpl_sideways.png");
				LogoHeight = 0;
			}
						
			if ((BannerHeight + ButtonsHeight) > ScreenHeight)
				BannerHeight = ScreenHeight - ButtonsHeight;

			//int	ButtonCenter = ((ScreenHeight - LogoHeight - BannerHeight) / 2) + BannerHeight;
			int	ButtonRoom = ScreenHeight - LogoHeight - BannerHeight;
			
			Overlay[0].SetCoords((ScreenWidth - Overlay[0].GetWidth()) / 2, 0);
			Overlay[2].SetCoords((ScreenWidth - Overlay[2].GetWidth()) / 2, BannerHeight + (ButtonRoom >> 2) - (Overlay[2].GetHeight() >> 1));
			Overlay[3].SetCoords((ScreenWidth - Overlay[3].GetWidth()) / 2, BannerHeight + (ButtonRoom >> 1) + (ButtonRoom >> 2) - (Overlay[3].GetHeight() >> 1));

			LogoSurface.SetCoords(ScreenWidth - LogoSurface.GetWidth(), ScreenHeight - LogoSurface.GetHeight());

			return true;
		}

		else if ((Scene == SCENE_GAME_PLAY) || (Scene == SCENE_LEARN_2))
		{
			Background.Clear();

			Corners[UPPER_LEFT].SetImage("gfx/overlays/corner_up_left.png");
			Corners[BOTTOM_LEFT].SetImage("gfx/overlays/corner_bottom_left.png");
			Corners[UPPER_RIGHT].SetImage("gfx/overlays/corner_up_right.png");
			Corners[BOTTOM_RIGHT].SetImage("gfx/overlays/corner_bottom_right.png");

			//Overlay[0].SetImage("gfx/overlays/game_play_1.png");
			Overlay[0].SetImage("gfx/scenes/green_bg.png");
			Overlay[1].SetImage("gfx/overlays/line_h.png");

			if (Dimensions::LandscapeMode)
				Overlay[2].SetImage("gfx/overlays/line_v.png");


			DiscardSurface.SetImage(Card::GetFileFromValue(DiscardTop));
			TargetSurface.SetImage("gfx/drop_target.png");

			if ((SourceDeck != 0) && (SourceDeck->Empty()))
				DrawCardSurface.SetImage(Card::GetFileFromValue(CARD_NULL_NULL));
			else
				DrawCardSurface.SetImage("gfx/card_bg.png");

			//if (Dimensions::LandscapeMode)
			//{
			//	DiscardSurface.SetX(Dimensions::GamePlayHandLeftX);

			//	if (Dimensions::GamePlayMultiRowTray)
			//	{
			//		DrawCardSurface.SetCoords(Dimensions::GamePlayHandLeftX + Dimensions::GamePlayCardWidth + Dimensions::GamePlayCardSpacingX, Dimensions::FirstRowY - ((Dimensions::GamePlayCardHeight + Dimensions::GamePlayCardSpacingY) << 1));
			//		DiscardSurface.SetY(DrawCardSurface.GetY());
			//	}
			//	else
			//	{
			//		DrawCardSurface.SetCoords(Dimensions::GamePlayHandLeftX, Dimensions::FirstRowY - ((Dimensions::GamePlayCardHeight + Dimensions::GamePlayCardSpacingY) * 3));
			//		DiscardSurface.SetY(DrawCardSurface.GetY() + Dimensions::GamePlayCardHeight + Dimensions::GamePlayCardSpacingY);
			//	}
			//}
			//else
			//{
			//	DrawCardSurface.SetCoords(SCREEN_EDGE_PADDING, ScreenHeight - TRAY_TOP_BOTTOM_PADDING - Dimensions::GamePlayCardHeight);

			//	if (Dimensions::GamePlayMultiRowTray)
			//		DiscardSurface.SetCoords(DrawCardSurface.GetX(), DrawCardSurface.GetY() - Dimensions::GamePlayCardHeight - Dimensions::GamePlayCardSpacingY);
			//	else
			//		DiscardSurface.SetCoords(DrawCardSurface.GetX() + Dimensions::GamePlayCardWidth + Dimensions::GamePlayCardSpacingX, DrawCardSurface.GetY());
			//}

			DiscardSurface.SetCoords(Dimensions::TrayDiscardX, Dimensions::TrayDiscardY);
			DrawCardSurface.SetCoords(Dimensions::TrayDrawX, Dimensions::TrayDrawY);

			if (DrawFont)
			{
				DrawTextSurface.SetInteger(DeckCount, DrawFont, true, &White);

				DrawTextSurface.SetCoords(DrawCardSurface.GetX() + (Dimensions::GamePlayCardWidth >> 1) - (DrawTextSurface.GetWidth() >> 1), DrawCardSurface.GetY() + (Dimensions::GamePlayCardHeight * .75) - (DrawTextSurface.GetHeight() >> 1));

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
			{
				MenuSurface.SetImage("gfx/menu.png");
				MenuSurface.SetCoords(2, 5);
			}
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

			if (Outcome == OUTCOME_WON)
			{
				Background.SetImage("gfx/scenes/black_bg.png");
				Overlay[0].SetImage("gfx/overlays/game_over_won.jpg");
			}
			else
			{
				Background.SetImage("gfx/scenes/green_bg.png");
				Overlay[0].SetImage("gfx/overlays/game_over.png");
			}

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
	}

	return false;
}

void	Game::OnRender			(SDL_Surface *Target, bool Force, bool Flip)
{
	static	Uint32	LastRender	= 0;
			Uint32	TickCount	= SDL_GetTicks();

	bool	RefreshedSomething =	false, // We only flip the display if something changed
			SceneChanged =			false; // Control variable. Do we need to call OnInit()?

	#ifdef DEBUG
	static	Uint32	LastReset = 0;
	static	Uint32	FrameCount = 0;

	++FrameCount;

	if ((LastReset + 333) < TickCount)
	{
		LastReset = TickCount;
		DebugSurface.SetInteger(FrameCount * 3, GameOverBig, true, &Black, &White);
		FrameCount = 0;
		RefreshedSomething = true;
	}
	#endif

	#ifdef	ANDROID_DEVICE
		if (EventCount > 0)
		{
			--EventCount;
			Dirty = true;
		}
	#endif

	if (LastRender < (TickCount - 1000))
	{
		Dirty = true;
		LastRender = TickCount;
	}

	//if ((Modal == MODAL_NONE) || Force)	//Don't re-render during modal, unless forced
	//{
	// If the scene, discard pile, or deck count have changed, we need to do a refresh
	SceneChanged |= CheckForChange(OldDeckCount, DeckCount);
	SceneChanged |= CheckForChange(OldDiscardTop, DiscardTop);
	SceneChanged |= CheckForChange(LastScene, Scene);
	SceneChanged |= CheckForChange(LastModal, Modal);

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

	if ((Modal < MODAL_NONE) && Dirty)
		SceneChanged = true;

	for (int i = 0; i < PLAYER_COUNT; ++i)
		Force |= Players[i].IsDirty();

	if (SceneChanged)
		Force = true;

	if (Force)
	{
		if (SceneChanged || (Message[0] != '\0'))
			OnInit(); //Refresh our surfaces

		//Force = true;
		RefreshedSomething = true;
	
		// Render the appropriate surfaces
		Background.Fill(0, 0, Target);

		OnRenderScene(Target);

		if (Modal < MODAL_NONE)
			OnRenderModal(Target);
	}

	#ifdef DEBUG
	DebugSurface.Render(0, 0, Target);
	#endif

	if (RefreshedSomething && Flip)
		SDL_Flip(Target);
}

void	Game::OnRenderModal		(SDL_Surface *Target)
{
	if (Modal < MODAL_NONE)
	{
		ShadowSurface.Fill(0, 0, Target);	//Render shadow

		if (Modal == MODAL_EXTENSION)
		{

			ModalSurface.Render(Target);

			Overlay[3].Render(Target);
			Overlay[4].Render(Target);
			Overlay[5].Render(Target);
		}
		else if (Modal <= MODAL_OPTIONS)
		{
			int	TextRowCount = 0;

			ModalSurface.Render(Target);
			Overlay[3].Render(Target);

			if (Modal == MODAL_GAME_MENU)
			{
				TextRowCount = MENU_SURFACE_COUNT;
				//Overlay[3].Render(236, 73, Target);

				//for (int i = 0; i < MENU_SURFACE_COUNT; ++i)	//Render options and other menu items
				//{
				//	MenuSurfaces[i][0].Render(50, 125 + (i * 40), Target);
				//}
			}
			else
			{
				//Overlay[3].Render(44, 73, Target);
					
				if (Modal == MODAL_STATS)
				{
					TextRowCount = STAT_CAPTIONS_SIZE;

					//for (int i = 0; i < STAT_CAPTIONS_SIZE; ++i)
					//{
					//	MenuSurfaces[i][0].Render(50, 125 + (i * 40), Target);
					//	MenuSurfaces[i][1].Render(220, 125 + (i * 40), Target);
					//}
				}
				else
				{
					//Overlay[1].Render((Dimensions::ScreenWidth - Overlay[1].GetWidth()) / 2, 10, Target, SCALE_Y);
					//Overlay[2].Render((Dimensions::ScreenWidth - Overlay[2].GetWidth()) / 2, 20 + Overlay[1].GetHeight(), Target, SCALE_Y);

					TextRowCount = OPTION_COUNT;
					//for (int i = 0; i < OPTION_COUNT; ++i)
					//{
					//	MenuSurfaces[i][0].Render(50, 125 + (i * 40), Target);
					//	MenuSurfaces[i][1].Render(240, 125 + (i * 40), Target);
					//}
				}
			}

			for (int i = 0; i < TextRowCount; ++i)
			{
				MenuSurfaces[i][0].Render(Target);
				MenuSurfaces[i][1].Render(Target);
			}
		}
		else if (Modal <= MODAL_CLEAR_STATS)
		{
			ModalSurface.Render(Target);
			Overlay[3].Render(Target);
			Overlay[4].Render(Target);
		}
	}
}

void	Game::OnRenderScene		(SDL_Surface *Target)
{
	if (Scene == SCENE_MAIN)
	{
		Overlay[0].Render(Target);
		Overlay[2].Render(Target);
		Overlay[3].Render(Target);

		LogoSurface.Render(Target);
	}
	else if ((Scene == SCENE_GAME_PLAY) || IN_DEMO)
	{
		//Overlay[0].Render(0, Dimensions::EffectiveTableauHeight - 1, Target, SCALE_NONE);
		//Overlay[0].Render(0, (Dimensions::EffectiveTableauHeight * 2) - 1, Target, SCALE_NONE);

		int FillX,
			FillY;

		if (Dimensions::LandscapeMode)
		{
			FillX = Dimensions::GamePlayTableauWidth + 1;
			FillY = 0;
		}
		else
		{
			FillX = 0;
			FillY = (Dimensions::TableauHeight << 1) + 1;
		}

		for (int i = (PLAYER_COUNT - 1); i >= 0; --i)
			Players[i].OnRender(Target, i, true);

			
		Overlay[1].RepeatX(0, Dimensions::TableauHeight, Target);

		if (Dimensions::LandscapeMode)
			Overlay[2].RepeatY(0, Dimensions::GamePlayTableauWidth, Target);
		else
			Overlay[1].RepeatX(0, Dimensions::TableauHeight << 1, Target);

		Overlay[0].Fill(FillX, FillY, Target);

		Players[0].OnRenderHand(Target, true);

		DiscardSurface.Render(Target);
		DrawCardSurface.Render(Target);
		DrawTextSurface.Render(Target);
			
		Corners[UPPER_LEFT].Render(0, 0, Target, SCALE_NONE);
		Corners[UPPER_LEFT].Render(0, Dimensions::EffectiveTableauHeight, Target, SCALE_NONE);
			
		Corners[BOTTOM_LEFT].Render(0, Dimensions::EffectiveTableauHeight - Corners[BOTTOM_LEFT].GetHeight(), Target, SCALE_NONE);
		Corners[BOTTOM_LEFT].Render(0, (Dimensions::EffectiveTableauHeight * 2) - Corners[BOTTOM_LEFT].GetHeight(), Target, SCALE_NONE);

		Corners[UPPER_RIGHT].Render(Dimensions::GamePlayTableauWidth - Corners[UPPER_RIGHT].GetWidth(), 0, Target, SCALE_NONE);
		Corners[UPPER_RIGHT].Render(Dimensions::GamePlayTableauWidth - Corners[UPPER_RIGHT].GetWidth(), Dimensions::EffectiveTableauHeight, Target, SCALE_NONE);

		Corners[BOTTOM_RIGHT].Render(Dimensions::GamePlayTableauWidth - Corners[BOTTOM_RIGHT].GetWidth(), Dimensions::EffectiveTableauHeight - Corners[BOTTOM_RIGHT].GetHeight(), Target, SCALE_NONE);
		Corners[BOTTOM_RIGHT].Render(Dimensions::GamePlayTableauWidth - Corners[BOTTOM_RIGHT].GetWidth(), (Dimensions::EffectiveTableauHeight * 2) - Corners[BOTTOM_RIGHT].GetHeight(), Target, SCALE_NONE);

		if (Dimensions::LandscapeMode)
		{
			Corners[UPPER_LEFT].Render(Dimensions::GamePlayTableauWidth, 0, Target, SCALE_NONE);
			Corners[BOTTOM_LEFT].Render(Dimensions::GamePlayTableauWidth, Dimensions::ScreenHeight - Corners[BOTTOM_LEFT].GetHeight(), Target, SCALE_NONE);
			Corners[UPPER_RIGHT].Render(Dimensions::ScreenWidth - Corners[UPPER_RIGHT].GetWidth(), 0, Target, SCALE_NONE);
			Corners[BOTTOM_RIGHT].Render(Dimensions::ScreenWidth - Corners[BOTTOM_RIGHT].GetWidth(), Dimensions::ScreenHeight - Corners[BOTTOM_RIGHT].GetHeight(), Target, SCALE_NONE);
		}
		else
		{
			Corners[UPPER_LEFT].Render(0, Dimensions::EffectiveTableauHeight * 2, Target, SCALE_NONE);
			Corners[BOTTOM_LEFT].Render(0, Dimensions::ScreenHeight - Corners[BOTTOM_LEFT].GetHeight(), Target, SCALE_NONE);
			Corners[UPPER_RIGHT].Render(Dimensions::GamePlayTableauWidth - Corners[UPPER_RIGHT].GetWidth(), Dimensions::EffectiveTableauHeight * 2, Target, SCALE_NONE);
			Corners[BOTTOM_RIGHT].Render(Dimensions::GamePlayTableauWidth - Corners[BOTTOM_RIGHT].GetWidth(), Dimensions::ScreenHeight - Corners[BOTTOM_RIGHT].GetHeight(), Target, SCALE_NONE);
		}
	}
	else if (Scene == SCENE_GAME_OVER)
	{
		int X = 0, Y = 0;

		Overlay[0].Render(0, (Dimensions::ScreenHeight - Overlay[0].GetHeight()) / 2, Target, SCALE_NONE);
		Overlay[1].Render((Dimensions::ScreenWidth - Overlay[1].GetWidth()) / 2, Dimensions::ScreenHeight - Overlay[1].GetHeight() - 12, Target, SCALE_NONE);

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

	if ((Scene == SCENE_GAME_PLAY) || IN_DEMO)
	{
		//Render card caption
		if (GameOptions.GetOpt(OPTION_CARD_CAPTIONS))
			CaptionSurface.Render((320 - CaptionSurface.GetWidth()) / 2, ((Dimensions::TableauHeight * 2) - CaptionSurface.GetHeight()) - 10, Target);

		if (Scene == SCENE_GAME_PLAY)
			MenuSurface.Render(Target);
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

void	Game::ShowLoading		(void)
{
	Overlay[1].Render((Dimensions::ScreenWidth - Overlay[1].GetWidth()) / 2, (Dimensions::ScreenHeight - Overlay[1].GetHeight()) / 2, Window, SCALE_NONE);
	SDL_Flip(Window);
}

void	Game::UpdateMetrics		(void)
{
	if (Window != 0)
	{
		Dimensions::SetDimensions(Window->w, Window->h, DiscardSurface.GetWidth(), DiscardSurface.GetHeight(), GameOptions.GetOpt(OPTION_VERTICAL_TRAY));
		Dirty = true;
	}
}

}
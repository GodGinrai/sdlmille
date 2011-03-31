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

#ifndef _SDLMILLE_GAME_H
#define	_SDLMILLE_GAME_H

#define	IN_DEMO ((Scene >= SCENE_LEARN_2) && (Scene <= SCENE_LEARN_7))
#define IN_TUTORIAL ((Scene >= SCENE_LEARN_1) && (Scene <= SCENE_LEARN_7))

#include "Player.h"
#include "Options.h"
#include "Stats.h"
#include <algorithm>

namespace _SDLMille
{

enum	{ANIMATION_PLAY = 0, ANIMATION_DISCARD, ANIMATION_RETURN, ANIMATION_COUP_FOURRE_BOUNCE, ANIMATION_SAFETY_SPAWN, ANIMATION_INVALID};

enum	{DIFFICULTY_EASY = 0, DIFFICULTY_NORMAL, DIFFICULTY_HARD, DIFFICULTY_LEVEL_COUNT};

const Uint8 MENU_ITEM_COUNT = 5,
			MENU_SURFACE_COUNT = STAT_CAPTIONS_SIZE,
			PLAYER_COUNT = 2,
			SCORE_CATEGORY_COUNT = 12,
			SCORE_COLUMN_COUNT = 3,
			MESSAGE_SIZE = 42;

const	char	CARD_CAPTIONS[CARD_SAFETY_RIGHT_OF_WAY + 1][20] = {
					"Crash HAZARD", "Fuel HAZARD", "Tire HAZARD", "Speed HAZARD", "Stop HAZARD",
					"Crash REMEDY", "Fuel REMEDY", "Tire REMEDY", "Speed REMEDY", "Roll REMEDY",
					"Crash SAFETY", "Fuel SAFETY", "Tire SAFETY", "Right-of-way SAFETY"	};
const	int		HAND_COORDS[] =	{	146,
									146,
									3	};
const	char	DIFFICULTY_TEXT[DIFFICULTY_LEVEL_COUNT][7] = {"Easy", "Normal", "Hard"};

const	char	MENU_ITEM_NAMES[MENU_ITEM_COUNT][15] = {"Options", "Stats", "New game", "Reset Stats", "Main menu"};

const	char	SCORE_CAT_NAMES[SCORE_CATEGORY_COUNT][15] = {
					"Mileage", "Safeties", "All 4", "Coup Fourres", "Completed Trip", "Delayed Action", "Safe Trip",
					"Extension", "Shutout", "Subtotal", "Previous", "Total"	};

const	char	TUTORIAL_TEXT[][MESSAGE_SIZE] = {
					"This is the computer's area.",
					"And this is your area.",
					"Click a card to select it.",
					"Click it again to play.",
					"Or click here to discard.",
					"Enjoy the game!"	};

const	char	VERSION_TEXT[] = "0.5.4-7 (beta4 test7)";

const	int		SAVE_FORMAT_VER = 8;

class Game
{
public:
				Game			(void);
				~Game			(void);
	bool		OnExecute		(void);
private:
	/* Methods */
	void		Animate			(Uint8 Index, Uint8 AnimationType, Uint8 Value = CARD_NULL_NULL);
	//bool		AnimationRunning	(void)				const;
	void		ChangePlayer	(void);
	bool		CheckForChange	(Uint8 &Old, Uint8 &New);
	//void		CheckTableau	(SDL_Surface *Target);
	void		ClearMessage	(void);
	bool		CouldHoldCard	(Uint8 PlayerIndex, Uint8 Value)	const;
	bool		ComputerDecideExtension	(void)			const;
	void		ComputerMove	(void);
	void		ComputerSmartMove	(void);
	void		DelayUntil		(Uint32 Ticks);
	bool		Discard			(void);
	bool		EndOfGame		(void)					const;
	Uint8		FindPopped		(void)					const;
	void		GetScores		(void);
	void		IgnoreEvents	(void);
	bool		InDiscardPile	(int X, int Y)			const;
	Uint8		InHand			(Uint8 Value)			const;
	bool		IsOneCardAway	(Uint8 PlayerIndex)		const;
	bool		IsValidPlay		(Uint8 Index)			const;
	Uint8		KnownCards		(Uint8 Value)			const;
	bool		MayHaveRoW		(Uint8 PlayerIndex)		const;
	void		OnClick			(int X, int Y);
	void		OnEvent			(SDL_Event * Event);
	bool		OnInit			(void);
	void		OnLoop			(void);
	void		OnMouseUp		(int X, int Y);
	void		OnPlay			(Uint8 Index, bool PlayerChange = true);
	void		OnRender		(SDL_Surface *Target, bool Force = false, bool Flip = true);
	void		Pop				(Uint8 Index);
	void		Reset			(bool SaveStats = false);
	void		ResetPortal		(void);
	bool		Restore			(void);
	bool		Save			(void);
	void		SetDifficulty	(void);
	void		ShowLoading		(void);
	void		ShowMessage		(const char * Msg, bool SetDirty = true);
	bool		ShowModal		(Uint8 ModalName);
	Uint8		UnknownCards	(Uint8 Value);

	/* Properties */
	SDL_Surface	*Window;
	Surface		ArrowSurfaces[2],
				Background,
				CaptionSurface,
				DebugSurface,
				DiscardSurface,
				DrawCardSurface, DrawTextSurface,
				FloatSurface,
				HandSurface,
				LogoSurface,
				MenuSurface,
				MessageSurface,
				MenuSurfaces[MENU_SURFACE_COUNT][2],
				ModalSurface,
				OrbSurface,
				Overlay[5],
				ResultTextSurface,
				ScoreSurfaces[SCORE_CATEGORY_COUNT + 1][SCORE_COLUMN_COUNT],
				ShadowSurface,
				TargetSurface,
				VersionSurface;
	Player		Players[PLAYER_COUNT];
	Options		GameOptions;
	Deck		*SourceDeck;
	Stats		PlayerStats;
	SDL_Rect	Portal;
	SDL_Color	Black,
				Green,
				Red,
				White,
				Yellow;
	int			DownX, DownY,
				DragX, DragY,
				Scores[PLAYER_COUNT],
				RunningScores[PLAYER_COUNT],
				ScoreBreakdown[PLAYER_COUNT][SCORE_CATEGORY_COUNT];
	Uint32		EventCount,
				FrozenAt,
				MessagedAt;
	bool		Animating,
				Dirty,
				Dragging,
				Extended, ExtensionDeclined,
				Frozen,
				MouseDown,
				Running;
	Uint8		Current,				// Index of current player
				DownIndex,
				Modal, LastModal,
				Scene, LastScene,
				DeckCount, OldDeckCount,
				DiscardTop, OldDiscardTop,
				ExposedCards[CARD_NULL_NULL],
				Outcome,
				Difficulty;
	char		Message[MESSAGE_SIZE];
	TTF_Font	*DrawFont, *GameOverBig, *GameOverSmall;


	enum {	SCENE_MAIN = 0, SCENE_GAME_PLAY, SCENE_GAME_MENU, SCENE_GAME_OVER,
			SCENE_LEARN_1, SCENE_LEARN_2, SCENE_LEARN_3, SCENE_LEARN_4, SCENE_LEARN_5, SCENE_LEARN_6, SCENE_LEARN_7,
			SCENE_LEGAL, SCENE_INVALID };
	enum {	MODAL_GAME_MENU = 0, MODAL_STATS, MODAL_OPTIONS, MODAL_EXTENSION, MODAL_NEW_GAME, MODAL_CLEAR_STATS, MODAL_NONE };
};

inline	Uint8		Game::UnknownCards	(Uint8 Value)
{
	return (EXISTING_CARDS[Value] - KnownCards(Value));
}

}

#endif

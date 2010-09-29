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

namespace _SDLMille
{

const Uint8 MENU_ITEM_COUNT = 2,
			PLAYER_COUNT = 2,
			SCORE_CATEGORY_COUNT = 12,
			SCORE_COLUMN_COUNT = 3,
			MESSAGE_SIZE = 31;

const	char	CARD_CAPTIONS[CARD_SAFETY_RIGHT_OF_WAY + 1][20] = {
					"Crash HAZARD", "Fuel HAZARD", "Tire HAZARD", "Speed HAZARD", "Stop HAZARD",
					"Crash REMEDY", "Fuel REMEDY", "Tire REMEDY", "Speed REMEDY", "Roll REMEDY",
					"Crash SAFETY", "Fuel SAFETY", "Tire SAFETY", "Right-of-way SAFETY"	};
const	int		HAND_COORDS[][2] =	{
					{146, 388},
					{146, 388},
					{3, 388}	};
const	char	MENU_ITEM_NAMES[MENU_ITEM_COUNT][10] = {"New game", "Main menu"};
const	int		ORB_COORDS[][2] =	{
					{136,	45},
					{136,	245},
					{146,	364},
					{146,	364}	};
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
const	char	VERSION_TEXT[] = "0.5.0 (beta1)";

class Game
{
public:
				Game			(void);
				~Game			(void);
	bool		OnExecute		(void);
private:
	/* Methods */
	void		ChangePlayer	(void);
	bool		CheckForChange	(Uint8 &Old, Uint8 &New);
	void		ClearMessage	(void);
	void		ComputerMove	(void);
	bool		Discard			(void);
	bool		EndOfGame		(void)					const;
	Uint8		FindPopped		(void)					const;
	void		GetScores		(void);
	bool		IsValidPlay		(Uint8 Index)			const;
	void		OnClick			(int X, int Y);
	void		OnEvent			(SDL_Event * Event);
	bool		OnInit			(void);
	void		OnLoop			(void);
	void		OnPlay			(Uint8 Index);
	void		OnRender		(bool Force = false, bool Flip = true);
	void		Pop				(Uint8 Index);
	void		Reset			(void);
	void		ShowMessage		(const char * Msg, bool SetDirty = true);
	bool		ShowModal		(Uint8 ModalName);

	/* Properties */
	SDL_Surface	*Window;
	Surface		ArrowSurfaces[2],
				Background,
				CaptionSurface,
				DiscardSurface,
				DrawCardSurface, DrawTextSurface,
				HandSurface,
				LogoSurface,
				MenuSurface,
				MessageSurface,
				MenuSurfaces[OPTION_COUNT + MENU_ITEM_COUNT][2],
				ModalSurface,
				OrbSurface,
				ResultTextSurface,
				ScoreSurfaces[SCORE_CATEGORY_COUNT + 1][SCORE_COLUMN_COUNT],
				VersionSurface;
	Player		Players[PLAYER_COUNT];
	Options		GameOptions;
	Deck		*SourceDeck;
	int			Scores[PLAYER_COUNT],
				RunningScores[PLAYER_COUNT],
				ScoreBreakdown[PLAYER_COUNT][SCORE_CATEGORY_COUNT];
	Uint32		FrozenAt,
				MessagedAt;
	bool		Dirty,
				Extended, ExtensionDeclined,
				Frozen,
				Running;
	Uint8		Current,				// Index of current player
				Modal,
				Scene, LastScene,
				DeckCount, OldDeckCount,
				DiscardTop, OldDiscardTop;
	char		Message[MESSAGE_SIZE];
	TTF_Font	*DrawFont, *GameOverFont;


	enum {	SCENE_MAIN = 0, SCENE_GAME_PLAY, SCENE_GAME_MENU, SCENE_GAME_OVER,
			SCENE_LEARN_1, SCENE_LEARN_2, SCENE_LEARN_3, SCENE_LEARN_4, SCENE_LEARN_5, SCENE_LEARN_6, SCENE_LEARN_7,
			SCENE_LEGAL, SCENE_INVALID };
	enum {	MODAL_EXTENSION = 0, MODAL_GAME_MENU, MODAL_NEW_GAME, MODAL_MAIN_MENU, MODAL_NONE };
};

}

#endif

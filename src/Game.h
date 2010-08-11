#ifndef _SDLMILLE_GAME_H
#define	_SDLMILLE_GAME_H

#include "Player.h"
#include "Options.h"

namespace _SDLMille
{

const Uint8 PLAYER_COUNT = 2,
			SCORE_CATEGORY_COUNT = 12,
			SCORE_COLUMN_COUNT = 3,
			MESSAGE_SIZE = 31;

const	char	CARD_CAPTIONS[CARD_SAFETY_RIGHT_OF_WAY + 1][21] = {
					"Crash HAZARD", "Fuel HAZARD", "Tire HAZARD", "Speed HAZARD", "Stop HAZARD",
					"Crash REMEDY", "Fuel REMEDY", "Tire REMEDY", "Speed REMEDY", "Roll REMEDY",
					"Crash SAFETY", "Fuel SAFETY", "Tire SAFETY", "Right-of-way SAFETY"
					};
const	char	SCORE_CAT_NAMES[SCORE_CATEGORY_COUNT][21] = {"Mileage", "Safeties", "All 4", "Coup Fourres", "Completed Trip", "Delayed Action", "Safe Trip", "Extension", "Shutout", "Subtotal", "Previous", "Total"};
const	char	VERSION_TEXT[] = "0.4.99-5 (Pre-beta1 test5)";

class Game
{
public:
				Game			(void);
				~Game			(void);
	bool		IsValidPlay		(Uint8 Index)			const;
	void		OnClick			(int X, int Y);
	void		OnEvent			(SDL_Event * Event);
	bool		OnExecute		(void);
	bool		OnInit			(void);
	void		OnLoop			(void);
	void		OnPlay			(Uint8 Index);
	void		OnRender		(bool Force = false, bool Flip = true);
	void		Reset			(void);
	void		ShowMessage		(const char * Msg);
	bool		ShowModal		(Uint8 ModalName);

private:
	/* Methods */
	void		ChangePlayer	(void);
	bool		Discard			(void);
	bool		EndOfGame		(void)					const;
	Uint8		FindPopped		(void)					const;
	void		GetScores		(void);
	void		Pop				(Uint8 Index);

	/* Properties */
	SDL_Surface	*Window;
	Surface		Background,
				CaptionSurface,
				DiscardSurface,
				DrawCardSurface, DrawTextSurface,
				LogoSurface,
				MenuSurface,
				MessageSurface,
				OptionSurfaces[OPTION_COUNT][2],
				ResultTextSurface,
				ScoreSurfaces[SCORE_CATEGORY_COUNT + 1][SCORE_COLUMN_COUNT],
				VersionSurface;
	Player		Players[PLAYER_COUNT];
	Options		GameOptions;
	Deck *		SourceDeck;
	int			DeckCount, OldDeckCount,
				Scores[PLAYER_COUNT],
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
				DiscardTop, OldDiscardTop;
	char		Message[MESSAGE_SIZE];
	TTF_Font	*DrawFont, *GameOverFont;


	enum { SCENE_MAIN = 0, SCENE_GAME_PLAY, SCENE_GAME_MENU, SCENE_GAME_OVER, SCENE_LEARN, SCENE_LEGAL, SCENE_INVALID };
	enum { MODAL_EXTENSION = 0, MODAL_GAME_MENU, MODAL_NONE };
};

}

#endif

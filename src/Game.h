#define WEBOS_DEVICE //Unfortunately, this flag is necessary, because of the way webOS handles SDL_SetVideoMode().

#ifndef _SDLMILLE_GAME_H
#define	_SDLMILLE_GAME_H

#include "Player.h"

const Uint8 PLAYER_COUNT = 2,
			SCORE_CATEGORY_COUNT = 9,
			SCORE_COLUMN_COUNT = 3;

const char	SCORE_CAT_NAMES[SCORE_CATEGORY_COUNT][20] = {"Mileage", "Safeties", "All 4", "Coup Fourres", "Completed Trip", "Delayed Action", "Safe Trip", "Extension", "Shutout"};

namespace _SDLMille
{

class Game
{
public:
				Game			(void);
				~Game			(void);
	bool		IsValidPlay		(Uint8 Index);
	void		OnClick			(int X, int Y);
	void		OnEvent			(SDL_Event * Event);
	bool		OnExecute		(void);
	bool		OnInit			(void);
	void		OnLoop			(void);
	void		OnPlay			(Uint8 Value);
	void		OnRender		(void);
	void		Reset			(void);

private:
	/* Methods */
	void		ChangePlayer	(void);
	bool		Discard			(void);
	bool		EndOfGame		(void);
	Uint8		FindPopped		(void);
	void		GetScores		(void);
	bool		Pop				(Uint8 Index);

	/* Properties */
	SDL_Surface	*Window, *Background,
				*DiscardSurface,
				*DrawCardSurface, *DrawTextSurface,
				*OutcomeSurface, *ScoresSurface,
				*ScoreSurfaces[SCORE_CATEGORY_COUNT + 2][SCORE_COLUMN_COUNT];
	Player		Players[PLAYER_COUNT];
	Deck *		SourceDeck;
	int			DeckCount, OldDeckCount,
				Scores[PLAYER_COUNT],
				ScoreBreakdown[PLAYER_COUNT][SCORE_CATEGORY_COUNT];
	bool		Dirty,
				Extended,
				Running;
	Uint8		Current,				// Index of current player
				Scene, LastScene,
				DiscardTop, OldDiscardTop;
	TTF_Font	*DrawFont, *GameOverFont;


	enum { SCENE_MAIN = 0, SCENE_GAME_PLAY, SCENE_GAME_MENU, SCENE_GAME_OVER, SCENE_LEARN, SCENE_INVALID };
};

}

#endif

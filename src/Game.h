#ifndef _SDLMILLE_GAME_H
#define	_SDLMILLE_GAME_H

#include "Player.h"

const Uint8 PLAYER_COUNT = 2,
			SCORE_CATEGORY_COUNT = 12,
			SCORE_COLUMN_COUNT = 3,
			MESSAGE_SIZE = 31;

const char	SCORE_CAT_NAMES[SCORE_CATEGORY_COUNT][20] = {"Mileage", "Safeties", "All 4", "Coup Fourres", "Completed Trip", "Delayed Action", "Safe Trip", "Extension", "Shutout", "Subtotal", "Previous", "Total"};

namespace _SDLMille
{

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
	void		OnRender		(bool Force = false);
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
				DiscardSurface,
				DrawCardSurface, DrawTextSurface,
				MessageSurface,
				ResultTextSurface,
				ScoreSurfaces[SCORE_CATEGORY_COUNT + 1][SCORE_COLUMN_COUNT];
	Player		Players[PLAYER_COUNT];
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


	enum { SCENE_MAIN = 0, SCENE_GAME_PLAY, SCENE_GAME_MENU, SCENE_GAME_OVER, SCENE_LEARN, SCENE_INVALID };
	enum { MODAL_EXTENSION = 0, MODAL_GAME_MENU, MODAL_NONE };
};

}

#endif

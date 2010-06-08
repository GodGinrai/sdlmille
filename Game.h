#ifndef _SDLMILLE_GAME_H
#define	_SDLMILLE_GAME_H

#include "Player.h"

const Uint8 PLAYER_COUNT = 2;

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

private:
	/* Methods */
	void		ChangePlayer	(void);
	bool		Discard			(void);
	bool		EndOfGame		(void);
	Uint8		FindPopped		(void);
	int			GetScore		(Uint8 PlayerIndex);
	bool		Pop				(Uint8 Index);

	/* Properties */
	SDL_Surface	*Window, *Background,
				*DiscardSurface,
				*DrawCardSurface, *DrawTextSurface,
				*OutcomeSurface, *ScoresSurface;
	Player		Players[PLAYER_COUNT];
	Deck *		SourceDeck;
	int			DeckCount, OldDeckCount,
				Scores[PLAYER_COUNT];
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
#ifndef _SDLMILLE_HAND_H
#define _SDLMILLE_HAND_H

#include "Card.h"

namespace	_SDLMille
{

const Uint8 HAND_SIZE = 7;

class Hand
{
public:
				Hand		(void);
				~Hand		(void);
	bool		Discard		(Uint8 Index);
	bool		Draw		(Deck * Source, Uint8 Index);
	Uint8		GetType		(Uint8 Index);
	Uint8		GetValue	(Uint8 Index);
	bool		IsPopped	(Uint8 Index);
	void		OnInit		(void);
	bool		OnRender	(SDL_Surface * Surface, bool Force = false);
	bool		Pop			(Uint8 Index);
	void		UnPop		(Uint8 Index);
private:
	Card		ThisHand[HAND_SIZE];
	bool		Popped[HAND_SIZE],
				Dirty;
	SDL_Surface	*CardSurfaces[HAND_SIZE],
				*Overlay;
};

}

#endif
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
	Uint8		GetType		(Uint8 Index)								const;
	Uint8		GetValue	(Uint8 Index)								const;
	bool		IsDirty		(void)										const;
	bool		IsPopped	(Uint8 Index)								const;
	void		OnInit		(void);
	bool		OnRender	(SDL_Surface * Surface, bool Force = false);
	void		Pop			(Uint8 Index);
	void		Reset		(void);
	void		UnPop		(Uint8 Index);
private:
	Card		ThisHand[HAND_SIZE];
	bool		Popped[HAND_SIZE],
				Dirty;
	Surface		CardSurfaces[HAND_SIZE],
				CancelSurface,
				Overlay;
};

inline	bool	Hand::IsDirty	(void)	const
{
	return Dirty;
}

}

#endif

#ifndef _SDLMILLE_PLAYER_H
#define	_SDLMILLE_PLAYER_H

#include "Hand.h"
#include "Tableau.h"

namespace _SDLMille
{

const int	PLAYER_GIVEN_NO_ARGUMENT =	3000,
			PLAYER_NOT_ENOUGH_CARDS =	3001;

class Player
{
public:
				Player			(void);
	bool		Discard			(Uint8 Index);
	void		Draw			(void);
	bool		Draw			(Uint8 Index);
	Uint8		Get200Count		(void);
	int			GetMileage		(void);
	Uint8		GetTopCard		(bool SpeedPile = false);
	Uint8		GetType			(Uint8 Index);
	Uint8		GetValue		(Uint8 Index);
	bool		HasCoupFourre	(Uint8 Value);
	bool		HasSafety		(Uint8 Value);
	bool		IsDirty			(void);
	bool		IsLimited		(void);
	bool		IsOutOfCards	(void);
	bool		IsPopped		(Uint8 Index);
	bool		IsRolling		(void);
	Uint8		OnPlay			(Uint8 Index);
	bool		OnRender		(SDL_Surface * Surface, Uint8 PlayerIndex, bool Force = false);
	bool		Pop				(Uint8 Index);
	bool		ReceiveHazard	(Uint8 Value);
	void		Reset			(void);
	void		SetSource		(Deck * ArgSource);
	void		UnPop			(Uint8 Index);
private:
	Tableau		MyTableau;
	Hand		PlayerHand;
	Uint8		QualifiedCoupFourre;
	Deck *		SourceDeck;
};

inline	Uint8	Player::GetTopCard	(bool SpeedPile)
{
	return MyTableau.GetTopCard(SpeedPile);
}

inline	int		Player::GetMileage	(void)
{
	return MyTableau.GetMileage();
}

inline	bool	Player::IsDirty		(void)
{
	return PlayerHand.IsDirty();
}

}

#endif

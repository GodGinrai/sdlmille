#ifndef	_SDLMILLE_DIMENSIONS_H
#define	_SDLMILLE_DIMENSIONS_H

#include <stdlib.h>

class	Dimensions
{
public:
	static	void	SetDimensions	(int Width, int Height);

	static	double	ScaleFactor;
	static	int		EffectiveTableauHeight,
					FirstRowY,
					PadLeft,
					SecondRowY,
					ScreenWidth,
					ScreenHeight,
					TableauHeight;
	static	bool	MultiRowSafeties;
};

#endif

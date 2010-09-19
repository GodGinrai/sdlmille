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

//#define	WEBOS_DEVICE
//#define	ANDROID_DEVICE

#ifndef _SDLMILLE_SURFACE_H
#define _SDLMILLE_SURFACE_H

#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <cmath>

namespace _SDLMille
{

class Surface
{
public:
							Surface			(void);
							~Surface		(void);
			void			Clear			(void);
			int				GetHeight		(void)																			const;
			int				GetWidth		(void)																			const;
			void			Render			(int X, int Y, SDL_Surface * Destination)										const;
			void			SetImage		(const char * File);
			void			SetInteger		(int Value, TTF_Font * Font, bool ShowZero = true);
			void			SetText			(const char * Text, TTF_Font * Font, int R = 0, int G = 0, int B = 0);
							operator bool	(void)																			const;

	static	bool			Draw			(SDL_Surface * Destination, SDL_Surface * Source, int X, int Y, bool Free = false);
	static	SDL_Surface *	Load			(const char * File);
	static	SDL_Surface *	RenderText		(const char * Text, TTF_Font *Font, int R, int G, int B);
private:
			bool			CheckCache		(const char * Text);

	SDL_Surface		*MySurface;
	char			*Cached;
	int				Integer,
					Length;
};

}

#endif

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

#define	ANDROID_DEVICE
//#define	WEBOS_DEVICE
//#define DEBUG

#ifdef	WEBOS_DEVICE
#define	SOFTWARE_MODE
#endif

#if defined	ANDROID_DEVICE
#define	SOFTWARE_MODE
#include <android/log.h>
#define LOG_TAG "com.webosnerd.sdlmille"
#define DEBUG_PRINT(s) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, s)
#elif defined DEBUG
#define DEBUG_PRINT(s) printf(s)
#else
#define DEBUG_PRINT(s) //
#endif

#ifndef _SDLMILLE_SURFACE_H
#define _SDLMILLE_SURFACE_H

#include "Dimensions.h"

#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <cmath>
#include <fstream>

namespace _SDLMille
{

enum {SCALE_NONE, SCALE_X, SCALE_Y, SCALE_X_Y};

class Surface
{
public:
							Surface			(void);
							~Surface		(void);
			void			Clear			(void);
			bool			DrawPart		(SDL_Rect &SourceRect, SDL_Surface *Destination);
			int				GetHeight		(void)																			const;
			int				GetWidth		(void)																			const;
			void			Render			(int X, int Y, SDL_Surface * Destination, int ScaleMode = SCALE_X_Y)			const;
			void			SetAlpha		(int AlphaValue);
			void			SetImage		(const char * File);
			void			SetInteger		(int Value, TTF_Font * Font, bool ShowZero = true, SDL_Color *fgColor = 0, SDL_Color *bgColor = 0);
			void			SetText			(const char * Text, TTF_Font * Font, SDL_Color *fgColor = 0, SDL_Color *bgColor = 0);
							operator bool	(void)																			const;

	static	bool			Draw			(SDL_Surface * Destination, SDL_Surface * Source, int X, int Y, int ScaleMode = SCALE_X_Y, bool Free = false);
	static	SDL_Surface *	Load			(const char * File);
	static	SDL_Surface *	RenderText		(const char * Text, TTF_Font *Font, SDL_Color *fgColor = 0, SDL_Color *bgColor = 0);
private:
			bool			CheckCache		(const char * Text);

	SDL_Surface		*MySurface;
	char			*Cached;
	int				Integer,
					Length;
};


inline					Surface::operator bool	(void)	const
{
	return (MySurface != 0);
}

}

#endif

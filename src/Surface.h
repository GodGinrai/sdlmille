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

//#define	ANDROID_DEVICE
//#define	WEBOS_DEVICE
//#define DEBUG

#ifdef	WEBOS_DEVICE
	#define	SOFTWARE_MODE
#endif

#ifdef	ANDROID_DEVICE
	#define	SOFTWARE_MODE
#endif

#ifdef	DEBUG
	#ifdef	ANDROID_DEVICE
		#include <android/log.h>
		#define LOG_TAG "com.webosnerd.sdlmille"
		#define DEBUG_PRINT(s) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, s)
	#else
		#define DEBUG_PRINT(s) printf(s)
	#endif
#else
	#define DEBUG_PRINT(s) //
#endif

#ifndef _SDLMILLE_SURFACE_H
#define _SDLMILLE_SURFACE_H

#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <cmath>
#include <fstream>
#include "Dimensions.h"

namespace _SDLMille
{

enum {SCALE_NONE, SCALE_X, SCALE_Y, SCALE_X_Y};

enum	{CENTER_X = 1, CENTER_Y, CENTER_X_Y};

class Surface
{
public:
							Surface			(void);
							~Surface		(void);
			void			Center			(Uint8 CenterMode);
			void			Clear			(void);
			bool			DrawPart		(SDL_Rect &SourceRect, SDL_Surface *Destination);
			void			Fill			(int X, int Y, SDL_Surface *Target);
			void			GetCoords		(int &XCoord, int &YCoord)														const;
			int				GetHeight		(void)																			const;
			int				GetWidth		(void)																			const;
			int				GetX			(void)																			const;
			int				GetY			(void)																			const;
			void			Render			(SDL_Surface *Destination)														const;
			void			Render			(int X, int Y, SDL_Surface * Destination, int ScaleMode = SCALE_X_Y)			const;
			void			RepeatX			(int StartX, int Y, SDL_Surface *Target)						const;
			void			RepeatY			(int StartY, int X, SDL_Surface *Target)						const;
			void			SetAlpha		(int AlphaValue);
			void			SetCoords		(int XCoord, int YCoord);
			void			SetImage		(const char * File);
			void			SetInteger		(int Value, TTF_Font * Font, bool ShowZero = true, SDL_Color *fgColor = 0, SDL_Color *bgColor = 0);
			//void			SetRGBALoss		(Uint8 R, Uint8 G, Uint8 B, Uint8 A);
			void			SetText			(const char * Text, TTF_Font * Font, SDL_Color *fgColor = 0, SDL_Color *bgColor = 0);
			void			SetX			(int XCoord);
			void			SetY			(int YCoord);
							operator bool	(void)																			const;

	static	bool			Draw			(SDL_Surface * Destination, SDL_Surface * Source, int X, int Y, int ScaleMode = SCALE_X_Y, bool Free = false);
	static	SDL_Surface *	Load			(const char * File);
	static	SDL_Surface *	RenderText		(const char * Text, TTF_Font *Font, SDL_Color *fgColor = 0, SDL_Color *bgColor = 0);
private:
			bool			CheckCache		(const char * Text);

	SDL_Surface		*MySurface;
	char			*Cached;
	int				Integer,
					Length,
					X,
					Y;
};

inline	int				Surface::GetX			(void)	const
{
	return X;
}

inline	int				Surface::GetY			(void)	const
{
	return Y;
}

inline	void			Surface::SetX			(int XCoord)
{
	X = XCoord;
}

inline	void			Surface::SetY			(int YCoord)
{
	Y = YCoord;
}

inline					Surface::operator bool	(void)	const
{
	return (MySurface != 0);
}

}

#endif

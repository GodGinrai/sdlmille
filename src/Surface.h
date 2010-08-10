#define	WEBOS_DEVICE
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
							Surface		(void);
							~Surface	(void);
			void			Clear		(void);
			int				GetHeight	(void)															const;
			int				GetWidth	(void)															const;
			void			Render		(int X, int Y, SDL_Surface * Destination)						const;
			void			SetImage	(const char * File);
			void			SetInteger	(int Value, TTF_Font * Font, bool ShowZero = true);
			void			SetText		(const char * Text, TTF_Font * Font, int R = 0, int G = 0, int B = 0);
							operator bool	(void)														const;

	static	bool			Draw		(SDL_Surface * Destination, SDL_Surface * Source, int X, int Y);
	static	SDL_Surface *	Load		(const char * File);
	static	SDL_Surface *	RenderText	(const char * Text, TTF_Font *Font, int R, int G, int B);

private:
	bool			CheckCache	(const char * Text);
	SDL_Surface		*MySurface;
	char			*Cached;
	int				Integer,
					Length;
};

}

#endif

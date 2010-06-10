#ifndef _SDLMILLE_SURFACE_H
#define _SDLMILLE_SURFACE_H

#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

namespace _SDLMille
{

class Surface
{
public:
	static	bool			Draw		(SDL_Surface * Destination, SDL_Surface * Source, int X, int Y);
	static	SDL_Surface *	Load		(const char * File);
	static	SDL_Surface *	RenderText	(const char * Text, TTF_Font *Font);
};

}

#endif
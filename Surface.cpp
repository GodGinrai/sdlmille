#include "Surface.h"

namespace _SDLMille
{

bool			Surface::Draw	(SDL_Surface * Destination, SDL_Surface * Source, int X, int Y)
{
	if (!Destination || !Source)
		return false;

	SDL_Rect	DestRect;

	DestRect.x = X;
	DestRect.y = Y;

	SDL_BlitSurface(Source, 0, Destination, &DestRect);

	return true;
}

SDL_Surface *	Surface::Load	(const char * File)
{
	if (File)
	{
		SDL_Surface	*Loaded = 0,
					*Formatted = 0;

		Loaded = IMG_Load(File);

		if (!Loaded)
			return 0;

		Formatted = SDL_DisplayFormatAlpha(Loaded);
		SDL_FreeSurface(Loaded);

		if (Formatted)
			return Formatted;
		else
			return 0;
	}
	else
		return 0;
}

SDL_Surface *	Surface::RenderText	(const char *Text, TTF_Font *Font)
{
	SDL_Color	TextColor = {0, 0, 0, 0};
	SDL_Surface	*Screen = 0,
				*TextSurface = 0;
	Screen = SDL_GetVideoSurface();

	if (Font && Screen)
	{
		if (!TTF_WasInit())
			// If TTF hasn't been initialized, there's no way *Font would be a
			// valid pointer.
			return 0;

		TextSurface = TTF_RenderText_Blended(Font, Text, TextColor);
		return TextSurface;
	}

	return 0;
}

}
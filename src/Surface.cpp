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

#include "Surface.h"

namespace _SDLMille
{

bool			Surface::Draw	(SDL_Surface * Destination, SDL_Surface * Source, int X, int Y)
{
	if (!Destination || !Source)
		return false;

	#ifdef	ANDROID_DEVICE
		X *= 1.5;
		Y *= 1.5;
		Y += 40;
	#endif

	SDL_Rect	DestRect;

	DestRect.x = X;
	DestRect.y = Y;

	SDL_BlitSurface(Source, 0, Destination, &DestRect);

	return true;
}

SDL_Surface *	Surface::Load	(const char * File)
{
	printf("Loading file.\n");
	
	if (File)
	{
		printf("-File provided.\n");
		
		SDL_Surface	*Loaded = 0,
					*Formatted = 0;

		printf("-Pointers created.\n");
	
		#ifdef	ANDROID_DEVICE
		char	*NewFile = 0;
		NewFile = new	char[strlen(File) + 5];
		if (NewFile)
		{
			strcpy(NewFile, "hd/");
			strcat(NewFile, File);
			Loaded = IMG_Load(NewFile);
			delete	[]	NewFile;
			NewFile = 0;
		}
		#else
		Loaded = IMG_Load(File);
		#endif

		if (!Loaded)
		{
			printf("-Not loaded.\n");
			return 0;
		}
		
		printf("-Loaded.\n");

		Formatted = SDL_DisplayFormatAlpha(Loaded);
		
		printf("-Formatted.\n");
		
		SDL_FreeSurface(Loaded);
		
		printf("-Freed.\n");

		if (Formatted)
		{
			printf("-Returning formatted surface.\n");
			return Formatted;
		}
		else
		{
			printf("-Unable to format. Returning 0.\n");
			return 0;
		}
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

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

#include "Game.h"
#ifdef WEBOS_DEVICE
#include <unistd.h>
#include "PDL.h"
#endif

using namespace _SDLMille;

int	main	(int argc, char* argv[])
{
	#ifdef WEBOS_DEVICE
	/*
	char WorkingDir[101];
	PDL_GetCallingPath(WorkingDir, 100);
	chdir(WorkingDir);
	
	Hack: PDL_GetCallingPath is not actually present yet :(
	*/
	chdir("/media/cryptofs/apps/usr/palm/applications/com.webosnerd.sdl-mille/");
	#endif

	bool	Success;
	Game	TheGame;

	Success = TheGame.OnExecute();

	if (SDL_WasInit(SDL_INIT_VIDEO))
		SDL_Quit();

	// Return 0 if the game executed OK. Otherwise, return -1.
	return (Success) ? 0 : -1;
}

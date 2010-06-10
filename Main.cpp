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

using namespace _SDLMille;

int	main	(int argc, char* argv[])
{
	printf("Started.");
	
	try
	{
		Game	TheGame;

		// Return 0 if the game executed OK. Otherwise, return -1.
		printf("Trying to execute game.");
		return (TheGame.OnExecute()) ? 0 : -1;
	}
	catch (int e)
	{
		printf("Error number %u caught.", e);
	}

	return 0;
}
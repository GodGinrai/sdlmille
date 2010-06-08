#include "Game.h"

using namespace _SDLMille;

int	main	(int argc, char* argv[])
{
	try
	{
		Game	TheGame;

		// Return 0 if the game executed OK. Otherwise, return -1.
		return (TheGame.OnExecute()) ? 0 : -1;
	}
	catch (int e) {}

	return 0;
}
#include "Game.h"
#include <windows.h>

/// <summary>
/// The Main function - Without this, you are completely fucked.
/// </summary>
/// <returns>1 for successful exit.</returns>
int main()
{
	Game *f_game = new Game();
	f_game->run();

	delete f_game;

	return 1;
}
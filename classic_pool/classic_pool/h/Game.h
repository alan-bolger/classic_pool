#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>

class Game
{
public:
	Game();
	~Game();
	void run();

private:
	sf::RenderWindow window;
	bool exitGame{ false };

	void processEvents();
	void update(sf::Time dt);
	void draw();
};

#endif // !GAME_H

#ifndef GAME_HPP
#define GAME_HPP

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
	float width = 2.84f;
	float height = 1.42f;
	float radius = 0.0285f;

	void processEvents();
	void update(sf::Time dt);
	void draw();
};

#endif // !GAME_H

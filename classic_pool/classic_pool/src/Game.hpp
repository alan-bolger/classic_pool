#ifndef GAME_HPP
#define GAME_HPP

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>

#include "physics/Ball.hpp"
#include "game/Table.hpp"

class Game
{
    public:

    Game();
    void run();

    private:

    sf::RenderWindow window;
    sf::View view;
    Table table;
	std::vector<Ball> balls;
    bool running = true;

    void processEvents();
    void update(float dt);
    void render();
    void createRack();
    void shootCueBall(sf::Vector2i mousePosition);
};

#endif // !GAME_HPP
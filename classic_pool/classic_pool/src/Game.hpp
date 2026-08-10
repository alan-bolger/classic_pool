#ifndef GAME_HPP
#define GAME_HPP

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>

#include "physics/Ball.hpp"
#include "game/Table.hpp"

constexpr float MAX_PULLBACK = 0.50f;
constexpr float MAX_SHOT_POWER = 3.0f;
constexpr float MIN_CUE_STRIKE_SPEED = 0.15f;
constexpr float MAX_CUE_STRIKE_SPEED = 12.0f;

enum class CueState
{
    Hidden,
    Aiming,
    Striking
};

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
    bool aiming = false;
    sf::Vector2f aimDirection;
    float shotPower = 0.0f;
    float maxShotPower = 2.0f;
    sf::RectangleShape cue;
    sf::Vector2f aimStart;
    CueState cueState = CueState::Hidden;
    float cuePullback = 0.0f;
    float cueStrikeDistance = 0.15f;
    float cueStrikeSpeed = 4.0f;

    void processEvents();
    void update(float dt);
    void render();
    void createRack();
    void updateAim();
    void updateCue(float dt);
    void beginStrike();
    void shootCueBall();
};

#endif // !GAME_HPP
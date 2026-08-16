#ifndef GAME_HPP
#define GAME_HPP

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>

#include "physics/Ball.hpp"
#include "game/Table.hpp"

constexpr float MAX_PULLBACK = 0.5f;
constexpr float MAX_SHOT_POWER = 10.0f;
constexpr float MIN_STRIKE_DURATION = 0.50f;
constexpr float MAX_STRIKE_DURATION = 0.08f;
constexpr float CUE_REST_DISTANCE = 0.02f;
constexpr float FOLLOW_THROUGH_DISTANCE = 0.02f;
constexpr float FOLLOW_THROUGH_SPEED = 0.6f;

constexpr float TRAJECTORY_LENGTH = 3.0f;
constexpr int TRAJECTORY_MAX_BOUNCES = 5;
constexpr float TRAJECTORY_STEP = 0.01f;
constexpr float TRAJECTORY_MAX_DISTANCE = 3.0f;
constexpr float TRAJECTORY_LINE_WIDTH = 0.006f;
constexpr float EPSILON = 0.00001f;


enum class CueState
{
    Hidden,
    Aiming,
    Striking,
    FollowThrough
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
    float cueStrikeProgress = 0.0f;
    float cueStrikeDuration = 0.0f;
    float cueFollowThrough = 0.0f;

    void processEvents();
    void update(float dt);
    void render();
    void createRack();
    void updateAim();
    void updateCue(float dt);
    void renderGhostBall(std::size_t ballIndex, sf::Vector2f collisionPosition);
    void renderTrajectory();
    void renderPocketHighlight(std::size_t pocketIndex);
    bool findPocketIntersection(sf::Vector2f position, sf::Vector2f direction, float maxDistance, std::size_t &pocketIndex, float &distance) const;
    bool findCushionIntersection(sf::Vector2f position, sf::Vector2f direction, float maxDistance, float &distance, sf::Vector2f &normal) const;
    bool findBallIntersection(sf::Vector2f position, sf::Vector2f direction, float maxDistance, std::size_t &ballIndex, float &distance) const;
    void respawnCueBallIfPocketed();
    void beginStrike();
	void strikeCueBall();
    void shootCueBall();
    Ball &getCueBall();
    const Ball &getCueBall() const;
    sf::Color getBallColor(const Ball &ball);
};

#endif // !GAME_HPP
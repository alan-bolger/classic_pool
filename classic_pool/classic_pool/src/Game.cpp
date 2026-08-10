#include "Game.hpp"
#include "physics/Physics.hpp"

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/System/Clock.hpp>
#include <algorithm>

namespace
{
    constexpr float FIXED_DT = 1.0f / 120.0f;
    constexpr float MAX_SHOT_POWER = 2.0f;
}

/// <summary>
/// Game constructor.
/// </summary>
Game::Game() : window(sf::VideoMode({ 1920, 1080 }), "Classic Pool"), view(sf::FloatRect({ 0.0f, 0.0f }, { 2.84f, 1.42f }))
{
    window.setView(view);
    createRack();
}

/// <summary>
/// Run application.
/// </summary>
void Game::run()
{
    sf::Clock clock;
    float accumulator = 0.0f;

    while (window.isOpen() && running)
    {
        const float frameTime = clock.restart().asSeconds();

        // Avoid the simulation exploding after the window is dragged/minimized/etc
        accumulator += std::min(frameTime, 0.25f);
        processEvents();

        while (accumulator >= FIXED_DT)
        {
            update(FIXED_DT);
            accumulator -= FIXED_DT;
        }

        render();
    }
}

/// <summary>
/// Process events.
/// </summary>
void Game::processEvents()
{
    while (const std::optional event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            window.close();
        }

        if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>())
        {
            if (keyPressed->code == sf::Keyboard::Key::Escape)
            {
                window.close();
            }
        }

        if (const auto *mousePressed = event->getIf<sf::Event::MouseButtonPressed>())
        {
            if (mousePressed->button == sf::Mouse::Button::Left)
            {
                shootCueBall(mousePressed->position);
            }
        }
    }
}

/// <summary>
/// Update.
/// </summary>
/// <param name="dt">Delta time.</param>
void Game::update(float dt)
{
    Physics::update(balls, table, dt);
}

/// <summary>
/// Render everything.
/// </summary>
void Game::render()
{
    window.clear(sf::Color(20, 20, 20));

    table.render(window);

    for (const Ball &ball : balls)
    {
        ball.render(window);
    }

    window.display();
}

/// <summary>
/// Create the rack of balls at the start of the game.
/// </summary>
void Game::createRack()
{
    balls.clear();

    // Cue ball
    balls.emplace_back(sf::Vector2f{ 0.71f, 0.71f });
    constexpr int rows = 5;
    const float spacing = 0.060f;
    const sf::Vector2f rackOrigin{ 1.85f, 0.71f };

    for (int row = 0; row < rows; ++row)
    {
        for (int column = 0; column <= row; ++column)
        {
            const float x = rackOrigin.x + row * spacing;
            const float y = rackOrigin.y + (column - row * 0.5f) * spacing;
            balls.emplace_back(sf::Vector2f{ x, y });
        }
    }

	balls[0].setColour(sf::Color::White);

    for (std::size_t i = 1; i < balls.size(); ++i)
    {
        balls[i].setColour(sf::Color::Yellow);
	}
}

/// <summary>
/// Shoot the cue ball/take shot.
/// </summary>
/// <param name="mousePosition">The current mouse position.</param>
void Game::shootCueBall(sf::Vector2i mousePosition)
{
	Ball &cueBall = balls[0];

    if (cueBall.isMoving())
    {
        return;
    }

    const sf::Vector2f mouseWorld = window.mapPixelToCoords(mousePosition);
    const sf::Vector2f direction = mouseWorld - cueBall.getPosition();

    if (direction.length() < 0.0001f)
    {
        return;
    }

    const sf::Vector2f normalised = direction.normalized();
    cueBall.setVelocity(normalised * MAX_SHOT_POWER);
}
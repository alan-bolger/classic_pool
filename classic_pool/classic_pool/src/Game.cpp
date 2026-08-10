#include "Game.hpp"
#include "physics/Physics.hpp"

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/System/Clock.hpp>
#include <algorithm>
#include <iostream>

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

    balls.emplace_back(
        sf::Vector2f{ 0.71f, 0.71f }
    );

    balls.emplace_back(
        sf::Vector2f{ 1.40f, 0.71f }
    );
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
                std::cout << "Click!" << std::endl;
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
/// Shoot the cue ball/take shot.
/// </summary>
/// <param name="mousePosition">The current mouse position.</param>
void Game::shootCueBall(sf::Vector2i mousePosition)
{
	Ball &cueBall = balls[0];

    if (cueBall.isMoving())
    {
        std::cout << "Ball is already moving\n";
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
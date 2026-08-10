#include "Game.hpp"
#include "physics/Physics.hpp"

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/System/Clock.hpp>
#include <algorithm>

/// <summary>
/// Game constructor.
/// </summary>
Game::Game() : window(sf::VideoMode({ 1920, 1080 }), "Classic Pool"), view(sf::FloatRect({ 0.0f, 0.0f }, { 2.84f, 1.42f }))
{
    window.setView(view);
    createRack();

	// Set up cue shape
    cue.setSize({ 0.8f, 0.015f });
    cue.setOrigin({ 0.0f, 0.0075f });
    cue.setFillColor(sf::Color(180, 130, 70));
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

		float FIXED_DT = 1.0f / 120.0f;

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
                if (!balls[0].isMoving() && cueState == CueState::Hidden)
                {
                    cueState = CueState::Aiming;
                }
            }
        }

        if (const auto *mouseReleased = event->getIf<sf::Event::MouseButtonReleased>())
        {
            if (mouseReleased->button == sf::Mouse::Button::Left)
            {
                if (cueState == CueState::Aiming)
                {
                    beginStrike();
                }
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

    if (cueState == CueState::Aiming)
    {
        updateAim();
    }

    updateCue(dt);
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

    if (cueState != CueState::Hidden)
    {
        window.draw(cue);
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
/// Update the aim direction and shot power based on the mouse position.
/// </summary>
void Game::updateAim()
{
    const sf::Vector2i mousePixel =
        sf::Mouse::getPosition(window);

    const sf::Vector2f mouseWorld =
        window.mapPixelToCoords(mousePixel);

    const sf::Vector2f ballPosition =
        balls[0].getPosition();

    const sf::Vector2f mouseDirection =
        mouseWorld - ballPosition;

    if (mouseDirection.length() < 0.0001f)
        return;

    // Shot direction points away from the mouse.
    aimDirection =
        -mouseDirection.normalized();

    // How far the mouse has been pulled behind
    // the cue ball.
    cuePullback =
        std::clamp(
        mouseDirection.length(),
        0.0f,
        MAX_PULLBACK
        );

    const float powerRatio =
        cuePullback / MAX_PULLBACK;

    shotPower =
        powerRatio * MAX_SHOT_POWER;
}

/// <summary>
/// Update the cue's position and rotation.
/// </summary>
/// <param name="dt">Delta time.</param>
void Game::updateCue(float dt)
{
    if (cueState == CueState::Hidden)
    {
        return;
    }

    const sf::Vector2f ballPosition = balls[0].getPosition();

    if (cueState == CueState::Aiming)
    {
        // The tip is pulled backwards away from the direction of the shot
        cue.setPosition(ballPosition - aimDirection * cuePullback);
        const float angle = std::atan2(-aimDirection.y, -aimDirection.x);
        cue.setRotation(sf::radians(angle));
    }
    else if (cueState == CueState::Striking)
    {
        const float powerRatio = shotPower / MAX_SHOT_POWER;
        const float strikeSpeed = MIN_CUE_STRIKE_SPEED + (MAX_CUE_STRIKE_SPEED - MIN_CUE_STRIKE_SPEED) * (powerRatio * powerRatio);
        cuePullback -= strikeSpeed * dt;

        // The cue has reached the ball
        if (cuePullback <= 0.0f)
        {
            cuePullback = 0.0f;
            cue.setPosition(ballPosition);
            shootCueBall();
            cueState = CueState::Hidden;

            return;
        }

        cue.setPosition(ballPosition - aimDirection * cuePullback);
        const float angle = std::atan2(-aimDirection.y, -aimDirection.x);
        cue.setRotation(sf::radians(angle));
    }
}

/// <summary>
/// Initiates a cue strike.
/// </summary>
void Game::beginStrike()
{
    if (shotPower <= 0.0f)
    {
        cueState = CueState::Hidden;
        return;
    }

    cueState = CueState::Striking;
}

/// <summary>
/// Shoot the cue ball/take shot.
/// </summary>
void Game::shootCueBall()
{
    //if (shotPower <= 0.0f)
    //{
    //    return;
    //}

	Ball &cueBall = balls[0];

    //if (cueBall.isMoving())
    //{
    //    return;
    //}

    //const sf::Vector2f mouseWorld = window.mapPixelToCoords(mousePosition);
    //const sf::Vector2f direction = mouseWorld - cueBall.getPosition();

    //if (direction.length() < 0.0001f)
    //{
    //    return;
    //}

    //const sf::Vector2f normalised = direction.normalized();
    //cueBall.setVelocity(normalised * MAX_SHOT_POWER);

    cueBall.setVelocity(aimDirection * shotPower);
    shotPower = 0.0f;
	cuePullback = 0.0f;
}
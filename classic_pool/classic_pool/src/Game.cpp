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
                if (!getCueBall().isMoving() && cueState == CueState::Hidden)
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

    if (cueState == CueState::Aiming)
    {
        renderTrajectory();
    }

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
    balls.emplace_back(sf::Vector2f{ 0.71f, 0.71f }, BallType::Cue, 0);
    constexpr float spacing = 0.060f;
    const sf::Vector2f rackOrigin{ 1.85f, 0.71f };
    int number = 1;

    for (int row = 0; row < 5; ++row)
    {
        for (int column = 0; column <= row; ++column)
        {
            BallType type;

            if (number == 8)
            {
                type = BallType::Eight;
            }
            else if (number <= 7)
            {
                type = BallType::Solid;
            }
            else
            {
                type = BallType::Stripe;
            }

            const float x = rackOrigin.x + row * spacing;
            const float y = rackOrigin.y + (column - row * 0.5f) * spacing;
            balls.emplace_back(sf::Vector2f{ x, y }, type, number);
            ++number;
        }
    }

    for (Ball &ball : balls)
    {
        ball.setColour(getBallColor(ball));
    }
}

/// <summary>
/// Update the aim direction and shot power based on the mouse position.
/// </summary>
void Game::updateAim()
{
    const sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
    const sf::Vector2f mouseWorld = window.mapPixelToCoords(mousePixel);
    const sf::Vector2f ballPosition = getCueBall().getPosition();
    const sf::Vector2f mouseDirection = mouseWorld - ballPosition;

    if (mouseDirection.length() < 0.0001f)
    {
        return;
    }

    // Shot direction points away from the mouse
    aimDirection = -mouseDirection.normalized();

    // How far the mouse has been pulled behind the cue ball
    cuePullback = std::clamp(mouseDirection.length(), 0.0f, MAX_PULLBACK);
    const float powerRatio = cuePullback / MAX_PULLBACK;
    shotPower = powerRatio * MAX_SHOT_POWER;
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

    const sf::Vector2f ballPosition = getCueBall().getPosition();
    const float angle = std::atan2(-aimDirection.y, -aimDirection.x);
    cue.setRotation(sf::radians(angle));

    if (cueState == CueState::Aiming)
    {
        cue.setPosition(ballPosition - aimDirection * (CUE_REST_DISTANCE + cuePullback));
        return;
    }

    if (cueState == CueState::Striking)
    {
        cueStrikeProgress += dt / cueStrikeDuration;
        const float progress = std::clamp(cueStrikeProgress, 0.0f, 1.0f);

        // Smooth acceleration:
        // slow at the beginning, fast near impact
        const float easedProgress = progress * progress;

        cuePullback = cuePullback * (1.0f - easedProgress);

        cue.setPosition(ballPosition - aimDirection * (CUE_REST_DISTANCE + cuePullback));

        if (progress >= 1.0f)
        {
            cuePullback = 0.0f;
            cue.setPosition(ballPosition - aimDirection * CUE_REST_DISTANCE);
            strikeCueBall();
            cueState = CueState::FollowThrough;
            cueFollowThrough = 0.0f;
        }

        return;
    }

    if (cueState == CueState::FollowThrough)
    {
        cueFollowThrough += FOLLOW_THROUGH_SPEED * dt;
        cue.setPosition(ballPosition - aimDirection * (CUE_REST_DISTANCE - cueFollowThrough));

        if (cueFollowThrough >= FOLLOW_THROUGH_DISTANCE)
        {
            cueState = CueState::Hidden;
        }
    }
}

/// <summary>
/// Renders a translucent "ghost" circle for a ball at the contact point to visualize a collision. The ghost position is computed by projecting the ball out along the collision normal and uses the reference radius of balls[0].
/// </summary>
/// <param name="ballIndex">Index of the ball in the Game::balls container for which to render the ghost. Must refer to a valid element.</param>
/// <param name="collisionPosition">World position of the collision contact used to compute the collision normal and determine the ghost's placement.</param>
void Game::renderGhostBall(std::size_t ballIndex, sf::Vector2f collisionPosition)
{
    const Ball &objectBall = balls[ballIndex];
    const sf::Vector2f collisionNormal = (objectBall.getPosition() - collisionPosition).normalized();
    const sf::Vector2f ghostPosition = objectBall.getPosition() - collisionNormal * (getCueBall().getRadius() + objectBall.getRadius());

    sf::CircleShape ghost;
    ghost.setRadius(getCueBall().getRadius());
    ghost.setOrigin({getCueBall().getRadius(), getCueBall().getRadius()});
    ghost.setPosition(ghostPosition);
    ghost.setFillColor(sf::Color(255, 255, 255, 25));
    ghost.setOutlineColor(sf::Color(255, 255, 255, 180));
    ghost.setOutlineThickness(0.006f);
    window.draw(ghost);
}

/// <summary>
/// Render the trajectory of the cue ball based on the current aim direction and shot power.
/// </summary>
void Game::renderTrajectory()
{
    if (cueState != CueState::Aiming)
    {
        return;
    }

    if (shotPower <= 0.0f)
    {
        return;
    }

    const Ball &cueBall = getCueBall();
    sf::Vector2f position = cueBall.getPosition();
    sf::Vector2f direction = aimDirection;
    float remainingDistance = TRAJECTORY_MAX_DISTANCE;
    sf::VertexArray cueTrajectory(sf::PrimitiveType::LineStrip);
    cueTrajectory.append(sf::Vertex(position));
    bool hitBall = false;

    for (int bounce = 0; bounce <= TRAJECTORY_MAX_BOUNCES; ++bounce)
    {
        float cushionDistance = remainingDistance;
        float ballDistance = remainingDistance;
        sf::Vector2f cushionNormal;
        std::size_t ballIndex = 0;
        const bool hitCushion = findCushionIntersection(position, direction, remainingDistance, cushionDistance, cushionNormal);
        const bool hitObjectBall = findBallIntersection(position, direction, remainingDistance, ballIndex, ballDistance);

        // Object ball is the first collision
        if (hitObjectBall && ballDistance <= cushionDistance)
        {
            position += direction * ballDistance;
            cueTrajectory.append(sf::Vertex(position));
            hitBall = true;

            renderGhostBall(ballIndex, position);

            // Work out the collision normal
            const sf::Vector2f collisionNormal = (balls[ballIndex].getPosition() - position).normalized();

            // For an initially stationary object ball,
            // the object ball travels along the collision
            // normal
            const sf::Vector2f objectPosition = balls[ballIndex].getPosition();
            sf::Vector2f objectDirection = collisionNormal;
            float pocketDistance = TRAJECTORY_MAX_DISTANCE;
            std::size_t pocketIndex = 0;
            const bool hitPocket = findPocketIntersection(objectPosition, objectDirection, TRAJECTORY_MAX_DISTANCE, pocketIndex, pocketDistance);

            // The cue ball continues along the tangent
            sf::Vector2f cueDirection = direction - collisionNormal * direction.dot(collisionNormal);

            if (cueDirection.length() > EPSILON)
            {
                cueDirection = cueDirection.normalized();

                // Draw the cue ball's predicted path
                sf::VertexArray cueAfterCollision(sf::PrimitiveType::LineStrip);
                cueAfterCollision.append(sf::Vertex(position));
                cueAfterCollision.append(sf::Vertex(position + cueDirection * 0.5f));
                window.draw(cueAfterCollision);
            }

            sf::VertexArray objectTrajectory(sf::PrimitiveType::LineStrip);
            objectTrajectory.append(sf::Vertex(objectPosition));

            if (hitPocket)
            {
                const sf::Vector2f pocketPosition = table.getPockets()[pocketIndex];
                objectTrajectory.append(sf::Vertex(objectPosition + objectDirection * pocketDistance));
                renderPocketHighlight(pocketIndex);
            }
            else
            {
                objectTrajectory.append(sf::Vertex(objectPosition + objectDirection * 0.75f));
            }

            window.draw(objectTrajectory);

            break;
        }

        // No cushion
        if (!hitCushion)
        {
            position += direction * remainingDistance;
            cueTrajectory.append(sf::Vertex(position));

            break;
        }

        // Cushion is the first collision
        position += direction * cushionDistance;
        cueTrajectory.append(sf::Vertex(position));
        remainingDistance -= cushionDistance;

        // Reflect direction
        direction = direction - 2.0f * direction.dot(cushionNormal) * cushionNormal;

        // Move slightly away from the cushion so we
        // don't immediately detect the same collision
        position += direction * 0.0001f;
    }

    window.draw(cueTrajectory);
}

/// <summary>
/// Renders a semi-transparent circular highlight over the specified pocket on the game table.
/// </summary>
/// <param name="pocketIndex">Index of the pocket to highlight. Must be a valid index into the table's pocket list (table.getPockets()).</param>
void Game::renderPocketHighlight(std::size_t pocketIndex)
{
    const float radius = table.getPocketRadius();

    sf::CircleShape highlight;
    highlight.setRadius(radius * 1.15f);
    highlight.setOrigin({ radius * 1.15f, radius * 1.15f });
    highlight.setPosition(table.getPockets()[pocketIndex]);
    highlight.setFillColor(sf::Color(255, 255, 255, 30));
    highlight.setOutlineColor(sf::Color(255, 255, 255, 180));
    highlight.setOutlineThickness(0.008f);

    window.draw(highlight);
}

/// <summary>
/// Searches along a ray from a position in a given direction for the nearest pocket capture intersection within a maximum distance.
/// </summary>
/// <param name="position">The start position (center of the object ball) in world coordinates from which the search begins.</param>
/// <param name="direction">A unit (or directional) vector indicating the search direction.</param>
/// <param name="maxDistance">The maximum distance to search along the direction. The search will not consider intersections beyond this distance.</param>
/// <param name="pocketIndex">Output parameter (reference). On success, set to the index of the pocket that the ray intersects first.</param>
/// <param name="distance">Output parameter (reference). On success, set to the distance from position to the intersection along direction. If no intersection is found it remains equal to maxDistance.</param>
/// <returns>true if a pocket intersection was found within maxDistance (and pocketIndex and distance are updated), false otherwise.</returns>
bool Game::findPocketIntersection(sf::Vector2f position, sf::Vector2f direction, float maxDistance, std::size_t &pocketIndex, float &distance) const
{
    bool found = false;    
    distance = maxDistance;    
    const float ballRadius = getCueBall().getRadius();
    
    // How far the centre of the object ball needs
    // to get into the pocket
    const float pocketCaptureRadius = table.getPocketRadius() + ballRadius * 0.35f;
    
    for (std::size_t i = 0; i < table.getPockets().size(); ++i)
    {
        const sf::Vector2f toPocket = table.getPockets()[i] - position;    
        const float projection = toPocket.dot(direction);
    
        if (projection <= 0.0f)
        {
            continue;
        }
    
        const float closestDistanceSquared = toPocket.dot(toPocket) - projection * projection;    
        const float captureRadiusSquared = pocketCaptureRadius * pocketCaptureRadius;
    
        if (closestDistanceSquared > captureRadiusSquared)
        {
            continue;
        }
    
        const float offset = std::sqrt(captureRadiusSquared - closestDistanceSquared);    
        const float intersectionDistance = projection - offset;
    
        if (intersectionDistance < 0.0f)
        {
            continue;
        }
    
        if (intersectionDistance < distance)
        {
            distance = intersectionDistance;
            pocketIndex = i;
            found = true;
        }
    }
    
    return found;
}

/// <summary>
/// Finds the nearest intersection (if any) between a ray starting at position and directed by direction and the table cushions, accounting for the ball radius by offsetting the cushion inward.
/// </summary>
/// <param name="position">The starting point of the ray (ball center position).</param>
/// <param name="direction">The direction vector of the ray. Should be a non-zero vector; not required to be normalized.</param>
/// <param name="maxDistance">The maximum distance along the ray to search for an intersection.</param>
/// <param name="distance">Output parameter. On return, set to the distance from position to the nearest cushion intersection found (unchanged if no intersection).</param>
/// <param name="normal">Output parameter. On return, set to the inward-facing normal of the cushion at the intersection point (undefined if no intersection).</param>
/// <returns>True if an intersection was found within maxDistance (and distance/normal were updated), false otherwise.</returns>
bool Game::findCushionIntersection(sf::Vector2f position, sf::Vector2f direction, float maxDistance, float &distance, sf::Vector2f &normal) const
{
    bool found = false;
    distance = maxDistance;
    const sf::Vector2f tableCentre = (table.getMin() + table.getMax()) * 0.5f;
    const float ballRadius = getCueBall().getRadius();

    for (const Cushion &cushion : table.getCushions())
    {
        const sf::Vector2f segment = cushion.end - cushion.start;
        const float segmentLength = segment.length();

        if (segmentLength < EPSILON)
        {
            continue;
        }

        const sf::Vector2f tangent = segment / segmentLength;
        sf::Vector2f cushionNormal{ -tangent.y, tangent.x };

        // Make the normal point INTO the table
        const sf::Vector2f midpoint = (cushion.start + cushion.end) * 0.5f;

        if (cushionNormal.dot(tableCentre - midpoint) < 0.0f)
        {
            cushionNormal = -cushionNormal;
        }

        // The ball centre has to stay one radius
        // inside the cushion
        const sf::Vector2f offsetStart = cushion.start + cushionNormal * ballRadius;
        const sf::Vector2f toStart = offsetStart - position;
        const float cross = direction.x * tangent.y - direction.y * tangent.x;

        if (std::abs(cross) < EPSILON)
        {
            continue;
        }

        const float rayDistance = (toStart.x * tangent.y - toStart.y * tangent.x) / cross;
        const float segmentPosition = (toStart.x * direction.y - toStart.y * direction.x) / cross;

        if (rayDistance <= EPSILON)
        {
            continue;
        }

        if (segmentPosition < 0.0f || segmentPosition > segmentLength)
        {
            continue;
        }

        if (rayDistance < distance)
        {
            distance = rayDistance;
            normal = cushionNormal;
            found = true;
        }
    }

    return found;
}

/// <summary>
/// Searches for the nearest intersection between a ray and active balls and returns the first hit within a maximum distance.
/// </summary>
/// <param name="position">The origin point of the ray in world coordinates.</param>
/// <param name="direction">The direction of the ray. This is expected to be a (unit) direction vector; if not normalized, the reported distance will be scaled accordingly.</param>
/// <param name="maxDistance">The maximum distance along the ray to consider for collisions. The function initializes the output distance to this value.</param>
/// <param name="ballIndex">Output parameter. On success, set to the index of the intersected active ball. Only valid if the function returns true.</param>
/// <param name="distance">Output parameter. On success, set to the distance from position to the collision point (<= maxDistance). If no collision is found, this is left at maxDistance.</param>
/// <returns>true if an intersection with an active ball was found within maxDistance (and ballIndex/distance were updated), false otherwise.</returns>
bool Game::findBallIntersection(sf::Vector2f position, sf::Vector2f direction, float maxDistance, std::size_t &ballIndex, float &distance) const
{
    bool found = false;
    distance = maxDistance;
    const float radius = getCueBall().getRadius();
    const float collisionRadius = radius * 2.0f;

    for (std::size_t i = 1; i < balls.size(); ++i)
    {
        const Ball &ball = balls[i];

        if (!ball.isActive())
        {
            continue;
        }

        const sf::Vector2f toBall = ball.getPosition() - position;
        const float projection = toBall.dot(direction);

        if (projection <= 0.0f)
        {
            continue;
        }

        const float closestDistanceSquared = toBall.dot(toBall) - projection * projection;
        const float radiusSquared = collisionRadius * collisionRadius;

        if (closestDistanceSquared > radiusSquared)
        {
            continue;
        }

        const float offset = std::sqrt(radiusSquared - closestDistanceSquared);
        const float collisionDistance = projection - offset;

        if (collisionDistance <= EPSILON)
        {
            continue;
        }

        if (collisionDistance < distance)
        {
            distance = collisionDistance;
            ballIndex = i;
            found = true;
        }
    }

    return found;
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

    const float powerRatio = std::clamp(shotPower / MAX_SHOT_POWER, 0.0f, 1.0f);
    cueStrikeDuration = MIN_STRIKE_DURATION + (MAX_STRIKE_DURATION - MIN_STRIKE_DURATION) * powerRatio;
    cueStrikeProgress = 0.0f;
    cueState = CueState::Striking;
}

/// <summary>
/// Strike the cue ball with the cue stick.
/// </summary>
void Game::strikeCueBall()
{
    Ball &cueBall = getCueBall();
    cueBall.setVelocity(aimDirection * shotPower);

    shotPower = 0.0f;
    cuePullback = 0.0f;
}

/// <summary>
/// Shoot the cue ball/take shot.
/// </summary>
void Game::shootCueBall()
{
	Ball &cueBall = getCueBall();
    cueBall.setVelocity(aimDirection * shotPower);
    shotPower = 0.0f;
	cuePullback = 0.0f;
}

/// <summary>
/// Get the cueball.
/// </summary>
/// <returns>The cueball.</returns>
Ball &Game::getCueBall()
{
    return balls[0];
}

/// <summary>
/// Get the cueball (const version).
/// </summary>
/// <returns>The cueball.</returns>
const Ball &Game::getCueBall() const
{
    return balls[0];
}

/// <summary>
/// Returns the display color for a ball based on its BallType.
/// </summary>
/// <param name="ball">The ball whose type is used to determine the color.</param>
/// <returns>An sf::Color for the ball's type: Cue -> White, Eight -> Black, Solid -> Yellow, Stripe -> White. Defaults to White if the type is unrecognized.</returns>
sf::Color Game::getBallColor(const Ball &ball)
{
    switch (ball.getType())
    {
        case BallType::Cue:
            return sf::Color::White;

        case BallType::Eight:
            return sf::Color::Black;

        case BallType::Solid:
            return sf::Color::Yellow;

        case BallType::Stripe:
            return sf::Color::White;
    }

    return sf::Color::White;
}

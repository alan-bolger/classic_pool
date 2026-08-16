#include "Ball.hpp"

#include <cmath>

/// <summary>
/// Ball constructor.
/// </summary>
/// <param name="position">The ball's new position.</param>
/// <param name="type">The ball type (Cue, Eight, Solid, Stripe).</param>
/// <param name="number"></param>
Ball::Ball(sf::Vector2f position, BallType type, int number) : position(position), type(type), number(number)
{
    shape.setRadius(radius);
    shape.setOrigin({ radius, radius });
    shape.setFillColor(sf::Color::White);
    shape.setPosition(position);
}

/// <summary>
/// Update .
/// </summary>
/// <param name="dt">Delta time.</param>
void Ball::update(float dt)
{
    if (!active)
    {
        return;
    }

    // Sinking animation: shrink, fade, and glide into the pocket centre
    if (sinking)
    {
        sinkProgress += dt / sinkDuration;
        const float t = std::min(sinkProgress, 1.0f);

        // Ease the ball toward the pocket centre
        position += (sinkTarget - position) * std::min(1.0f, dt * 12.0f);

        // Scale down to 40% and fade alpha to zero
        const float scale = 1.0f - 0.3f * t;
        shape.setScale({ scale, scale });

        sf::Color faded = baseColour;
        faded.a = static_cast<std::uint8_t>(255.0f * (1.0f - t));
        shape.setFillColor(faded);
        shape.setPosition(position);

        if (t >= 1.0f)
        {
            sinking = false;
            active = false;
        }

        return;
    }

    position += velocity * dt;
    velocity *= std::pow(friction, dt); // Velocity is multplied by friction every second

    // Stop ball if velocity is small enough
    if (velocity.length() < 0.001f)
    {
        velocity = { 0.0f, 0.0f };
    }

    shape.setPosition(position);
}

/// <summary>
/// Render.
/// </summary>
/// <param name="target">The render target.</param>
void Ball::render(sf::RenderTarget &target) const
{
    if (!active)
    {
        return;
    }

    target.draw(shape);
}

/// <summary>
/// Get the ball's position.
/// </summary>
/// <returns></returns>
sf::Vector2f Ball::getPosition() const
{
    return position;
}

/// <summary>
/// Get the ball's velocity.
/// </summary>
/// <returns>The ball's velocity as a sf::Vector2f.</returns>
sf::Vector2f Ball::getVelocity() const
{
    return velocity;
}

/// <summary>
/// Set the ball's position.
/// </summary>
/// <param name="position">The ball's new position.</param>
void Ball::setPosition(sf::Vector2f position)
{
    this->position = position;
    shape.setPosition(position);
}

/// <summary>
/// Set the ball's velocity.
/// </summary>
/// <param name="velocity">The ball's new velocity.</param>
void Ball::setVelocity(sf::Vector2f velocity)
{
    this->velocity = velocity;
}

/// <summary>
/// Set the ball's colour.
/// </summary>
/// <param name="colour">The ball's new colour.</param>
void Ball::setColour(sf::Color colour)
{
    baseColour = colour;
    shape.setFillColor(colour);
}

/// <summary>
/// Get the ball's radius.
/// </summary>
/// <returns>The ball's radius as a float.
/// </returns>
float Ball::getRadius() const
{
    return radius;
}

/// <summary>
/// Check if the ball is moving.
/// </summary>
/// <returns>True if the ball is moving, otherwise false.</returns>
bool Ball::isMoving() const
{
    return velocity.length() > 0.0001f;
}

/// <summary>
/// Check if the ball is active.
/// </summary>
/// <returns>True if the Ball instance is active, otherwise false.</returns>
bool Ball::isActive() const
{
    return active;
}

/// <summary>
/// Set the ball's active state.
/// </summary>
/// <param name="active">True for active, otherwise false.</param>
void Ball::setActive(bool active)
{
	this->active = active;
}

/// <summary>
/// Get the ball's type (Cue, Solid, Stripe, Eight).
/// </summary>
/// <returns>The ball type.</returns>
BallType Ball::getType() const
{
    return type;
}

/// <summary>
/// Get the ball's number (0 for cue ball, 1-15 for object balls).
/// </summary>
/// <returns>The ball's number.</returns>
int Ball::getNumber() const
{
    return number;
}

void Ball::setFont(const sf::Font &font)
{

}

/// <summary>
/// Check if the ball is playing its pocket-sinking animation.
/// </summary>
/// <returns>True while the ball is animating into a pocket.</returns>
bool Ball::isSinking() const
{
    return sinking;
}

/// <summary>
/// Begin the pocket-sinking animation toward the given pocket centre.
/// </summary>
/// <param name="pocketCentre">World position of the pocket centre.</param>
void Ball::startSinking(sf::Vector2f pocketCentre)
{
    if (sinking)
    {
        return;
    }

    sinking = true;
    sinkProgress = 0.0f;
    sinkTarget = pocketCentre;
    velocity = { 0.0f, 0.0f };
}

/// <summary>
/// Restores the ball to a clean, playable state at the given position,
/// clearing any sinking animation and resetting scale, colour and motion.
/// </summary>
/// <param name="newPosition">Where to place the ball (e.g. the head spot).</param>
void Ball::reset(sf::Vector2f newPosition)
{
    active = true;
    sinking = false;
    sinkProgress = 0.0f;
    velocity = { 0.0f, 0.0f };
    position = newPosition;

    // Undo the sinking animation's visual changes.
    shape.setScale({ 1.0f, 1.0f });
    shape.setFillColor(baseColour);
    shape.setPosition(position);
}

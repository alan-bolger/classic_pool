#include "Ball.hpp"

#include <cmath>

/// <summary>
/// Ball constructor.
/// </summary>
/// <param name="position">Set the ball's position.</param>
Ball::Ball(sf::Vector2f position) : position(position)
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
    position += velocity * dt;
    velocity *= std::pow(friction, dt); // Velocity is multplied by friction every second

    // Stop ball if velocity is small enough
    if (velocity.length() < 0.0001f)
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
    target.draw(shape);
}

/// <summary>
/// Apply impulse.
/// </summary>
/// <param name="impulse">The imulse value.</param>
void Ball::applyImpulse(sf::Vector2f impulse)
{
    velocity += impulse;
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
    position = position;
    shape.setPosition(position);
}

/// <summary>
/// Set the ball's velocity.
/// </summary>
/// <param name="velocity">The ball's new velocity.</param>
void Ball::setVelocity(sf::Vector2f velocity)
{
    velocity = velocity;
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
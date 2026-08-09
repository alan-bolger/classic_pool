#include "Physics.hpp"

/// <summary>
/// Update ball physics.
/// </summary>
/// <param name="ball">A reference to the ball.</param>
/// <param name="table">A reference to the table.</param>
/// <param name="dt">Delta time.</param>
void Physics::updateBall(Ball &ball, const Table &table, float dt)
{
    ball.update(dt);
    resolveTableCollision(ball, table);
}

/// <summary>
/// Resolve table collision.
/// </summary>
/// <param name="ball">A reference to the ball.</param>
/// <param name="table">A reference to the table.</param>
void Physics::resolveTableCollision(Ball &ball, const Table &table)
{
    sf::Vector2f position = ball.getPosition();
    sf::Vector2f velocity = ball.getVelocity();

    const float radius = ball.getRadius();

    const sf::Vector2f min = table.getMin();
    const sf::Vector2f max = table.getMax();

    // Left cushion
    if (position.x - radius < min.x)
    {
        position.x = min.x + radius;

        if (velocity.x < 0.0f)
        {
            velocity.x = -velocity.x;
        }
    }

    // Right cushion
    if (position.x + radius > max.x)
    {
        position.x = max.x - radius;

        if (velocity.x > 0.0f)
        {
            velocity.x = -velocity.x;
        }
    }

    // Top cushion
    if (position.y - radius < min.y)
    {
        position.y = min.y + radius;

        if (velocity.y < 0.0f)
        {
            velocity.y = -velocity.y;
        }
    }

    // Bottom cushion
    if (position.y + radius > max.y)
    {
        position.y = max.y - radius;

        if (velocity.y > 0.0f)
        {
            velocity.y = -velocity.y;
        }
    }

    ball.setPosition(position);
    ball.setVelocity(velocity);
}
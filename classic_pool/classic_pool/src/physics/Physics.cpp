#include "Physics.hpp"

/// <summary>
/// Physics update.
/// </summary>
/// <param name="balls">A reference to a vector containing the balls.</param>
/// <param name="table">A reference to the table.</param>
/// <param name="dt">Delta time.</param>
void Physics::update(std::vector<Ball> &balls, const Table &table, float dt)
{
    // Update movement and cushion collisions
    for (Ball &ball : balls)
    {
        if (!ball.isActive())
        {
            continue;
        }

        updateBall(ball, table, dt);
    }

    // Check every unique pair of balls
    for (std::size_t i = 0; i < balls.size(); ++i)
    {
        if (!balls[i].isActive())
        {
            continue;
        }

        for (std::size_t j = i + 1; j < balls.size(); ++j)
        {
            if (!balls[j].isActive())
            {
                continue;
            }

            resolveBallCollision(balls[i], balls[j]);
        }
    }
}

/// <summary>
/// Update a single ball.
/// </summary>
/// <param name="ball">A reference to the ball.</param>
/// <param name="table">A reference to the table.</param>
/// <param name="dt">Delta time.</param>
void Physics::updateBall(Ball &ball, const Table &table, float dt)
{
    ball.update(dt);    
    resolvePocketCollision(ball, table);

    if (!ball.isActive())
    {
        return;
    }

    resolveTableCollision(ball, table);
}

/// <summary>
/// Resolve ball collision between two balls.
/// </summary>
/// <param name="a">A reference to ball A.</param>
/// <param name="b">A reference to ball B.</param>
void Physics::resolveBallCollision(Ball &a, Ball &b)
{
    const sf::Vector2f difference = b.getPosition() - a.getPosition();
    const float distance = difference.length();
    const float minimumDistance = a.getRadius() + b.getRadius();

    // The balls aren't touching
    if (distance >= minimumDistance)
    {
        return;
    }

    // Prevent division by zero
    if (distance < 0.000001f)
    {
        return;
    }

    const sf::Vector2f normal = difference / distance;

    // Push the balls apart so they aren't left overlapping
    const float penetration = minimumDistance - distance;
    a.setPosition(a.getPosition() - normal * (penetration * 0.5f));
    b.setPosition(b.getPosition() + normal * (penetration * 0.5f));

    // Relative velocity
    const sf::Vector2f relativeVelocity = b.getVelocity() - a.getVelocity();

    // Relative velocity along the collision normal
    const float velocityAlongNormal = relativeVelocity.dot(normal);

    // The balls are already moving apart
    if (velocityAlongNormal > 0.0f)
    {
        return;
    }

    // Equal-mass elastic collision
    const float impulseMagnitude = -velocityAlongNormal;
    const sf::Vector2f impulse = normal * impulseMagnitude;
    a.setVelocity(a.getVelocity() - impulse);
    b.setVelocity(b.getVelocity() + impulse);
}

/// <summary>
/// Resolve table collision for a single ball.
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

/// <summary>
/// Resolve pocket collision for a single ball.
/// </summary>
/// <param name="ball">A reference to the ball.</param>
/// <param name="table">A reference to the table.</param>
void Physics::resolvePocketCollision(Ball &ball, const Table &table)
{
    if (!ball.isActive())
    {
        return;
    }

    const float pocketRadius = table.getPocketRadius();
    const float ballRadius = ball.getRadius();

    for (const sf::Vector2f &pocket : table.getPockets())
    {
        const sf::Vector2f difference = ball.getPosition() - pocket;
        const float distance = difference.length();

        if (distance < pocketRadius)
        {
            ball.setActive(false);
            return;
        }
    }
}

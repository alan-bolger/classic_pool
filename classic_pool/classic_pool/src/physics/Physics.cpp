#include "Physics.hpp"

#include <algorithm>

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
    const float radius = ball.getRadius();
    sf::Vector2f position = ball.getPosition();
    sf::Vector2f velocity = ball.getVelocity();

    for (const Cushion &cushion : table.getCushions())
    {
        const sf::Vector2f segment = cushion.end - cushion.start;
        const float segmentLengthSquared = segment.dot(segment);

        if (segmentLengthSquared < 0.000001f)
        {
            continue;
        }

        const sf::Vector2f toBall = position - cushion.start;
        float t = toBall.dot(segment) / segmentLengthSquared;

        // Clamp to the actual line segment
        t = std::clamp(t, 0.0f, 1.0f);

        const sf::Vector2f closestPoint = cushion.start + segment * t;
        const sf::Vector2f difference = position - closestPoint;
        const float distance = difference.length();

        if (distance >= radius)
        {
            continue;
        }

        if (distance < 0.000001f)
        {
            continue;
        }

        const sf::Vector2f normal = difference / distance;

        // Push the ball out of the cushion
        const float penetration = radius - distance;
        position += normal * penetration;

        // Only bounce if travelling into the cushion
        const float velocityIntoCushion = velocity.dot(normal);

        if (velocityIntoCushion < 0.0f)
        {
            velocity -= 2.0f * velocityIntoCushion * normal;
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

    for (const sf::Vector2f &pocket : table.getPockets())
    {
        const sf::Vector2f difference = ball.getPosition() - pocket;
        const float distance = difference.length();
        const float pocketThreshold = table.getPocketRadius() + ball.getRadius() * 0.35f;

        if (distance < pocketThreshold)
        {
            ball.setActive(false);
            return;
        }
    }
}

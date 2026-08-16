#include "Physics.hpp"

#include <algorithm>
#include <cmath>

/// <summary>
/// Physics update.
/// </summary>
/// <param name="balls">A reference to a vector containing the balls.</param>
/// <param name="table">A reference to the table.</param>
/// <param name="dt">Delta time.</param>
void Physics::update(std::vector<Ball> &balls, const Table &table, float dt)
{
    // Find the fastest active ball
    float maximumSpeed = 0.0f;

    for (const Ball &ball : balls)
    {
        if (!ball.isActive())
        {
            continue;
        }

        maximumSpeed = std::max(
            maximumSpeed,
            ball.getVelocity().length()
        );
    }

    // Keep each sub-step small enough that a ball
    // can't travel a significant fraction of its
    // diameter without collision checks
    const float maximumStepDistance = 0.25f * balls[0].getRadius();

    int subSteps = 1;

    if (maximumSpeed > 0.0f)
    {
        subSteps = static_cast<int>(std::ceil(maximumSpeed * dt / maximumStepDistance));
    }

    // Put a sensible upper limit on the amount of
    // work we do during one physics tick
    subSteps = std::clamp(subSteps, 1, 32);
    const float subDt = dt / static_cast<float>(subSteps);

    for (int step = 0; step < subSteps; ++step)
    {
        // Move balls and handle cushions/pockets
        for (Ball &ball : balls)
        {
            if (!ball.isActive())
            {
                continue;
            }

            updateBall(ball, table, subDt);
        }

        // Resolve ball-to-ball collisions
        for (std::size_t i = 0; i < balls.size(); ++i)
        {
            if (!balls[i].isActive() || balls[i].isSinking())
            {
                continue;
            }

            for (std::size_t j = i + 1; j < balls.size(); ++j)
            {
                if (!balls[j].isActive() || balls[j].isSinking())
                {
                    continue;
                }

                resolveBallCollision(balls[i], balls[j]);
            }
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

    if (!ball.isActive() || ball.isSinking())
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

    for (const std::vector<sf::Vector2f> &outline : table.getCushionOutlines())
    {
        const std::size_t pointCount = outline.size();

        if (pointCount < 2)
        {
            continue;
        }

        // Find the closest point on the whole cushion outline (all edges,
        // including the curved end segments).
        float bestDistanceSquared = std::numeric_limits<float>::max();
        sf::Vector2f bestPoint;
        bool found = false;

        for (std::size_t i = 0; i < pointCount; ++i)
        {
            const sf::Vector2f a = outline[i];
            const sf::Vector2f b = outline[(i + 1) % pointCount];

            const sf::Vector2f segment = b - a;
            const float segmentLengthSquared = segment.dot(segment);

            if (segmentLengthSquared < 0.000001f)
            {
                continue;
            }

            const sf::Vector2f toBall = position - a;
            float t = toBall.dot(segment) / segmentLengthSquared;
            t = std::clamp(t, 0.0f, 1.0f);

            const sf::Vector2f point = a + segment * t;
            const sf::Vector2f difference = position - point;
            const float distanceSquared = difference.dot(difference);

            if (distanceSquared < bestDistanceSquared)
            {
                bestDistanceSquared = distanceSquared;
                bestPoint = point;
                found = true;
            }
        }

        if (!found)
        {
            continue;
        }

        const float distance = std::sqrt(bestDistanceSquared);

        if (distance >= radius || distance < 0.000001f)
        {
            continue;
        }

        const sf::Vector2f normal = (position - bestPoint) / distance;

        // Push the ball out of the cushion.
        const float penetration = radius - distance;
        position += normal * penetration;

        // Only bounce if travelling into the cushion.
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
    if (!ball.isActive() || ball.isSinking())
    {
        return;
    }

    const std::array<sf::Vector2f, 6> &pockets = table.getPockets();

    for (const sf::Vector2f &pocket : pockets)
    {
        const sf::Vector2f difference = ball.getPosition() - pocket;
        const float distance = difference.length();

        // Only capture once the ball's CENTRE crosses the pocket circle
        if (distance < table.getPocketRadius())
        {
            ball.startSinking(pocket);
            return;
        }
    }
}

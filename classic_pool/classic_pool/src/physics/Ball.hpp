#ifndef BALL_HPP
#define BALL_HPP

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

class Ball
{
    public:

    explicit Ball(sf::Vector2f position);
    void update(float dt);
    void render(sf::RenderTarget &target) const;
    void applyImpulse(sf::Vector2f impulse);
    sf::Vector2f getPosition() const;
    sf::Vector2f getVelocity() const;
    void setPosition(sf::Vector2f position);
    void setVelocity(sf::Vector2f velocity);
	void setColour(sf::Color colour);
    float getRadius() const;
    bool isMoving() const;

    private:

    sf::Vector2f position;
    sf::Vector2f velocity;
    float radius = 0.0285f;
    float friction = 0.35f;
    sf::CircleShape shape;
};

#endif // !BALL_HPP
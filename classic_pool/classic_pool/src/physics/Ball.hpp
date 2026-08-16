#ifndef BALL_HPP
#define BALL_HPP

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Font.hpp>

enum class BallType
{
    Cue,
    Solid,
    Stripe,
    Eight
};

class Ball
{
    public:

    explicit Ball(sf::Vector2f position, BallType type = BallType::Solid, int number = 0);
    void update(float dt);
    void render(sf::RenderTarget &target) const;
    sf::Vector2f getPosition() const;
    sf::Vector2f getVelocity() const;
    void setPosition(sf::Vector2f position);
    void setVelocity(sf::Vector2f velocity);
	void setColour(sf::Color colour);
    float getRadius() const;
    bool isMoving() const;
	bool isActive() const;
	void setActive(bool active);
    BallType getType() const;
    int getNumber() const;
    void setFont(const sf::Font &font);
    bool isSinking() const;
    void startSinking(sf::Vector2f pocketCentre);
    void reset(sf::Vector2f newPosition);

    private:

    sf::Vector2f position;
    sf::Vector2f velocity;
    float radius = 0.045f;   // ~15% larger than regulation for better top-down readability
    float friction = 0.35f;
	bool active = true;
    bool sinking = false;
    float sinkProgress = 0.0f;
    float sinkDuration = 0.30f;
    sf::Vector2f sinkTarget;
    sf::Color baseColour = sf::Color::White;
    sf::CircleShape shape;
    BallType type;
    int number;
    const sf::Font *font = nullptr;
};

#endif // !BALL_HPP
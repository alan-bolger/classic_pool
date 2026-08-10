#ifndef TABLE_HPP
#define TABLE_HPP

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

struct Cushion
{
    sf::Vector2f start;
	sf::Vector2f end;
};

class Table
{
    public:

    Table();
    void render(sf::RenderTarget &target) const;
    sf::Vector2f getMin() const;
    sf::Vector2f getMax() const;
    const std::array<sf::Vector2f, 6> &getPockets() const;
    float getPocketRadius() const;
    const std::array<Cushion, 6> &getCushions() const;

    private:

    sf::Vector2f min;
    sf::Vector2f max;
    std::array<sf::Vector2f, 6> pockets;
	std::array<Cushion, 6> cushions;
    float pocketRadius = 0.075f;
    sf::RectangleShape tableShape;
};

#endif // !TABLE_HPP
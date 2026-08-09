#ifndef TABLE_HPP
#define TABLE_HPP

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

class Table
{
    public:

    Table();
    void render(sf::RenderTarget &target) const;
    sf::Vector2f getMin() const;
    sf::Vector2f getMax() const;

    private:

    sf::Vector2f min;
    sf::Vector2f max;
    sf::RectangleShape tableShape;
};

#endif // !TABLE_HPP
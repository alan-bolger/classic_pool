#ifndef TABLE_HPP
#define TABLE_HPP

#include <vector>
#include <array>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/ConvexShape.hpp>

struct Cushion
{
	sf::Vector2f start;
	sf::Vector2f end;
    sf::Vector2f topLeft;
    sf::Vector2f topRight;
    sf::Vector2f bottomRight;
    sf::Vector2f bottomLeft;
};

struct PocketJaw
{
    sf::Vector2f start;
    sf::Vector2f end;
};

struct PocketGeometry
{
    sf::Vector2f mouthLeft;
    sf::Vector2f mouthRight;
    sf::Vector2f throatLeft;
    sf::Vector2f throatRight;
};

struct RailGeometry
{
    sf::Vector2f noseStart;
    sf::Vector2f noseEnd;
    sf::Vector2f jawStart;
    sf::Vector2f jawEnd;
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
    const std::array<std::vector<sf::Vector2f>, 6> &getCushionOutlines() const;
    const std::array<PocketJaw, 12> &getPocketJaws() const;

    private:

    sf::Vector2f min;
    sf::Vector2f max;
    float pocketOpening = 0.18f;
    float jawDepth = 0.065f;
    float cushionWidth = 0.045f;
    float railWidth = 0.08f;
    std::array<sf::Vector2f, 6> pockets;
	std::array<Cushion, 6> cushions;
    std::array<std::vector<sf::Vector2f>, 6> cushionOutlines;
    float pocketRadius = 0.075f;
    sf::RectangleShape tableShape;
    std::array<PocketJaw, 12> pocketJaws;
    std::array<float, 6> pocketRadii;
    std::array<PocketGeometry, 6> pocketGeometry;

    static std::vector<sf::Vector2f> buildCushionOutline(const Cushion &cushion, float cornerRadius, int arcSegments);
};

#endif // !TABLE_HPP
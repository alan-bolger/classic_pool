#include "Table.hpp"

/// <summary>
/// Table constructor.
/// </summary>
Table::Table()
{
    // Arbitrary world units
    min = { 0.0f, 0.0f };
    max = { 2.84f, 1.42f };

    tableShape.setPosition(min);
    tableShape.setSize(max - min);
    tableShape.setFillColor(sf::Color(30, 100, 50));

	// Set pocket positions
    pockets[0] = { min.x, min.y }; // Top-left    
    pockets[1] = { (min.x + max.x) * 0.5f, min.y }; // Top-middle    
    pockets[2] = { max.x, min.y }; // Top-right    
    pockets[3] = { min.x, max.y }; // Bottom-left    
    pockets[4] = { (min.x + max.x) * 0.5f, max.y }; // Bottom-middle    
    pockets[5] = { max.x, max.y }; // Bottom-right

	// Set cushion positions
    const float pocketGap = pocketRadius * 1.5f;

	cushions[0] = { { min.x + pocketGap, min.y }, { (min.x + max.x) * 0.5f - pocketGap, min.y } }; // Top-left
	cushions[1] = { { (min.x + max.x) * 0.5f + pocketGap, min.y }, { max.x - pocketGap, min.y } }; // Top-right
	cushions[2] = { { min.x + pocketGap, max.y }, { (min.x + max.x) * 0.5f - pocketGap, max.y } }; // Bottom-left
	cushions[3] = { { (min.x + max.x) * 0.5f + pocketGap, max.y }, { max.x - pocketGap, max.y } }; // Bottom-right
	cushions[4] = { { min.x, min.y + pocketGap }, { min.x, max.y - pocketGap } }; // Left
	cushions[5] = { { max.x, min.y + pocketGap }, { max.x, max.y - pocketGap } }; // Right
}

/// <summary>
/// Render.
/// </summary>
/// <param name="target">The render target.</param>
void Table::render(sf::RenderTarget &target) const
{
    target.draw(tableShape);

    sf::CircleShape pocketShape;
    pocketShape.setRadius(pocketRadius);
    pocketShape.setOrigin({ pocketRadius, pocketRadius });
    pocketShape.setFillColor(sf::Color::Black);

    for (const sf::Vector2f &position : pockets)
    {
        pocketShape.setPosition(position);
        target.draw(pocketShape);
    }
}

/// <summary>
/// Get the table's minimum value.
/// </summary>
/// <returns>The table's minimum value as a sf::Vector2f.</returns>
sf::Vector2f Table::getMin() const
{
    return min;
}

/// <summary>
/// Get the table's maximum value.
/// </summary>
/// <returns>The table's maximum value as a sf::Vector2f.</returns>
sf::Vector2f Table::getMax() const
{
    return max;
}

/// <summary>
/// Returns the positions of the table pockets.
/// </summary>
/// <returns>A const reference to an std::array of containing the pockets.</returns>
const std::array<sf::Vector2f, 6> &Table::getPockets() const
{
    return pockets;
}

/// <summary>
/// Get the radius of the table pockets.
/// </summary>
/// <returns>The radius of the pockets.</returns>
float Table::getPocketRadius() const
{
    return pocketRadius;
}

/// <summary>
/// Returns a const reference to the table's array of six cushions.
/// </summary>
/// <returns>A const reference to the std::array<Cushion, 6> containing the Table's cushions.</returns>
const std::array<Cushion, 6> &Table::getCushions() const
{
	return cushions;
}

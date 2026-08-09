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
}

/// <summary>
/// Render.
/// </summary>
/// <param name="target">The render target.</param>
void Table::render(sf::RenderTarget &target) const
{
    target.draw(tableShape);
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
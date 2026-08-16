#include "Table.hpp"

#include <cmath>
#include <numbers>
#include <iostream>

/// <summary>
/// Constructs and initializes a Table object. It sets the table bounds and visual shape, computes pocket centers and geometry, configures cushion shapes and positions, and initializes pocket jaw lines used for collision detection.
/// </summary>
Table::Table()
{
    min = { 0.0f, 0.0f };
    max = { 2.84f, 1.42f };

    tableShape.setPosition(min);
    tableShape.setSize(max - min);
    tableShape.setFillColor(sf::Color(30, 100, 50));

    const float centreX = (min.x + max.x) * 0.5f;
    const float unitsPerInch = 2.84f / 100.0f;

    const float cornerMouth = 5.0f * unitsPerInch;
    const float sideMouth = 4.5f * unitsPerInch;
    const float cushionWidth = 1.9375f * unitsPerInch;

    const float cornerShelf = 1.75f * unitsPerInch;
    const float sideShelf = 0.25f * unitsPerInch;

    // Angled facing setback (how far the nose edge is pulled back from the mouth at each cushion end). Corners cut back ~45 degrees; sides shallower
    const float cornerFacing = cushionWidth * 1.35f;
    const float sideFacing = cushionWidth * 0.7f;

    const float cornerTheta = (cornerFacing - 135.0f) * 3.14159265f / 180.0f;
    const float sideTheta = (sideFacing - 135.0f) * 3.14159265f / 180.0f;

    const float cornerMouthOffset = cornerMouth / std::sqrt(2.0f);
    const float sideMouthHalf = sideMouth * 0.5f;

    const float cornerThroat = cornerMouth - 2.0f * cornerShelf * std::tan(cornerTheta);
    const float sideThroat = sideMouth - 2.0f * sideShelf * std::tan(std::abs(sideTheta));

    const float cornerThroatOffset = cornerThroat / std::sqrt(2.0f);
    const float sideThroatHalf = sideThroat * 0.5f;

    // Pocket centers
    const float sideRadius = sideMouth * 0.5f;

    // Side pockets use the SAME radius as the corner pockets so the arcs
    // match. The centre is then placed one radius back from the cushion
    // nose line, keeping the inner arc flush with the adjacent cushions
    const float cornerPocketRadius = cornerMouth / std::sqrt(2.0f);
    const float sidePocketRadius = cornerPocketRadius;

    pockets[0] = { min.x, min.y };
    pockets[1] = { centreX, min.y + cushionWidth - sidePocketRadius };
    pockets[2] = { max.x, min.y };
    pockets[3] = { min.x, max.y };
    pockets[4] = { centreX, max.y - cushionWidth + sidePocketRadius };
    pockets[5] = { max.x, max.y };

    // Slide the corner pockets inward along their 45-degree diagonals so the
    // inner arc sits flush with (or just behind) the adjacent cushion tips
    const float cornerPush = cushionWidth * 1.15f;
    const float cornerPushDiag = cornerPush / std::sqrt(2.0f);

    pockets[0] += {  cornerPushDiag,  cornerPushDiag };
    pockets[2] += { -cornerPushDiag,  cornerPushDiag };
    pockets[3] += {  cornerPushDiag, -cornerPushDiag };
    pockets[5] += { -cornerPushDiag, -cornerPushDiag };

    // Corner pocket geometry

    // Top-left
    pocketGeometry[0].mouthLeft = { cornerMouthOffset, min.y };
    pocketGeometry[0].mouthRight = { min.x, cornerMouthOffset };
    pocketGeometry[0].throatLeft = { cornerThroatOffset, cornerShelf };
    pocketGeometry[0].throatRight = { cornerShelf, cornerThroatOffset };

    // Top-right
    pocketGeometry[2].mouthLeft = { max.x - cornerMouthOffset, min.y };
    pocketGeometry[2].mouthRight = { max.x, cornerMouthOffset };
    pocketGeometry[2].throatLeft = { max.x - cornerThroatOffset, cornerShelf };
    pocketGeometry[2].throatRight = { max.x - cornerShelf, cornerThroatOffset };

    // Bottom-left
    pocketGeometry[3].mouthLeft = { min.x, max.y - cornerMouthOffset };
    pocketGeometry[3].mouthRight = { cornerMouthOffset, max.y };
    pocketGeometry[3].throatLeft = { cornerShelf, max.y - cornerThroatOffset };
    pocketGeometry[3].throatRight = { cornerThroatOffset, max.y - cornerShelf };

    // Bottom-right
    pocketGeometry[5].mouthLeft = { max.x, max.y - cornerMouthOffset };
    pocketGeometry[5].mouthRight = { max.x - cornerMouthOffset, max.y };
    pocketGeometry[5].throatLeft = { max.x - cornerShelf, max.y - cornerThroatOffset };
    pocketGeometry[5].throatRight = { max.x - cornerThroatOffset, max.y - cornerShelf };

	// Side pocket geometry

    // Top-middle
    pocketGeometry[1].mouthLeft = { centreX - sideMouthHalf, min.y };
    pocketGeometry[1].mouthRight = { centreX + sideMouthHalf, min.y };
    pocketGeometry[1].throatLeft = { centreX - sideThroatHalf, sideShelf };
    pocketGeometry[1].throatRight = { centreX + sideThroatHalf, sideShelf };

    // Bottom-middle
    pocketGeometry[4].mouthLeft = { centreX - sideMouthHalf, max.y };
    pocketGeometry[4].mouthRight = { centreX + sideMouthHalf, max.y };
    pocketGeometry[4].throatLeft = { centreX - sideThroatHalf, max.y - sideShelf };
    pocketGeometry[4].throatRight = { centreX + sideThroatHalf, max.y - sideShelf };

    // Cushion bodies:
    // Each cushion is a trapezoid - the rail edge spans the full mouth
    // width, while the nose edge (start/end - the line the ball strikes)
    // is pulled back at each end to form the angled pocket facings

    // Top-left (top rail, corner[0] -> side[1])
    cushions[0].topLeft = pocketGeometry[0].mouthLeft;
    cushions[0].topRight = pocketGeometry[1].mouthLeft;
    cushions[0].bottomLeft = { pocketGeometry[0].mouthLeft.x + cornerFacing, min.y + cushionWidth };
    cushions[0].bottomRight = { pocketGeometry[1].mouthLeft.x - sideFacing, min.y + cushionWidth };
    cushions[0].start = cushions[0].bottomLeft;
    cushions[0].end = cushions[0].bottomRight;

    // Top-right (top rail, side[1] -> corner[2])
    cushions[1].topLeft = pocketGeometry[1].mouthRight;
    cushions[1].topRight = pocketGeometry[2].mouthLeft;
    cushions[1].bottomLeft = { pocketGeometry[1].mouthRight.x + sideFacing, min.y + cushionWidth };
    cushions[1].bottomRight = { pocketGeometry[2].mouthLeft.x - cornerFacing, min.y + cushionWidth };
    cushions[1].start = cushions[1].bottomLeft;
    cushions[1].end = cushions[1].bottomRight;

    // Bottom-left (bottom rail, corner[3] -> side[4])
    cushions[2].topLeft = { pocketGeometry[3].mouthRight.x + cornerFacing, max.y - cushionWidth };
    cushions[2].topRight = { pocketGeometry[4].mouthLeft.x - sideFacing, max.y - cushionWidth };
    cushions[2].bottomLeft = pocketGeometry[3].mouthRight;
    cushions[2].bottomRight = pocketGeometry[4].mouthLeft;
    cushions[2].start = cushions[2].topLeft;
    cushions[2].end = cushions[2].topRight;

    // Bottom-right (bottom rail, side[4] -> corner[5])
    cushions[3].topLeft = { pocketGeometry[4].mouthRight.x + sideFacing, max.y - cushionWidth };
    cushions[3].topRight = { pocketGeometry[5].mouthRight.x - cornerFacing, max.y - cushionWidth };
    cushions[3].bottomLeft = pocketGeometry[4].mouthRight;
    cushions[3].bottomRight = pocketGeometry[5].mouthRight;
    cushions[3].start = cushions[3].topLeft;
    cushions[3].end = cushions[3].topRight;

    // Left (left rail, corner[0] -> corner[3])
    cushions[4].topLeft = pocketGeometry[0].mouthRight;
    cushions[4].bottomLeft = pocketGeometry[3].mouthLeft;
    cushions[4].topRight = { min.x + cushionWidth, pocketGeometry[0].mouthRight.y + cornerFacing };
    cushions[4].bottomRight = { min.x + cushionWidth, pocketGeometry[3].mouthLeft.y - cornerFacing };
    cushions[4].start = cushions[4].topRight;
    cushions[4].end = cushions[4].bottomRight;

    // Right (right rail, corner[2] -> corner[5])
    cushions[5].topRight = pocketGeometry[2].mouthRight;
    cushions[5].bottomRight = pocketGeometry[5].mouthLeft;
    cushions[5].topLeft = { max.x - cushionWidth, pocketGeometry[2].mouthRight.y + cornerFacing };
    cushions[5].bottomLeft = { max.x - cushionWidth, pocketGeometry[5].mouthLeft.y - cornerFacing };
    cushions[5].start = cushions[5].topLeft;
    cushions[5].end = cushions[5].bottomLeft;

	// Pocket jaws (the lines that define the pocket throat for collision detection)
    pocketJaws[0].start = pocketGeometry[0].mouthLeft;
    pocketJaws[0].end = pocketGeometry[0].throatLeft;

    pocketJaws[1].start = pocketGeometry[0].mouthRight;
    pocketJaws[1].end = pocketGeometry[0].throatRight;

    pocketJaws[2].start = pocketGeometry[1].mouthLeft;
    pocketJaws[2].end = pocketGeometry[1].throatLeft;

    pocketJaws[3].start = pocketGeometry[1].mouthRight;
    pocketJaws[3].end = pocketGeometry[1].throatRight;

    pocketJaws[4].start = pocketGeometry[2].mouthLeft;
    pocketJaws[4].end = pocketGeometry[2].throatLeft;

    pocketJaws[5].start = pocketGeometry[2].mouthRight;
    pocketJaws[5].end = pocketGeometry[2].throatRight;

    pocketJaws[6].start = pocketGeometry[3].mouthLeft;
    pocketJaws[6].end = pocketGeometry[3].throatLeft;

    pocketJaws[7].start = pocketGeometry[3].mouthRight;
    pocketJaws[7].end = pocketGeometry[3].throatRight;

    pocketJaws[8].start = pocketGeometry[4].mouthLeft;
    pocketJaws[8].end = pocketGeometry[4].throatLeft;

    pocketJaws[9].start = pocketGeometry[4].mouthRight;
    pocketJaws[9].end = pocketGeometry[4].throatRight;

    pocketJaws[10].start = pocketGeometry[5].mouthLeft;
    pocketJaws[10].end = pocketGeometry[5].throatLeft;

    pocketJaws[11].start = pocketGeometry[5].mouthRight;
    pocketJaws[11].end = pocketGeometry[5].throatRight;

	// Cushion collision outlines (the rounded trapezoid shape the ball collides with)
    // Precompute the exact rendered outline for each cushion so physics
    // collides against the same rounded polygon that is drawn
    const float outlineCornerRadius = cushionWidth * 0.45f;
    const int outlineArcSegments = 6;

    for (std::size_t i = 0; i < cushions.size(); ++i)
    {
        cushionOutlines[i] = buildCushionOutline(cushions[i], outlineCornerRadius, outlineArcSegments);
    }
}

/// <summary>
/// Renders the table to the given SFML render target, drawing the outer rail, playing surface, cushion bodies, and pockets. The method is const and issues draw calls to the provided target.
/// </summary>
/// <param name="target">SFML render target (sf::RenderTarget) onto which the table is drawn. The function issues draw calls to this target.</param>
void Table::render(sf::RenderTarget &target) const
{
    // Outer rail
    sf::RectangleShape outerRail;
    outerRail.setPosition({ min.x - 0.08f, min.y - 0.08f });
    outerRail.setSize({ (max.x - min.x) + 0.16f, (max.y - min.y) + 0.16f });
    outerRail.setFillColor(sf::Color(75, 45, 25));
    target.draw(outerRail);

    // Playing surface
    target.draw(tableShape);

    // Pockets
    for (const PocketGeometry &pocket : pocketGeometry)
    {
        sf::VertexArray shape(sf::PrimitiveType::TriangleStrip, 4);

        shape[0].position = pocket.mouthLeft;
        shape[1].position = pocket.mouthRight;
        shape[2].position = pocket.throatLeft;
        shape[3].position = pocket.throatRight;

        shape[0].color = sf::Color::Red;
        shape[1].color = sf::Color::Red;
        shape[2].color = sf::Color::Red;
        shape[3].color = sf::Color::Red;

        target.draw(shape);
    }

    // Cushion bodies (rounded ends) - drawn from the shared collision outline
    for (const std::vector<sf::Vector2f> &outline : cushionOutlines)
    {
        sf::ConvexShape shape;
        shape.setPointCount(outline.size());

        for (std::size_t i = 0; i < outline.size(); ++i)
        {
            shape.setPoint(i, outline[i]);
        }

        shape.setFillColor(sf::Color(25, 80, 40));

        target.draw(shape);
    }

    // Pockets
    const float renderUnitsPerInch = 2.84f / 100.0f;
    const float cornerRadius = (5.0f * renderUnitsPerInch) / std::sqrt(2.0f);

    const float sideRadius = cornerRadius;

    sf::CircleShape cornerPocket;
    cornerPocket.setRadius(cornerRadius);
    cornerPocket.setOrigin({ cornerRadius, cornerRadius });
    cornerPocket.setFillColor(sf::Color::Black);

    sf::CircleShape sidePocket;
    sidePocket.setRadius(sideRadius);
    sidePocket.setOrigin({ sideRadius, sideRadius });
    sidePocket.setFillColor(sf::Color::Black);

    for (int i = 0; i < 6; ++i)
    {
        if (i == 1 || i == 4)
        {
            sidePocket.setPosition(pockets[i]);
            //target.draw(sidePocket);
        }
        else
        {
            cornerPocket.setPosition(pockets[i]);
            //target.draw(cornerPocket);
        }
    }
}

/// <summary>
/// Returns the minimum two-dimensional vector associated with this Table.
/// </summary>
/// <returns>A copy of the stored sf::Vector2f representing the minimum (x, y) value for the Table.</returns>
sf::Vector2f Table::getMin() const
{
    return min;
}

/// <summary>
/// Returns the stored maximum vector for the table.
/// </summary>
/// <returns>The stored maximum as an sf::Vector2f. The method is const and does not modify the object.</returns>
sf::Vector2f Table::getMax() const
{
    return max;
}

/// <summary>
/// Returns a const reference to the table's six pocket positions.
/// </summary>
/// <returns>A const reference to a std::array of six sf::Vector2f elements representing the pocket positions; the function is const-qualified and does not modify the Table.</returns>
const std::array<sf::Vector2f, 6> &Table::getPockets() const
{
    return pockets;
}

/// <summary>
/// Returns the table's pocket radius.
/// </summary>
/// <returns>The pocket radius of the table as a float.</returns>
float Table::getPocketRadius() const
{
    return pocketRadius;
}

/// <summary>
/// Returns a const reference to the table's cushions.
/// </summary>
/// <returns>A const reference to the internal std::array of six Cushion objects belonging to the Table (read-only).</returns>
const std::array<Cushion, 6> &Table::getCushions() const
{
    return cushions;
}

/// <summary>
/// Returns the outlines for the table cushions as a read-only reference.
/// </summary>
/// <returns>A const reference to an std::array of six std::vector<sf::Vector2f>, where each vector contains the sequence of 2D points (sf::Vector2f) defining a cushion's outline. The reference refers to the object's internal storage and should not be modified.</returns>
const std::array<std::vector<sf::Vector2f>, 6> &Table::getCushionOutlines() const
{
    return cushionOutlines;
}

/// <summary>
/// Returns a const reference to the array of twelve PocketJaw objects owned by the Table.
/// </summary>
/// <returns>A const reference to the internal std::array<PocketJaw, 12> that holds the table's pocket jaws. The const qualification prevents modification through the returned reference.</returns>
const std::array<PocketJaw, 12> &Table::getPocketJaws() const
{
    return pocketJaws;
}

/// <summary>
/// Builds a polyline approximating the cushion outline with rounded corners using quadratic arcs.
/// </summary>
/// <param name="cushion">A Cushion describing the corner positions (expects members topLeft, topRight, bottomRight, bottomLeft). Those points are used as corner control points for the arcs.</param>
/// <param name="cornerRadius">Desired radius for the rounded corners. For each corner this value is clamped to at most half the length of the adjacent edge to avoid overlap.</param>
/// <param name="arcSegments">Number of segments used to approximate each corner arc. Each corner produces (arcSegments + 1) points. Should be a positive value (>= 1).</param>
/// <returns>A std::vector<sf::Vector2f> containing the outline points in the same corner order (topLeft, topRight, bottomRight, bottomLeft). The vector typically contains corners.size() * (arcSegments + 1) points and represents the cushion outline in the same coordinate space as the input points.</returns>
std::vector<sf::Vector2f> Table::buildCushionOutline(const Cushion &cushion, float cornerRadius, int arcSegments)
{
    const std::array<sf::Vector2f, 4> corners = { cushion.topLeft, cushion.topRight, cushion.bottomRight, cushion.bottomLeft };

    std::vector<sf::Vector2f> outline;
    outline.reserve(corners.size() * (arcSegments + 1));

    for (std::size_t i = 0; i < corners.size(); ++i)
    {
        const sf::Vector2f prev = corners[(i + corners.size() - 1) % corners.size()];
        const sf::Vector2f curr = corners[i];
        const sf::Vector2f next = corners[(i + 1) % corners.size()];

        auto shorten = [&](const sf::Vector2f &from) -> sf::Vector2f
        {
            sf::Vector2f dir = from - curr;
            const float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            const float r = std::min(cornerRadius, len * 0.5f);

            if (len > 0.0f)
            {
                dir = { dir.x / len, dir.y / len };
            }

            return { curr.x + dir.x * r, curr.y + dir.y * r };
        };

        const sf::Vector2f entry = shorten(prev);
        const sf::Vector2f exit = shorten(next);

        // Quadratic arc from entry -> exit, bulging toward the corner
        for (int s = 0; s <= arcSegments; ++s)
        {
            const float t = static_cast<float>(s) / static_cast<float>(arcSegments);
            const float u = 1.0f - t;
            outline.push_back({ u * u * entry.x + 2.0f * u * t * curr.x + t * t * exit.x, u * u * entry.y + 2.0f * u * t * curr.y + t * t * exit.y });
        }
    }

    return outline;
}
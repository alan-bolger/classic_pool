#pragma once

#include <vector>

#include "Ball.hpp"
#include "game/Table.hpp"

class Physics
{
    public:

    static void update(std::vector<Ball> &balls, const Table &table, float dt);

    private:

    static void updateBall(Ball &ball, const Table &table, float dt);
    static void resolveBallCollision(Ball &a, Ball &b);
    static void resolveTableCollision(Ball &ball, const Table &table);
};
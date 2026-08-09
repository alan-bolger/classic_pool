#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include "Ball.hpp"
#include "game/Table.hpp"

class Physics
{
    public:

    static void updateBall(Ball &ball, const Table &table, float dt);

    private:

    static void resolveTableCollision(Ball &ball, const Table &table);
};

#endif // !PHYSICS_HPP
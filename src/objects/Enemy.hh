#ifndef __ENEMY_HH__
#define __ENEMY_HH__

#include "GameObject.hh"
#include <vector>

class Enemy : public GameObject
{
public:
    enum AIState {
        AI_PATROL = 0,
        AI_CHASE = 1,
        AI_ATTACK = 2,
        AI_DEAD = 3
    };

    constexpr static float VIEW_RADIUS = 90;
    constexpr static float VIEW_ANGLE = 90;
    constexpr static float ATTACK_RADIUS = 70;
    constexpr static float ATTACK_CHARGE_TIME = 30;

    Enemy(int id);

    Enemy(int id, Enemy* ancestor)
        : GameObject(id, ancestor)
        , moveSpeed(ancestor->moveSpeed)
        , patrolPoints(ancestor->patrolPoints)
    {
    }

    ObjectType type() override
    {
        return ENEMY;
    }

    float moveSpeed;

    std::vector<point_t> patrolPoints;
};

#endif
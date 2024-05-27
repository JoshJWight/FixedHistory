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

    Enemy(int id);

    float moveSpeed;

    std::vector<point_t> patrolPoints;
};

#endif
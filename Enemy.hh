#ifndef __ENEMY_HH__
#define __ENEMY_HH__

#include "GameObject.hh"
#include <vector>

class Enemy : public GameObject
{
public:
    Enemy(int id);

    float moveSpeed;

    int deathTimeline;

    std::vector<point_t> patrolPoints;
};

#endif
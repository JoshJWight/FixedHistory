#ifndef __BULLET_HH__
#define __BULLET_HH__

#include "GameObject.hh"
#include "MathUtil.hh"

class Bullet : public GameObject
{
public:
    //constexpr static int LIFETIME = 300;
    constexpr static float SPEED = 10;

    Bullet(int id);

    ObjectType type() override
    {
        return BULLET;
    }

    point_t velocity;

    int creatorId;
};

#endif
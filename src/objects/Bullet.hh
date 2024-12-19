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

    Bullet(int id, Bullet* ancestor)
        : GameObject(id, ancestor)
        , velocity(ancestor->velocity)
        , creatorId(ancestor->creatorId)
    {
    }

    bool isTransient() override
    {
        return true;
    }

    ObjectType type() override
    {
        return BULLET;
    }

    point_t velocity;

    int creatorId;
};

#endif
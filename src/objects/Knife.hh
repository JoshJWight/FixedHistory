#ifndef __KNIFE_HH__
#define __KNIFE_HH__

#include "Throwable.hh"

class Knife : public Throwable
{
public:
    Knife(int id)
        : Throwable(id)
    {
        colliderType = CIRCLE;
        size = point_t(10, 10);
        setupSprites({"knife.png"});

        deadly = true;
        drag = 0.02f;
        throwSpeed = 6.0f;
        bounciness = 0.70f;
    }

    Knife(int id, Knife* ancestor)
        : Throwable(id, ancestor)
    {
    }

    ObjectType type() override
    {
        return KNIFE;
    }
};

#endif
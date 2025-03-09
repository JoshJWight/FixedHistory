#ifndef __GUN_HH__
#define __GUN_HH__

#include "Throwable.hh"

class Gun : public Throwable
{
public:
    Gun(int id)
        : Throwable(id)
    {
        colliderType = CIRCLE;
        size = point_t(10, 10);
        setupSprites({"gun.png"});

        deadly = false;
        drag = 0.05f;
        throwSpeed = 5.0f;
        bounciness = 0.70f;

        //Short use duration, basically just fire the bullet
        useDuration = 10;
    }

    Gun(int id, Gun* ancestor)
        : Throwable(id, ancestor)
    {
    }

    ObjectType type() override
    {
        return GUN;
    }
};

#endif
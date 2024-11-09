#ifndef __THROWABLE_HH__
#define __THROWABLE_HH__

#include "GameObject.hh"

class Throwable : public GameObject
{
public:
    enum ThrowableState
    {
        STILL = 0,
        HELD = 1,
        THROWN = 2
    };

    Throwable(int id)
        : GameObject(id)
        , throwSpeed(5.0f)
        , drag(0.03f)
        , bounciness(0.80f)
        , deadly(false)
    {
    }

    Throwable(int id, Throwable* ancestor)
        : GameObject(id, ancestor)
        , throwSpeed(ancestor->throwSpeed)
        , drag(ancestor->drag)
        , bounciness(ancestor->bounciness)
        , deadly(ancestor->deadly)
    {
    }

    //Initial speed when thrown
    float throwSpeed;
    //Speed lost per tick
    float drag;
    //Fraction of speed retained on bouncing off a wall or obstruction
    float bounciness;

    bool deadly;
};

#endif
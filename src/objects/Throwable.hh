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
        , drag(0.1f)
        , bounciness(0.5f)
        , deadly(false)
    {
    }

    //Initial speed when thrown
    const float throwSpeed;
    //Speed lost per tick
    const float drag;
    //Fraction of speed retained on bouncing off a wall or obstruction
    const float bounciness;

    const bool deadly;
};

#endif
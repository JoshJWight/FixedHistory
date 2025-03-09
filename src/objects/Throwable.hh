#ifndef __THROWABLE_HH__
#define __THROWABLE_HH__

#include "GameObject.hh"

class Throwable : public GameObject
{
public:
    enum ThrowableState
    {
        STILL = 0, //Lying on ground, not moving
        HELD = 1, //Held by a player
        THROWN = 2, //Flying through the air
        USED = 3 //Held by a player and being used (knife slashing, gun shooting)
    };

    Throwable(int id)
        : GameObject(id)
        , throwSpeed(5.0f)
        , drag(0.03f)
        , bounciness(0.80f)
        , deadly(false)
        , useDuration(0)
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

    //Does this kill enemies when thrown?
    bool deadly;

    //Length of animation when used
    int useDuration;
};

#endif
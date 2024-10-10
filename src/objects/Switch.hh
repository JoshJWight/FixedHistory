#ifndef __SWITCH_HH__
#define __SWITCH_HH__

#include "GameObject.hh"


class Switch : public GameObject
{
public:
    enum SwitchState {
        ON = 0,
        OFF = 1
    };

    Switch(int id)
        : GameObject(id)
    {
        colliderType = BOX;
        size = point_t(8, 8);
        setupSprites({"switchon.png", "switchoff.png"});
    }

    ObjectType type() override
    {
        return SWITCH;
    }
};

#endif
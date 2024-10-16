#ifndef __TURNSTILE_HH__
#define __TURNSTILE_HH__

#include "GameObject.hh"

class Turnstile: public GameObject
{

public:
    constexpr static int OCCUPANCY_SPACING = 60;

    Turnstile(int id)
        : GameObject(id)
        , activeOccupant(nullptr)
    {
        colliderType = BOX;
        size = point_t(25, 25);
        setupSprites({"turnstile.png"});
    }

    ObjectType type() override
    {
        return TURNSTILE;
    }

    GameObject* activeOccupant;
};

#endif
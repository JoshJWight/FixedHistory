#ifndef __CLOSET_HH__
#define __CLOSET_HH__

#include "GameObject.hh"

class Closet : public GameObject
{
public:
    constexpr static int OCCUPANCY_SPACING = 60;
    Closet(int id)
        : GameObject(id)
        , activeOccupant(nullptr)
    {
        colliderType = BOX;
        size = point_t(25, 25);
        setupSprites({"closet.png"});
    }

    ObjectType type() override
    {
        return CLOSET;
    }

    GameObject* activeOccupant;
};

#endif
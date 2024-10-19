#ifndef __CLOSET_HH__
#define __CLOSET_HH__

#include "GameObject.hh"
#include "Container.hh"

class Closet : public Container
{
public:
    constexpr static int OCCUPANCY_SPACING = 60;

    Closet(int id)
        : Container(id, false, false)
    {
        colliderType = BOX;
        size = point_t(25, 25);
        setupSprites({"closet.png"});
    }

    ObjectType type() override
    {
        return CLOSET;
    }
};

#endif
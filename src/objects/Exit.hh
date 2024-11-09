#ifndef __EXIT_HH__
#define __EXIT_HH__

#include "GameObject.hh"

class Exit : public GameObject
{
public:
    Exit(int id)
        : GameObject(id)
    {
        colliderType = BOX;
        size = point_t(20, 20);
        setupSprites({"exit.png"});
    }

    ObjectType type() override
    {
        return EXIT;
    }
};

#endif

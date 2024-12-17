#ifndef __TURNSTILE_HH__
#define __TURNSTILE_HH__

#include "GameObject.hh"
#include "Container.hh"

class Turnstile: public Container
{

public:

    Turnstile(int id)
        : Container(id, true, false)
    {
        colliderType = BOX;
        size = point_t(25, 25);
        setupSprites({"turnstile.png"});
    }

    Turnstile(int id, Turnstile* ancestor)
        : Container(id, ancestor)
    {
    }

    ObjectType type() override
    {
        return TURNSTILE;
    }
};

#endif
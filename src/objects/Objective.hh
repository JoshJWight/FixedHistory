#ifndef __OBJECTIVE_HH__
#define __OBJECTIVE_HH__

#include "GameObject.hh"
#include "Throwable.hh"

class Objective : public Throwable
{
public:
    Objective(int id)
        : Throwable(id)
    {
        colliderType = CIRCLE;
        size = point_t(10, 10);
        setupSprites({"trophy.png"});
    }

    ObjectType type() override
    {
        return OBJECTIVE;
    }
};

#endif
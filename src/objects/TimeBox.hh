#ifndef __TIMEBOX_HH__
#define __TIMEBOX_HH__

#include "GameObject.hh"
#include "Container.hh"

class TimeBox: public Container
{

public:

    TimeBox(int id)
        : Container(id, true, true)
    {
        colliderType = BOX;
        size = point_t(25, 25);
        setupSprites({"box.png"});
    }

    TimeBox(int id, TimeBox* ancestor)
        : Container(id, ancestor)
    {
    }

    ObjectType type() override
    {
        return TIMEBOX;
    }
};

#endif
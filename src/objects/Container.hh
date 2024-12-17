#ifndef __CONTAINER_HH__
#define __CONTAINER_HH__

#include "GameObject.hh"

//Abstract parent class for TimeBox, Closet, and Turnstile

class Container: public GameObject
{
public:
    constexpr static int OCCUPANCY_SPACING = 60;

    Container(int id, bool _reverseOnEnter, bool _reverseOnExit)
        : GameObject(id)
        , activeOccupant(-1)
        , reverseOnEnter(_reverseOnEnter)
        , reverseOnExit(_reverseOnExit)
    {
    }

    Container(int id, Container* ancestor)
        : GameObject(id, ancestor)
        , activeOccupant(ancestor->activeOccupant)
        , reverseOnEnter(ancestor->reverseOnEnter)
        , reverseOnExit(ancestor->reverseOnExit)
    {
    }

    int activeOccupant;
    const bool reverseOnEnter;
    const bool reverseOnExit;
};

#endif
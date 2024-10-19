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
        , activeOccupant(nullptr)
        , reverseOnEnter(_reverseOnEnter)
        , reverseOnExit(_reverseOnExit)
    {
    }

    GameObject* activeOccupant;
    const bool reverseOnEnter;
    const bool reverseOnExit;
};

#endif
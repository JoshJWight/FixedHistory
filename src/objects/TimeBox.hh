#ifndef __TIMEBOX_HH__
#define __TIMEBOX_HH__

class TimeBox: public GameObject
{

public:
    constexpr static int OCCUPANCY_SPACING = 60;

    TimeBox(int id)
        : GameObject(id)
        , activeOccupant(nullptr)
    {
        colliderType = BOX;
        size = point_t(25, 25);
        setupSprites({"box.png"});
    }

    GameObject* activeOccupant;
};

#endif
#ifndef __ALARM_HH__
#define __ALARM_HH__

#include "GameObject.hh"

class Alarm : public GameObject
{
public:
    constexpr static float DEFAULT_RADIUS = 150;

    Alarm(int id)
        : GameObject(id)
    {
        colliderType = CIRCLE;
        size = point_t(20, 20);
        setupSprites({"alarm.png"});
    }

    Alarm(int id, Alarm* ancestor)
        : GameObject(id, ancestor)
    {
    }

    ObjectType type() override
    {
        return ALARM;
    }

    bool isTransient() override
    {
        return true;
    }

    //These should be updated every tick based on the state of enemies/crimes
    std::vector<int> enemies;
    std::vector<int> crimes;

};


#endif
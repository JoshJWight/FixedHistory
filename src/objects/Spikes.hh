#ifndef __SPIKES_HH__
#define __SPIKES_HH__

#include "GameObject.hh"

class Spikes : public GameObject
{
public:
    const static int WARNING_DURATION = 15;

    enum SpikesState
    {
        DOWN = 0,
        WARNING = 1,
        UP = 2
    };

    Spikes(int id)
        : GameObject(id)
        , downDuration(0)
        , upDuration(0)
        , cycleOffset(0)
    {
        colliderType = BOX;
        size = point_t(20, 20);
        setupSprites({"spikes_down.png", "spikes_warning.png", "spikes_up.png"});
    }

    Spikes(int id, int _downDuration, int _upDuration, int _cycleOffset)
        : GameObject(id)
        , downDuration(_downDuration)
        , upDuration(_upDuration)
        , cycleOffset(_cycleOffset)
    {
        colliderType = BOX;
        size = point_t(20, 20);
        setupSprites({"spikes_down.png", "spikes_warning.png", "spikes_up.png"});
    }

    Spikes(int id, Spikes* ancestor)
        : GameObject(id, ancestor)
        , downDuration(ancestor->downDuration)
        , upDuration(ancestor->upDuration)
        , cycleOffset(ancestor->cycleOffset)
    {
    }

    ObjectType type() override
    {
        return SPIKES;
    }

    int downDuration;
    int upDuration;
    int cycleOffset;
};

#endif
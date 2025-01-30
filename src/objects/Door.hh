#ifndef __DOOR_HH__
#define __DOOR_HH__

#include "GameObject.hh"
#include "Switch.hh"
#include <vector>

class Door : public GameObject
{
public:
    enum DoorState {
        CLOSED = 0,
        OPEN = 1
    };

    Door(int id);

    Door(int id, Door* ancestor)
        : GameObject(id, ancestor)
        , connectedSwitches(ancestor->connectedSwitches)
    {
    }

    void toggle();
    void addSwitch(Switch* sw);
    const std::vector<int>& getConnectedSwitches() const;

    bool isObstruction() override
    {
        return state.aiState == CLOSED;
    }

    ObjectType type() override
    {
        return DOOR;
    }

    std::vector<int> connectedSwitches;
};

#endif

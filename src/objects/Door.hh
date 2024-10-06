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

    void toggle();
    void addSwitch(Switch* sw);
    const std::vector<Switch*>& getConnectedSwitches() const;

    bool isObstruction() override
    {
        return state.aiState == CLOSED;
    }

private:
    std::vector<Switch*> m_connectedSwitches;
};

#endif

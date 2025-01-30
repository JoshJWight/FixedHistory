#include "Door.hh"
#include <io/TextureBank.hh>

Door::Door(int id)
    : GameObject(id)
{
    colliderType = BOX;
    size = point_t(20, 20);
    setupSprites({"Closed.png", "Open.png"});
}

void Door::addSwitch(Switch* sw)
{
    connectedSwitches.push_back(sw->id);
}

const std::vector<int>& Door::getConnectedSwitches() const
{
    return connectedSwitches;
}

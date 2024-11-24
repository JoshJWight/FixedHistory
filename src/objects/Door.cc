#include "Door.hh"
#include "TextureBank.hh"

Door::Door(int id)
    : GameObject(id)
{
    colliderType = BOX;
    size = point_t(20, 20);
    setupSprites({"Closed.png", "Open.png"});
}

void Door::addSwitch(Switch* sw)
{
    m_connectedSwitches.push_back(sw);
}

const std::vector<Switch*>& Door::getConnectedSwitches() const
{
    return m_connectedSwitches;
}

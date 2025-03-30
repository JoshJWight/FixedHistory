#include "Enemy.hh"
#include <io/TextureBank.hh>

Enemy::Enemy(int id)
    : GameObject(id)
    , assignedAlarm(-1)
{
    colliderType = CIRCLE;
    size = point_t(10, 10);
    setupSprites({"bored.png", "frowny.png", "enraged.png", "dead.png", "searching.png"});
}
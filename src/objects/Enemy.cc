#include "Enemy.hh"
#include "TextureBank.hh"

Enemy::Enemy(int id)
    : GameObject(id)
    , moveSpeed(0.5)
{
    colliderType = CIRCLE;
    size = point_t(10, 10);
    setupSprites({"bored.png", "frowny.png", "enraged.png", "dead.png", "searching.png"});
}
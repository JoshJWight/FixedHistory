#include "Enemy.hh"
#include "TextureBank.hh"

Enemy::Enemy(int id)
    : GameObject(id)
    , deathTimeline(-1)
    , moveSpeed(0.5)
{
    colliderType = CIRCLE;
    size = point_t(10, 10);
    sprite.setTexture(TextureBank::get("frowny.png"));
}
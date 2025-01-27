#include "Bullet.hh"
#include <io/TextureBank.hh>

Bullet::Bullet(int id)
    : GameObject(id)
    , velocity(0, 0)
{

    colliderType = CIRCLE;
    size = point_t(5, 5);
    setupSprites({"blam.png"});
}
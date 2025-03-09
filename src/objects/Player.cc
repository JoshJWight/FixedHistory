#include "Player.hh"
#include <io/TextureBank.hh>

Player::Player(int id)
    : GameObject(id)
    , moveSpeed(1.5)
    , fireCooldown(60)
{

    colliderType = CIRCLE;
    size = point_t(10, 10);
    setupSprites({"smiley.png"});
}

Player::Player(int id, Player* ancestor)
    : GameObject(id, ancestor)
    , moveSpeed(ancestor->moveSpeed)
    , fireCooldown(ancestor->fireCooldown)
{
}
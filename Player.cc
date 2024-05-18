#include "Player.hh"
#include "TextureBank.hh"

Player::Player(int id)
    : GameObject(id)
    , moveSpeed(1)
    , fireCooldown(60)
{

    colliderType = CIRCLE;
    size = point_t(10, 10);
    sprite.setTexture(TextureBank::get("smiley.png"));
}

Player::Player(int id, Player* ancestor)
    : GameObject(id, ancestor)
    , moveSpeed(ancestor->moveSpeed)
    , fireCooldown(ancestor->fireCooldown)
{
}
#include "Player.hh"

Player::Player(int id)
    : GameObject(id)
    , moveSpeed(1)
    , fireCooldown(60)
{
}

Player::Player(int id, Player* ancestor)
    : GameObject(id, ancestor)
    , moveSpeed(ancestor->moveSpeed)
    , fireCooldown(ancestor->fireCooldown)
{
}